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
#include <rtl/ustring.hxx>
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

    explicit DdeItemImpData( sal_uLong nH ) : nHCnv( nH ), nCnt( 1 ) {}
};

class DdeItemImp {
public:
    DdeItemImp() : mvData() {}

    size_t size() const { return mvData.size(); }

    std::vector<DdeItemImpData>::iterator begin() { return mvData.begin(); }

    void erase(std::vector<DdeItemImpData>::iterator it) { mvData.erase(it); }

    void push_back(const DdeItemImpData& rData) { mvData.push_back(rData); }

    DdeItemImpData& operator[](size_t i) { return mvData[i]; }

private:
    std::vector<DdeItemImpData> mvData;
};

HDDEDATA CALLBACK DdeInternal::SvrCallback(
            WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, DWORD, DWORD )
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

            for (DdeServices::iterator aI = rAll.begin(); aI != rAll.end(); ++aI)
            {
                pService = *aI;
                if ( !hText2 || ( *pService->pName == hText2 ) )
                {
                    OUString sTopics( pService->Topics() );
                    if (!sTopics.isEmpty())
                    {
                        if( hText1 )
                        {
                            sal_Int32 n = 0;
                            while( -1 != n )
                            {
                                OUString s( sTopics.getToken( 0, '\t', n ));
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

            HSZPAIR* q = pPairs;
            for (DdeServices::iterator aI = rAll.begin(); aI != rAll.end(); ++aI)
            {
                pService = *aI;
                if ( !hText2 || (*pService->pName == hText2 ) )
                {
                    OUString sTopics( pService->Topics() );
                    sal_Int32 n = 0;
                    while( -1 != n )
                    {
                        OUString s( sTopics.getToken( 0, '\t', n ));
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
                    pC = new Conversation;
                    pC->hConv = hConv;
                    pC->pTopic = pTopic;
                    pService->pConv->push_back( pC );
                }
            }
            return (HDDEDATA)NULL;
    }

    for (DdeServices::iterator aI = rAll.begin(); aI != rAll.end(); ++aI)
    {
        pService = *aI;
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
        pC->pTopic->Disconnect( (sal_IntPtr) hConv );
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

    bool bExec = nCode == XTYP_EXECUTE;
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
        (pTopic->aItem).clear();

    bool bRes = false;
    pInst->hCurConvSvr = (sal_IntPtr)hConv;
    switch( nCode )
    {
    case XTYP_REQUEST:
    case XTYP_ADVREQ:
        {
            OUString aRes; // Must be free not until the end!
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
                    aRes = OUString();
                else
                    aRes = OUString();

                if ( !aRes.isEmpty() )
                    pData = new DdeData( aRes );
                else
                    pData = NULL;
            }
            else if( DDEGETPUTITEM == pItem->nType )
            {
                pData = ((DdeGetPutItem*)pItem)->Get( DdeData::GetInternalFormat( nCbType ) );
            }
            else
            {
                pData = pTopic->Get( DdeData::GetInternalFormat( nCbType ));
            }

            if ( pData )
            {
                return DdeCreateDataHandle( pInst->hDdeInstSvr,
                                            (LPBYTE)pData->pImp->pData,
                                            pData->pImp->nData,
                                            0, hText2,
                                            DdeData::GetExternalFormat(
                                                pData->pImp->nFmt ),
                                            0 );
            }
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
            // Is the Item turning into a HotLink for the first time?
            if( !pItem->pImpData && pTopic->StartAdviseLoop() )
            {
                // Then the Item has been exchanged
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
                        // It was exchanged indeed
                        delete pItem;
                        pItem = 0;
                        break;
                    }
                }

                if( pItem )
                    // It was not exchange, so back in
                    pTopic->aItems.push_back(pItem);
                else
                    pItem = iter != pTopic->aItems.end() ? *iter : NULL;
            }

            if (pItem)
            {
                pItem->IncMonitor( (sal_IntPtr)hConv );
                pInst->hCurConvSvr = 0;
            }
        }
        return (HDDEDATA)sal_True;

    case XTYP_ADVSTOP:
        pItem->DecMonitor( (sal_IntPtr)hConv );
        pInst->hCurConvSvr = 0;
        return (HDDEDATA)sal_True;

    case XTYP_EXECUTE:
        {
            DdeData aExec;
            aExec.pImp->hData = hData;
            aExec.pImp->nFmt  = DdeData::GetInternalFormat( nCbType );
            aExec.Lock();
            OUString aName;

            aName = (const sal_Unicode *)aExec.pImp->pData;

            if( pTopic->IsSystemTopic() )
                bRes = false;
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

DdeService* DdeInternal::FindService( HSZ hService )
{
    DdeServices& rSvc = DdeService::GetServices();
    for (DdeServices::iterator aI = rSvc.begin(); aI != rSvc.end(); ++aI)
    {
        DdeService*  s = *aI;
        if ( *s->pName == hService )
            return s;
    }

    return NULL;
}

DdeTopic* DdeInternal::FindTopic( DdeService& rService, HSZ hTopic )
{
    std::vector<DdeTopic*>::iterator iter;
    std::vector<DdeTopic*> &rTopics = rService.aTopics;
    bool bContinue = false;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");

    do
    {   // middle check loop
        for ( iter = rTopics.begin(); iter != rTopics.end(); ++iter )
        {
            if ( *(*iter)->pName == hTopic )
                return *iter;
        }

        bContinue = !bContinue;
        if( !bContinue )
            break;

        // Let's query our subclass
        TCHAR chBuf[250];
        DdeQueryString(pInst->hDdeInstSvr,hTopic,chBuf,sizeof(chBuf)/sizeof(TCHAR),CP_WINUNICODE );
        bContinue = false;
        // We need to search again
    }
    while( bContinue );

    return 0;
}

DdeItem* DdeInternal::FindItem( DdeTopic& rTopic, HSZ hItem )
{
    std::vector<DdeItem*>::iterator iter;
    std::vector<DdeItem*> &rItems = rTopic.aItems;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    bool bContinue = false;

    do
    {   // middle check loop
        for ( iter = rItems.begin(); iter != rItems.end(); ++iter )
        {
            if ( *(*iter)->pName == hItem )
                return *iter;
        }
        bContinue = !bContinue;
        if( !bContinue )
            break;

        // Let's query our subclass
        TCHAR chBuf[250];
        DdeQueryString(pInst->hDdeInstSvr,hItem,chBuf,sizeof(chBuf)/sizeof(TCHAR),CP_WINUNICODE );
        bContinue = rTopic.MakeItem( reinterpret_cast<const sal_Unicode*>(chBuf) );
        // We need to search again
    }
    while( bContinue );

    return 0;
}

DdeService::DdeService( const OUString& rService )
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
        pInst->pServicesSvr->push_back( this );

    pName = new DdeString( pInst->hDdeInstSvr, rService );
    if ( nStatus == DMLERR_NO_ERROR )
    {
        if ( !DdeNameService( pInst->hDdeInstSvr, *pName, NULL,
                              DNS_REGISTER | DNS_FILTEROFF ) )
        {
            nStatus = DMLERR_SYS_ERROR;
        }
    }
    AddFormat( SotClipboardFormatId::STRING );
    pSysTopic = new DdeTopic( reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_TOPICS) ) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_SYSITEMS) ) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_STATUS) ) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_FORMATS) ) );
    pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_HELP) ) );
    AddTopic( *pSysTopic );
}

