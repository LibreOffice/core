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


#ifdef SOLARIS
#include <ctime>
#endif

#include <string>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/weak.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <comphelper/processfactory.hxx>

#include <sfx2/app.hxx>
#include <sfx2/appuno.hxx>
#include "statcach.hxx"
#include <sfx2/msg.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxuno.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

//====================================================================

DBG_NAME(SfxStateCache)
DBG_NAME(SfxStateCacheSetState)

//-----------------------------------------------------------------------------
BindDispatch_Impl::BindDispatch_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > & rDisp, const ::com::sun::star::util::URL& rURL, SfxStateCache *pStateCache, const SfxSlot* pS )
    : xDisp( rDisp )
    , aURL( rURL )
    , pCache( pStateCache )
    , pSlot( pS )
{
    DBG_ASSERT( pCache && pSlot, "Invalid BindDispatch!");
    aStatus.IsEnabled = sal_True;
}

void SAL_CALL BindDispatch_Impl::disposing( const ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( xDisp.is() )
    {
        xDisp->removeStatusListener( (::com::sun::star::frame::XStatusListener*) this, aURL );
        xDisp = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();
    }
}

void SAL_CALL  BindDispatch_Impl::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& rEvent ) throw( ::com::sun::star::uno::RuntimeException )
{
    aStatus = rEvent;
    if ( !pCache )
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  xRef( (::cppu::OWeakObject*)this, ::com::sun::star::uno::UNO_QUERY );
    if ( aStatus.Requery )
        pCache->Invalidate( sal_True );
    else
    {
        SfxPoolItem *pItem=NULL;
        sal_uInt16 nId = pCache->GetId();
        SfxItemState eState = SFX_ITEM_DISABLED;
        if ( !aStatus.IsEnabled )
        {
            // default
        }
        else if (aStatus.State.hasValue())
        {
            eState = SFX_ITEM_AVAILABLE;
            ::com::sun::star::uno::Any aAny = aStatus.State;

            ::com::sun::star::uno::Type pType = aAny.getValueType();
            if ( pType == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = false;
                aAny >>= bTemp ;
                pItem = new SfxBoolItem( nId, bTemp );
            }
            else if ( pType == ::getCppuType((const sal_uInt16*)0) )
            {
                sal_uInt16 nTemp = 0;
                aAny >>= nTemp ;
                pItem = new SfxUInt16Item( nId, nTemp );
            }
            else if ( pType == ::getCppuType((const sal_uInt32*)0) )
            {
                sal_uInt32 nTemp = 0;
                aAny >>= nTemp ;
                pItem = new SfxUInt32Item( nId, nTemp );
            }
            else if ( pType == ::getCppuType((const OUString*)0) )
            {
                OUString sTemp ;
                aAny >>= sTemp ;
                pItem = new SfxStringItem( nId, sTemp );
            }
            else
            {
                if ( pSlot )
                    pItem = pSlot->GetType()->CreateItem();
                if ( pItem )
                {
                    pItem->SetWhich( nId );
                    pItem->PutValue( aAny );
                }
                else
                    pItem = new SfxVoidItem( nId );
            }
        }
        else
        {
            // DONTCARE status
            pItem = new SfxVoidItem(0);
            eState = SFX_ITEM_UNKNOWN;
        }

        for ( SfxControllerItem *pCtrl = pCache->GetItemLink();
            pCtrl;
            pCtrl = pCtrl->GetItemLink() )
            pCtrl->StateChanged( nId, eState, pItem );

       delete pItem;
    }
}

void BindDispatch_Impl::Release()
{
    if ( xDisp.is() )
    {
        xDisp->removeStatusListener( (::com::sun::star::frame::XStatusListener*) this, aURL );
        xDisp = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();
    }

    pCache = NULL;
    release();
}

const ::com::sun::star::frame::FeatureStateEvent& BindDispatch_Impl::GetStatus() const
{
    return aStatus;
}

