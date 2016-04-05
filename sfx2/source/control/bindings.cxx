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
#define TIMEOUT_IDLE       2500

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

class SfxUnoControllerArr_Impl
{
    typedef std::vector<SfxUnoControllerItem*> DataType;
    DataType maData;

public:

    typedef DataType::iterator iterator;

    iterator begin()
    {
        return maData.begin();
    }

    iterator end()
    {
        return maData.end();
    }

    void erase( iterator it )
    {
        maData.erase(it);
    }

    SfxUnoControllerItem* operator[] ( size_t i )
    {
        return maData[i];
    }

    size_t size() const
    {
        return maData.size();
    }

    void push_back( SfxUnoControllerItem* p )
    {
        maData.push_back(p);
    }
};

class SfxAsyncExec_Impl
{
    css::util::URL aCommand;
    css::uno::Reference< css::frame::XDispatch > xDisp;
    Timer           aTimer;

public:

    SfxAsyncExec_Impl( const css::util::URL& rCmd, const css::uno::Reference< css::frame::XDispatch >& rDisp )
        : aCommand( rCmd )
        , xDisp( rDisp )
    {
        aTimer.SetTimeoutHdl( LINK(this, SfxAsyncExec_Impl, TimerHdl) );
        aTimer.SetTimeout( 0 );
        aTimer.Start();
    }

    DECL_LINK_TYPED( TimerHdl, Timer*, void);
};

IMPL_LINK_TYPED(SfxAsyncExec_Impl, TimerHdl, Timer*, pTimer, void)
{
    (void)pTimer; // unused
    aTimer.Stop();

    Sequence<beans::PropertyValue> aSeq;
    xDisp->dispatch( aCommand, aSeq );

    delete this;
}

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
    SfxUnoControllerArr_Impl*
                            pUnoCtrlArr;
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
    AutoTimer               aTimer;         // for volatile Slots
    bool                    bInUpdate;      // for Assertions
    bool                    bInNextJob;     // for Assertions
    bool                    bFirstRound;    // First round in Update
    sal_uInt16              nFirstShell;    // Shell, the first round is preferred
    sal_uInt16              nOwnRegLevel;   // Counts the real Locks, except those of the Super Bindings
    InvalidateSlotMap       m_aInvalidateSlots; // store slots which are invalidated while in update
};

SfxBindings::SfxBindings()
:   pImp(new SfxBindings_Impl),
    pDispatcher(nullptr),
    nRegLevel(1)    // first becomes 0, when the Dispatcher is set
{
    pImp->nMsgPos = 0;
    pImp->bAllMsgDirty = true;
    pImp->bContextChanged = false;
    pImp->bMsgDirty = true;
    pImp->bAllDirty = true;
    pImp->ePopupAction = SfxPopupAction::DELETE;
    pImp->nCachedFunc1 = 0;
    pImp->nCachedFunc2 = 0;
    pImp->bCtrlReleased = false;
    pImp->bFirstRound = false;
    pImp->bInNextJob = false;
    pImp->bInUpdate = false;
    pImp->pSubBindings = nullptr;
    pImp->pSuperBindings = nullptr;
    pImp->pWorkWin = nullptr;
    pImp->pUnoCtrlArr = nullptr;
    pImp->nOwnRegLevel = nRegLevel;

    // all caches are valid (no pending invalidate-job)
    // create the list of caches
    pImp->pCaches = new SfxStateCacheArr_Impl;
    pImp->aTimer.SetTimeoutHdl( LINK(this, SfxBindings, NextJob) );
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
    pImp->pSubBindings = nullptr;

    ENTERREGISTRATIONS();

    pImp->aTimer.Stop();
    DeleteControllers_Impl();

    // Delete Caches
    for(SfxStateCacheArr_Impl::const_iterator it = pImp->pCaches->begin(); it != pImp->pCaches->end(); ++it)
        delete *it;

    DELETEZ( pImp->pWorkWin );

    delete pImp->pCaches;
    delete pImp;
}


