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
#include "ddeimp.hxx"
#include <algorithm>
#include <comphelper/string.hxx>
#include <svl/svdde.hxx>
#include <tools/debug.hxx>
#include <osl/thread.h>
#include <o3tl/sorted_vector.hxx>

enum DdeItemType
{
    DDEITEM,
    DDEGETPUTITEM
};

struct DdeItemImpData
{
    sal_uLong nHCnv;
    sal_uInt16 nCnt;

    DdeItemImpData( sal_uLong nH ) : nHCnv( nH ), nCnt( 1 ) {}
};

class DdeItemImp : public std::vector<DdeItemImpData> {};

// --- DdeInternat::SvrCallback() ----------------------------------

#ifdef WNT
HDDEDATA CALLBACK DdeInternal::SvrCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, DWORD, DWORD )
#else
#if defined( ICC )
HDDEDATA CALLBACK DdeInternal::SvrCallback(
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
            int nTopics = 0;

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
                            sal_uInt16 n = 0;
                            while( STRING_NOTFOUND != n )
                            {
                                String s( sTopics.GetToken( 0, '\t', n ));
                                if( s == reinterpret_cast<const sal_Unicode*>(chTopicBuf) )
                                    ++nTopics;
                            }
                        }
                        else
                            nTopics += comphelper::string::getTokenCount(sTopics, '\t');
                    }
                }
            }

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
                    String sTopics( pService->Topics() );
                    sal_uInt16 n = 0;
                    while( STRING_NOTFOUND != n )
                    {
                        String s( sTopics.GetToken( 0, '\t', n ));
                        s = comphelper::string::remove(s, '\n');
                        s = comphelper::string::remove(s, '\r');
                        if( !hText1 || s == reinterpret_cast<const sal_Unicode*>(chTopicBuf) )
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
                }
            }

            q->hszSvc   = NULL;
            q->hszTopic = NULL;
            HDDEDATA h = DdeCreateDataHandle(
                            pInst->hDdeInstSvr, (LPBYTE) pPairs,
                            sizeof(HSZPAIR) * (nTopics+1),
                            0, NULL, nCbType, 0);
            delete [] pPairs;
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
                    pService->pConv->push_back( pC );
                }
            }
            return (HDDEDATA)NULL;
    }

    for ( pService = rAll.First(); pService; pService = rAll.Next() )
    {
        for ( size_t i = 0, n = pService->pConv->size(); i < n; ++i )
        {
            pC = (*pService->pConv)[ i ];
            if ( pC->hConv == hConv )
                goto found;
        }
    }

    return (HDDEDATA) DDE_FNOTPROCESSED;

found:
    if ( nCode == XTYP_DISCONNECT)
    {
        pC->pTopic->_Disconnect( (long) hConv );
        for ( ConvList::iterator it = pService->pConv->begin();
              it != pService->pConv->end();
              ++it
        ) {
            if ( *it == pC )
            {
                delete *it;
                pService->pConv->erase( it );
                break;
            }
        }
        return (HDDEDATA)NULL;
    }

    sal_Bool bExec = sal_Bool(nCode == XTYP_EXECUTE);
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

    sal_Bool bRes = sal_False;
    pInst->hCurConvSvr = (long)hConv;
    switch( nCode )
    {
        case XTYP_REQUEST:
        case XTYP_ADVREQ:
            {
            String aRes;          // darf erst am Ende freigegeben werden!!
            if ( pTopic->IsSystemTopic() )
            {
                if ( pTopic->aItem == reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_TOPICS) )
                    aRes = pService->Topics();
                else if ( pTopic->aItem == reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_SYSITEMS) )
                    aRes = pService->SysItems();
                else if ( pTopic->aItem == reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_STATUS) )
                    aRes = pService->Status();
                else if ( pTopic->aItem == reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_FORMATS) )
                    aRes = pService->Formats();
                else if ( pTopic->aItem ==  reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_HELP) )
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
            pInst->hCurConvSvr = 0;
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
                    std::vector<DdeItem*>::iterator it(std::find(pTopic->aItems.begin(),
                                                                 pTopic->aItems.end(),
                                                                 pItem));
                    if (it != pTopic->aItems.end())
                        pTopic->aItems.erase(it);

                    std::vector<DdeItem*>::iterator iter;
                    for( iter = pTopic->aItems.begin();
                         iter != pTopic->aItems.end();
                         ++iter )
                    {
                        if( *(*iter)->pName == hText2 )
                        {
                            // es wurde tatsaechlich ausgewechselt
                            delete pItem;
                            pItem = 0;
                            break;
                        }
                    }

                    if( pItem )
                        // es wurde doch nicht ausgewechselt, also wieder rein
                        pTopic->aItems.push_back(pItem);
                    else
                        pItem = iter != pTopic->aItems.end() ? *iter : NULL;
                }

                if (pItem)
                {
                    pItem->IncMonitor( (long)hConv );
                    pInst->hCurConvSvr = 0;
                }
            }
            return (HDDEDATA)sal_True;

        case XTYP_ADVSTOP:
            pItem->DecMonitor( (long)hConv );
            if( !pItem->pImpData )
                pTopic->StopAdviseLoop();
            pInst->hCurConvSvr = 0;
            return (HDDEDATA)sal_True;

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
            pInst->hCurConvSvr = 0;
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
    std::vector<DdeTopic*>::iterator iter;
    std::vector<DdeTopic*> &rTopics = rService.aTopics;
    int bWeiter = sal_False;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");

    do {            // middle check loop
        for ( iter = rTopics.begin(); iter != rTopics.end(); ++iter )
        {
            if ( *(*iter)->pName == hTopic )
                return *iter;
        }

        bWeiter = !bWeiter;
        if( !bWeiter )
            break;

        // dann befragen wir doch mal unsere Ableitung:
        TCHAR chBuf[250];
        DdeQueryString(pInst->hDdeInstSvr,hTopic,chBuf,sizeof(chBuf)/sizeof(TCHAR),CP_WINUNICODE );
        bWeiter = rService.MakeTopic( reinterpret_cast<const sal_Unicode*>(chBuf) );
        // dann muessen wir noch mal suchen
    } while( bWeiter );

    return 0;
}

