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

#include <sal/config.h>

#include <iomanip>

#include <sal/log.hxx>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/eitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <comphelper/processfactory.hxx>
#include "itemdel.hxx"

//Includes below due to nInReschedule
#include "appdata.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/msg.hxx>
#include "statcach.hxx"
#include <sfx2/ctrlitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include "sfxtypes.hxx"
#include "workwin.hxx"
#include <sfx2/unoctitm.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxuno.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/msgpool.hxx>

#include <com/sun/star/frame/XModuleManager.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

static sal_uInt16 nTimeOut = 300;

#define TIMEOUT_FIRST       nTimeOut
#define TIMEOUT_UPDATING     20

typedef std::unordered_map< sal_uInt16, bool > InvalidateSlotMap;


typedef std::vector<SfxStateCache*> SfxStateCacheArr_Impl;

struct SfxFoundCache_Impl
{
    sal_uInt16      nWhichId;  // If available: Which-Id, else: nSlotId
    const SfxSlot*  pSlot;     // Pointer to <Master-Slot>
    SfxStateCache*  pCache;    // Pointer to StatusCache, if possible NULL

    SfxFoundCache_Impl(sal_uInt16 nW, const SfxSlot *pS, SfxStateCache *pC ):
        nWhichId(nW),
        pSlot(pS),
        pCache(pC)
    {}
};

class SfxFoundCacheArr_Impl
{
    typedef std::vector<std::unique_ptr<SfxFoundCache_Impl> > DataType;
    DataType maData;

public:

    SfxFoundCache_Impl& operator[] ( size_t i )
    {
        return *maData[i].get();
    }

    size_t size() const
    {
        return maData.size();
    }

    void push_back( SfxFoundCache_Impl* p )
    {
        maData.push_back(std::unique_ptr<SfxFoundCache_Impl>(p));
    }
};

enum class SfxPopupAction
{
    DELETE,
    HIDE,
    SHOW
};


class SfxBindings_Impl
{
public:
    css::uno::Reference< css::frame::XDispatchRecorder > xRecorder;
    css::uno::Reference< css::frame::XDispatchProvider >  xProv;
    SfxWorkWindow*          pWorkWin;
    SfxBindings*            pSubBindings;
    SfxBindings*            pSuperBindings;
    SfxStateCacheArr_Impl*  pCaches;        // One chache for each binding
    sal_uInt16              nCachedFunc1;   // index for the last one called
    sal_uInt16              nCachedFunc2;   // index for the second last called
    sal_uInt16              nMsgPos;        // Message-Position relative the one to be updated
    SfxPopupAction          ePopupAction;   // Checked in DeleteFloatinWindow()
    bool                    bContextChanged;
    bool                    bMsgDirty;      // Has a MessageServer been invalidated?
    bool                    bAllMsgDirty;   //  Has a MessageServer been invalidated?
    bool                    bAllDirty;      // After InvalidateAll
    bool                    bCtrlReleased;  // while EnterRegistrations
    AutoTimer               aAutoTimer;     // for volatile Slots
    bool                    bInUpdate;      // for Assertions
    bool                    bInNextJob;     // for Assertions
    bool                    bFirstRound;    // First round in Update
    sal_uInt16              nFirstShell;    // Shell, the first round is preferred
    sal_uInt16              nOwnRegLevel;   // Counts the real Locks, except those of the Super Bindings
    InvalidateSlotMap       m_aInvalidateSlots; // store slots which are invalidated while in update
};

SfxBindings::SfxBindings()
:   pImpl(new SfxBindings_Impl),
    pDispatcher(nullptr),
    nRegLevel(1)    // first becomes 0, when the Dispatcher is set

{
    pImpl->nMsgPos = 0;
    pImpl->bAllMsgDirty = true;
    pImpl->bContextChanged = false;
    pImpl->bMsgDirty = true;
    pImpl->bAllDirty = true;
    pImpl->ePopupAction = SfxPopupAction::DELETE;
    pImpl->nCachedFunc1 = 0;
    pImpl->nCachedFunc2 = 0;
    pImpl->bCtrlReleased = false;
    pImpl->bFirstRound = false;
    pImpl->bInNextJob = false;
    pImpl->bInUpdate = false;
    pImpl->pSubBindings = nullptr;
    pImpl->pSuperBindings = nullptr;
    pImpl->pWorkWin = nullptr;
    pImpl->nOwnRegLevel = nRegLevel;

    // all caches are valid (no pending invalidate-job)
    // create the list of caches
    pImpl->pCaches = new SfxStateCacheArr_Impl;
    pImpl->aAutoTimer.SetTimeoutHdl( LINK(this, SfxBindings, NextJob) );
    pImpl->aAutoTimer.SetDebugName( "sfx::SfxBindings aAutoTimer" );
}


SfxBindings::~SfxBindings()

/*  [Description]

    Destructor of the SfxBindings class. The one, for each <SfxApplication>
    existing Instance is automatically destroyed by the <SfxApplication>
    after the execution of <SfxApplication::Exit()>.

    The still existing <SfxControllerItem> instances, which are registered
    by the SfxBindings instance, are automatically destroyed in the Destructor.
    These are usually the Floating-Toolboxen, Value-Sets
    etc. Arrays of SfxControllerItems may at this time no longer exist.
*/

{
    // The SubBindings should not be locked!
    pImpl->pSubBindings = nullptr;

    ENTERREGISTRATIONS();

    pImpl->aAutoTimer.Stop();
    DeleteControllers_Impl();

    // Delete Caches
    for(SfxStateCacheArr_Impl::const_iterator it = pImpl->pCaches->begin(); it != pImpl->pCaches->end(); ++it)
        delete *it;

    DELETEZ( pImpl->pWorkWin );

    delete pImpl->pCaches;
}


void SfxBindings::DeleteControllers_Impl()
{
    // in the first round delete SfxPopupWindows
    sal_uInt16 nCount = pImpl->pCaches->size();
    sal_uInt16 nCache;
    for ( nCache = 0; nCache < nCount; ++nCache )
    {
        // Remember were you are
        SfxStateCache *pCache = (*pImpl->pCaches)[nCache];
        sal_uInt16 nSlotId = pCache->GetId();

        // Re-align, because the cache may have been reduced
        sal_uInt16 nNewCount = pImpl->pCaches->size();
        if ( nNewCount < nCount )
        {
            nCache = GetSlotPos(nSlotId);
            if ( nCache >= nNewCount ||
                 nSlotId != (*pImpl->pCaches)[nCache]->GetId() )
                --nCache;
            nCount = nNewCount;
        }
    }

    // Delete all Caches
    for ( nCache = pImpl->pCaches->size(); nCache > 0; --nCache )
    {
        // Get Cache via css::sdbcx::Index
        SfxStateCache *pCache = (*pImpl->pCaches)[ nCache-1 ];

        // unbind all controllers in the cache
        SfxControllerItem *pNext;
        for ( SfxControllerItem *pCtrl = pCache->GetItemLink();
              pCtrl; pCtrl = pNext )
        {
            pNext = pCtrl->GetItemLink();
            pCtrl->UnBind();
        }

        if ( pCache->GetInternalController() )
            pCache->GetInternalController()->UnBind();

        // Delete Cache
        if( nCache-1 < (sal_uInt16) pImpl->pCaches->size() )
            delete (*pImpl->pCaches)[nCache-1];
        pImpl->pCaches->erase(pImpl->pCaches->begin()+ nCache - 1);
    }
}


void SfxBindings::HidePopups( bool bHide )
{
    // Hide SfxPopupWindows
    HidePopupCtrls_Impl( bHide );
    SfxBindings *pSub = pImpl->pSubBindings;
    while ( pSub )
    {
        pImpl->pSubBindings->HidePopupCtrls_Impl( bHide );
        pSub = pSub->pImpl->pSubBindings;
    }

    // Hide SfxChildWindows
    DBG_ASSERT( pDispatcher, "HidePopups not allowed without dispatcher" );
    if ( pImpl->pWorkWin )
        pImpl->pWorkWin->HidePopups_Impl( bHide, true );
}

void SfxBindings::HidePopupCtrls_Impl( bool bHide )
{
    if ( bHide )
    {
        // Hide SfxPopupWindows
        pImpl->ePopupAction = SfxPopupAction::HIDE;
    }
    else
    {
        // Show SfxPopupWindows
        pImpl->ePopupAction = SfxPopupAction::SHOW;
    }

    pImpl->ePopupAction = SfxPopupAction::DELETE;
}


