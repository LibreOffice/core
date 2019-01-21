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


#include <string.h>
#include <algorithm>
#include "ddeimp.hxx"
#include <svl/svdde.hxx>
#include <osl/thread.h>
#include <comphelper/solarmutex.hxx>

namespace {

DdeInstData * theDdeInstData;

}

DdeInstData* ImpGetInstData()
{
    return theDdeInstData;
}

DdeInstData* ImpInitInstData()
{
    theDdeInstData = new DdeInstData;
    return theDdeInstData;
}

void ImpDeinitInstData()
{
    delete theDdeInstData;
    theDdeInstData = nullptr;
}


struct DdeImp
{
    HCONV   hConv;
    long    nStatus;
};

HDDEDATA CALLBACK DdeInternal::CliCallback( UINT nCode, UINT nCbType,
                                            HCONV hConv, HSZ, HSZ hText2,
                                            HDDEDATA hData, ULONG_PTR nInfo1, ULONG_PTR )
{
    HDDEDATA nRet = DDE_FNOTPROCESSED;
    const std::vector<DdeConnection*> &rAll = DdeConnection::GetConnections();
    DdeConnection*      self = nullptr;

    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);

    for ( size_t i = 0; i < rAll.size(); ++i)
    {
        self = rAll[i];

        if ( self->pImp->hConv == hConv )
            break;
    }

    if( self )
    {
        bool bFound = false;
        std::vector<DdeTransaction*>::iterator iter;
        for( iter = self->aTransactions.begin(); iter != self->aTransactions.end(); ++iter )
        {
            switch( nCode )
            {
            case XTYP_XACT_COMPLETE:
                if( static_cast<DWORD>((*iter)->nId) == nInfo1 )
                {
                    nCode = (*iter)->nType & (XCLASS_MASK | XTYP_MASK);
                    (*iter)->bBusy = false;
                    (*iter)->Done( nullptr != hData );
                    bFound = true;
                }
                break;

            case XTYP_DISCONNECT:
                self->pImp->hConv = DdeReconnect( hConv );
                self->pImp->nStatus = self->pImp->hConv
                    ? DMLERR_NO_ERROR
                    : DdeGetLastError( pInst->hDdeInstCli );
                iter = self->aTransactions.end();
                nRet = nullptr;
                bFound = true;
                break;

            case XTYP_ADVDATA:
                bFound = *(*iter)->pName == hText2;
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
                    nRet = reinterpret_cast<HDDEDATA>(DDE_FACK);
                    break;
                }
                [[fallthrough]];

            case XTYP_REQUEST:
                DdeData d;
                d.xImp->hData = hData;
                d.xImp->nFmt  = DdeData::GetInternalFormat( nCbType );
                d.Lock();
                (*iter)->Data( &d );
                nRet = reinterpret_cast<HDDEDATA>(DDE_FACK);
                break;
            }
        }
    }
    return nRet;
}

DdeConnection::DdeConnection( const OUString& rService, const OUString& rTopic ):
    pImp(std::make_unique<DdeImp>())
{
    pImp->nStatus  = DMLERR_NO_ERROR;
    pImp->hConv    = nullptr;

    DdeInstData* pInst = ImpGetInstData();
    if( !pInst )
        pInst = ImpInitInstData();
    pInst->nRefCount++;
    pInst->nInstanceCli++;
    if ( !pInst->hDdeInstCli )
    {
        pImp->nStatus = DdeInitializeW( &pInst->hDdeInstCli,
                                        DdeInternal::CliCallback,
                                        APPCLASS_STANDARD | APPCMD_CLIENTONLY |
                                        CBF_FAIL_ALLSVRXACTIONS |
                                        CBF_SKIP_REGISTRATIONS  |
                                        CBF_SKIP_UNREGISTRATIONS, 0L );
    }

    pService = new DdeString( pInst->hDdeInstCli, rService );
    pTopic   = new DdeString( pInst->hDdeInstCli, rTopic );

    if ( pImp->nStatus == DMLERR_NO_ERROR )
    {
        pImp->hConv = DdeConnect( pInst->hDdeInstCli,pService->getHSZ(),pTopic->getHSZ(), nullptr);
        if( !pImp->hConv )
            pImp->nStatus = DdeGetLastError( pInst->hDdeInstCli );
    }

    pInst->aConnections.push_back( this );
}

DdeConnection::~DdeConnection()
{
    if ( pImp->hConv )
        DdeDisconnect( pImp->hConv );

    delete pService;
    delete pTopic;

    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);

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
}

