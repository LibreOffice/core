/*************************************************************************
 *
 *  $RCSfile: dndTest.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jl $ $Date: 2001-02-26 15:29:45 $
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
#include "transferable.hxx"
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

HRESULT doTest();

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

    HRESULT hr= S_OK;
    RECT pos1={0,0,300,200};
    AWindow win(_T("Drag and Drop, OLE STA"), pos1);

    RECT pos2={ 0, 205, 300, 405};
    AWindow win2( _T("Drag and Drop, MTA"), pos2, true);

    MSG msg;
    while( GetMessage(&msg, (HWND)NULL, 0, 0) )
    {
        TranslateMessage(  &msg);
        DispatchMessage( &msg);
    }

    return S_OK;
}

extern Reference<XMultiServiceFactory> MultiServiceFactory;
DWORD WINAPI MTAFunc(LPVOID pParams)
{
    HRESULT hr= S_OK;
    hr= CoInitializeEx( NULL, COINIT_MULTITHREADED);
    ATLASSERT( FAILED(hr) );

    ThreadData data= *( ThreadData*)pParams;
    SetEvent(data.evtThreadReady);

    data.source->startDrag( DragGestureEvent(),
            ACTION_LINK|ACTION_MOVE|ACTION_COPY,
            0,
            0,
            data.transferable,
            Reference<XDragSourceListener>( static_cast<XDragSourceListener*>
                ( new DragSourceListener())));


    CoUninitialize();
    return 0;
}