void SfxBindings::Update_Impl
(
    SfxStateCache*  pCache      // The up to date SfxStatusCache
)
{
    if( pCache->GetDispatch().is() && pCache->GetItemLink() )
    {
        pCache->SetCachedState(true);
        if ( !pCache->GetInternalController() )
            return;
    }

    if ( !pDispatcher )
        return;

    // gather together all with the same status method which are dirty
    SfxDispatcher &rDispat = *pDispatcher;
    const SfxSlot *pRealSlot = nullptr;
    const SfxSlotServer* pMsgServer = nullptr;
    SfxFoundCacheArr_Impl aFound;
    SfxItemSet *pSet = CreateSet_Impl( pCache, pRealSlot, &pMsgServer, aFound );
    bool bUpdated = false;
    if ( pSet )
    {
        // Query Status
        if ( rDispat.FillState_( *pMsgServer, *pSet, pRealSlot ) )
        {
            // Post Status
            const SfxInterface *pInterface =
                rDispat.GetShell(pMsgServer->GetShellLevel())->GetInterface();
            for ( size_t nPos = 0; nPos < aFound.size(); ++nPos )
            {
                const SfxFoundCache_Impl& rFound = aFound[nPos];
                sal_uInt16 nWhich = rFound.nWhichId;
                const SfxPoolItem *pItem = nullptr;
                SfxItemState eState = pSet->GetItemState(nWhich, true, &pItem);
                if ( eState == SfxItemState::DEFAULT && SfxItemPool::IsWhich(nWhich) )
                    pItem = &pSet->Get(nWhich);
                UpdateControllers_Impl( pInterface, aFound[nPos], pItem, eState );
            }
            bUpdated = true;
        }

        delete pSet;
    }

    if ( !bUpdated && pCache )
    {
        // When pCache == NULL and no SlotServer
        // (for example due to locked Dispatcher! ),
        // obviously do not try to update
        SfxFoundCache_Impl aFoundCache(0, pRealSlot, pCache );
        UpdateControllers_Impl( nullptr, aFoundCache, nullptr, SfxItemState::DISABLED);
    }
}


void SfxBindings::InvalidateSlotsInMap_Impl()
{
    InvalidateSlotMap::const_iterator pIter = pImpl->m_aInvalidateSlots.begin();
    while ( pIter != pImpl->m_aInvalidateSlots.end() )
    {
        Invalidate( pIter->first );
        ++pIter;
    }
    pImpl->m_aInvalidateSlots.clear();
}


void SfxBindings::AddSlotToInvalidateSlotsMap_Impl( sal_uInt16 nId )
{
    pImpl->m_aInvalidateSlots[nId] = true;
}


void SfxBindings::Update
(
    sal_uInt16      nId     // the bound and up-to-date Slot-Id
)
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );

    if ( pDispatcher )
        pDispatcher->Flush();

    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->Update( nId );

    SfxStateCache* pCache = GetStateCache( nId );
    if ( pCache )
    {
        pImpl->bInUpdate = true;
        if ( pImpl->bMsgDirty )
        {
            UpdateSlotServer_Impl();
            pCache = GetStateCache( nId );
        }

        if (pCache)
        {
            bool bInternalUpdate = true;
            if( pCache->GetDispatch().is() && pCache->GetItemLink() )
            {
                pCache->SetCachedState(true);
                bInternalUpdate = ( pCache->GetInternalController() != nullptr );
            }

            if ( bInternalUpdate )
            {
                // Query Status
                const SfxSlotServer* pMsgServer = pDispatcher ? pCache->GetSlotServer(*pDispatcher, pImpl->xProv) : nullptr;
                if ( !pCache->IsControllerDirty() )
                {
                    pImpl->bInUpdate = false;
                    InvalidateSlotsInMap_Impl();
                    return;
                }
                if (!pMsgServer)
                {
                    pCache->SetState(SfxItemState::DISABLED, nullptr);
                    pImpl->bInUpdate = false;
                    InvalidateSlotsInMap_Impl();
                    return;
                }

                Update_Impl(pCache);
            }

            pImpl->bAllDirty = false;
        }

        pImpl->bInUpdate = false;
        InvalidateSlotsInMap_Impl();
    }
}


void SfxBindings::Update()
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );

    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->Update();

    if ( pDispatcher )
    {
        if ( nRegLevel )
            return;

        pImpl->bInUpdate = true;
        pDispatcher->Flush();
        pDispatcher->Update_Impl();
        while ( !NextJob_Impl(nullptr) )
            ; // loop
        pImpl->bInUpdate = false;
        InvalidateSlotsInMap_Impl();
    }
}


void SfxBindings::SetState
(
    const SfxItemSet&   rSet    // status values to be set
)
{
    // when locked then only invalidate
    if ( nRegLevel )
    {
        SfxItemIter aIter(rSet);
        for ( const SfxPoolItem *pItem = aIter.FirstItem();
              pItem;
              pItem = aIter.NextItem() )
            Invalidate( pItem->Which() );
    }
    else
    {
        // Status may be accepted only if all slot-pointers are set
        if ( pImpl->bMsgDirty )
            UpdateSlotServer_Impl();

        // Iterate over the itemset, update if the slot bound
        //! Bug: Use WhichIter and possibly send VoidItems up
        SfxItemIter aIter(rSet);
        for ( const SfxPoolItem *pItem = aIter.FirstItem();
              pItem;
              pItem = aIter.NextItem() )
        {
            SfxStateCache* pCache =
                    GetStateCache( rSet.GetPool()->GetSlotId(pItem->Which()) );
            if ( pCache )
            {
                // Update status
                if ( !pCache->IsControllerDirty() )
                    pCache->Invalidate(false);
                pCache->SetState( SfxItemState::DEFAULT, pItem );

                //! Not implemented: Updates from EnumSlots via master slots
            }
        }
    }
}


void SfxBindings::SetState
(
    const SfxPoolItem&  rItem   // Status value to be set
)
{
    if ( nRegLevel )
    {
        Invalidate( rItem.Which() );
    }
    else
    {
        // Status may be accepted only if all slot-pointers are set
        if ( pImpl->bMsgDirty )
            UpdateSlotServer_Impl();

        //update if the slot bound
        DBG_ASSERT( SfxItemPool::IsSlot( rItem.Which() ),
                    "cannot set items with which-id" );
        SfxStateCache* pCache = GetStateCache( rItem.Which() );
        if ( pCache )
        {
            // Update Status
            if ( !pCache->IsControllerDirty() )
                pCache->Invalidate(false);
            pCache->SetState( SfxItemState::DEFAULT, &rItem );

            //! Not implemented: Updates from EnumSlots via master slots
        }
    }
}


SfxStateCache* SfxBindings::GetAnyStateCache_Impl( sal_uInt16 nId )
{
    SfxStateCache* pCache = GetStateCache( nId );
    if ( !pCache && pImpl->pSubBindings )
        return pImpl->pSubBindings->GetAnyStateCache_Impl( nId );
    return pCache;
}

SfxStateCache* SfxBindings::GetStateCache
(
    sal_uInt16   nId   /*  Slot-Id, which SfxStatusCache is to be found */
)
{
    return GetStateCache(nId, nullptr);
}

SfxStateCache* SfxBindings::GetStateCache
(
    sal_uInt16   nId,   /*  Slot-Id, which SfxStatusCache is to be found */
    sal_uInt16*  pPos   /*  NULL for instance the position from which the
                           bindings are to be searched binary. Returns the
                           position back for where the nId was found,
                           or where it was inserted. */
)
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );
    // is the specified function bound?
    const sal_uInt16 nStart = ( pPos ? *pPos : 0 );
    const sal_uInt16 nPos = GetSlotPos( nId, nStart );

    if ( nPos < pImpl->pCaches->size() &&
         (*pImpl->pCaches)[nPos]->GetId() == nId )
    {
        if ( pPos )
            *pPos = nPos;
        return (*pImpl->pCaches)[nPos];
    }
    return nullptr;
}


