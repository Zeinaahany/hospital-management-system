#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string.h>
using namespace std;
// =====================================================
//               PRIMARY INDEX
// =====================================================
struct PrimaryIndex
{
	int id, offset;
	bool operator < (const PrimaryIndex& pi) const
	{
		return id < pi.id;
	}
};

// =====================================================
//               DOCTOR
// =====================================================
vector<PrimaryIndex> doctorIndexList;

void writeDoctorIndexFile()
{
	ofstream outFile("DoctorIndex.txt", ios::binary | ios::out | ios::trunc);
	for (int i = 0; i < (int)doctorIndexList.size(); i++)
	{
		PrimaryIndex pi = doctorIndexList[i];
		outFile.write((char*)&pi, sizeof(pi));
	}
	outFile.close();
}
void readDoctorIndexFile()
{
	doctorIndexList.clear();
	ifstream inFile("DoctorIndex.txt", ios::binary | ios::in);
	if (!inFile) return;
	PrimaryIndex pi;
	while (inFile.read((char*)&pi, sizeof(pi)))
		doctorIndexList.push_back(pi);
	inFile.close();
}
void addToDoctorIndexList(PrimaryIndex pi)
{
	doctorIndexList.push_back(pi);
	sort(doctorIndexList.begin(), doctorIndexList.end());
}
void deleteFromDoctorIndexList(const int& id)
{
	int low = 0, high = (int)doctorIndexList.size() - 1;
	while (low <= high)
	{
		int mid = (low + high) / 2;
		if (doctorIndexList[mid].id == id)
		{
			doctorIndexList.erase(doctorIndexList.begin() + mid);
			writeDoctorIndexFile();
			return;
		}
		else if (doctorIndexList[mid].id < id) low = mid + 1;
		else high = mid - 1;
	}
}
int searchInDoctorIndexList(const int& id)
{
	int low = 0, high = (int)doctorIndexList.size() - 1;
	while (low <= high)
	{
		int mid = (low + high) / 2;
		if (doctorIndexList[mid].id == id) return doctorIndexList[mid].offset;
		else if (doctorIndexList[mid].id < id) low = mid + 1;
		else high = mid - 1;
	}
	return -1;
}

struct Doctor
{
	int id;
	char name[30];
	char specialty[30];
	char phone[15];

	void solveGetLineProblem()
	{
		cin.ignore(1000, '\n');
	}
	void readDoctor(bool isupdate = false)
	{
		if (!isupdate)
		{
			while (true)
			{
				int tempId;
				cout << "Enter Doctor ID: ";
				cin >> tempId;
				solveGetLineProblem();
				if (searchInDoctorIndexList(tempId) != -1)
					cout << "Error: ID " << tempId << " already exists. Please enter a unique ID.\n";
				else { id = tempId; break; }
			}
		}
		do
		{
			cout << "Enter Doctor Name (min 5 chars): ";
			cin.getline(name, 30);
			if (strlen(name) < 5)
				cout << ">> Error: Name too short! At least 5 characters.\n";
		} while (strlen(name) < 5);

		cout << "Enter Doctor Specialty: ";
		cin.getline(specialty, 30);

		cout << "Enter Doctor Phone: ";
		cin.getline(phone, 15);
	}
	void displayDoctor()
	{
		cout << "Doctor ID       : " << id << endl;
		cout << "Doctor Name     : " << name << endl;
		cout << "Doctor Specialty: " << specialty << endl;
		cout << "Doctor Phone    : " << phone << endl;
	}
	short calcLength()
	{
		short fixedLen = sizeof(int);
		short varLen = (short)(strlen(name) + strlen(specialty) + strlen(phone));
		short lengthIndicator = 3 * sizeof(short);
		return fixedLen + varLen + lengthIndicator;
	}
};

void writeDoctorToFile(fstream& ioFile, const Doctor& d)
{
	ioFile.write((char*)&d.id, sizeof(d.id));

	short len = (short)strlen(d.name);
	ioFile.write((char*)&len, sizeof(len));
	ioFile.write(d.name, len);

	len = (short)strlen(d.specialty);
	ioFile.write((char*)&len, sizeof(len));
	ioFile.write(d.specialty, len);

	len = (short)strlen(d.phone);
	ioFile.write((char*)&len, sizeof(len));
	ioFile.write(d.phone, len);
}
void readDoctorFromFile(fstream& ioFile, Doctor& d)
{
	ioFile.read((char*)&d.id, sizeof(d.id));
	short len;

	ioFile.read((char*)&len, sizeof(len));
	ioFile.read(d.name, len);
	d.name[len] = '\0';

	ioFile.read((char*)&len, sizeof(len));
	ioFile.read(d.specialty, len);
	d.specialty[len] = '\0';

	ioFile.read((char*)&len, sizeof(len));
	ioFile.read(d.phone, len);
	d.phone[len] = '\0';
}

