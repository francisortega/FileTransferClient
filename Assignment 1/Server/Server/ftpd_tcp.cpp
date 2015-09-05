// SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.

// @author Francis Ortega 1295578
// @version 1.0 10-03-2014

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <fstream>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <vector>
#include <comdef.h> 
#pragma comment(lib, "User32.lib")



using namespace std;

//port data types

#define REQUEST_PORT 0x7070

int port = REQUEST_PORT;

//socket data types
SOCKET s;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port
union {
	struct sockaddr generic;
	struct sockaddr_in ca_in;
}ca;

int calen = sizeof(ca);

//buffer data types
char clientbuffer[128];

char *buffer;
int ibufferlen;
int ibytesrecv;

int ibytessent;

//host data types
char localhost[20];

HOSTENT *hp;

//wait variables
int nsa1;
int r, infds = 1, outfds = 0;
struct timeval timeout;
const struct timeval *tp = &timeout;

fd_set readfds;

//others
HANDLE test;

DWORD dwtest;

//reference for used structures

/*  * Host structure

struct  hostent {
char    FAR * h_name;             official name of host *
char    FAR * FAR * h_aliases;    alias list *
short   h_addrtype;               host address type *
short   h_length;                 length of address *
char    FAR * FAR * h_addr_list;  list of addresses *
#define h_addr  h_addr_list[0]            address, for backward compat *
};

* Socket address structure

struct sockaddr_in {
short   sin_family;
u_short sin_port;
struct  in_addr sin_addr;
char    sin_zero[8];
}; */

unsigned __stdcall ClientSession(void *data);

string DOWNLOAD_PATH = "C:\\Users\\Francis\\Documents\\COMP445\\Assignment1\\Server\\Server\\Download\\";
//string DOWNLOAD_PATH = "";

int main(void){

	WSADATA wsadata;

	try{
		if (WSAStartup(0x0202, &wsadata) != 0){
			cout << "Error in starting WSAStartup()\n";
		}
		else{
			buffer = "WSAStartup was suuccessful\n";
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);

			/* display the wsadata structure
			cout<< endl
			<< "wsadata.wVersion "       << wsadata.wVersion       << endl
			<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
			<< "wsadata.szDescription "  << wsadata.szDescription  << endl
			<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
			<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
			<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;
			*/
		}

		// Display info of local host
		gethostname(localhost, 20); // Get computer name: change to localhost
		cout << "ftpd_tcp starting at host: " << localhost << endl;

		if ((hp = gethostbyname(localhost)) == NULL) {
			cout << "gethostbyname() cannot get local host info?"
				<< WSAGetLastError() << endl;
			exit(1);
		}

		// Create the server socket
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			throw "can't initialize socket";
		// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 

		// Fill-in Server Port and Address info.
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);


		//Bind the server port
		if (bind(s, (LPSOCKADDR)&sa, sizeof(sa)) == SOCKET_ERROR)
			throw "can't bind the socket";
		//else cout << "Bind was successful" << endl;

		//Successfull bind, now listen for client requests.
		if (listen(s, 10) == SOCKET_ERROR)
			throw "couldn't  set up listen on socket";
		//else cout << "Listen was successful" << endl;

		FD_ZERO(&readfds);

		//wait loop
		SOCKET cl_socket;
		while (true) {
			//Found a connection request, try to accept.  /***** MULTI THREAD HERE *****/
			cl_socket = accept(s, &ca.generic, &calen);

			FD_SET(s, &readfds);  //always check the listener

			if (!(outfds = select(infds, &readfds, NULL, NULL, tp))) {
				cout << "waiting to be contacted for transferring files...\n" << endl;
			}
			else if (outfds == SOCKET_ERROR) {
				throw "failure in Select";
			}
			else if (FD_ISSET(s, &readfds)) {
				cout << "got a connection request" << endl;
			}

			//Connection request accepted.
			cout << "accepted connection from " << inet_ntoa(ca.ca_in.sin_addr) << ":"
				<< hex << htons(ca.ca_in.sin_port) << "\n" << endl;


			unsigned threadID;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ClientSession, (void*)cl_socket, 0, &threadID);
			

		} //wait loop
		closesocket(cl_socket);

	} //try loop

	//Display needed error message.

	catch (char* str) { cerr << str << WSAGetLastError() << endl; }

	//close server socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();
	return 0;
}