void SfxBindings::InvalidateAll
(
    bool  bWithMsg  /* true   Mark Slot Server as invalid
                       false  Slot Server remains valid */
)
{
    DBG_ASSERT( !pImpl->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->InvalidateAll( bWithMsg );

    // everything is already set dirty or downing => nothing to do
    if ( !pDispatcher ||
         ( pImpl->bAllDirty && ( !bWithMsg || pImpl->bAllMsgDirty ) ) ||
         SfxGetpApp()->IsDowning() )
    {
        return;
    }

    pImpl->bAllMsgDirty = pImpl->bAllMsgDirty || bWithMsg;
    pImpl->bMsgDirty = pImpl->bMsgDirty || pImpl->bAllMsgDirty || bWithMsg;
    pImpl->bAllDirty = true;

    for (SfxStateCache* pCache : *pImpl->pCaches)
        pCache->Invalidate(bWithMsg);

    pImpl->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImpl->aAutoTimer.Stop();
        pImpl->aAutoTimer.SetTimeout(TIMEOUT_FIRST);
        pImpl->aAutoTimer.Start();
    }
}


void SfxBindings::Invalidate
(
    const sal_uInt16* pIds /* numerically sorted NULL-terminated array of
                              slot IDs (individual, not as a couple!) */
)
{
    if ( pImpl->bInUpdate )
    {
        sal_Int32 i = 0;
        while ( pIds[i] != 0 )
            AddSlotToInvalidateSlotsMap_Impl( pIds[i++] );

        if ( pImpl->pSubBindings )
            pImpl->pSubBindings->Invalidate( pIds );
        return;
    }

    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->Invalidate( pIds );

    // everything is already set dirty or downing => nothing to do
    if ( !pDispatcher || pImpl->bAllDirty || SfxGetpApp()->IsDowning() )
        return;

    // Search binary in always smaller areas
    for ( sal_uInt16 n = GetSlotPos(*pIds);
          *pIds && n < pImpl->pCaches->size();
          n = GetSlotPos(*pIds, n) )
    {
        // If SID is ever bound, then invalidate the cache
        SfxStateCache *pCache = (*pImpl->pCaches)[n];
        if ( pCache->GetId() == *pIds )
            pCache->Invalidate(false);

        // Next SID
        if ( !*++pIds )
            break;
        assert( *pIds > *(pIds-1) );
    }

    // if not enticed to start update timer
    pImpl->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImpl->aAutoTimer.Stop();
        pImpl->aAutoTimer.SetTimeout(TIMEOUT_FIRST);
        pImpl->aAutoTimer.Start();
    }
}


void SfxBindings::InvalidateShell
(
    const SfxShell&  rSh,  /* <SfxShell> whose Slot-Ids should be
                              invalidated */
    bool             bDeep /* true
                              also the SfxShell's inherited slot IDs are invalidated

                              false
                              the inherited and not overridden Slot-Ids are
                              invalidated */
                             // for now always bDeep
)
{
    DBG_ASSERT( !pImpl->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->InvalidateShell( rSh, bDeep );

    if ( !pDispatcher || pImpl->bAllDirty || SfxGetpApp()->IsDowning() )
        return;

    // flush now already, it is done in GetShellLevel (rsh) anyway,
    // important so that is set correctly: pImpl-> ball(Msg)Dirty
    pDispatcher->Flush();

    if ((pImpl->bAllDirty && pImpl->bAllMsgDirty) || SfxGetpApp()->IsDowning())
    {
        // if the next one is anyway, then all the servers are collected
        return;
    }

    // Find Level
    sal_uInt16 nLevel = pDispatcher->GetShellLevel(rSh);
    if ( nLevel != USHRT_MAX )
    {
        for (SfxStateCache* pCache : *pImpl->pCaches)
        {
            const SfxSlotServer *pMsgServer =
                pCache->GetSlotServer(*pDispatcher, pImpl->xProv);
            if ( pMsgServer && pMsgServer->GetShellLevel() == nLevel )
                pCache->Invalidate(false);
        }
        pImpl->nMsgPos = 0;
        if ( !nRegLevel )
        {
            pImpl->aAutoTimer.Stop();
            pImpl->aAutoTimer.SetTimeout(TIMEOUT_FIRST);
            pImpl->aAutoTimer.Start();
            pImpl->bFirstRound = true;
            pImpl->nFirstShell = nLevel;
        }
    }
}


void SfxBindings::Invalidate
(
    sal_uInt16 nId              // Status value to be set
)
{
    if ( pImpl->bInUpdate )
    {
        AddSlotToInvalidateSlotsMap_Impl( nId );
        if ( pImpl->pSubBindings )
            pImpl->pSubBindings->Invalidate( nId );
        return;
    }

    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->Invalidate( nId );

    if ( !pDispatcher || pImpl->bAllDirty || SfxGetpApp()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        pCache->Invalidate(false);
        pImpl->nMsgPos = std::min(GetSlotPos(nId), pImpl->nMsgPos);
        if ( !nRegLevel )
        {
            pImpl->aAutoTimer.Stop();
            pImpl->aAutoTimer.SetTimeout(TIMEOUT_FIRST);
            pImpl->aAutoTimer.Start();
        }
    }
}


void SfxBindings::Invalidate
(
    sal_uInt16  nId,                // Status value to be set
    bool        bWithItem,          // Clear StateCache?
    bool        bWithMsg            // Get new SlotServer?
)
{
    DBG_ASSERT( !pImpl->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->Invalidate( nId, bWithItem, bWithMsg );

    if ( SfxGetpApp()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        if ( bWithItem )
            pCache->ClearCache();
        pCache->Invalidate(bWithMsg);

        if ( !pDispatcher || pImpl->bAllDirty )
            return;

        pImpl->nMsgPos = std::min(GetSlotPos(nId), pImpl->nMsgPos);
        if ( !nRegLevel )
        {
            pImpl->aAutoTimer.Stop();
            pImpl->aAutoTimer.SetTimeout(TIMEOUT_FIRST);
            pImpl->aAutoTimer.Start();
        }
    }
}


bool SfxBindings::IsBound( sal_uInt16 nSlotId )
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );
    sal_uInt16 nStartSearchAt = 0;
    return GetStateCache(nSlotId, &nStartSearchAt ) != nullptr;
}


sal_uInt16 SfxBindings::GetSlotPos( sal_uInt16 nId, sal_uInt16 nStartSearchAt )
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );

    // answer immediately if a function-seek comes repeated
    if ( pImpl->nCachedFunc1 < pImpl->pCaches->size() &&
         (*pImpl->pCaches)[pImpl->nCachedFunc1]->GetId() == nId )
    {
        return pImpl->nCachedFunc1;
    }
    if ( pImpl->nCachedFunc2 < pImpl->pCaches->size() &&
         (*pImpl->pCaches)[pImpl->nCachedFunc2]->GetId() == nId )
    {
        // swap the caches
        sal_uInt16 nTemp = pImpl->nCachedFunc1;
        pImpl->nCachedFunc1 = pImpl->nCachedFunc2;
        pImpl->nCachedFunc2 = nTemp;
        return pImpl->nCachedFunc1;
    }

    // binary search, if not found, seek to target-position
    if ( pImpl->pCaches->size() <= nStartSearchAt )
    {
        return 0;
    }
    if ( (sal_uInt16) pImpl->pCaches->size() == (nStartSearchAt+1) )
    {
        return (*pImpl->pCaches)[nStartSearchAt]->GetId() >= nId ? 0 : 1;
    }
    sal_uInt16 nLow = nStartSearchAt;
    sal_uInt16 nMid = 0;
    sal_uInt16 nHigh = 0;
    bool bFound = false;
    nHigh = pImpl->pCaches->size() - 1;
    while ( !bFound && nLow <= nHigh )
    {
        nMid = (nLow + nHigh) >> 1;
        DBG_ASSERT( nMid < pImpl->pCaches->size(), "bsearch is buggy" );
        int nDiff = (int) nId - (int) ( ((*pImpl->pCaches)[nMid])->GetId() );
        if ( nDiff < 0)
        {   if ( nMid == 0 )
                break;
            nHigh = nMid - 1;
        }
        else if ( nDiff > 0 )
        {   nLow = nMid + 1;
            if ( nLow == 0 )
                break;
        }
        else
            bFound = true;
    }
    sal_uInt16 nPos = bFound ? nMid : nLow;
    DBG_ASSERT( nPos <= pImpl->pCaches->size(), "" );
    DBG_ASSERT( nPos == pImpl->pCaches->size() ||
                nId <= (*pImpl->pCaches)[nPos]->GetId(), "" );
    DBG_ASSERT( nPos == nStartSearchAt ||
                nId > (*pImpl->pCaches)[nPos-1]->GetId(), "" );
    DBG_ASSERT( ( (nPos+1) >= (sal_uInt16) pImpl->pCaches->size() ) ||
                nId < (*pImpl->pCaches)[nPos+1]->GetId(), "" );
    pImpl->nCachedFunc2 = pImpl->nCachedFunc1;
    pImpl->nCachedFunc1 = nPos;
    return nPos;
}