void BindDispatch_Impl::Dispatch( uno::Sequence < beans::PropertyValue > aProps, sal_Bool bForceSynchron )
{
    if ( xDisp.is() && aStatus.IsEnabled )
    {
        sal_Int32 nLength = aProps.getLength();
        aProps.realloc(nLength+1);
        aProps[nLength].Name = "SynchronMode";
        aProps[nLength].Value <<= bForceSynchron ;
        xDisp->dispatch( aURL, aProps );
    }
}

//--------------------------------------------------------------------
// This constructor for an invalid cache that is updated in the first request.

SfxStateCache::SfxStateCache( sal_uInt16 nFuncId ):
    pDispatch( 0 ),
    nId(nFuncId),
    pInternalController(0),
    pController(0),
    pLastItem( 0 ),
    eLastState( 0 ),
    bItemVisible( sal_True )
{
    DBG_CTOR(SfxStateCache, 0);
    bCtrlDirty = sal_True;
    bSlotDirty = sal_True;
    bItemDirty = sal_True;
}

//--------------------------------------------------------------------
// The Destructor checks by assertion, even if controllers are registered.

SfxStateCache::~SfxStateCache()
{
    DBG_DTOR(SfxStateCache, 0);
    DBG_ASSERT( pController == 0 && pInternalController == 0, "there are still Controllers registered" );
    if ( !IsInvalidItem(pLastItem) )
        delete pLastItem;
    if ( pDispatch )
    {
        pDispatch->Release();
        pDispatch = NULL;
    }
}

//--------------------------------------------------------------------
// invalidates the cache (next request will force update)
void SfxStateCache::Invalidate( sal_Bool bWithMsg )
{
    bCtrlDirty = sal_True;
    if ( bWithMsg )
    {
        bSlotDirty = sal_True;
        aSlotServ.SetSlot( 0 );
        if ( pDispatch )
        {
            pDispatch->Release();
            pDispatch = NULL;
        }
    }
}

//--------------------------------------------------------------------
// gets the corresponding function from the dispatcher or the cache

const SfxSlotServer* SfxStateCache::GetSlotServer( SfxDispatcher &rDispat , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & xProv )
{
    DBG_CHKTHIS(SfxStateCache, 0);

    if ( bSlotDirty )
    {
        // get the SlotServer; we need it for internal controllers anyway, but also in most cases
        rDispat._FindServer( nId, aSlotServ, sal_False );

        DBG_ASSERT( !pDispatch, "Old Dispatch not removed!" );

        // we don't need to check the dispatch provider if we only have an internal controller
        if ( xProv.is() )
        {
            const SfxSlot* pSlot = aSlotServ.GetSlot();
            if ( !pSlot )
                // get the slot - even if it is disabled on the dispatcher
                pSlot = SfxSlotPool::GetSlotPool( rDispat.GetFrame() ).GetSlot( nId );

            if ( !pSlot || !pSlot->pUnoName )
            {
                bSlotDirty = sal_False;
                bCtrlDirty = sal_True;
                return aSlotServ.GetSlot()? &aSlotServ: 0;
            }

            // create the dispatch URL from the slot data
            ::com::sun::star::util::URL aURL;
            OUString aCmd = ".uno:";
            aURL.Protocol = aCmd;
            aURL.Path = OUString::createFromAscii( pSlot->GetUnoName() );
            aCmd += aURL.Path;
            aURL.Complete = aCmd;
            aURL.Main = aCmd;

            // try to get a dispatch object for this command
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = xProv->queryDispatch( aURL, OUString(), 0 );
            if ( xDisp.is() )
            {
                // test the dispatch object if it is just a wrapper for a SfxDispatcher
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel( xDisp, ::com::sun::star::uno::UNO_QUERY );
                SfxOfficeDispatch* pDisp = NULL;
                if ( xTunnel.is() )
                {
                    sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                    pDisp = reinterpret_cast< SfxOfficeDispatch* >(sal::static_int_cast< sal_IntPtr >( nImplementation ));
                }

                if ( pDisp )
                {
                    // The intercepting object is an SFX component
                    // If this dispatch object does not use the wanted dispatcher or the AppDispatcher, it's treated like any other UNO component
                    // (intercepting by internal dispatches)
                    SfxDispatcher *pDispatcher = pDisp->GetDispatcher_Impl();
                    if ( pDispatcher == &rDispat || pDispatcher == SFX_APP()->GetAppDispatcher_Impl() )
                    {
                        // so we can use it directly
                        bSlotDirty = sal_False;
                        bCtrlDirty = sal_True;
                        return aSlotServ.GetSlot()? &aSlotServ: 0;
                    }
                }

                // so the dispatch object isn't a SfxDispatcher wrapper or it is one, but it uses another dispatcher, but not rDispat
                pDispatch = new BindDispatch_Impl( xDisp, aURL, this, pSlot );
                pDispatch->acquire();

                // flags must be set before adding StatusListener because the dispatch object will set the state
                bSlotDirty = sal_False;
                bCtrlDirty = sal_True;
                xDisp->addStatusListener( pDispatch, aURL );
            }
            else if ( rDispat.GetFrame() )
            {
                ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchProvider > xFrameProv(
                        rDispat.GetFrame()->GetFrame().GetFrameInterface(), ::com::sun::star::uno::UNO_QUERY );
                if ( xFrameProv != xProv )
                    return GetSlotServer( rDispat, xFrameProv );
            }
        }

        bSlotDirty = sal_False;
        bCtrlDirty = sal_True;
    }

    // we *always* return a SlotServer (if there is one); but in case of an external dispatch we might not use it
    // for the "real" (non internal) controllers
    return aSlotServ.GetSlot()? &aSlotServ: 0;
}


