/*************************************************************************
 *
 *  $RCSfile: statcach.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HPP_
#include <com/sun/star/frame/FrameActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/stritem.hxx>
#include <unotools/processfactory.hxx>

#pragma hdrstop

#include "statcach.hxx"
#include "msg.hxx"
#include "ctrlitem.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"
#include "sfxuno.hxx"
#include "unoctitm.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

//====================================================================

DBG_NAME(SfxStateCache);
DBG_NAME(SfxStateCacheSetState);

SFX_IMPL_XINTERFACE_2( BindDispatch_Impl, OWeakObject, ::com::sun::star::frame::XStatusListener, ::com::sun::star::lang::XEventListener )
SFX_IMPL_XTYPEPROVIDER_2( BindDispatch_Impl, ::com::sun::star::frame::XStatusListener, ::com::sun::star::lang::XEventListener )

//-----------------------------------------------------------------------------
BindDispatch_Impl::BindDispatch_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > & rDisp, const ::com::sun::star::util::URL& rURL, SfxStateCache *pStateCache )
    : xDisp( rDisp )
    , aURL( rURL )
    , pCache( pStateCache )
{
    aStatus.IsEnabled = sal_True;
}

void SAL_CALL BindDispatch_Impl::disposing( const ::com::sun::star::lang::EventObject& rEvent ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( xDisp.is() )
    {
        xDisp->removeStatusListener( (::com::sun::star::frame::XStatusListener*) this, ::com::sun::star::util::URL() );
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
        pCache->Invalidate( sal_False );
        if ( !aStatus.IsEnabled )
            pCache->SetState_Impl( SFX_ITEM_DISABLED, NULL );
        else
        {
            sal_uInt16 nId = pCache->GetId();
            SfxItemState eState = SFX_ITEM_AVAILABLE;
            SfxPoolItem *pItem=NULL;
            ::com::sun::star::uno::Any aAny = aStatus.State;
            ::com::sun::star::uno::Type pType = aAny.getValueType();

            if ( pType == ::getBooleanCppuType() )
            {
                sal_Bool bTemp ;
                aAny >>= bTemp ;
                pItem = new SfxBoolItem( nId, bTemp );
            }
            else if ( pType == ::getCppuType((const sal_uInt16*)0) )
            {
                sal_uInt16 nTemp ;
                aAny >>= nTemp ;
                pItem = new SfxUInt16Item( nId, nTemp );
            }
            else if ( pType == ::getCppuType((const sal_uInt32*)0) )
            {
                sal_uInt32 nTemp ;
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
                pItem = new SfxVoidItem( nId );
            pCache->SetState_Impl( eState, pItem );
            delete pItem;
        }
    }
}

void BindDispatch_Impl::Release()
{
    if ( xDisp.is() )
    {
        xDisp->removeStatusListener( (::com::sun::star::frame::XStatusListener*) this, ::com::sun::star::util::URL() );
        xDisp = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();
    }

    pCache = NULL;
    release();
}

const ::com::sun::star::frame::FeatureStateEvent& BindDispatch_Impl::GetStatus() const
{
    return aStatus;
}

void BindDispatch_Impl::Dispatch( sal_Bool bForceSynchron )
{
    if ( xDisp.is() && aStatus.IsEnabled )
        if ( bForceSynchron )
        {
            ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aProps(1);
            aProps.getArray()[0].Name = DEFINE_CONST_UNICODE("SynchronMode");
            aProps.getArray()[0].Value <<= sal_True ;
            xDisp->dispatch( aURL, aProps );
        }
        else
            xDisp->dispatch( aURL, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >() );
}

//--------------------------------------------------------------------

/*  Dieser Konstruktor fuer einen ungueltigen Cache, der sich also
    bei der ersten Anfrage zun"achst updated.
 */