short readDoctorHeader(fstream& ioFile)
{
	short header;
	ioFile.seekg(0);
	ioFile.read((char*)&header, sizeof(header));
	return header;
}
void writeDoctorHeader(fstream& ioFile, short header)
{
	ioFile.seekp(0);
	ioFile.write((char*)&header, sizeof(header));
	ioFile.flush();
}

void firstFitDoctor(fstream& ioFile, Doctor& d)
{

	short header = readDoctorHeader(ioFile);
	short neededSize = d.calcLength();

	if (header == -1)
	{
		ioFile.seekp(0, ios::end);
		int offset = (int)ioFile.tellp();
		addToDoctorIndexList({ d.id, offset });
		writeDoctorToFile(ioFile, d);
	}
	else
	{

		short currentPos = header, prevPos = -1;
		while (currentPos != -1)
		{

			ioFile.seekg(currentPos + sizeof(int) + sizeof(short) + sizeof(char));
			short nextDeleted, deletedSize;
			ioFile.read((char*)&nextDeleted, sizeof(nextDeleted));
			ioFile.read((char*)&deletedSize, sizeof(deletedSize));

			if (deletedSize >= neededSize)
			{
				ioFile.seekp(currentPos);
				addToDoctorIndexList({ d.id, currentPos });
				writeDoctorToFile(ioFile, d);

				short padding = deletedSize - neededSize;
				while (padding--) ioFile.put('#');
				if (prevPos == -1)
					writeDoctorHeader(ioFile, nextDeleted);
				else
				{
					ioFile.seekp(prevPos + sizeof(int) + sizeof(short) + sizeof(char));
					ioFile.write((char*)&nextDeleted, sizeof(nextDeleted));
					ioFile.flush();
				}
				return;
			}
			prevPos = currentPos;
			currentPos = nextDeleted;
		}
		ioFile.seekp(0, ios::end);
		int offset = (int)ioFile.tellp();
		addToDoctorIndexList({ d.id, offset });
		writeDoctorToFile(ioFile, d);
	}
	ioFile.flush();
}


