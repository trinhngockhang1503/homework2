// ChatServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

DWORD WINAPI ClientThread(LPVOID);
void RemoveClient(SOCKET);
SOCKET clients[64];
int numClients;
int main(int argc,char *argv[])
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

	while (true) {
		SOCKET client = accept(listener, NULL, NULL);
		printf("Co nguoi ket noi: %d \n", client);
		clients[numClients] = client;
		numClients++;
		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}
	WSACleanup();
    return 0;
}

DWORD WINAPI ClientThread(LPVOID lpParam) {
	SOCKET client = *(SOCKET *)lpParam;
	char *nameClient;
	char buf[1024];
	int ret;
	// kiem tra dang ky ten
	while (true) {
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0) {
			RemoveClient(client);
			break;
		}
		buf[ret-1] = 0;
		printf("nhan dc tu %d : %s \n", client, buf);
		if (strncmp(buf, "client_id:", 10) == 0 ) {
			nameClient = buf + 9;
			printf("ten client la: %s", nameClient);
			break;
		}
		char wrongSyntax[] = "De nghi nhap lai \n";
		send(client, wrongSyntax, strlen(wrongSyntax), 0);
	}
	// gui cho client khac
	while (true) {
		char messBuf[1024];
		ret = recv(client, messBuf, sizeof(messBuf), 0);
		if (ret <= 0) {
			RemoveClient(client);
			break;
		}
		messBuf[ret] = 0;
		char sendBuf[1024];
		strcpy_s(sendBuf, nameClient);
		strcat_s(sendBuf, ":");
		strcat_s(sendBuf, messBuf);

		for (int i = 0; i < numClients; i++) {
			if (client != clients[i]) {
				send(clients[i], sendBuf, strlen(sendBuf), 0);
			}
		}
	}
	closesocket(client);
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