//--------------------------------------------------------------------

// Set Status in all Controllers

void SfxStateCache::SetState
(
    SfxItemState        eState,  // <SfxItemState> from 'pState'
    const SfxPoolItem*  pState,  // Slot Status, 0 or -1
    sal_Bool bMaybeDirty
)

/*  [Description]

    This method distributes the status of all of this SID bound
    <SfxControllerItem>s. If the value is the same as before, and if neither
    controller was registered nor invalidated inbetween, then no value is
    passed. This way the flickering is for example avoided in ListBoxes.
*/
{
    SetState_Impl( eState, pState, bMaybeDirty );
}

//--------------------------------------------------------------------

void SfxStateCache::SetVisibleState( sal_Bool bShow )
{
    SfxItemState        eState( SFX_ITEM_AVAILABLE );
    const SfxPoolItem*  pState( NULL );
    sal_Bool            bDeleteItem( sal_False );

    if ( bShow != bItemVisible )
    {
        bItemVisible = bShow;
        if ( bShow )
        {
            if ( IsInvalidItem(pLastItem) || ( pLastItem == NULL ))
            {
                pState = new SfxVoidItem( nId );
                bDeleteItem = sal_True;
            }
            else
                pState = pLastItem;

            eState = eLastState;
        }
        else
        {
            pState = new SfxVisibilityItem( nId, sal_False );
            bDeleteItem = sal_True;
        }

        // Update Controller
        if ( !pDispatch && pController )
        {
            for ( SfxControllerItem *pCtrl = pController;
                    pCtrl;
                    pCtrl = pCtrl->GetItemLink() )
                pCtrl->StateChanged( nId, eState, pState );
        }

        if ( pInternalController )
            pInternalController->StateChanged( nId, eState, pState );

        if ( bDeleteItem )
            delete pState;
    }
}

//--------------------------------------------------------------------