void SfxBindings::RegisterInternal_Impl( SfxControllerItem& rItem )
{
    Register_Impl( rItem, true );

}

void SfxBindings::Register( SfxControllerItem& rItem )
{
    Register_Impl( rItem, false );
}

void SfxBindings::Register_Impl( SfxControllerItem& rItem, bool bInternal )
{
//    DBG_ASSERT( nRegLevel > 0, "registration without EnterRegistrations" );
    DBG_ASSERT( !pImpl->bInNextJob, "SfxBindings::Register while status-updating" );

    // insert new cache if it does not already exist
    sal_uInt16 nId = rItem.GetId();
    sal_uInt16 nPos = GetSlotPos(nId);
    if ( nPos >= pImpl->pCaches->size() ||
         (*pImpl->pCaches)[nPos]->GetId() != nId )
    {
        SfxStateCache* pCache = new SfxStateCache(nId);
        pImpl->pCaches->insert( pImpl->pCaches->begin() + nPos, pCache );
        DBG_ASSERT( nPos == 0 ||
                    (*pImpl->pCaches)[nPos]->GetId() >
                        (*pImpl->pCaches)[nPos-1]->GetId(), "" );
        DBG_ASSERT( (nPos == pImpl->pCaches->size()-1) ||
                    (*pImpl->pCaches)[nPos]->GetId() <
                        (*pImpl->pCaches)[nPos+1]->GetId(), "" );
        pImpl->bMsgDirty = true;
    }

    // enqueue the new binding
    if ( bInternal )
    {
        (*pImpl->pCaches)[nPos]->SetInternalController( &rItem );
    }
    else
    {
        SfxControllerItem *pOldItem = (*pImpl->pCaches)[nPos]->ChangeItemLink(&rItem);
        rItem.ChangeItemLink(pOldItem);
    }
}


void SfxBindings::Release( SfxControllerItem& rItem )
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );
    DBG_ASSERT( !pImpl->bInNextJob, "SfxBindings::Release while status-updating" );
    ENTERREGISTRATIONS();

    // find the bound function
    sal_uInt16 nId = rItem.GetId();
    sal_uInt16 nPos = GetSlotPos(nId);
    SfxStateCache* pCache = (nPos < pImpl->pCaches->size()) ? (*pImpl->pCaches)[nPos] : nullptr;
    if ( pCache && pCache->GetId() == nId )
    {
        if ( pCache->GetInternalController() == &rItem )
        {
            pCache->ReleaseInternalController();
        }
        else
        {
            // is this the first binding in the list?
            SfxControllerItem* pItem = pCache->GetItemLink();
            if ( pItem == &rItem )
                pCache->ChangeItemLink( rItem.GetItemLink() );
            else
            {
                // search the binding in the list
                while ( pItem && pItem->GetItemLink() != &rItem )
                    pItem = pItem->GetItemLink();

                // unlink it if it was found
                if ( pItem )
                    pItem->ChangeItemLink( rItem.GetItemLink() );
            }
        }

        // was this the last controller?
        if ( pCache->GetItemLink() == nullptr && !pCache->GetInternalController() )
        {
            pImpl->bCtrlReleased = true;
        }
    }

    LEAVEREGISTRATIONS();
}


const SfxPoolItem* SfxBindings::ExecuteSynchron( sal_uInt16 nId, const SfxPoolItem** ppItems )
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return nullptr;

    return Execute_Impl( nId, ppItems, 0, SfxCallMode::SYNCHRON, nullptr );
}

bool SfxBindings::Execute( sal_uInt16 nId, const SfxPoolItem** ppItems, SfxCallMode nCallMode )
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return false;

    const SfxPoolItem* pRet = Execute_Impl( nId, ppItems, 0, nCallMode, nullptr );
    return ( pRet != nullptr );
}

const SfxPoolItem* SfxBindings::Execute_Impl( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi, SfxCallMode nCallMode,
                        const SfxPoolItem **ppInternalArgs, bool bGlobalOnly )
{
    SfxStateCache *pCache = GetStateCache( nId );
    if ( !pCache )
    {
        SfxBindings *pBind = pImpl->pSubBindings;
        while ( pBind )
        {
            if ( pBind->GetStateCache( nId ) )
                return pBind->Execute_Impl( nId, ppItems, nModi, nCallMode, ppInternalArgs, bGlobalOnly );
            pBind = pBind->pImpl->pSubBindings;
        }
    }

    SfxDispatcher &rDispatcher = *pDispatcher;
    rDispatcher.Flush();

    // get SlotServer (Slot+ShellLevel) and Shell from cache
    std::unique_ptr<SfxStateCache> xCache;
    if ( !pCache )
    {
        // Execution of non cached slots (Accelerators don't use Controllers)
        // slot is uncached, use SlotCache to handle external dispatch providers
        xCache.reset(new SfxStateCache(nId));
        pCache = xCache.get();
        pCache->GetSlotServer( rDispatcher, pImpl->xProv );
    }

    if ( pCache->GetDispatch().is() )
    {
        DBG_ASSERT( !ppInternalArgs, "Internal args get lost when dispatched!" );

        SfxItemPool &rPool = GetDispatcher()->GetFrame()->GetObjectShell()->GetPool();
        SfxRequest aReq( nId, nCallMode, rPool );
        aReq.SetModifier( nModi );
        if( ppItems )
            while( *ppItems )
                aReq.AppendItem( **ppItems++ );

        // cache binds to an external dispatch provider
        sal_Int16 eRet = pCache->Dispatch( aReq.GetArgs(), nCallMode == SfxCallMode::SYNCHRON );
        SfxPoolItem *pPool;
        if ( eRet == css::frame::DispatchResultState::DONTKNOW )
            pPool = new SfxVoidItem( nId );
        else
            pPool = new SfxBoolItem( nId, eRet == css::frame::DispatchResultState::SUCCESS);

        DeleteItemOnIdle( pPool );
        return pPool;
    }

    // slot is handled internally by SfxDispatcher
    if ( pImpl->bMsgDirty )
        UpdateSlotServer_Impl();

    SfxShell *pShell=nullptr;
    const SfxSlot *pSlot=nullptr;

    const SfxSlotServer* pServer = pCache->GetSlotServer( rDispatcher, pImpl->xProv );
    if ( !pServer )
    {
        return nullptr;
    }
    else
    {
        pShell = rDispatcher.GetShell( pServer->GetShellLevel() );
        pSlot = pServer->GetSlot();
    }

    if ( bGlobalOnly )
        if ( dynamic_cast< const SfxModule *>( pShell ) == nullptr && dynamic_cast< const SfxApplication *>( pShell ) == nullptr && dynamic_cast< const SfxViewFrame *>( pShell ) == nullptr )
            return nullptr;

    SfxItemPool &rPool = pShell->GetPool();
    SfxRequest aReq( nId, nCallMode, rPool );
    aReq.SetModifier( nModi );
    if( ppItems )
        while( *ppItems )
            aReq.AppendItem( **ppItems++ );
    if ( ppInternalArgs )
    {
        SfxAllItemSet aSet( rPool );
        for ( const SfxPoolItem **pArg = ppInternalArgs; *pArg; ++pArg )
            aSet.Put( **pArg );
        aReq.SetInternalArgs_Impl( aSet );
    }

    Execute_Impl( aReq, pSlot, pShell );

    const SfxPoolItem* pRet = aReq.GetReturnValue();
    if ( !pRet )
    {
        SfxPoolItem *pVoid = new SfxVoidItem( nId );
        DeleteItemOnIdle( pVoid );
        pRet = pVoid;
    }

    return pRet;
}