SfxStateCache::SfxStateCache( sal_uInt16 nFuncId ):
    nId(nFuncId),
    pController(0),
    pLastItem( 0 ),
    eLastState( 0 ),
    pDispatch( 0 )
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
    DBG_ASSERT( pController == 0, "es sind noch Controller angemeldet" );
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
        if ( xProv.is() )
        {
            DBG_ASSERT( !pDispatch, "Altes Dispatch nicht entfernt!" );

//(mba)/compview
            const SfxSlot* pSlot = 0;//SfxComponentViewShell::GetUnoSlot( nId );
            ::com::sun::star::util::URL aURL;
            String aName( pSlot ? String::CreateFromAscii(pSlot->GetUnoName()) : String() );
            String aCmd;
            if ( aName.Len() )
            {
                aCmd = DEFINE_CONST_UNICODE(".uno:");
                aCmd += aName;
            }
            else
            {
                aCmd = DEFINE_CONST_UNICODE("slot:");
                aCmd += String::CreateFromInt32( nId );
            }

            aURL.Complete = aCmd;
            Reference < XURLTransformer > xTrans( ::utl::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
            xTrans->parseStrict( aURL );
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
            if ( xDisp.is() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel( xDisp, ::com::sun::star::uno::UNO_QUERY );
                SfxOfficeDispatch* pDisp = NULL;
                if ( xTunnel.is() )
                {
                    sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                    pDisp = (SfxOfficeDispatch*)(nImplementation);
                }

                if ( pDisp )
                {
                    // The intercepting object is a SFX component
                    // Get the shell that is the slotserver of this component
                    SfxDispatcher *pDispatcher = pDisp->GetDispatcher_Impl();
                    pDispatcher->_FindServer( nId, aSlotServ, sal_False );
                    SfxShell* pShell = pDispatcher->GetShell( aSlotServ.GetShellLevel() );

                    // Check if this shell is active on the active dispatcher
                    sal_uInt16 nLevel = rDispat.GetShellLevel( *pShell );
                    if ( nLevel != USHRT_MAX )
                    {
                        // so we can use this shell direct without StarONE connection
                        aSlotServ.SetShellLevel( nLevel );
                        bSlotDirty = sal_False;
                        bCtrlDirty = sal_True;

                        //MI: wozu das? bItemDirty = sal_True;
                        return aSlotServ.GetSlot()? &aSlotServ: 0;
                    }
                    else
                    {
                        pDispatch = new BindDispatch_Impl( xDisp, aURL, this );
                        pDispatch->acquire();

                        // flags must be set before adding StatusListener because the dispatch object will set the state
                        bSlotDirty = sal_False;
                        bCtrlDirty = sal_True;
                        xDisp->addStatusListener( pDispatch, aURL );
                        aSlotServ.SetSlot(0);
                        return NULL;
                    }
                }
                else
                {
                    pDispatch = new BindDispatch_Impl( xDisp, aURL, this );
                    pDispatch->acquire();

                    // flags must be set before adding StatusListener because the dispatch object will set the state
                    bSlotDirty = sal_False;
                    bCtrlDirty = sal_True;
                    xDisp->addStatusListener( pDispatch, aURL );
                    aSlotServ.SetSlot(0);
                    return NULL;
                }
            }
        }

//      if ( !rDispat._TryIntercept_Impl( nId, aSlotServ, sal_False ) )
        rDispat._FindServer(nId, aSlotServ, sal_False);
        bSlotDirty = sal_False;
        bCtrlDirty = sal_True;
        //MI: wozu das? bItemDirty = sal_True;
    }

    return aSlotServ.GetSlot()? &aSlotServ: 0;
}


//--------------------------------------------------------------------

// Status setzen in allen Controllern

void SfxStateCache::SetState
(
    SfxItemState        eState,     // <SfxItemState> von 'pState'
    const SfxPoolItem*  pState      // Status des Slots, ggf. 0 oder -1
)

/*  [Beschreibung]

    Diese Methode verteilt die Status auf alle an dieser SID gebundenen
    <SfxControllerItem>s. Ist der Wert derselbe wie zuvor und wurde in-
    zwischen weder ein Controller angemeldet, noch ein Controller invalidiert,
    dann wird kein Wert weitergeleitet. Dadurch wird z.B. Flackern in
    ListBoxen vermieden.
*/

{
    if ( pDispatch )
        return;
    SetState_Impl( eState, pState );
}


