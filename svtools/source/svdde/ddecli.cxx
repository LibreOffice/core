/*************************************************************************
 *
 *  $RCSfile: ddecli.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:14:02 $
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

#define UNICODE
#include <string.h> // memset
#include "ddeimp.hxx"
#include <svdde.hxx>

#include <osl/thread.h>

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

// static DWORD        hDdeInst  = NULL;
// static short        nInstance = 0;

// DdeConnections*     DdeConnection::pConnections = NULL;

DdeInstData* ImpInitInstData()
{
    DdeInstData* pData = new DdeInstData;
    memset( pData,0,sizeof(DdeInstData) );
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

#ifdef WNT
HDDEDATA CALLBACK DdeInternal::CliCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, DWORD nInfo1, DWORD nInfo2 )
#else
#if defined ( MTW ) || ( defined ( GCC ) && defined ( OS2 )) || defined( ICC )
HDDEDATA CALLBACK __EXPORT DdeInternal::CliCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, DWORD nInfo1, DWORD nInfo2 )
#else
HDDEDATA CALLBACK _export DdeInternal::CliCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, DWORD nInfo1, DWORD nInfo2 )
#endif
#endif
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
    ULONG   nIntFmt = aDdeData.pImp->nFmt;
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
    Application::GetSolarMutex().acquire();
    aData.Call( (void*)p );
    Application::GetSolarMutex().release();
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
    nType = (short)XTYP_ADVSTOP;
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
//  ByteString aByteStr( rData, osl_getThreadTextEncoding() );
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
//  ByteString aByteStr( rData, osl_getThreadTextEncoding() );
    aDdeData = DdeData( (void*)rData.GetBuffer(), sizeof(sal_Unicode) * (rData.Len() + 1), CF_TEXT );
    nType = XTYP_EXECUTE;
}

// --- DdeConnection::GetError() -----------------------------------

long DdeConnection::GetError()
{
    return pImp->nStatus;
}
