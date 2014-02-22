/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/config.h>

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



static sal_uInt16 nTimeOut = 300;

#define TIMEOUT_FIRST       nTimeOut
#define TIMEOUT_UPDATING     20
#define TIMEOUT_IDLE       2500

typedef boost::unordered_map< sal_uInt16, bool > InvalidateSlotMap;



typedef std::vector<SfxStateCache*> SfxStateCacheArr_Impl;



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
    (void)pTimer; 
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
    SfxStateCacheArr_Impl*  pCaches;        
    sal_uInt16              nCachedFunc1;   
    sal_uInt16              nCachedFunc2;   
    sal_uInt16              nMsgPos;        
    SfxPopupAction          ePopupAction;   
    sal_Bool                bContextChanged;
    sal_Bool                bMsgDirty;      
    sal_Bool                bAllMsgDirty;   
    sal_Bool                bAllDirty;      
    sal_Bool                bCtrlReleased;  
    AutoTimer               aTimer;         
    sal_Bool                bInUpdate;      
    sal_Bool                bInNextJob;     
    sal_Bool                bFirstRound;    
    sal_uInt16              nFirstShell;    
    sal_uInt16              nOwnRegLevel;   
    InvalidateSlotMap       m_aInvalidateSlots; 
};



struct SfxFoundCache_Impl
{
    sal_uInt16      nSlotId;   
    sal_uInt16      nWhichId;  
    const SfxSlot*  pSlot;     
    SfxStateCache*  pCache;    

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

    bool operator<( const SfxFoundCache_Impl &r ) const
    { return nWhichId < r.nWhichId; }

    bool operator==( const SfxFoundCache_Impl &r ) const
    { return nWhichId== r.nWhichId; }
};



class SfxFoundCacheArr_Impl : public std::vector<SfxFoundCache_Impl*>
{
public:
    ~SfxFoundCacheArr_Impl()
    {
        for(const_iterator it = begin(); it != end(); ++it)
            delete *it;
    }
};



SfxBindings::SfxBindings()
:   pImp(new SfxBindings_Impl),
    pDispatcher(0),
    nRegLevel(1)    
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

    
    
    pImp->pCaches = new SfxStateCacheArr_Impl;
    pImp->aTimer.SetTimeoutHdl( LINK(this, SfxBindings, NextJob_Impl) );
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
    
    pImp->pSubBindings = NULL;

    ENTERREGISTRATIONS();

    pImp->aTimer.Stop();
    DeleteControllers_Impl();

    
    for(SfxStateCacheArr_Impl::const_iterator it = pImp->pCaches->begin(); it != pImp->pCaches->end(); ++it)
        delete *it;

    DELETEZ( pImp->pWorkWin );

    delete pImp->pCaches;
    delete pImp;
}



