/*************************************************************************
 *
 *  $RCSfile: ctrlitem.cxx,v $
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

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#pragma hdrstop

#include "ctrlitem.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "msgpool.hxx"
#include "statcach.hxx"

//====================================================================

DBG_NAME(SfxControllerItem);

//--------------------------------------------------------------------
#ifdef DBG_UTIL

void SfxControllerItem::CheckConfigure_Impl( ULONG nType )
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

// returns TRUE if this binding is really bound to a function

BOOL SfxControllerItem::IsBound() const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    return pNext != this;
}

//--------------------------------------------------------------------

// returns the associated function-id or 0 if none

// USHORT SfxControllerItem::GetId() const;

//====================================================================

// registeres with the id at the bindings

void SfxControllerItem::Bind( USHORT nNewId, SfxBindings *pBindinx )
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

void SfxControllerItem::SetId( USHORT nItemId )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxControllerItem, 0);
    DBG_ASSERT( !IsBound(), "changing id of bound binding" );
    nId = nItemId;
}

//--------------------------------------------------------------------

SvStream& operator<<( SvStream& rStream, const SfxControllerItem& rFunc )
{
    DBG_MEMTEST();
    SFX_SLOTPOOL().StoreId( rStream, rFunc.nId );
    return rStream;
}

//--------------------------------------------------------------------

SvStream& operator>>( SvStream& rStream, SfxControllerItem& rFunc )
{
    DBG_MEMTEST();
    USHORT nId;
    SFX_SLOTPOOL().LoadId( rStream, nId );
    rFunc.Bind(nId);
    return rStream;
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

SfxControllerItem::SfxControllerItem( USHORT nId, SfxBindings &rBindings ):
    nId(nId),
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
    USHORT              nSID,       // <SID> des ausl"osenden Slot
    SfxItemState        eState,     // <SfxItemState> von 'pState'
    const SfxPoolItem*  pState      // Slot-Status, ggf. 0 oder IsInvalidItem()
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
    USHORT              nSID,       // <SID> des ausl"osenden Slot
    SfxItemState        eState,     // <SfxItemState> von 'pState'
    const SfxPoolItem*  pState      // Slot-Status, ggf. 0 oder IsInvalidItem()
)

{
    pMaster->StateChanged( nSID, eState, pState );
}

//--------------------------------------------------------------------

SfxStatusForwarder::SfxStatusForwarder(
            USHORT              nSlotId,
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
    const SfxSlotServer *pServer = pCache->GetSlotServer( *pDispat );
    SfxShell *pSh = pDispat->GetShell( pServer->GetShellLevel() );
    SfxItemPool &rPool = pSh->GetPool();
    USHORT nWhich = rPool.GetWhich( nId );
    return rPool.GetMetric( nWhich );
}

//------------------------------------------------------------------------

#ifdef WNT
#pragma optimize("g",off)
#endif


