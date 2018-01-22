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

#include "ddeimp.hxx"
#include <algorithm>
#include <memory>
#include <comphelper/string.hxx>
#include <rtl/ustring.hxx>
#include <svl/svdde.hxx>
#include <osl/thread.h>
#include <o3tl/sorted_vector.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

enum DdeItemType
{
    DDEITEM,
    DDEGETPUTITEM
};

struct DdeItemImpData
{
    HCONV nHCnv;
    sal_uInt16 nCnt;

    explicit DdeItemImpData( HCONV nH ) : nHCnv( nH ), nCnt( 1 ) {}
};

HDDEDATA CALLBACK DdeInternal::SvrCallback(
            UINT nCode, UINT nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
            HDDEDATA hData, ULONG_PTR, ULONG_PTR )
{
    DdeServices&    rAll = DdeService::GetServices();
    DdeService*     pService;
    DdeTopic*       pTopic;
    DdeItem*        pItem;
    DdeData*        pData;
    Conversation*   pC;

    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);

    switch( nCode )
    {
        case XTYP_WILDCONNECT:
        {
            int nTopics = 0;

            WCHAR chTopicBuf[250];
            if( hText1 )
                DdeQueryStringW( pInst->hDdeInstSvr, hText1, chTopicBuf,
                                SAL_N_ELEMENTS(chTopicBuf), CP_WINUNICODE );

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
                                if( s == o3tl::toU(chTopicBuf) )
                                    ++nTopics;
                            }
                        }
                        else
                            nTopics += comphelper::string::getTokenCount(sTopics, '\t');
                    }
                }
            }

            if( !nTopics )
                return nullptr;

            auto pPairs = std::unique_ptr<HSZPAIR[]>(new HSZPAIR [nTopics + 1]);

            HSZPAIR* q = pPairs.get();
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
                        s = s.replaceAll("\n", "").replaceAll("\r", "");
                        if( !hText1 || s == o3tl::toU(chTopicBuf) )
                        {
                            DdeString aDStr( pInst->hDdeInstSvr, s );
                            pTopic = FindTopic( *pService, aDStr.getHSZ() );
                            if( pTopic )
                            {
                                q->hszSvc   = pService->pName->getHSZ();
                                q->hszTopic = pTopic->pName->getHSZ();
                                q++;
                            }
                        }
                    }
                }
            }

            q->hszSvc   = nullptr;
            q->hszTopic = nullptr;
            HDDEDATA h = DdeCreateDataHandle(
                            pInst->hDdeInstSvr,
                            reinterpret_cast<LPBYTE>(pPairs.get()),
                            sizeof(HSZPAIR) * (nTopics+1),
                            0, nullptr, nCbType, 0);
            return h;
        }

        case XTYP_CONNECT:
            pService = FindService( hText2 );
            if ( pService)
                pTopic = FindTopic( *pService, hText1 );
            else
                pTopic = nullptr;
            if ( pTopic )
                return reinterpret_cast<HDDEDATA>(DDE_FACK);
            else
                return nullptr;

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
                    pService->m_vConv.emplace_back( pC );
                }
            }
            return nullptr;
    }

    for (DdeServices::iterator aI = rAll.begin(); aI != rAll.end(); ++aI)
    {
        pService = *aI;
        for ( size_t i = 0, n = pService->m_vConv.size(); i < n; ++i )
        {
            pC = pService->m_vConv[ i ].get();
            if ( pC->hConv == hConv )
                goto found;
        }
    }

    return reinterpret_cast<HDDEDATA>(DDE_FNOTPROCESSED);

