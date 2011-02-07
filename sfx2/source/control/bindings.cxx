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
#include "precompiled_sfx2.hxx"

#include <hash_map>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/eitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
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

#ifndef GCC
#endif

// wg. nInReschedule
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

#include <comphelper/uieventslogger.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>


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

static sal_uInt32 nCache1 = 0;
static sal_uInt32 nCache2 = 0;

typedef std::hash_map< sal_uInt16, bool > InvalidateSlotMap;

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
    SfxStateCacheArr_Impl*  pCaches;        // je ein cache fuer jede gebundene
    sal_uInt16                  nCachedFunc1;   // index der zuletzt gerufenen
    sal_uInt16                  nCachedFunc2;   // index der vorletzt gerufenen
    sal_uInt16                  nMsgPos;        // Message-Position, ab der zu aktualisieren ist
    SfxPopupAction          ePopupAction;   // in DeleteFloatinWindow() abgefragt
    sal_Bool                    bContextChanged;
    sal_Bool                    bMsgDirty;      // wurde ein MessageServer invalidiert?
    sal_Bool                    bAllMsgDirty;   // wurden die MessageServer invalidiert?
    sal_Bool                    bAllDirty;      // nach InvalidateAll
    sal_Bool                    bCtrlReleased;  // waehrend EnterRegistrations
    AutoTimer               aTimer;         // fuer volatile Slots
    sal_Bool                    bInUpdate;      // fuer Assertions
    sal_Bool                    bInNextJob;     // fuer Assertions
    sal_Bool                    bFirstRound;    // Erste Runde im Update
    sal_uInt16                  nFirstShell;    // Shell, die in erster Runde bevorzugt wird
    sal_uInt16                  nOwnRegLevel;   // z"ahlt die echten Locks, ohne die der SuperBindings
    InvalidateSlotMap           m_aInvalidateSlots; // store slots which are invalidated while in update
};

//--------------------------------------------------------------------