DdeService::~DdeService()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    if ( pInst->pServicesSvr )
        pInst->pServicesSvr->erase(std::remove(pInst->pServicesSvr->begin(), pInst->pServicesSvr->end(), this), pInst->pServicesSvr->end());

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

const OUString DdeService::GetName() const
{
    return pName->toOUString();
}

DdeServices& DdeService::GetServices()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    return *(pInst->pServicesSvr);
}

void DdeService::AddTopic( const DdeTopic& rTopic )
{
    RemoveTopic( rTopic );
    aTopics.push_back((DdeTopic *) &rTopic);
}

void DdeService::RemoveTopic( const DdeTopic& rTopic )
{
    std::vector<DdeTopic*>::iterator iter;
    for ( iter = aTopics.begin(); iter != aTopics.end(); ++iter )
    {
        if ( !DdeCmpStringHandles (*(*iter)->pName, *rTopic.pName ) )
        {
            aTopics.erase(iter);
            // Delete all conversions!
            // Or else we work on deleted topics!
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

bool DdeService::HasCbFormat( sal_uInt16 nFmt )
{
    for ( size_t i = 0, n = aFormats.size(); i < n; ++i )
        if ( aFormats[ i ] == nFmt )
            return true;
    return false;
}

bool DdeService::HasFormat(SotClipboardFormatId nFmt)
{
    return HasCbFormat( (sal_uInt16)DdeData::GetExternalFormat( nFmt ));
}

void DdeService::AddFormat(SotClipboardFormatId nFmt)
{
    sal_uLong nExternalFmt = DdeData::GetExternalFormat( nFmt );
    for ( size_t i = 0, n = aFormats.size(); i < n; ++i )
        if ( (sal_uLong) aFormats[ i ] == nExternalFmt )
            return;
    aFormats.push_back( nExternalFmt );
}

void DdeService::RemoveFormat(SotClipboardFormatId nFmt)
{
    sal_uLong nExternalFmt = DdeData::GetExternalFormat( nFmt );
    for ( DdeFormats::iterator it = aFormats.begin(); it != aFormats.end(); ++it )
    {
        if ( (sal_uLong) *it == nExternalFmt )
        {
            aFormats.erase( it );
            break;
        }
    }
}

DdeTopic::DdeTopic( const OUString& rName )
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    pName = new DdeString( pInst->hDdeInstSvr, rName );
}

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

const OUString DdeTopic::GetName() const
{
    return pName->toOUString();
}

bool DdeTopic::IsSystemTopic()
{
    return GetName() == reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC);
}

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

void DdeTopic::InsertItem( DdeItem* pNew )
{
    if( pNew )
    {
        aItems.push_back( pNew );
        pNew->pMyTopic = this;
    }
}

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

void DdeTopic::NotifyClient( const OUString& rItem )
{
    std::vector<DdeItem*>::iterator iter;
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    for ( iter = aItems.begin(); iter != aItems.end(); ++iter)
    {
        if ( (*iter)->GetName().equals(rItem) && (*iter)->pImpData)
        {
            DdePostAdvise( pInst->hDdeInstSvr, *pName, *(*iter)->pName );
            break;
        }
    }
}

void DdeTopic::Disconnect( sal_IntPtr nId )
{
    std::vector<DdeItem*>::iterator iter;
    for (iter = aItems.begin(); iter != aItems.end(); ++iter)
        (*iter)->DecMonitor( nId );
}

DdeData* DdeTopic::Get(SotClipboardFormatId /*nFmt*/)
{
    return NULL;
}

bool DdeTopic::Put( const DdeData* )
{
    return false;
}

bool DdeTopic::Execute( const OUString* )
{
    return false;
}

long DdeTopic::GetConvId()
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    return pInst->hCurConvSvr;
}