found:
    if ( nCode == XTYP_DISCONNECT)
    {
        DisconnectTopic(*pC->pTopic, hConv);
        for ( ConvList::iterator it = pService->m_vConv.begin();
              it != pService->m_vConv.end();
              ++it
        ) {
            if ( it->get() == pC )
            {
                pService->m_vConv.erase( it );
                break;
            }
        }
        return nullptr;
    }

    bool bExec = nCode == XTYP_EXECUTE;
    pTopic = pC->pTopic;
    if ( pTopic && !bExec )
        pItem = FindItem( *pTopic, hText2 );
    else
        pItem = nullptr;

    if ( !bExec && !pService->HasCbFormat( nCbType ) )
        pItem = nullptr;
    if ( !pItem && !bExec )
        return static_cast<HDDEDATA>(DDE_FNOTPROCESSED);
    if ( pItem )
        pTopic->aItem = pItem->GetName();
    else
        pTopic->aItem.clear();

    bool bRes = false;
    switch( nCode )
    {
    case XTYP_REQUEST:
    case XTYP_ADVREQ:
        {
            OUString aRes; // Must be free not until the end!
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
                else if ( pTopic->aItem == SZDDESYS_ITEM_HELP )
                    aRes = OUString();
                else
                    aRes = OUString();

                if ( !aRes.isEmpty() )
                    pData = new DdeData( aRes );
                else
                    pData = nullptr;
            }
            else if( DDEGETPUTITEM == pItem->nType )
            {
                pData = static_cast<DdeGetPutItem*>(pItem)->Get( DdeData::GetInternalFormat( nCbType ) );
            }
            else
            {
                pData = pTopic->Get( DdeData::GetInternalFormat( nCbType ));
            }

            if ( pData )
            {
                return DdeCreateDataHandle( pInst->hDdeInstSvr,
                                            static_cast<LPBYTE>(const_cast<void *>(pData->xImp->pData)),
                                            pData->xImp->nData,
                                            0, hText2,
                                            DdeData::GetExternalFormat(
                                                pData->xImp->nFmt ),
                                            0 );
            }
        }
        break;

    case XTYP_POKE:
        if ( !pTopic->IsSystemTopic() )
        {
            DdeData d;
            d.xImp->hData = hData;
            d.xImp->nFmt  = DdeData::GetInternalFormat( nCbType );
            d.Lock();
            if( DDEGETPUTITEM == pItem->nType )
                bRes = static_cast<DdeGetPutItem*>(pItem)->Put( &d );
            else
                bRes = pTopic->Put( &d );
        }
        if ( bRes )
            return reinterpret_cast<HDDEDATA>(DDE_FACK);
        else
            return reinterpret_cast<HDDEDATA>(DDE_FNOTPROCESSED);

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
                        pItem = nullptr;
                        break;
                    }
                }

                if( pItem )
                    // It was not exchange, so back in
                    pTopic->aItems.push_back(pItem);
                else
                    pItem = iter != pTopic->aItems.end() ? *iter : nullptr;
            }

            if (pItem)
            {
                IncMonitor(pItem, hConv);
            }
        }
        return reinterpret_cast<HDDEDATA>(TRUE);

    case XTYP_ADVSTOP:
        DecMonitor(pItem, hConv);
        return reinterpret_cast<HDDEDATA>(TRUE);

    case XTYP_EXECUTE:
        {
            DdeData aExec;
            aExec.xImp->hData = hData;
            aExec.xImp->nFmt  = DdeData::GetInternalFormat( nCbType );
            aExec.Lock();
            OUString aName;

            aName = static_cast<const sal_Unicode *>(aExec.xImp->pData);

            if( pTopic->IsSystemTopic() )
                bRes = false;
            else
                bRes = pTopic->Execute( &aName );
        }
        if ( bRes )
            return reinterpret_cast<HDDEDATA>(DDE_FACK);
        else
            return reinterpret_cast<HDDEDATA>(DDE_FNOTPROCESSED);
    }

    return nullptr;
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

    return nullptr;
}

DdeTopic* DdeInternal::FindTopic( DdeService& rService, HSZ hTopic )
{
    std::vector<DdeTopic*>::iterator iter;
    std::vector<DdeTopic*> &rTopics = rService.aTopics;
    bool bContinue = false;
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);

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
        WCHAR chBuf[250];
        DdeQueryStringW(pInst->hDdeInstSvr,hTopic,chBuf,SAL_N_ELEMENTS(chBuf),CP_WINUNICODE );
        bContinue = false;
        // We need to search again
    }
    while( bContinue );

    return nullptr;
}