void addNewDoctorToFile(fstream& ioFile, Doctor& d)
{
	firstFitDoctor(ioFile, d);
	writeDoctorIndexFile();
	cout << "Doctor added successfully.\n";
}                 // ali
void displayAllDoctors(fstream& ioFile)
{
	if (doctorIndexList.empty()) { cout << "No doctors found.\n"; return; }
	Doctor d;
	cout << "====== Doctors ======\n";
	for (int i = 0; i < (int)doctorIndexList.size(); i++)
	{
		ioFile.clear();
		ioFile.seekg(doctorIndexList[i].offset);
		readDoctorFromFile(ioFile, d);
		d.displayDoctor();
		cout << "\n-----------------------------------\n";
	}
}                                     // selawn
void findDoctorById(fstream& ioFile, const int id)
{
	/*
لما المستخدم يبحث عن ID
البرنامج بيروح للـ menu ويستقبل الـ id
ثم ينادي findDoctorById

الفانكشن دي بتستخدم searchInDoctorIndexList
عشان تعمل Binary Search في الـ index

لو لقت الـ id ترجع الـ offset

بعدها البرنامج يستخدم seekg
عشان يروح لمكان الريكورد في الفايل

ثم readDoctorFromFile
لقراءة البيانات وتحويلها لـ struct

وأخيرًا displayDoctor لعرض النتيجة
*/

	Doctor d;
	int offset = searchInDoctorIndexList(id);
	if (offset == -1)
		cout << "Doctor Not Found.\n";
	else
	{
		ioFile.clear();
		ioFile.seekg(offset);
		readDoctorFromFile(ioFile, d);
		cout << "Doctor Found.\n";
		d.displayDoctor();
		cout << "\n--------------------------\n";
	}
}               // mohamed
void deleteDoctorById(fstream& ioFile, const int id)
{
	/*
	لما المستخدم يعمل delete
البرنامج بيروح للـ menu ويقرأ الـ id

ثم ينادي deleteDoctorById

أول حاجة بيقرأ الـ header (بداية الـ avail list)

بعد كده يستخدم searchInDoctorIndexList
عشان يجيب الـ offset

لو موجود → يقرأ الريكورد ويعرضه

بعد كده يحوله لـ deleted record
عن طريق كتابة '*'

ثم يربطه بالـ avail list
بحيث يشاور على القديم

بعد كده يحدّث الـ header
عشان يبقى أول عنصر

وأخيرًا يحذفه من الـ index
	*/
	short header = readDoctorHeader(ioFile);

	Doctor d;
	int offsetInt = searchInDoctorIndexList(id);
	if (offsetInt == -1) {
		cout << "Doctor Not Found.\n";
		return;
	}

	ioFile.clear();
	ioFile.seekg(offsetInt);
	readDoctorFromFile(ioFile, d);
	d.displayDoctor();
	cout << "\n--------------------------\n";

	short offset = (short)offsetInt;
	ioFile.seekp(offsetInt + sizeof(int) + sizeof(short));
	ioFile.put('*');
	ioFile.write((char*)&header, sizeof(header));
	short deletedSize = d.calcLength();
	ioFile.write((char*)&deletedSize, sizeof(deletedSize));
	ioFile.flush();

	writeDoctorHeader(ioFile, offset);

	deleteFromDoctorIndexList(id);
	cout << "Doctor deleted successfully.\n";
}             // mohamed
void updateDoctorById(fstream& ioFile, const int id)
{
	short header = readDoctorHeader(ioFile);

	Doctor d;
	int offsetInt = searchInDoctorIndexList(id);
	if (offsetInt == -1)
	{
		cout << "Doctor Not Found.\n";
		return;
	}

	ioFile.clear();
	ioFile.seekg(offsetInt);
	readDoctorFromFile(ioFile, d);
	cout << "Current Data::\n";
	d.displayDoctor();
	cout << "\n--------------------------\n";
	short oldLen = d.calcLength();

	cout << "Edit Data::\n";
	d.readDoctor(true);
	cout << "New Data::\n";
	d.displayDoctor();
	cout << "\n--------------------------\n";
	short newLen = d.calcLength();

	if (oldLen >= newLen)
	{
		ioFile.seekp(offsetInt);
		writeDoctorToFile(ioFile, d);
		short padding = oldLen - newLen;
		while (padding--) ioFile.put('#');
		ioFile.flush();
	}
	else
	{
		short offset = (short)offsetInt;
		ioFile.seekp(offsetInt + sizeof(int) + sizeof(short));
		ioFile.put('*');
		ioFile.write((char*)&header, sizeof(header));
		ioFile.write((char*)&oldLen, sizeof(oldLen));
		ioFile.flush();

		writeDoctorHeader(ioFile, offset);
		deleteFromDoctorIndexList(id);

		ioFile.seekp(0, ios::end);
		int newOffset = (int)ioFile.tellp();
		addToDoctorIndexList({ d.id, newOffset });
		writeDoctorToFile(ioFile, d);
		ioFile.flush();
	}
	writeDoctorIndexFile();
	cout << "Doctor updated successfully.\n";
}                // ali
void displayTop5Doctors(fstream& ioFile)
{
	if (doctorIndexList.empty()) { cout << "No doctors found.\n"; return; }
	Doctor d;
	int limit = min(5, (int)doctorIndexList.size());
	cout << "====== Top 5 Doctors ======\n";
	for (int i = 0; i < limit; i++)
	{
		ioFile.clear();
		ioFile.seekg(doctorIndexList[i].offset);
		readDoctorFromFile(ioFile, d);
		d.displayDoctor();
		cout << "\n-----------------------------------\n";
	}
}                                // selwan
void findDoctorByName(fstream& ioFile, const char* name)
{
	Doctor d;
	bool found = false;
	for (int i = 0; i < (int)doctorIndexList.size(); i++)
	{
		ioFile.clear();
		ioFile.seekg(doctorIndexList[i].offset);
		readDoctorFromFile(ioFile, d);
		if (strcmp(d.name, name) == 0)
		{
			cout << "Doctor Found.\n";
			d.displayDoctor();
			cout << "\n--------------------------\n";
			found = true;
			break;
		}
	}
	if (!found) cout << "Doctor Not Found.\n";
}              // shahd
void deleteDoctorByName(fstream& ioFile, const char* name)
{
	short header = readDoctorHeader(ioFile);

	Doctor d;
	bool found = false;
	for (int i = 0; i < (int)doctorIndexList.size(); i++)
	{
		ioFile.clear();
		ioFile.seekg(doctorIndexList[i].offset);
		readDoctorFromFile(ioFile, d);
		if (strcmp(d.name, name) == 0)
		{
			int offsetInt = doctorIndexList[i].offset;
			short offset = (short)offsetInt;
			d.displayDoctor();
			cout << "\n--------------------------\n";

			ioFile.seekp(offsetInt + sizeof(int) + sizeof(short));
			ioFile.put('*');
			ioFile.write((char*)&header, sizeof(header));
			short deletedSize = d.calcLength();
			ioFile.write((char*)&deletedSize, sizeof(deletedSize));
			ioFile.flush();

			writeDoctorHeader(ioFile, offset);

			deleteFromDoctorIndexList(d.id);
			cout << "Doctor deleted successfully.\n";
			found = true;
			break;
		}
	}
	if (!found) cout << "Doctor Not Found.\n";
}            // shahd
void updateDoctorByName(fstream& ioFile, const char* name)
{
	short header = readDoctorHeader(ioFile);

	Doctor d;
	bool found = false;
	for (int i = 0; i < (int)doctorIndexList.size(); i++)
	{
		ioFile.clear();
		ioFile.seekg(doctorIndexList[i].offset);
		readDoctorFromFile(ioFile, d);
		if (strcmp(d.name, name) == 0)
		{
			int offsetInt = doctorIndexList[i].offset;
			short offset = (short)offsetInt;
			cout << "Current Data::\n";
			d.displayDoctor();
			cout << "\n--------------------------\n";
			short oldLen = d.calcLength();

			cout << "Edit Data::\n";
			d.readDoctor(true);
			cout << "New Data::\n";
			d.displayDoctor();
			cout << "\n--------------------------\n";
			short newLen = d.calcLength();

			if (oldLen >= newLen)
			{
				ioFile.seekp(offsetInt);
				writeDoctorToFile(ioFile, d);
				short padding = oldLen - newLen;
				while (padding--) ioFile.put('#');
				ioFile.flush();
			}
			else
			{
				ioFile.seekp(offsetInt + sizeof(int) + sizeof(short));
				ioFile.put('*');
				ioFile.write((char*)&header, sizeof(header));
				ioFile.write((char*)&oldLen, sizeof(oldLen));
				ioFile.flush();

				writeDoctorHeader(ioFile, offset);
				deleteFromDoctorIndexList(d.id);

				ioFile.seekp(0, ios::end);
				int newOffset = (int)ioFile.tellp();
				addToDoctorIndexList({ d.id, newOffset });
				writeDoctorToFile(ioFile, d);
				ioFile.flush();
			}
			writeDoctorIndexFile();
			cout << "Doctor updated successfully.\n";
			found = true;
			break;
		}
	}
	if (!found) cout << "Doctor Not Found.\n";
}     // zeina