bool DdeTopic::StartAdviseLoop()
{
    return false;
}

DdeItem::DdeItem( const sal_Unicode* p )
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    pName = new DdeString( pInst->hDdeInstSvr, p );
    nType = DDEITEM;
    pMyTopic = 0;
    pImpData = 0;
}

DdeItem::DdeItem( const OUString& r)
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    pName = new DdeString( pInst->hDdeInstSvr, r );
    nType = DDEITEM;
    pMyTopic = 0;
    pImpData = 0;
}

DdeItem::DdeItem( const DdeItem& r)
{
    DdeInstData* pInst = ImpGetInstData();
    DBG_ASSERT(pInst,"SVDDE:No instance data");
    pName = new DdeString( pInst->hDdeInstSvr, r.pName->toOUString() );
    nType = DDEITEM;
    pMyTopic = 0;
    pImpData = 0;
}

DdeItem::~DdeItem()
{
    if( pMyTopic )
        pMyTopic->aItems.erase(std::remove(pMyTopic->aItems.begin(),
                                           pMyTopic->aItems.end(),this));
    delete pName;
    delete pImpData;
}

const OUString DdeItem::GetName() const
{
    return pName->toOUString();
}

void DdeItem::NotifyClient()
{
    if( pMyTopic && pImpData )
    {
        DdeInstData* pInst = ImpGetInstData();
        DBG_ASSERT(pInst,"SVDDE:No instance data");
        DdePostAdvise( pInst->hDdeInstSvr, *pMyTopic->pName, *pName );
    }
}

