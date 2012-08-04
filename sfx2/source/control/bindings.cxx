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

#include "sal/config.h"

#include <iomanip>

#include <boost/unordered_map.hpp>
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
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <comphelper/processfactory.hxx>
#include <svtools/itemdel.hxx>

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

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

DBG_NAME(SfxBindingsMsgPos)
DBG_NAME(SfxBindingsUpdateServers)
DBG_NAME(SfxBindingsCreateSet)
DBG_NAME(SfxBindingsUpdateCtrl1)
DBG_NAME(SfxBindingsUpdateCtrl2)
DBG_NAME(SfxBindingsNextJob_Impl0)
DBG_NAME(SfxBindingsNextJob_Impl)
DBG_NAME(SfxBindingsUpdate_Impl)
DBG_NAME(SfxBindingsInvalidateAll)

//====================================================================

static sal_uInt16 nTimeOut = 300;

#define TIMEOUT_FIRST       nTimeOut
#define TIMEOUT_UPDATING     20
#define TIMEOUT_IDLE       2500

typedef boost::unordered_map< sal_uInt16, bool > InvalidateSlotMap;

//====================================================================

DECL_PTRARRAY(SfxStateCacheArr_Impl, SfxStateCache*, 32, 16)

//====================================================================

class SfxAsyncExec_Impl
{
    ::com::sun::star::util::URL aCommand;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDisp;
    Timer           aTimer;

public:

    SfxAsyncExec_Impl( const ::com::sun::star::util::URL& rCmd, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& rDisp )
        : aCommand( rCmd )
        , xDisp( rDisp )
    {
        aTimer.SetTimeoutHdl( LINK(this, SfxAsyncExec_Impl, TimerHdl) );
        aTimer.SetTimeout( 0 );
        aTimer.Start();
    }

    DECL_LINK( TimerHdl, Timer*);
};

IMPL_LINK(SfxAsyncExec_Impl, TimerHdl, Timer*, pTimer)
{
    (void)pTimer; // unused
    aTimer.Stop();

    Sequence<beans::PropertyValue> aSeq;
    xDisp->dispatch( aCommand, aSeq );

    delete this;
    return 0L;
}

class SfxBindings_Impl
{
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchRecorder > xRecorder;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  xProv;
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
    sal_Bool                bContextChanged;
    sal_Bool                bMsgDirty;      // Has a MessageServer been invalidated?
    sal_Bool                bAllMsgDirty;   //  Has a MessageServer been invalidated?
    sal_Bool                bAllDirty;      // After InvalidateAll
    sal_Bool                bCtrlReleased;  // while EnterRegistrations
    AutoTimer               aTimer;         // for volatile Slots
    sal_Bool                bInUpdate;      // for Assertions
    sal_Bool                bInNextJob;     // for Assertions
    sal_Bool                bFirstRound;    // First round in Update
    sal_uInt16              nFirstShell;    // Shell, the first round is prefered
    sal_uInt16              nOwnRegLevel;   // Counts the real Locks, exept those of the Super Bindings
    InvalidateSlotMap       m_aInvalidateSlots; // store slots which are invalidated while in update
};

//--------------------------------------------------------------------

struct SfxFoundCache_Impl
{
    sal_uInt16      nSlotId;   // the Slot-Id
    sal_uInt16      nWhichId;  // If available: Which-Id, else: nSlotId
    const SfxSlot*  pSlot;     // Pointer to <Master-Slot>
    SfxStateCache*  pCache;    // Pointer to StatusCache, if possible NULL

    SfxFoundCache_Impl():
        nSlotId(0),
        nWhichId(0),
        pSlot(0),
        pCache(0)
    {}

    SfxFoundCache_Impl(SfxFoundCache_Impl&r):
        nSlotId(r.nSlotId),
        nWhichId(r.nWhichId),
        pSlot(r.pSlot),
        pCache(r.pCache)
    {}

    SfxFoundCache_Impl(sal_uInt16 nS, sal_uInt16 nW, const SfxSlot *pS, SfxStateCache *pC ):
        nSlotId(nS),
        nWhichId(nW),
        pSlot(pS),
        pCache(pC)
    {}

    int operator<( const SfxFoundCache_Impl &r ) const
    { return nWhichId < r.nWhichId; }

    int operator==( const SfxFoundCache_Impl &r ) const
    { return nWhichId== r.nWhichId; }
};

//--------------------------------------------------------------------------

class SfxFoundCacheArr_Impl : public std::vector<SfxFoundCache_Impl*>
{
public:
    ~SfxFoundCacheArr_Impl()
    {
        for(const_iterator it = begin(); it != end(); ++it)
            delete *it;
    }
};

//==========================================================================

SfxBindings::SfxBindings()
:   pImp(new SfxBindings_Impl),
    pDispatcher(0),
    nRegLevel(1)    // first becomes 0, when the Dispatcher is set
{
    pImp->nMsgPos = 0;
    pImp->bAllMsgDirty = sal_True;
    pImp->bContextChanged = sal_False;
    pImp->bMsgDirty = sal_True;
    pImp->bAllDirty = sal_True;
    pImp->ePopupAction = SFX_POPUP_DELETE;
    pImp->nCachedFunc1 = 0;
    pImp->nCachedFunc2 = 0;
    pImp->bCtrlReleased = sal_False;
    pImp->bFirstRound = sal_False;
    pImp->bInNextJob = sal_False;
    pImp->bInUpdate = sal_False;
    pImp->pSubBindings = NULL;
    pImp->pSuperBindings = NULL;
    pImp->pWorkWin = NULL;
    pImp->pUnoCtrlArr = NULL;
    pImp->nOwnRegLevel = nRegLevel;

    // all caches are valid (no pending invalidate-job)
    // create the list of caches
    pImp->pCaches = new SfxStateCacheArr_Impl;
    pImp->aTimer.SetTimeoutHdl( LINK(this, SfxBindings, NextJob_Impl) );
}

//====================================================================

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
    pImp->pSubBindings = NULL;

    ENTERREGISTRATIONS();

    pImp->aTimer.Stop();
    DeleteControllers_Impl();

    // Delete Caches
    sal_uInt16 nCount = pImp->pCaches->Count();
    for ( sal_uInt16 nCache = 0; nCache < nCount; ++nCache )
        delete pImp->pCaches->GetObject(nCache);

    DELETEZ( pImp->pWorkWin );

    delete pImp->pCaches;
    delete pImp;
}

//--------------------------------------------------------------------

