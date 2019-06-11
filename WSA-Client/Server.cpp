#include <iostream>  
#include <iomanip>
#include <cstdio>  
#include <string>  
#include <cstring>  
#include <vector>  
#include <iterator>  
#include <algorithm>  
#include <Winsock2.h>  
#pragma comment( lib, "ws2_32.lib" )
#include <Windows.h>  
#pragma warning(disable:4996)

using namespace std;
HANDLE bufferMutex;     // �����ܻ���ɹ�����ͨ�ŵ��ź������  
SOCKET sockConn;        // �ͻ��˵��׽���  
vector <SOCKET> clientSocketGroup;
string broadcast[3] = {"no", "nobody","none" };

int main()
{
	// ����socket��̬���ӿ�(dll)  
	WORD wVersionRequested;
	WSADATA wsaData;    // ��ṹ�����ڽ���Wjndows Socket�Ľṹ��Ϣ��  
	wVersionRequested = MAKEWORD(2, 2);   // ����2.2�汾��WinSock��  
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -1;          // ����ֵΪ���ʱ���Ǳ�ʾ�ɹ�����WSAStartup  
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) { // ����Ƿ�2.2�汾��socket��  
		WSACleanup();
		return -1;
	}

	// ����socket������������ʽ�׽��֣������׽��ֺ�sockSrv  
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);

	// �׽���sockSrv�뱾�ص�ַ����  
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // ��INADDR_ANYת��Ϊ�����ֽ��򣬵��� htonl(long��)��htons(����)  
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000);

	if (SOCKET_ERROR == bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) { // �ڶ�����Ҫǿ������ת��  
		return -1;
	}

	// ���׽�������Ϊ����ģʽ���������󣩣� listen()֪ͨTCP������׼���ý�������  
	listen(sockSrv, 20);

	cout << "###Server is online now,a chat room is established.###\n";
	// accept()���������ӣ��ȴ��ͻ�������  

	bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);

	

	DWORD WINAPI SendMessageThread(LPVOID IpParameter);
	DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter);

	HANDLE sendThread = CreateThread(NULL, 0, SendMessageThread, NULL, 0, NULL);

	while (true) {    // ���ϵȴ��ͻ�������ĵ���  
		sockConn = accept(sockSrv, NULL, NULL);
		if (SOCKET_ERROR != sockConn) {
			clientSocketGroup.push_back(sockConn);
		}
		HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, (LPVOID)sockConn, 0, NULL);
//		HANDLE sendThread = CreateThread(NULL, 0, SendMessageThread, (LPVOID)sockConn, 0, NULL);

		WaitForSingleObject(bufferMutex, INFINITE);     // P����Դδ��ռ�ã�   
		if (NULL == receiveThread) {
			printf("###CreatThread AnswerThread() failed.###\n");
		}
		else {
			printf("###Create A Receive Client Thread.###\n");
		}
		ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�  
	}

	WaitForSingleObject(sendThread, INFINITE);  // �ȴ��߳̽���  
	CloseHandle(sendThread);
	CloseHandle(bufferMutex);
	WSACleanup();   // ��ֹ���׽��ֿ��ʹ��  
	printf("\n");
	system("pause");
	return 0;
}



DWORD WINAPI SendMessageThread(LPVOID IpParameter)
{
	while (1) {
		WaitForSingleObject(bufferMutex, INFINITE); // P����Դδ��ռ�ã�  
		if (!broadcast[0].compare("yes")) {
			string talk = "(" + broadcast[1] + "):" + broadcast[2];
			//		getline(cin, talk);
			//		printf("I Say:(\"quit\"to exit):");
			//		cout << talk;
			for (int i = 0; i < clientSocketGroup.size(); ++i) {
				//      send(clientSocketGroup[i], talk.c_str(), talk.size(), 0);   // ������Ϣ  
				send(clientSocketGroup[i], talk.c_str(), 200, 0);   // ������Ϣ  
			}
//			send((SOCKET)(LPVOID)IpParameter, talk.c_str(), 200, 0);
			ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�   
		}
		ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�
	}
	return 0;
}


DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter)
{
	string name = "One Client";
	SOCKET ClientSocket = (SOCKET)(LPVOID)IpParameter;
	while (1) {
//		broadcast[0] = "no";
		char recvBuf[300];
		recv(ClientSocket, recvBuf, 200, 0);
		WaitForSingleObject(bufferMutex, INFINITE);     // P����Դδ��ռ�ã�    
		string recvS = recvBuf;
//		recvS.erase(0, recvS.find_first_not_of(" "));
//		recvS.erase(recvS.find_last_not_of(" ") + 1);
		recvS = recvS.substr(0,recvS.find("\n"));
		if (recvBuf[0] == 'n' && recvBuf[1] == 'a' && recvBuf[2] == 'm' && recvBuf[3] == 'e' && recvBuf[4] == ':') {
			//del_sp(recvBuf);
			name = recvS.substr(5, recvS.length() - 5);
		}
		if (recvBuf[0] == 'q' && recvBuf[1] == 'u' && recvBuf[2] == 'i' && recvBuf[3] == 't' && recvBuf[4] == '\0') {
			vector<SOCKET>::iterator result = find(clientSocketGroup.begin(), clientSocketGroup.end(), ClientSocket);
			clientSocketGroup.erase(result);
			closesocket(ClientSocket);
			ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�   
			cout << "###Atention: " << name <<setw(10) << " left the room.###" << endl;
			break;
		}
		broadcast[0] = "yes";
		broadcast[1] = name;
		broadcast[2] = recvS;
		cout << "**" << setw(10) << left << setfill('*') << name << "**" << ": " << recvS << endl;
//		printf("%s Says: %s\n",name, string(recvBuf).substr(0,end));     // ������Ϣ  

		ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�   
	}
	return 0;
}

