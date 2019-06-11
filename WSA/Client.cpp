#include <iostream>  
#include <cstdio>  
#include <string>  
#include <cstring>  
#include <winsock2.h>  
#pragma comment( lib, "ws2_32.lib" )
#include <Windows.h>  
#pragma warning(disable:4996)
using namespace std;

SOCKET sockClient;      // ���ӳɹ�����׽���  
HANDLE bufferMutex;     // �����ܻ���ɹ�����ͨ�ŵ��ź������  

int main()
{
	// ����socket��̬���ӿ�(dll)  
	WORD wVersionRequested;
	WSADATA wsaData;    // ��ṹ�����ڽ���Wjndows Socket�Ľṹ��Ϣ��  
	wVersionRequested = MAKEWORD(2, 2);   // ����2.2�汾��WinSock��  
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {   // ����ֵΪ���ʱ���Ǳ�ʾ�ɹ�����WSAStartup  
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) { // ���汾���Ƿ���ȷ  
		WSACleanup();
		return -1;
	}

	// ����socket������������ʽ�׽��֣������׽��ֺ�sockClient  
	sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (sockClient == INVALID_SOCKET) {
		printf("Error at socket():%ld\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// ���׽���sockClient��Զ����������  
	// int connect( SOCKET s,  const struct sockaddr* name,  int namelen);  
	// ��һ����������Ҫ�������Ӳ������׽���  
	// �ڶ����������趨����Ҫ���ӵĵ�ַ��Ϣ  
	// ��������������ַ�ĳ���  
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");      // ���ػ�·��ַ��127.0.0.1;   
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000);
	connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	cout << "###You are in the room now.###" << endl;
	cout << "###First set you name by entering (name:xxx)###" << endl;
	cout << "###Enter (quit) to leave.###" << endl;

	//  send(sockClient, "\nAttention: A Client has enter...\n", strlen("Attention: A Client has enter...\n")+1, 0);  
//	send(sockClient, "\nAttention: A Client has enter...\n", 200, 0);

	bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);

	DWORD WINAPI SendMessageThread(LPVOID IpParameter);
	DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter);

	HANDLE sendThread = CreateThread(NULL, 0, SendMessageThread, NULL, 0, NULL);
	HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, NULL, 0, NULL);


	WaitForSingleObject(sendThread, INFINITE);  // �ȴ��߳̽���  
	closesocket(sockClient);
	CloseHandle(sendThread);
	CloseHandle(receiveThread);
	CloseHandle(bufferMutex);
	WSACleanup();   // ��ֹ���׽��ֿ��ʹ��  

	printf("###End linking...###\n");
	printf("\n");
	system("pause");
	return 0;
}


DWORD WINAPI SendMessageThread(LPVOID IpParameter)
{
	while (1) {
		string talk;
//		cout << ": ";
		getline(cin, talk);
		WaitForSingleObject(bufferMutex, INFINITE);     // P����Դδ��ռ�ã�    
		if ("quit" == talk) {
			talk.push_back('\0');
			//          send(sockClient, talk.c_str(), talk.size(), 0);  
			send(sockClient, talk.c_str(), 200, 0);

			break;
		}
		else {
			talk.append("\n");
		}
//		printf("\n**I**: ");
//		cout << talk;
		//  send(sockClient, talk.c_str(), talk.size(), 0); // ������Ϣ  
		send(sockClient, talk.c_str(), 200, 0); // ������Ϣ  
		ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�   
	}
	return 0;
}


DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter)
{
	while (1) {
		char recvBuf[300];
		recv(sockClient, recvBuf, 200, 0);
		WaitForSingleObject(bufferMutex, INFINITE);     // P����Դδ��ռ�ã�    
		cout << recvBuf << endl;
//		printf("%s Says: %s\n", "Server", recvBuf);     // ������Ϣ  

		ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�   
	}
	return 0;
}