void SfxBindings::DeleteControllers_Impl()
{
    // in the first round delete SfxPopupWindows
    sal_uInt16 nCount = pImp->pCaches->Count();
    sal_uInt16 nCache;
    for ( nCache = 0; nCache < nCount; ++nCache )
    {
        // Remember were you are
        SfxStateCache *pCache = pImp->pCaches->GetObject(nCache);
        sal_uInt16 nSlotId = pCache->GetId();

        // Delete SfxPopupWindow
        pCache->DeleteFloatingWindows();

        // Re-align, because the cache may have been reduced
        sal_uInt16 nNewCount = pImp->pCaches->Count();
        if ( nNewCount < nCount )
        {
            nCache = GetSlotPos(nSlotId);
            if ( nCache >= nNewCount ||
                 nSlotId != pImp->pCaches->GetObject(nCache)->GetId() )
                --nCache;
            nCount = nNewCount;
        }
    }

    // Delete all Caches
    for ( nCache = pImp->pCaches->Count(); nCache > 0; --nCache )
    {
        // Get Cache via ::com::sun::star::sdbcx::Index
        SfxStateCache *pCache = pImp->pCaches->GetObject(nCache-1);

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
        if( nCache-1 < pImp->pCaches->Count() )
            delete (*pImp->pCaches)[nCache-1];
        pImp->pCaches->Remove(nCache-1, 1);
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

//--------------------------------------------------------------------

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
        pImp->pWorkWin->HidePopups_Impl( bHide, sal_True );
}

void SfxBindings::HidePopupCtrls_Impl( bool bHide )
{
    if ( bHide )
    {
        // Hide SfxPopupWindows
        pImp->ePopupAction = SFX_POPUP_HIDE;
    }
    else
    {
        // Show SfxPopupWindows
        pImp->ePopupAction = SFX_POPUP_SHOW;
    }

    for ( sal_uInt16 nCache = 0; nCache < pImp->pCaches->Count(); ++nCache )
        pImp->pCaches->GetObject(nCache)->DeleteFloatingWindows();
    pImp->ePopupAction = SFX_POPUP_DELETE;
}

//--------------------------------------------------------------------

void SfxBindings::Update_Impl
(
    SfxStateCache*  pCache      // The up to date SfxStatusCache
)
{
    if( pCache->GetDispatch().is() && pCache->GetItemLink() )
    {
        pCache->SetCachedState(sal_True);
        if ( !pCache->GetInternalController() )
            return;
    }

    if ( !pDispatcher )
        return;
    DBG_PROFSTART(SfxBindingsUpdate_Impl);

    // gather together all with the same status method which are dirty
    SfxDispatcher &rDispat = *pDispatcher;
    const SfxSlot *pRealSlot = 0;
    const SfxSlotServer* pMsgServer = 0;
    SfxFoundCacheArr_Impl aFound;
    SfxItemSet *pSet = CreateSet_Impl( pCache, pRealSlot, &pMsgServer, aFound );
    sal_Bool bUpdated = sal_False;
    if ( pSet )
    {
        // Query Status
        if ( rDispat._FillState( *pMsgServer, *pSet, pRealSlot ) )
        {
            // Post Status
            const SfxInterface *pInterface =
                rDispat.GetShell(pMsgServer->GetShellLevel())->GetInterface();
            for ( sal_uInt16 nPos = 0; nPos < aFound.size(); ++nPos )
            {
                const SfxFoundCache_Impl *pFound = aFound[nPos];
                sal_uInt16 nWhich = pFound->nWhichId;
                const SfxPoolItem *pItem = 0;
                SfxItemState eState = pSet->GetItemState(nWhich, sal_True, &pItem);
                if ( eState == SFX_ITEM_DEFAULT && SfxItemPool::IsWhich(nWhich) )
                    pItem = &pSet->Get(nWhich);
                UpdateControllers_Impl( pInterface, aFound[nPos], pItem, eState );
            }
            bUpdated = sal_True;
        }

        delete pSet;
    }

    if ( !bUpdated && pCache )
    {
        // When pCache == NULL and no SlotServer
        // (for example due to locked Dispatcher! ),
        // obviously do not try to update
        SfxFoundCache_Impl aFoundCache(
                            pCache->GetId(), 0,
                            pRealSlot, pCache );
        UpdateControllers_Impl( 0, &aFoundCache, 0, SFX_ITEM_DISABLED);
    }

    DBG_PROFSTOP(SfxBindingsUpdate_Impl);
}

//--------------------------------------------------------------------

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

//--------------------------------------------------------------------

void SfxBindings::AddSlotToInvalidateSlotsMap_Impl( sal_uInt16 nId )
{
    pImp->m_aInvalidateSlots[nId] = sal_True;
}

//--------------------------------------------------------------------

void SfxBindings::Update
(
    sal_uInt16      nId     // the bound and up-to-date Slot-Id
)
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if ( pDispatcher )
        pDispatcher->Flush();

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Update( nId );

    SfxStateCache* pCache = GetStateCache( nId );
    if ( pCache )
    {
        pImp->bInUpdate = sal_True;
        if ( pImp->bMsgDirty )
        {
            UpdateSlotServer_Impl();
            pCache = GetStateCache( nId );
        }

        if (pCache)
        {
            sal_Bool bInternalUpdate = sal_True;
            if( pCache->GetDispatch().is() && pCache->GetItemLink() )
            {
                pCache->SetCachedState(sal_True);
                bInternalUpdate = ( pCache->GetInternalController() != 0 );
            }

            if ( bInternalUpdate )
            {
                // Query Status
                const SfxSlotServer* pMsgServer = pCache->GetSlotServer(*pDispatcher, pImp->xProv);
                if ( !pCache->IsControllerDirty() &&
                    ( !pMsgServer ||
                    !pMsgServer->GetSlot()->IsMode(SFX_SLOT_VOLATILE) ) )
                {
                    pImp->bInUpdate = sal_False;
                    InvalidateSlotsInMap_Impl();
                    return;
                }
                if (!pMsgServer)
                {
                    pCache->SetState(SFX_ITEM_DISABLED, 0);
                    pImp->bInUpdate = sal_False;
                    InvalidateSlotsInMap_Impl();
                    return;
                }

                Update_Impl(pCache);
            }

            pImp->bAllDirty = sal_False;
        }

        pImp->bInUpdate = sal_False;
        InvalidateSlotsInMap_Impl();
    }
}

//--------------------------------------------------------------------

void SfxBindings::Update()
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Update();

    if ( pDispatcher )
    {
        if ( nRegLevel )
            return;

        pImp->bInUpdate = sal_True;
        pDispatcher->Flush();
        pDispatcher->Update_Impl();
        while ( !NextJob_Impl(0) )
            ; // loop
        pImp->bInUpdate = sal_False;
        InvalidateSlotsInMap_Impl();
    }
}

//--------------------------------------------------------------------

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
                    pCache->Invalidate(sal_False);
                pCache->SetState( SFX_ITEM_AVAILABLE, pItem );

                //! Not implemented: Updates from EnumSlots via master slots
            }
        }
    }
}