void SfxBindings::DeleteControllers_Impl()
{
    // in the first round delete SfxPopupWindows
    sal_uInt16 nCount = pImp->pCaches->size();
    sal_uInt16 nCache;
    for ( nCache = 0; nCache < nCount; ++nCache )
    {
        // Remember were you are
        SfxStateCache *pCache = (*pImp->pCaches)[nCache];
        sal_uInt16 nSlotId = pCache->GetId();

        // Re-align, because the cache may have been reduced
        sal_uInt16 nNewCount = pImp->pCaches->size();
        if ( nNewCount < nCount )
        {
            nCache = GetSlotPos(nSlotId);
            if ( nCache >= nNewCount ||
                 nSlotId != (*pImp->pCaches)[nCache]->GetId() )
                --nCache;
            nCount = nNewCount;
        }
    }

    // Delete all Caches
    for ( nCache = pImp->pCaches->size(); nCache > 0; --nCache )
    {
        // Get Cache via css::sdbcx::Index
        SfxStateCache *pCache = (*pImp->pCaches)[ nCache-1 ];

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
        if( nCache-1 < (sal_uInt16) pImp->pCaches->size() )
            delete (*pImp->pCaches)[nCache-1];
        pImp->pCaches->erase(pImp->pCaches->begin()+ nCache - 1);
    }

    if( pImp->pUnoCtrlArr )
    {
        sal_uInt16 nCtrlCount = pImp->pUnoCtrlArr->size();
        for ( sal_uInt16 n=nCtrlCount; n>0; n-- )
        {
            SfxUnoControllerItem *pCtrl = (*pImp->pUnoCtrlArr)[n-1];
            pCtrl->ReleaseBindings();
        }

        DBG_ASSERT( !pImp->pUnoCtrlArr->size(), "Do not remove UnoControllerItems!" );
        DELETEZ( pImp->pUnoCtrlArr );
    }
}


void SfxBindings::HidePopups( bool bHide )
{
    // Hide SfxPopupWindows
    HidePopupCtrls_Impl( bHide );
    SfxBindings *pSub = pImp->pSubBindings;
    while ( pSub )
    {
        pImp->pSubBindings->HidePopupCtrls_Impl( bHide );
        pSub = pSub->pImp->pSubBindings;
    }

    // Hide SfxChildWindows
    DBG_ASSERT( pDispatcher, "HidePopups not allowed without dispatcher" );
    if ( pImp->pWorkWin )
        pImp->pWorkWin->HidePopups_Impl( bHide, true );
}

void SfxBindings::HidePopupCtrls_Impl( bool bHide )
{
    if ( bHide )
    {
        // Hide SfxPopupWindows
        pImp->ePopupAction = SfxPopupAction::HIDE;
    }
    else
    {
        // Show SfxPopupWindows
        pImp->ePopupAction = SfxPopupAction::SHOW;
    }

    pImp->ePopupAction = SfxPopupAction::DELETE;
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
    InvalidateSlotMap::const_iterator pIter = pImp->m_aInvalidateSlots.begin();
    while ( pIter != pImp->m_aInvalidateSlots.end() )
    {
        Invalidate( pIter->first );
        ++pIter;
    }
    pImp->m_aInvalidateSlots.clear();
}


void SfxBindings::AddSlotToInvalidateSlotsMap_Impl( sal_uInt16 nId )
{
    pImp->m_aInvalidateSlots[nId] = true;
}


void SfxBindings::Update
(
    sal_uInt16      nId     // the bound and up-to-date Slot-Id
)
{
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );

    if ( pDispatcher )
        pDispatcher->Flush();

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Update( nId );

    SfxStateCache* pCache = GetStateCache( nId );
    if ( pCache )
    {
        pImp->bInUpdate = true;
        if ( pImp->bMsgDirty )
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
                const SfxSlotServer* pMsgServer = pDispatcher ? pCache->GetSlotServer(*pDispatcher, pImp->xProv) : nullptr;
                if ( !pCache->IsControllerDirty() &&
                    ( !pMsgServer ||
                    !pMsgServer->GetSlot()->IsMode(SfxSlotMode::VOLATILE) ) )
                {
                    pImp->bInUpdate = false;
                    InvalidateSlotsInMap_Impl();
                    return;
                }
                if (!pMsgServer)
                {
                    pCache->SetState(SfxItemState::DISABLED, nullptr);
                    pImp->bInUpdate = false;
                    InvalidateSlotsInMap_Impl();
                    return;
                }

                Update_Impl(pCache);
            }

            pImp->bAllDirty = false;
        }

        pImp->bInUpdate = false;
        InvalidateSlotsInMap_Impl();
    }
}


void SfxBindings::Update()
{
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Update();

    if ( pDispatcher )
    {
        if ( nRegLevel )
            return;

        pImp->bInUpdate = true;
        pDispatcher->Flush();
        pDispatcher->Update_Impl();
        while ( !NextJob_Impl(nullptr) )
            ; // loop
        pImp->bInUpdate = false;
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
        if ( pImp->bMsgDirty )
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
        if ( pImp->bMsgDirty )
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
    if ( !pCache && pImp->pSubBindings )
        return pImp->pSubBindings->GetAnyStateCache_Impl( nId );
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
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );
    // is the specified function bound?
    const sal_uInt16 nStart = ( pPos ? *pPos : 0 );
    const sal_uInt16 nPos = GetSlotPos( nId, nStart );

    if ( nPos < pImp->pCaches->size() &&
         (*pImp->pCaches)[nPos]->GetId() == nId )
    {
        if ( pPos )
            *pPos = nPos;
        return (*pImp->pCaches)[nPos];
    }
    return nullptr;
}