void initDoctorFile()
{
	fstream test;
	test.open("Doctors.txt", ios::binary | ios::in | ios::out);
	if (!test)
	{
		ofstream out("Doctors.txt", ios::binary | ios::out);
		short header = -1;
		out.write((char*)&header, sizeof(header));
		out.close();
	}
	else test.close();
}

void DoctorsMenu()
{
	initDoctorFile();
	fstream ioFile;
	ioFile.open("Doctors.txt", ios::binary | ios::in | ios::out);
	if (!ioFile) { cout << "Error opening Doctors file!\n"; return; }
	readDoctorIndexFile();

	int choice, id;
	char nameInput[30];
	Doctor d;
	do
	{
		cout << "\n========== Doctor Menu ==========\n";
		cout << "1.  Add New Doctor.\n";
		cout << "2.  Display All Doctors.\n";
		cout << "3.  Find Doctor By ID.\n";
		cout << "4.  Update Doctor By ID.\n";
		cout << "5.  Delete Doctor By ID.\n";
		cout << "6.  Find Doctor By Name.\n";
		cout << "7.  Update Doctor By Name.\n";
		cout << "8.  Delete Doctor By Name.\n";
		cout << "9. Display Top 5 Doctors.\n";
		cout << "0.  Back.\n";
		cout << "Enter Choice: ";
		cin >> choice;
		cin.ignore(1000, '\n');

		switch (choice)
		{
		case 1: d.readDoctor(); addNewDoctorToFile(ioFile, d); break;
		case 2: displayAllDoctors(ioFile); break;
		case 3: cout << "Enter ID: "; cin >> id; cin.ignore(1000, '\n'); findDoctorById(ioFile, id); break;
		case 4: cout << "Enter ID: "; cin >> id; cin.ignore(1000, '\n'); updateDoctorById(ioFile, id); break;
		case 5: cout << "Enter ID: "; cin >> id; cin.ignore(1000, '\n'); deleteDoctorById(ioFile, id); break;
		case 6: cout << "Enter Name: "; cin.getline(nameInput, 30); findDoctorByName(ioFile, nameInput); break;
		case 7: cout << "Enter Name: "; cin.getline(nameInput, 30); updateDoctorByName(ioFile, nameInput); break;
		case 8: cout << "Enter Name: "; cin.getline(nameInput, 30); deleteDoctorByName(ioFile, nameInput); break;
		case 9: displayTop5Doctors(ioFile); break;
		case 0: cout << "Back...\n"; break;
		default: cout << "Invalid Choice!\n";
		}
		if (choice != 0) { system("pause"); system("cls"); }
		ioFile.clear();
	} while (choice != 0);

	ioFile.flush();
	ioFile.close();
}