//--------------------------------------------------------------------

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
                pCache->Invalidate(sal_False);
            pCache->SetState( SFX_ITEM_AVAILABLE, &rItem );

            //! Not implemented: Updates from EnumSlots via master slots
        }
    }
}


//--------------------------------------------------------------------

SfxStateCache* SfxBindings::GetAnyStateCache_Impl( sal_uInt16 nId )
{
    SfxStateCache* pCache = GetStateCache( nId );
    if ( !pCache && pImp->pSubBindings )
        return pImp->pSubBindings->GetAnyStateCache_Impl( nId );
    return pCache;
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
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    // is the specified function bound?
    const sal_uInt16 nStart = ( pPos ? *pPos : 0 );
    const sal_uInt16 nPos = GetSlotPos( nId, nStart );

    if ( nPos < pImp->pCaches->Count() &&
         (*pImp->pCaches)[nPos]->GetId() == nId )
    {
        if ( pPos )
            *pPos = nPos;
        return (*pImp->pCaches)[nPos];
    }
    return 0;
}

//--------------------------------------------------------------------

void SfxBindings::InvalidateAll
(
    sal_Bool  bWithMsg  /* sal_True   Mark Slot Server as invalid
                           sal_False  Slot Server remains valid */
)
{
    DBG_PROFSTART(SfxBindingsInvalidateAll);
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateAll( bWithMsg );

    // everything is already set dirty or downing => nothing to do
    if ( !pDispatcher ||
         ( pImp->bAllDirty && ( !bWithMsg || pImp->bAllMsgDirty ) ) ||
         SFX_APP()->IsDowning() )
    {
        DBG_PROFSTOP(SfxBindingsInvalidateAll);
        return;
    }

    pImp->bAllMsgDirty = pImp->bAllMsgDirty || bWithMsg;
    pImp->bMsgDirty = pImp->bMsgDirty || pImp->bAllMsgDirty || bWithMsg;
    pImp->bAllDirty = sal_True;

    for ( sal_uInt16 n = 0; n < pImp->pCaches->Count(); ++n )
        pImp->pCaches->GetObject(n)->Invalidate(bWithMsg);

    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.Stop();
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}

//--------------------------------------------------------------------

void SfxBindings::Invalidate
(
    const sal_uInt16* pIds /* numerically sorted NULL-terminated array of
                              slot IDs (individual, not as a couple!) */
)
{
    DBG_PROFSTART(SfxBindingsInvalidateAll);

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
    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    // Search binary in always smaller areas
    for ( sal_uInt16 n = GetSlotPos(*pIds);
          *pIds && n < pImp->pCaches->Count();
          n = GetSlotPos(*pIds, n) )
    {
        // If SID is ever bound, then invalidate the cache
        SfxStateCache *pCache = pImp->pCaches->GetObject(n);
        if ( pCache->GetId() == *pIds )
            pCache->Invalidate(sal_False);

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

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}

//--------------------------------------------------------------------

void SfxBindings::InvalidateShell
(
    const SfxShell&  rSh,  /* <SfxShell>, which Slot-Ids should be
                              invalidated */
    sal_Bool         bDeep /* sal_True
                              also inherited slot IDs of SfxShell are invalidert

                              sal_False
                              the inherited and not overloaded Slot-Ids were
                              invalidiert */
                             // for now always bDeep
)
{
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateShell( rSh, bDeep );

    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    DBG_PROFSTART(SfxBindingsInvalidateAll);

    // flush now already, it is done in GetShellLevel (rsh) anyway,
    // important so that is set correctly: pimp-> ball(Msg)Dirty
    pDispatcher->Flush();

    if ( !pDispatcher ||
         ( pImp->bAllDirty && pImp->bAllMsgDirty ) ||
         SFX_APP()->IsDowning() )
    {
        // if the next one is anyway, then all the servers are collected
        return;
    }

    // Find Level
    sal_uInt16 nLevel = pDispatcher->GetShellLevel(rSh);
    if ( nLevel != USHRT_MAX )
    {
        for ( sal_uInt16 n = 0; n < pImp->pCaches->Count(); ++n )
        {
            SfxStateCache *pCache = pImp->pCaches->GetObject(n);
            const SfxSlotServer *pMsgServer =
                pCache->GetSlotServer(*pDispatcher, pImp->xProv);
            if ( pMsgServer && pMsgServer->GetShellLevel() == nLevel )
                pCache->Invalidate(sal_False);
        }
        pImp->nMsgPos = 0;
        if ( !nRegLevel )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
            pImp->bFirstRound = sal_True;
            pImp->nFirstShell = nLevel;
        }
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}

//--------------------------------------------------------------------

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

    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        pCache->Invalidate(sal_False);
        pImp->nMsgPos = Min(GetSlotPos(nId), pImp->nMsgPos);
        if ( !nRegLevel )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
        }
    }
}

//--------------------------------------------------------------------

void SfxBindings::Invalidate
(
    sal_uInt16  nId,                // Status value to be set
    sal_Bool    bWithItem,          // Clear StateCache?
    sal_Bool    bWithMsg            // Get new SlotServer?
)
{
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Invalidate( nId, bWithItem, bWithMsg );

    if ( SFX_APP()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        if ( bWithItem )
            pCache->ClearCache();
        pCache->Invalidate(bWithMsg);

        if ( !pDispatcher || pImp->bAllDirty )
            return;

        pImp->nMsgPos = Min(GetSlotPos(nId), pImp->nMsgPos);
        if ( !nRegLevel )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
        }
    }
}

//--------------------------------------------------------------------

sal_Bool SfxBindings::IsBound( sal_uInt16 nSlotId, sal_uInt16 nStartSearchAt )
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    return GetStateCache(nSlotId, &nStartSearchAt ) != 0;
}

//--------------------------------------------------------------------