void SfxBindings::InvalidateAll
(
    bool  bWithMsg  /* true   Mark Slot Server as invalid
                       false  Slot Server remains valid */
)
{
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateAll( bWithMsg );

    // everything is already set dirty or downing => nothing to do
    if ( !pDispatcher ||
         ( pImp->bAllDirty && ( !bWithMsg || pImp->bAllMsgDirty ) ) ||
         SfxGetpApp()->IsDowning() )
    {
        return;
    }

    pImp->bAllMsgDirty = pImp->bAllMsgDirty || bWithMsg;
    pImp->bMsgDirty = pImp->bMsgDirty || pImp->bAllMsgDirty || bWithMsg;
    pImp->bAllDirty = true;

    for ( size_t n = 0; n < pImp->pCaches->size(); ++n )
        (*pImp->pCaches)[n]->Invalidate(bWithMsg);

    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.Stop();
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
    }
}


void SfxBindings::Invalidate
(
    const sal_uInt16* pIds /* numerically sorted NULL-terminated array of
                              slot IDs (individual, not as a couple!) */
)
{
    if ( pImp->bInUpdate )
    {
        sal_Int32 i = 0;
        while ( pIds[i] != 0 )
            AddSlotToInvalidateSlotsMap_Impl( pIds[i++] );

        if ( pImp->pSubBindings )
            pImp->pSubBindings->Invalidate( pIds );
        return;
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Invalidate( pIds );

    // everything is already set dirty or downing => nothing to do
    if ( !pDispatcher || pImp->bAllDirty || SfxGetpApp()->IsDowning() )
        return;

    // Search binary in always smaller areas
    for ( sal_uInt16 n = GetSlotPos(*pIds);
          *pIds && n < pImp->pCaches->size();
          n = GetSlotPos(*pIds, n) )
    {
        // If SID is ever bound, then invalidate the cache
        SfxStateCache *pCache = (*pImp->pCaches)[n];
        if ( pCache->GetId() == *pIds )
            pCache->Invalidate(false);

        // Next SID
        if ( !*++pIds )
            break;
        DBG_ASSERT( *pIds > *(pIds-1), "pIds unsorted" );
    }

    // if not enticed to start update timer
    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.Stop();
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
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
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateShell( rSh, bDeep );

    if ( !pDispatcher || pImp->bAllDirty || SfxGetpApp()->IsDowning() )
        return;

    // flush now already, it is done in GetShellLevel (rsh) anyway,
    // important so that is set correctly: pimp-> ball(Msg)Dirty
    pDispatcher->Flush();

    if ((pImp->bAllDirty && pImp->bAllMsgDirty) || SfxGetpApp()->IsDowning())
    {
        // if the next one is anyway, then all the servers are collected
        return;
    }

    // Find Level
    sal_uInt16 nLevel = pDispatcher->GetShellLevel(rSh);
    if ( nLevel != USHRT_MAX )
    {
        for ( size_t n = 0; n < pImp->pCaches->size(); ++n )
        {
            SfxStateCache *pCache = (*pImp->pCaches)[n];
            const SfxSlotServer *pMsgServer =
                pCache->GetSlotServer(*pDispatcher, pImp->xProv);
            if ( pMsgServer && pMsgServer->GetShellLevel() == nLevel )
                pCache->Invalidate(false);
        }
        pImp->nMsgPos = 0;
        if ( !nRegLevel )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
            pImp->bFirstRound = true;
            pImp->nFirstShell = nLevel;
        }
    }
}


void SfxBindings::Invalidate
(
    sal_uInt16 nId              // Status value to be set
)
{
    if ( pImp->bInUpdate )
    {
        AddSlotToInvalidateSlotsMap_Impl( nId );
        if ( pImp->pSubBindings )
            pImp->pSubBindings->Invalidate( nId );
        return;
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Invalidate( nId );

    if ( !pDispatcher || pImp->bAllDirty || SfxGetpApp()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        pCache->Invalidate(false);
        pImp->nMsgPos = std::min(GetSlotPos(nId), pImp->nMsgPos);
        if ( !nRegLevel )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
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
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Invalidate( nId, bWithItem, bWithMsg );

    if ( SfxGetpApp()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        if ( bWithItem )
            pCache->ClearCache();
        pCache->Invalidate(bWithMsg);

        if ( !pDispatcher || pImp->bAllDirty )
            return;

        pImp->nMsgPos = std::min(GetSlotPos(nId), pImp->nMsgPos);
        if ( !nRegLevel )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
        }
    }
}


bool SfxBindings::IsBound( sal_uInt16 nSlotId, sal_uInt16 nStartSearchAt )
{
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );
    return GetStateCache(nSlotId, &nStartSearchAt ) != nullptr;
}