void SfxBindings::DeleteControllers_Impl()
{
    
    sal_uInt16 nCount = pImp->pCaches->size();
    sal_uInt16 nCache;
    for ( nCache = 0; nCache < nCount; ++nCache )
    {
        
        SfxStateCache *pCache = (*pImp->pCaches)[nCache];
        sal_uInt16 nSlotId = pCache->GetId();

        
        pCache->DeleteFloatingWindows();

        
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

    
    for ( nCache = pImp->pCaches->size(); nCache > 0; --nCache )
    {
        
        SfxStateCache *pCache = (*pImp->pCaches)[ nCache-1 ];

        
        SfxControllerItem *pNext;
        for ( SfxControllerItem *pCtrl = pCache->GetItemLink();
              pCtrl; pCtrl = pNext )
        {
            pNext = pCtrl->GetItemLink();
            pCtrl->UnBind();
        }

        if ( pCache->GetInternalController() )
            pCache->GetInternalController()->UnBind();

        
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
    
    HidePopupCtrls_Impl( bHide );
    SfxBindings *pSub = pImp->pSubBindings;
    while ( pSub )
    {
        pImp->pSubBindings->HidePopupCtrls_Impl( bHide );
        pSub = pSub->pImp->pSubBindings;
    }

    
    DBG_ASSERT( pDispatcher, "HidePopups not allowed without dispatcher" );
    if ( pImp->pWorkWin )
        pImp->pWorkWin->HidePopups_Impl( bHide, sal_True );
}

void SfxBindings::HidePopupCtrls_Impl( bool bHide )
{
    if ( bHide )
    {
        
        pImp->ePopupAction = SFX_POPUP_HIDE;
    }
    else
    {
        
        pImp->ePopupAction = SFX_POPUP_SHOW;
    }

    for(SfxStateCacheArr_Impl::const_iterator it = pImp->pCaches->begin(); it != pImp->pCaches->end(); ++it)
        (*it)->DeleteFloatingWindows();
    pImp->ePopupAction = SFX_POPUP_DELETE;
}



void SfxBindings::Update_Impl
(
    SfxStateCache*  pCache      
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

    
    SfxDispatcher &rDispat = *pDispatcher;
    const SfxSlot *pRealSlot = 0;
    const SfxSlotServer* pMsgServer = 0;
    SfxFoundCacheArr_Impl aFound;
    SfxItemSet *pSet = CreateSet_Impl( pCache, pRealSlot, &pMsgServer, aFound );
    sal_Bool bUpdated = sal_False;
    if ( pSet )
    {
        
        if ( rDispat._FillState( *pMsgServer, *pSet, pRealSlot ) )
        {
            
            const SfxInterface *pInterface =
                rDispat.GetShell(pMsgServer->GetShellLevel())->GetInterface();
            for ( sal_uInt16 nPos = 0; nPos < aFound.size(); ++nPos )
            {
                const SfxFoundCache_Impl *pFound = aFound[nPos];
                sal_uInt16 nWhich = pFound->nWhichId;
                const SfxPoolItem *pItem = 0;
                SfxItemState eState = pSet->GetItemState(nWhich, true, &pItem);
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
        
        
        
        SfxFoundCache_Impl aFoundCache(
                            pCache->GetId(), 0,
                            pRealSlot, pCache );
        UpdateControllers_Impl( 0, &aFoundCache, 0, SFX_ITEM_DISABLED);
    }

    DBG_PROFSTOP(SfxBindingsUpdate_Impl);
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
    sal_uInt16      nId     
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
            ; 
        pImp->bInUpdate = sal_False;
        InvalidateSlotsInMap_Impl();
    }
}



void SfxBindings::SetState
(
    const SfxItemSet&   rSet    
)
{
    
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
        
        if ( pImp->bMsgDirty )
            UpdateSlotServer_Impl();

        
        
        SfxItemIter aIter(rSet);
        for ( const SfxPoolItem *pItem = aIter.FirstItem();
              pItem;
              pItem = aIter.NextItem() )
        {
            SfxStateCache* pCache =
                    GetStateCache( rSet.GetPool()->GetSlotId(pItem->Which()) );
            if ( pCache )
            {
                
                if ( !pCache->IsControllerDirty() )
                    pCache->Invalidate(sal_False);
                pCache->SetState( SFX_ITEM_AVAILABLE, pItem );

                
            }
        }
    }
}



void SfxBindings::SetState
(
    const SfxPoolItem&  rItem   
)
{
    if ( nRegLevel )
    {
        Invalidate( rItem.Which() );
    }
    else
    {
        
        if ( pImp->bMsgDirty )
            UpdateSlotServer_Impl();

        
        DBG_ASSERT( SfxItemPool::IsSlot( rItem.Which() ),
                    "cannot set items with which-id" );
        SfxStateCache* pCache = GetStateCache( rItem.Which() );
        if ( pCache )
        {
            
            if ( !pCache->IsControllerDirty() )
                pCache->Invalidate(sal_False);
            pCache->SetState( SFX_ITEM_AVAILABLE, &rItem );

            
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
    return GetStateCache(nId, 0);
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
    
    const sal_uInt16 nStart = ( pPos ? *pPos : 0 );
    const sal_uInt16 nPos = GetSlotPos( nId, nStart );

    if ( nPos < pImp->pCaches->size() &&
         (*pImp->pCaches)[nPos]->GetId() == nId )
    {
        if ( pPos )
            *pPos = nPos;
        return (*pImp->pCaches)[nPos];
    }
    return 0;
}



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

    for ( sal_uInt16 n = 0; n < pImp->pCaches->size(); ++n )
        (*pImp->pCaches)[n]->Invalidate(bWithMsg);

    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.Stop();
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}



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

    
    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    
    for ( sal_uInt16 n = GetSlotPos(*pIds);
          *pIds && n < pImp->pCaches->size();
          n = GetSlotPos(*pIds, n) )
    {
        
        SfxStateCache *pCache = (*pImp->pCaches)[n];
        if ( pCache->GetId() == *pIds )
            pCache->Invalidate(sal_False);

        
        if ( !*++pIds )
            break;
        DBG_ASSERT( *pIds > *(pIds-1), "pIds unsorted" );
    }

    
    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.Stop();
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}



void SfxBindings::InvalidateShell
(
    const SfxShell&  rSh,  /* <SfxShell>, which Slot-Ids should be
                              invalidated */
    sal_Bool         bDeep /* sal_True
                              also inherited slot IDs of SfxShell are invalidert

                              sal_False
                              the inherited and not overloaded Slot-Ids were
                              invalidiert */
                             
)
{
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateShell( rSh, bDeep );

    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    DBG_PROFSTART(SfxBindingsInvalidateAll);

    
    
    pDispatcher->Flush();

    if ( !pDispatcher ||
         ( pImp->bAllDirty && pImp->bAllMsgDirty ) ||
         SFX_APP()->IsDowning() )
    {
        
        return;
    }

    
    sal_uInt16 nLevel = pDispatcher->GetShellLevel(rSh);
    if ( nLevel != USHRT_MAX )
    {
        for ( sal_uInt16 n = 0; n < pImp->pCaches->size(); ++n )
        {
            SfxStateCache *pCache = (*pImp->pCaches)[n];
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



void SfxBindings::Invalidate
(
    sal_uInt16 nId              
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
    sal_uInt16  nId,                
    sal_Bool    bWithItem,          
    sal_Bool    bWithMsg            
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

        pImp->nMsgPos = std::min(GetSlotPos(nId), pImp->nMsgPos);
        if ( !nRegLevel )
        {
            pImp->aTimer.Stop();
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
        }
    }
}



sal_Bool SfxBindings::IsBound( sal_uInt16 nSlotId, sal_uInt16 nStartSearchAt )
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    return GetStateCache(nSlotId, &nStartSearchAt ) != 0;
}



sal_uInt16 SfxBindings::GetSlotPos( sal_uInt16 nId, sal_uInt16 nStartSearchAt )
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    DBG_PROFSTART(SfxBindingsMsgPos);

    
    if ( pImp->nCachedFunc1 < pImp->pCaches->size() &&
         (*pImp->pCaches)[pImp->nCachedFunc1]->GetId() == nId )
    {
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return pImp->nCachedFunc1;
    }
    if ( pImp->nCachedFunc2 < pImp->pCaches->size() &&
         (*pImp->pCaches)[pImp->nCachedFunc2]->GetId() == nId )
    {
        
        sal_uInt16 nTemp = pImp->nCachedFunc1;
        pImp->nCachedFunc1 = pImp->nCachedFunc2;
        pImp->nCachedFunc2 = nTemp;
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return pImp->nCachedFunc1;
    }

    
    if ( pImp->pCaches->size() <= nStartSearchAt )
    {
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return 0;
    }
    if ( (sal_uInt16) pImp->pCaches->size() == (nStartSearchAt+1) )
    {
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return (*pImp->pCaches)[nStartSearchAt]->GetId() >= nId ? 0 : 1;
    }
    sal_uInt16 nLow = nStartSearchAt;
    sal_uInt16 nMid = 0;
    sal_uInt16 nHigh = 0;
    sal_Bool bFound = sal_False;
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
            bFound = sal_True;
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
    DBG_PROFSTOP(SfxBindingsMsgPos);
    return nPos;
}

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

    DBG_ASSERT( !pImp->bInNextJob, "SfxBindings::Register while status-updating" );

    
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
        pImp->bMsgDirty = sal_True;
    }

    
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
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    DBG_ASSERT( !pImp->bInNextJob, "SfxBindings::Release while status-updating" );
    ENTERREGISTRATIONS();

    
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
            
            SfxControllerItem* pItem = pCache->GetItemLink();
            if ( pItem == &rItem )
                pCache->ChangeItemLink( rItem.GetItemLink() );
            else
            {
                
                while ( pItem && pItem->GetItemLink() != &rItem )
                    pItem = pItem->GetItemLink();

                
                if ( pItem )
                    pItem->ChangeItemLink( rItem.GetItemLink() );
            }
        }

        
        if ( pCache->GetItemLink() == 0 && !pCache->GetInternalController() )
        {
            pImp->bCtrlReleased = sal_True;
        }
    }

    LEAVEREGISTRATIONS();
}


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
    rDispatcher.GetFrame();  

    
    ::boost::scoped_ptr<SfxStateCache> xCache;
    if ( !pCache )
    {
        
        
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

        
        pCache->Dispatch( aReq.GetArgs(), nCallMode == SFX_CALLMODE_SYNCHRON );
        SfxPoolItem *pVoid = new SfxVoidItem( nId );
        DeleteItemOnIdle( pVoid );
        return pVoid;
    }

    
    if ( pImp->bMsgDirty )
        UpdateSlotServer_Impl();

    SfxShell *pShell=0;
    const SfxSlot *pSlot=0;

    
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
        
        
        const SfxSlot *pRealSlot = pShell->GetInterface()->GetRealSlot(pSlot);
        const sal_uInt16 nSlotId = pRealSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        aReq.AppendItem( SfxAllEnumItem( rPool.GetWhich(nSlotId), pSlot->GetValue() ) );
        pDispatcher->_Execute( *pShell, *pRealSlot, aReq, aReq.GetCallMode() | SFX_CALLMODE_RECORD );
    }
    else if ( SFX_KIND_ATTR == pSlot->GetKind() )
    {
        
        const sal_uInt16 nSlotId = pSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        if ( pSlot->IsMode(SFX_SLOT_TOGGLE) )
        {
            
            sal_uInt16 nWhich = pSlot->GetWhich(rPool);
            SfxItemSet aSet(rPool, nWhich, nWhich);
            SfxStateFunc aFunc  = pSlot->GetStateFnc();
            pShell->CallState( aFunc, aSet );
            const SfxPoolItem *pOldItem;
            SfxItemState eState = aSet.GetItemState(nWhich, true, &pOldItem);
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
                    
                    sal_Bool bOldValue = ((const SfxBoolItem *)pOldItem)->GetValue();
                    SfxBoolItem *pNewItem = (SfxBoolItem*) (pOldItem->Clone());
                    pNewItem->SetValue( !bOldValue );
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else if ( pOldItem->ISA(SfxEnumItemInterface) &&
                        ((SfxEnumItemInterface *)pOldItem)->HasBoolValue())
                {
                    
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
                
                SfxPoolItem *pNewItem = pSlot->GetType()->CreateItem();
                DBG_ASSERT( pNewItem, "Toggle to slot without ItemFactory" );
                pNewItem->SetWhich( nWhich );

                if ( pNewItem->ISA(SfxBoolItem) )
                {
                  
                    ((SfxBoolItem*)pNewItem)->SetValue( true );
                    aReq.AppendItem( *pNewItem );
                }
                else if ( pNewItem->ISA(SfxEnumItemInterface) &&
                        ((SfxEnumItemInterface *)pNewItem)->HasBoolValue())
                {
                    
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



void SfxBindings::UpdateSlotServer_Impl()
{
    DBG_PROFSTART(SfxBindingsUpdateServers);
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    
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

    for (size_t i = 0, nCount = pImp->pCaches->size(); i < nCount; ++i)
    {
        SfxStateCache *pCache = (*pImp->pCaches)[i];
        
        pCache->GetSlotServer(*pDispatcher, pImp->xProv);
    }
    pImp->bMsgDirty = pImp->bAllMsgDirty = sal_False;

    Broadcast( SfxSimpleHint(SFX_HINT_DOCCHANGED) );

    DBG_PROFSTOP(SfxBindingsUpdateServers);
}



SfxItemSet* SfxBindings::CreateSet_Impl
(
    SfxStateCache*&         pCache,     
    const SfxSlot*&         pRealSlot,  
    const SfxSlotServer**   pMsgServer, 
    SfxFoundCacheArr_Impl&  rFound      
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
    if ( !pShell ) 
        return 0;

    SfxItemPool &rPool = pShell->GetPool();

    
    SfxStateFunc pFnc = 0;
    const SfxInterface *pInterface = pShell->GetInterface();
    if ( SFX_KIND_ENUM == pMsgSvr->GetSlot()->GetKind() )
    {
        pRealSlot = pInterface->GetRealSlot(pMsgSvr->GetSlot());
        pCache = GetStateCache( pRealSlot->GetSlotId() );
    }
    else
        pRealSlot = pMsgSvr->GetSlot();

    

    pFnc = pRealSlot->GetStateFnc();

    
    SfxFoundCache_Impl *pFound = new SfxFoundCache_Impl(
        pRealSlot->GetSlotId(), pRealSlot->GetWhich(rPool), pRealSlot, pCache );
    rFound.push_back( pFound );

    sal_uInt16 nSlot = pRealSlot->GetSlotId();
    if ( !(nSlot >= SID_VERB_START && nSlot <= SID_VERB_END) )
    {
        pInterface = pInterface->GetRealInterfaceForSlot( pRealSlot );
        DBG_ASSERT (pInterface,"Slot in the given shell is not found");
    }

    

    
    sal_uInt16 nCachePos = pImp->nMsgPos;
    const SfxSlot *pSibling = pRealSlot->GetNextSlot();

    
    while ( pSibling > pRealSlot )
    {
        SfxStateFunc pSiblingFnc=0;
        SfxStateCache *pSiblingCache =
                GetStateCache( pSibling->GetSlotId(), &nCachePos );

        
        if ( pSiblingCache )
        {
            const SfxSlotServer *pServ = pSiblingCache->GetSlotServer(*pDispatcher, pImp->xProv);
            if ( pServ && pServ->GetShellLevel() == nShellLevel )
                pSiblingFnc = pServ->GetSlot()->GetStateFnc();
        }

        
        bool bInsert = pSiblingCache && pSiblingCache->IsControllerDirty();

        
        bool bSameMethod = pSiblingCache && pFnc == pSiblingFnc;

        
        
        if ( !bInsert && bSameMethod && pSibling->GetLinkedSlot() )
        {
            
            const SfxSlot* pFirstSlave = pSibling->GetLinkedSlot();
            for ( const SfxSlot *pSlaveSlot = pFirstSlave;
                  !bInsert;
                  pSlaveSlot = pSlaveSlot->GetNextSlot())
            {
                
                DBG_ASSERT(pSlaveSlot->GetLinkedSlot() == pSibling,
                    "Wrong Master/Slave relationship!");

                sal_uInt16 nCurMsgPos = pImp->nMsgPos;
                const SfxStateCache *pSlaveCache =
                    GetStateCache( pSlaveSlot->GetSlotId(), &nCurMsgPos );

                
                bInsert = pSlaveCache && pSlaveCache->IsControllerDirty();

                
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

    
    sal_uInt16 *pRanges = new sal_uInt16[rFound.size() * 2 + 1];
    int j = 0;
    sal_uInt16 i = 0;
    while ( i < rFound.size() )
    {
        pRanges[j++] = rFound[i]->nWhichId;
            
        for ( ; i < rFound.size()-1; ++i )
            if ( rFound[i]->nWhichId+1 != rFound[i+1]->nWhichId )
                break;
        pRanges[j++] = rFound[i++]->nWhichId;
    }
    pRanges[j] = 0; 
    SfxItemSet *pSet = new SfxItemSet(rPool, pRanges);
    delete [] pRanges;
    DBG_PROFSTOP(SfxBindingsCreateSet);
    return pSet;
}



void SfxBindings::UpdateControllers_Impl
(
    const SfxInterface*         pIF,    
    const SfxFoundCache_Impl*   pFound, 
    const SfxPoolItem*          pItem,  
    SfxItemState                eState  
)
{
    DBG_ASSERT( !pFound->pSlot || SFX_KIND_ENUM != pFound->pSlot->GetKind(),
                "direct update of enum slot isn't allowed" );
    DBG_PROFSTART(SfxBindingsUpdateCtrl1);

    SfxStateCache* pCache = pFound->pCache;
    const SfxSlot* pSlot = pFound->pSlot;
    DBG_ASSERT( !pCache || !pSlot || pCache->GetId() == pSlot->GetSlotId(), "SID mismatch" );

    
    if ( pCache && pCache->IsControllerDirty() )
    {
        if ( SFX_ITEM_DONTCARE == eState )
        {
            
            pCache->SetState( SFX_ITEM_DONTCARE, (SfxPoolItem *)-1 );
        }
        else if ( SFX_ITEM_DEFAULT == eState &&
                    pFound->nWhichId > SFX_WHICH_MAX )
        {
            
            SfxVoidItem aVoid(0);
            pCache->SetState( SFX_ITEM_UNKNOWN, &aVoid );
        }
        else if ( SFX_ITEM_DISABLED == eState )
            pCache->SetState(SFX_ITEM_DISABLED, 0);
        else
            pCache->SetState(SFX_ITEM_AVAILABLE, pItem);
    }

    DBG_PROFSTOP(SfxBindingsUpdateCtrl1);

    
    
    DBG_PROFSTART(SfxBindingsUpdateCtrl2);
    DBG_ASSERT( !pSlot || 0 == pSlot->GetLinkedSlot() || !pItem ||
                pItem->ISA(SfxEnumItemInterface),
                "master slot with non-enum-type found" );
    const SfxSlot *pFirstSlave = pSlot ? pSlot->GetLinkedSlot() : 0;
    if ( pIF && pFirstSlave)
    {
        
        const SfxEnumItemInterface *pEnumItem =
                PTR_CAST(SfxEnumItemInterface,pItem);
        if ( eState == SFX_ITEM_AVAILABLE && !pEnumItem )
            eState = SFX_ITEM_DONTCARE;
        else
            eState = SfxControllerItem::GetItemState( pEnumItem );

        
        for ( const SfxSlot *pSlave = pFirstSlave; pSlave; pSlave = pSlave->GetNextSlot() )
        {
            DBG_ASSERT(pSlave, "Wrong SlaveSlot binding!");
            DBG_ASSERT(SFX_KIND_ENUM == pSlave->GetKind(),"non enum slaves aren't allowed");
            DBG_ASSERT(pSlave->GetMasterSlotId() == pSlot->GetSlotId(),"Wrong MasterSlot!");

            
            SfxStateCache *pEnumCache = GetStateCache( pSlave->GetSlotId() );
            if ( pEnumCache )
            {
                pEnumCache->Invalidate(sal_False);

                
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
                    
                    pEnumCache->SetState(SFX_ITEM_DISABLED, 0);
                }
                else if ( SFX_ITEM_AVAILABLE == eState )
                {
                    
                    sal_uInt16 nValue = pEnumItem->GetEnumValue();
                    SfxBoolItem aBool( pFound->nWhichId, pSlave->GetValue() == nValue );
                    pEnumCache->SetState(SFX_ITEM_AVAILABLE, &aBool);
                }
                else
                {
                    
                    pEnumCache->SetState( SFX_ITEM_DONTCARE, (SfxPoolItem *)-1 );
                }
            }

            if (pSlave->GetNextSlot() == pFirstSlave)
                break;
        }
    }

    DBG_PROFSTOP(SfxBindingsUpdateCtrl2);
}




IMPL_LINK( SfxBindings, NextJob_Impl, Timer *, pTimer )
{
#ifdef DBG_UTIL
    
    
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

    
    SfxViewFrame* pFrame = pDispatcher ? pDispatcher->GetFrame() : NULL;
    if ( (pFrame && !pFrame->GetObjectShell()->AcceptStateUpdate()) || pSfxApp->IsDowning() || pImp->pCaches->empty() )
    {
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_True;
    }
    if ( !pDispatcher || !pDispatcher->IsFlushed() )
    {
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_True;
    }

    
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

    
    bool bPreEmptive = pTimer && !pSfxApp->Get_Impl()->nInReschedule;
    sal_uInt16 nLoops = 10;
    pImp->bInNextJob = sal_True;
    const sal_uInt16 nCount = pImp->pCaches->size();
    while ( pImp->nMsgPos < nCount )
    {
        
        sal_Bool bJobDone = sal_False;
        while ( !bJobDone )
        {
            SfxStateCache* pCache = (*pImp->pCaches)[pImp->nMsgPos];
            DBG_ASSERT( pCache, "invalid SfxStateCache-position in job queue" );
            sal_Bool bWasDirty = pCache->IsControllerDirty();
            if ( bWasDirty )
            {
                    Update_Impl( pCache );
                    DBG_ASSERT( nCount == pImp->pCaches->size(),
                            "Reschedule in StateChanged => buff" );
            }

            
            ++pImp->nMsgPos;

            
            bJobDone = pImp->nMsgPos >= nCount;
            if ( bJobDone && pImp->bFirstRound )
            {

                
                
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



sal_uInt16 SfxBindings::EnterRegistrations(const char *pFile, int nLine)
{
    SAL_INFO(
        "sfx.control",
        std::setw(std::min(nRegLevel, sal_uInt16(8))) << ' ' << "this = " << this
            << " Level = " << nRegLevel << " SfxBindings::EnterRegistrations "
            << (pFile
                ? SAL_STREAM("File: " << pFile << " Line: " << nLine) : ""));

    
    if ( pImp->pSubBindings )
    {
        pImp->pSubBindings->ENTERREGISTRATIONS();

        
        pImp->pSubBindings->pImp->nOwnRegLevel--;

        
        pImp->pSubBindings->nRegLevel = nRegLevel + pImp->pSubBindings->pImp->nOwnRegLevel + 1;
    }

    pImp->nOwnRegLevel++;

    
    if ( ++nRegLevel == 1 )
    {
        
        pImp->aTimer.Stop();

        
        pImp->nCachedFunc1 = 0;
        pImp->nCachedFunc2 = 0;

        
        pImp->bCtrlReleased = sal_False;
    }

    return nRegLevel;
}


void SfxBindings::LeaveRegistrations( sal_uInt16 nLevel, const char *pFile, int nLine )
{
    (void)nLevel; 
    DBG_ASSERT( nRegLevel, "Leave without Enter" );
    DBG_ASSERT( nLevel == USHRT_MAX || nLevel == nRegLevel, "wrong Leave" );

    
    
    if ( pImp->pSubBindings && pImp->pSubBindings->nRegLevel > pImp->pSubBindings->pImp->nOwnRegLevel )
    {
        
        pImp->pSubBindings->nRegLevel = nRegLevel + pImp->pSubBindings->pImp->nOwnRegLevel;

        
        pImp->pSubBindings->pImp->nOwnRegLevel++;
        pImp->pSubBindings->LEAVEREGISTRATIONS();
    }

    pImp->nOwnRegLevel--;

    
    if ( --nRegLevel == 0 && !SFX_APP()->IsDowning() )
    {
        if ( pImp->bContextChanged )
        {
            pImp->bContextChanged = sal_False;
        }

        SfxViewFrame* pFrame = pDispatcher->GetFrame();

        
        if ( pImp->bCtrlReleased )
        {
            for ( sal_uInt16 nCache = pImp->pCaches->size(); nCache > 0; --nCache )
            {
                
                SfxStateCache *pCache = (*pImp->pCaches)[nCache-1];

                
                if ( pCache->GetItemLink() == 0 && !pCache->GetInternalController() )
                {
                    
                    pImp->pCaches->erase(pImp->pCaches->begin() + nCache - 1);
                    delete pCache;
                }
            }
        }

        
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



const SfxSlot* SfxBindings::GetSlot(sal_uInt16 nSlotId)
{
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    
    pDispatcher->Flush();
    if ( pImp->bMsgDirty )
        UpdateSlotServer_Impl();

    
    SfxStateCache* pCache = GetStateCache(nSlotId);
    return pCache && pCache->GetSlotServer(*pDispatcher, pImp->xProv)?
            pCache->GetSlotServer(*pDispatcher, pImp->xProv)->GetSlot(): 0;
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



void SfxBindings::ClearCache_Impl( sal_uInt16 nSlotId )
{
    SfxStateCache* pCache = GetStateCache(nSlotId);
    if (!pCache)
        return;
    pCache->ClearCache();
}


void SfxBindings::StartUpdate_Impl( sal_Bool bComplete )
{
    if ( pImp->pSubBindings )
        pImp->pSubBindings->StartUpdate_Impl( bComplete );

    if ( !bComplete )
        
        NextJob_Impl(&pImp->aTimer);
    else
        
        NextJob_Impl(0);
}



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
                    else if ( pType == ::getCppuType((const OUString*)0) )
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

sal_Bool SfxBindings::ExecuteCommand_Impl( const OUString& rCommand )
{
    ::com::sun::star::util::URL aURL;
    aURL.Complete = rCommand;
    Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aURL );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = pImp->xProv->queryDispatch( aURL, OUString(), 0 );
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