sal_uInt16 SfxBindings::GetSlotPos( sal_uInt16 nId, sal_uInt16 nStartSearchAt )
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    DBG_PROFSTART(SfxBindingsMsgPos);

    // answer immediately if a function-seek comes repeated
    if ( pImp->nCachedFunc1 < pImp->pCaches->Count() &&
         (*pImp->pCaches)[pImp->nCachedFunc1]->GetId() == nId )
    {
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return pImp->nCachedFunc1;
    }
    if ( pImp->nCachedFunc2 < pImp->pCaches->Count() &&
         (*pImp->pCaches)[pImp->nCachedFunc2]->GetId() == nId )
    {
        // swap the caches
        sal_uInt16 nTemp = pImp->nCachedFunc1;
        pImp->nCachedFunc1 = pImp->nCachedFunc2;
        pImp->nCachedFunc2 = nTemp;
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return pImp->nCachedFunc1;
    }

    // binary search, if not found, seek to target-position
    if ( pImp->pCaches->Count() <= nStartSearchAt )
    {
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return 0;
    }
    if ( pImp->pCaches->Count() == (nStartSearchAt+1) )
    {
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return (*pImp->pCaches)[nStartSearchAt]->GetId() >= nId ? 0 : 1;
    }
    sal_uInt16 nLow = nStartSearchAt;
    sal_uInt16 nMid = 0;
    sal_uInt16 nHigh = 0;
    sal_Bool bFound = sal_False;
    nHigh = pImp->pCaches->Count() - 1;
    while ( !bFound && nLow <= nHigh )
    {
        nMid = (nLow + nHigh) >> 1;
        DBG_ASSERT( nMid < pImp->pCaches->Count(), "bsearch is buggy" );
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
            bFound = sal_True;
    }
    sal_uInt16 nPos = bFound ? nMid : nLow;
    DBG_ASSERT( nPos <= pImp->pCaches->Count(), "" );
    DBG_ASSERT( nPos == pImp->pCaches->Count() ||
                nId <= (*pImp->pCaches)[nPos]->GetId(), "" );
    DBG_ASSERT( nPos == nStartSearchAt ||
                nId > (*pImp->pCaches)[nPos-1]->GetId(), "" );
    DBG_ASSERT( ( (nPos+1) >= pImp->pCaches->Count() ) ||
                nId < (*pImp->pCaches)[nPos+1]->GetId(), "" );
    pImp->nCachedFunc2 = pImp->nCachedFunc1;
    pImp->nCachedFunc1 = nPos;
    DBG_PROFSTOP(SfxBindingsMsgPos);
    return nPos;
}
//--------------------------------------------------------------------
void SfxBindings::RegisterInternal_Impl( SfxControllerItem& rItem )
{
    Register_Impl( rItem, sal_True );

}

void SfxBindings::Register( SfxControllerItem& rItem )
{
    Register_Impl( rItem, sal_False );
}

void SfxBindings::Register_Impl( SfxControllerItem& rItem, sal_Bool bInternal )
{
    DBG_ASSERT( nRegLevel > 0, "registration without EnterRegistrations" );
    DBG_ASSERT( !pImp->bInNextJob, "SfxBindings::Register while status-updating" );

    // insert new cache if it does not already exist
    sal_uInt16 nId = rItem.GetId();
    sal_uInt16 nPos = GetSlotPos(nId);
    if ( nPos >= pImp->pCaches->Count() ||
         (*pImp->pCaches)[nPos]->GetId() != nId )
    {
        SfxStateCache* pCache = new SfxStateCache(nId);
        pImp->pCaches->Insert( nPos, pCache );
        DBG_ASSERT( nPos == 0 ||
                    (*pImp->pCaches)[nPos]->GetId() >
                        (*pImp->pCaches)[nPos-1]->GetId(), "" );
        DBG_ASSERT( (nPos == pImp->pCaches->Count()-1) ||
                    (*pImp->pCaches)[nPos]->GetId() <
                        (*pImp->pCaches)[nPos+1]->GetId(), "" );
        pImp->bMsgDirty = sal_True;
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

//--------------------------------------------------------------------

void SfxBindings::Release( SfxControllerItem& rItem )
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    DBG_ASSERT( !pImp->bInNextJob, "SfxBindings::Release while status-updating" );
    ENTERREGISTRATIONS();

    // find the bound function
    sal_uInt16 nId = rItem.GetId();
    sal_uInt16 nPos = GetSlotPos(nId);
    SfxStateCache* pCache = (*pImp->pCaches)[nPos];
    if ( pCache->GetId() == nId )
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
        if ( pCache->GetItemLink() == 0 && !pCache->GetInternalController() )
        {
            pImp->bCtrlReleased = sal_True;
        }
    }

    LEAVEREGISTRATIONS();
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxBindings::ExecuteSynchron( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi,
            const SfxPoolItem **ppInternalArgs )
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return NULL;

    return Execute_Impl( nId, ppItems, nModi, SFX_CALLMODE_SYNCHRON, ppInternalArgs );
}

sal_Bool SfxBindings::Execute( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi, SfxCallMode nCallMode,
                        const SfxPoolItem **ppInternalArgs )
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return sal_False;

    const SfxPoolItem* pRet = Execute_Impl( nId, ppItems, nModi, nCallMode, ppInternalArgs );
    return ( pRet != 0 );
}