sal_uInt16 SfxBindings::GetSlotPos( sal_uInt16 nId, sal_uInt16 nStartSearchAt )
{
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );

    // answer immediately if a function-seek comes repeated
    if ( pImp->nCachedFunc1 < pImp->pCaches->size() &&
         (*pImp->pCaches)[pImp->nCachedFunc1]->GetId() == nId )
    {
        return pImp->nCachedFunc1;
    }
    if ( pImp->nCachedFunc2 < pImp->pCaches->size() &&
         (*pImp->pCaches)[pImp->nCachedFunc2]->GetId() == nId )
    {
        // swap the caches
        sal_uInt16 nTemp = pImp->nCachedFunc1;
        pImp->nCachedFunc1 = pImp->nCachedFunc2;
        pImp->nCachedFunc2 = nTemp;
        return pImp->nCachedFunc1;
    }

    // binary search, if not found, seek to target-position
    if ( pImp->pCaches->size() <= nStartSearchAt )
    {
        return 0;
    }
    if ( (sal_uInt16) pImp->pCaches->size() == (nStartSearchAt+1) )
    {
        return (*pImp->pCaches)[nStartSearchAt]->GetId() >= nId ? 0 : 1;
    }
    sal_uInt16 nLow = nStartSearchAt;
    sal_uInt16 nMid = 0;
    sal_uInt16 nHigh = 0;
    bool bFound = false;
    nHigh = pImp->pCaches->size() - 1;
    while ( !bFound && nLow <= nHigh )
    {
        nMid = (nLow + nHigh) >> 1;
        DBG_ASSERT( nMid < pImp->pCaches->size(), "bsearch is buggy" );
        int nDiff = (int) nId - (int) ( ((*pImp->pCaches)[nMid])->GetId() );
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
    DBG_ASSERT( nPos <= pImp->pCaches->size(), "" );
    DBG_ASSERT( nPos == pImp->pCaches->size() ||
                nId <= (*pImp->pCaches)[nPos]->GetId(), "" );
    DBG_ASSERT( nPos == nStartSearchAt ||
                nId > (*pImp->pCaches)[nPos-1]->GetId(), "" );
    DBG_ASSERT( ( (nPos+1) >= (sal_uInt16) pImp->pCaches->size() ) ||
                nId < (*pImp->pCaches)[nPos+1]->GetId(), "" );
    pImp->nCachedFunc2 = pImp->nCachedFunc1;
    pImp->nCachedFunc1 = nPos;
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
    DBG_ASSERT( !pImp->bInNextJob, "SfxBindings::Register while status-updating" );

    // insert new cache if it does not already exist
    sal_uInt16 nId = rItem.GetId();
    sal_uInt16 nPos = GetSlotPos(nId);
    if ( nPos >= pImp->pCaches->size() ||
         (*pImp->pCaches)[nPos]->GetId() != nId )
    {
        SfxStateCache* pCache = new SfxStateCache(nId);
        pImp->pCaches->insert( pImp->pCaches->begin() + nPos, pCache );
        DBG_ASSERT( nPos == 0 ||
                    (*pImp->pCaches)[nPos]->GetId() >
                        (*pImp->pCaches)[nPos-1]->GetId(), "" );
        DBG_ASSERT( (nPos == pImp->pCaches->size()-1) ||
                    (*pImp->pCaches)[nPos]->GetId() <
                        (*pImp->pCaches)[nPos+1]->GetId(), "" );
        pImp->bMsgDirty = true;
    }

    // enqueue the new binding
    if ( bInternal )
    {
        (*pImp->pCaches)[nPos]->SetInternalController( &rItem );
    }
    else
    {
        SfxControllerItem *pOldItem = (*pImp->pCaches)[nPos]->ChangeItemLink(&rItem);
        rItem.ChangeItemLink(pOldItem);
    }
}


void SfxBindings::Release( SfxControllerItem& rItem )
{
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );
    DBG_ASSERT( !pImp->bInNextJob, "SfxBindings::Release while status-updating" );
    ENTERREGISTRATIONS();

    // find the bound function
    sal_uInt16 nId = rItem.GetId();
    sal_uInt16 nPos = GetSlotPos(nId);
    SfxStateCache* pCache = (nPos < pImp->pCaches->size()) ? (*pImp->pCaches)[nPos] : nullptr;
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
            pImp->bCtrlReleased = true;
        }
    }

    LEAVEREGISTRATIONS();
}