unsigned __stdcall ClientSession(void *data)
{
	SOCKET cl_socket = (SOCKET)data;

	// KEEP ON LOOPING
	while (true) {
		/* Clean buffer */
		memset(&clientbuffer[0], 0, sizeof(clientbuffer));

		// Waiting message
		cout << "\nWaiting for commands...\n" << endl;
		
		if ((ibytesrecv = recv(cl_socket, clientbuffer, 128, 0)) == SOCKET_ERROR) {
			cout << "Lost connection. " << endl;
			throw "Receive error in server program\n";
		}
		else {

			string clientname, directiontype, namefile;

			// Clientbuffer as it is accepted: clienthost?namefile?directiontype
			// Seperate client buffer using '?' as delimeter
			int qPos = 1;
			for (int i = 0; i < strlen(clientbuffer); i++) {
				int j = 0;
				if (clientbuffer[i] != '?') {
					switch (qPos) {
					case 1: clientname += clientbuffer[i]; break;
					case 2: namefile += clientbuffer[i]; break;
					case 3: directiontype += clientbuffer[i]; break;
					}
				}
				else {
					qPos++;
				}
			}

			/* GET FROM SERVER */
			if (directiontype.compare("get") == 0) {
				cout << "User \"" << clientname << "\" requested file " << namefile << " to be sent." << endl;

				// SEARCH THEN SEND FILE HERE
				FILE *fp;
				//string _filepath = DOWNLOAD_PATH + namefile;
				string _filepath = string(namefile);
				if (fp = fopen(_filepath.c_str(), "rb")) {
					cout << "FILE EXISTS" << endl;
					cout << "Sending " << namefile << " to " << clientname << ", waiting..." << endl;

					// Get file length
					fseek(fp, 0, SEEK_END);
					long fileSize = ftell(fp);
					rewind(fp);

					/* Send response + file size to client to prepare */
					string _response = "FILE EXISTS" + to_string(fileSize);
					cout << _response << endl;
					send(cl_socket, _response.c_str(), 128, 0);
					printf("length %d \n\n", fileSize);

					// Loop
					while (true) {
						// Create buffer to store data in chunks if it is big
						char* mfcc = new char[256];

						// Allocate buffer with data from file by size
						int reader = fread(mfcc, 1, 256, fp);
						cout << " LENGTH: " << strlen(mfcc) << " READER: " << reader << endl;

						// Send buffer to client
						send(cl_socket, mfcc, 256, 0);

						if (feof(fp)) {
							// Inform user that end of file is reached.
							send(cl_socket, "END OF FILE", 256, 0);
							cout << "\nEnd-of-File reached." << endl;
							fclose(fp);
							break;
						}
					}
				}
				else {
					// File doesn't exists
					cout << "File doesn't exist" << endl;
					send(cl_socket, "FILE DOESN'T EXIST", 128, 0);
				}
			}
			/* PUT IN SERVER */
			else if (directiontype.compare("put") == 0) {
				cout << "User \"" << clientname << "\" requested file " << namefile << " to be uploaded." << endl;
				cout << "Retrieving file from " << clientname << ", waiting...\n" << endl;

				char _response[128];
				/* Get server response */
				recv(cl_socket, _response, 128, 0);
				cout << "Response: " << _response << endl;
				string _r2 = _response, _fe = "FILE EXISTS";

				if (_r2.find(_fe) != string::npos) {
					// Retrieve file length
					long int _fileSize = atol(_r2.substr(strlen("FILE EXISTS")).c_str());
					cout << "File size: " << _fileSize << endl;

					char *_filebuffer;
					int _getbuffersize = 256;

					// Create file in binary mode
					//string _filepath = DOWNLOAD_PATH + string(namefile);
					string _filepath = string(namefile);
					FILE *fp = fopen(_filepath.c_str(), "wb+");

					while (true) {
						// Buffer container
						_filebuffer = new char[256];

						// Receive buffer from server
						int reader = recv(cl_socket, _filebuffer, 256, 0);

						// Check if end is reached
						if (strcmp(_filebuffer, "END OF FILE") == 0) {
							cout << "File received." << endl;
							fclose(fp);
							break;
						}

						// Write buffer into current file
						fwrite(_filebuffer, 1, _getbuffersize, fp);

						// Decrement current size
						_fileSize -= reader;
						_getbuffersize = (_fileSize < 256) ? _fileSize : 256;
					}
				}
			}
			/* RETRIEVE LIST FROM SERVER */
			else if (directiontype.compare("list") == 0 || namefile.compare("list") == 0) {
				cout << "User \"" << clientname << "\" requested to display all the files." << endl;

				WIN32_FIND_DATA searchFile;
				memset(&searchFile, 0, sizeof(WIN32_FIND_DATA));

				// Change to school directory or add Downloads folder
				LPCWSTR _filepath = L"G:\\My Documents\\COMP445\\A1DEMO\\Server\\Server\\*";

				// Laptop directory
				//LPCWSTR _filepath = L"C:\\Users\\Francis\\Documents\\COMP445\\Assignment1\\Server\\Server\\*";

				HANDLE handle = FindFirstFile(_filepath, &searchFile);

				while (handle != INVALID_HANDLE_VALUE) {
					// Look for end of string name
					int bufferLength = 0;
					for (int fileLen = 0; fileLen < 260; fileLen++)
					{
						if (searchFile.cFileName[fileLen] == '\0') {
							bufferLength = fileLen + 1;
							break;
						}
					}

					_bstr_t b(searchFile.cFileName);
					const char* c = b;
					if ((ibytessent = send(cl_socket, c, bufferLength, 0)) == SOCKET_ERROR)
						throw "error in send in server program\n";

					cout << c << endl;

					if (!FindNextFile(handle, &searchFile))
						break;
				}

				send(cl_socket, "END OF LIST", 128, 0);
			}
			/* DELETE FILE FROM SERVER */
			else if (directiontype.compare("delete") == 0) {
				//string _filepath = DOWNLOAD_PATH + namefile;
				string _filepath = string(namefile);
				if (remove(_filepath.c_str()) == 0) {
					send(cl_socket, "DELETED", 128, 0);
					cout << namefile << " has been deleted..." << endl;
				}
				else {
					send(cl_socket, "ERROR", 128, 0);
					cout << "Error: " << namefile << " probably does not exist." << endl;
				}
			}
			/* CLOSE CLIENT SOCKET */
			else if (directiontype.compare("quit") == 0) {
				cout << "Client \"" << clientname << "\" has disconnected..." << endl;
				closesocket(cl_socket);
				break;
			}
		}
	}
}