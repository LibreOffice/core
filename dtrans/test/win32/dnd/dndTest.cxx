/*************************************************************************
 *
 *  $RCSfile: dndTest.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jl $ $Date: 2001-07-19 11:14:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <windows.h>
#include <comdef.h>
#include <tchar.h>
#include <atlbase.h>
CComModule _Module;
#include<atlcom.h>
#include<atlimpl.cpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/process.h>
#include <cppuhelper/servicefactory.hxx>
//#include "transferable.hxx"
#include "sourcelistener.hxx"


#include "atlwindow.hxx"
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace rtl;

// defined in atlwindow.hxx
// #define WM_SOURCE_INIT WM_APP+100
// #define WM_SOURCE_STARTDRAG WM_APP+101
#define WM_CREATE_MTA_WND

HRESULT doTest();
DWORD WINAPI MTAFunc( void* threadData);

Reference< XMultiServiceFactory > MultiServiceFactory;
//int APIENTRY WinMain(HINSTANCE hInstance,
//                     HINSTANCE hPrevInstance,
//                     LPSTR     lpCmdLine,
//                     int       nCmdShow)
//int _tmain( int argc, TCHAR *argv[ ], TCHAR *envp[ ] )
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
        const TCHAR * errMsg= err.ErrorMessage();
//      MessageBox( NULL, errMsg, "Test failed", MB_ICONERROR);
    }


    _Module.Term();
    CoUninitialize();
    return 0;
}

HRESULT doTest()
{

    MultiServiceFactory= createRegistryServiceFactory( OUString(L"applicat.rdb"));

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
//  ThreadData data= *( ThreadData*)pParams;
//  SetEvent(data.evtThreadReady);
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
