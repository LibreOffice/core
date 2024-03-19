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

#include <sal/log.hxx>
#include <svl/itempool.hxx>

#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <statcach.hxx>
#include <sfx2/viewfrm.hxx>

// returns the next registered SfxControllerItem with the same id

SfxControllerItem* SfxControllerItem::GetItemLink()
{
    return pNext == this ? nullptr : pNext;
}


// returns sal_True if this binding is really bound to a function

bool SfxControllerItem::IsBound() const
{
    return pNext != this;
}


// registers with the id at the bindings

void SfxControllerItem::Bind( sal_uInt16 nNewId, SfxBindings *pBindinx )
{
    DBG_ASSERT(pBindings || pBindinx, "No Bindings");

    if ( IsBound() ) {
        DBG_ASSERT(pBindings, "No Bindings");
        pBindings->Release(*this);
    }

    nId = nNewId;
    pNext = nullptr;

    if (pBindinx)
        pBindings = pBindinx;
    pBindings->Register(*this);
}

void SfxControllerItem::BindInternal_Impl( sal_uInt16 nNewId, SfxBindings *pBindinx )
{
    DBG_ASSERT(pBindings || pBindinx, "No Bindings");

    if ( IsBound() ) {
        DBG_ASSERT(pBindings, "No Bindings");
        pBindings->Release(*this);
    }

    nId = nNewId;
    pNext = nullptr;

    if (pBindinx)
        pBindings = pBindinx;
    pBindings->RegisterInternal_Impl(*this);
}


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
    DBG_ASSERT(pBindings, "No Bindings");
    DBG_ASSERT( IsBound(), "unbindings unbound SfxControllerItem" );

    pBindings->Release(*this);
    pNext = this;
}


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
    DBG_ASSERT(pBindings, "No Bindings");
    DBG_ASSERT( !IsBound(), "bindings rebound SfxControllerItem" );

    pBindings->Register(*this);
}


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
    DBG_ASSERT(pBindings, "No Bindings");

    pBindings->ClearCache_Impl( GetId() );
}

// replaces the successor in the list of bindings of the same id
SfxControllerItem* SfxControllerItem::ChangeItemLink( SfxControllerItem* pNewLink )
{
    SfxControllerItem* pOldLink = pNext;
    pNext = pNewLink;
    return pOldLink == this ? nullptr : pOldLink;
}


// changes the id of unbound functions (e.g. for sub-menu-ids)
void SfxControllerItem::SetId( sal_uInt16 nItemId )
{
    DBG_ASSERT( !IsBound(), "changing id of bound binding" );
    nId = nItemId;
}

// creates an atomic item for a controller without registration.
SfxControllerItem::SfxControllerItem()
    : pNext(this)
    , pBindings(nullptr)
    , eFallbackCoreMetric(MapUnit::Map100thMM)
    , nId(0)
{
}

// creates a representation of the function nId and registers it
SfxControllerItem::SfxControllerItem(sal_uInt16 nID, SfxBindings &rBindings)
    : pNext(this)
    , pBindings(&rBindings)
    , eFallbackCoreMetric(MapUnit::Map100thMM)
    , nId(nID)
{
    Bind(nId, &rBindings);
}

// unregisters the item in the bindings
SfxControllerItem::~SfxControllerItem()
{
    dispose();
}

void SfxControllerItem::dispose()
{
    if ( IsBound() )
        UnBind();
}

void SfxControllerItem::StateChangedAtToolBoxControl
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

    Beware! The method is not called when the slot is invalid, however
    has again assumed the same value.

    This base class need not be called, further interim steps however
    (eg <SfxToolboxControl> ) should be called.
*/

{
}

void SfxControllerItem::GetControlState
(
    sal_uInt16,
    boost::property_tree::ptree&
)
{
}

void SfxStatusForwarder::StateChangedAtToolBoxControl
(
    sal_uInt16          nSID,    // <SID> of the triggering slot
    SfxItemState        eState,  // <SfxItemState> of 'pState'
    const SfxPoolItem*  pState   // Slot-Status, NULL or IsInvalidItem()
)

{
    pMaster->StateChangedAtToolBoxControl( nSID, eState, pState );
}


SfxStatusForwarder::SfxStatusForwarder(
            sal_uInt16              nSlotId,
            SfxControllerItem&  rMaster ):
    SfxControllerItem( nSlotId, rMaster.GetBindings() ),
    pMaster( &rMaster )
{
}


SfxItemState SfxControllerItem::GetItemState
(
    const SfxPoolItem* pState   /*  Pointer to  <SfxPoolItem>, which
                                    Status should be queried. */
)

/*  [Description]

    Static method to determine the status of the SfxPoolItem-Pointers, to be
    used in the method <SfxControllerItem::StateChanged(const SfxPoolItem*)>

    [Return value]

    SfxItemState        SfxItemState::UNKNOWN
                        Enabled, but no further status information available.
                        Typical for <Slot>s, which anyway are sometimes
                        disabled, but otherwise do not change their appearance.

                        SfxItemState::DISABLED
                        Disabled and no further status information available.
                        All other values that may appear should be reset to
                        default.

                        SfxItemState::INVALID
                        Enabled but there were only ambiguous values available
                        (i.e. non that can be queried).

                        SfxItemState::DEFAULT
                        Enabled and with available values, which are queried
                        by 'pState'. The Type is thus clearly defined in the
                        entire Program and specified through the Slot.
*/

{
    return !pState
                ? SfxItemState::DISABLED
                : IsInvalidItem(pState)
                    ? SfxItemState::INVALID
                    : IsDisabledItem(pState)
                        ? SfxItemState::UNKNOWN
                        : SfxItemState::DEFAULT;
}


MapUnit SfxControllerItem::GetCoreMetric() const

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
        if ( pViewFrame )
            pDispat = pViewFrame->GetDispatcher();
    }

    if ( pDispat && pCache )
    {
        const SfxSlotServer *pServer = pCache->GetSlotServer( *pDispat );
        if ( pServer )
        {
            if (SfxShell *pSh = pDispat->GetShell( pServer->GetShellLevel() ))
            {
                SfxItemPool &rPool = pSh->GetPool();
                sal_uInt16 nWhich = rPool.GetWhichIDFromSlotID( nId );

                // invalidate slot and its message|slot server as 'global' information
                // about the validated message|slot server is not made available
                pCache->Invalidate( true );

                return rPool.GetMetric( nWhich );
            }
        }
    }

    SAL_INFO( "sfx.control", "W1: Can not find ItemPool!" );
    return eFallbackCoreMetric;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