const SfxPoolItem* SfxBindings::ExecuteSynchron( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi,
            const SfxPoolItem **ppInternalArgs )
{
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return nullptr;

    return Execute_Impl( nId, ppItems, nModi, SfxCallMode::SYNCHRON, ppInternalArgs );
}

bool SfxBindings::Execute( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi, SfxCallMode nCallMode,
                        const SfxPoolItem **ppInternalArgs )
{
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return false;

    const SfxPoolItem* pRet = Execute_Impl( nId, ppItems, nModi, nCallMode, ppInternalArgs );
    return ( pRet != nullptr );
}

const SfxPoolItem* SfxBindings::Execute_Impl( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi, SfxCallMode nCallMode,
                        const SfxPoolItem **ppInternalArgs, bool bGlobalOnly )
{
    SfxStateCache *pCache = GetStateCache( nId );
    if ( !pCache )
    {
        SfxBindings *pBind = pImp->pSubBindings;
        while ( pBind )
        {
            if ( pBind->GetStateCache( nId ) )
                return pBind->Execute_Impl( nId, ppItems, nModi, nCallMode, ppInternalArgs, bGlobalOnly );
            pBind = pBind->pImp->pSubBindings;
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
        pCache->GetSlotServer( rDispatcher, pImp->xProv );
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
    if ( pImp->bMsgDirty )
        UpdateSlotServer_Impl();

    SfxShell *pShell=nullptr;
    const SfxSlot *pSlot=nullptr;

    const SfxSlotServer* pServer = pCache->GetSlotServer( rDispatcher, pImp->xProv );
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

    if ( SFX_KIND_ENUM == pSlot->GetKind() )
    {
        // for Enum-Slots, the Master has to be executed with the value
        // of the enums
        const SfxSlot *pRealSlot = pShell->GetInterface()->GetRealSlot(pSlot);
        const sal_uInt16 nSlotId = pRealSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        aReq.AppendItem( SfxAllEnumItem( rPool.GetWhich(nSlotId), pSlot->GetValue() ) );
        pDispatcher->Execute_( *pShell, *pRealSlot, aReq, aReq.GetCallMode() | SfxCallMode::RECORD );
    }
    else if ( SFX_KIND_ATTR == pSlot->GetKind() )
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
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );

    // synchronize
    pDispatcher->Flush();

    if ( pImp->bAllMsgDirty )
    {
        if ( !nRegLevel )
        {
            css::uno::Reference < css::frame::XFrame > xFrame
                ( pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );
            pImp->bContextChanged = false;
        }
        else
            pImp->bContextChanged = true;
    }

    for (size_t i = 0, nCount = pImp->pCaches->size(); i < nCount; ++i)
    {
        SfxStateCache *pCache = (*pImp->pCaches)[i];
        //GetSlotServer can modify pImp->pCaches
        pCache->GetSlotServer(*pDispatcher, pImp->xProv);
    }
    pImp->bMsgDirty = pImp->bAllMsgDirty = false;

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
    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );

    DBG_ASSERT( !pImp->bMsgDirty, "CreateSet_Impl with dirty MessageServer" );
    assert(pDispatcher);

    const SfxSlotServer* pMsgSvr = pCache->GetSlotServer(*pDispatcher, pImp->xProv);
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
    if ( SFX_KIND_ENUM == pMsgSvr->GetSlot()->GetKind() )
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
    sal_uInt16 nCachePos = pImp->nMsgPos;
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
            const SfxSlotServer *pServ = pSiblingCache->GetSlotServer(*pDispatcher, pImp->xProv);
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

                sal_uInt16 nCurMsgPos = pImp->nMsgPos;
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
    DBG_ASSERT( !rFound.pSlot || SFX_KIND_ENUM != rFound.pSlot->GetKind(),
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
                    rFound.nWhichId > SFX_WHICH_MAX )
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
            DBG_ASSERT(SFX_KIND_ENUM == pSlave->GetKind(),"non enum slaves aren't allowed");
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

    DBG_ASSERT( pImp->pCaches != nullptr, "SfxBindings not initialized" );

    if ( Application::GetLastInputInterval() < MAX_INPUT_DELAY && pTimer )
    {
        pImp->aTimer.SetTimeout(TIMEOUT_UPDATING);
        return true;
    }

    SfxApplication *pSfxApp = SfxGetpApp();

    if( pDispatcher )
        pDispatcher->Update_Impl();

    // modifying the SfxObjectInterface-stack without SfxBindings => nothing to do
    SfxViewFrame* pFrame = pDispatcher ? pDispatcher->GetFrame() : nullptr;
    if ( (pFrame && !pFrame->GetObjectShell()->AcceptStateUpdate()) || pSfxApp->IsDowning() || pImp->pCaches->empty() )
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
    if ( pImp->bMsgDirty && !bTest )
    {
        UpdateSlotServer_Impl();
        return false;
    }

    pImp->bAllDirty = false;
    pImp->aTimer.SetTimeout(TIMEOUT_UPDATING);

    // at least 10 loops and further if more jobs are available but no input
    bool bPreEmptive = pTimer && !pSfxApp->Get_Impl()->nInReschedule;
    sal_uInt16 nLoops = 10;
    pImp->bInNextJob = true;
    const sal_uInt16 nCount = pImp->pCaches->size();
    while ( pImp->nMsgPos < nCount )
    {
        // iterate through the bound functions
        bool bJobDone = false;
        while ( !bJobDone )
        {
            SfxStateCache* pCache = (*pImp->pCaches)[pImp->nMsgPos];
            DBG_ASSERT( pCache, "invalid SfxStateCache-position in job queue" );
            bool bWasDirty = pCache->IsControllerDirty();
            if ( bWasDirty )
            {
                    Update_Impl( pCache );
                    DBG_ASSERT( nCount == pImp->pCaches->size(),
                            "Reschedule in StateChanged => buff" );
            }

            // skip to next function binding
            ++pImp->nMsgPos;

            // keep job if it is not completed, but any input is available
            bJobDone = pImp->nMsgPos >= nCount;
            if ( bJobDone && pImp->bFirstRound )
            {

                // Update of the  preferred shell has been done, now may
                // also the others shells be updated
                bJobDone = false;
                pImp->bFirstRound = false;
                pImp->nMsgPos = 0;
            }

            if ( bWasDirty && !bJobDone && bPreEmptive && (--nLoops == 0) )
            {
                pImp->bInNextJob = false;
                return false;
            }
        }
    }

    pImp->nMsgPos = 0;

    // check for volatile slots
    bool bVolatileSlotsPresent = false;
    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        SfxStateCache* pCache = (*pImp->pCaches)[n];
        const SfxSlotServer *pSlotServer = pCache->GetSlotServer(*pDispatcher, pImp->xProv);
        if ( pSlotServer && pSlotServer->GetSlot()->IsMode(SfxSlotMode::VOLATILE) )
        {
            pCache->Invalidate(false);
            bVolatileSlotsPresent = true;
        }
    }

    if (bVolatileSlotsPresent)
        pImp->aTimer.SetTimeout(TIMEOUT_IDLE);
    else
        pImp->aTimer.Stop();

    // Update round is finished
    pImp->bInNextJob = false;
    Broadcast(SfxSimpleHint(SFX_HINT_UPDATEDONE));
    return true;