bool DdeConnection::IsConnected()
{
    CONVINFO c;
    c.cb = sizeof( c );
    if ( DdeQueryConvInfo( pImp->hConv, QID_SYNC, &c ) )
        return true;
    else
    {
        DdeInstData* pInst = ImpGetInstData();
        pImp->hConv = DdeReconnect( pImp->hConv );
        pImp->nStatus = pImp->hConv ? DMLERR_NO_ERROR : DdeGetLastError( pInst->hDdeInstCli );
        return pImp->nStatus == DMLERR_NO_ERROR;
    }
}

const OUString DdeConnection::GetServiceName()
{
    return pService->toOUString();
}

const OUString DdeConnection::GetTopicName()
{
    return pTopic->toOUString();
}

const std::vector<DdeConnection*>& DdeConnection::GetConnections()
{
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
    return pInst->aConnections;
}

DdeTransaction::DdeTransaction( DdeConnection& d, const OUString& rItemName,
                                long n )
    : rDde( d )
{
    DdeInstData* pInst = ImpGetInstData();
    pName = new DdeString( pInst->hDdeInstCli, rItemName );
    nTime = n;
    nId   = 0;
    nType = 0;
    bBusy = false;

    rDde.aTransactions.push_back( this );
}

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

void DdeTransaction::Execute()
{
    HSZ     hItem = pName->getHSZ();
    void const * pData = aDdeData.getData();
    DWORD   nData = static_cast<DWORD>(aDdeData.getSize());
    SotClipboardFormatId nIntFmt = aDdeData.xImp->nFmt;
    UINT    nExtFmt  = DdeData::GetExternalFormat( nIntFmt );
    DdeInstData* pInst = ImpGetInstData();

    if ( nType == XTYP_EXECUTE )
        hItem = nullptr;
    if ( nType != XTYP_EXECUTE && nType != XTYP_POKE )
    {
        pData = nullptr;
        nData = 0;
    }
    if ( nTime )
    {
        HDDEDATA hData = DdeClientTransaction( static_cast<LPBYTE>(const_cast<void *>(pData)),
                                               nData, rDde.pImp->hConv,
                                               hItem, nExtFmt, static_cast<UINT>(nType),
                                               static_cast<DWORD>(nTime), nullptr );

        rDde.pImp->nStatus = DdeGetLastError( pInst->hDdeInstCli );
        if( hData && nType == XTYP_REQUEST )
        {
            {
                DdeData d;
                d.xImp->hData = hData;
                d.xImp->nFmt = nIntFmt;
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
        bBusy = true;
        DWORD result;
        HDDEDATA hRet = DdeClientTransaction( static_cast<LPBYTE>(const_cast<void *>(pData)), nData,
                                            rDde.pImp->hConv, hItem, nExtFmt,
                                            static_cast<UINT>(nType), TIMEOUT_ASYNC,
                                            &result );
        nId = result;
        rDde.pImp->nStatus = hRet ? DMLERR_NO_ERROR
                                  : DdeGetLastError( pInst->hDdeInstCli );
    }
}

const OUString DdeTransaction::GetName() const
{
    return pName->toOUString();
}

void DdeTransaction::Data( const DdeData* p )
{
    comphelper::SolarMutex *pSolarMutex = comphelper::SolarMutex::get();
    if ( pSolarMutex )
    {
        pSolarMutex->acquire();
        aData.Call( p );
        pSolarMutex = comphelper::SolarMutex::get();
        if ( pSolarMutex )
            pSolarMutex->release();
    }
}

void DdeTransaction::Done( bool bDataValid )
{
    aDone.Call( bDataValid );
}

DdeLink::DdeLink( DdeConnection& d, const OUString& aItemName, long n )
    : DdeTransaction (d, aItemName, n)
{
}

DdeLink::~DdeLink()
{
    nType = sal_uInt16(XTYP_ADVSTOP);
    nTime = 0;
}

void DdeLink::Notify()
{
    aNotify.Call( nullptr );
}

DdeRequest::DdeRequest( DdeConnection& d, const OUString& i, long n )
    : DdeTransaction( d, i, n )
{
    nType = XTYP_REQUEST;
}

DdeHotLink::DdeHotLink( DdeConnection& d, const OUString& i )
    : DdeLink( d, i, 0 )
{
    nType = XTYP_ADVSTART;
}

DdePoke::DdePoke( DdeConnection& d, const OUString& i, const DdeData& rData,
                  long n )
    : DdeTransaction( d, i, n )
{
    aDdeData = rData;
    nType = XTYP_POKE;
}

DdeExecute::DdeExecute( DdeConnection& d, const OUString& rData, long n )
    : DdeTransaction( d, OUString(), n )
{
    aDdeData = DdeData( rData.getStr(), sizeof(sal_Unicode) * (rData.getLength() + 1), SotClipboardFormatId::STRING );
    nType = XTYP_EXECUTE;
}

long DdeConnection::GetError()
{
    return pImp->nStatus;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