void DdeItem::IncMonitor( sal_uLong nHCnv )
{
    if( !pImpData )
    {
        pImpData = new DdeItemImp;
        if( DDEGETPUTITEM == nType )
            ((DdeGetPutItem*)this)->AdviseLoop( true );
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
                            ((DdeGetPutItem*)this)->AdviseLoop( false );
                    }
                }
                return ;
            }
        }
    }
}

short DdeItem::GetLinks()
{
    short nCnt = 0;
    if( pImpData )
    {
        for( sal_uInt16 n = pImpData->size(); n; )
        {
            nCnt = nCnt + (*pImpData)[ --n ].nCnt;
        }
    }
    return nCnt;
}

DdeGetPutItem::DdeGetPutItem( const sal_Unicode* p )
    : DdeItem( p )
{
    nType = DDEGETPUTITEM;
}

DdeGetPutItem::DdeGetPutItem( const OUString& rStr )
    : DdeItem( rStr )
{
    nType = DDEGETPUTITEM;
}

DdeGetPutItem::DdeGetPutItem( const DdeItem& rItem )
    : DdeItem( rItem )
{
    nType = DDEGETPUTITEM;
}

DdeData* DdeGetPutItem::Get(SotClipboardFormatId)
{
    return 0;
}

bool DdeGetPutItem::Put( const DdeData* )
{
    return false;
}

void DdeGetPutItem::AdviseLoop( bool )
{
}

OUString DdeService::SysItems()
{
    OUString s;
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
                    s += "\t";
                s += (*iterItem)->GetName();
            }
            s += "\r\n";
        }
    }

    return s;
}

OUString DdeService::Topics()
{
    OUString    s;
    std::vector<DdeTopic*>::iterator iter;
    short       n = 0;

    for ( iter = aTopics.begin(); iter != aTopics.end(); ++iter, n++ )
    {
        if ( n )
            s += "\t";
        s += (*iter)->GetName();
    }
    s += "\r\n";

    return s;
}

OUString DdeService::Formats()
{
    OUString    s;
    short       n = 0;

    for (size_t i = 0; i < aFormats.size(); ++i, ++n)
    {
        long f = aFormats[ i ];
        if ( n )
            s += "\t";

        switch( (sal_uInt16)f )
        {
        case CF_TEXT:
            s += "TEXT";
            break;
        case CF_BITMAP:
            s += "BITMAP";
            break;
        default:
            {
                TCHAR buf[128];
                GetClipboardFormatName( (UINT)f, buf, sizeof(buf) / sizeof(TCHAR) );
                s += OUString(reinterpret_cast<sal_Unicode*>(buf));
            }
            break;
        }

    }
    s += "\r\n";

    return s;
}

OUString DdeService::Status()
{
    return OUString("Ready\r\n");
}

bool DdeTopic::MakeItem( const OUString& )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