// --- DdeInternal::FindItem() -------------------------------------

DdeItem* DdeInternal::FindItem( DdeTopic& rTopic, HSZ hItem )
{
    std::vector<DdeItem*>::iterator iter;
    std::vector<DdeItem*> &rItems = rTopic.aItems;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    int bWeiter = sal_False;

    do {            // middle check loop

        for ( iter = rItems.begin(); iter != rItems.end(); ++iter )
            if ( *(*iter)->pName == hItem )
                return *iter;

        bWeiter = !bWeiter;
        if( !bWeiter )
            break;

        // dann befragen wir doch mal unsere Ableitung:
        TCHAR chBuf[250];
        DdeQueryString(pInst->hDdeInstSvr,hItem,chBuf,sizeof(chBuf)/sizeof(TCHAR),CP_WINUNICODE );
        bWeiter = rTopic.MakeItem( reinterpret_cast<const sal_Unicode*>(chBuf) );
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
        nStatus = sal::static_int_cast< short >(
            DdeInitialize( &pInst->hDdeInstSvr,
                           (PFNCALLBACK)DdeInternal::SvrCallback,
                           APPCLASS_STANDARD |
                           CBF_SKIP_REGISTRATIONS |
                           CBF_SKIP_UNREGISTRATIONS, 0L ) );
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
    pSysTopic = new DdeTopic( reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_TOPICS) ) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_SYSITEMS) ) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_STATUS) ) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_FORMATS) ) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_HELP) ) );
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

    delete pSysTopic;
    delete pName;

    pInst->nInstanceSvr--;
    pInst->nRefCount--;
    if ( !pInst->nInstanceSvr && pInst->hDdeInstSvr )
    {
        if( DdeUninitialize( pInst->hDdeInstSvr ) )
        {
            pInst->hDdeInstSvr = 0;
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
    aTopics.push_back((DdeTopic *) &rTopic);
}

// --- DdeService::RemoveTopic() -----------------------------------

void DdeService::RemoveTopic( const DdeTopic& rTopic )
{
    std::vector<DdeTopic*>::iterator iter;
    for ( iter = aTopics.begin(); iter != aTopics.end(); ++iter )
    {
        if ( !DdeCmpStringHandles (*(*iter)->pName, *rTopic.pName ) )
        {
            aTopics.erase(iter);
            // JP 27.07.95: und alle Conversions loeschen !!!
            //              (sonst wird auf geloeschten Topics gearbeitet!!)
            for( size_t n = pConv->size(); n; )
            {
                Conversation* pC = (*pConv)[ --n ];
                if( pC->pTopic == &rTopic )
                {
                    ConvList::iterator it = pConv->begin();
                    ::std::advance( it, n );
                    delete *it;
                    pConv->erase( it );
                }
            }
            break;
        }
    }
}

// --- DdeService::HasCbFormat() -----------------------------------

sal_Bool DdeService::HasCbFormat( sal_uInt16 nFmt )
{
    for ( size_t i = 0, n = aFormats.size(); i < n; ++i )
        if ( aFormats[ i ] == nFmt )
            return true;
    return false;
}

// --- DdeService::HasFormat() -------------------------------------

sal_Bool DdeService::HasFormat( sal_uLong nFmt )
{
    return HasCbFormat( (sal_uInt16)DdeData::GetExternalFormat( nFmt ));
}

// --- DdeService::AddFormat() -------------------------------------

void DdeService::AddFormat( sal_uLong nFmt )
{
    nFmt = DdeData::GetExternalFormat( nFmt );
    for ( size_t i = 0, n = aFormats.size(); i < n; ++i )
        if ( (sal_uLong) aFormats[ i ] == nFmt )
            return;
    aFormats.push_back( nFmt );
}

// --- DdeService::RemoveFormat() ----------------------------------

void DdeService::RemoveFormat( sal_uLong nFmt )
{
    nFmt = DdeData::GetExternalFormat( nFmt );
    for ( DdeFormats::iterator it = aFormats.begin(); it != aFormats.end(); ++it ) {
        if ( (sal_uLong) *it == nFmt ) {
            aFormats.erase( it );
            break;
        }
    }
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
    std::vector<DdeItem*>::iterator iter;
    for (iter = aItems.begin(); iter != aItems.end(); ++iter)
    {
        (*iter)->pMyTopic = 0;
        delete *iter;
    }

    delete pName;
}

// --- DdeTopic::GetName() -----------------------------------------

const String& DdeTopic::GetName() const
{
    return *pName;
}

// --- DdeTopic::IsSystemTopic() -----------------------------------

sal_Bool DdeTopic::IsSystemTopic()
{
    return sal_Bool (GetName() == reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC));
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
        aItems.push_back( s );
        s->pMyTopic = this;
    }
    return s;
}

