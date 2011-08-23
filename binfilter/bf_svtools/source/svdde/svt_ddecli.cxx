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


#define UNICODE
#include <string.h> // memset
#include "ddeimp.hxx"
#include <bf_svtools/svdde.hxx>

#include <osl/thread.h>

#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace binfilter
{

// static DWORD        hDdeInst  = NULL;
// static short        nInstance = 0;

// DdeConnections*     DdeConnection::pConnections = NULL;

DdeInstData* ImpInitInstData()
{
    DdeInstData* pData = new DdeInstData;
    memset( pData,0,sizeof(DdeInstData) );
    DdeInstData** ppInst = (DdeInstData**)GetAppData( BF_SHL_SVDDE );
    *ppInst = pData;
    return pData;
}

void ImpDeinitInstData()
{
    DdeInstData** ppInst = (DdeInstData**)GetAppData( BF_SHL_SVDDE );
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
    DdeConnections&     rAll = (DdeConnections&)DdeConnection::GetConnections();
    DdeConnection*      self = 0;

    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");

    for ( self = rAll.First(); self; self = rAll.Next() )
        if ( self->pImp->hConv == hConv )
            break;

    if( self )
    {
        DdeTransaction* t;
        BOOL bFound = FALSE;
        for( t = self->aTransactions.First(); t; t = self->aTransactions.Next() )
        {
            switch( nCode )
            {
                case XTYP_XACT_COMPLETE:
                    if( (DWORD)t->nId == nInfo1 )
                    {
                        nCode = t->nType & (XCLASS_MASK | XTYP_MASK);
                        t->bBusy = FALSE;
                        t->Done( 0 != hData );
                        bFound = TRUE;
                    }
                    break;

                case XTYP_DISCONNECT:
                    self->pImp->hConv = DdeReconnect( hConv );
                    self->pImp->nStatus = self->pImp->hConv
                                    ? DMLERR_NO_ERROR
                                    : DdeGetLastError( pInst->hDdeInstCli );
                    t = 0;
                    nRet = 0;
                    bFound = TRUE;
                    break;

                case XTYP_ADVDATA:
                    bFound = BOOL( *t->pName == hText2 );
                    break;
            }
            if( bFound )
                break;
        }

        if( t )
        {
            switch( nCode )
            {
            case XTYP_ADVDATA:
                if( !hData )
                {
                    ((DdeLink*) t)->Notify();
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
                t->Data( &d );
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
        pInst->pConnections = new DdeConnections;
    }

    pService = new DdeString( pInst->hDdeInstCli, rService );
    pTopic   = new DdeString( pInst->hDdeInstCli, rTopic );

    if ( pImp->nStatus == DMLERR_NO_ERROR )
    {
        pImp->hConv = DdeConnect( pInst->hDdeInstCli,*pService,*pTopic, NULL);
        if( !pImp->hConv )
            pImp->nStatus = DdeGetLastError( pInst->hDdeInstCli );
    }

    if ( pInst->pConnections )
        pInst->pConnections->Insert( this );
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
    if ( pInst->pConnections )
        pInst->pConnections->Remove( this );

    pInst->nInstanceCli--;
    pInst->nRefCount--;
    if ( !pInst->nInstanceCli && pInst->hDdeInstCli )
    {
        if( DdeUninitialize( pInst->hDdeInstCli ) )
        {
            pInst->hDdeInstCli = NULL;
            delete pInst->pConnections;
            pInst->pConnections = NULL;
            if( pInst->nRefCount == 0 )
                ImpDeinitInstData();
        }
    }
    delete pImp;
}

// --- DdeConnection::IsConnected() --------------------------------

BOOL DdeConnection::IsConnected()
{
    CONVINFO c;
#ifdef OS2
    c.nSize = sizeof( c );
#else
    c.cb = sizeof( c );
#endif
    if ( DdeQueryConvInfo( pImp->hConv, QID_SYNC, &c ) )
        return TRUE;
    else
    {
        DdeInstData* pInst = ImpGetInstData();
        pImp->hConv = DdeReconnect( pImp->hConv );
        pImp->nStatus = pImp->hConv ? DMLERR_NO_ERROR : DdeGetLastError( pInst->hDdeInstCli );
        return BOOL( pImp->nStatus == DMLERR_NO_ERROR );
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

const DdeConnections& DdeConnection::GetConnections()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    return *(pInst->pConnections);
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
    bBusy = FALSE;

    rDde.aTransactions.Insert( this );
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
    rDde.aTransactions.Remove( this );
}

// --- DdeTransaction::Execute() -----------------------------------

void DdeTransaction::Execute()
{
    HSZ     hItem = *pName;
    void*   pData = (void*)(const void *)aDdeData;
    DWORD   nData = (DWORD)(long)aDdeData;
    ULONG  	nIntFmt = aDdeData.pImp->nFmt;
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
        bBusy = TRUE;
        HDDEDATA hRet = DdeClientTransaction( (unsigned char*)pData, nData,
                                            rDde.pImp->hConv, hItem, nExtFmt,
                                            (UINT)nType, TIMEOUT_ASYNC,
                                            (DWORD FAR *) ((long*) &nId) );
        rDde.pImp->nStatus = hRet ? DMLERR_NO_ERROR
                                  : DdeGetLastError( pInst->hDdeInstCli );
    }
}

// --- DdeTransaction::GetName() -----------------------------------

const String& DdeTransaction::GetName() const
{
    return *pName;
}

// --- DdeTransaction::Data() --------------------------------------


void __EXPORT DdeTransaction::Data( const DdeData* p )
{
    SolarMutexGuard aSolarGuard;

    aData.Call( (void*)p );
}

// --- DdeTransaction::Done() --------------------------------------

void __EXPORT DdeTransaction::Done( BOOL bDataValid )
{
    aDone.Call( (void*)bDataValid );
}

// --- DdeLink::DdeLink() ------------------------------------------

DdeLink::DdeLink( DdeConnection& d, const String& aItemName, long n ) :
            DdeTransaction (d, aItemName, n)
{
}

// --- DdeLink::~DdeLink() -----------------------------------------

DdeLink::~DdeLink()
{
    nType = (USHORT)XTYP_ADVSTOP;
    nTime = 0;
}

// --- DdeLink::Notify() -----------------------------------------

void __EXPORT DdeLink::Notify()
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
                  long l, ULONG f, long n ) :
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
//	ByteString aByteStr( rData, osl_getThreadTextEncoding() );
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
//	ByteString aByteStr( rData, osl_getThreadTextEncoding() );
    aDdeData = DdeData( (void*)rData.GetBuffer(), sizeof(sal_Unicode) * (rData.Len() + 1), CF_TEXT );
    nType = XTYP_EXECUTE;
}

// --- DdeConnection::GetError() -----------------------------------

long DdeConnection::GetError()
{
    return pImp->nStatus;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
