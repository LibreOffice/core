/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



//

#define _WIN32_DCOM
#undef _UNICODE

#include "stdafx.h"

#include <windows.h>

#include <ole2.h>
#include <objidl.h>
#include <objbase.h>
#include <process.h>
#include <olectl.h>
#include <stdlib.h>
#include <malloc.h>
#include <..\..\inc\systools\win32\MtaOleClipb.h>
#include "XTDataObject.hxx"

#include "resource.h"

#define MAX_LOADSTRING 100
#undef USE_MTACB

#define MSG_FLUSHCLIPBOARD WM_USER + 1


HINSTANCE           hInst;                      
TCHAR               szTitle[MAX_LOADSTRING];            
TCHAR               szWindowClass[MAX_LOADSTRING];  
ATOM                MyRegisterClass( HINSTANCE hInstance );
BOOL                InitInstance( HINSTANCE, int );
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK    About( HWND, UINT, WPARAM, LPARAM );
void                CopyClipboardData(HWND hwndParent);
void                FlushClipboard( );
void                PasteData( HWND hWnd );
void                SetLocale();


LPSTREAM            g_pStm    = NULL;
char*               pTextBuff = NULL;
DWORD               lData     = 0;
CXTDataObject*      g_xtDo    = NULL;
HWND                g_hWnd;
HANDLE              g_hEvent;
BOOL                g_bEnd;





unsigned int _stdcall ThreadProc(LPVOID pParam)
{
    while( !g_bEnd )
    {
        WaitForSingleObject( g_hEvent, INFINITE );
        SendMessage( g_hWnd, MSG_FLUSHCLIPBOARD, WPARAM(0), LPARAM(0) );
    }

    return 0;
}





int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
{
    
    MSG     msg;
    HACCEL  hAccelTable;
    HRESULT hr = E_FAIL;

    
    
#ifdef USE_MTACB
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
#else
    hr = OleInitialize( NULL );
#endif


    
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_TESTWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    
    if( !InitInstance( hInstance, nCmdShow ) )
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TESTWIN32);

    
    while( GetMessage(&msg, NULL, 0, 0) )
    {
        if( !TranslateAccelerator (msg.hwnd, hAccelTable, &msg) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    
#ifdef USE_MTACB
    CoUninitialize( );
#else
    OleUninitialize( );
#endif

    CloseHandle( g_hEvent );

    return msg.wParam;
}





//

//







ATOM MyRegisterClass( HINSTANCE hInstance )
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_TESTWIN32);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = (LPCTSTR)IDC_TESTWIN32;
    wcex.lpszClassName  = _T(szWindowClass);
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx(&wcex);
}



//

//





BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
   hInst = hInstance; 

   g_hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if( !g_hWnd )
   {
      return FALSE;
   }

   ShowWindow( g_hWnd, nCmdShow );
   UpdateWindow( g_hWnd );

   return TRUE;
}



//

//





LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int         wmId;
    PAINTSTRUCT ps;
    HDC         hdc;
    TCHAR       szHello[MAX_LOADSTRING];


    LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

    switch( message )
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam);
            
            switch( wmId )
            {
                case IDD_COPY:
                    CopyClipboardData(hWnd);
                    break;
                case IDD_PASTE2:
                    PasteData(hWnd);
                    break;
                case IDD_LOCALE:
                    SetLocale();
                    break;
                case IDM_EXIT:
                   DestroyWindow( hWnd );
                   break;

                default:
                   return DefWindowProc( hWnd, message, wParam, lParam );
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint (hWnd, &ps);
            
            RECT rt;
            GetClientRect( hWnd, &rt );

            if ( NULL != pTextBuff )
            {
                DrawText( hdc, pTextBuff, lData, &rt, DT_CENTER );
            }
            else
            {
                DrawText( hdc, szHello, strlen(szHello), &rt, DT_CENTER );
            }

            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            g_bEnd = TRUE;
            SetEvent( g_hEvent );
            FlushClipboard( );
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}





void CopyClipboardData( HWND hWnd )
{
    g_xtDo = new CXTDataObject( 1 );
#ifdef USE_MTACB
    MTASetClipboard( static_cast< IDataObject* >( g_xtDo ) );
#else
    OleSetClipboard( static_cast< IDataObject* >( g_xtDo ) );
#endif
}





void FlushClipboard( )
{
    if ( NULL != g_xtDo )
    {
#ifdef USE_MTACB
        HRESULT hr = MTAIsCurrentClipboard( static_cast< IDataObject* >( g_xtDo ) );
        if ( S_OK == hr )
            MTAFlushClipboard( );
#else
        HRESULT hr = OleIsCurrentClipboard( static_cast< IDataObject* >( g_xtDo ) );
        if ( S_OK == hr )
            OleFlushClipboard( );
#endif

        static_cast< IDataObject* >( g_xtDo )->Release( );
    }
}


void PasteData(HWND hWnd)
{
    IDataObject* pDataObj;

    

    HRESULT hr = OleGetClipboard( &pDataObj );
    if ( SUCCEEDED( hr ) )
    {
        FORMATETC fetc;
        STGMEDIUM stgmedium;

        fetc.cfFormat = CF_LOCALE;
        fetc.ptd      = NULL;
        fetc.dwAspect = DVASPECT_CONTENT;
        fetc.lindex   = -1;
        fetc.tymed    = TYMED_HGLOBAL;

        hr = pDataObj->GetData( &fetc, &stgmedium );
        if ( SUCCEEDED( hr ) )
        {
            LPVOID lpData = GlobalLock( stgmedium.hGlobal );

            if ( NULL != lpData )
            {
                LCID lcid = *( (WORD*)lpData );

                WORD langID = LANGIDFROMLCID( lcid );
                WORD sublangID = SUBLANGID( langID );

                TCHAR buff[6];
                int cbWritten = GetLocaleInfo( lcid, LOCALE_IDEFAULTANSICODEPAGE, buff, sizeof( buff ) );
                cbWritten     = GetLocaleInfo( lcid, LOCALE_IDEFAULTCODEPAGE, buff, sizeof( buff ) );

                GlobalUnlock( stgmedium.hGlobal );
            }
            else
            {
                DWORD dwLastError = GetLastError( );
            }

            ReleaseStgMedium( &stgmedium );
        }
    }
}


void SetLocale()
{
    LCID threadLcid = GetThreadLocale();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
