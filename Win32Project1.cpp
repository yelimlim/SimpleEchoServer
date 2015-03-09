// Win32Project1.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "Win32Project1.h"
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#pragma comment ( lib, "Ws2_32.lib" )
#include <WinSock2.h>
#include <map>

#define MAX_LOADSTRING 100

#define WM_SOCKET WM_USER + 1

#define BUF_SIZE    4096
#define MAX_CLNT    100
#define PORT        9001
#define SOCKET_LIST std::map<SOCKET, Socket_Info*>


struct Socket_Info
{
    Socket_Info(SOCKET sock)
    {
        mSock = sock;
        mRecvSize = 0;
        mSendSize = 0;
    }

    SOCKET mSock;
    char mRecvBuffer[BUF_SIZE + 1];
    char mSendBuffer[BUF_SIZE + 1];
    int mRecvSize;
    int mSendSize;
};

// ���� ����:
HINSTANCE hInst;								// ���� �ν��Ͻ��Դϴ�.
TCHAR szTitle[MAX_LOADSTRING];					// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR szWindowClass[MAX_LOADSTRING];			// �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void                ErrQuit(char* msg);

BOOL            AddActiveSocket(SOCKET client);
void            RemoveActiveSocket(SOCKET client);
Socket_Info*    GetActiveSocketInfo(SOCKET client);

SOCKET      listenSocket = NULL;
//SOCKET_LIST freeSocketList;
SOCKET_LIST activeSocketList;


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: ���⿡ �ڵ带 �Է��մϴ�.

    int retValue;

    BOOL isServerRun = false;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        //ErrQuit("WSAStartup() error!");
        return FALSE;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
    {
        //ErrQuit("socket() error!");
        return FALSE;
    }


	MSG msg;
	HACCEL hAccelTable;

	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

    HWND hWnd = GetActiveWindow();

    retValue = WSAAsyncSelect(listenSocket, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
    if (retValue == SOCKET_ERROR)
    {
        //ErrQuit("WSAAsyncSelect()");
        return FALSE;
    }

    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retValue = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (retValue == SOCKET_ERROR)
    {
        //ErrQuit("bind() error!");
        return FALSE;
    }

    retValue = listen(listenSocket, SOMAXCONN);
    if (retValue == SOCKET_ERROR)
    {
        //ErrQuit("listen() error!");
        return FALSE;
    }


	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

	// �⺻ �޽��� �����Դϴ�.
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����:  �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND	- ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT	- �� â�� �׸��ϴ�.
//  WM_DESTROY	- ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �޴� ������ ���� �м��մϴ�.
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

    case WM_SOCKET:
    {
        int retValue;

        if (WSAGETSELECTERROR(lParam))
        {
            RemoveActiveSocket(wParam);
            return 0;
        }

        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_ACCEPT:
        {
            SOCKADDR_IN clientAddr;
            int addrLength = sizeof(clientAddr);
            SOCKET client = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLength);

            if (client == INVALID_SOCKET)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    perror("accept() error!");
                    break;
                }
                return 0;
            }

            retValue = WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
            if (retValue == SOCKET_ERROR)
            {
                perror("WSAAsyncSelect() error!");
                closesocket(client);
                return 0;
            }

            AddActiveSocket(client);
        }
            break;

        case FD_READ:
        {
            Socket_Info* pSocketInfo = GetActiveSocketInfo(wParam);

            retValue = recv(pSocketInfo->mSock, pSocketInfo->mRecvBuffer, BUF_SIZE, 0);
            if (retValue == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    perror("recv");
                    RemoveActiveSocket(wParam);
                    return 0;
                }
            }
            pSocketInfo->mRecvSize = retValue;
            pSocketInfo->mRecvBuffer[retValue] = '\0';

            SOCKADDR_IN socketAddr;
            int nAddrLength = sizeof(socketAddr);
            getpeername(pSocketInfo->mSock, (SOCKADDR*)&socketAddr, &nAddrLength);
            //printf()

            memcpy(pSocketInfo->mSendBuffer, pSocketInfo->mRecvBuffer, retValue);
            pSocketInfo->mSendBuffer[retValue] = '\0';
            pSocketInfo->mSendSize = retValue;

            PostMessage(hWnd, WM_SOCKET, wParam, FD_WRITE);
        }
            break;

        case FD_WRITE:
        {
            Socket_Info* pSocketInfo = GetActiveSocketInfo(wParam);

            if (pSocketInfo == NULL)
            {
                return 0;
            }

            if (pSocketInfo->mSendBuffer <= 0)
            {
                return 0;
            }

            retValue = send(pSocketInfo->mSock, pSocketInfo->mSendBuffer, pSocketInfo->mSendSize, 0);
            if (retValue == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    RemoveActiveSocket(wParam);
                    return 0;
                }
            }

            pSocketInfo->mSendSize = 0;
        }
            break;
        case FD_CLOSE:
            RemoveActiveSocket(wParam);
            return 0;
        }
    }
        break;


	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

BOOL AddActiveSocket(SOCKET client)
{
    if (activeSocketList.size() >= 100)
    {
        return FALSE;
    }

    Socket_Info* pSocketInfo = new Socket_Info(client);

    activeSocketList[client] = pSocketInfo;

    return TRUE;
}

void RemoveActiveSocket(SOCKET client)
{
    SOCKADDR_IN socketAddr;
    int nAddrLength = sizeof(socketAddr);
    getpeername(client, (SOCKADDR*)&socketAddr, &nAddrLength);

    printf("Ŭ���̾�Ʈ ����\n");

    auto sockInfo = activeSocketList.find(client);

    if (sockInfo != activeSocketList.end())
    {
        delete sockInfo->second;
        sockInfo->second = nullptr;
        activeSocketList.erase(sockInfo);
    }
    return;
}

Socket_Info* GetActiveSocketInfo(SOCKET client)
{
    auto sockInfo = activeSocketList.find(client);

    if (sockInfo != activeSocketList.end())
    {
        return sockInfo->second;
    }

    return NULL;
}

void ErrQuit(char* msg)
{
    
}
