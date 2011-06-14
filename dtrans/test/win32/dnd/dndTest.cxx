/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <comdef.h>
#include <tchar.h>
#include <atlbase.h>
CComModule _Module;
#include<atlcom.h>
#include<atlimpl.cpp>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include <com/sun/star/lang/XInitialization.hpp>
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

using ::rtl::OUString;

#define WM_CREATE_MTA_WND

HRESULT doTest();
DWORD WINAPI MTAFunc( void* threadData);

Reference< XMultiServiceFactory > MultiServiceFactory;

int main( int argc, char *argv[ ], char *envp[ ] )
{
    HRESULT hr;
    if( FAILED( hr=CoInitialize(NULL )))
    {
        _tprintf(_T("CoInitialize failed \n"));
        return -1;
    }


    _Module.Init( ObjectMap, GetModuleHandle( NULL));

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
    HANDLE evt= CreateEvent(NULL, FALSE, FALSE, NULL);
    DWORD threadIdMTA=0;
    HANDLE hMTAThread= CreateThread( NULL, 0, MTAFunc, &evt, 0, &threadIdMTA);
    WaitForSingleObject( evt, INFINITE);
    CloseHandle(evt);


    HRESULT hr= S_OK;
    RECT pos1={0,0,300,200};
    AWindow win(_T("DnD starting in Ole STA"), threadIdMTA, pos1);

    RECT pos2={ 0, 205, 300, 405};
    AWindow win2( _T("DnD starting in MTA"), threadIdMTA, pos2, true);

    // win3 and win4 call initialize from an MTA but they are created in an STA
    RECT pos3={300,0,600,200};
    AWindow win3(_T("DnD starting in OLE STA"), threadIdMTA, pos3, false, true);

    RECT pos4={ 300, 205, 600, 405};
    AWindow win24( _T("DnD starting in Ole MTA"), threadIdMTA, pos4, true, true);


    MSG msg;
    while( GetMessage(&msg, (HWND)NULL, 0, 0) )
    {
        TranslateMessage(  &msg);
        DispatchMessage( &msg);
    }

    // Shut down the MTA thread
    PostThreadMessage( threadIdMTA, WM_QUIT, 0, 0);
    WaitForSingleObject(hMTAThread, INFINITE);
    CloseHandle(hMTAThread);

    return S_OK;
}

extern Reference<XMultiServiceFactory> MultiServiceFactory;
DWORD WINAPI MTAFunc( void* threadData)
{
    HRESULT hr= S_OK;
    hr= CoInitializeEx( NULL, COINIT_MULTITHREADED);
    ATLASSERT( FAILED(hr) );
    MSG msg;
    // force the creation of a message queue
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    SetEvent( *(HANDLE*)threadData );

    RECT pos={0, 406, 300, 605};
    AWindow win(_T("DnD, full MTA"), GetCurrentThreadId(), pos, false, true);

    while( GetMessage(&msg, (HWND)NULL, 0, 0) )
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
            for( int i=0; i<seq.getLength(); i++)
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
        DispatchMessage( &msg);
    }


    CoUninitialize();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
