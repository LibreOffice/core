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


#define UNICODE
#include <string.h> // memset
#include <algorithm>
#include "ddeimp.hxx"
#include <svl/svdde.hxx>

#include <osl/thread.h>
#include <tools/debug.hxx>
#include <tools/solarmutex.hxx>
#include <osl/mutex.hxx>

DdeInstData* ImpInitInstData()
{
    DdeInstData* pData = new DdeInstData;

    DdeInstData** ppInst = (DdeInstData**)GetAppData( SHL_SVDDE );
    *ppInst = pData;
    return pData;
}

void ImpDeinitInstData()
{
    DdeInstData** ppInst = (DdeInstData**)GetAppData( SHL_SVDDE );
    delete (*ppInst);
    *ppInst = 0;
}


struct DdeImp
{
    HCONV   hConv;
    long    nStatus;
};

// --- DdeInternat::CliCallback() ----------------------------------

HDDEDATA CALLBACK DdeInternal::CliCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ, HSZ hText2,
            HDDEDATA hData, DWORD nInfo1, DWORD )
{
    HDDEDATA nRet = DDE_FNOTPROCESSED;
    const std::vector<DdeConnection*> &rAll = DdeConnection::GetConnections();
    DdeConnection*      self = 0;

    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");

    for ( size_t i = 0; i < rAll.size(); ++i)
    {
        self = rAll[i];

        if ( self->pImp->hConv == hConv )
            break;
    }

    if( self )
    {
        sal_Bool bFound = sal_False;
        std::vector<DdeTransaction*>::iterator iter;
        for( iter = self->aTransactions.begin(); iter != self->aTransactions.end(); ++iter )
        {
            switch( nCode )
            {
                case XTYP_XACT_COMPLETE:
                    if( (DWORD)(*iter)->nId == nInfo1 )
                    {
                        nCode = (*iter)->nType & (XCLASS_MASK | XTYP_MASK);
                        (*iter)->bBusy = sal_False;
                        (*iter)->Done( 0 != hData );
                        bFound = sal_True;
                    }
                    break;

                case XTYP_DISCONNECT:
                    self->pImp->hConv = DdeReconnect( hConv );
                    self->pImp->nStatus = self->pImp->hConv
                                    ? DMLERR_NO_ERROR
                                    : DdeGetLastError( pInst->hDdeInstCli );
                    iter = self->aTransactions.end();
                    nRet = 0;
                    bFound = sal_True;
                    break;

                case XTYP_ADVDATA:
                    bFound = sal_Bool( *(*iter)->pName == hText2 );
                    break;
            }
            if( bFound )
                break;
        }

        if( iter != self->aTransactions.end() )
        {
            switch( nCode )
            {
            case XTYP_ADVDATA:
                if( !hData )
                {
                    static_cast<DdeLink*>(*iter)->Notify();
                    nRet = (HDDEDATA)DDE_FACK;
                    break;
                }
                // kein break;

            case XTYP_REQUEST:
                if( !hData && XTYP_REQUEST == nCode )
                {

                }

                DdeData d;
                d.pImp->hData = hData;
                d.pImp->nFmt  = DdeData::GetInternalFormat( nCbType );
                d.Lock();
                (*iter)->Data( &d );
                nRet = (HDDEDATA)DDE_FACK;
                break;
            }
        }
    }
    return nRet;
}

// --- DdeConnection::DdeConnection() ------------------------------

DdeConnection::DdeConnection( const String& rService, const String& rTopic )
{
    pImp = new DdeImp;
    pImp->nStatus  = DMLERR_NO_ERROR;
    pImp->hConv    = NULL;

    DdeInstData* pInst = ImpGetInstData();
    if( !pInst )
        pInst = ImpInitInstData();
    pInst->nRefCount++;
    pInst->nInstanceCli++;
    if ( !pInst->hDdeInstCli )
    {
        pImp->nStatus = DdeInitialize( &pInst->hDdeInstCli,
                                       (PFNCALLBACK)DdeInternal::CliCallback,
                                       APPCLASS_STANDARD | APPCMD_CLIENTONLY |
                                       CBF_FAIL_ALLSVRXACTIONS |
                                       CBF_SKIP_REGISTRATIONS  |
                                       CBF_SKIP_UNREGISTRATIONS, 0L );
    }

    pService = new DdeString( pInst->hDdeInstCli, rService );
    pTopic   = new DdeString( pInst->hDdeInstCli, rTopic );

    if ( pImp->nStatus == DMLERR_NO_ERROR )
    {
        pImp->hConv = DdeConnect( pInst->hDdeInstCli,*pService,*pTopic, NULL);
        if( !pImp->hConv )
            pImp->nStatus = DdeGetLastError( pInst->hDdeInstCli );
    }

    pInst->aConnections.push_back( this );
}