DdeItem* DdeInternal::FindItem( DdeTopic& rTopic, HSZ hItem )
{
    std::vector<DdeItem*>::iterator iter;
    std::vector<DdeItem*> &rItems = rTopic.aItems;
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
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
        WCHAR chBuf[250];
        DdeQueryStringW(pInst->hDdeInstSvr,hItem,chBuf,SAL_N_ELEMENTS(chBuf),CP_WINUNICODE );
        bContinue = rTopic.MakeItem( o3tl::toU(chBuf) );
        // We need to search again
    }
    while( bContinue );

    return nullptr;
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
            DdeInitializeW( &pInst->hDdeInstSvr,
                            DdeInternal::SvrCallback,
                            APPCLASS_STANDARD |
                            CBF_SKIP_REGISTRATIONS |
                            CBF_SKIP_UNREGISTRATIONS, 0L ) );
        pInst->pServicesSvr = new DdeServices;
    }
    else
        nStatus = DMLERR_NO_ERROR;

    if ( pInst->pServicesSvr )
        pInst->pServicesSvr->push_back( this );

    pName = new DdeString( pInst->hDdeInstSvr, rService );
    if ( nStatus == DMLERR_NO_ERROR )
    {
        if ( !DdeNameService( pInst->hDdeInstSvr, pName->getHSZ(), nullptr,
                              DNS_REGISTER | DNS_FILTEROFF ) )
        {
            nStatus = DMLERR_SYS_ERROR;
        }
    }
    AddFormat( SotClipboardFormatId::STRING );
    pSysTopic = new DdeTopic( SZDDESYS_TOPIC );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_TOPICS ) );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_SYSITEMS ) );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_STATUS ) );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_FORMATS ) );
    pSysTopic->AddItem( DdeItem( SZDDESYS_ITEM_HELP ) );
    AddTopic( *pSysTopic );
}

DdeService::~DdeService()
{
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
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
            pInst->pServicesSvr = nullptr;
            if( pInst->nRefCount == 0)
                ImpDeinitInstData();
        }
    }
}

const OUString DdeService::GetName() const
{
    return pName->toOUString();
}

DdeServices& DdeService::GetServices()
{
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
    return *(pInst->pServicesSvr);
}

void DdeService::AddTopic( const DdeTopic& rTopic )
{
    RemoveTopic( rTopic );
    aTopics.push_back(const_cast<DdeTopic *>(&rTopic));
}