const SfxPoolItem* SfxBindings::Execute_Impl( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi, SfxCallMode nCallMode,
                        const SfxPoolItem **ppInternalArgs, sal_Bool bGlobalOnly )
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
        };
    }

    SfxDispatcher &rDispatcher = *pDispatcher;
    rDispatcher.Flush();
    rDispatcher.GetFrame();  // -Wall is this required???

    // get SlotServer (Slot+ShellLevel) and Shell from cache
    ::boost::scoped_ptr<SfxStateCache> xCache;
    if ( !pCache )
    {
        // Execution of non cached slots (Accelerators don't use Controllers)
        // slot is uncached, use SlotCache to handle external dispatch providers
        xCache.reset(new SfxStateCache(nId));
        pCache = xCache.get();
        pCache->GetSlotServer( rDispatcher, pImp->xProv );
    }

    if ( pCache && pCache->GetDispatch().is() )
    {
        DBG_ASSERT( !ppInternalArgs, "Internal args get lost when dispatched!" );

        SfxItemPool &rPool = GetDispatcher()->GetFrame()->GetObjectShell()->GetPool();
        SfxRequest aReq( nId, nCallMode, rPool );
        aReq.SetModifier( nModi );
        if( ppItems )
            while( *ppItems )
                aReq.AppendItem( **ppItems++ );

        // cache binds to an external dispatch provider
        pCache->Dispatch( aReq.GetArgs(), nCallMode == SFX_CALLMODE_SYNCHRON );
        SfxPoolItem *pVoid = new SfxVoidItem( nId );
        DeleteItemOnIdle( pVoid );
        return pVoid;
    }

    // slot is handled internally by SfxDispatcher
    if ( pImp->bMsgDirty )
        UpdateSlotServer_Impl();

    SfxShell *pShell=0;
    const SfxSlot *pSlot=0;

    // if slot was uncached, we should have created a cache in this method!
    DBG_ASSERT( pCache, "This code needs a cache!");
    const SfxSlotServer* pServer = pCache ? pCache->GetSlotServer( rDispatcher, pImp->xProv ) : 0;
    if ( !pServer )
    {
        return NULL;
    }
    else
    {
        pShell = rDispatcher.GetShell( pServer->GetShellLevel() );
        pSlot = pServer->GetSlot();
    }

    if ( bGlobalOnly )
        if ( !pShell->ISA(SfxModule) && !pShell->ISA(SfxApplication) && !pShell->ISA(SfxViewFrame) )
            return NULL;

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
        // for Enum-Slots, the Master has to be excecuted with the value
        // of the enums
        const SfxSlot *pRealSlot = pShell->GetInterface()->GetRealSlot(pSlot);
        const sal_uInt16 nSlotId = pRealSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        aReq.AppendItem( SfxAllEnumItem( rPool.GetWhich(nSlotId), pSlot->GetValue() ) );
        pDispatcher->_Execute( *pShell, *pRealSlot, aReq, aReq.GetCallMode() | SFX_CALLMODE_RECORD );
    }
    else if ( SFX_KIND_ATTR == pSlot->GetKind() )
    {
        // Which value has to be mapped for Attribute slots
        const sal_uInt16 nSlotId = pSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        if ( pSlot->IsMode(SFX_SLOT_TOGGLE) )
        {
            // The value is attached to a toggleable attribute (Bools)
            sal_uInt16 nWhich = pSlot->GetWhich(rPool);
            SfxItemSet aSet(rPool, nWhich, nWhich);
            SfxStateFunc aFunc  = pSlot->GetStateFnc();
            pShell->CallState( aFunc, aSet );
            const SfxPoolItem *pOldItem;
            SfxItemState eState = aSet.GetItemState(nWhich, sal_True, &pOldItem);
            if ( eState == SFX_ITEM_DISABLED )
                return;

            if ( SFX_ITEM_AVAILABLE == eState && SfxItemPool::IsWhich(nWhich) )
                pOldItem = &aSet.Get(nWhich);

            if ( SFX_ITEM_SET == eState ||
                 ( SFX_ITEM_AVAILABLE == eState &&
                   SfxItemPool::IsWhich(nWhich) &&
                   pOldItem ) )
            {
                if ( pOldItem->ISA(SfxBoolItem) )
                {
                    // we can toggle Bools
                    sal_Bool bOldValue = ((const SfxBoolItem *)pOldItem)->GetValue();
                    SfxBoolItem *pNewItem = (SfxBoolItem*) (pOldItem->Clone());
                    pNewItem->SetValue( !bOldValue );
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else if ( pOldItem->ISA(SfxEnumItemInterface) &&
                        ((SfxEnumItemInterface *)pOldItem)->HasBoolValue())
                {
                    // and Enums with Bool-Interface
                    SfxEnumItemInterface *pNewItem =
                        (SfxEnumItemInterface*) (pOldItem->Clone());
                    pNewItem->SetBoolValue(!((SfxEnumItemInterface *)pOldItem)->GetBoolValue());
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else {
                    OSL_FAIL( "Toggle only for Enums and Bools allowed" );
                }
            }
            else if ( SFX_ITEM_DONTCARE == eState )
            {
                // Create one Status-Item for each Factory
                SfxPoolItem *pNewItem = pSlot->GetType()->CreateItem();
                DBG_ASSERT( pNewItem, "Toggle to slot without ItemFactory" );
                pNewItem->SetWhich( nWhich );

                if ( pNewItem->ISA(SfxBoolItem) )
                {
                  // we can toggle Bools
                    ((SfxBoolItem*)pNewItem)->SetValue( sal_True );
                    aReq.AppendItem( *pNewItem );
                }
                else if ( pNewItem->ISA(SfxEnumItemInterface) &&
                        ((SfxEnumItemInterface *)pNewItem)->HasBoolValue())
                {
                    // and Enums with Bool-Interface
                    ((SfxEnumItemInterface*)pNewItem)->SetBoolValue(sal_True);
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

        pDispatcher->_Execute( *pShell, *pSlot, aReq, aReq.GetCallMode() | SFX_CALLMODE_RECORD );
    }
    else
        pDispatcher->_Execute( *pShell, *pSlot, aReq, aReq.GetCallMode() | SFX_CALLMODE_RECORD );
}

//--------------------------------------------------------------------

void SfxBindings::UpdateSlotServer_Impl()
{
    DBG_PROFSTART(SfxBindingsUpdateServers);
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    // synchronize
    pDispatcher->Flush();

    if ( pImp->bAllMsgDirty )
    {
        if ( !nRegLevel )
        {
            ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame
                ( pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );
            pImp->bContextChanged = sal_False;
        }
        else
            pImp->bContextChanged = sal_True;
    }

    const sal_uInt16 nCount = pImp->pCaches->Count();
    for(sal_uInt16 i = 0; i < nCount; ++i)
    {
        SfxStateCache *pCache = pImp->pCaches->GetObject(i);
        pCache->GetSlotServer(*pDispatcher, pImp->xProv);
    }
    pImp->bMsgDirty = pImp->bAllMsgDirty = sal_False;

    Broadcast( SfxSimpleHint(SFX_HINT_DOCCHANGED) );

    DBG_PROFSTOP(SfxBindingsUpdateServers);
}

//--------------------------------------------------------------------

SfxItemSet* SfxBindings::CreateSet_Impl
(
    SfxStateCache*&         pCache,     // in: Status-Cache from nId
    const SfxSlot*&         pRealSlot,  // out: RealSlot to nId
    const SfxSlotServer**   pMsgServer, // out: Slot-Server to nId
    SfxFoundCacheArr_Impl&  rFound      // out: List of Caches for Siblings
)
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    DBG_ASSERT( !pImp->bMsgDirty, "CreateSet_Impl with dirty MessageServer" );

    const SfxSlotServer* pMsgSvr = pCache->GetSlotServer(*pDispatcher, pImp->xProv);
    if(!pMsgSvr || !pDispatcher)
        return 0;

    DBG_PROFSTART(SfxBindingsCreateSet);
    pRealSlot = 0;
    *pMsgServer = pMsgSvr;

    sal_uInt16 nShellLevel = pMsgSvr->GetShellLevel();
    SfxShell *pShell = pDispatcher->GetShell( nShellLevel );
    if ( !pShell ) // rare GPF when browsing through update from Inet-Notify
        return 0;

    SfxItemPool &rPool = pShell->GetPool();

    // get the status method, which is served by the pCache
    SfxStateFunc pFnc = 0;
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
        pRealSlot->GetSlotId(), pRealSlot->GetWhich(rPool), pRealSlot, pCache );
    rFound.push_back( pFound );

    sal_uInt16 nSlot = pRealSlot->GetSlotId();
    if ( !(nSlot >= SID_VERB_START && nSlot <= SID_VERB_END) )
    {
        pInterface = pInterface->GetRealInterfaceForSlot( pRealSlot );
        DBG_ASSERT (pInterface,"Slot in the given shell is not found");
    }

    // Search through the bindings for slots served by the same function. This ,    // will only affect slots which are present in the found interface.

    // The position of the  Statecaches in StateCache-Array
    sal_uInt16 nCachePos = pImp->nMsgPos;
    const SfxSlot *pSibling = pRealSlot->GetNextSlot();

    // the Slots odf a interfaces ar linked in a circle
    while ( pSibling > pRealSlot )
    {
        SfxStateFunc pSiblingFnc=0;
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
                pSibling->GetSlotId(), pSibling->GetWhich(rPool),
                pSibling, pSiblingCache );

            rFound.push_back( pFoundCache );
        }

        pSibling = pSibling->GetNextSlot();
    }

    // Create a Set from the ranges
    sal_uInt16 *pRanges = new sal_uInt16[rFound.size() * 2 + 1];
    int j = 0;
    sal_uInt16 i = 0;
    while ( i < rFound.size() )
    {
        pRanges[j++] = rFound[i]->nWhichId;
            // consecutive numbers
        for ( ; i < rFound.size()-1; ++i )
            if ( rFound[i]->nWhichId+1 != rFound[i+1]->nWhichId )
                break;
        pRanges[j++] = rFound[i++]->nWhichId;
    }
    pRanges[j] = 0; // terminating NULL
    SfxItemSet *pSet = new SfxItemSet(rPool, pRanges);
    delete [] pRanges;
    DBG_PROFSTOP(SfxBindingsCreateSet);
    return pSet;
}