// =====================================================
//               PATIENT
// =====================================================
vector<PrimaryIndex> patientIndexList;

void writePatientIndexFile()
{
	ofstream outFile("PatientIndex.txt", ios::binary | ios::out | ios::trunc);
	for (int i = 0; i < (int)patientIndexList.size(); i++)
	{
		PrimaryIndex pi = patientIndexList[i];
		outFile.write((char*)&pi, sizeof(pi));
	}
	outFile.close();
}
void readPatientIndexFile()
{
	patientIndexList.clear();
	ifstream inFile("PatientIndex.txt", ios::binary | ios::in);
	if (!inFile) return;
	PrimaryIndex pi;
	while (inFile.read((char*)&pi, sizeof(pi)))
		patientIndexList.push_back(pi);
	inFile.close();
}
void addToPatientIndexList(PrimaryIndex pi)
{
	patientIndexList.push_back(pi);
	sort(patientIndexList.begin(), patientIndexList.end());
}
void deleteFromPatientIndexList(const int& id)
{
	int low = 0, high = (int)patientIndexList.size() - 1;
	while (low <= high)
	{
		int mid = (low + high) / 2;
		if (patientIndexList[mid].id == id)
		{
			patientIndexList.erase(patientIndexList.begin() + mid);
			writePatientIndexFile();
			return;
		}
		else if (patientIndexList[mid].id < id) low = mid + 1;
		else high = mid - 1;
	}
}
int searchInPatientIndexList(const int& id)
{
	int low = 0, high = (int)patientIndexList.size() - 1;
	while (low <= high)
	{
		int mid = (low + high) / 2;
		if (patientIndexList[mid].id == id) return patientIndexList[mid].offset;
		else if (patientIndexList[mid].id < id) low = mid + 1;
		else high = mid - 1;
	}
	return -1;
}

struct Patient
{
	int id;
	int age;
	char name[30];
	char diagnosis[50];

	void solveGetLineProblem()
	{
		cin.ignore(1000, '\n');
	}
	void readPatient(bool isupdate = false)
	{
		if (!isupdate)
		{
			while (true)
			{
				int tempId;
				cout << "Enter Patient ID: ";
				cin >> tempId;
				cin.ignore(1000, '\n');
				if (searchInPatientIndexList(tempId) != -1)
					cout << "Error: ID " << tempId << " already exists. Please enter a unique ID.\n";
				else { id = tempId; break; }
			}
		}
		do
		{
			cout << "Enter Patient Name (min 5 chars): ";
			cin.getline(name, 30);
			if (strlen(name) < 5)
				cout << ">> Error: Name too short! At least 5 characters.\n";
		} while (strlen(name) < 5);

		cout << "Enter Patient Age: ";
		cin >> age;
		cin.ignore(1000, '\n');

		cout << "Enter Patient Diagnosis: ";
		cin.getline(diagnosis, 50);
	}
	void displayPatient()
	{
		cout << "Patient ID       : " << id << endl;
		cout << "Patient Name     : " << name << endl;
		cout << "Patient Age      : " << age << endl;
		cout << "Patient Diagnosis: " << diagnosis << endl;
	}
	short calcLength()
	{
		short fixedLen = sizeof(int) + sizeof(int); // id + age
		short varLen = (short)(strlen(name) + strlen(diagnosis));
		short lengthIndicator = 2 * sizeof(short);
		return fixedLen + varLen + lengthIndicator;
	}
};

void writePatientToFile(fstream& ioFile, const Patient& p)
{
	ioFile.write((char*)&p.id, sizeof(p.id));
	ioFile.write((char*)&p.age, sizeof(p.age));

	short len = (short)strlen(p.name);
	ioFile.write((char*)&len, sizeof(len));
	ioFile.write(p.name, len);

	len = (short)strlen(p.diagnosis);
	ioFile.write((char*)&len, sizeof(len));
	ioFile.write(p.diagnosis, len);
}
void readPatientFromFile(fstream& ioFile, Patient& p)
{
	ioFile.read((char*)&p.id, sizeof(p.id));
	ioFile.read((char*)&p.age, sizeof(p.age));

	short len;
	ioFile.read((char*)&len, sizeof(len));
	ioFile.read(p.name, len);
	p.name[len] = '\0';

	ioFile.read((char*)&len, sizeof(len));
	ioFile.read(p.diagnosis, len);
	p.diagnosis[len] = '\0';
}