void SfxBindings::Execute_Impl( SfxRequest& aReq, const SfxSlot* pSlot, SfxShell* pShell )
{
    SfxItemPool &rPool = pShell->GetPool();

    if ( SfxSlotKind::Enum == pSlot->GetKind() )
    {
        // for Enum-Slots, the Master has to be executed with the value
        // of the enums
        const SfxSlot *pRealSlot = pShell->GetInterface()->GetRealSlot(pSlot);
        const sal_uInt16 nSlotId = pRealSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        aReq.AppendItem( SfxAllEnumItem( rPool.GetWhich(nSlotId), pSlot->GetValue() ) );
        pDispatcher->Execute_( *pShell, *pRealSlot, aReq, aReq.GetCallMode() | SfxCallMode::RECORD );
    }
    else if ( SfxSlotKind::Attribute == pSlot->GetKind() )
    {
        // Which value has to be mapped for Attribute slots
        const sal_uInt16 nSlotId = pSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        if ( pSlot->IsMode(SfxSlotMode::TOGGLE) )
        {
            // The value is attached to a toggleable attribute (Bools)
            sal_uInt16 nWhich = pSlot->GetWhich(rPool);
            SfxItemSet aSet(rPool, nWhich, nWhich);
            SfxStateFunc aFunc  = pSlot->GetStateFnc();
            pShell->CallState( aFunc, aSet );
            const SfxPoolItem *pOldItem;
            SfxItemState eState = aSet.GetItemState(nWhich, true, &pOldItem);
            if ( eState == SfxItemState::DISABLED )
                return;

            if ( SfxItemState::DEFAULT == eState && SfxItemPool::IsWhich(nWhich) )
                pOldItem = &aSet.Get(nWhich);

            if ( SfxItemState::SET == eState ||
                 ( SfxItemState::DEFAULT == eState &&
                   SfxItemPool::IsWhich(nWhich) &&
                   pOldItem ) )
            {
                if ( dynamic_cast< const SfxBoolItem *>( pOldItem ) !=  nullptr )
                {
                    // we can toggle Bools
                    bool bOldValue = static_cast<const SfxBoolItem *>(pOldItem)->GetValue();
                    SfxBoolItem *pNewItem = static_cast<SfxBoolItem*>(pOldItem->Clone());
                    pNewItem->SetValue( !bOldValue );
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else if ( dynamic_cast< const SfxEnumItemInterface *>( pOldItem ) !=  nullptr &&
                        static_cast<const SfxEnumItemInterface *>(pOldItem)->HasBoolValue())
                {
                    // and Enums with Bool-Interface
                    SfxEnumItemInterface *pNewItem =
                        static_cast<SfxEnumItemInterface*>(pOldItem->Clone());
                    pNewItem->SetBoolValue(!static_cast<const SfxEnumItemInterface *>(pOldItem)->GetBoolValue());
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else {
                    OSL_FAIL( "Toggle only for Enums and Bools allowed" );
                }
            }
            else if ( SfxItemState::DONTCARE == eState )
            {
                // Create one Status-Item for each Factory
                SfxPoolItem *pNewItem = pSlot->GetType()->CreateItem();
                DBG_ASSERT( pNewItem, "Toggle to slot without ItemFactory" );
                pNewItem->SetWhich( nWhich );

                if ( dynamic_cast< const SfxBoolItem *>( pNewItem ) !=  nullptr )
                {
                  // we can toggle Bools
                    static_cast<SfxBoolItem*>(pNewItem)->SetValue( true );
                    aReq.AppendItem( *pNewItem );
                }
                else if ( dynamic_cast< const SfxEnumItemInterface *>( pNewItem ) !=  nullptr &&
                        static_cast<SfxEnumItemInterface *>(pNewItem)->HasBoolValue())
                {
                    // and Enums with Bool-Interface
                    static_cast<SfxEnumItemInterface*>(pNewItem)->SetBoolValue(true);
                    aReq.AppendItem( *pNewItem );
                }
                else {
                    OSL_FAIL( "Toggle only for Enums and Bools allowed" );
                }
                delete pNewItem;
            }
            else {
                OSL_FAIL( "suspicious Toggle-Slot" );
            }
        }

        pDispatcher->Execute_( *pShell, *pSlot, aReq, aReq.GetCallMode() | SfxCallMode::RECORD );
    }
    else
        pDispatcher->Execute_( *pShell, *pSlot, aReq, aReq.GetCallMode() | SfxCallMode::RECORD );
}


void SfxBindings::UpdateSlotServer_Impl()
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );

    // synchronize
    pDispatcher->Flush();

    if ( pImpl->bAllMsgDirty )
    {
        if ( !nRegLevel )
        {
            css::uno::Reference < css::frame::XFrame > xFrame
                ( pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );
            pImpl->bContextChanged = false;
        }
        else
            pImpl->bContextChanged = true;
    }

    for (SfxStateCache* pCache : *pImpl->pCaches)
    {
        //GetSlotServer can modify pImpl->pCaches
        pCache->GetSlotServer(*pDispatcher, pImpl->xProv);
    }
    pImpl->bMsgDirty = pImpl->bAllMsgDirty = false;

    Broadcast( SfxSimpleHint(SFX_HINT_DOCCHANGED) );
}


SfxItemSet* SfxBindings::CreateSet_Impl
(
    SfxStateCache*&         pCache,     // in: Status-Cache from nId
    const SfxSlot*&         pRealSlot,  // out: RealSlot to nId
    const SfxSlotServer**   pMsgServer, // out: Slot-Server to nId
    SfxFoundCacheArr_Impl&  rFound      // out: List of Caches for Siblings
)
{
    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );

    DBG_ASSERT( !pImpl->bMsgDirty, "CreateSet_Impl with dirty MessageServer" );
    assert(pDispatcher);

    const SfxSlotServer* pMsgSvr = pCache->GetSlotServer(*pDispatcher, pImpl->xProv);
    if (!pMsgSvr)
        return nullptr;

    pRealSlot = nullptr;
    *pMsgServer = pMsgSvr;

    sal_uInt16 nShellLevel = pMsgSvr->GetShellLevel();
    SfxShell *pShell = pDispatcher->GetShell( nShellLevel );
    if ( !pShell ) // rare GPF when browsing through update from Inet-Notify
        return nullptr;

    SfxItemPool &rPool = pShell->GetPool();

    // get the status method, which is served by the pCache
    SfxStateFunc pFnc = nullptr;
    const SfxInterface *pInterface = pShell->GetInterface();
    if ( SfxSlotKind::Enum == pMsgSvr->GetSlot()->GetKind() )
    {
        pRealSlot = pInterface->GetRealSlot(pMsgSvr->GetSlot());
        pCache = GetStateCache( pRealSlot->GetSlotId() );
    }
    else
        pRealSlot = pMsgSvr->GetSlot();

    // Note: pCache can be NULL!

    pFnc = pRealSlot->GetStateFnc();

    // the RealSlot is always on
    SfxFoundCache_Impl *pFound = new SfxFoundCache_Impl(
        pRealSlot->GetWhich(rPool), pRealSlot, pCache );
    rFound.push_back( pFound );

    // Search through the bindings for slots served by the same function. This ,    // will only affect slots which are present in the found interface.

    // The position of the  Statecaches in StateCache-Array
    sal_uInt16 nCachePos = pImpl->nMsgPos;
    const SfxSlot *pSibling = pRealSlot->GetNextSlot();

    // the Slots ODF a interfaces ar linked in a circle
    while ( pSibling > pRealSlot )
    {
        SfxStateFunc pSiblingFnc=nullptr;
        SfxStateCache *pSiblingCache =
                GetStateCache( pSibling->GetSlotId(), &nCachePos );

        // Is the slot cached ?
        if ( pSiblingCache )
        {
            const SfxSlotServer *pServ = pSiblingCache->GetSlotServer(*pDispatcher, pImpl->xProv);
            if ( pServ && pServ->GetShellLevel() == nShellLevel )
                pSiblingFnc = pServ->GetSlot()->GetStateFnc();
        }

        // Does the slot have to be updated at all?
        bool bInsert = pSiblingCache && pSiblingCache->IsControllerDirty();

        // It is not enough to ask for the same shell!!
        bool bSameMethod = pSiblingCache && pFnc == pSiblingFnc;

        // If the slot is a non-dirty master slot, then maybe one of his slaves
        // is dirty? Then the master slot is still inserted.
        if ( !bInsert && bSameMethod && pSibling->GetLinkedSlot() )
        {
            // Also check slave slots for Binding
            const SfxSlot* pFirstSlave = pSibling->GetLinkedSlot();
            for ( const SfxSlot *pSlaveSlot = pFirstSlave;
                  !bInsert;
                  pSlaveSlot = pSlaveSlot->GetNextSlot())
            {
                // the slaves points to its master
                DBG_ASSERT(pSlaveSlot->GetLinkedSlot() == pSibling,
                    "Wrong Master/Slave relationship!");

                sal_uInt16 nCurMsgPos = pImpl->nMsgPos;
                const SfxStateCache *pSlaveCache =
                    GetStateCache( pSlaveSlot->GetSlotId(), &nCurMsgPos );

                // Is the slave slot chached and dirty ?
                bInsert = pSlaveCache && pSlaveCache->IsControllerDirty();

                // Slaves are chained together in a circle
                if (pSlaveSlot->GetNextSlot() == pFirstSlave)
                    break;
            }
        }

        if ( bInsert && bSameMethod )
        {
            SfxFoundCache_Impl *pFoundCache = new SfxFoundCache_Impl(
                pSibling->GetWhich(rPool),
                pSibling, pSiblingCache );

            rFound.push_back( pFoundCache );
        }

        pSibling = pSibling->GetNextSlot();
    }

    // Create a Set from the ranges
    std::unique_ptr<sal_uInt16[]> pRanges(new sal_uInt16[rFound.size() * 2 + 1]);
    int j = 0;
    sal_uInt16 i = 0;
    while ( i < rFound.size() )
    {
        pRanges[j++] = rFound[i].nWhichId;
            // consecutive numbers
        for ( ; i < rFound.size()-1; ++i )
            if ( rFound[i].nWhichId+1 != rFound[i+1].nWhichId )
                break;
        pRanges[j++] = rFound[i++].nWhichId;
    }
    pRanges[j] = 0; // terminating NULL
    SfxItemSet *pSet = new SfxItemSet(rPool, pRanges.get());
    pRanges.reset();
    return pSet;
}


