/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modctrl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:45:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#define _SVX_MODCTRL_CXX

#include "dialogs.hrc"

#include "modctrl.hxx"
#include "dialmgr.hxx"

SFX_IMPL_STATUSBAR_CONTROL(SvxModifyControl, SfxBoolItem);

// class SvxModifyControl ------------------------------------------------

SvxModifyControl::SvxModifyControl( USHORT nSlotId,
                                    USHORT nId,
                                    StatusBar& rStb ) :

    SfxStatusBarControl( nSlotId, nId, rStb ),
    bState( TRUE )
{
}

// -----------------------------------------------------------------------

void SvxModifyControl::StateChanged( USHORT nSID, SfxItemState eState,
                                     const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( pState->ISA( SfxBoolItem ), "invalid item type" );
        SfxBoolItem* pItem = (SfxBoolItem*)pState;
        bState = pItem->GetValue();
        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxModifyControl::Paint( const UserDrawEvent& rUsrEvt )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxModifyControl::DrawItemText_Impl()
{
    String sMode;

    if ( bState )
        sMode = '*';
    GetStatusBar().SetItemText( GetId(), sMode );
}

ULONG SvxModifyControl::GetDefItemWidth(const StatusBar& rStb)
{
    return rStb.GetTextWidth(String::CreateFromAscii("XX"));
}


