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

BindDispatch_Impl::BindDispatch_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > & rDisp, const ::com::sun::star::util::URL& rURL, SfxStateCache *pStateCache, const SfxSlot* pS )
    : xDisp( rDisp )
    , aURL( rURL )
    , pCache( pStateCache )
    , pSlot( pS )
{
    DBG_ASSERT( pCache && pSlot, "Invalid BindDispatch!");
    aStatus.IsEnabled = sal_True;
}

void SAL_CALL BindDispatch_Impl::disposing( const ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    if ( xDisp.is() )
    {
        xDisp->removeStatusListener( (::com::sun::star::frame::XStatusListener*) this, aURL );
        xDisp = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();
    }
}

void SAL_CALL  BindDispatch_Impl::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& rEvent ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    aStatus = rEvent;
    if ( !pCache )
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  xRef( (::cppu::OWeakObject*)this, ::com::sun::star::uno::UNO_QUERY );
    if ( aStatus.Requery )
        pCache->Invalidate( true );
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
                bool bTemp = false;
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

void BindDispatch_Impl::Dispatch( uno::Sequence < beans::PropertyValue > aProps, bool bForceSynchron )
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


// This constructor for an invalid cache that is updated in the first request.

SfxStateCache::SfxStateCache( sal_uInt16 nFuncId ):
    pDispatch( 0 ),
    nId(nFuncId),
    pInternalController(0),
    pController(0),
    pLastItem( 0 ),
    eLastState( 0 ),
    bItemVisible( true )
{
    bCtrlDirty = true;
    bSlotDirty = true;
    bItemDirty = true;
}


// The Destructor checks by assertion, even if controllers are registered.

SfxStateCache::~SfxStateCache()
{
    DBG_ASSERT( pController == 0 && pInternalController == 0, "there are still Controllers registered" );
    if ( !IsInvalidItem(pLastItem) )
        delete pLastItem;
    if ( pDispatch )
    {
        pDispatch->Release();
        pDispatch = NULL;
    }
}


// invalidates the cache (next request will force update)
void SfxStateCache::Invalidate( bool bWithMsg )
{
    bCtrlDirty = true;
    if ( bWithMsg )
    {
        bSlotDirty = true;
        aSlotServ.SetSlot( 0 );
        if ( pDispatch )
        {
            pDispatch->Release();
            pDispatch = NULL;
        }
    }
}


// gets the corresponding function from the dispatcher or the cache

const SfxSlotServer* SfxStateCache::GetSlotServer( SfxDispatcher &rDispat , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & xProv )
{

    if ( bSlotDirty )
    {
        // get the SlotServer; we need it for internal controllers anyway, but also in most cases
        rDispat._FindServer( nId, aSlotServ, false );

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
                bSlotDirty = false;
                bCtrlDirty = true;
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
                        bSlotDirty = false;
                        bCtrlDirty = true;
                        return aSlotServ.GetSlot()? &aSlotServ: 0;
                    }
                }

                // so the dispatch object isn't a SfxDispatcher wrapper or it is one, but it uses another dispatcher, but not rDispat
                pDispatch = new BindDispatch_Impl( xDisp, aURL, this, pSlot );
                pDispatch->acquire();

                // flags must be set before adding StatusListener because the dispatch object will set the state
                bSlotDirty = false;
                bCtrlDirty = true;
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

        bSlotDirty = false;
        bCtrlDirty = true;
    }

    // we *always* return a SlotServer (if there is one); but in case of an external dispatch we might not use it
    // for the "real" (non internal) controllers
    return aSlotServ.GetSlot()? &aSlotServ: 0;
}




// Set Status in all Controllers

void SfxStateCache::SetState
(
    SfxItemState        eState,  // <SfxItemState> from 'pState'
    const SfxPoolItem*  pState,  // Slot Status, 0 or -1
    bool bMaybeDirty
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



void SfxStateCache::SetVisibleState( bool bShow )
{
    SfxItemState        eState( SFX_ITEM_AVAILABLE );
    const SfxPoolItem*  pState( NULL );
    bool            bDeleteItem( false );

    if ( bShow != bItemVisible )
    {
        bItemVisible = bShow;
        if ( bShow )
        {
            if ( IsInvalidItem(pLastItem) || ( pLastItem == NULL ))
            {
                pState = new SfxVoidItem( nId );
                bDeleteItem = true;
            }
            else
                pState = pLastItem;

            eState = eLastState;
        }
        else
        {
            pState = new SfxVisibilityItem( nId, false );
            bDeleteItem = true;
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



void SfxStateCache::SetState_Impl
(
    SfxItemState        eState,  // <SfxItemState> from 'pState'
    const SfxPoolItem*  pState,  // Slot Status, 0 or -1
    bool bMaybeDirty
)
{
    (void)bMaybeDirty; //unused

    // If a hard update occurs between enter- and leave-registrations is a
    // can also intermediate Cached exist without controller.
    if ( !pController && !pInternalController )
        return;

    DBG_ASSERT( bMaybeDirty || !bSlotDirty, "setting state of dirty message" );
    DBG_ASSERT( SfxControllerItem::GetItemState(pState) == eState, "invalid SfxItemState" );

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
        bItemDirty = false;
    }

    bCtrlDirty = false;
}



// Set old status again in all the controllers

void SfxStateCache::SetCachedState( bool bAlways )
{
    DBG_ASSERT(pController==NULL||pController->GetId()==nId, "Cache with wrong ControllerItem" );

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
        bCtrlDirty = true;
    }
}



// Destroy FloatingWindows in all Controls with this Id

void SfxStateCache::DeleteFloatingWindows()
{

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

void SfxStateCache::Dispatch( const SfxItemSet* pSet, bool bForceSynchron )
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
