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
#include <svl/itempool.hxx>

#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msgpool.hxx>
#include "statcach.hxx"
#include <sfx2/viewfrm.hxx>

//====================================================================

DBG_NAME(SfxControllerItem);

//--------------------------------------------------------------------
#ifdef DBG_UTIL

void SfxControllerItem::CheckConfigure_Impl( sal_uIntPtr nType )
{
    // Real Slot? (i.e. no Separator etc.)
    if ( !nId )
        return;

    // is the ID configurable at all in 'nType'?
    const SfxSlot *pSlot = SFX_SLOTPOOL().GetSlot(nId);
    DBG_ASSERTWARNING( pSlot, "SfxControllerItem: binding not existing slot" );
    if ( pSlot && !pSlot->IsMode(nType) )
    {
        DBG_WARNING( "SfxControllerItem: slot without ...Config-flag" );
        DbgOutf( "SfxControllerItem: Config-flag missing at SID %5d",
                 pSlot->GetSlotId() );
    }
}

#endif

//--------------------------------------------------------------------
// returns the next registered SfxControllerItem with the same id

SfxControllerItem* SfxControllerItem::GetItemLink()
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    return pNext == this ? 0 : pNext;
}

//--------------------------------------------------------------------
// returns sal_True if this binding is really bound to a function

sal_Bool SfxControllerItem::IsBound() const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    return pNext != this;
}

//--------------------------------------------------------------------
// returns the associated function-id or 0 if none

// sal_uInt16 SfxControllerItem::GetId() const;

//====================================================================
// registeres with the id at the bindings

void SfxControllerItem::Bind( sal_uInt16 nNewId, SfxBindings *pBindinx )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings || pBindinx, "No Bindings");

    if ( IsBound() ) {
        DBG_ASSERT(pBindings, "No Bindings");
        pBindings->Release(*this);
    }

    nId = nNewId;
    pNext = 0;

    if (pBindinx)
        pBindings = pBindinx;
    pBindings->Register(*this);
}

void SfxControllerItem::BindInternal_Impl( sal_uInt16 nNewId, SfxBindings *pBindinx )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings || pBindinx, "No Bindings");

    if ( IsBound() ) {
        DBG_ASSERT(pBindings, "No Bindings");
        pBindings->Release(*this);
    }

    nId = nNewId;
    pNext = 0;

    if (pBindinx)
        pBindings = pBindinx;
    pBindings->RegisterInternal_Impl(*this);
}


//====================================================================

void SfxControllerItem::UnBind()

/*  [Description]

    Unbinds the connection of this SfxControllerItems with the SfxBindings
    instance with which it to time is bound. From this time on it does not
    receive any status notifications (<SfxControllerItem::StateChented()>)
    anymore.

    [Cross-reference]

    <SfxControllerItem::ReBind()>
    <SfxControllerItem::ClearCache()>
*/
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings, "No Bindings");
    DBG_ASSERT( IsBound(), "unbindings unbound SfxControllerItem" );

    pBindings->Release(*this);
    pNext = this;
}

//====================================================================

void SfxControllerItem::ReBind()

/*  [Description]

    Binds this SfxControllerItem with the SfxBindings instance again,
    with which it was last bound. From this time on it does receive status
    notifications (<SfxControllerItem::StateChented()>) again.

    [Cross-reference]

    <SfxControllerItem::UnBind()>
    <SfxControllerItem::ClearCache()>
*/

{
    DBG_MEMTEST();
DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings, "No Bindings");
    DBG_ASSERT( !IsBound(), "bindings rebound SfxControllerItem" );

    pBindings->Register(*this);
}

//====================================================================

void SfxControllerItem::UpdateSlot()

/*  [Description]

    Get the Status again.

    [Cross-reference]

    <SfxControllerItem::ClearCache()>
*/

{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings, "No Bindings");

    pBindings->Update( GetId() );
}

//--------------------------------------------------------------------

void SfxControllerItem::ClearCache()

/*  [Description]

    Clears the cache status for this SfxControllerItem. That is by the next
    status update is the <SfxPoolItem> sent in any case, even if the same was
    sent before. This is needed if a controller can be switched on and note
    that status themselves.

    [Example]

    The combined controller for adjusting the surface type and the concrete
    expression (blue color, or hatching X) can be changed in type, but is then
    notified of the next selection again, even if it the same data.

    [Cross-reference]

    <SfxControllerItem::UnBind()>
    <SfxControllerItem::ReBind()>
*/


{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings, "No Bindings");

    pBindings->ClearCache_Impl( GetId() );
}

//--------------------------------------------------------------------
// replaces the successor in the list of bindings of the same id

SfxControllerItem* SfxControllerItem::ChangeItemLink( SfxControllerItem* pNewLink )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    SfxControllerItem* pOldLink = pNext;
    pNext = pNewLink;
    return pOldLink == this ? 0 : pOldLink;
}

//--------------------------------------------------------------------
// changes the id of unbound functions (e.g. for sub-menu-ids)

void SfxControllerItem::SetId( sal_uInt16 nItemId )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT( !IsBound(), "changing id of bound binding" );
    nId = nItemId;
}

//--------------------------------------------------------------------

// creates a atomic item for a controller without registration.