//بيحددو مكان الاوفسيت
//* nextdeleted deletedsize
const int PATIENT_MARKER_OFFSET = sizeof(int) + sizeof(int) + sizeof(short);//id +age+len
const int DOCTOR_MARKER_OFFSET  = sizeof(int) + sizeof(short);//id+len

short readPatientHeader(fstream& ioFile)
{
	short header;
	ioFile.seekg(0);
	ioFile.read((char*)&header, sizeof(header));
	return header;
}
void writePatientHeader(fstream& ioFile, short header)
{
	ioFile.seekp(0);
	ioFile.write((char*)&header, sizeof(header));
	ioFile.flush();
}

void firstFitPatient(fstream& ioFile, Patient& p)
{
	short header = readPatientHeader(ioFile);
	short neededSize = p.calcLength();

	if (header == -1)
	{
		ioFile.seekp(0, ios::end);
		int offset = (int)ioFile.tellp();
		addToPatientIndexList({ p.id, offset });
		writePatientToFile(ioFile, p);
	}
	else
	{
		short currentPos = header, prevPos = -1;
		while (currentPos != -1)
		{
			ioFile.clear();
			ioFile.seekg(currentPos + PATIENT_MARKER_OFFSET + sizeof(char));
			short nextDeleted, deletedSize;
			ioFile.read((char*)&nextDeleted, sizeof(nextDeleted));
			ioFile.read((char*)&deletedSize, sizeof(deletedSize));

			if (deletedSize >= neededSize)
			{
				ioFile.seekp(currentPos);
				addToPatientIndexList({ p.id, currentPos });
				writePatientToFile(ioFile, p);
				short padding = deletedSize - neededSize;
				while (padding--) ioFile.put('#');

				if (prevPos == -1)
					writePatientHeader(ioFile, nextDeleted);
				else
				{
					ioFile.seekp(prevPos + PATIENT_MARKER_OFFSET + sizeof(char));
					ioFile.write((char*)&nextDeleted, sizeof(nextDeleted));
					ioFile.flush();
				}
				return;
			}
			prevPos = currentPos;
			currentPos = nextDeleted;
		}
		ioFile.seekp(0, ios::end);
		int offset = (int)ioFile.tellp();
		addToPatientIndexList({ p.id, offset });
		writePatientToFile(ioFile, p);
	}
	ioFile.flush();
}

