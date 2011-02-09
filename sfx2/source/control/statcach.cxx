/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

SFX_IMPL_XINTERFACE_2( BindDispatch_Impl, OWeakObject, ::com::sun::star::frame::XStatusListener, ::com::sun::star::lang::XEventListener )
SFX_IMPL_XTYPEPROVIDER_2( BindDispatch_Impl, ::com::sun::star::frame::XStatusListener, ::com::sun::star::lang::XEventListener )

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
        // pCache->Invalidate( sal_False );
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
            else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
            {
                ::rtl::OUString sTemp ;
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
        aProps[nLength].Name = DEFINE_CONST_UNICODE("SynchronMode");
        aProps[nLength].Value <<= bForceSynchron ;
        xDisp->dispatch( aURL, aProps );
    }
}

//--------------------------------------------------------------------

/*  Dieser Konstruktor fuer einen ungueltigen Cache, der sich also
    bei der ersten Anfrage zun"achst updated.
 */

SfxStateCache::SfxStateCache( sal_uInt16 nFuncId ):
    pDispatch( 0 ),
    nId(nFuncId),
    pInternalController(0),
    pController(0),
    pLastItem( 0 ),
    eLastState( 0 ),
    bItemVisible( sal_True )
{
    DBG_MEMTEST();
    DBG_CTOR(SfxStateCache, 0);
    bCtrlDirty = sal_True;
    bSlotDirty = sal_True;
    bItemDirty = sal_True;
}

//--------------------------------------------------------------------

/*  Der Destruktor pr"uft per Assertion, ob noch Controller angemeldet
    sind.
 */

SfxStateCache::~SfxStateCache()
{
    DBG_MEMTEST();
    DBG_DTOR(SfxStateCache, 0);
    DBG_ASSERT( pController == 0 && pInternalController == 0, "es sind noch Controller angemeldet" );
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
    DBG_MEMTEST();
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
            ::rtl::OUString aCmd = DEFINE_CONST_UNICODE(".uno:");
            aURL.Protocol = aCmd;
            aURL.Path = ::rtl::OUString::createFromAscii( pSlot->GetUnoName() );
            aCmd += aURL.Path;
            aURL.Complete = aCmd;
            aURL.Main = aCmd;

            // try to get a dispatch object for this command
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
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

// Status setzen in allen Controllern

void SfxStateCache::SetState
(
    SfxItemState        eState,     // <SfxItemState> von 'pState'
    const SfxPoolItem*  pState,     // Status des Slots, ggf. 0 oder -1
    BOOL bMaybeDirty
)

/*  [Beschreibung]

    Diese Methode verteilt die Status auf alle an dieser SID gebundenen
    <SfxControllerItem>s. Ist der Wert derselbe wie zuvor und wurde in-
    zwischen weder ein Controller angemeldet, noch ein Controller invalidiert,
    dann wird kein Wert weitergeleitet. Dadurch wird z.B. Flackern in
    ListBoxen vermieden.
*/

{
//    if ( pDispatch )
//        return;
    SetState_Impl( eState, pState, bMaybeDirty );
}

//--------------------------------------------------------------------

void SfxStateCache::SetVisibleState( BOOL bShow )
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
            pState = new SfxVisibilityItem( nId, FALSE );
            bDeleteItem = sal_True;
        }

        // Controller updaten
        if ( !pDispatch && pController )
        {
            for ( SfxControllerItem *pCtrl = pController;
                    pCtrl;
                    pCtrl = pCtrl->GetItemLink() )
                pCtrl->StateChanged( nId, eState, pState );
        }

        if ( pInternalController )
            pInternalController->StateChanged( nId, eState, pState );

        if ( !bDeleteItem )
            delete pState;
    }
}

//--------------------------------------------------------------------

void SfxStateCache::SetState_Impl
(
    SfxItemState        eState,     // <SfxItemState> von 'pState'
    const SfxPoolItem*  pState,     // Status des Slots, ggf. 0 oder -1
    BOOL bMaybeDirty
)
{
    (void)bMaybeDirty; //unused
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxStateCache, 0);

    // wenn zwischen Enter- und LeaveRegistrations ein hartes Update kommt
    // k"onnen zwischenzeitlich auch Cached ohne Controller exisitieren
    if ( !pController && !pInternalController )
        return;

    DBG_ASSERT( bMaybeDirty || !bSlotDirty, "setting state of dirty message" );
//  DBG_ASSERT( bCtrlDirty || ( aSlotServ.GetSlot() && aSlotServ.GetSlot()->IsMode(SFX_SLOT_VOLATILE) ), ! Discussed with MBA
//              "setting state of non dirty controller" );
    DBG_ASSERT( SfxControllerItem::GetItemState(pState) == eState, "invalid SfxItemState" );
    DBG_PROFSTART(SfxStateCacheSetState);

    // m"ussen die Controller "uberhaupt benachrichtigt werden?
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
        // Controller updaten
        if ( !pDispatch && pController )
        {
            for ( SfxControllerItem *pCtrl = pController;
                pCtrl;
                pCtrl = pCtrl->GetItemLink() )
                pCtrl->StateChanged( nId, eState, pState );
        }

        if ( pInternalController )
            ((SfxDispatchController_Impl *)pInternalController)->StateChanged( nId, eState, pState, &aSlotServ );

        // neuen Wert merken
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

// alten Status in allen Controllern nochmal setzen

void SfxStateCache::SetCachedState( BOOL bAlways )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxStateCache, 0);
    DBG_ASSERT(pController==NULL||pController->GetId()==nId, "Cache mit falschem ControllerItem" );
    DBG_PROFSTART(SfxStateCacheSetState);

    // nur updaten wenn cached item vorhanden und auch verarbeitbar
    // (Wenn der State gesendet wird, mu\s sichergestellt sein, da\s ein
    // Slotserver vorhanden ist, s. SfxControllerItem::GetCoreMetric() )
    if ( bAlways || ( !bItemDirty && !bSlotDirty ) )
    {
        // Controller updaten
        if ( !pDispatch && pController )
        {
            for ( SfxControllerItem *pCtrl = pController;
                pCtrl;
                pCtrl = pCtrl->GetItemLink() )
                pCtrl->StateChanged( nId, eLastState, pLastItem );
        }

        if ( pInternalController )
            ((SfxDispatchController_Impl *)pInternalController)->StateChanged( nId, eLastState, pLastItem, &aSlotServ );

        // Controller sind jetzt ok
        bCtrlDirty = sal_True;
    }

    DBG_PROFSTOP(SfxStateCacheSetState);
}


//--------------------------------------------------------------------

// FloatingWindows in allen Controls mit dieser Id zerstoeren

void SfxStateCache::DeleteFloatingWindows()
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxStateCache, 0);

    SfxControllerItem *pNextCtrl=0;
    for ( SfxControllerItem *pCtrl=pController; pCtrl; pCtrl=pNextCtrl )
    {
        DBG_TRACE((ByteString("pCtrl: ").Append(ByteString::CreateFromInt64((sal_uIntPtr)pCtrl))).GetBuffer());
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
