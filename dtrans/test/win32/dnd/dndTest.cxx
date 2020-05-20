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

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <comdef.h>
#include <atlbase.h>
CComModule _Module;
#include <atlcom.h>
#include <atlimpl.cpp>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/process.h>
#include <cppuhelper/servicefactory.hxx>
#include "sourcelistener.hxx"

#include "atlwindow.hxx"
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;

HRESULT doTest();
DWORD WINAPI MTAFunc( void* threadData);

Reference< XMultiServiceFactory > MultiServiceFactory;

int main( int argc, char *argv[ ], char *envp[ ] )
{
    HRESULT hr;
    if( FAILED( hr=CoInitialize(NULL )))
    {
        printf("CoInitialize failed \n");
        return -1;
    }

    _Module.Init( ObjectMap, GetModuleHandleA( NULL));

    if( FAILED(hr=doTest()))
    {
        _com_error err( hr);
    }

    _Module.Term();
    CoUninitialize();
    return 0;
}

HRESULT doTest()
{

    MultiServiceFactory= createRegistryServiceFactory( OUString(L"types.rdb"), OUString( L"services.rdb") , sal_True);

    // create the MTA thread that is used to realize MTA calls to the services
    // We create the thread and wait until the thread has created its message queue
    HANDLE evt= CreateEventA(NULL, FALSE, FALSE, NULL);
    DWORD threadIdMTA=0;
    HANDLE hMTAThread= CreateThread( NULL, 0, MTAFunc, &evt, 0, &threadIdMTA);
    WaitForSingleObject( evt, INFINITE);
    CloseHandle(evt);

    HRESULT hr= S_OK;
    RECT pos1={0,0,300,200};
    AWindow win("DnD starting in Ole STA", threadIdMTA, pos1);

    RECT pos2={ 0, 205, 300, 405};
    AWindow win2("DnD starting in MTA", threadIdMTA, pos2, true);

    // win3 and win4 call initialize from an MTA but they are created in an STA
    RECT pos3={300,0,600,200};
    AWindow win3("DnD starting in OLE STA", threadIdMTA, pos3, false, true);

    RECT pos4={ 300, 205, 600, 405};
    AWindow win24("DnD starting in Ole MTA", threadIdMTA, pos4, true, true);

    MSG msg;
    while( GetMessageA(&msg, (HWND)NULL, 0, 0) )
    {
        TranslateMessage(  &msg);
        DispatchMessageA( &msg);
    }

    // Shut down the MTA thread
    PostThreadMessageA( threadIdMTA, WM_QUIT, 0, 0);
    WaitForSingleObject(hMTAThread, INFINITE);
    CloseHandle(hMTAThread);

    return S_OK;
}

extern Reference<XMultiServiceFactory> MultiServiceFactory;
DWORD WINAPI MTAFunc( void* threadData)
{
    HRESULT hr= CoInitializeEx( NULL, COINIT_MULTITHREADED);
    ATLASSERT( FAILED(hr) );
    MSG msg;
    // force the creation of a message queue
    PeekMessageA(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    SetEvent( *(HANDLE*)threadData );

    RECT pos={0, 406, 300, 605};
    AWindow win("DnD, full MTA", GetCurrentThreadId(), pos, false, true);

    while( GetMessageA(&msg, (HWND)NULL, 0, 0) )
    {
        switch( msg.message)
        {
        case  WM_SOURCE_INIT:
        {
            InitializationData* pData= (InitializationData*)msg.wParam;
            Any any;
            any <<= (sal_uInt32) pData->hWnd;
            pData->xInit->initialize( Sequence<Any>( &any, 1));

            CoTaskMemFree( pData);
            break;
        }
        case WM_SOURCE_STARTDRAG:
        {
            // wParam contains necessary data
            StartDragData* pData= (StartDragData*)msg.wParam;
            Sequence<DataFlavor> seq= pData->transferable->getTransferDataFlavors();
            // have a look what flavours are supported
            for( int i=0; i<seq.(); i++)
            {
                DataFlavor d= seq[i];
            }
            pData->source->startDrag( DragGestureEvent(),
                                      ACTION_LINK|ACTION_MOVE|ACTION_COPY,
                                      0,
                                      0,
                                      pData->transferable,
                                      Reference<XDragSourceListener>( static_cast<XDragSourceListener*>
                                                                      ( new DragSourceListener())));
            CoTaskMemFree( pData);
            break;
        }

        } // end switch

        TranslateMessage(  &msg);
        DispatchMessageA( &msg);
    }

    CoUninitialize();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