void addNewPatientToFile(fstream& ioFile, Patient& p)
{
	firstFitPatient(ioFile, p);
	writePatientIndexFile();
	cout << "Patient added successfully.\n";
}
void displayAllPatients(fstream& ioFile)
{
	if (patientIndexList.empty()) { cout << "No patients found.\n"; return; }
	Patient p;
	cout << "====== Patients ======\n";
	for (int i = 0; i < (int)patientIndexList.size(); i++)
	{
		ioFile.clear();
		ioFile.seekg(patientIndexList[i].offset);
		readPatientFromFile(ioFile, p);
		p.displayPatient();
		cout << "\n-----------------------------------\n";
	}
}
void findPatientById(fstream& ioFile, const int id)
{
	Patient p;
	int offset = searchInPatientIndexList(id);
	if (offset == -1)
		cout << "Patient Not Found.\n";
	else
	{
		ioFile.clear();
		ioFile.seekg(offset);
		readPatientFromFile(ioFile, p);
		cout << "Patient Found.\n";
		p.displayPatient();
		cout << "\n--------------------------\n";
	}
}
void deletePatientById(fstream& ioFile, const int id)
{
	short header = readPatientHeader(ioFile);

	Patient p;
	int offsetInt = searchInPatientIndexList(id);
	if (offsetInt == -1)
	{
		cout << "Patient Not Found.\n";
		return;
	}

	ioFile.clear();
	ioFile.seekg(offsetInt);
	readPatientFromFile(ioFile, p);
	p.displayPatient();
	cout << "\n--------------------------\n";

	short offset = (short)offsetInt;
	ioFile.seekp(offsetInt + PATIENT_MARKER_OFFSET);
	ioFile.put('*');
	ioFile.write((char*)&header, sizeof(header));
	short deletedSize = p.calcLength();
	ioFile.write((char*)&deletedSize, sizeof(deletedSize));
	ioFile.flush();

	writePatientHeader(ioFile, offset);

	deleteFromPatientIndexList(id);
	cout << "Patient deleted successfully.\n";
}
void updatePatientById(fstream& ioFile, const int id)
{
	short header = readPatientHeader(ioFile);

	Patient p;
	int offsetInt = searchInPatientIndexList(id);
	if (offsetInt == -1)
	{
		cout << "Patient Not Found.\n";
		return;
	}

	ioFile.clear();
	ioFile.seekg(offsetInt);
	readPatientFromFile(ioFile, p);
	cout << "Current Data::\n";
	p.displayPatient();
	cout << "\n--------------------------\n";
	short oldLen = p.calcLength();

	cout << "Edit Data::\n";
	p.readPatient(true);
	cout << "New Data::\n";
	p.displayPatient();
	cout << "\n--------------------------\n";
	short newLen = p.calcLength();

	if (oldLen >= newLen)
	{
		ioFile.seekp(offsetInt);
		writePatientToFile(ioFile, p);
		short padding = oldLen - newLen;
		while (padding--) ioFile.put('#');
		ioFile.flush();
	}
	else
	{
		short offset = (short)offsetInt;
		ioFile.seekp(offsetInt + PATIENT_MARKER_OFFSET);
		ioFile.put('*');
		ioFile.write((char*)&header, sizeof(header));
		ioFile.write((char*)&oldLen, sizeof(oldLen));
		ioFile.flush();

		writePatientHeader(ioFile, offset);
		deleteFromPatientIndexList(id);

		ioFile.seekp(0, ios::end);
		int newOffset = (int)ioFile.tellp();
		addToPatientIndexList({ p.id, newOffset });
		writePatientToFile(ioFile, p);
		ioFile.flush();
	}
	writePatientIndexFile();
	cout << "Patient updated successfully.\n";
}
void displayTop5Patients(fstream& ioFile)
{
	if (patientIndexList.empty()) { cout << "No patients found.\n"; return; }
	Patient p;
	int limit = min(5, (int)patientIndexList.size());
	cout << "====== Top 5 Patients ======\n";
	for (int i = 0; i < limit; i++)
	{
		ioFile.clear();
		ioFile.seekg(patientIndexList[i].offset);
		readPatientFromFile(ioFile, p);
		p.displayPatient();
		cout << "\n-----------------------------------\n";
	}
}
void findPatientByName(fstream& ioFile, const char* name)
{
	Patient p;
	bool found = false;
	for (int i = 0; i < (int)patientIndexList.size(); i++)
	{
		ioFile.clear();
		ioFile.seekg(patientIndexList[i].offset);
		readPatientFromFile(ioFile, p);
		if (strcmp(p.name, name) == 0)
		{
			cout << "Patient Found.\n";
			p.displayPatient();
			cout << "\n--------------------------\n";
			found = true;
			break;
		}
	}
	if (!found) cout << "Patient Not Found.\n";
}
void deletePatientByName(fstream& ioFile, const char* name)
{
	short header = readPatientHeader(ioFile);

	Patient p;
	bool found = false;
	for (int i = 0; i < (int)patientIndexList.size(); i++)
	{
		ioFile.clear();
		ioFile.seekg(patientIndexList[i].offset);
		readPatientFromFile(ioFile, p);
		if (strcmp(p.name, name) == 0)
		{
			int offsetInt = patientIndexList[i].offset;
			short offset = (short)offsetInt;
			p.displayPatient();
			cout << "\n--------------------------\n";
			ioFile.seekp(offsetInt + PATIENT_MARKER_OFFSET);
			ioFile.put('*');
			ioFile.write((char*)&header, sizeof(header));
			short deletedSize = p.calcLength();
			ioFile.write((char*)&deletedSize, sizeof(deletedSize));
			ioFile.flush();

			writePatientHeader(ioFile, offset);

			deleteFromPatientIndexList(p.id);
			cout << "Patient deleted successfully.\n";
			found = true;
			break;
		}
	}
	if (!found) cout << "Patient Not Found.\n";
}
void updatePatientByName(fstream& ioFile, const char* name)
{

	short header = readPatientHeader(ioFile);
	Patient p;
	bool found = false;
	for (int i = 0; i < (int)patientIndexList.size(); i++)
	{
		ioFile.clear();
		ioFile.seekg(patientIndexList[i].offset);
		readPatientFromFile(ioFile, p);
		if (strcmp(p.name, name) == 0)
		{
			int offsetInt = patientIndexList[i].offset;//specific for seek
			short offset = (short)offsetInt;//specific for header
			cout << "Current Data::\n";
			p.displayPatient();
			cout << "\n--------------------------\n";
			short oldLen = p.calcLength();

			cout << "Edit Data::\n";
			p.readPatient(true);//to not change id
			cout << "New Data::\n";
			p.displayPatient();
			cout << "\n--------------------------\n";
			short newLen = p.calcLength();

			if (oldLen >= newLen)
			{
				ioFile.seekp(offsetInt);
				writePatientToFile(ioFile, p);
				short padding = oldLen - newLen;
				while (padding--) ioFile.put('#');
				ioFile.flush();
			}
			else
			{
				// FIX: استخدم الأوفست الصح للـ patient marker
				ioFile.seekp(offsetInt + PATIENT_MARKER_OFFSET);
				ioFile.put('*');
				ioFile.write((char*)&header, sizeof(header));
				ioFile.write((char*)&oldLen, sizeof(oldLen));
				ioFile.flush();

				writePatientHeader(ioFile, offset);//update header
				deleteFromPatientIndexList(p.id);

				ioFile.seekp(0, ios::end);
				int newOffset = (int)ioFile.tellp();
				addToPatientIndexList({ p.id, newOffset });
				writePatientToFile(ioFile, p);
				ioFile.flush();
			}
			writePatientIndexFile();
			cout << "Patient updated successfully.\n";
			found = true;
			break;
		}
	}
	if (!found) cout << "Patient Not Found.\n";
}

