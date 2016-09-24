/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

// TestWin32.cpp : Defines the entry point for the application

#define _WIN32_DCOM

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

#include "resource.h"

#define MAX_LOADSTRING 100

// Global variables:
HINSTANCE           hInst;                      // current instance
WCHAR               szTitle[MAX_LOADSTRING];            // Text for title
WCHAR               szWindowClass[MAX_LOADSTRING];  // Text for title
ATOM                MyRegisterClass( HINSTANCE hInstance );
BOOL                InitInstance( HINSTANCE, int );
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK    About( HWND, UINT, WPARAM, LPARAM );
void                PasteClipboardData(HWND hwndParent);
void                PasteClipboardData2(HWND hwndParent);

LPSTREAM            g_pStm    = NULL;
char*               pTextBuff = NULL;
DWORD               lData     = 0;

// a thread function

unsigned int _stdcall ThreadProc(LPVOID pParam)
{
    IDataObject* pIDataObj = NULL;
    FORMATETC    formatETC;
    STGMEDIUM    stgMedium;
    LPVOID       pGlobMem;
    HWND         hwnd;
    DWORD        sizeGlobBuff;
    HRESULT      hr;

    hwnd = (HWND)pParam;

    OleInitialize( NULL );

    hr = OleGetClipboard( &pIDataObj );

    hr = CoGetInterfaceAndReleaseStream(
        g_pStm,
        __uuidof(IDataObject),
        reinterpret_cast<LPVOID*>(&pIDataObj));

    formatETC.cfFormat = CF_TEXT;
    formatETC.ptd      = NULL;
    formatETC.dwAspect = DVASPECT_CONTENT;
    formatETC.lindex   = -1;
    formatETC.tymed    = TYMED_HGLOBAL;

    hr = pIDataObj->GetData( &formatETC, &stgMedium );
    pGlobMem = GlobalLock( stgMedium.hGlobal );
    if ( NULL != pGlobMem )
    {
        if ( NULL != pTextBuff )
        {
            free( pTextBuff );
        }

        sizeGlobBuff = GlobalSize( stgMedium.hGlobal );
        pTextBuff = (char*)malloc( sizeGlobBuff + 1 );
        ZeroMemory( pTextBuff, sizeGlobBuff + 1 );

        memcpy( pTextBuff, pGlobMem, sizeGlobBuff );
        lData = sizeGlobBuff;

        InvalidateRect( hwnd, NULL, TRUE );
        UpdateWindow( hwnd );
    }

    GlobalUnlock( stgMedium.hGlobal );

    ReleaseStgMedium( &stgMedium );

    pIDataObj->Release();

    //CoUninitialize( );

    OleUninitialize( );

    return 0;
}

// WinMain

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
{
    // TODO: Add code here.
    MSG     msg;
    HACCEL  hAccelTable;
    HRESULT hr = E_FAIL;

    // it's important to initialize ole
    // in order to use the clipboard
    //hr = OleInitialize( NULL );
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    //hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Initialization of the application to perform:
    if( !InitInstance( hInstance, nCmdShow ) )
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TESTWIN32);

    // Main message loop:
    while( GetMessage(&msg, NULL, 0, 0) )
    {
        if( !TranslateAccelerator (msg.hwnd, hAccelTable, &msg) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    // uninitializing the ole libraries
    //OleUninitialize( );
    CoUninitialize( );

    return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class
//
//  COMMENTS:
//    This function and its usage are only necessary if this code
//    needs to be compatible with Win32 systems prior to 'RegisterClassEx'
//    function, which was added to Windows 95. If it important to call
//    this function to allow the use of small icons in the correct proportions.

ATOM MyRegisterClass( HINSTANCE hInstance )
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_TESTWIN32);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = (LPCWSTR)IDC_TESTWIN32;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//        In this function, the instance access number is stored in a global variable
//        and the main program window is displayed.
//
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
   HWND hWnd;

   hInst = hInstance; // Store instance access number in our global variable

   hWnd = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if( !hWnd )
   {
      return FALSE;
   }

   ShowWindow( hWnd, nCmdShow );
   UpdateWindow( hWnd );

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - Handle application menu
//  WM_PAINT    - Display main windows
//  WM_DESTROY  - Output completion message and return
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
            // Analyze menu selections
            switch( wmId )
            {
                case IDD_PASTE:
                    //PasteClipboardData(hWnd);
                    PasteClipboardData2(hWnd);
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
            // TODO: Add any code for drawing
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
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}

void PasteClipboardData2(HWND hwndParent)
{
    IDataObject* pIDataObject;
    HRESULT      hr;
    FORMATETC    formatETC;
    STGMEDIUM    stgMedium;
    LPVOID       pGlobMem;
    HWND         hwnd;
    DWORD        sizeGlobBuff;

    hr = MTAGetClipboard( &pIDataObject );
    if ( SUCCEEDED( hr ) )
    {
        formatETC.cfFormat = CF_TEXT;
        formatETC.ptd      = NULL;
        formatETC.dwAspect = DVASPECT_CONTENT;
        formatETC.lindex   = -1;
        formatETC.tymed    = TYMED_HGLOBAL;

        hr = pIDataObject->GetData( &formatETC, &stgMedium );
        pGlobMem = GlobalLock( stgMedium.hGlobal );
        if ( NULL != pGlobMem )
        {
            if ( NULL != pTextBuff )
            {
                free( pTextBuff );
            }

            sizeGlobBuff = GlobalSize( stgMedium.hGlobal );
            pTextBuff = (char*)malloc( sizeGlobBuff + 1 );
            ZeroMemory( pTextBuff, sizeGlobBuff + 1 );

            memcpy( pTextBuff, pGlobMem, sizeGlobBuff );
            lData = sizeGlobBuff;

            InvalidateRect( hwndParent, NULL, TRUE );
            UpdateWindow( hwndParent );
        }

        GlobalUnlock( stgMedium.hGlobal );

        ReleaseStgMedium( &stgMedium );

        pIDataObject->Release();
    }
}

// clipboard handling

/*
void PasteClipboardData(HWND hwndParent)
{
    IDataObject* pIDataObj = NULL;
    HRESULT      hr        = E_FAIL;
    unsigned int dwId;

    hr = OleGetClipboard( &pIDataObj );
    if ( SUCCEEDED( hr ) )
    {
        HRESULT hr = CoMarshalInterThreadInterfaceInStream(
            __uuidof(IDataObject), //The IID of interface to be marshaled
            pIDataObj,       //The interface pointer
            &g_pStm          //IStream pointer
            );

        HANDLE hThread = (HANDLE)_beginthreadex(
                NULL,       //Security
                0,          //Stack Size
                ThreadProc, //Start Address
                NULL,       //Parmeter
                (unsigned int)hwndParent,   //Creation Flag
                &dwId       //Thread Id
                );

        //Wait for the thread to finish execution
        //A thread handle is signaled is thread execution
        //is complete
        for(;;)
        {
            DWORD dwRet = ::MsgWaitForMultipleObjects(
                                    1,          //Count of objects
                                    &hThread,   //pointer to the array of objects
                                    FALSE,      //Wait for all objects?
                                    INFINITE,   //Wait How Long?
                                    QS_ALLINPUT //Wait for all messages
                                    );

            //This means that the object is signaled
            if ( dwRet != WAIT_OBJECT_0 + 1 )
                break;

            //Remove the messages from the queue
            MSG msg;

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
            {
                //Not essential
                TranslateMessage(&msg);
                //Let the windowproc handle the message
                DispatchMessage(&msg);
            }
        }

        CloseHandle( hThread );
        pIDataObj->Release();
    }
}
*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