#ifdef DBG_UTIL
    }
    catch (...)
    {
        OSL_FAIL("C++ exception caught!");
        pImp->bInNextJob = false;
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
    if ( pImp->pSubBindings )
    {
        pImp->pSubBindings->ENTERREGISTRATIONS();

        // These EnterRegistrations are not "real" for the SubBindings
        pImp->pSubBindings->pImp->nOwnRegLevel--;

        // Synchronize Bindings
        pImp->pSubBindings->nRegLevel = nRegLevel + pImp->pSubBindings->pImp->nOwnRegLevel + 1;
    }

    pImp->nOwnRegLevel++;

    // check if this is the outer most level
    if ( ++nRegLevel == 1 )
    {
        // stop background-processing
        pImp->aTimer.Stop();

        // flush the cache
        pImp->nCachedFunc1 = 0;
        pImp->nCachedFunc2 = 0;

        // Mark if the all of the Caches have disappeared.
        pImp->bCtrlReleased = false;
    }

    return nRegLevel;
}


void SfxBindings::LeaveRegistrations( sal_uInt16 nLevel, const char *pFile, int nLine )
{
    (void)nLevel; // unused variable
    DBG_ASSERT( nRegLevel, "Leave without Enter" );
    DBG_ASSERT( nLevel == USHRT_MAX || nLevel == nRegLevel, "wrong Leave" );

    // Only when the SubBindings are still locked by the Superbindings,
    // remove this lock (i.e. if there are more locks than "real" ones)
    if ( pImp->pSubBindings && pImp->pSubBindings->nRegLevel > pImp->pSubBindings->pImp->nOwnRegLevel )
    {
        // Synchronize Bindings
        pImp->pSubBindings->nRegLevel = nRegLevel + pImp->pSubBindings->pImp->nOwnRegLevel;

        // This LeaveRegistrations is not "real" for SubBindings
        pImp->pSubBindings->pImp->nOwnRegLevel++;
        pImp->pSubBindings->LEAVEREGISTRATIONS();
    }

    pImp->nOwnRegLevel--;

    // check if this is the outer most level
    if ( --nRegLevel == 0 && !SfxGetpApp()->IsDowning() )
    {
        if ( pImp->bContextChanged )
        {
            pImp->bContextChanged = false;
        }

        SfxViewFrame* pFrame = pDispatcher->GetFrame();

        // If possible remove unused Caches, for example prepare PlugInInfo
        if ( pImp->bCtrlReleased )
        {
            for ( sal_uInt16 nCache = pImp->pCaches->size(); nCache > 0; --nCache )
            {
                // Get Cache via css::sdbcx::Index
                SfxStateCache *pCache = (*pImp->pCaches)[nCache-1];

                // No interested Controller present
                if ( pCache->GetItemLink() == nullptr && !pCache->GetInternalController() )
                {
                    // Remove Cache. Safety: first remove and then delete
                    pImp->pCaches->erase(pImp->pCaches->begin() + nCache - 1);
                    delete pCache;
                }
            }
        }

        // restart background-processing
        pImp->nMsgPos = 0;
        if ( !pFrame || !pFrame->GetObjectShell() )
            return;
        if ( pImp->pCaches && !pImp->pCaches->empty() )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
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
                if ( pBind->pImp->pSubBindings == this && pBind->pDispatcher != pDisp )
                    pBind->SetSubBindings_Impl( nullptr );
                pBind = pBind->pImp->pSubBindings;
            }
        }

        pDispatcher = pDisp;

        css::uno::Reference < css::frame::XDispatchProvider > xProv;
        if ( pDisp )
            xProv.set( pDisp->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );

        SetDispatchProvider_Impl( xProv );
        InvalidateAll( true );
        InvalidateUnoControllers_Impl();

        if ( pDispatcher && !pOldDispat )
        {
            if ( pImp->pSubBindings && pImp->pSubBindings->pDispatcher != pOldDispat )
            {
                OSL_FAIL( "SubBindings already set before activating!" );
                pImp->pSubBindings->ENTERREGISTRATIONS();
            }
            LEAVEREGISTRATIONS();
        }
        else if( !pDispatcher )
        {
            ENTERREGISTRATIONS();
            if ( pImp->pSubBindings && pImp->pSubBindings->pDispatcher != pOldDispat )
            {
                OSL_FAIL( "SubBindings still set even when deactivating!" );
                pImp->pSubBindings->LEAVEREGISTRATIONS();
            }
        }

        Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

        if ( pDisp )
        {
            SfxBindings* pBind = pDisp->GetBindings();
            while ( pBind && pBind != this )
            {
                if ( !pBind->pImp->pSubBindings )
                {
                    pBind->SetSubBindings_Impl( this );
                    break;
                }

                pBind = pBind->pImp->pSubBindings;
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
    if ( pImp->pSubBindings )
        pImp->pSubBindings->StartUpdate_Impl( bComplete );

    if ( !bComplete )
        // Update may be interrupted
        NextJob_Impl(&pImp->aTimer);
    else
        // Update all slots in a row
        NextJob_Impl(nullptr);
}



SfxItemState SfxBindings::QueryState( sal_uInt16 nSlot, SfxPoolItem* &rpState )
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
            xDisp = pImp->xProv->queryDispatch( aURL, OUString(), 0 );

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
                    pCache->GetSlotServer( *GetDispatcher_Impl(), pImp->xProv );
                    bDeleteCache = true;
                }

                SfxItemState eState = SfxItemState::SET;
                SfxPoolItem *pItem=nullptr;
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
                        pItem = new SfxBoolItem( nSlot, bTemp );
                    }
                    else if ( aType == ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get() )
                    {
                        sal_uInt16 nTemp = 0;
                        aAny >>= nTemp ;
                        pItem = new SfxUInt16Item( nSlot, nTemp );
                    }
                    else if ( aType == cppu::UnoType<sal_uInt32>::get() )
                    {
                        sal_uInt32 nTemp = 0;
                        aAny >>= nTemp ;
                        pItem = new SfxUInt32Item( nSlot, nTemp );
                    }
                    else if ( aType == cppu::UnoType<OUString>::get() )
                    {
                        OUString sTemp ;
                        aAny >>= sTemp ;
                        pItem = new SfxStringItem( nSlot, sTemp );
                    }
                    else
                        pItem = new SfxVoidItem( nSlot );
                }

                xDisp->removeStatusListener( pBind, aURL );
                pBind->Release();
                rpState = pItem;
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
            rpState = pItem->Clone();
    }
    else if ( eState == SfxItemState::DEFAULT && pItem )
    {
        rpState = pItem->Clone();
    }

    return eState;
}

