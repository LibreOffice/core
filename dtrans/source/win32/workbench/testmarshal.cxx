/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testmarshal.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:08:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


//_________________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <stdio.h>
#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <memory>

#include <process.h>
#include "XTDo.hxx"

//-------------------------------------------------------------
// my defines
//-------------------------------------------------------------

#define WRITE_CB
#define EVT_MANUAL_RESET     TRUE
#define EVT_INIT_NONSIGNALED FALSE
#define EVT_NONAME           ""
#define WAIT_MSGLOOP
#define RAW_MARSHALING

//------------------------------------------------------------
//  namesapces
//------------------------------------------------------------

using namespace ::rtl;
using namespace ::std;

//------------------------------------------------------------
//  globales
//------------------------------------------------------------

HANDLE  g_hEvtThreadWakeup;

#ifdef RAW_MARSHALING
    HGLOBAL g_hGlob;
#else
    IStream* g_pStm;
#endif

//################################################################
// a thread in another apartment to test apartment transparency

unsigned int _stdcall ThreadProc(LPVOID pParam)
{
    // setup another apartment
    HRESULT hr = OleInitialize( NULL );

    WaitForSingleObject( g_hEvtThreadWakeup, INFINITE );

    IDataObject* pIDo;

#ifdef RAW_MARSHALING

    IStream* pStm = NULL;
    hr = CreateStreamOnHGlobal( g_hGlob, FALSE, &pStm );
    if ( SUCCEEDED( hr ) )
    {
        hr = CoUnmarshalInterface(
                pStm,
                __uuidof( IDataObject ),
                (void**)&pIDo );

        hr = pStm->Release( );
    }

#else

    hr = CoGetInterfaceAndReleaseStream(
        g_pStm,
        __uuidof( IDataObject ),
        (void**)&pIDo
        );

#endif

    IEnumFORMATETC* pIEEtc;
    hr = pIDo->EnumFormatEtc( DATADIR_GET, &pIEEtc );

    hr = OleIsCurrentClipboard( pIDo );

    hr = OleFlushClipboard( );

    OleUninitialize( );

    return 0;
}

//################################################################

//----------------------------------------------------------------
//  main
//----------------------------------------------------------------

int SAL_CALL main( int nArgc, char* Argv[] )
{
    HRESULT hr = OleInitialize( NULL );

    g_hEvtThreadWakeup = CreateEvent( 0,
                                      EVT_MANUAL_RESET,
                                      EVT_INIT_NONSIGNALED,
                                      EVT_NONAME );

    unsigned uThreadId;
    HANDLE   hThread;

    // create a thread in another apartment
    hThread = (void*)_beginthreadex( NULL, 0, ThreadProc, NULL, 0, &uThreadId );

    IDataObject* pIDo = new CXTDataObject( );

    hr = OleSetClipboard( pIDo );
    hr = E_FAIL;

    hr = OleIsCurrentClipboard( pIDo );

    //hr = OleGetClipboard( &pIDo );
    if ( SUCCEEDED( hr ) )
    {
#ifdef RAW_MARSHALING

        IStream* pStm = NULL;

        hr = CreateStreamOnHGlobal( 0, FALSE, &pStm );
        if ( SUCCEEDED( hr ) )
        {
            hr = CoMarshalInterface(
                pStm,
                __uuidof( IDataObject ),
                pIDo,
                MSHCTX_INPROC,
                0,
                MSHLFLAGS_NORMAL );
            if ( SUCCEEDED( hr ) )
                hr = GetHGlobalFromStream( pStm, &g_hGlob );

            hr = pStm->Release( );
        }

#else

        hr = CoMarshalInterThreadInterfaceInStream(
                __uuidof( IDataObject ),
                pIDo,
                &g_pStm );

#endif

        if ( SUCCEEDED( hr ) )
        {
            // wakeup the thread and waiting util it ends
            SetEvent( g_hEvtThreadWakeup );

#ifdef WAIT_MSGLOOP

            BOOL bContinue = TRUE;

            while( bContinue )
            {
                DWORD dwResult = WaitForMultipleObjects(
                    1,
                    &hThread,
                    TRUE,
                    0 );

                if ( WAIT_OBJECT_0 == dwResult )
                {
                    bContinue = FALSE;
                }
                else
                {
                    MSG msg;
                    while( PeekMessage(
                            &msg,
                            NULL,
                            0,
                            0,
                            PM_REMOVE ) )
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            } // while

#endif

        } // if
    } // if

    OleFlushClipboard( );

    OleUninitialize( );

    return 0;
}