// --- DdeTopic::InsertItem() -----------------------------------------

void DdeTopic::InsertItem( DdeItem* pNew )
{
    if( pNew )
    {
        aItems.push_back( pNew );
        pNew->pMyTopic = this;
    }
}

// --- DdeTopic::RemoveItem() --------------------------------------

void DdeTopic::RemoveItem( const DdeItem& r )
{
    std::vector<DdeItem*>::iterator iter;
    for (iter = aItems.begin(); iter != aItems.end(); ++iter)
    {
        if ( !DdeCmpStringHandles (*(*iter)->pName, *r.pName ) )
            break;
    }

    if ( iter != aItems.end() )
    {
        (*iter)->pMyTopic = 0;
        delete *iter;
        aItems.erase(iter);
    }
}

// --- DdeTopic::NotifyClient() ------------------------------------

void DdeTopic::NotifyClient( const String& rItem )
{
    std::vector<DdeItem*>::iterator iter;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    for ( iter = aItems.begin(); iter != aItems.end(); ++iter)
    {
        if ( (*iter)->GetName() == rItem && (*iter)->pImpData)
        {
            DdePostAdvise( pInst->hDdeInstSvr, *pName, *(*iter)->pName );
            break;
        }
    }
}

// --- DdeTopic::Connect() -----------------------------------------

void DdeTopic::Connect( long nId )
{
    aConnectLink.Call( (void*)nId );
}

// --- DdeTopic::Disconnect() --------------------------------------

void DdeTopic::Disconnect( long nId )
{
    aDisconnectLink.Call( (void*)nId );
}

// --- DdeTopic::_Disconnect() --------------------------------------

void DdeTopic::_Disconnect( long nId )
{
    std::vector<DdeItem*>::iterator iter;
    for (iter = aItems.begin(); iter != aItems.end(); ++iter)
        (*iter)->DecMonitor( nId );

    Disconnect( nId );
}

// --- DdeTopic::Get() ---------------------------------------------

DdeData* DdeTopic::Get( sal_uLong nFmt )
{
    if ( aGetLink.IsSet() )
        return (DdeData*)aGetLink.Call( (void*)nFmt );
    else
        return NULL;
}

// --- DdeTopic::Put() ---------------------------------------------

sal_Bool DdeTopic::Put( const DdeData* r )
{
    if ( aPutLink.IsSet() )
        return (sal_Bool)aPutLink.Call( (void*) r );
    else
        return sal_False;
}

// --- DdeTopic::Execute() -----------------------------------------

sal_Bool DdeTopic::Execute( const String* r )
{
    if ( aExecLink.IsSet() )
        return (sal_Bool)aExecLink.Call( (void*)r );
    else
        return sal_False;
}

// --- DdeTopic::GetConvId() ---------------------------------------

long DdeTopic::GetConvId()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    return pInst->hCurConvSvr;
}

// --- DdeTopic::StartAdviseLoop() ---------------------------------

sal_Bool DdeTopic::StartAdviseLoop()
{
    return sal_False;
}

// --- DdeTopic::StopAdviseLoop() ----------------------------------