// --- DdeConnection::~DdeConnection() -----------------------------

DdeConnection::~DdeConnection()
{
    if ( pImp->hConv )
        DdeDisconnect( pImp->hConv );

    delete pService;
    delete pTopic;

    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");

    std::vector<DdeConnection*>::iterator it(std::find(pInst->aConnections.begin(),
                                                        pInst->aConnections.end(),
                                                        this));
    if (it != pInst->aConnections.end())
        pInst->aConnections.erase(it);

    pInst->nInstanceCli--;
    pInst->nRefCount--;
    if ( !pInst->nInstanceCli && pInst->hDdeInstCli )
    {
        if( DdeUninitialize( pInst->hDdeInstCli ) )
        {
            pInst->hDdeInstCli = 0;
            if( pInst->nRefCount == 0 )
                ImpDeinitInstData();
        }
    }
    delete pImp;
}

// --- DdeConnection::IsConnected() --------------------------------

sal_Bool DdeConnection::IsConnected()
{
    CONVINFO c;
    c.cb = sizeof( c );
    if ( DdeQueryConvInfo( pImp->hConv, QID_SYNC, &c ) )
        return sal_True;
    else
    {
        DdeInstData* pInst = ImpGetInstData();
        pImp->hConv = DdeReconnect( pImp->hConv );
        pImp->nStatus = pImp->hConv ? DMLERR_NO_ERROR : DdeGetLastError( pInst->hDdeInstCli );
        return sal_Bool( pImp->nStatus == DMLERR_NO_ERROR );
    }
}

// --- DdeConnection::GetServiceName() -----------------------------

const String& DdeConnection::GetServiceName()
{
    return (const String&)*pService;
}

// --- DdeConnection::GetTopicName() -------------------------------

const String& DdeConnection::GetTopicName()
{
    return (const String&)*pTopic;
}

// --- DdeConnection::GetConvId() ----------------------------------

long DdeConnection::GetConvId()
{
    return (long)pImp->hConv;
}

const std::vector<DdeConnection*>& DdeConnection::GetConnections()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    return pInst->aConnections;
}

// --- DdeTransaction::DdeTransaction() ----------------------------

DdeTransaction::DdeTransaction( DdeConnection& d, const String& rItemName,
                                long n ) :
                    rDde( d )
{
    DdeInstData* pInst = ImpGetInstData();
    pName = new DdeString( pInst->hDdeInstCli, rItemName );
    nTime = n;
    nId   = 0;
    nType = 0;
    bBusy = sal_False;

    rDde.aTransactions.push_back( this );
}

// --- DdeTransaction::~DdeTransaction() ---------------------------

DdeTransaction::~DdeTransaction()
{
    if ( nId && rDde.pImp->hConv )
    {
        DdeInstData* pInst = ImpGetInstData();
        DdeAbandonTransaction( pInst->hDdeInstCli, rDde.pImp->hConv, nId );
    }

    delete pName;
    rDde.aTransactions.erase(std::remove(rDde.aTransactions.begin(),
                                         rDde.aTransactions.end(),this));
}

// --- DdeTransaction::Execute() -----------------------------------

void DdeTransaction::Execute()
{
    HSZ     hItem = *pName;
    void*   pData = (void*)(const void *)aDdeData;
    DWORD   nData = (DWORD)(long)aDdeData;
    sal_uLong   nIntFmt = aDdeData.pImp->nFmt;
    UINT    nExtFmt  = DdeData::GetExternalFormat( nIntFmt );
    DdeInstData* pInst = ImpGetInstData();

    if ( nType == XTYP_EXECUTE )
        hItem = NULL;
    if ( nType != XTYP_EXECUTE && nType != XTYP_POKE )
    {
        pData = NULL;
        nData = 0L;
    }
    if ( nTime )
    {
        HDDEDATA hData = DdeClientTransaction( (unsigned char*)pData,
                                               nData, rDde.pImp->hConv,
                                               hItem, nExtFmt, (UINT)nType,
                                               (DWORD)nTime, (DWORD FAR*)NULL );

        rDde.pImp->nStatus = DdeGetLastError( pInst->hDdeInstCli );
        if( hData && nType == XTYP_REQUEST )
        {
            {
                DdeData d;
                d.pImp->hData = hData;
                d.pImp->nFmt = nIntFmt;
                d.Lock();
                Data( &d );
            }
            DdeFreeDataHandle( hData );
        }
    }
    else
    {
        if ( nId && rDde.pImp->hConv )
            DdeAbandonTransaction( pInst->hDdeInstCli, rDde.pImp->hConv, nId);
        nId = 0;
        bBusy = sal_True;
        HDDEDATA hRet = DdeClientTransaction( (unsigned char*)pData, nData,
                                            rDde.pImp->hConv, hItem, nExtFmt,
                                            (UINT)nType, TIMEOUT_ASYNC,
                                            (DWORD FAR *) ((long*) &nId) );
        rDde.pImp->nStatus = hRet ? DMLERR_NO_ERROR
                                  : DdeGetLastError( pInst->hDdeInstCli );
    }
}