void SfxStateCache::SetState_Impl
(
    SfxItemState        eState,     // <SfxItemState> von 'pState'
    const SfxPoolItem*  pState      // Status des Slots, ggf. 0 oder -1
)
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxStateCache, 0);

    // wenn zwischen Enter- und LeaveRegistrations ein hartes Update kommt
    // k"onnen zwischenzeitlich auch Cached ohne Controller exisitieren
    if ( !pController )
        return;

    DBG_ASSERT( pController->GetId()==nId, "Cache mit falschem ControllerItem" );
    DBG_ASSERT( !bSlotDirty, "setting state of dirty message" );
    DBG_ASSERT( bCtrlDirty ||
                ( aSlotServ.GetSlot() && aSlotServ.GetSlot()->IsMode(SFX_SLOT_VOLATILE) ),
                "setting state of non dirty controller" );
    DBG_ASSERT( SfxControllerItem::GetItemState(pState) == eState,
                "invalid SfxItemState" );
    DBG_PROFSTART(SfxStateCacheSetState);

    // m"ussen die Controller "uberhaupt benachrichtigt werden?
    FASTBOOL bNotify = bItemDirty;
    if ( !bItemDirty )
    {
        FASTBOOL bBothAvailable = pLastItem && pState &&
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
        for ( SfxControllerItem *pCtrl = pController;
              pCtrl;
              pCtrl = pCtrl->GetItemLink() )
            pCtrl->StateChanged( nId, eState, pState );

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

void SfxStateCache::SetCachedState()
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxStateCache, 0);
    DBG_ASSERT( pController, "Cache ohne ControllerItem" );
    DBG_ASSERT( pController->GetId()==nId, "Cache mit falschem ControllerItem" );
    DBG_PROFSTART(SfxStateCacheSetState);

    // nur updaten wenn cached item vorhanden und auch verarbeitbar
    // (Wenn der State gesendet wird, mu\s sichergestellt sein, da\s ein
    // Slotserver vorhanden ist, s. SfxControllerItem::GetCoreMetric() )
    if ( !bItemDirty && !bSlotDirty )
    {
        // Controller updaten
        for ( SfxControllerItem *pCtrl = pController;
              pCtrl;
              pCtrl = pCtrl->GetItemLink() )
            pCtrl->StateChanged( nId, eLastState, pLastItem );

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
        DBG_TRACE((ByteString("pCtrl: ").Append(ByteString::CreateFromInt32((sal_uInt32)pCtrl))).GetBuffer());
        pNextCtrl = pCtrl->GetItemLink();
        pCtrl->DeleteFloatingWindow();
    }
}

sal_Bool SfxStateCache::UpdateDispatch( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & xProv )
{
    if ( bSlotDirty && xProv.is() )
    {
        DBG_ASSERT( !pDispatch, "Altes Dispatch nicht entfernt!" );

//(mba)/compview
        const SfxSlot* pSlot = 0;//SfxComponentViewShell::GetUnoSlot( nId );
        ::com::sun::star::util::URL aURL;
        String aName( pSlot ? String::CreateFromAscii(pSlot->GetUnoName()) : String() );
        String aCmd;
        if ( aName.Len() )
        {
            aCmd = DEFINE_CONST_UNICODE(".uno:");
            aCmd += aName;
        }
        else
        {
            aCmd = DEFINE_CONST_UNICODE("slot:");
            aCmd += String::CreateFromInt32( nId );
        }

        aURL.Complete = aCmd;
        Reference < XURLTransformer > xTrans( ::utl::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aURL );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
        if ( xDisp.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel( xDisp, ::com::sun::star::uno::UNO_QUERY );
            SfxOfficeDispatch* pDisp = NULL;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = (SfxOfficeDispatch*)(nImplementation);
            }

            if ( pDisp && !pDisp->IsInterceptDispatch() )
                return sal_False;
            pDispatch = new BindDispatch_Impl( xDisp, aURL, this );
            pDispatch->acquire();
            bSlotDirty = sal_False;
            bCtrlDirty = sal_True;
            xDisp->addStatusListener( pDispatch, aURL );
            aSlotServ.SetSlot(0);
        }
    }

    return ( pDispatch != NULL );
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SfxStateCache::GetDispatch() const
{
    if ( pDispatch )
        return pDispatch->xDisp;
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();
}

void SfxStateCache::Dispatch( sal_Bool bForceSynchron )
{
    if ( pDispatch )
        pDispatch->Dispatch( bForceSynchron );
}


