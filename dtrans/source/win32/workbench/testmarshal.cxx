/*************************************************************************
 *
 *  $RCSfile: testmarshal.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-07-26 11:20:56 $
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

/*
    Marshalling raw: we use CoMarshalInterface and CoUnmarshalInterface
    directly

    Marshalling non-raw: wee use CoMarshalInterThreadInterfaceInStream and
    CoGetInteraface and ReleaseStream

    We test the following cases:

    Marshalling         Target Thread Apartment
    -------------------------------------------
    Raw                 Sta
    Raw                 Mta
    Non-Raw             Sta
    Non-Raw             Mta
*/

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <stdio.h>
#include <windows.h>
#include <objbase.h>

#include <process.h>
#include <comdef.h>

//-------------------------------------------------------------
// my defines
//-------------------------------------------------------------

const bool EVT_MANUAL_RESET     = true;
const bool EVT_AUTO_RESET       = false;
const bool EVT_INIT_NONSIGNALED = false;
const bool EVT_INIT_SIGNALED    = true;
const char EVT_NONAME[]         = "";
const int  MAX_LOOP             = 1000;

//------------------------------------------------------------
//  namesapces
//------------------------------------------------------------

using namespace ::rtl;
using namespace ::std;

//------------------------------------------------------------
//  globales
//------------------------------------------------------------

HANDLE     g_HandleArray[2];
HANDLE&    g_hEvtMarshalClipDataObj = g_HandleArray[0];
HANDLE&    g_hThread                = g_HandleArray[1];
HANDLE     g_hEvtThreadWakeup;
BOOL       g_bRawMarshalling = false;
HGLOBAL    g_hGlob = NULL;
IStreamPtr g_pStm;


enum APARTMENT_MODEL
{
    STA = 0,
    MTA,
    OLE
};

//------------------------------------------------------------
//
//------------------------------------------------------------