struct SfxFoundCache_Impl
{
    sal_uInt16          nSlotId;    // die Slot-Id
    sal_uInt16          nWhichId;   // falls verf"ugbar die Which-Id, sonst nSlotId
    const SfxSlot*  pSlot;      // Pointer auf den <Master-Slot>
    SfxStateCache*  pCache;     // Pointer auf den StatusCache, ggf. 0

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

SV_DECL_PTRARR_SORT_DEL(SfxFoundCacheArr_Impl, SfxFoundCache_Impl*, 16, 16 )
SV_IMPL_OP_PTRARR_SORT(SfxFoundCacheArr_Impl, SfxFoundCache_Impl*);

//==========================================================================

SfxBindings::SfxBindings()
:   pImp(new SfxBindings_Impl),
    pDispatcher(0),
    nRegLevel(1)    // geht erst auf 0, wenn Dispatcher gesetzt
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

/*  [Beschreibung]

    Destruktor der Klasse SfxBindings. Die eine, f"ur jede <SfxApplication>
    existierende Instanz wird von der <SfxApplication> nach Ausf"urhung
    von <SfxApplication::Exit()> automatisch zerst"ort.

    Noch existierende <SfxControllerItem> Instanzen, die bei dieser
    SfxBindings Instanz angemeldet sind, werden im Destruktor
    automatisch zerst"ort. Dies sind i.d.R. Floating-Toolboxen, Value-Sets
    etc. Arrays von SfxControllerItems d"urfen zu diesem Zeitpunkt nicht
    mehr exisitieren.
*/

{
    // Die SubBindings sollen ja nicht gelocked werden !
    pImp->pSubBindings = NULL;

    ENTERREGISTRATIONS();

    pImp->aTimer.Stop();
    DeleteControllers_Impl();

    // Caches selbst l"oschen
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
    // in der ersten Runde den SfxPopupWindows l"oschen
    sal_uInt16 nCount = pImp->pCaches->Count();
    sal_uInt16 nCache;
    for ( nCache = 0; nCache < nCount; ++nCache )
    {
        // merken wo man ist
        SfxStateCache *pCache = pImp->pCaches->GetObject(nCache);
        sal_uInt16 nSlotId = pCache->GetId();

        // SfxPopupWindow l"oschen lassen
        pCache->DeleteFloatingWindows();

        // da der Cache verkleinert worden sein kann, wiederaufsetzen
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

    // alle Caches l"oschen
    for ( nCache = pImp->pCaches->Count(); nCache > 0; --nCache )
    {
        // Cache via ::com::sun::star::sdbcx::Index besorgen
        SfxStateCache *pCache = pImp->pCaches->GetObject(nCache-1);

        // alle Controller in dem Cache unbinden
        SfxControllerItem *pNext;
        for ( SfxControllerItem *pCtrl = pCache->GetItemLink();
              pCtrl; pCtrl = pNext )
        {
            pNext = pCtrl->GetItemLink();
            pCtrl->UnBind();
        }

        if ( pCache->GetInternalController() )
            pCache->GetInternalController()->UnBind();

        // Cache l"oschen
        if( nCache-1 < pImp->pCaches->Count() )
            delete (*pImp->pCaches)[nCache-1];
        pImp->pCaches->Remove(nCache-1, 1);
    }

    if( pImp->pUnoCtrlArr )
    {
        sal_uInt16 nCtrlCount = pImp->pUnoCtrlArr->Count();
        for ( sal_uInt16 n=nCtrlCount; n>0; n-- )
        {
            SfxUnoControllerItem *pCtrl = (*pImp->pUnoCtrlArr)[n-1];
            pCtrl->ReleaseBindings();
        }

        DBG_ASSERT( !pImp->pUnoCtrlArr->Count(), "UnoControllerItems nicht entfernt!" );
        DELETEZ( pImp->pUnoCtrlArr );
    }
}

//--------------------------------------------------------------------

SfxPopupAction SfxBindings::GetPopupAction_Impl() const
{
    return pImp->ePopupAction;
}


//--------------------------------------------------------------------

void SfxBindings::HidePopups( bool bHide )
{
    // SfxPopupWindows hiden
    HidePopupCtrls_Impl( bHide );
    SfxBindings *pSub = pImp->pSubBindings;
    while ( pSub )
    {
        pImp->pSubBindings->HidePopupCtrls_Impl( bHide );
        pSub = pSub->pImp->pSubBindings;
    }

    // SfxChildWindows hiden
    DBG_ASSERT( pDispatcher, "HidePopups not allowed without dispatcher" );
    if ( pImp->pWorkWin )
        pImp->pWorkWin->HidePopups_Impl( bHide, sal_True );
}

void SfxBindings::HidePopupCtrls_Impl( FASTBOOL bHide )
{
    if ( bHide )
    {
        // SfxPopupWindows hiden
        pImp->ePopupAction = SFX_POPUP_HIDE;
    }
    else
    {
        // SfxPopupWindows showen
        pImp->ePopupAction = SFX_POPUP_SHOW;
    }

    for ( sal_uInt16 nCache = 0; nCache < pImp->pCaches->Count(); ++nCache )
        pImp->pCaches->GetObject(nCache)->DeleteFloatingWindows();
    pImp->ePopupAction = SFX_POPUP_DELETE;
}

//--------------------------------------------------------------------

void SfxBindings::Update_Impl
(
    SfxStateCache*  pCache      // der upzudatende SfxStatusCache
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

    // alle mit derselben Statusmethode zusammensammeln, die dirty sind
    SfxDispatcher &rDispat = *pDispatcher;
    const SfxSlot *pRealSlot = 0;
    const SfxSlotServer* pMsgServer = 0;
    SfxFoundCacheArr_Impl aFound;
    SfxItemSet *pSet = CreateSet_Impl( pCache, pRealSlot, &pMsgServer, aFound );
    sal_Bool bUpdated = sal_False;
    if ( pSet )
    {
        // Status erfragen
        if ( rDispat._FillState( *pMsgServer, *pSet, pRealSlot ) )
        {
            // Status posten
            const SfxInterface *pInterface =
                rDispat.GetShell(pMsgServer->GetShellLevel())->GetInterface();
            for ( sal_uInt16 nPos = 0; nPos < aFound.Count(); ++nPos )
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
        // Wenn pCache == NULL und kein SlotServer ( z.B. weil Dispatcher gelockt! ),
        // darf nat"urlich kein Update versucht werden
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
    sal_uInt16      nId     // die gebundene und upzudatende Slot-Id
)
{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

//!!TLX: Fuehrte zu Vorlagenkatalogstillstand
//  if ( nRegLevel )
//      return;

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
                // Status erfragen
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
    DBG_MEMTEST();
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
    const SfxItemSet&   rSet    // zu setzende Status-Werte
)
{
    // wenn gelockt, dann nur invalidieren
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
        // Status d"urfen nur angenommen werden, wenn alle Slot-Pointer gesetzt sind
        if ( pImp->bMsgDirty )
            UpdateSlotServer_Impl();

        // "uber das ItemSet iterieren, falls Slot gebunden, updaten
        //! Bug: WhichIter verwenden und ggf. VoidItems hochschicken
        SfxItemIter aIter(rSet);
        for ( const SfxPoolItem *pItem = aIter.FirstItem();
              pItem;
              pItem = aIter.NextItem() )
        {
            SfxStateCache* pCache =
                    GetStateCache( rSet.GetPool()->GetSlotId(pItem->Which()) );
            if ( pCache )
            {
                // Status updaten
                if ( !pCache->IsControllerDirty() )
                    pCache->Invalidate(sal_False);
                pCache->SetState( SFX_ITEM_AVAILABLE, pItem );

                //! nicht implementiert: Updates von EnumSlots via MasterSlots
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxBindings::SetState
(
    const SfxPoolItem&  rItem   // zu setzender Status-Wert
)
{
    if ( nRegLevel )
    {
        Invalidate( rItem.Which() );
    }
    else
    {
        // Status d"urfen nur angenommen werden, wenn alle Slot-Pointer gesetzt sind
        if ( pImp->bMsgDirty )
            UpdateSlotServer_Impl();

        // falls der Slot gebunden ist, updaten
        DBG_ASSERT( SfxItemPool::IsSlot( rItem.Which() ),
                    "cannot set items with which-id" );
        SfxStateCache* pCache = GetStateCache( rItem.Which() );
        if ( pCache )
        {
            // Status updaten
            if ( !pCache->IsControllerDirty() )
                pCache->Invalidate(sal_False);
            pCache->SetState( SFX_ITEM_AVAILABLE, &rItem );

            //! nicht implementiert: Updates von EnumSlots via MasterSlots
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
    sal_uInt16      nId         /*  Slot-Id, deren SfxStatusCache gefunden
                                werden soll */,
    sal_uInt16*     pPos        /*  0 bzw. Position, ab der die Bindings
                                bin"ar durchsucht werden sollen. Liefert
                                die Position zur"uck, an der nId gefunden
                                wurde, bzw. an der es einfef"ugt werden
                                w"urde. */
)
{
    DBG_MEMTEST();
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
    sal_Bool    bWithMsg        /*  sal_True
                                Slot-Server als ung"ultig markieren

                                sal_False
                                Slot-Server bleiben g"ultig */
)
{
    DBG_PROFSTART(SfxBindingsInvalidateAll);
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    DBG_MEMTEST();

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateAll( bWithMsg );

    // ist schon alles dirty gesetzt oder downing => nicht zu tun
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
/*
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame
        ( pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );

    if ( bWithMsg && xFrame.is() )
        xFrame->contextChanged();
*/
    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.Stop();
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
//      pImp->bFirstRound = sal_True;
//      pImp->nFirstShell = 0;
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}

//--------------------------------------------------------------------

void SfxBindings::Invalidate
(
    const sal_uInt16* pIds      /*  numerisch sortiertes 0-terminiertes Array
                                von Slot-Ids (einzel, nicht als Paare!) */
)
{
    DBG_PROFSTART(SfxBindingsInvalidateAll);
//  DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    DBG_MEMTEST();

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

    // ist schon alles dirty gesetzt oder downing => nicht zu tun
    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    // in immer kleiner werdenden Berichen bin"ar suchen
    for ( sal_uInt16 n = GetSlotPos(*pIds);
          *pIds && n < pImp->pCaches->Count();
          n = GetSlotPos(*pIds, n) )
    {
        // falls SID "uberhaupt gebunden ist, den Cache invalidieren
        SfxStateCache *pCache = pImp->pCaches->GetObject(n);
        if ( pCache->GetId() == *pIds )
            pCache->Invalidate(sal_False);

        // n"achste SID
        if ( !*++pIds )
            break;
        DBG_ASSERT( *pIds > *(pIds-1), "pIds unsorted" );
    }

    // falls nicht gelockt, Update-Timer starten
    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.Stop();
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
//      pImp->bFirstRound = sal_True;
//      pImp->nFirstShell = 0;
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}

//--------------------------------------------------------------------

void SfxBindings::InvalidateShell
(
    const SfxShell&     rSh             /*  Die <SfxShell>, deren Slot-Ids
                                            invalidiert werden sollen. */,

    sal_Bool                bDeep           /*  sal_True
                                            auch die, von der SfxShell
                                            ererbten Slot-Ids werden invalidert

                                            sal_False
                                            die ererbten und nicht "uberladenen
                                            Slot-Ids werden invalidiert */
                                            //! MI: z. Zt. immer bDeep
)
{
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateShell( rSh, bDeep );

    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    DBG_PROFSTART(SfxBindingsInvalidateAll);
    DBG_MEMTEST();

    // Jetzt schon flushen, wird in GetShellLevel(rSh) sowieso gemacht; wichtig,
    // damit pImp->bAll(Msg)Dirty korrekt gesetzt ist
    pDispatcher->Flush();

    if ( !pDispatcher ||
         ( pImp->bAllDirty && pImp->bAllMsgDirty ) ||
         SFX_APP()->IsDowning() )
    {
        // Wenn sowieso demn"achst alle Server geholt werden
        return;
    }

    // Level finden
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
    sal_uInt16 nId              // zu invalidierende Slot-Id
)
{
    DBG_MEMTEST();
//  DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

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
    sal_uInt16  nId,                // zu invalidierende Slot-Id
    sal_Bool    bWithItem,          // StateCache clearen ?
    sal_Bool    bWithMsg            // SlotServer neu holen ?
)
{
    DBG_MEMTEST();
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
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    return GetStateCache(nSlotId, &nStartSearchAt ) != 0;
}

//--------------------------------------------------------------------

sal_uInt16 SfxBindings::GetSlotPos( sal_uInt16 nId, sal_uInt16 nStartSearchAt )
{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    DBG_PROFSTART(SfxBindingsMsgPos);

    // answer immediately if a function-seek comes repeated
    if ( pImp->nCachedFunc1 < pImp->pCaches->Count() &&
         (*pImp->pCaches)[pImp->nCachedFunc1]->GetId() == nId )
    {
        ++nCache1;
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return pImp->nCachedFunc1;
    }
    if ( pImp->nCachedFunc2 < pImp->pCaches->Count() &&
         (*pImp->pCaches)[pImp->nCachedFunc2]->GetId() == nId )
    {
        ++nCache2;

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
        DBG_ASSERT( nMid < pImp->pCaches->Count(), "bsearch ist buggy" );
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
    DBG_MEMTEST();
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
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    //! DBG_ASSERT( nRegLevel > 0, "release without EnterRegistrations" );
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
#ifdef slow
            // remove the BoundFunc
            delete (*pImp->pCaches)[nPos];
            pImp->pCaches->Remove(nPos, 1);
#endif
            pImp->bCtrlReleased = sal_True;
        }
    }

    LEAVEREGISTRATIONS();
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxBindings::ExecuteSynchron( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi,
            const SfxPoolItem **ppInternalArgs )
{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return NULL;

    return Execute_Impl( nId, ppItems, nModi, SFX_CALLMODE_SYNCHRON, ppInternalArgs );
}

sal_Bool SfxBindings::Execute( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi, SfxCallMode nCallMode,
                        const SfxPoolItem **ppInternalArgs )
{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return sal_False;

    const SfxPoolItem* pRet = Execute_Impl( nId, ppItems, nModi, nCallMode, ppInternalArgs );
    return ( pRet != 0 );
}

void SfxBindings::ExecuteGlobal_Impl( sal_uInt16 nId )
{
    if( nId && pDispatcher )
        Execute_Impl( nId, NULL, 0, SFX_CALLMODE_ASYNCHRON, NULL, sal_True );
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
    sal_Bool bDeleteCache = sal_False;
    if ( !pCache )
    {
        // Execution of non cached slots (Accelerators don't use Controllers)
        // slot is uncached, use SlotCache to handle external dispatch providers
        pCache = new SfxStateCache( nId );
        pCache->GetSlotServer( rDispatcher, pImp->xProv );
        bDeleteCache = sal_True;
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
        if ( bDeleteCache )
            DELETEZ( pCache );
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

    if ( bDeleteCache )
        delete pCache;

    return pRet;
}

void SfxBindings::Execute_Impl( SfxRequest& aReq, const SfxSlot* pSlot, SfxShell* pShell )
{
    SfxItemPool &rPool = pShell->GetPool();

    if ( SFX_KIND_ENUM == pSlot->GetKind() )
    {
        // bei Enum-Slots muss der Master mit dem Wert des Enums executet werden
        const SfxSlot *pRealSlot = pShell->GetInterface()->GetRealSlot(pSlot);
        const sal_uInt16 nSlotId = pRealSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        aReq.AppendItem( SfxAllEnumItem( rPool.GetWhich(nSlotId), pSlot->GetValue() ) );
        pDispatcher->_Execute( *pShell, *pRealSlot, aReq, aReq.GetCallMode() | SFX_CALLMODE_RECORD );
    }
    else if ( SFX_KIND_ATTR == pSlot->GetKind() )
    {
        // bei Attr-Slots muss der Which-Wert gemapped werden
        const sal_uInt16 nSlotId = pSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        if ( pSlot->IsMode(SFX_SLOT_TOGGLE) )
        {
            // an togglebare-Attribs (Bools) wird der Wert angeheangt
            sal_uInt16 nWhich = pSlot->GetWhich(rPool);
            SfxItemSet aSet(rPool, nWhich, nWhich, 0);
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
                    // wir koennen Bools toggeln
                    sal_Bool bOldValue = ((const SfxBoolItem *)pOldItem)->GetValue();
                    SfxBoolItem *pNewItem = (SfxBoolItem*) (pOldItem->Clone());
                    pNewItem->SetValue( !bOldValue );
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else if ( pOldItem->ISA(SfxEnumItemInterface) &&
                        ((SfxEnumItemInterface *)pOldItem)->HasBoolValue())
                {
                    // und Enums mit Bool-Interface
                    SfxEnumItemInterface *pNewItem =
                        (SfxEnumItemInterface*) (pOldItem->Clone());
                    pNewItem->SetBoolValue(!((SfxEnumItemInterface *)pOldItem)->GetBoolValue());
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else {
                    DBG_ERROR( "Toggle only for Enums and Bools allowed" );
                }
            }
            else if ( SFX_ITEM_DONTCARE == eState )
            {
                // ein Status-Item per Factory erzeugen
                SfxPoolItem *pNewItem = pSlot->GetType()->CreateItem();
                DBG_ASSERT( pNewItem, "Toggle an Slot ohne ItemFactory" );
                pNewItem->SetWhich( nWhich );

                if ( pNewItem->ISA(SfxBoolItem) )
                {
                    // wir koennen Bools toggeln
                    ((SfxBoolItem*)pNewItem)->SetValue( sal_True );
                    aReq.AppendItem( *pNewItem );
                }
                else if ( pNewItem->ISA(SfxEnumItemInterface) &&
                        ((SfxEnumItemInterface *)pNewItem)->HasBoolValue())
                {
                    // und Enums mit Bool-Interface
                    ((SfxEnumItemInterface*)pNewItem)->SetBoolValue(sal_True);
                    aReq.AppendItem( *pNewItem );
                }
                else {
                    DBG_ERROR( "Toggle only for Enums and Bools allowed" );
                }
                delete pNewItem;
            }
            else {
                DBG_ERROR( "suspicious Toggle-Slot" );
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
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    // synchronisieren
    pDispatcher->Flush();
//  pDispatcher->Update_Impl();

    if ( pImp->bAllMsgDirty )
    {
        if ( !nRegLevel )
        {
            ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame
                ( pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );
            //if ( xFrame.is() )
            //    xFrame->contextChanged();
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

#ifdef WNT
int __cdecl CmpUS_Impl(const void *p1, const void *p2)
#else
int CmpUS_Impl(const void *p1, const void *p2)
#endif

/*  [Beschreibung]

    Interne Vergleichsfunktion fuer qsort.
*/

{
    return *(sal_uInt16 *)p1 - *(sal_uInt16 *)p2;
}

//--------------------------------------------------------------------

SfxItemSet* SfxBindings::CreateSet_Impl
(
    SfxStateCache*&             pCache,     // in: Status-Cache von nId
    const SfxSlot*&             pRealSlot,  // out: RealSlot zu nId
    const SfxSlotServer**    pMsgServer, // out: Slot-Server zu nId
    SfxFoundCacheArr_Impl&      rFound      // out: Liste der Caches der Siblings
)
{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    DBG_ASSERT( !pImp->bMsgDirty, "CreateSet_Impl mit dirty MessageServer" );

    const SfxSlotServer* pMsgSvr = pCache->GetSlotServer(*pDispatcher, pImp->xProv);
    if(!pMsgSvr || !pDispatcher)
        return 0;

    DBG_PROFSTART(SfxBindingsCreateSet);
    pRealSlot = 0;
    *pMsgServer = pMsgSvr;

    sal_uInt16 nShellLevel = pMsgSvr->GetShellLevel();
    SfxShell *pShell = pDispatcher->GetShell( nShellLevel );
    if ( !pShell ) // seltener GPF beim Browsen durch Update aus Inet-Notify
        return 0;

    SfxItemPool &rPool = pShell->GetPool();

    // hole die Status-Methode, von der pCache bedient wird
    SfxStateFunc pFnc = 0;
    const SfxInterface *pInterface = pShell->GetInterface();
    if ( SFX_KIND_ENUM == pMsgSvr->GetSlot()->GetKind() )
    {
        pRealSlot = pInterface->GetRealSlot(pMsgSvr->GetSlot());
        pCache = GetStateCache( pRealSlot->GetSlotId() );
//      DBG_ASSERT( pCache, "Kein Slotcache fuer den Masterslot gefunden!" );
    }
    else
        pRealSlot = pMsgSvr->GetSlot();

    //
    // Achtung: pCache darf auch NULL sein !!!
    //

    pFnc = pRealSlot->GetStateFnc();

    // der RealSlot ist immer drin
    const SfxFoundCache_Impl *pFound = new SfxFoundCache_Impl(
        pRealSlot->GetSlotId(), pRealSlot->GetWhich(rPool), pRealSlot, pCache );
    rFound.Insert( pFound );

    sal_uInt16 nSlot = pRealSlot->GetSlotId();
    if ( !(nSlot >= SID_VERB_START && nSlot <= SID_VERB_END) )
    {
        pInterface = pInterface->GetRealInterfaceForSlot( pRealSlot );
        DBG_ASSERT (pInterface,"Slot in angegebener Shell nicht gefunden!");
    }

    // Durchsuche die Bindings nach den von derselben Funktion bedienten Slots.
    // Daf"ur kommen nur Slots in Frage, die es im gefundenen Interface gibt.

    // Die Position des Statecaches im StateCache-Array
    sal_uInt16 nCachePos = pImp->nMsgPos;
    const SfxSlot *pSibling = pRealSlot->GetNextSlot();

    // Die Slots eines Interfaces sind im Kreis verkettet
    while ( pSibling > pRealSlot )
    {
        SfxStateFunc pSiblingFnc=0;
        SfxStateCache *pSiblingCache =
                GetStateCache( pSibling->GetSlotId(), &nCachePos );

        // Ist der Slot "uberhaupt gecached ?
        if ( pSiblingCache )
        {
            const SfxSlotServer *pServ = pSiblingCache->GetSlotServer(*pDispatcher, pImp->xProv);
            if ( pServ && pServ->GetShellLevel() == nShellLevel )
                pSiblingFnc = pServ->GetSlot()->GetStateFnc();
        }

        // Mu\s der Slot "uberhaupt upgedatet werden ?
        bool bInsert = pSiblingCache && pSiblingCache->IsControllerDirty();

        // Bugfix #26161#: Es reicht nicht, nach der selben Shell zu fragen !!
        bool bSameMethod = pSiblingCache && pFnc == pSiblingFnc;

        // Wenn der Slot ein nicht-dirty MasterSlot ist, dann ist vielleicht
        // einer seiner Slaves dirty ? Dann wird der Masterslot doch eingef"ugt.
        if ( !bInsert && bSameMethod && pSibling->GetLinkedSlot() )
        {
            // auch Slave-Slots auf Binding pru"fen
            const SfxSlot* pFirstSlave = pSibling->GetLinkedSlot();
            for ( const SfxSlot *pSlaveSlot = pFirstSlave;
                  !bInsert;
                  pSlaveSlot = pSlaveSlot->GetNextSlot())
            {
                // Die Slaves zeigen auf ihren Master
                DBG_ASSERT(pSlaveSlot->GetLinkedSlot() == pSibling,
                    "Falsche Master/Slave-Beziehung!");

                sal_uInt16 nCurMsgPos = pImp->nMsgPos;
                const SfxStateCache *pSlaveCache =
                    GetStateCache( pSlaveSlot->GetSlotId(), &nCurMsgPos );

                // Ist der Slave-Slot gecached und dirty ?
                bInsert = pSlaveCache && pSlaveCache->IsControllerDirty();

                // Slaves sind untereinander im Kreis verkettet
                if (pSlaveSlot->GetNextSlot() == pFirstSlave)
                    break;
            }
        }

        if ( bInsert && bSameMethod )
        {
            const SfxFoundCache_Impl *pFoundCache = new SfxFoundCache_Impl(
                pSibling->GetSlotId(), pSibling->GetWhich(rPool),
                pSibling, pSiblingCache );

            rFound.Insert( pFoundCache );
        }

        pSibling = pSibling->GetNextSlot();
    }

    // aus den Ranges ein Set erzeugen
    sal_uInt16 *pRanges = new sal_uInt16[rFound.Count() * 2 + 1];
    int j = 0;
    sal_uInt16 i = 0;
    while ( i < rFound.Count() )
    {
        pRanges[j++] = rFound[i]->nWhichId;
            // aufeinanderfolgende Zahlen
        for ( ; i < rFound.Count()-1; ++i )
            if ( rFound[i]->nWhichId+1 != rFound[i+1]->nWhichId )
                break;
        pRanges[j++] = rFound[i++]->nWhichId;
    }
    pRanges[j] = 0; // terminierende NULL
    SfxItemSet *pSet = new SfxItemSet(rPool, pRanges);
    delete [] pRanges;
    DBG_PROFSTOP(SfxBindingsCreateSet);
    return pSet;
}

//--------------------------------------------------------------------

void SfxBindings::UpdateControllers_Impl
(
    const SfxInterface*         pIF,    // das diese Id momentan bedienende Interface
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

    // insofern gebunden, die Controller f"uer den Slot selbst updaten
    if ( pCache && pCache->IsControllerDirty() )
    {
        if ( SFX_ITEM_DONTCARE == eState )
        {
            // uneindeuting
            pCache->SetState( SFX_ITEM_DONTCARE, (SfxPoolItem *)-1 );
        }
        else if ( SFX_ITEM_DEFAULT == eState &&
                    pFound->nWhichId > SFX_WHICH_MAX )
        {
            // kein Status oder Default aber ohne Pool
            SfxVoidItem aVoid(0);
            pCache->SetState( SFX_ITEM_UNKNOWN, &aVoid );
        }
        else if ( SFX_ITEM_DISABLED == eState )
            pCache->SetState(SFX_ITEM_DISABLED, 0);
        else
            pCache->SetState(SFX_ITEM_AVAILABLE, pItem);
    }

    DBG_PROFSTOP(SfxBindingsUpdateCtrl1);

    // insofern vorhanden und gebunden, die Controller f"uer Slave-Slots
    // (Enum-Werte) des Slots updaten
    DBG_PROFSTART(SfxBindingsUpdateCtrl2);
    DBG_ASSERT( !pSlot || 0 == pSlot->GetLinkedSlot() || !pItem ||
                pItem->ISA(SfxEnumItemInterface),
                "master slot with non-enum-type found" );
    const SfxSlot *pFirstSlave = pSlot ? pSlot->GetLinkedSlot() : 0;
    if ( pIF && pFirstSlave)
    {
        // Items auf EnumItem casten
        const SfxEnumItemInterface *pEnumItem =
                PTR_CAST(SfxEnumItemInterface,pItem);
        if ( eState == SFX_ITEM_AVAILABLE && !pEnumItem )
            eState = SFX_ITEM_DONTCARE;
        else
            eState = SfxControllerItem::GetItemState( pEnumItem );

        // "uber alle Slaves-Slots iterieren
        for ( const SfxSlot *pSlave = pFirstSlave; pSlave; pSlave = pSlave->GetNextSlot() )
        {
            DBG_ASSERT(pSlave, "Falsche SlaveSlot-Verkettung!");
            DBG_ASSERT(SFX_KIND_ENUM == pSlave->GetKind(),"non enum slaves aren't allowed");
            DBG_ASSERT(pSlave->GetMasterSlotId() == pSlot->GetSlotId(),"falscher MasterSlot!");

            // ist die Funktion gebunden?
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
                    // enum-Wert ermitteln
                    sal_uInt16 nValue = pEnumItem->GetEnumValue();
                    SfxBoolItem aBool( pFound->nWhichId, pSlave->GetValue() == nValue );
                    pEnumCache->SetState(SFX_ITEM_AVAILABLE, &aBool);
                }
                else
                {
                    // uneindeuting
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
    // on Windows very often C++ Exceptions (GPF etc.) are caught by MSVCRT or another MS library
    // try to get them here
    try
    {
#endif
    const unsigned MAX_INPUT_DELAY = 200;

    DBG_MEMTEST();
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
    //<!--Modified by PengYunQuan for Validity Cell Range Picker
    //if ( (pFrame && pFrame->GetObjectShell()->IsInModalMode()) || pSfxApp->IsDowning() || !pImp->pCaches->Count() )
    if ( (pFrame && !pFrame->GetObjectShell()->AcceptStateUpdate()) || pSfxApp->IsDowning() || !pImp->pCaches->Count() )
    //-->Modified by PengYunQuan for Validity Cell Range Picker
    {
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_True;
    }
    if ( !pDispatcher || !pDispatcher->IsFlushed() )
    {
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_True;
    }

    // gfs. alle Server aktualisieren / geschieht in eigener Zeitscheibe
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
/*
                sal_Bool bSkip = sal_False;
                if ( pImp->bFirstRound )
                {
                    // Falls beim Update eine Shell vorgezogen werden soll,
                    // kommt in einer ersten Update-Runde nur diese dran
                    const SfxSlotServer *pMsgServer =
                        pCache->GetSlotServer(*pDispatcher, pImp->xProv);
                    if ( pMsgServer &&
                        pMsgServer->GetShellLevel() != pImp->nFirstShell )
                            bSkip = sal_True;
                }

                if ( !bSkip )
                {
*/
                    Update_Impl( pCache );
                    DBG_ASSERT( nCount == pImp->pCaches->Count(),
                            "Reschedule in StateChanged => buff" );
//              }
            }

            // skip to next function binding
            ++pImp->nMsgPos;

            // keep job if it is not completed, but any input is available
            bJobDone = pImp->nMsgPos >= nCount;
            if ( bJobDone && pImp->bFirstRound )
            {
                // Update der bevorzugten Shell ist gelaufen, nun d"urfen
                // auch die anderen
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

    // Update-Runde ist beendet
    pImp->bInNextJob = sal_False;
    Broadcast(SfxSimpleHint(SFX_HINT_UPDATEDONE));
    DBG_PROFSTOP(SfxBindingsNextJob_Impl);
    return sal_True;
#ifdef DBG_UTIL
    }
    catch (...)
    {
        DBG_ERROR("C++ exception caught!");
        pImp->bInNextJob = sal_False;
    }

    return sal_False;
#endif
}

//--------------------------------------------------------------------

sal_uInt16 SfxBindings::EnterRegistrations(const char *pFile, int nLine)
{
    (void)pFile;
    (void)nLine;
    DBG_MEMTEST();
#ifdef DBG_UTIL
    ByteString aMsg;
    aMsg.Fill( Min(nRegLevel, sal_uInt16(8) ) );
    aMsg += "this = ";
    aMsg += ByteString::CreateFromInt32((long)this);
    aMsg += " Level = ";
    aMsg += ByteString::CreateFromInt32(nRegLevel);
    aMsg += " SfxBindings::EnterRegistrations ";
    if(pFile) {
        aMsg += "File: ";
        aMsg += pFile;
        aMsg += " Line: ";
        aMsg += ByteString::CreateFromInt32(nLine);
    }
//    FILE* pLog = fopen( "c:\\bindings.log", "a+w" );
//    fwrite( aMsg.GetBuffer(), 1, aMsg.Len(), pLog );
//    fclose( pLog );
    DbgTrace( aMsg.GetBuffer() );
#endif

    // Wenn Bindings gelockt werden, auch SubBindings locken
    if ( pImp->pSubBindings )
    {
        pImp->pSubBindings->ENTERREGISTRATIONS();

        // Dieses EnterRegistrations ist f"ur die SubBindings kein "echtes"
        pImp->pSubBindings->pImp->nOwnRegLevel--;

        // Bindings synchronisieren
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

        // merken, ob ganze Caches verschwunden sind
        pImp->bCtrlReleased = sal_False;
    }

    return nRegLevel;
}
//--------------------------------------------------------------------

void SfxBindings::LeaveRegistrations( sal_uInt16 nLevel, const char *pFile, int nLine )
{
    (void)nLevel; // unused variable
    (void)pFile;
    (void)nLine;
    DBG_MEMTEST();
    DBG_ASSERT( nRegLevel, "Leave without Enter" );
    DBG_ASSERT( nLevel == USHRT_MAX || nLevel == nRegLevel, "wrong Leave" );

    // Nur wenn die SubBindings noch von den SuperBindings gelockt sind, diesen Lock entfernen
    // ( d.h. wenn es mehr Locks als "echte" Locks dort gibt )
    if ( pImp->pSubBindings && pImp->pSubBindings->nRegLevel > pImp->pSubBindings->pImp->nOwnRegLevel )
    {
        // Bindings synchronisieren
        pImp->pSubBindings->nRegLevel = nRegLevel + pImp->pSubBindings->pImp->nOwnRegLevel;

        // Dieses LeaveRegistrations ist f"ur die SubBindings kein "echtes"
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
            /*
            ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame
                ( pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );
            if ( xFrame.is() )
                xFrame->contextChanged();*/
        }

#ifndef slow
        SfxViewFrame* pFrame = pDispatcher->GetFrame();

        // ggf unbenutzte Caches entfernen bzw. PlugInInfo aufbereiten
        if ( pImp->bCtrlReleased )
        {
            for ( sal_uInt16 nCache = pImp->pCaches->Count(); nCache > 0; --nCache )
            {
                // Cache via ::com::sun::star::sdbcx::Index besorgen
                SfxStateCache *pCache = pImp->pCaches->GetObject(nCache-1);

                // kein Controller mehr interessiert
                if ( pCache->GetItemLink() == 0 && !pCache->GetInternalController() )
                {
                    // Cache entfernen. Safety: first remove and then delete
                    SfxStateCache* pSfxStateCache = (*pImp->pCaches)[nCache-1];
                    pImp->pCaches->Remove(nCache-1, 1);
                    delete pSfxStateCache;
                }
                else
                {
                    // neue Controller mit den alten Items benachrichtigen
                    //!pCache->SetCachedState();
                }
            }
        }
#endif
        // restart background-processing
        pImp->nMsgPos = 0;
        if ( !pFrame || !pFrame->GetObjectShell() )
            return;
        if ( pImp->pCaches && pImp->pCaches->Count() )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
//          pImp->bFirstRound = sal_True;
        }
    }

#ifdef DBG_UTIL
    ByteString aMsg;
    aMsg.Fill( Min(nRegLevel, sal_uInt16(8)) );
    aMsg += "this = ";
    aMsg += ByteString::CreateFromInt32((long)this);
    aMsg += " Level = ";
    aMsg += ByteString::CreateFromInt32(nRegLevel);
    aMsg += " SfxBindings::LeaveRegistrations ";
    if(pFile) {
        aMsg += "File: ";
        aMsg += pFile;
        aMsg += " Line: ";
        aMsg += ByteString::CreateFromInt32(nLine);
    }
//    FILE* pLog = fopen( "c:\\bindings.log", "a+w" );
//    fwrite( aMsg.GetBuffer(), 1, aMsg.Len(), pLog );
//    fclose( pLog );
    DbgTrace( aMsg.GetBuffer() );
#endif
}

//--------------------------------------------------------------------

const SfxSlot* SfxBindings::GetSlot(sal_uInt16 nSlotId)
{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    // syncronisieren
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
                DBG_ERROR( "SubBindings vor Aktivieren schon gesetzt!" );
                pImp->pSubBindings->ENTERREGISTRATIONS();
            }
            LEAVEREGISTRATIONS();
        }
        else if( !pDispatcher )
        {
            ENTERREGISTRATIONS();
            if ( pImp->pSubBindings && pImp->pSubBindings->pDispatcher != pOldDispat )
            {
                DBG_ERROR( "SubBindings im Deaktivieren immer noch gesetzt!" );
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
        // Update darf unterbrochen werden
        NextJob_Impl(&pImp->aTimer);
    else
        // alle Slots am St"uck updaten
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

    // Dann am Dispatcher testen; da die von dort zur"uckgegebenen Items immer
    // DELETE_ON_IDLE sind, mu\s eine Kopie davon gezogen werden, um einen
    // Eigent"umer"ubergang zu erm"oglichen
    const SfxPoolItem *pItem = NULL;
    SfxItemState eState = pDispatcher->QueryState( nSlot, pItem );
    if ( eState == SFX_ITEM_SET )
    {
        DBG_ASSERT( pItem, "SFX_ITEM_SET aber kein Item!" );
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
    pImp->pUnoCtrlArr->Insert( pControl, pImp->pUnoCtrlArr->Count() );
}

void SfxBindings::ReleaseUnoController_Impl( SfxUnoControllerItem* pControl )
{
    if ( pImp->pUnoCtrlArr )
    {
        sal_uInt16 nPos = pImp->pUnoCtrlArr->GetPos( pControl );
        if ( nPos != 0xFFFF )
        {
            pImp->pUnoCtrlArr->Remove( nPos );
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
        sal_uInt16 nCount = pImp->pUnoCtrlArr->Count();
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
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
    xTrans->parseStrict( aURL );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = pImp->xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
    if ( xDisp.is() )
    {
        if(::comphelper::UiEventsLogger::isEnabled()) //#i88653#
        {
            ::rtl::OUString sAppName;
            try
            {
                static ::rtl::OUString our_aModuleManagerName = ::rtl::OUString::createFromAscii("com.sun.star.frame.ModuleManager");
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager =
                    ::comphelper::getProcessServiceFactory();
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > xModuleManager(
                    xServiceManager->createInstance(our_aModuleManagerName)
                    , ::com::sun::star::uno::UNO_QUERY_THROW);
                ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame(
                    pDispatcher->GetFrame()->GetFrame().GetFrameInterface(), UNO_QUERY_THROW);
                sAppName = xModuleManager->identify(xFrame);
            } catch(::com::sun::star::uno::Exception&) {}
            Sequence<beans::PropertyValue> source;
            ::comphelper::UiEventsLogger::appendDispatchOrigin(source, sAppName, ::rtl::OUString::createFromAscii("SfxAsyncExec"));
            ::comphelper::UiEventsLogger::logDispatch(aURL, source);
        }
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