void SfxBindings::UpdateControllers_Impl
(
    const SfxInterface*         pIF,    // Id of the current serving Interface
    const SfxFoundCache_Impl&   rFound, // Cache, Slot, Which etc.
    const SfxPoolItem*          pItem,  // item to send to controller
    SfxItemState                eState  // state of item
)
{
    DBG_ASSERT( !rFound.pSlot || SfxSlotKind::Enum != rFound.pSlot->GetKind(),
                "direct update of enum slot isn't allowed" );

    SfxStateCache* pCache = rFound.pCache;
    const SfxSlot* pSlot = rFound.pSlot;
    DBG_ASSERT( !pCache || !pSlot || pCache->GetId() == pSlot->GetSlotId(), "SID mismatch" );

    // bound until now, the Controller to update the Slot.
    if ( pCache && pCache->IsControllerDirty() )
    {
        if ( SfxItemState::DONTCARE == eState )
        {
            // ambiguous
            pCache->SetState( SfxItemState::DONTCARE, reinterpret_cast<SfxPoolItem *>(-1) );
        }
        else if ( SfxItemState::DEFAULT == eState &&
                  SfxItemPool::IsSlot(rFound.nWhichId) )
        {
            // no Status or Default but without Pool
            SfxVoidItem aVoid(0);
            pCache->SetState( SfxItemState::UNKNOWN, &aVoid );
        }
        else if ( SfxItemState::DISABLED == eState )
            pCache->SetState(SfxItemState::DISABLED, nullptr);
        else
            pCache->SetState(SfxItemState::DEFAULT, pItem);
    }

    // Update the slots for so far available and bound Controllers for
    // Slave-Slots (Enum-value)
    DBG_ASSERT( !pSlot || nullptr == pSlot->GetLinkedSlot() || !pItem ||
                dynamic_cast< const SfxEnumItemInterface *>( pItem ) !=  nullptr,
                "master slot with non-enum-type found" );
    const SfxSlot *pFirstSlave = pSlot ? pSlot->GetLinkedSlot() : nullptr;
    if ( pIF && pFirstSlave)
    {
        // Items cast on EnumItem
        const SfxEnumItemInterface *pEnumItem = dynamic_cast< const SfxEnumItemInterface* >(pItem);
        if ( eState == SfxItemState::DEFAULT && !pEnumItem )
            eState = SfxItemState::DONTCARE;
        else
            eState = SfxControllerItem::GetItemState( pEnumItem );

        // Iterate over all Slaves-Slots
        for ( const SfxSlot *pSlave = pFirstSlave; pSlave; pSlave = pSlave->GetNextSlot() )
        {
            DBG_ASSERT(pSlave, "Wrong SlaveSlot binding!");
            DBG_ASSERT(SfxSlotKind::Enum == pSlave->GetKind(),"non enum slaves aren't allowed");
            DBG_ASSERT(pSlave->GetMasterSlotId() == pSlot->GetSlotId(),"Wrong MasterSlot!");

            // Binding exist for function ?
            SfxStateCache *pEnumCache = GetStateCache( pSlave->GetSlotId() );
            if ( pEnumCache )
            {
                pEnumCache->Invalidate(false);

                // HACK(CONTROL/SELECT Kram) ???
                if ( eState == SfxItemState::DONTCARE && rFound.nWhichId == 10144 )
                {
                    SfxVoidItem aVoid(0);
                    pEnumCache->SetState( SfxItemState::UNKNOWN, &aVoid );

                    if (pSlave->GetNextSlot() == pFirstSlave)
                        break;

                    continue;
                }

                if ( SfxItemState::DISABLED == eState || (pEnumItem && !pEnumItem->IsEnabled( pSlave->GetSlotId())) )
                {
                    // disabled
                    pEnumCache->SetState(SfxItemState::DISABLED, nullptr);
                }
                else if ( SfxItemState::DEFAULT == eState && pEnumItem )
                {
                    // Determine enum value
                    sal_uInt16 nValue = pEnumItem->GetEnumValue();
                    SfxBoolItem aBool( rFound.nWhichId, pSlave->GetValue() == nValue );
                    pEnumCache->SetState(SfxItemState::DEFAULT, &aBool);
                }
                else
                {
                    // ambiguous
                    pEnumCache->SetState( SfxItemState::DONTCARE, reinterpret_cast<SfxPoolItem *>(-1) );
                }
            }

            if (pSlave->GetNextSlot() == pFirstSlave)
                break;
        }
    }
}

IMPL_LINK_TYPED( SfxBindings, NextJob, Timer *, pTimer, void )
{
    NextJob_Impl(pTimer);
}

bool SfxBindings::NextJob_Impl(Timer * pTimer)
{
#ifdef DBG_UTIL
    // on Windows very often C++ Exceptions (GPF etc.) are caught by MSVCRT
    // or another MS library try to get them here
    try
    {
#endif
    const unsigned MAX_INPUT_DELAY = 200;

    DBG_ASSERT( pImpl->pCaches != nullptr, "SfxBindings not initialized" );

    if ( Application::GetLastInputInterval() < MAX_INPUT_DELAY && pTimer )
    {
        pImpl->aAutoTimer.SetTimeout(TIMEOUT_UPDATING);
        return true;
    }

    SfxApplication *pSfxApp = SfxGetpApp();

    if( pDispatcher )
        pDispatcher->Update_Impl();

    // modifying the SfxObjectInterface-stack without SfxBindings => nothing to do
    SfxViewFrame* pFrame = pDispatcher ? pDispatcher->GetFrame() : nullptr;
    if ( (pFrame && !pFrame->GetObjectShell()->AcceptStateUpdate()) || pSfxApp->IsDowning() || pImpl->pCaches->empty() )
    {
        return true;
    }
    if ( !pDispatcher || !pDispatcher->IsFlushed() )
    {
        return true;
    }

    // if possible Update all server / happens in its own time slice
    // but process all events at once when unit testing, for reliability reasons
    static bool bTest = getenv("LO_TESTNAME");
    if ( pImpl->bMsgDirty && !bTest )
    {
        UpdateSlotServer_Impl();
        return false;
    }

    pImpl->bAllDirty = false;
    pImpl->aAutoTimer.SetTimeout(TIMEOUT_UPDATING);

    // at least 10 loops and further if more jobs are available but no input
    bool bPreEmptive = pTimer && !pSfxApp->Get_Impl()->nInReschedule;
    sal_uInt16 nLoops = 10;
    pImpl->bInNextJob = true;
    const sal_uInt16 nCount = pImpl->pCaches->size();
    while ( pImpl->nMsgPos < nCount )
    {
        // iterate through the bound functions
        bool bJobDone = false;
        while ( !bJobDone )
        {
            SfxStateCache* pCache = (*pImpl->pCaches)[pImpl->nMsgPos];
            DBG_ASSERT( pCache, "invalid SfxStateCache-position in job queue" );
            bool bWasDirty = pCache->IsControllerDirty();
            if ( bWasDirty )
            {
                    Update_Impl( pCache );
                    DBG_ASSERT( nCount == pImpl->pCaches->size(),
                            "Reschedule in StateChanged => buff" );
            }

            // skip to next function binding
            ++pImpl->nMsgPos;

            // keep job if it is not completed, but any input is available
            bJobDone = pImpl->nMsgPos >= nCount;
            if ( bJobDone && pImpl->bFirstRound )
            {

                // Update of the  preferred shell has been done, now may
                // also the others shells be updated
                bJobDone = false;
                pImpl->bFirstRound = false;
                pImpl->nMsgPos = 0;
            }

            if ( bWasDirty && !bJobDone && bPreEmptive && (--nLoops == 0) )
            {
                pImpl->bInNextJob = false;
                return false;
            }
        }
    }

    pImpl->nMsgPos = 0;

    pImpl->aAutoTimer.Stop();

    // Update round is finished
    pImpl->bInNextJob = false;
    Broadcast(SfxSimpleHint(SFX_HINT_UPDATEDONE));
    return true;
#ifdef DBG_UTIL
    }
    catch (...)
    {
        OSL_FAIL("C++ exception caught!");
        pImpl->bInNextJob = false;
    }

    return false;
#endif
}


