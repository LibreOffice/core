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
#include "precompiled_svl.hxx"

#define UNICODE

#include <string.h>
#include "ddeimp.hxx"
#include <svl/svdde.hxx>

// --- DdeInternal::InfCallback() ----------------------------------

#ifdef WNT
HDDEDATA CALLBACK DdeInternal::InfCallback(
                WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD )
#else
#if ( defined ( GCC ) && defined ( OS2 )) || defined( ICC )
HDDEDATA CALLBACK DdeInternal::InfCallback(
                WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD )
#else
HDDEDATA CALLBACK _export DdeInternal::InfCallback(
                WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD )
#endif
#endif
{
    return (HDDEDATA)DDE_FNOTPROCESSED;
}

// --- DdeServiceList::DdeServiceList() ----------------------------

DdeServiceList::DdeServiceList( const String* pTopic )
{
    DWORD       hDdeInst = NULL;
    HCONVLIST   hConvList = NULL;
    HCONV       hConv = NULL;
    UINT        nStatus = DMLERR_NO_ERROR;
    HSZ         hTopic = NULL;

#ifndef OS2 // YD FIXME

    nStatus = DdeInitialize( &hDdeInst, (PFNCALLBACK) DdeInternal::InfCallback,
                             APPCLASS_STANDARD | APPCMD_CLIENTONLY |
                             CBF_FAIL_ALLSVRXACTIONS |
                             CBF_SKIP_ALLNOTIFICATIONS, 0L );

    if ( nStatus == DMLERR_NO_ERROR )
    {
        if ( pTopic )
        {
            LPCTSTR p = reinterpret_cast<LPCTSTR>(pTopic->GetBuffer());
#ifdef __MINGW32__
            hTopic = DdeCreateStringHandle( hDdeInst, const_cast<LPTSTR>(p), CP_WINUNICODE );
#else
            hTopic = DdeCreateStringHandle( hDdeInst, p, CP_WINUNICODE );
#endif
        }

        hConvList = DdeConnectList( hDdeInst, NULL, hTopic, NULL, NULL );
        nStatus = DdeGetLastError( hDdeInst );
    }

    if ( nStatus == DMLERR_NO_ERROR )
    {
        while ( ( hConv = DdeQueryNextServer( hConvList, hConv ) ) != NULL)
        {
            CONVINFO    aInf;
            TCHAR       buf[256], *p;
            HSZ         h;
#ifdef OS2
            aInf.nSize = sizeof( aInf );
#else
            aInf.cb = sizeof( aInf );
#endif
            if( DdeQueryConvInfo( hConv, QID_SYNC, &aInf))
            {
                h = aInf.hszServiceReq;
                if ( !h )
#ifndef OS2
                    h = aInf.hszSvcPartner;
#else
                    h = aInf.hszPartner;
#endif
                DdeQueryString( hDdeInst, h, buf, sizeof(buf) / sizeof(TCHAR), CP_WINUNICODE );
                p = buf + lstrlen( buf );
                *p++ = '|'; *p = 0;
                DdeQueryString( hDdeInst, aInf.hszTopic, p, sizeof(buf)/sizeof(TCHAR)-lstrlen( buf ),
                                CP_WINUNICODE );
                aServices.Insert( new String( reinterpret_cast<const sal_Unicode*>(buf) ) );
            }
        }
        DdeDisconnectList( hConvList );
    }

    if ( hTopic)
        DdeFreeStringHandle( hDdeInst, hTopic );
    if ( hDdeInst )
        DdeUninitialize( hDdeInst );

#endif

}

// --- DdeServiceList::~DdeServiceList() ---------------------------

DdeServiceList::~DdeServiceList()
{
    String* s;
    while ( ( s = aServices.First() ) != NULL )
    {
        aServices.Remove( s );
        delete s;
    }
}

// --- DdeTopicList::DdeTopicList() --------------------------------

DdeTopicList::DdeTopicList( const String& rService )
{
    DdeConnection aSys( rService, String( reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC) ) );

    if ( !aSys.GetError() )
    {
        DdeRequest aReq( aSys, String( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_TOPICS) ), 500 );
        aReq.SetDataHdl( LINK( this, DdeTopicList, Data ) );
        aReq.Execute();
    }
}

// --- DdeTopicList::~DdeTopicList() -------------------------------

DdeTopicList::~DdeTopicList()
{
    String* s;
    while ( ( s = aTopics.First() ) != NULL )
    {
        aTopics.Remove( s );
        delete s;
    }
}

// --- DdeTopicList::Data() --------------------------------------------

IMPL_LINK( DdeTopicList, Data, DdeData*, pData )
{
    char*   p = (char*) (const void *) *pData;
    char*   q = p;
    short   i;
    char    buf[256];

    while ( *p && *p != '\r' && *p != '\n' )
    {
        q = buf; i = 0;
        while ( i < 255 && *p && *p != '\r' && *p != '\n' && *p != '\t' )
            *q++ = *p++, i++;
        *q = 0;
        while ( *p && *p != '\r' && *p != '\n' && *p != '\t' )
            p++;
        aTopics.Insert( new String( String::CreateFromAscii(buf) ) );
        if ( *p == '\t' )
            p++;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
