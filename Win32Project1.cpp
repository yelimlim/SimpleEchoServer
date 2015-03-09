// Win32Project1.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
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

// 전역 변수:
HINSTANCE hInst;								// 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];					// 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];			// 기본 창 클래스 이름입니다.

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
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

 	// TODO: 여기에 코드를 입력합니다.

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

	// 전역 문자열을 초기화합니다.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 응용 프로그램 초기화를 수행합니다.
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

	// 기본 메시지 루프입니다.
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
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
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
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

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
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND	- 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT	- 주 창을 그립니다.
//  WM_DESTROY	- 종료 메시지를 게시하고 반환합니다.
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
		// 메뉴 선택을 구문 분석합니다.
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
		// TODO: 여기에 그리기 코드를 추가합니다.
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

// 정보 대화 상자의 메시지 처리기입니다.
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

    printf("클라이언트 종료\n");

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