//--------------------------------------------------------------------

void SfxBindings::UpdateControllers_Impl
(
    const SfxInterface*         pIF,    // Id of the current serving Interface
    const SfxFoundCache_Impl*   pFound, // Cache, Slot, Which etc.
    const SfxPoolItem*          pItem,  // item to send to controller
    SfxItemState                eState  // state of item
)
{
    DBG_ASSERT( !pFound->pSlot || SFX_KIND_ENUM != pFound->pSlot->GetKind(),
                "direct update of enum slot isn't allowed" );
    DBG_PROFSTART(SfxBindingsUpdateCtrl1);

    SfxStateCache* pCache = pFound->pCache;
    const SfxSlot* pSlot = pFound->pSlot;
    DBG_ASSERT( !pCache || !pSlot || pCache->GetId() == pSlot->GetSlotId(), "SID mismatch" );

    // bound until now, the Controller to update the Slot.
    if ( pCache && pCache->IsControllerDirty() )
    {
        if ( SFX_ITEM_DONTCARE == eState )
        {
            // ambiguous
            pCache->SetState( SFX_ITEM_DONTCARE, (SfxPoolItem *)-1 );
        }
        else if ( SFX_ITEM_DEFAULT == eState &&
                    pFound->nWhichId > SFX_WHICH_MAX )
        {
            // no Status or Default but without Pool
            SfxVoidItem aVoid(0);
            pCache->SetState( SFX_ITEM_UNKNOWN, &aVoid );
        }
        else if ( SFX_ITEM_DISABLED == eState )
            pCache->SetState(SFX_ITEM_DISABLED, 0);
        else
            pCache->SetState(SFX_ITEM_AVAILABLE, pItem);
    }

    DBG_PROFSTOP(SfxBindingsUpdateCtrl1);

    // Update the slots for so far available and bound Controllers for
    // Slave-Slots (Enum-value)
    DBG_PROFSTART(SfxBindingsUpdateCtrl2);
    DBG_ASSERT( !pSlot || 0 == pSlot->GetLinkedSlot() || !pItem ||
                pItem->ISA(SfxEnumItemInterface),
                "master slot with non-enum-type found" );
    const SfxSlot *pFirstSlave = pSlot ? pSlot->GetLinkedSlot() : 0;
    if ( pIF && pFirstSlave)
    {
        // Items cast on EnumItem
        const SfxEnumItemInterface *pEnumItem =
                PTR_CAST(SfxEnumItemInterface,pItem);
        if ( eState == SFX_ITEM_AVAILABLE && !pEnumItem )
            eState = SFX_ITEM_DONTCARE;
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
                pEnumCache->Invalidate(sal_False);

                HACK(CONTROL/SELECT Kram)
                if ( eState == SFX_ITEM_DONTCARE && pFound->nWhichId == 10144 )
                {
                    SfxVoidItem aVoid(0);
                    pEnumCache->SetState( SFX_ITEM_UNKNOWN, &aVoid );

                    if (pSlave->GetNextSlot() == pFirstSlave)
                        break;

                    continue;
                }

                if ( SFX_ITEM_DISABLED == eState || !pEnumItem->IsEnabled( pSlave->GetSlotId()) )
                {
                    // disabled
                    pEnumCache->SetState(SFX_ITEM_DISABLED, 0);
                }
                else if ( SFX_ITEM_AVAILABLE == eState )
                {
                    // Determine enum value
                    sal_uInt16 nValue = pEnumItem->GetEnumValue();
                    SfxBoolItem aBool( pFound->nWhichId, pSlave->GetValue() == nValue );
                    pEnumCache->SetState(SFX_ITEM_AVAILABLE, &aBool);
                }
                else
                {
                    // ambiguous
                    pEnumCache->SetState( SFX_ITEM_DONTCARE, (SfxPoolItem *)-1 );
                }
            }

            if (pSlave->GetNextSlot() == pFirstSlave)
                break;
        }
    }

    DBG_PROFSTOP(SfxBindingsUpdateCtrl2);
}


//--------------------------------------------------------------------