void DdeService::RemoveTopic( const DdeTopic& rTopic )
{
    std::vector<DdeTopic*>::iterator iter;
    for ( iter = aTopics.begin(); iter != aTopics.end(); ++iter )
    {
        if ( !DdeCmpStringHandles ((*iter)->pName->getHSZ(), rTopic.pName->getHSZ() ) )
        {
            aTopics.erase(iter);
            // Delete all conversions!
            // Or else we work on deleted topics!
            for( size_t n = m_vConv.size(); n; )
            {
                auto const& pC = m_vConv[ --n ];
                if( pC->pTopic == &rTopic )
                    m_vConv.erase( m_vConv.begin() + n );
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
    return HasCbFormat( static_cast<sal_uInt16>(DdeData::GetExternalFormat( nFmt )));
}

void DdeService::AddFormat(SotClipboardFormatId nFmt)
{
    sal_uLong nExternalFmt = DdeData::GetExternalFormat( nFmt );
    for ( size_t i = 0, n = aFormats.size(); i < n; ++i )
        if ( static_cast<sal_uLong>(aFormats[ i ]) == nExternalFmt )
            return;
    aFormats.push_back( nExternalFmt );
}

void DdeService::RemoveFormat(SotClipboardFormatId nFmt)
{
    sal_uLong nExternalFmt = DdeData::GetExternalFormat( nFmt );
    for ( DdeFormats::iterator it = aFormats.begin(); it != aFormats.end(); ++it )
    {
        if ( static_cast<sal_uLong>(*it) == nExternalFmt )
        {
            aFormats.erase( it );
            break;
        }
    }
}

DdeTopic::DdeTopic( const OUString& rName )
{
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
    pName = new DdeString( pInst->hDdeInstSvr, rName );
}

DdeTopic::~DdeTopic()
{
    std::vector<DdeItem*>::iterator iter;
    for (iter = aItems.begin(); iter != aItems.end(); ++iter)
    {
        (*iter)->pMyTopic = nullptr;
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
    return GetName() == SZDDESYS_TOPIC;
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
        if ( !DdeCmpStringHandles ((*iter)->pName->getHSZ(), r.pName->getHSZ() ) )
            break;
    }

    if ( iter != aItems.end() )
    {
        (*iter)->pMyTopic = nullptr;
        delete *iter;
        aItems.erase(iter);
    }
}

void DdeTopic::NotifyClient( const OUString& rItem )
{
    std::vector<DdeItem*>::iterator iter;
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
    for ( iter = aItems.begin(); iter != aItems.end(); ++iter)
    {
        if ( (*iter)->GetName().equals(rItem) && (*iter)->pImpData)
        {
            DdePostAdvise( pInst->hDdeInstSvr, pName->getHSZ(), (*iter)->pName->getHSZ() );
            break;
        }
    }
}

void DdeInternal::DisconnectTopic(DdeTopic & rTopic, HCONV nId)
{
    std::vector<DdeItem*>::iterator iter;
    for (iter = rTopic.aItems.begin(); iter != rTopic.aItems.end(); ++iter)
    {
        DecMonitor(*iter, nId);
    }
}

DdeData* DdeTopic::Get(SotClipboardFormatId /*nFmt*/)
{
    return nullptr;
}

bool DdeTopic::Put( const DdeData* )
{
    return false;
}

bool DdeTopic::Execute( const OUString* )
{
    return false;
}

bool DdeTopic::StartAdviseLoop()
{
    return false;
}

DdeItem::DdeItem( const sal_Unicode* p )
{
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
    pName = new DdeString( pInst->hDdeInstSvr, p );
    nType = DDEITEM;
    pMyTopic = nullptr;
    pImpData = nullptr;
}

DdeItem::DdeItem( const OUString& r)
{
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
    pName = new DdeString( pInst->hDdeInstSvr, r );
    nType = DDEITEM;
    pMyTopic = nullptr;
    pImpData = nullptr;
}

DdeItem::DdeItem( const DdeItem& r)
{
    DdeInstData* pInst = ImpGetInstData();
    assert(pInst);
    pName = new DdeString( pInst->hDdeInstSvr, r.pName->toOUString() );
    nType = DDEITEM;
    pMyTopic = nullptr;
    pImpData = nullptr;
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
        assert(pInst);
        DdePostAdvise( pInst->hDdeInstSvr, pMyTopic->pName->getHSZ(), pName->getHSZ() );
    }
}

void DdeInternal::IncMonitor(DdeItem *const pItem, HCONV nHCnv)
{
    if (!pItem->pImpData)
    {
        pItem->pImpData = new std::vector<DdeItemImpData>;
        if (DDEGETPUTITEM == pItem->nType)
        {
            static_cast<DdeGetPutItem*>(pItem)->AdviseLoop( true );
        }
    }
    else
    {
        for (size_t n = pItem->pImpData->size(); n; )
        {
            if ((*pItem->pImpData)[ --n ].nHCnv == nHCnv)
            {
                ++(*pItem->pImpData)[ n ].nHCnv;
                return ;
            }
        }
    }

    pItem->pImpData->push_back( DdeItemImpData( nHCnv ) );
}

void DdeInternal::DecMonitor(DdeItem *const pItem, HCONV nHCnv)
{
    if (pItem->pImpData)
    {
        for( size_t n = 0; n < pItem->pImpData->size(); ++n )
        {
            DdeItemImpData* pData = &(*pItem->pImpData)[n];
            if( pData->nHCnv == nHCnv )
            {
                if( !pData->nCnt || !--pData->nCnt )
                {
                    if (1 < pItem->pImpData->size())
                    {
                        pItem->pImpData->erase(pItem->pImpData->begin() + n);
                    }
                    else
                    {
                        delete pItem->pImpData;
                        pItem->pImpData = nullptr;
                        if (DDEGETPUTITEM == pItem->nType)
                        {
                            static_cast<DdeGetPutItem*>(pItem)->AdviseLoop(false);
                        }
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
    return nullptr;
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
        if ( (*iter)->GetName() == SZDDESYS_TOPIC )
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

        switch( static_cast<sal_uInt16>(f) )
        {
        case CF_TEXT:
            s += "TEXT";
            break;
        case CF_BITMAP:
            s += "BITMAP";
            break;
        default:
            {
                WCHAR buf[128];
                GetClipboardFormatNameW( static_cast<UINT>(f), buf, SAL_N_ELEMENTS(buf) );
                s += o3tl::toU(buf);
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