void SfxBindings::SetSubBindings_Impl( SfxBindings *pSub )
{
    if ( pImp->pSubBindings )
    {
        pImp->pSubBindings->SetDispatchProvider_Impl( css::uno::Reference< css::frame::XDispatchProvider > () );
        pImp->pSubBindings->pImp->pSuperBindings = nullptr;
    }

    pImp->pSubBindings = pSub;

    if ( pSub )
    {
        pImp->pSubBindings->SetDispatchProvider_Impl( pImp->xProv );
        pSub->pImp->pSuperBindings = this;
    }
}

SfxBindings* SfxBindings::GetSubBindings_Impl( bool bTop ) const
{
    SfxBindings *pRet = pImp->pSubBindings;
    if ( bTop )
    {
        while ( pRet->pImp->pSubBindings )
            pRet = pRet->pImp->pSubBindings;
    }

    return pRet;
}

void SfxBindings::SetWorkWindow_Impl( SfxWorkWindow* pWork )
{
    pImp->pWorkWin = pWork;
}

SfxWorkWindow* SfxBindings::GetWorkWindow_Impl() const
{
    return pImp->pWorkWin;
}

void SfxBindings::RegisterUnoController_Impl( SfxUnoControllerItem* pControl )
{
    if ( !pImp->pUnoCtrlArr )
        pImp->pUnoCtrlArr = new SfxUnoControllerArr_Impl;
    pImp->pUnoCtrlArr->push_back( pControl );
}