SfxControllerItem::SfxControllerItem():
    nId(0),
    pNext(this),
    pBindings(0)
{
    DBG_MEMTEST();
    DBG_CTOR(SfxControllerItem, 0);
}

//--------------------------------------------------------------------
// creates a representation of the function nId and registeres it

SfxControllerItem::SfxControllerItem( sal_uInt16 nID, SfxBindings &rBindings ):
    nId(nID),
    pNext(this),
    pBindings(&rBindings)
{
    DBG_MEMTEST();
    DBG_CTOR(SfxControllerItem, 0);
    Bind(nId, &rBindings);
}

//--------------------------------------------------------------------
// unregisteres the item in the bindings

SfxControllerItem::~SfxControllerItem()
{
    DBG_MEMTEST();
    if ( IsBound() )
        pBindings->Release(*this);
    DBG_DTOR(SfxControllerItem, 0);
}

//--------------------------------------------------------------------

void SfxControllerItem::StateChanged
(
    sal_uInt16,          // <SID> of the triggering slot
    SfxItemState,       // <SfxItemState> of 'pState'
    const SfxPoolItem*  // Slot-Status, NULL or IsInvalidItem()
)

/*  [Description]

    This virtual method is called by the SFx to inform the <SfxControllerItem>s
    is about that state of the slots 'NSID' has changed. The new value and the
    value determined by this status is given as 'pState' or 'eState'.

    The status of a slot may change, for example when the MDI window is
    switched or when the slot was invalidated explicitly with
    <SfxBindings::Invalidate()>.

    Achtung! Die Methode wird nicht gerufen, wenn der Slot ung"ultig wurde,
    danach jedoch wieder denselben Wert angenommen hat.

    Beware! The method is not called when the slot is invalid, however
    has again assumed the same value.

    This base class need not be called, further interim steps however
    (eg <SfxToolboxControl> ) should be called.
*/

{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
}

//--------------------------------------------------------------------

void SfxControllerItem::DeleteFloatingWindow()
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
}

//--------------------------------------------------------------------

void SfxStatusForwarder::StateChanged
(
    sal_uInt16          nSID,    // <SID> of the triggering slot
    SfxItemState        eState,  // <SfxItemState> of 'pState'
    const SfxPoolItem*  pState   // Slot-Status, NULL or IsInvalidItem()
)

{
    pMaster->StateChanged( nSID, eState, pState );
}

//--------------------------------------------------------------------

SfxStatusForwarder::SfxStatusForwarder(
            sal_uInt16              nSlotId,
            SfxControllerItem&  rMaster ):
    SfxControllerItem( nSlotId, rMaster.GetBindings() ),
    pMaster( &rMaster )
{
}

//--------------------------------------------------------------------

SfxItemState SfxControllerItem::GetItemState
(
    const SfxPoolItem* pState   /*  Pointer to  <SfxPoolItem>, which
                                    Status should be queried. */
)

/*  [Description]

    Static method to determine the status of the SfxPoolItem-Pointers, to be
    used in the method <SfxControllerItem::StateChanged(const SfxPoolItem*)>

    [Return value]

    SfxItemState        SFX_ITEM_UNKNOWN
                        Enabled, but no further status information available.
                        Typical for <Slot>s, which anyway are sometimes
                        disabled, but otherwise do not change their appearance.

                        SFX_ITEM_DISABLED
                        Disabled and no further status information available.
                        All other values that may appear should be reset to
                        default.

                        SFX_ITEM_DONTCARE
                        Enabled but there were only ambiguous values available
                        (i.e. non that can be queried).

                        SFX_ITEM_AVAILABLE
                        Enabled and with available values, which are queried
                        by 'pState'. The Type is thus clearly defined in the
                        entire Program and specified through the Slot.
*/

{
    return !pState
                ? SFX_ITEM_DISABLED
                : IsInvalidItem(pState)
                    ? SFX_ITEM_DONTCARE
                    : pState->ISA(SfxVoidItem) && !pState->Which()
                        ? SFX_ITEM_UNKNOWN
                        : SFX_ITEM_AVAILABLE;
}

//--------------------------------------------------------------------

SfxMapUnit SfxControllerItem::GetCoreMetric() const

/*  [Description]

    Gets the measurement unit from the competent pool, in which the Status
    item exist.
*/

{
    SfxStateCache *pCache = pBindings->GetStateCache( nId );
    SfxDispatcher *pDispat = pBindings->GetDispatcher_Impl();

    if ( !pDispat )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        if ( !pViewFrame )
            SfxViewFrame::GetFirst();
        if ( pViewFrame )
            pDispat = pViewFrame->GetDispatcher();
    }

    if ( pDispat && pCache )
    {
        const SfxSlotServer *pServer = pCache->GetSlotServer( *pDispat );
        if ( pServer )
        {
            SfxShell *pSh = pDispat->GetShell( pServer->GetShellLevel() );
            SfxItemPool &rPool = pSh->GetPool();
            sal_uInt16 nWhich = rPool.GetWhich( nId );
            return rPool.GetMetric( nWhich );
        }
    }

    DBG_WARNING( "W1: Can not find ItemPool!" );
    return SFX_MAPUNIT_100TH_MM;
}

//------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize("g",off)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