void initPatientFile()
{
	fstream test;
	test.open("Patients.txt", ios::binary | ios::in | ios::out);
	if (!test)
	{
		ofstream out("Patients.txt", ios::binary | ios::out);
		short header = -1;
		out.write((char*)&header, sizeof(header));
		out.close();
	}
	else test.close();
}

void PatientsMenu()
{
	initPatientFile();
	fstream ioFile;
	ioFile.open("Patients.txt", ios::binary | ios::in | ios::out);
	if (!ioFile) { cout << "Error opening Patients file!\n"; return; }
	readPatientIndexFile();

	int choice, id;
	char nameInput[30];
	Patient p;
	do
	{
		cout << "\n========== Patient Menu ==========\n";
		cout << "1.  Add New Patient.\n";
		cout << "2.  Display All Patients.\n";
		cout << "3.  Find Patient By ID.\n";
		cout << "4.  Update Patient By ID.\n";
		cout << "5.  Delete Patient By ID.\n";
		cout << "6.  Find Patient By Name.\n";
		cout << "7.  Update Patient By Name.\n";
		cout << "8.  Delete Patient By Name.\n";
		cout << "9. Display Top 5 Patients.\n";
		cout << "0.  Back.\n";
		cout << "Enter Choice: ";
		cin >> choice;
		cin.ignore(1000, '\n');

		switch (choice)
		{
		case 1: p.readPatient(); addNewPatientToFile(ioFile, p); break;
		case 2: displayAllPatients(ioFile); break;
		case 3: cout << "Enter ID: "; cin >> id; cin.ignore(1000, '\n'); findPatientById(ioFile, id); break;
		case 4: cout << "Enter ID: "; cin >> id; cin.ignore(1000, '\n'); updatePatientById(ioFile, id); break;
		case 5: cout << "Enter ID: "; cin >> id; cin.ignore(1000, '\n'); deletePatientById(ioFile, id); break;
		case 6: cout << "Enter Name: "; cin.getline(nameInput, 30); findPatientByName(ioFile, nameInput); break;
		case 7: cout << "Enter Name: "; cin.getline(nameInput, 30); updatePatientByName(ioFile, nameInput); break;
		case 8: cout << "Enter Name: "; cin.getline(nameInput, 30); deletePatientByName(ioFile, nameInput); break;
		case 9: displayTop5Patients(ioFile); break;
		case 0: cout << "Back...\n"; break;
		default: cout << "Invalid Choice!\n";
		}
		if (choice != 0) { system("pause"); system("cls"); }
		ioFile.clear();
	} while (choice != 0);

	ioFile.flush();
	ioFile.close();
}

// =====================================================
//               MAIN MENU
// =====================================================
int main()
{
	int choice;
	do
	{
		cout << "\n========== Hospital System ==========\n";
		cout << "1. Doctors.\n";
		cout << "2. Patients.\n";
		cout << "0. Exit.\n";
		cout << "Enter Choice: ";
		cin >> choice;
		cin.ignore(1000, '\n');

		switch (choice)
		{
		case 1: DoctorsMenu(); break;
		case 2: PatientsMenu(); break;
		case 0: cout << "Goodbye!\n"; break;
		default: cout << "Invalid Choice!\n";
		}
	} while (choice != 0);

	return 0;
}