void SfxBindings::ReleaseUnoController_Impl( SfxUnoControllerItem* pControl )
{
    if ( pImp->pUnoCtrlArr )
    {
        SfxUnoControllerArr_Impl::iterator it = std::find(
            pImp->pUnoCtrlArr->begin(), pImp->pUnoCtrlArr->end(), pControl );
        if ( it != pImp->pUnoCtrlArr->end() )
        {
            pImp->pUnoCtrlArr->erase( it );
            return;
        }
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->ReleaseUnoController_Impl( pControl );
}

void SfxBindings::InvalidateUnoControllers_Impl()
{
    if ( pImp->pUnoCtrlArr )
    {
        sal_uInt16 nCount = pImp->pUnoCtrlArr->size();
        for ( sal_uInt16 n=nCount; n>0; n-- )
        {
            SfxUnoControllerItem *pCtrl = (*pImp->pUnoCtrlArr)[n-1];
            css::uno::Reference< css::frame::XStatusListener >  xRef( static_cast<cppu::OWeakObject*>(pCtrl), css::uno::UNO_QUERY );
            pCtrl->ReleaseDispatch();
            pCtrl->GetNewDispatch();
        }
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateUnoControllers_Impl();
}

bool SfxBindings::IsInUpdate() const
{
    bool bInUpdate = pImp->bInUpdate;
    if ( !bInUpdate && pImp->pSubBindings )
        bInUpdate = pImp->pSubBindings->IsInUpdate();
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
    const css::uno::Reference< css::frame::XFrame > xFrame( pImp->xProv, css::uno::UNO_QUERY );
    if ( xFrame.is() || !pDispatcher )
        return xFrame;
    else
        return pDispatcher->GetFrame()->GetFrame().GetFrameInterface();
}

void SfxBindings::SetDispatchProvider_Impl( const css::uno::Reference< css::frame::XDispatchProvider > & rProv )
{
    bool bInvalidate = ( rProv != pImp->xProv );
    if ( bInvalidate )
    {
        pImp->xProv = rProv;
        InvalidateAll( true );
        InvalidateUnoControllers_Impl();
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->SetDispatchProvider_Impl( pImp->xProv );
}

bool SfxBindings::ExecuteCommand_Impl( const OUString& rCommand )
{
    css::util::URL aURL;
    aURL.Complete = rCommand;
    Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aURL );
    css::uno::Reference< css::frame::XDispatch >  xDisp = pImp->xProv->queryDispatch( aURL, OUString(), 0 );
    if ( xDisp.is() )
    {
        new SfxAsyncExec_Impl( aURL, xDisp );
        return true;
    }

    return false;
}

css::uno::Reference< css::frame::XDispatchRecorder > SfxBindings::GetRecorder() const
{
    return pImp->xRecorder;
}

void SfxBindings::SetRecorder_Impl( css::uno::Reference< css::frame::XDispatchRecorder >& rRecorder )
{
    pImp->xRecorder = rRecorder;
}

void SfxBindings::ContextChanged_Impl()
{
    if ( !pImp->bInUpdate && ( !pImp->bContextChanged || !pImp->bAllMsgDirty ) )
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
