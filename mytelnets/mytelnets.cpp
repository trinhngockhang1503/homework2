// ChatServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

DWORD WINAPI ClientThread(LPVOID);
void RemoveClient(SOCKET);
SOCKET clients[64];
int numClients;
bool enter = true;
CRITICAL_SECTION CriticalSection;
int main(int argc, char *argv[])
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[1]));

	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);
	numClients = 0;
	if (!InitializeCriticalSectionAndSpinCount(&CriticalSection,0x00000400))
		return 0;
	while (true) {
		SOCKET client = accept(listener, NULL, NULL);
		printf("Co nguoi ket noi: %d \n", client);
		clients[numClients] = client;
		numClients++;
		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}
	DeleteCriticalSection(&CriticalSection);
	WSACleanup();
	return 0;
}

DWORD WINAPI ClientThread(LPVOID lpParam) {
	SOCKET client = *(SOCKET *)lpParam;
	char buf[1024];
	int ret;
	char mess[64] = "Nhap username password \n";
	send(client, mess, strlen(mess), 0);
	bool authorize = false;
	// xac thuc nguoi dung
	while (true)
	{
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
		{
			// Ngat ket noi, xoa client khoi mang
			RemoveClient(client);
			break;
		}

		buf[ret - 1] = 0;
		printf("Received from %d: %s\n", client, buf);
		// lay username pass
		char username[64];
		ret = sscanf(buf, "%s", username);
		char *pass = buf + strlen(username) + 1;

		//doc file
		FILE *f = fopen("UserData.txt", "rb");
		char file[1024];
		int count = 0;
		
		while (true) {
			int read = fread(file, 1, sizeof(file), f);
			if (read > 0) {
				file[read] = 0;
				//printf("file la %s",  file);
				for (int i = 0; i < read + 2 ; i++) {
					char *lineInText;
					if (file[i] == '\n' || i == read + 1) {
						lineInText = file + count;
						lineInText[i - count - 1] = 0;	
						
						if (i == read + 1) {
							lineInText = file + count;
							//lineInText[i - count - 1] = 0;
						}
						count = i + 1;
						int slpit;
						char usernameText[64];
						slpit = sscanf(lineInText, "%s", usernameText);
						if (strcmp(usernameText, username) == 0) {
							printf("dung user r %s \n", username);
							char *passText = lineInText + strlen(usernameText) + 1;
							
							
							if (strcmp(pass, passText) == 0) {
								printf("dung mat khau r");
								authorize = true;
								break;
							}
						}
					}		
				}
			}
			else {
				break;
			}
		}
		fclose(f);
		if (authorize) {
			char au[64] = "DA nhap dung pass \n";
			send(client, au, strlen(au), 0);
			break;
		}
		
		char au[64] = "Moi ban nhap lai \n";
		send(client, au, strlen(au), 0);	
	}
	char command[1024];
	while (true) {
		ret = recv(client, command, sizeof(command), 0);
		if (ret <= 0)
		{
			// Ngat ket noi, xoa client khoi mang
			RemoveClient(client);
			break;
		}
		if (strncmp(command, "exit", 4) == 0) break;

		command[ret - 1] = 0;
		char finalCommand[1024];
		EnterCriticalSection(&CriticalSection);
		strcpy_s(finalCommand, command);
		strcat_s(finalCommand, " > log.txt");
		system(finalCommand);
		LeaveCriticalSection(&CriticalSection);
	}

	closesocket(client);
	return 0;
}
void RemoveClient(SOCKET client) {
	int i = 0;
	for (; i < numClients; i++)
		if (clients[i] == client)
			break;
	//xoa phan tu
	if (i < numClients - 1)
		clients[i] = clients[numClients - 1];
	numClients--;
}
