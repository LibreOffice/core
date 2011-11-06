/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#if defined ( MTW ) || ( defined ( GCC ) && defined ( OS2 )) || defined( ICC )
HDDEDATA CALLBACK __EXPORT DdeInternal::InfCallback(
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