// --- DdeTransaction::GetName() -----------------------------------

const rtl::OUString DdeTransaction::GetName() const
{
    return pName->toOUString();
}

// --- DdeTransaction::Data() --------------------------------------


void DdeTransaction::Data( const DdeData* p )
{
    if ( ::tools::SolarMutex::Acquire() )
    {
        aData.Call( (void*)p );
        ::tools::SolarMutex::Release();
    }
}

// --- DdeTransaction::Done() --------------------------------------

void DdeTransaction::Done( sal_Bool bDataValid )
{
    const sal_uIntPtr nDataValid(bDataValid);
    aDone.Call( reinterpret_cast<void*>(nDataValid) );
}

// --- DdeLink::DdeLink() ------------------------------------------

DdeLink::DdeLink( DdeConnection& d, const String& aItemName, long n ) :
            DdeTransaction (d, aItemName, n)
{
}

// --- DdeLink::~DdeLink() -----------------------------------------

DdeLink::~DdeLink()
{
    nType = (sal_uInt16)XTYP_ADVSTOP;
    nTime = 0;
}

// --- DdeLink::Notify() -----------------------------------------

void DdeLink::Notify()
{
    aNotify.Call( NULL );
}

// --- DdeRequest::DdeRequest() ------------------------------------

DdeRequest::DdeRequest( DdeConnection& d, const String& i, long n ) :
                DdeTransaction( d, i, n )
{
    nType = XTYP_REQUEST;
}

// --- DdeWarmLink::DdeWarmLink() ----------------------------------

DdeWarmLink::DdeWarmLink( DdeConnection& d, const String& i, long n ) :
                DdeLink( d, i, n )
{
    nType = XTYP_ADVSTART | XTYPF_NODATA;
}

// --- DdeHotLink::DdeHotLink() ------------------------------------

DdeHotLink::DdeHotLink( DdeConnection& d, const String& i, long n ) :
                DdeLink( d, i, n )
{
    nType = XTYP_ADVSTART;
}

// --- DdePoke::DdePoke() ------------------------------------------

DdePoke::DdePoke( DdeConnection& d, const String& i, const char* p,
                  long l, sal_uLong f, long n ) :
            DdeTransaction( d, i, n )
{
    aDdeData = DdeData( p, l, f );
    nType = XTYP_POKE;
}

// --- DdePoke::DdePoke() ------------------------------------------

DdePoke::DdePoke( DdeConnection& d, const String& i, const String& rData,
                  long n ) :
            DdeTransaction( d, i, n )
{
    aDdeData = DdeData( (void*) rData.GetBuffer(), sizeof(sal_Unicode) * (rData.Len()), CF_TEXT );
    nType = XTYP_POKE;
}

// --- DdePoke::DdePoke() ------------------------------------------

DdePoke::DdePoke( DdeConnection& d, const String& i, const DdeData& rData,
                  long n ) :
            DdeTransaction( d, i, n )
{
    aDdeData = rData;
    nType = XTYP_POKE;
}

// --- DdeExecute::DdeExecute() ------------------------------------

DdeExecute::DdeExecute( DdeConnection& d, const String& rData, long n ) :
                DdeTransaction( d, String(), n )
{
    aDdeData = DdeData( (void*)rData.GetBuffer(), sizeof(sal_Unicode) * (rData.Len() + 1), CF_TEXT );
    nType = XTYP_EXECUTE;
}

// --- DdeConnection::GetError() -----------------------------------

long DdeConnection::GetError()
{
    return pImp->nStatus;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
