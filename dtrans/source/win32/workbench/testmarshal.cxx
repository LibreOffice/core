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

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
