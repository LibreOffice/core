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
#ifndef GCC
#endif

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
    // echter Slot? (also kein Separator etc.)
    if ( !nId )
        return;

    // ist die Id "uberhaupt in 'nType' konfigurierbar?
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
    DBG_ASSERT(pBindings || pBindinx, "Keine Bindings");

    if ( IsBound() ) {
        DBG_ASSERT(pBindings, "Keine Bindings");
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
    DBG_ASSERT(pBindings || pBindinx, "Keine Bindings");

    if ( IsBound() ) {
        DBG_ASSERT(pBindings, "Keine Bindings");
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

/*  [Beschreibung]

    "ost die Verbindung dieses SfxControllerItems mit der SfxBindings-Instanz,
    an der es zur Zeit gebunden ist. Ab diesem Zeitpunkt erh"alt es keine
    Statusbenachrichtigungen (<SfxControllerItem::StateChented()>) mehr.


    [Querverweise]

    <SfxControllerItem::ReBind()>
    <SfxControllerItem::ClearCache()>
*/
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings, "Keine Bindings");
    DBG_ASSERT( IsBound(), "unbindings unbound SfxControllerItem" );

    pBindings->Release(*this);
    pNext = this;
}

//====================================================================

void SfxControllerItem::ReBind()

/*  [Beschreibung]

    Binded dieses SfxControllerItem wieder an die SfxBindings-Instanz,
    an der es zuletzt gebunden war. Ab diesem Zeitpunkt erh"alt es wieder
    Statusbenachrichtigungen (<SfxControllerItem::StateChented()>).


    [Querverweise]

    <SfxControllerItem::UnBind()>
    <SfxControllerItem::ClearCache()>
*/

{
    DBG_MEMTEST();
DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings, "Keine Bindings");
    DBG_ASSERT( !IsBound(), "bindings rebound SfxControllerItem" );

    pBindings->Register(*this);
}

//====================================================================

void SfxControllerItem::UpdateSlot()

/*  [Beschreibung]

    Holt den Status 'hart' neu.

    [Querverweise]

    <SfxControllerItem::ClearCache()>
*/

{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings, "Keine Bindings");

    pBindings->Update( GetId() );
}

//--------------------------------------------------------------------

void SfxControllerItem::ClearCache()

/*  [Beschreibung]

    "oscht den Status-Cache f"ur dieses SfxControllerItem. D.h. beim
    n"achsten Status-Update wird das <SfxPoolItem> auf jeden Fall geschickt,
    auch wenn zuvor dasselbe geschickt wurde. Dies wird ben"otigt, wenn
    ein Controller umgeschaltet werden kann und sich diesen Status
    selbst merkt.


    [Beispiel]

    Der Kombi-Controller f"ur das Einstellen des Fl"achentyps und der
    konkreten Auspr"agung (Farbe blau oder Schraffur X) kann im Typ
    umgestellt werden, wird jedoch dann bei der n"achsten Selektion
    wieder benachrichtigt, auch wenn es dieselben Daten sind.


    [Querverweise]

    <SfxControllerItem::UnBind()>
    <SfxControllerItem::ReBind()>
*/


{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT(pBindings, "Keine Bindings");

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

// creates a atomic item for a controller  without registration

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
    sal_uInt16              ,   // <SID> des ausl"osenden Slot
    SfxItemState        ,   // <SfxItemState> von 'pState'
    const SfxPoolItem*      // Slot-Status, ggf. 0 oder IsInvalidItem()
)

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx gerufen, um <SfxControllerItem>s
    dar"uber zu benachrichtigen, da\s sich der Status des Slots 'nSID'
    ge"andert hat. Der neue Wert sowie der von diesem Wert ermittelte
    Status wird als 'pState' bzw. 'eState' mitgegeben.

    Der Status eines Slots kann sich "andern, wenn z.B. das MDI-Fenster
    gewechselt wird oder der Slot explizit mit <SfxBindings::Invalidate()>
    invalidiert wurde.

    Achtung! Die Methode wird nicht gerufen, wenn der Slot ung"ultig wurde,
    danach jedoch wieder denselben Wert angenommen hat.

    Diese Basisklasse braucht nicht gerufen zu werden, weitere Zwischenstufen
    jedoch (z.B. <SfxToolboxControl>) sollten gerufen werden.
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
    sal_uInt16              nSID,       // <SID> des ausl"osenden Slot
    SfxItemState        eState,     // <SfxItemState> von 'pState'
    const SfxPoolItem*  pState      // Slot-Status, ggf. 0 oder IsInvalidItem()
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
    const SfxPoolItem* pState   /*  Pointer auf das <SfxPoolItem>, dessen
                                    Status erfragt werden soll. */
)

/*  [Beschreibung]

    Statische Methode zum Ermitteln des Status des SfxPoolItem-Pointers,
    in der Methode <SfxControllerItem::StateChanged(const SfxPoolItem*)>
    zu verwenden.

    [R"uckgabewert]

    SfxItemState        SFX_ITEM_UNKNOWN
                        Enabled, aber keine weitere Statusinformation
                        verf"ugbar. Typisch f"ur <Slot>s, die allenfalls
                        zeitweise disabled sind, aber ihre Darstellung sonst
                        nicht "andern.

                        SFX_ITEM_DISABLED
                        Disabled und keine weiter Statusinformation
                        verf"ugbar. Alle anderen ggf. angezeigten Werte sollten
                        auf den Default zur"uckgesetzt werden.

                        SFX_ITEM_DONTCARE
                        Enabled aber es waren nur uneindeutige Werte
                        verf"ugbar (also keine, die abgefragt werden k"onnen).

                        SFX_ITEM_AVAILABLE
                        Enabled und mit verf"ugbarem Wert, der von 'pState'
                        erfragbar ist. Der Typ ist dabei im gesamten
                        Programm eindeutig und durch den Slot festgelegt.
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

/*  [Beschreibung]

    Holt vom zust"andigen Pool die Ma\seinheit ab, in der das Status-Item
    vorliegt.
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


