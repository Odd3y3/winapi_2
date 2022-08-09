// winapi_2.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "winapi_2.h"
#include <list>

using namespace std;

#define MAX_LOADSTRING 100

typedef struct _tagRectangle
{
    float l, t, r, b;
}RECTANGLE, *PRECTANGLE;



// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND g_hWnd;
HDC g_hDC;
bool g_bLoop = true;
RECTANGLE g_tPlayerRC = { 100, 100, 200, 200 };
RECTANGLE g_tEnemyRC = { 600, 0, 700, 100 };
int Enemy_direction = -1;

typedef struct _tagBullet
{
    RECTANGLE rc;
    float fDist;
    float fLimitDist;
}BULLET, *PBULLET;

// 플레이어,enemy 총알
list<BULLET> g_PlayerBulletList;
list<BULLET> g_EnemyBulletList;
float g_EnemyBulletCoolTime = 0.f;

// 시간을 구하기 위한 변수들
LARGE_INTEGER   g_tSecond;
LARGE_INTEGER   g_tTime;
float           g_fDeltaTime;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void Run();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPI2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    //화면용 DC 생성
    g_hDC = GetDC(g_hWnd);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPI2));

    MSG msg;

    QueryPerformanceFrequency(&g_tSecond);

    // 기본 메시지 루프입니다:
    while (g_bLoop)
    {
        //PeekMessage는 메세지가 메세지큐에 없어도 바로 빠져나가고
        //메세지가 있으면 true, 없으면 false
        //메세지가 없는경우가 윈도우의 데드타임
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Run();
        }
    }
    ReleaseDC(g_hWnd, g_hDC);

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPI2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_WINAPI2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;

   //실제 윈도우 타이틀바나 메뉴를 포함한 윈도우의 크기를 구해준다.
   RECT rc = { 0,0,800,600 };
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

   //위에서 구해준 크기로 윈도우 클라이언트 영역의 크기를 원하는 크기로 맞춰줘야함.
   SetWindowPos(hWnd, HWND_TOPMOST, 100, 100, rc.right - rc.left,
       rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
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
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        g_bLoop = false;
        PostQuitMessage(0);
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

void Run() 
{
    // DeltaTime 을 구해준다.
    LARGE_INTEGER tTime;
    QueryPerformanceCounter(&tTime);

    g_fDeltaTime = (tTime.QuadPart - g_tTime.QuadPart) / (float)g_tSecond.QuadPart;
    g_tTime = tTime;

    static float fTimeScale = 1.0f;

    if (GetAsyncKeyState(VK_F1) & 0x8000)
    {
        fTimeScale -= g_fDeltaTime;
        if (fTimeScale < 0.f)
            fTimeScale = 0.f;
    }
    if (GetAsyncKeyState(VK_F2) & 0x8000)
    {
        fTimeScale += g_fDeltaTime;
        if (fTimeScale > 1.f)
            fTimeScale = 1.f;
    }

    // 플레이어 초당 이동속도
    float fSpeed = 600 * g_fDeltaTime * fTimeScale;

    RECT rcWindow;
    GetClientRect(g_hWnd, &rcWindow);

    if (GetAsyncKeyState('D') & 0x8000)
    {
        g_tPlayerRC.l += fSpeed;
        g_tPlayerRC.r += fSpeed;
    }
    if (GetAsyncKeyState('A') & 0x8000)
    {
        g_tPlayerRC.l -= fSpeed;
        g_tPlayerRC.r -= fSpeed;
    }
    if (GetAsyncKeyState('W') & 0x8000)
    {
        g_tPlayerRC.t -= fSpeed;
        g_tPlayerRC.b -= fSpeed;
    }
    if (GetAsyncKeyState('S') & 0x8000)
    {
        g_tPlayerRC.t += fSpeed;
        g_tPlayerRC.b += fSpeed;
    }

    if (g_tPlayerRC.r > rcWindow.right)
    {
        g_tPlayerRC.r = rcWindow.right;
        g_tPlayerRC.l = rcWindow.right - 100;
    }
    else if (g_tPlayerRC.l < rcWindow.left)
    {
        g_tPlayerRC.r = rcWindow.left + 100;
        g_tPlayerRC.l = rcWindow.left;
    }
    if (g_tPlayerRC.t < rcWindow.top)
    {
        g_tPlayerRC.t = rcWindow.top;
        g_tPlayerRC.b = rcWindow.top + 100;
    }
    else if (g_tPlayerRC.b > rcWindow.bottom)
    {
        g_tPlayerRC.t = rcWindow.bottom - 100;
        g_tPlayerRC.b = rcWindow.bottom;
    }

    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        BULLET tBullet;

        tBullet.rc.l = g_tPlayerRC.r;
        tBullet.rc.r = g_tPlayerRC.r + 50.f;
        tBullet.rc.t = (g_tPlayerRC.t + g_tPlayerRC.b) / 2.f - 25.f;
        tBullet.rc.b = tBullet.rc.t + 50.f;
        tBullet.fDist = 0.f;
        tBullet.fLimitDist = 500.f;

        g_PlayerBulletList.push_back(tBullet);

    }

    // 플레이어 총알 이동
    list<BULLET>::iterator iter;
    list<BULLET>::iterator iterEnd = g_PlayerBulletList.end();

    fSpeed = 1000.f * g_fDeltaTime * fTimeScale;
    for (iter = g_PlayerBulletList.begin(); iter != iterEnd;)
    {
        (*iter).rc.l += fSpeed;
        (*iter).rc.r += fSpeed;
        (*iter).fDist += fSpeed;
        if ((*iter).fDist >= (*iter).fLimitDist)
        {
            iter = g_PlayerBulletList.erase(iter);
            iterEnd = g_PlayerBulletList.end();
        }
        else if ((*iter).rc.l > rcWindow.right)
        {
            iter = g_PlayerBulletList.erase(iter);
            iterEnd = g_PlayerBulletList.end();
        }
        else
            iter++;

	}

    // Enemy
    fSpeed = 600.0f * g_fDeltaTime * fTimeScale;
    g_tEnemyRC.t += Enemy_direction * fSpeed;
    g_tEnemyRC.b += Enemy_direction * fSpeed;
    if (g_tEnemyRC.b > 600) {
        g_tEnemyRC.t = 500;
        g_tEnemyRC.b = 600;
        Enemy_direction = -1;
    }
    else if (g_tEnemyRC.t < 0) {
        g_tEnemyRC.t = 0;
        g_tEnemyRC.b = 100;
        Enemy_direction = 1;
    }
    // Enemy 총알 생성
    fSpeed = 800 * g_fDeltaTime * fTimeScale;

    g_EnemyBulletCoolTime += 10.f * g_fDeltaTime * fTimeScale;

    if (g_EnemyBulletCoolTime > 1.f)
    {
        BULLET e_bullet;
        e_bullet.rc.l = g_tEnemyRC.l - 50;
        e_bullet.rc.r = g_tEnemyRC.l;
        e_bullet.rc.t = (g_tEnemyRC.t + g_tEnemyRC.b) / 2 - 25;
        e_bullet.rc.b = e_bullet.rc.t + 50;
        e_bullet.fDist = 0.f;
        e_bullet.fLimitDist = 600.f;

        g_EnemyBulletList.push_back(e_bullet);

        g_EnemyBulletCoolTime = 0.f;
    }
    // Enemy 총알 이동
    list<BULLET>::iterator e_iter;
    list<BULLET>::iterator e_iterEnd = g_EnemyBulletList.end();
    for (e_iter = g_EnemyBulletList.begin(); e_iter != e_iterEnd;)
    {
        (*e_iter).rc.l -= fSpeed;
        (*e_iter).rc.r -= fSpeed;
        (*e_iter).fDist += fSpeed;
        if ((*e_iter).fDist >= (*e_iter).fLimitDist)
        {
            e_iter = g_EnemyBulletList.erase(e_iter);
            e_iterEnd = g_EnemyBulletList.end();
        }
        else if ((*e_iter).rc.r < rcWindow.left)
        {
            e_iter = g_EnemyBulletList.erase(e_iter);
            e_iterEnd = g_EnemyBulletList.end();
        }
        else
            e_iter++;
    }

    // 출력
    Rectangle(g_hDC, 0, 0, rcWindow.right, rcWindow.bottom);
    
    Rectangle(g_hDC, g_tPlayerRC.l, g_tPlayerRC.t, g_tPlayerRC.r, g_tPlayerRC.b);

    Rectangle(g_hDC, g_tEnemyRC.l, g_tEnemyRC.t, g_tEnemyRC.r, g_tEnemyRC.b);

    for (iter = g_PlayerBulletList.begin(); iter != iterEnd; iter++)
    {
        Rectangle(g_hDC, (*iter).rc.l, (*iter).rc.t, (*iter).rc.r, (*iter).rc.b);
    }
    for (e_iter = g_EnemyBulletList.begin(); e_iter != e_iterEnd; e_iter++)
    {
        Rectangle(g_hDC, (*e_iter).rc.l, (*e_iter).rc.t, (*e_iter).rc.r, (*e_iter).rc.b);
    }
}