void SfxStateCache::SetState_Impl
(
    SfxItemState        eState,  // <SfxItemState> from 'pState'
    const SfxPoolItem*  pState,  // Slot Status, 0 or -1
    sal_Bool bMaybeDirty
)
{
    (void)bMaybeDirty; //unused
    DBG_CHKTHIS(SfxStateCache, 0);

    // If a hard update occurs between enter- and leave-registrations is a
    // can also intermediate Cached exist without controller.
    if ( !pController && !pInternalController )
        return;

    DBG_ASSERT( bMaybeDirty || !bSlotDirty, "setting state of dirty message" );
    DBG_ASSERT( SfxControllerItem::GetItemState(pState) == eState, "invalid SfxItemState" );
    DBG_PROFSTART(SfxStateCacheSetState);

    // does the controller have to be notified at all?
    bool bNotify = bItemDirty;
    if ( !bItemDirty )
    {
        bool bBothAvailable = pLastItem && pState &&
                    !IsInvalidItem(pState) && !IsInvalidItem(pLastItem);
        DBG_ASSERT( !bBothAvailable || pState != pLastItem, "setting state with own item" );
        if ( bBothAvailable )
            bNotify = pState->Type() != pLastItem->Type() ||
                      *pState != *pLastItem;
        else
            bNotify = ( pState != pLastItem ) || ( eState != eLastState );
    }

    if ( bNotify )
    {
        // Update Controller
        if ( !pDispatch && pController )
        {
            for ( SfxControllerItem *pCtrl = pController;
                pCtrl;
                pCtrl = pCtrl->GetItemLink() )
                pCtrl->StateChanged( nId, eState, pState );
        }

        if ( pInternalController )
            ((SfxDispatchController_Impl *)pInternalController)->StateChanged( nId, eState, pState, &aSlotServ );

        // Remember new value
        if ( !IsInvalidItem(pLastItem) )
            DELETEZ(pLastItem);
        if ( pState && !IsInvalidItem(pState) )
            pLastItem = pState->Clone();
        else
            pLastItem = 0;
        eLastState = eState;
        bItemDirty = sal_False;
    }

    bCtrlDirty = sal_False;
    DBG_PROFSTOP(SfxStateCacheSetState);
}


//--------------------------------------------------------------------
// Set old status again in all the controllers

void SfxStateCache::SetCachedState( sal_Bool bAlways )
{
    DBG_CHKTHIS(SfxStateCache, 0);
    DBG_ASSERT(pController==NULL||pController->GetId()==nId, "Cache with wrong ControllerItem" );
    DBG_PROFSTART(SfxStateCacheSetState);

    // Only update if cached item exists and also able to process.
    // (If the State is sent, it must be ensured that a SlotServer is present,
    // see SfxControllerItem:: GetCoreMetric())
    if ( bAlways || ( !bItemDirty && !bSlotDirty ) )
    {
        // Update Controller
        if ( !pDispatch && pController )
        {
            for ( SfxControllerItem *pCtrl = pController;
                pCtrl;
                pCtrl = pCtrl->GetItemLink() )
                pCtrl->StateChanged( nId, eLastState, pLastItem );
        }

        if ( pInternalController )
            ((SfxDispatchController_Impl *)pInternalController)->StateChanged( nId, eLastState, pLastItem, &aSlotServ );

        // Controller is now ok
        bCtrlDirty = sal_True;
    }

    DBG_PROFSTOP(SfxStateCacheSetState);
}


//--------------------------------------------------------------------
// Destroy FloatingWindows in all Controls with this Id

void SfxStateCache::DeleteFloatingWindows()
{
    DBG_CHKTHIS(SfxStateCache, 0);

    SfxControllerItem *pNextCtrl=0;
    for ( SfxControllerItem *pCtrl=pController; pCtrl; pCtrl=pNextCtrl )
    {
        pNextCtrl = pCtrl->GetItemLink();
        pCtrl->DeleteFloatingWindow();
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SfxStateCache::GetDispatch() const
{
    if ( pDispatch )
        return pDispatch->xDisp;
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();
}

void SfxStateCache::Dispatch( const SfxItemSet* pSet, sal_Bool bForceSynchron )
{
    // protect pDispatch against destruction in the call
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XStatusListener > xKeepAlive( pDispatch );
    if ( pDispatch )
    {
        uno::Sequence < beans::PropertyValue > aArgs;
        if (pSet)
            TransformItems( nId, *pSet, aArgs );
        pDispatch->Dispatch( aArgs, bForceSynchron );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