sal_uInt16 SfxBindings::EnterRegistrations(const char *pFile, int nLine)
{
    SAL_INFO(
        "sfx.control",
        std::setw(std::min(nRegLevel, sal_uInt16(8))) << ' ' << "this = " << this
            << " Level = " << nRegLevel << " SfxBindings::EnterRegistrations "
            << (pFile
                ? SAL_STREAM("File: " << pFile << " Line: " << nLine) : ""));

    // When bindings are locked, also lock sub bindings.
    if ( pImpl->pSubBindings )
    {
        pImpl->pSubBindings->ENTERREGISTRATIONS();

        // These EnterRegistrations are not "real" for the SubBindings
        pImpl->pSubBindings->pImpl->nOwnRegLevel--;

        // Synchronize Bindings
        pImpl->pSubBindings->nRegLevel = nRegLevel + pImpl->pSubBindings->pImpl->nOwnRegLevel + 1;
    }

    pImpl->nOwnRegLevel++;

    // check if this is the outer most level
    if ( ++nRegLevel == 1 )
    {
        // stop background-processing
        pImpl->aAutoTimer.Stop();

        // flush the cache
        pImpl->nCachedFunc1 = 0;
        pImpl->nCachedFunc2 = 0;

        // Mark if the all of the Caches have disappeared.
        pImpl->bCtrlReleased = false;
    }

    return nRegLevel;
}


void SfxBindings::LeaveRegistrations( const char *pFile, int nLine )
{
    DBG_ASSERT( nRegLevel, "Leave without Enter" );

    // Only when the SubBindings are still locked by the Superbindings,
    // remove this lock (i.e. if there are more locks than "real" ones)
    if ( pImpl->pSubBindings && pImpl->pSubBindings->nRegLevel > pImpl->pSubBindings->pImpl->nOwnRegLevel )
    {
        // Synchronize Bindings
        pImpl->pSubBindings->nRegLevel = nRegLevel + pImpl->pSubBindings->pImpl->nOwnRegLevel;

        // This LeaveRegistrations is not "real" for SubBindings
        pImpl->pSubBindings->pImpl->nOwnRegLevel++;
        pImpl->pSubBindings->LEAVEREGISTRATIONS();
    }

    pImpl->nOwnRegLevel--;

    // check if this is the outer most level
    if ( --nRegLevel == 0 && !SfxGetpApp()->IsDowning() )
    {
        if ( pImpl->bContextChanged )
        {
            pImpl->bContextChanged = false;
        }

        SfxViewFrame* pFrame = pDispatcher->GetFrame();

        // If possible remove unused Caches, for example prepare PlugInInfo
        if ( pImpl->bCtrlReleased )
        {
            for ( sal_uInt16 nCache = pImpl->pCaches->size(); nCache > 0; --nCache )
            {
                // Get Cache via css::sdbcx::Index
                SfxStateCache *pCache = (*pImpl->pCaches)[nCache-1];

                // No interested Controller present
                if ( pCache->GetItemLink() == nullptr && !pCache->GetInternalController() )
                {
                    // Remove Cache. Safety: first remove and then delete
                    pImpl->pCaches->erase(pImpl->pCaches->begin() + nCache - 1);
                    delete pCache;
                }
            }
        }

        // restart background-processing
        pImpl->nMsgPos = 0;
        if ( !pFrame || !pFrame->GetObjectShell() )
            return;
        if ( pImpl->pCaches && !pImpl->pCaches->empty() )
        {
            pImpl->aAutoTimer.Stop();
            pImpl->aAutoTimer.SetTimeout(TIMEOUT_FIRST);
            pImpl->aAutoTimer.Start();
        }
    }

    SAL_INFO(
        "sfx.control",
        std::setw(std::min(nRegLevel, sal_uInt16(8))) << ' ' << "this = " << this
            << " Level = " << nRegLevel << " SfxBindings::LeaveRegistrations "
            << (pFile
                ? SAL_STREAM("File: " << pFile << " Line: " << nLine) : ""));
}


void SfxBindings::SetDispatcher( SfxDispatcher *pDisp )
{
    SfxDispatcher *pOldDispat = pDispatcher;
    if ( pDisp != pDispatcher )
    {
        if ( pOldDispat )
        {
            SfxBindings* pBind = pOldDispat->GetBindings();
            while ( pBind )
            {
                if ( pBind->pImpl->pSubBindings == this && pBind->pDispatcher != pDisp )
                    pBind->SetSubBindings_Impl( nullptr );
                pBind = pBind->pImpl->pSubBindings;
            }
        }

        pDispatcher = pDisp;

        css::uno::Reference < css::frame::XDispatchProvider > xProv;
        if ( pDisp )
            xProv.set( pDisp->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );

        SetDispatchProvider_Impl( xProv );
        InvalidateAll( true );

        if ( pDispatcher && !pOldDispat )
        {
            if ( pImpl->pSubBindings && pImpl->pSubBindings->pDispatcher != pOldDispat )
            {
                OSL_FAIL( "SubBindings already set before activating!" );
                pImpl->pSubBindings->ENTERREGISTRATIONS();
            }
            LEAVEREGISTRATIONS();
        }
        else if( !pDispatcher )
        {
            ENTERREGISTRATIONS();
            if ( pImpl->pSubBindings && pImpl->pSubBindings->pDispatcher != pOldDispat )
            {
                OSL_FAIL( "SubBindings still set even when deactivating!" );
                pImpl->pSubBindings->LEAVEREGISTRATIONS();
            }
        }

        Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

        if ( pDisp )
        {
            SfxBindings* pBind = pDisp->GetBindings();
            while ( pBind && pBind != this )
            {
                if ( !pBind->pImpl->pSubBindings )
                {
                    pBind->SetSubBindings_Impl( this );
                    break;
                }

                pBind = pBind->pImpl->pSubBindings;
            }
        }
    }
}


void SfxBindings::ClearCache_Impl( sal_uInt16 nSlotId )
{
    SfxStateCache* pCache = GetStateCache(nSlotId);
    if (!pCache)
        return;
    pCache->ClearCache();
}


void SfxBindings::StartUpdate_Impl( bool bComplete )
{
    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->StartUpdate_Impl( bComplete );

    if ( !bComplete )
        // Update may be interrupted
        NextJob_Impl(&pImpl->aAutoTimer);
    else
        // Update all slots in a row
        NextJob_Impl(nullptr);
}