sal_Bool DdeTopic::StopAdviseLoop()
{
    return sal_False;
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
        pMyTopic->aItems.erase(std::remove(pMyTopic->aItems.begin(),
                                           pMyTopic->aItems.end(),this));
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

void DdeItem::IncMonitor( sal_uLong nHCnv )
{
    if( !pImpData )
    {
        pImpData = new DdeItemImp;
        if( DDEGETPUTITEM == nType )
            ((DdeGetPutItem*)this)->AdviseLoop( sal_True );
    }
    else
    {
        for( sal_uInt16 n = pImpData->size(); n; )
            if( (*pImpData)[ --n ].nHCnv == nHCnv )
            {
                ++(*pImpData)[ n ].nHCnv;
                return ;
            }
    }

    pImpData->push_back( DdeItemImpData( nHCnv ) );
}

// --- DdeItem::DecMonitor() ------------------------------------------

void DdeItem::DecMonitor( sal_uLong nHCnv )
{
    if( pImpData )
    {
        for( sal_uInt16 n = 0; n < pImpData->size(); ++n )
        {
            DdeItemImpData* pData = &(*pImpData)[n];
            if( pData->nHCnv == nHCnv )
            {
                if( !pData->nCnt || !--pData->nCnt )
                {
                    if( 1 < pImpData->size() )
                    {
                        pImpData->erase(pImpData->begin() + n);
                    }
                    else
                    {
                        delete pImpData, pImpData = 0;
                        if( DDEGETPUTITEM == nType )
                            ((DdeGetPutItem*)this)->AdviseLoop( sal_False );
                    }
                }
                return ;
            }
        }
    }
}

// --- DdeItem::GetLinks() ------------------------------------------

short DdeItem::GetLinks()
{
    short nCnt = 0;
    if( pImpData )
        for( sal_uInt16 n = pImpData->size(); n; )
        {
            nCnt = nCnt + (*pImpData)[ --n ].nCnt;
        }
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

DdeData* DdeGetPutItem::Get( sal_uLong )
{
    return 0;
}

// --- DdeGetPutData::Put() ----------------------------------------

sal_Bool DdeGetPutItem::Put( const DdeData* )
{
    return sal_False;
}

// --- DdeGetPutData::AdviseLoop() ---------------------------------

void DdeGetPutItem::AdviseLoop( sal_Bool )
{
}


// --- DdeService::SysItems() --------------------------------------

String DdeService::SysItems()
{
    String s;
    std::vector<DdeTopic*>::iterator iter;
    std::vector<DdeItem*>::iterator iterItem;
    for ( iter = aTopics.begin(); iter != aTopics.end(); ++iter )
    {
        if ( (*iter)->GetName() == reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC) )
        {
            short n = 0;
            for ( iterItem = (*iter)->aItems.begin(); iterItem != (*iter)->aItems.end(); ++iterItem, n++ )
            {
                if ( n )
                    s += '\t';
                s += (*iterItem)->GetName();
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
    std::vector<DdeTopic*>::iterator iter;
    short       n = 0;

    for ( iter = aTopics.begin(); iter != aTopics.end(); ++iter, n++ )
    {
        if ( n )
            s += '\t';
        s += (*iter)->GetName();
    }
    s += String::CreateFromAscii("\r\n");

    return s;
}

// --- DdeService::Formats() ---------------------------------------

String DdeService::Formats()
{
    String      s;
    long        f;
    short       n = 0;

    for (size_t i = 0; i < aFormats.size(); ++i, ++n)
    {
        f = aFormats[ i ];
        if ( n )
            s += '\t';

        switch( (sal_uInt16)f )
        {
            case CF_TEXT:
                s += rtl::OUString("TEXT");
                break;
            case CF_BITMAP:
                s += rtl::OUString("BITMAP");
                break;
            default:
                {
                    TCHAR buf[128];
                    GetClipboardFormatName( (UINT)f, buf, sizeof(buf) / sizeof(TCHAR) );
                    s += rtl::OUString(buf);
                }
                break;
        }

    }
    s += rtl::OUString("\r\n");

    return s;
}

// --- DdeService::Status() ----------------------------------------

String DdeService::Status()
{
    return IsBusy() ? String::CreateFromAscii("Busy\r\n") : String::CreateFromAscii("Ready\r\n");
}

// --- DdeService::IsBusy() ----------------------------------------

sal_Bool DdeService::IsBusy()
{
    return sal_False;
}

// --- DdeService::GetHelp() ----------------------------------------

String DdeService::GetHelp()
{
    return String();
}

sal_Bool DdeTopic::MakeItem( const String& )
{
    return sal_False;
}

sal_Bool DdeService::MakeTopic( const String& )
{
    return sal_False;
}

String DdeService::SysTopicGet( const String& )
{
    return String();
}

sal_Bool DdeService::SysTopicExecute( const String* )
{
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