IMPL_LINK( SfxBindings, NextJob_Impl, Timer *, pTimer )
{
#ifdef DBG_UTIL
    // on Windows very often C++ Exceptions (GPF etc.) are caught by MSVCRT
    // or another MS library try to get them here
    try
    {
#endif
    const unsigned MAX_INPUT_DELAY = 200;

    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    DBG_PROFSTART(SfxBindingsNextJob_Impl0);

    if ( Application::GetLastInputInterval() < MAX_INPUT_DELAY && pTimer )
    {
        pImp->aTimer.SetTimeout(TIMEOUT_UPDATING);
        return sal_True;
    }

    SfxApplication *pSfxApp = SFX_APP();

    if( pDispatcher )
        pDispatcher->Update_Impl();

    // modifying the SfxObjectInterface-stack without SfxBindings => nothing to do
    SfxViewFrame* pFrame = pDispatcher->GetFrame();
    if ( (pFrame && !pFrame->GetObjectShell()->AcceptStateUpdate()) || pSfxApp->IsDowning() || !pImp->pCaches->Count() )
    {
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_True;
    }
    if ( !pDispatcher || !pDispatcher->IsFlushed() )
    {
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_True;
    }

    // if possible Update all server / happens in its own time slice
    if ( pImp->bMsgDirty )
    {
        UpdateSlotServer_Impl();
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_False;
    }

    DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
    DBG_PROFSTART(SfxBindingsNextJob_Impl);
    pImp->bAllDirty = sal_False;
    pImp->aTimer.SetTimeout(TIMEOUT_UPDATING);

    // at least 10 loops and further if more jobs are available but no input
    bool bPreEmptive = pTimer && !pSfxApp->Get_Impl()->nInReschedule;
    sal_uInt16 nLoops = 10;
    pImp->bInNextJob = sal_True;
    const sal_uInt16 nCount = pImp->pCaches->Count();
    while ( pImp->nMsgPos < nCount )
    {
        // iterate through the bound functions
        sal_Bool bJobDone = sal_False;
        while ( !bJobDone )
        {
            SfxStateCache* pCache = (*pImp->pCaches)[pImp->nMsgPos];
            DBG_ASSERT( pCache, "invalid SfxStateCache-position in job queue" );
            sal_Bool bWasDirty = pCache->IsControllerDirty();
            if ( bWasDirty )
            {
                    Update_Impl( pCache );
                    DBG_ASSERT( nCount == pImp->pCaches->Count(),
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
                bJobDone = sal_False;
                pImp->bFirstRound = sal_False;
                pImp->nMsgPos = 0;
            }

            if ( bWasDirty && !bJobDone && bPreEmptive && (--nLoops == 0) )
            {
                DBG_PROFSTOP(SfxBindingsNextJob_Impl);
                pImp->bInNextJob = sal_False;
                return sal_False;
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
        if ( pSlotServer && pSlotServer->GetSlot()->IsMode(SFX_SLOT_VOLATILE) )
        {
            pCache->Invalidate(sal_False);
            bVolatileSlotsPresent = true;
        }
    }

    if (bVolatileSlotsPresent)
        pImp->aTimer.SetTimeout(TIMEOUT_IDLE);
    else
        pImp->aTimer.Stop();

    // Update round is finished
    pImp->bInNextJob = sal_False;
    Broadcast(SfxSimpleHint(SFX_HINT_UPDATEDONE));
    DBG_PROFSTOP(SfxBindingsNextJob_Impl);
    return sal_True;
#ifdef DBG_UTIL
    }
    catch (...)
    {
        OSL_FAIL("C++ exception caught!");
        pImp->bInNextJob = sal_False;
    }

    return sal_False;
#endif
}

//--------------------------------------------------------------------

sal_uInt16 SfxBindings::EnterRegistrations(const char *pFile, int nLine)
{
    SAL_INFO(
        "sfx2",
        std::setw(Min(nRegLevel, sal_uInt16(8))) << ' ' << "this = " << this
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

        // Mark if the all of the Caches have dissapered.
        pImp->bCtrlReleased = sal_False;
    }

    return nRegLevel;
}
//--------------------------------------------------------------------

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
    if ( --nRegLevel == 0 && !SFX_APP()->IsDowning() )
    {
        if ( pImp->bContextChanged )
        {
            pImp->bContextChanged = sal_False;
        }

        SfxViewFrame* pFrame = pDispatcher->GetFrame();

        // If possible remove unused Caches, for example prepare PlugInInfo
        if ( pImp->bCtrlReleased )
        {
            for ( sal_uInt16 nCache = pImp->pCaches->Count(); nCache > 0; --nCache )
            {
                // Get Cache via ::com::sun::star::sdbcx::Index
                SfxStateCache *pCache = pImp->pCaches->GetObject(nCache-1);

                // No interested Controller present
                if ( pCache->GetItemLink() == 0 && !pCache->GetInternalController() )
                {
                    // Remove Cache. Safety: first remove and then delete
                    SfxStateCache* pSfxStateCache = (*pImp->pCaches)[nCache-1];
                    pImp->pCaches->Remove(nCache-1, 1);
                    delete pSfxStateCache;
                }
            }
        }

        // restart background-processing
        pImp->nMsgPos = 0;
        if ( !pFrame || !pFrame->GetObjectShell() )
            return;
        if ( pImp->pCaches && pImp->pCaches->Count() )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
        }
    }

    SAL_INFO(
        "sfx2",
        std::setw(Min(nRegLevel, sal_uInt16(8))) << ' ' << "this = " << this
            << " Level = " << nRegLevel << " SfxBindings::LeaveRegistrations "
            << (pFile
                ? SAL_STREAM("File: " << pFile << " Line: " << nLine) : ""));
}

//--------------------------------------------------------------------

const SfxSlot* SfxBindings::GetSlot(sal_uInt16 nSlotId)
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    // synchronizing
    pDispatcher->Flush();
    if ( pImp->bMsgDirty )
        UpdateSlotServer_Impl();

    // get the cache for the specified function; return if not bound
    SfxStateCache* pCache = GetStateCache(nSlotId);
    return pCache && pCache->GetSlotServer(*pDispatcher, pImp->xProv)?
            pCache->GetSlotServer(*pDispatcher, pImp->xProv)->GetSlot(): 0;
}

//--------------------------------------------------------------------

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
                    pBind->SetSubBindings_Impl( NULL );
                pBind = pBind->pImp->pSubBindings;
            }
        }

        pDispatcher = pDisp;

        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchProvider > xProv;
        if ( pDisp )
            xProv = ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchProvider >
                                        ( pDisp->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );

        SetDispatchProvider_Impl( xProv );
        InvalidateAll( sal_True );
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

//--------------------------------------------------------------------

void SfxBindings::ClearCache_Impl( sal_uInt16 nSlotId )
{
    GetStateCache(nSlotId)->ClearCache();
}

//--------------------------------------------------------------------
void SfxBindings::StartUpdate_Impl( sal_Bool bComplete )
{
    if ( pImp->pSubBindings )
        pImp->pSubBindings->StartUpdate_Impl( bComplete );

    if ( !bComplete )
        // Update may be interrupted
        NextJob_Impl(&pImp->aTimer);
    else
        // Update all slots in a row
        NextJob_Impl(0);
}

//-------------------------------------------------------------------------

