/*************************************************************************
 *
 *  $RCSfile: ddesvr.cxx,v $
 *
 *  $Revision: 1.3 $
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
#include "ddeimp.hxx"
#include <svdde.hxx>
#include <svarray.hxx>

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

//static long         hCurConv  = 0;
//static DWORD        hDdeInst  = NULL;
//static short        nInstance = 0;
//static DdeServices* pServices;

enum DdeItemType
{
    DDEITEM,
    DDEGETPUTITEM
};

struct DdeItemImpData
{
    ULONG nHCnv;
    USHORT nCnt;

    DdeItemImpData( ULONG nH ) : nHCnv( nH ), nCnt( 1 ) {}
};

SV_DECL_VARARR( DdeItemImp, DdeItemImpData, 1, 1 )
SV_IMPL_VARARR( DdeItemImp, DdeItemImpData )

// --- DdeInternat::SvrCallback() ----------------------------------

#ifdef WNT
HDDEDATA CALLBACK DdeInternal::SvrCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, DWORD, DWORD )
#else
#if defined ( MTW ) || ( defined ( GCC ) && defined ( OS2 )) || defined( ICC )
HDDEDATA CALLBACK __EXPORT DdeInternal::SvrCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, DWORD, DWORD )
#else
HDDEDATA CALLBACK _export DdeInternal::SvrCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, DWORD, DWORD )
#endif
#endif
{
    DdeServices&    rAll = DdeService::GetServices();
    DdeService*     pService;
    DdeTopic*       pTopic;
    DdeItem*        pItem;
    DdeData*        pData;
    Conversation*   pC;

    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");

    switch( nCode )
    {
        case XTYP_WILDCONNECT:
        {
            short nTopics = 0;

#if 1
            TCHAR chTopicBuf[250];
            if( hText1 )
                DdeQueryString( pInst->hDdeInstSvr, hText1, chTopicBuf,
                                sizeof(chTopicBuf)/sizeof(TCHAR), CP_WINUNICODE );

            for( pService = rAll.First();pService;pService = rAll.Next() )
            {
                if ( !hText2 || ( *pService->pName == hText2 ) )
                {
                    String sTopics( pService->Topics() );
                    if( sTopics.Len() )
                    {
                        if( hText1 )
                        {
                            USHORT n = 0;
                            while( STRING_NOTFOUND != n )
                            {
                                String s( sTopics.GetToken( 0, '\t', n ));
                                if( s == chTopicBuf )
                                    ++nTopics;
                            }
                        }
                        else
                            nTopics += sTopics.GetTokenCount( '\t' );
                    }
                }
            }

#else
            for( pService = rAll.First();pService;pService = rAll.Next() )
            {
                if ( !hText2 || ( *pService->pName == hText2 ) )
                {
                    for( pTopic = pService->aTopics.First(); pTopic;
                         pTopic = pService->aTopics.Next() )
                    {
                        if ( !hText1 || (*pTopic->pName == hText1) )
                            nTopics++;
                    }
                }
            }
#endif
            if( !nTopics )
                return (HDDEDATA)NULL;

            HSZPAIR* pPairs = new HSZPAIR [nTopics + 1];
            if ( !pPairs )
                return (HDDEDATA)NULL;

            HSZPAIR* q = pPairs;
            for( pService = rAll.First(); pService; pService = rAll.Next() )
            {
                if ( !hText2 || (*pService->pName == hText2 ) )
                {
#if 0
                    for ( pTopic = pService->aTopics.First(); pTopic;
                          pTopic = pService->aTopics.Next() )
                    {
                        if ( !hText1 || (*pTopic->pName == hText1) )
                        {
                            q->hszSvc   = *pService->pName;
                            q->hszTopic = *pTopic->pName;
                            q++;
                        }
                    }
#else
                    String sTopics( pService->Topics() );
                    USHORT n = 0;
                    while( STRING_NOTFOUND != n )
                    {
                        String s( sTopics.GetToken( 0, '\t', n ));
                        s.EraseAllChars( '\n' ).EraseAllChars( '\r' );
                        if( !hText1 || s == chTopicBuf )
                        {
                            DdeString aDStr( pInst->hDdeInstSvr, s );
                            pTopic = FindTopic( *pService, (HSZ)aDStr );
                            if( pTopic )
                            {
                                q->hszSvc   = *pService->pName;
                                q->hszTopic = *pTopic->pName;
                                q++;
                            }
                        }
                    }

#endif
                }
            }

            q->hszSvc   = NULL;
            q->hszTopic = NULL;
            HDDEDATA h = DdeCreateDataHandle(
                            pInst->hDdeInstSvr, (LPBYTE) pPairs,
                            sizeof(HSZPAIR) * (nTopics+1),
                            0, NULL, nCbType, 0);
            delete pPairs;
            return h;
        }

        case XTYP_CONNECT:
            pService = FindService( hText2 );
            if ( pService)
                pTopic = FindTopic( *pService, hText1 );
            else
                pTopic = NULL;
            if ( pTopic )
                return (HDDEDATA)DDE_FACK;
            else
                return (HDDEDATA) NULL;

        case XTYP_CONNECT_CONFIRM:
            pService = FindService( hText2 );
            if ( pService )
            {
                pTopic = FindTopic( *pService, hText1 );
                if ( pTopic )
                {
                    pTopic->Connect( (long) hConv );
                    pC = new Conversation;
                    pC->hConv = hConv;
                    pC->pTopic = pTopic;
                    pService->pConv->Insert( pC );
                }
            }
            return (HDDEDATA)NULL;
    }

    for ( pService = rAll.First(); pService; pService = rAll.Next() )
    {
        for( pC = pService->pConv->First(); pC;
             pC = pService->pConv->Next() )
        {
            if ( pC->hConv == hConv )
                goto found;
        }
    }

    return (HDDEDATA) DDE_FNOTPROCESSED;

found:
    if ( nCode == XTYP_DISCONNECT)
    {
        pC->pTopic->_Disconnect( (long) hConv );
        pService->pConv->Remove( pC );
        delete pC;
        return (HDDEDATA)NULL;
    }

    BOOL bExec = BOOL(nCode == XTYP_EXECUTE);
    pTopic = pC->pTopic;
    if ( pTopic && !bExec )
        pItem = FindItem( *pTopic, hText2 );
    else
        pItem = NULL;

    if ( !bExec && !pService->HasCbFormat( nCbType ) )
        pItem = NULL;
    if ( !pItem && !bExec )
        return (HDDEDATA)DDE_FNOTPROCESSED;
    if ( pItem )
        pTopic->aItem = pItem->GetName();
    else
        pTopic->aItem.Erase();

    BOOL bRes = FALSE;
    pInst->hCurConvSvr = (long)hConv;
    switch( nCode )
    {
        case XTYP_REQUEST:
        case XTYP_ADVREQ:
            {
            String aRes;          // darf erst am Ende freigegeben werden!!
            if ( pTopic->IsSystemTopic() )
            {
                if ( pTopic->aItem == SZDDESYS_ITEM_TOPICS )
                    aRes = pService->Topics();
                else if ( pTopic->aItem == SZDDESYS_ITEM_SYSITEMS )
                    aRes = pService->SysItems();
                else if ( pTopic->aItem == SZDDESYS_ITEM_STATUS )
                    aRes = pService->Status();
                else if ( pTopic->aItem == SZDDESYS_ITEM_FORMATS )
                    aRes = pService->Formats();
                else if ( pTopic->aItem ==  SZDDESYS_ITEM_HELP )
                    aRes = pService->GetHelp();
                else
                    aRes = pService->SysTopicGet( pTopic->aItem );

                if ( aRes.Len() )
                    pData = new DdeData( aRes );
                else
                    pData = NULL;
            }
            else if( DDEGETPUTITEM == pItem->nType )
                pData = ((DdeGetPutItem*)pItem)->Get(
                            DdeData::GetInternalFormat( nCbType ) );
            else
                pData = pTopic->Get( DdeData::GetInternalFormat( nCbType ));

            if ( pData )
                return DdeCreateDataHandle( pInst->hDdeInstSvr,
                                            (LPBYTE)pData->pImp->pData,
                                            pData->pImp->nData,
                                            0, hText2,
                                            DdeData::GetExternalFormat(
                                                pData->pImp->nFmt ),
                                            0 );
            }
            break;

        case XTYP_POKE:
            if ( !pTopic->IsSystemTopic() )
            {
                DdeData d;
                d.pImp->hData = hData;
                d.pImp->nFmt  = DdeData::GetInternalFormat( nCbType );
                d.Lock();
                if( DDEGETPUTITEM == pItem->nType )
                    bRes = ((DdeGetPutItem*)pItem)->Put( &d );
                else
                    bRes = pTopic->Put( &d );
            }
            pInst->hCurConvSvr = NULL;
            if ( bRes )
                return (HDDEDATA)DDE_FACK;
            else
                return (HDDEDATA) DDE_FNOTPROCESSED;

        case XTYP_ADVSTART:
            {
                // wird das Item zum erstenmal ein HotLink ?
                if( !pItem->pImpData && pTopic->StartAdviseLoop() )
                {
                    // dann wurde das Item ausgewechselt
                    pTopic->aItems.Remove( pItem );
                    DdeItem* pTmp;
                    for(  pTmp = pTopic->aItems.First(); pTmp;
                                    pTmp = pTopic->aItems.Next() )
                        if( *pTmp->pName == hText2 )
                        {
                            // es wurde tatsaechlich ausgewechselt
                            delete pItem;
                            pItem = 0;
                            break;
                        }
                    if( pItem )
                        // es wurde doch nicht ausgewechselt, also wieder rein
                        pTopic->aItems.Insert( pItem );
                    else
                        pItem = pTmp;
                }
                pItem->IncMonitor( (long)hConv );
                pInst->hCurConvSvr = NULL;
            }
            return (HDDEDATA)TRUE;

        case XTYP_ADVSTOP:
            pItem->DecMonitor( (long)hConv );
            if( !pItem->pImpData )
                pTopic->StopAdviseLoop();
            pInst->hCurConvSvr = NULL;
            return (HDDEDATA)TRUE;

        case XTYP_EXECUTE:
            {
                DdeData aExec;
                aExec.pImp->hData = hData;
                aExec.pImp->nFmt  = DdeData::GetInternalFormat( nCbType );
                aExec.Lock();
                String aName;

                aName = (const sal_Unicode *)aExec.pImp->pData;

                if( pTopic->IsSystemTopic() )
                    bRes = pService->SysTopicExecute( &aName );
                else
                    bRes = pTopic->Execute( &aName );
            }
            pInst->hCurConvSvr = NULL;
            if ( bRes )
                return (HDDEDATA)DDE_FACK;
            else
                return (HDDEDATA)DDE_FNOTPROCESSED;
    }

    return (HDDEDATA)NULL;
}

// --- DdeInternat::FindService() ----------------------------------

DdeService* DdeInternal::FindService( HSZ hService )
{
    DdeService*  s;
    DdeServices& rSvc = DdeService::GetServices();
    for ( s = rSvc.First(); s; s = rSvc.Next() )
    {
        if ( *s->pName == hService )
            return s;
    }

    return NULL;
}

// --- DdeInternat::FindTopic() ------------------------------------

DdeTopic* DdeInternal::FindTopic( DdeService& rService, HSZ hTopic )
{
    DdeTopic* s;
    DdeTopics& rTopics = rService.aTopics;
    int bWeiter = FALSE;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");

    do {            // middle check loop
        for ( s = rTopics.First(); s; s = rTopics.Next() )
        {
            if ( *s->pName == hTopic )
                return s;
        }

        bWeiter = !bWeiter;
        if( !bWeiter )
            break;

        // dann befragen wir doch mal unsere Ableitung:
        TCHAR chBuf[250];
        DdeQueryString(pInst->hDdeInstSvr,hTopic,chBuf,sizeof(chBuf)/sizeof(TCHAR),CP_WINUNICODE );
        bWeiter = rService.MakeTopic( chBuf );
        // dann muessen wir noch mal suchen
    } while( bWeiter );

    return 0;
}

// --- DdeInternal::FindItem() -------------------------------------

DdeItem* DdeInternal::FindItem( DdeTopic& rTopic, HSZ hItem )
{
    DdeItem* s;
    DdeItems& rItems = rTopic.aItems;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    int bWeiter = FALSE;

    do {            // middle check loop

        for ( s = rItems.First(); s; s = rItems.Next() )
            if ( *s->pName == hItem )
                return s;

        bWeiter = !bWeiter;
        if( !bWeiter )
            break;

        // dann befragen wir doch mal unsere Ableitung:
        TCHAR chBuf[250];
        DdeQueryString(pInst->hDdeInstSvr,hItem,chBuf,sizeof(chBuf)/sizeof(TCHAR),CP_WINUNICODE );
        bWeiter = rTopic.MakeItem( chBuf );
        // dann muessen wir noch mal suchen
    } while( bWeiter );

    return 0;
}

// --- DdeService::DdeService() ------------------------------------

DdeService::DdeService( const String& rService )
{
    DdeInstData* pInst = ImpGetInstData();
    if( !pInst )
        pInst = ImpInitInstData();
    pInst->nRefCount++;
    pInst->nInstanceSvr++;

    if ( !pInst->hDdeInstSvr )
    {
        nStatus = DdeInitialize( &pInst->hDdeInstSvr,
                                 (PFNCALLBACK)DdeInternal::SvrCallback,
                                 APPCLASS_STANDARD |
                                 CBF_SKIP_REGISTRATIONS |
                                 CBF_SKIP_UNREGISTRATIONS, 0L );
        pInst->pServicesSvr = new DdeServices;
    }
    else
        nStatus = DMLERR_NO_ERROR;

    pConv = new ConvList;

    if ( pInst->pServicesSvr )
        pInst->pServicesSvr->Insert( this );

    pName = new DdeString( pInst->hDdeInstSvr, rService );
    if ( nStatus == DMLERR_NO_ERROR )
        if ( !DdeNameService( pInst->hDdeInstSvr, *pName, NULL,
                                DNS_REGISTER | DNS_FILTEROFF ) )
            nStatus = DMLERR_SYS_ERROR;

    AddFormat( FORMAT_STRING );
    pSysTopic = new DdeTopic( SZDDESYS_TOPIC );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_TOPICS ) );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_SYSITEMS ) );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_STATUS ) );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_FORMATS ) );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_HELP ) );
    AddTopic( *pSysTopic );
}

// --- DdeService::~DdeService() -----------------------------------

DdeService::~DdeService()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    if ( pInst->pServicesSvr )
        pInst->pServicesSvr->Remove( this );

    // MT: Im Auftrage des Herrn (AM) auskommentiert...
    // Grund:
    // Bei Client/Server werden die Server nicht beendet, wenn mehr
    // als einer gestartet.
    // Weil keine System-Messagequeue ?!

#ifndef VCL
    DdeNameService( pInst->hDdeInstSvr, *pName, NULL, DNS_UNREGISTER );
#endif

    delete pSysTopic;
    delete pName;

    pInst->nInstanceSvr--;
    pInst->nRefCount--;
    if ( !pInst->nInstanceSvr && pInst->hDdeInstSvr )
    {
        if( DdeUninitialize( pInst->hDdeInstSvr ) )
        {
            pInst->hDdeInstSvr = NULL;
            delete pInst->pServicesSvr;
            pInst->pServicesSvr = NULL;
            if( pInst->nRefCount == 0)
                ImpDeinitInstData();
        }
    }
    delete pConv;
}

// --- DdeService::GetName() ---------------------------------------

const String& DdeService::GetName() const
{
    return *pName;
}

// --- DdeService::GetServices() -----------------------------------

DdeServices& DdeService::GetServices()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    return *(pInst->pServicesSvr);
}

// --- DdeService::AddTopic() --------------------------------------

void DdeService::AddTopic( const DdeTopic& rTopic )
{
    RemoveTopic( rTopic );
    aTopics.Insert( (DdeTopic*) &rTopic );
}

// --- DdeService::RemoveTopic() -----------------------------------

void DdeService::RemoveTopic( const DdeTopic& rTopic )
{
    DdeTopic* t;
    for ( t = aTopics.First(); t; t = aTopics.Next() )
    {
        if ( !DdeCmpStringHandles (*t->pName, *rTopic.pName ) )
        {
            aTopics.Remove( t );
            // JP 27.07.95: und alle Conversions loeschen !!!
            //              (sonst wird auf geloeschten Topics gearbeitet!!)
            for( ULONG n = pConv->Count(); n; )
            {
                Conversation* pC = pConv->GetObject( --n );
                if( pC->pTopic == &rTopic )
                {
                    pConv->Remove( pC );
                    delete pC;
                }
            }
            break;
        }
    }
}

// --- DdeService::HasCbFormat() -----------------------------------

BOOL DdeService::HasCbFormat( USHORT nFmt )
{
    return BOOL( aFormats.GetPos( nFmt ) != LIST_ENTRY_NOTFOUND );
}

// --- DdeService::HasFormat() -------------------------------------

BOOL DdeService::HasFormat( ULONG nFmt )
{
    return HasCbFormat( (USHORT)DdeData::GetExternalFormat( nFmt ));
}

// --- DdeService::AddFormat() -------------------------------------

void DdeService::AddFormat( ULONG nFmt )
{
    nFmt = DdeData::GetExternalFormat( nFmt );
    aFormats.Remove( nFmt );
    aFormats.Insert( nFmt );
}

// --- DdeService::RemoveFormat() ----------------------------------

void DdeService::RemoveFormat( ULONG nFmt )
{
    aFormats.Remove( DdeData::GetExternalFormat( nFmt ) );
}

// --- DdeTopic::DdeTopic() ----------------------------------------

DdeTopic::DdeTopic( const String& rName )
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    pName = new DdeString( pInst->hDdeInstSvr, rName );
}

// --- DdeTopic::~DdeTopic() ---------------------------------------

DdeTopic::~DdeTopic()
{
    DdeItem* t;
    while( ( t = aItems.First() ) != NULL )
    {
        aItems.Remove( t );
        t->pMyTopic = 0;
        delete t;
    }
    delete pName;
}

// --- DdeTopic::GetName() -----------------------------------------

const String& DdeTopic::GetName() const
{
    return *pName;
}

// --- DdeTopic::IsSystemTopic() -----------------------------------

BOOL DdeTopic::IsSystemTopic()
{
    return BOOL (GetName() == SZDDESYS_TOPIC);
}

// --- DdeTopic::AddItem() -----------------------------------------

DdeItem* DdeTopic::AddItem( const DdeItem& r )
{
    DdeItem* s;
    if( DDEGETPUTITEM == r.nType )
        s = new DdeGetPutItem( r );
    else
        s = new DdeItem( r );
    if ( s )
    {
        aItems.Insert( s );
        s->pMyTopic = this;
    }
    return s;
}

// --- DdeTopic::InsertItem() -----------------------------------------

void DdeTopic::InsertItem( DdeItem* pNew )
{
    if( pNew )
    {
        aItems.Insert( pNew );
        pNew->pMyTopic = this;
    }
}

// --- DdeTopic::RemoveItem() --------------------------------------

void DdeTopic::RemoveItem( const DdeItem& r )
{
    DdeItem* s;
    for ( s = aItems.First(); s; s = aItems.Next() )
    {
        if ( !DdeCmpStringHandles (*s->pName, *r.pName ) )
            break;
    }

    if ( s )
    {
        aItems.Remove( s );
        s->pMyTopic = 0;
        delete s;
    }
}

// --- DdeTopic::NotifyClient() ------------------------------------

void DdeTopic::NotifyClient( const String& rItem )
{
    DdeItem* pItem;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    for ( pItem = aItems.First(); pItem; pItem = aItems.Next() )
    {
        if ( pItem->GetName() == rItem )
        {
            if ( pItem->pImpData )
                DdePostAdvise( pInst->hDdeInstSvr, *pName, *pItem->pName );
        }
        break;
    }
}

// --- DdeTopic::Connect() -----------------------------------------

void __EXPORT DdeTopic::Connect( long nId )
{
    aConnectLink.Call( (void*)nId );
}

// --- DdeTopic::Disconnect() --------------------------------------

void __EXPORT DdeTopic::Disconnect( long nId )
{
    aDisconnectLink.Call( (void*)nId );
}

// --- DdeTopic::_Disconnect() --------------------------------------

void __EXPORT DdeTopic::_Disconnect( long nId )
{
    for( DdeItem* pItem = aItems.First(); pItem; pItem = aItems.Next() )
        pItem->DecMonitor( nId );

    Disconnect( nId );
}

// --- DdeTopic::Get() ---------------------------------------------

DdeData* __EXPORT DdeTopic::Get( ULONG nFmt )
{
    if ( aGetLink.IsSet() )
        return (DdeData*)aGetLink.Call( (void*)nFmt );
    else
        return NULL;
}

// --- DdeTopic::Put() ---------------------------------------------

BOOL __EXPORT DdeTopic::Put( const DdeData* r )
{
    if ( aPutLink.IsSet() )
        return (BOOL)aPutLink.Call( (void*) r );
    else
        return FALSE;
}

// --- DdeTopic::Execute() -----------------------------------------

BOOL __EXPORT DdeTopic::Execute( const String* r )
{
    if ( aExecLink.IsSet() )
        return (BOOL)aExecLink.Call( (void*)r );
    else
        return FALSE;
}

// --- DdeTopic::GetConvId() ---------------------------------------

long DdeTopic::GetConvId()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    return pInst->hCurConvSvr;
}

// --- DdeTopic::StartAdviseLoop() ---------------------------------

BOOL DdeTopic::StartAdviseLoop()
{
    return FALSE;
}

// --- DdeTopic::StopAdviseLoop() ----------------------------------

BOOL DdeTopic::StopAdviseLoop()
{
    return FALSE;
}

// --- DdeItem::DdeItem() ------------------------------------------

DdeItem::DdeItem( const sal_Unicode* p )
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    pName = new DdeString( pInst->hDdeInstSvr, p );
    nType = DDEITEM;
    pMyTopic = 0;
    pImpData = 0;
}

// --- DdeItem::DdeItem() ------------------------------------------

DdeItem::DdeItem( const String& r)
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    pName = new DdeString( pInst->hDdeInstSvr, r );
    nType = DDEITEM;
    pMyTopic = 0;
    pImpData = 0;
}

// --- DdeItem::DdeItem() ------------------------------------------

DdeItem::DdeItem( const DdeItem& r)
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    pName = new DdeString( pInst->hDdeInstSvr, *r.pName );
    nType = DDEITEM;
    pMyTopic = 0;
    pImpData = 0;
}

// --- DdeItem::~DdeItem() -----------------------------------------

DdeItem::~DdeItem()
{
    if( pMyTopic )
        pMyTopic->aItems.Remove( this );
    delete pName;
    delete pImpData;
}

// --- DdeItem::GetName() ------------------------------------------

const String& DdeItem::GetName() const
{
    return *pName;
}

// --- DdeItem::NotifyClient() ------------------------------------------

void DdeItem::NotifyClient()
{
    if( pMyTopic && pImpData )
    {
        DdeInstData* pInst = ImpGetInstData();
        DBG_ASSERT(pInst,"SVDDE:No instance data");
        DdePostAdvise( pInst->hDdeInstSvr, *pMyTopic->pName, *pName );
    }
}

// --- DdeItem::IncMonitor() ------------------------------------------

void DdeItem::IncMonitor( ULONG nHCnv )
{
    if( !pImpData )
    {
        pImpData = new DdeItemImp;
        if( DDEGETPUTITEM == nType )
            ((DdeGetPutItem*)this)->AdviseLoop( TRUE );
    }
    else
    {
        for( USHORT n = pImpData->Count(); n; )
            if( (*pImpData)[ --n ].nHCnv == nHCnv )
            {
                ++(*pImpData)[ n ].nHCnv;
                return ;
            }
    }

    pImpData->Insert( DdeItemImpData( nHCnv ), pImpData->Count() );
}

// --- DdeItem::DecMonitor() ------------------------------------------

void DdeItem::DecMonitor( ULONG nHCnv )
{
    if( pImpData )
    {
        DdeItemImpData* pData = (DdeItemImpData*)pImpData->GetData();
        for( USHORT n = pImpData->Count(); n; --n, ++pData )
            if( pData->nHCnv == nHCnv )
            {
                if( !pData->nCnt || !--pData->nCnt )
                {
                    if( 1 < pImpData->Count() )
                        pImpData->Remove( pImpData->Count() - n );
                    else
                    {
                        delete pImpData, pImpData = 0;
                        if( DDEGETPUTITEM == nType )
                            ((DdeGetPutItem*)this)->AdviseLoop( FALSE );
                    }
                }
                return ;
            }
    }
}

// --- DdeItem::GetLinks() ------------------------------------------

short DdeItem::GetLinks()
{
    short nCnt = 0;
    if( pImpData )
        for( USHORT n = pImpData->Count(); n; )
            nCnt += (*pImpData)[ --n ].nCnt;
    return nCnt;
}

// --- DdeGetPutItem::DdeGetPutItem() ------------------------------

DdeGetPutItem::DdeGetPutItem( const sal_Unicode* p )
    : DdeItem( p )
{
    nType = DDEGETPUTITEM;
}

// --- DdeGetPutItem::DdeGetPutItem() ------------------------------

DdeGetPutItem::DdeGetPutItem( const String& rStr )
    : DdeItem( rStr )
{
    nType = DDEGETPUTITEM;
}

// --- DdeGetPutItem::DdeGetPutItem() ------------------------------

DdeGetPutItem::DdeGetPutItem( const DdeItem& rItem )
    : DdeItem( rItem )
{
    nType = DDEGETPUTITEM;
}


// --- DdeGetPutData::Get() ----------------------------------------

DdeData* DdeGetPutItem::Get( ULONG )
{
    return 0;
}

// --- DdeGetPutData::Put() ----------------------------------------

BOOL DdeGetPutItem::Put( const DdeData* )
{
    return FALSE;
}

// --- DdeGetPutData::AdviseLoop() ---------------------------------

void DdeGetPutItem::AdviseLoop( BOOL )
{
}


// --- DdeService::SysItems() --------------------------------------

String DdeService::SysItems()
{
    String s;
    DdeTopic* t;
    for ( t = aTopics.First(); t; t = aTopics.Next() )
    {
        if ( t->GetName() == String::CreateFromAscii( (const sal_Char *)SZDDESYS_TOPIC ) )
        {
            short n = 0;
            DdeItem* pi;
            for ( pi = t->aItems.First(); pi; pi = t->aItems.Next(), n++ )
            {
                if ( n )
                    s += '\t';
                s += pi->GetName();
            }
            s += String::CreateFromAscii("\r\n");
        }
    }

    return s;
}

// --- DdeService::Topics() ----------------------------------------

String DdeService::Topics()
{
    String      s;
    DdeTopic*   t;
    short       n = 0;

    for ( t = aTopics.First(); t; t = aTopics.Next(), n++ )
    {
        if ( n )
            s += '\t';
        s += t->GetName();
    }
    s += String::CreateFromAscii("\r\n");

    return s;
}

// --- DdeService::Formats() ---------------------------------------

String DdeService::Formats()
{
    String      s;
    long        f;
    TCHAR       buf[128];
    LPCTSTR     p;
    short       n = 0;

    for ( f = aFormats.First(); f; f = aFormats.Next(), n++ )
    {
        if ( n )
            s += '\t';
        p = buf;

        switch( (USHORT)f )
        {
            case CF_TEXT:
                p = String::CreateFromAscii("TEXT").GetBuffer();
                break;
            case CF_BITMAP:
                p = String::CreateFromAscii("BITMAP").GetBuffer();
                break;
#ifdef OS2
            case CF_DSPTEXT:
                p = String::CreateFromAscii("TEXT").GetBuffer();
                break;
            case CF_DSPBITMAP:
                p = String::CreateFromAscii("BITMAP").GetBuffer();
                break;
            case CF_METAFILE:
                p = String::CreateFromAscii("METAFILE").GetBuffer();
                break;
            case CF_DSPMETAFILE:
                p = String::CreateFromAscii("METAFILE").GetBuffer();
                break;
            case CF_PALETTE:
                p = String::CreateFromAscii("PALETTE").GetBuffer();
                break;
            default:
                p= String::CreateFromAscii("PRIVATE").GetBuffer();
#else
            default:
                GetClipboardFormatName( (UINT)f, buf, sizeof(buf) / sizeof(TCHAR) );
#endif
        }
        s += String( p );
    }
    s += String::CreateFromAscii("\r\n");

    return s;
}

// --- DdeService::Status() ----------------------------------------

String DdeService::Status()
{
    return IsBusy() ? String::CreateFromAscii("Busy\r\n") : String::CreateFromAscii("Ready\r\n");
}

// --- DdeService::IsBusy() ----------------------------------------

BOOL __EXPORT DdeService::IsBusy()
{
    return FALSE;
}

// --- DdeService::GetHelp() ----------------------------------------

String __EXPORT DdeService::GetHelp()
{
    return String();
}

BOOL DdeTopic::MakeItem( const String& )
{
    return FALSE;
}

BOOL DdeService::MakeTopic( const String& )
{
    return FALSE;
}

String DdeService::SysTopicGet( const String& )
{
    return String();
}

BOOL DdeService::SysTopicExecute( const String* )
{
    return FALSE;
}

