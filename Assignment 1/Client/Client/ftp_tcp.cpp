// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood
// 1999 June 30
//
// @author Francis Ortega 1295578
// @version 1.0 10-03-2014


char* getmessage(char *);



/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsock.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <windows.h>
#include <fstream>
#include <ctype.h>
#include <locale>

using namespace std;

//user defined port number
#define REQUEST_PORT 0x7070;

int port = REQUEST_PORT;



//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port



//buffer data types
char szbuffer[128];

char *buffer;

int ibufferlen = 0;

int ibytessent;
int ibytesrecv = 0;



//host data types
HOSTENT *hp;
HOSTENT *rp;

char remotehost[11];

//other

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

boolean isFileValid(char namefile[]);
boolean isDirectionValid(char directiontype[]);
boolean isStringEmpty(char stringChars[]);
string removeWhiteSpaces(char paramText[]);
string lowerCaseString(char stringChars[]);

int main(void){

	WSADATA wsadata;

	try {

		if (WSAStartup(0x0202, &wsadata) != 0){
			cout << "Error in starting WSAStartup()" << endl;
		}
		else {
			buffer = "WSAStartup was successful\n";
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);
		}

		/* HANDLE IF HOST NAME IS QUIT */

		//Create the socket
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			throw "Socket failed\n";
		/* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */

		//Ask for name of remote server
		cout << "Type name of ftp server: " << flush;
		cin >> remotehost;

		// If user types quit in any case end session
		string quitCommand = lowerCaseString(remotehost);
		if (quitCommand.compare("quit") == 0) {
			cout << "Quitting..." << endl;
			closesocket(s);
			Sleep(3000);
			exit(0);
		}

		if ((rp = gethostbyname(remotehost)) == NULL)
			throw "remote gethostbyname failed\n";

		//Specify server address for client to connect to server.
		memset(&sa_in, 0, sizeof(sa_in));
		memcpy(&sa_in.sin_addr, rp->h_addr, rp->h_length);
		sa_in.sin_family = rp->h_addrtype;
		sa_in.sin_port = htons(port);

		//Display the host machine internet address
		cout << "Connecting to remote host:";
		cout << inet_ntoa(sa_in.sin_addr) << endl;

		//Connect Client to the server
		if (connect(s, (LPSOCKADDR)&sa_in, sizeof(sa_in)) == SOCKET_ERROR)
			throw "connect failed\n";
		else cout << "Connected to server: \"" << remotehost << "\"" << endl;

		while (true) {
			/* Clean buffer */
			memset(&szbuffer[0], 0, sizeof(szbuffer));

			/* Have an open connection, so, server is

			- waiting for the client request message
			- don't forget to append <carriage return>
			- <line feed> characters after the send buffer to indicate end-of file */

			char localhost[11], namefile[50], directiontype[5];

			// Display info of local host
			gethostname(localhost, 20);
			strcat_s(szbuffer, localhost);
			strcat_s(szbuffer, "?");

			// Ask for file to be put to/get from server
			cout << "\nType name of file to be transferred \n(type 'list' to display files from server): " << flush;
			cin >> namefile;

			// Validate namefile
			while (!isFileValid(namefile) || string(namefile).empty()) {
				cout << "\nFile names should not contain: \\/:*?\"<>| or any blank spaces." << endl;
				cout << "Type name of file to be transferred: " << flush;
				cin >> namefile;
			}
			
			// Concatenate localhost and namefile
			strcat_s(szbuffer, namefile);

			// Call server for lists (ie. namefile == list)
			if (strcmp(namefile, "list") == 0) {
				ibytessent = 0;
				ibufferlen = strlen(szbuffer);
				send(s, szbuffer, ibufferlen, 0); // SEND TO SERVER!!

				char *_listBuffer;
				// DISPLAY FILE LIST HERE
				while (true) {
					_listBuffer = new char[128];
					recv(s, _listBuffer, 128, 0);

					cout << _listBuffer << endl;

					if (strcmp(_listBuffer, "END OF LIST") == 0)
						break;
				}
	
				continue; // Continue to ask user for new files
			}
				
			strcat_s(szbuffer, "?");
	
			// Ask for direction type: get, put, list
			cout << "\nType direction of transfer \n(get, put, list, delete, quit): " << flush;
			cin >> directiontype;
			
			// Validate namefile
			while (!isDirectionValid(directiontype) || string(directiontype).empty()) {
				cout << "Available directions: \n" <<
						"\t get  - get file from server\n" <<
						"\t put  - put file to server\n" << 
						"\t list - list files available for get\n" <<
						"\t delete - delete file from the server\n" <<
						"\t quit - quit session\n" << endl;
				cout << "Type direction of transfer: " << flush;
				cin >> directiontype;
			}
			strcat_s(szbuffer, directiontype);

			ibytessent = 0;
			ibufferlen = strlen(szbuffer);
			ibytessent = send(s, szbuffer, ibufferlen, 0); // SEND TO SERVER!!


			// Inform server first then quit command
			if (strcmp(directiontype, "quit") == 0) {
				cout << "\nQuitting..." << endl;
				closesocket(s);
				Sleep(3000);
				exit(0);
				break;
			}

			/* GET FROM SERVER */
			if (strcmp(directiontype, "get") == 0) {
				char _response[128];
				cout << "\nSent request to " << remotehost << ", waiting...\n" << endl;

				/* Get server response */
				recv(s, _response, 128, 0);
				cout << "Response: " << _response << endl;
				string _r2 = _response, _fe = "FILE EXISTS";

				if (_r2.find(_fe) != string::npos) {
					// Retrieve file length
					long int _fileSize = atol(_r2.substr(strlen("FILE EXISTS")).c_str());
					cout << "File size: " << _fileSize << endl;

					char *_filebuffer;
					int _getbuffersize = 256;

					// Create file in binary mode
					//string _filepath = "C:\\Users\\Francis\\Documents\\COMP445\\Assignment1\\Client\\Client\\Download\\" + string(namefile);
					string _filepath = string(namefile); // Local client folder
					FILE *fp = fopen(_filepath.c_str() , "wb+");

					while (true) {
						// Buffer container
						_filebuffer = new char[256];

						// Receive buffer from server
						int reader = recv(s, _filebuffer, 256, 0);

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
				else {
					// Error
					cout << "Server response: " << _response << endl;
				}
			}
			/* PUT IN SERVER */
			else if (strcmp(directiontype, "put") == 0) {
				// SEARCH THEN SEND FILE HERE
				FILE *fp;
				//string filepath = "C:\\Users\\Francis\\Documents\\COMP445\\Assignment1\\Client\\Client\\Download\\" + string(namefile);
				string _filepath = string(namefile); // Local client folder
				if (fp = fopen(_filepath.c_str(), "rb")) {
					cout << "FILE EXISTS" << endl;
					cout << "Sending " << namefile << " to " << remotehost << ", waiting..." << endl;

					// Get file length
					fseek(fp, 0, SEEK_END);
					long fileSize = ftell(fp);
					rewind(fp);

					/* Send response + file size to client to prepare */
					string _response = "FILE EXISTS" + to_string(fileSize);
					cout << _response << endl;
					send(s, _response.c_str(), 128, 0);
					printf("length %d \n\n", fileSize);

					// Loop
					while (true) {
						// Create buffer to store data in chunks if it is big
						char* mfcc = new char[256];

						// Allocate buffer with data from file by size
						int reader = fread(mfcc, 1, 256, fp);
						cout << " LENGTH: " << strlen(mfcc) << " READER: " << reader << endl;

						// Send buffer to client
						send(s, mfcc, 256, 0);

						if (feof(fp)) {
							// Inform user that end of file is reached.
							send(s, "END OF FILE", 256, 0);
							cout << "\nEnd-of-File reached." << endl;
							fclose(fp);
							break;
						}
					}
				}
				else {
					// File doesn't exists: send message to server so it dow
					cout << "File doesn't exist" << endl;
					send(s, "FILE DOESN'T EXIST", 128, 0);
				}
			}
			/* RETRIEVE LIST FROM SERVER */
			else if (strcmp(directiontype, "list") == 0) {
				char *_listBuffer;
				// DISPLAY FILE LIST HERE
				while (true) {
					_listBuffer = new char[128];
					recv(s, _listBuffer, 128, 0);

					cout << _listBuffer << endl;

					if (strcmp(_listBuffer, "END OF LIST") == 0)
						break;
				}
			}
			/* DELETE FILE FROM SERVER */
			else if (strcmp(directiontype, "delete") == 0) {
				char *_responseBuffer = new char[128];
				recv(s, _responseBuffer, 128, 0);

				if (strcmp(_responseBuffer, "DELETED") == 0) {
					cout << "File successfully deleted." << endl;
				}
				else if (strcmp(_responseBuffer, "ERROR") == 0) {
					cout << "Error: " << namefile << " probably does not exist." << endl;
				}
			}
			else if (strcmp(directiontype, "quit") == 0) {
				closesocket(s);
				exit(0);
				break;
			}
			continue;
		} // End while
	} 

	//Display any needed error response.

	catch (char *str) { cerr << str << ":" << dec << WSAGetLastError() << endl; }

	//close the client socket


	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();

	return 0;
}

boolean isFileValid(char namefile[]) {
	char keys[] = "\\/:*?\"<>|";

	// False: Not valid, True: Valid
	return (strpbrk(namefile, keys) != NULL) ? false : true;
}

boolean isDirectionValid(char directiontype[]) {
	string strdirection = lowerCaseString(directiontype);

	if (strdirection.compare("get") == 0 ||
		strdirection.compare("put") == 0 ||
		strdirection.compare("list") == 0 ||
		strdirection.compare("quit") == 0 ||
		strdirection.compare("delete") == 0) {
		return true; // Valid
	}
	else {
		return false; // Not valid
	}
}

string lowerCaseString(char stringChars[]) {
	string newString;
	
	for (int i = 0; i < strlen(stringChars); i++) {
		newString += tolower(stringChars[i]);
	}

	return newString;
}