SfxItemState SfxBindings::QueryState( sal_uInt16 nSlot, SfxPoolItem* &rpState )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp;
    SfxStateCache *pCache = GetStateCache( nSlot );
    if ( pCache )
        xDisp = pCache->GetDispatch();
    if ( xDisp.is() || !pCache )
    {
        const SfxSlot* pSlot = SfxSlotPool::GetSlotPool( pDispatcher->GetFrame() ).GetSlot( nSlot );
        if ( !pSlot || !pSlot->pUnoName )
            return SFX_ITEM_DISABLED;

        ::com::sun::star::util::URL aURL;
        ::rtl::OUString aCmd( DEFINE_CONST_UNICODE(".uno:"));
        aURL.Protocol = aCmd;
        aURL.Path = ::rtl::OUString::createFromAscii(pSlot->GetUnoName());
        aCmd += aURL.Path;
        aURL.Complete = aCmd;
        aURL.Main = aCmd;

        if ( !xDisp.is() )
            xDisp = pImp->xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );

        if ( xDisp.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel( xDisp, ::com::sun::star::uno::UNO_QUERY );
            SfxOfficeDispatch* pDisp = NULL;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = reinterpret_cast< SfxOfficeDispatch* >( sal::static_int_cast< sal_IntPtr >( nImplementation ));
            }

            if ( !pDisp )
            {
                sal_Bool bDeleteCache = sal_False;
                if ( !pCache )
                {
                    pCache = new SfxStateCache( nSlot );
                    pCache->GetSlotServer( *GetDispatcher_Impl(), pImp->xProv );
                    bDeleteCache = sal_True;
                }

                SfxItemState eState = SFX_ITEM_SET;
                SfxPoolItem *pItem=NULL;
                BindDispatch_Impl *pBind = new BindDispatch_Impl( xDisp, aURL, pCache, pSlot );
                pBind->acquire();
                xDisp->addStatusListener( pBind, aURL );
                if ( !pBind->GetStatus().IsEnabled )
                {
                    eState = SFX_ITEM_DISABLED;
                }
                else
                {
                    ::com::sun::star::uno::Any aAny = pBind->GetStatus().State;
                    ::com::sun::star::uno::Type pType = aAny.getValueType();

                    if ( pType == ::getBooleanCppuType() )
                    {
                        sal_Bool bTemp = false;
                        aAny >>= bTemp ;
                        pItem = new SfxBoolItem( nSlot, bTemp );
                    }
                    else if ( pType == ::getCppuType((const sal_uInt16*)0) )
                    {
                        sal_uInt16 nTemp = 0;
                        aAny >>= nTemp ;
                        pItem = new SfxUInt16Item( nSlot, nTemp );
                    }
                    else if ( pType == ::getCppuType((const sal_uInt32*)0) )
                    {
                        sal_uInt32 nTemp = 0;
                        aAny >>= nTemp ;
                        pItem = new SfxUInt32Item( nSlot, nTemp );
                    }
                    else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
                    {
                        ::rtl::OUString sTemp ;
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
    const SfxPoolItem *pItem = NULL;
    SfxItemState eState = pDispatcher->QueryState( nSlot, pItem );
    if ( eState == SFX_ITEM_SET )
    {
        DBG_ASSERT( pItem, "SFX_ITEM_SET but no item!" );
        if ( pItem )
            rpState = pItem->Clone();
    }
    else if ( eState == SFX_ITEM_AVAILABLE && pItem )
    {
        rpState = pItem->Clone();
    }

    return eState;
}

void SfxBindings::SetSubBindings_Impl( SfxBindings *pSub )
{
    if ( pImp->pSubBindings )
    {
        pImp->pSubBindings->SetDispatchProvider_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > () );
        pImp->pSubBindings->pImp->pSuperBindings = NULL;
    }

    pImp->pSubBindings = pSub;

    if ( pSub )
    {
        pImp->pSubBindings->SetDispatchProvider_Impl( pImp->xProv );
        pSub->pImp->pSuperBindings = this;
    }
}

SfxBindings* SfxBindings::GetSubBindings_Impl( sal_Bool bTop ) const
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
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  xRef( (::cppu::OWeakObject*)pCtrl, ::com::sun::star::uno::UNO_QUERY );
            pCtrl->ReleaseDispatch();
            pCtrl->GetNewDispatch();
        }
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateUnoControllers_Impl();
}

sal_Bool SfxBindings::IsInUpdate() const
{
    sal_Bool bInUpdate = pImp->bInUpdate;
    if ( !bInUpdate && pImp->pSubBindings )
        bInUpdate = pImp->pSubBindings->IsInUpdate();
    return bInUpdate;
}

void SfxBindings::SetVisibleState( sal_uInt16 nId, sal_Bool bShow )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp;
    SfxStateCache *pCache = GetStateCache( nId );
    if ( pCache )
        pCache->SetVisibleState( bShow );
}

void SfxBindings::SetActiveFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & rFrame )
{
    if ( rFrame.is() || !pDispatcher )
        SetDispatchProvider_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > ( rFrame, ::com::sun::star::uno::UNO_QUERY ) );
    else
        SetDispatchProvider_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > (
            pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), ::com::sun::star::uno::UNO_QUERY ) );
}

const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SfxBindings::GetActiveFrame() const
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame( pImp->xProv, ::com::sun::star::uno::UNO_QUERY );
    if ( xFrame.is() || !pDispatcher )
        return xFrame;
    else
        return pDispatcher->GetFrame()->GetFrame().GetFrameInterface();
}

void SfxBindings::SetDispatchProvider_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & rProv )
{
    sal_Bool bInvalidate = ( rProv != pImp->xProv );
    if ( bInvalidate )
    {
        pImp->xProv = rProv;
        InvalidateAll( sal_True );
        InvalidateUnoControllers_Impl();
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->SetDispatchProvider_Impl( pImp->xProv );
}

const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & SfxBindings::GetDispatchProvider_Impl() const
{
    return pImp->xProv;
}

SystemWindow* SfxBindings::GetSystemWindow() const
{
    SfxViewFrame *pFrame = pDispatcher->GetFrame();
    while ( pFrame->GetParentViewFrame_Impl() )
        pFrame = pFrame->GetParentViewFrame_Impl();
    SfxViewFrame* pTop = pFrame->GetTopViewFrame();
    return pTop->GetFrame().GetTopWindow_Impl();
}

sal_Bool SfxBindings::ExecuteCommand_Impl( const String& rCommand )
{
    ::com::sun::star::util::URL aURL;
    aURL.Complete = rCommand;
    Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aURL );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = pImp->xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
    if ( xDisp.is() )
    {
        new SfxAsyncExec_Impl( aURL, xDisp );
        return sal_True;
    }

    return sal_False;
}

com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > SfxBindings::GetRecorder() const
{
    return pImp->xRecorder;
}

void SfxBindings::SetRecorder_Impl( com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder >& rRecorder )
{
    pImp->xRecorder = rRecorder;
}

void SfxBindings::ContextChanged_Impl()
{
    if ( !pImp->bInUpdate && ( !pImp->bContextChanged || !pImp->bAllMsgDirty ) )
    {
        InvalidateAll( sal_True );
    }
}

uno::Reference < frame::XDispatch > SfxBindings::GetDispatch( const SfxSlot* pSlot, const util::URL& aURL, sal_Bool bMasterCommand )
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
        xRet = uno::Reference < frame::XDispatch >( pDispatch );
        if ( !pCache )
            pCache = GetStateCache( pSlot->nSlotId );

        DBG_ASSERT( pCache, "No cache for OfficeDispatch!" );
        if ( pCache && !bMasterCommand )
            pCache->SetInternalDispatch( xRet );
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
