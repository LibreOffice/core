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


HINSTANCE           hInst;                      
WCHAR               szTitle[MAX_LOADSTRING];            
WCHAR               szWindowClass[MAX_LOADSTRING];  
ATOM                MyRegisterClass( HINSTANCE hInstance );
BOOL                InitInstance( HINSTANCE, int );
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK    About( HWND, UINT, WPARAM, LPARAM );
void                PasteClipboardData(HWND hwndParent);
void                PasteClipboardData2(HWND hwndParent);

LPSTREAM            g_pStm    = NULL;
char*               pTextBuff = NULL;
DWORD               lData     = 0;





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

    

    OleUninitialize( );

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

    
    
    
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    

    
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTWIN32, szWindowClass, MAX_LOADSTRING);
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

    
    
    CoUninitialize( );

    return msg.wParam;
}



//

//

//






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

//

//



//
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
   HWND hWnd;

   hInst = hInstance; 

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

//

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
                case IDD_PASTE:
                    
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
            __uuidof(IDataObject), 
            pIDataObj,       
            &g_pStm          
            );

        HANDLE hThread = (HANDLE)_beginthreadex(
                NULL,       
                0,          
                ThreadProc, 
                NULL,       
                (unsigned int)hwndParent,   
                &dwId       
                );

        
        
        
        for(;;)
        {
            DWORD dwRet = ::MsgWaitForMultipleObjects(
                                    1,          
                                    &hThread,   
                                    FALSE,      
                                    INFINITE,   
                                    QS_ALLINPUT 
                                    );

            
            if ( dwRet != WAIT_OBJECT_0 + 1 )
                break;

            
            MSG msg;

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
            {
                
                TranslateMessage(&msg);
                
                DispatchMessage(&msg);
            }
        }

        CloseHandle( hThread );
        pIDataObj->Release();
    }
}
*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