void SetupApartment( APARTMENT_MODEL apm )
{
    // setup another apartment
    HRESULT hr;

    switch ( apm )
    {
        case STA:
            hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
            OSL_ENSURE( SUCCEEDED( hr ), "CoInitialize STA failed" );
            break;

        case MTA:
            hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
            OSL_ENSURE( SUCCEEDED( hr ), "CoInitialize MTA failed" );
            break;

        case OLE:
            hr = OleInitialize( NULL );
            OSL_ENSURE( SUCCEEDED( hr ), "OleInitialize failed" );
            break;

        default:
            OSL_ENSURE( false, "invalid apartment model" );
    }
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void ShutdownApartment( APARTMENT_MODEL apm )
{
    switch ( apm )
    {
    case STA:
    case MTA:
        CoUninitialize( );
        break;
    case OLE:
        OleUninitialize( );
        break;
    }
}

//################################################################
// a thread in another apartment to test apartment transparency

unsigned int _stdcall ThreadProc(LPVOID pParam)
{
    APARTMENT_MODEL* apm = reinterpret_cast< APARTMENT_MODEL* >( pParam );

    SetupApartment( *apm );

    for ( int i = 0; i < MAX_LOOP; i++ )
    {
        WaitForSingleObject( g_hEvtThreadWakeup, INFINITE );

        try
        {
            HRESULT hr;
            IDataObjectPtr pIDo;

            if ( g_bRawMarshalling )
            {
                IStreamPtr pStm = NULL;
                hr = CreateStreamOnHGlobal( g_hGlob, FALSE, &pStm );
                OSL_ENSURE( SUCCEEDED( hr ), "CreateStreamOnHGlobal failed" );

                hr = CoUnmarshalInterface(
                    pStm, __uuidof( IDataObject ), (void**)&pIDo );
                OSL_ENSURE( SUCCEEDED( hr ), "CoUnmarshalInterface failed" );
            }
            else
            {
                IStream* pIStm = g_pStm.Detach( );

                hr = CoGetInterfaceAndReleaseStream(
                    pIStm, __uuidof( IDataObject ), (void**)&pIDo );

                OSL_ENSURE( SUCCEEDED( hr ), "CoGetInterfaceAndReleaseStream failed" );
            }

            // only to call some interface functions
            IEnumFORMATETCPtr pIEnumFetc;
            hr = pIDo->EnumFormatEtc( DATADIR_GET, &pIEnumFetc );
            OSL_ENSURE( SUCCEEDED( hr ), "EnumFormatEtc failed" );

            // release the object before calling
            // CoUninitialize
            pIDo.Detach( );
        }
        catch( _com_error& ex )
        {
            HRESULT hr = ex.Error( );
            OSL_ENSURE( false, "com exception caught" );
        }

        SetEvent( g_HandleArray[0] );
    } // end for

    ShutdownApartment( *apm );

    return 0;
}

//################################################################

//----------------------------------------------------------------
//
//----------------------------------------------------------------

void MarshalClipboardDataObject( IDataObjectPtr pIDo )
{
    HRESULT hr;

    if ( g_bRawMarshalling )
    {
        IStreamPtr pStm = NULL;

        hr = CreateStreamOnHGlobal( 0, false, &pStm );
        OSL_ENSURE( SUCCEEDED( hr ), "CreateStreamOnHGlobal failed" );

        hr = CoMarshalInterface(
            pStm,
            __uuidof( IDataObject ),
            pIDo,
            MSHCTX_INPROC,
            0,
            MSHLFLAGS_NORMAL );

        OSL_ENSURE( SUCCEEDED( hr ), "CoMarshalInterface failed" );

        hr = GetHGlobalFromStream( pStm, &g_hGlob );

        OSL_ENSURE( SUCCEEDED( hr ), "GetHGlobalFromStream failed" );
    }
    else
    {
        hr = CoMarshalInterThreadInterfaceInStream(
                __uuidof( IDataObject ),
                pIDo,
                &g_pStm );

        OSL_ENSURE( SUCCEEDED( hr ), "CoMarshalInterThreadInterfaceInStream failed" );
    }
}

//----------------------------------------------------------------
//  main
//----------------------------------------------------------------

int SAL_CALL main( int nArgc, char* Argv[] )
{
    if ( nArgc < 4 )
    {
        printf( "Use 0|1|2 [sta,mta,ole] 0|1|2 [sta,mta,ole] 0|1 [raw, non-raw]\n" );
        return 0;
    }

    SetupApartment( (APARTMENT_MODEL)atoi( Argv[1] ) );

    // read parameter

    APARTMENT_MODEL apm = (APARTMENT_MODEL)atoi( Argv[2] );
    g_bRawMarshalling   = (bool)atoi( Argv[3] );

    g_hEvtThreadWakeup = CreateEventA(
        0, EVT_AUTO_RESET, EVT_INIT_NONSIGNALED, EVT_NONAME );

    OSL_ENSURE( g_hEvtThreadWakeup, "CreateEvent failed" );

    g_HandleArray[0] = CreateEventA(
        0, EVT_AUTO_RESET, EVT_INIT_SIGNALED, EVT_NONAME );

    OSL_ENSURE( g_hEvtMarshalClipDataObj, "CreateEvent failed" );

    unsigned uThreadId;

    // create a thread in another apartment
    g_HandleArray[1] = (HANDLE)_beginthreadex(
        NULL, 0, ThreadProc, &apm, 0, &uThreadId );

    OSL_ENSURE( g_hThread, "create thread failed" );

    bool bContinue = true;
    HRESULT hr;

    while( bContinue )
    {
        IDataObjectPtr pIDo;

        DWORD dwResult = MsgWaitForMultipleObjects(
            2, g_HandleArray, false, INFINITE, QS_ALLEVENTS );

        switch( dwResult )
        {
            case WAIT_OBJECT_0:
                pIDo.Detach( );
                hr = OleGetClipboard( &pIDo );
                OSL_ENSURE( SUCCEEDED( hr ), "OleGetClipboard failed" );

                MarshalClipboardDataObject( pIDo );

                // wakeup the thread
                SetEvent( g_hEvtThreadWakeup );
                break;

            case WAIT_OBJECT_0 + 1:
                bContinue = false;
                break;

            case WAIT_OBJECT_0 + 2:
            {
                // sta threads need to have a message loop
                // else the deadlock
                MSG msg;
                while( PeekMessageA( &msg, NULL, 0, 0, PM_REMOVE ) )
                    DispatchMessageA(&msg);
            }
            break;
        }
    } // while

    // cleanup
    CloseHandle( g_hEvtThreadWakeup );
    CloseHandle( g_hEvtMarshalClipDataObj );
    CloseHandle( g_hThread );

    ShutdownApartment( (APARTMENT_MODEL)atoi( Argv[1] ) );

    return 0;
}