SfxItemState SfxBindings::QueryState( sal_uInt16 nSlot, std::unique_ptr<SfxPoolItem> &rpState )
{
    css::uno::Reference< css::frame::XDispatch >  xDisp;
    SfxStateCache *pCache = GetStateCache( nSlot );
    if ( pCache )
        xDisp = pCache->GetDispatch();
    if ( xDisp.is() || !pCache )
    {
        const SfxSlot* pSlot = SfxSlotPool::GetSlotPool( pDispatcher->GetFrame() ).GetSlot( nSlot );
        if ( !pSlot || !pSlot->pUnoName )
            return SfxItemState::DISABLED;

        css::util::URL aURL;
        OUString aCmd( ".uno:" );
        aURL.Protocol = aCmd;
        aURL.Path = OUString::createFromAscii(pSlot->GetUnoName());
        aCmd += aURL.Path;
        aURL.Complete = aCmd;
        aURL.Main = aCmd;

        if ( !xDisp.is() )
            xDisp = pImpl->xProv->queryDispatch( aURL, OUString(), 0 );

        if ( xDisp.is() )
        {
            css::uno::Reference< css::lang::XUnoTunnel > xTunnel( xDisp, css::uno::UNO_QUERY );
            SfxOfficeDispatch* pDisp = nullptr;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = reinterpret_cast< SfxOfficeDispatch* >( sal::static_int_cast< sal_IntPtr >( nImplementation ));
            }

            if ( !pDisp )
            {
                bool bDeleteCache = false;
                if ( !pCache )
                {
                    pCache = new SfxStateCache( nSlot );
                    pCache->GetSlotServer( *GetDispatcher_Impl(), pImpl->xProv );
                    bDeleteCache = true;
                }

                SfxItemState eState = SfxItemState::SET;
                BindDispatch_Impl *pBind = new BindDispatch_Impl( xDisp, aURL, pCache, pSlot );
                pBind->acquire();
                xDisp->addStatusListener( pBind, aURL );
                if ( !pBind->GetStatus().IsEnabled )
                {
                    eState = SfxItemState::DISABLED;
                }
                else
                {
                    css::uno::Any aAny = pBind->GetStatus().State;
                    css::uno::Type aType = aAny.getValueType();

                    if ( aType == cppu::UnoType<bool>::get() )
                    {
                        bool bTemp = false;
                        aAny >>= bTemp ;
                        rpState.reset(new SfxBoolItem( nSlot, bTemp ));
                    }
                    else if ( aType == ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get() )
                    {
                        sal_uInt16 nTemp = 0;
                        aAny >>= nTemp ;
                        rpState.reset(new SfxUInt16Item( nSlot, nTemp ));
                    }
                    else if ( aType == cppu::UnoType<sal_uInt32>::get() )
                    {
                        sal_uInt32 nTemp = 0;
                        aAny >>= nTemp ;
                        rpState.reset(new SfxUInt32Item( nSlot, nTemp ));
                    }
                    else if ( aType == cppu::UnoType<OUString>::get() )
                    {
                        OUString sTemp ;
                        aAny >>= sTemp ;
                        rpState.reset(new SfxStringItem( nSlot, sTemp ));
                    }
                    else
                        rpState.reset(new SfxVoidItem( nSlot ));
                }

                xDisp->removeStatusListener( pBind, aURL );
                pBind->Release();
                if ( bDeleteCache )
                    DELETEZ( pCache );
                return eState;
            }
        }
    }

    // Then test at the dispatcher to check if the returned items from
    // there are always DELETE_ON_IDLE, a copy of it has to be made in
    // order to allow for transition of ownership.
    const SfxPoolItem *pItem = nullptr;
    SfxItemState eState = pDispatcher->QueryState( nSlot, pItem );
    if ( eState == SfxItemState::SET )
    {
        DBG_ASSERT( pItem, "SfxItemState::SET but no item!" );
        if ( pItem )
            rpState.reset(pItem->Clone());
    }
    else if ( eState == SfxItemState::DEFAULT && pItem )
    {
        rpState.reset(pItem->Clone());
    }

    return eState;
}

void SfxBindings::SetSubBindings_Impl( SfxBindings *pSub )
{
    if ( pImpl->pSubBindings )
    {
        pImpl->pSubBindings->SetDispatchProvider_Impl( css::uno::Reference< css::frame::XDispatchProvider > () );
        pImpl->pSubBindings->pImpl->pSuperBindings = nullptr;
    }

    pImpl->pSubBindings = pSub;

    if ( pSub )
    {
        pImpl->pSubBindings->SetDispatchProvider_Impl( pImpl->xProv );
        pSub->pImpl->pSuperBindings = this;
    }
}

SfxBindings* SfxBindings::GetSubBindings_Impl() const
{
    return pImpl->pSubBindings;
}

void SfxBindings::SetWorkWindow_Impl( SfxWorkWindow* pWork )
{
    pImpl->pWorkWin = pWork;
}

SfxWorkWindow* SfxBindings::GetWorkWindow_Impl() const
{
    return pImpl->pWorkWin;
}

bool SfxBindings::IsInUpdate() const
{
    bool bInUpdate = pImpl->bInUpdate;
    if ( !bInUpdate && pImpl->pSubBindings )
        bInUpdate = pImpl->pSubBindings->IsInUpdate();
    return bInUpdate;
}

void SfxBindings::SetVisibleState( sal_uInt16 nId, bool bShow )
{
    css::uno::Reference< css::frame::XDispatch >  xDisp;
    SfxStateCache *pCache = GetStateCache( nId );
    if ( pCache )
        pCache->SetVisibleState( bShow );
}

void SfxBindings::SetActiveFrame( const css::uno::Reference< css::frame::XFrame > & rFrame )
{
    if ( rFrame.is() || !pDispatcher )
        SetDispatchProvider_Impl( css::uno::Reference< css::frame::XDispatchProvider > ( rFrame, css::uno::UNO_QUERY ) );
    else
        SetDispatchProvider_Impl( css::uno::Reference< css::frame::XDispatchProvider > (
            pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), css::uno::UNO_QUERY ) );
}

const css::uno::Reference< css::frame::XFrame > SfxBindings::GetActiveFrame() const
{
    const css::uno::Reference< css::frame::XFrame > xFrame( pImpl->xProv, css::uno::UNO_QUERY );
    if ( xFrame.is() || !pDispatcher )
        return xFrame;
    else
        return pDispatcher->GetFrame()->GetFrame().GetFrameInterface();
}

void SfxBindings::SetDispatchProvider_Impl( const css::uno::Reference< css::frame::XDispatchProvider > & rProv )
{
    bool bInvalidate = ( rProv != pImpl->xProv );
    if ( bInvalidate )
    {
        pImpl->xProv = rProv;
        InvalidateAll( true );
    }

    if ( pImpl->pSubBindings )
        pImpl->pSubBindings->SetDispatchProvider_Impl( pImpl->xProv );
}

const css::uno::Reference< css::frame::XDispatchRecorder >& SfxBindings::GetRecorder() const
{
    return pImpl->xRecorder;
}

void SfxBindings::SetRecorder_Impl( css::uno::Reference< css::frame::XDispatchRecorder >& rRecorder )
{
    pImpl->xRecorder = rRecorder;
}

void SfxBindings::ContextChanged_Impl()
{
    if ( !pImpl->bInUpdate && ( !pImpl->bContextChanged || !pImpl->bAllMsgDirty ) )
    {
        InvalidateAll( true );
    }
}

uno::Reference < frame::XDispatch > SfxBindings::GetDispatch( const SfxSlot* pSlot, const util::URL& aURL, bool bMasterCommand )
{
    uno::Reference < frame::XDispatch > xRet;
    SfxStateCache* pCache = GetStateCache( pSlot->nSlotId );
    if ( pCache && !bMasterCommand )
        xRet = pCache->GetInternalDispatch();
    if ( !xRet.is() )
    {
        // dispatches for slaves are unbound, they don't have a state
        SfxOfficeDispatch* pDispatch = bMasterCommand ?
            new SfxOfficeDispatch( pDispatcher, pSlot, aURL ) :
            new SfxOfficeDispatch( *this, pDispatcher, pSlot, aURL );

        pDispatch->SetMasterUnoCommand( bMasterCommand );
        xRet.set( pDispatch );
        if ( !pCache )
            pCache = GetStateCache( pSlot->nSlotId );

        DBG_ASSERT( pCache, "No cache for OfficeDispatch!" );
        if ( pCache && !bMasterCommand )
            pCache->SetInternalDispatch( xRet );
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
