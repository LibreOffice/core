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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>

#define _SVX_MODCTRL_CXX

#include <svx/dialogs.hrc>

#include "svx/modctrl.hxx"
#include <svx/dialmgr.hxx>

SFX_IMPL_STATUSBAR_CONTROL(SvxModifyControl, SfxBoolItem);

// class SvxModifyControl ------------------------------------------------

SvxModifyControl::SvxModifyControl( sal_uInt16 _nSlotId,
                                    sal_uInt16 _nId,
                                    StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    bState( sal_True )
{
}

// -----------------------------------------------------------------------

void SvxModifyControl::StateChanged( sal_uInt16, SfxItemState eState,
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

void SvxModifyControl::Paint( const UserDrawEvent& )
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

sal_uIntPtr SvxModifyControl::GetDefItemWidth(const StatusBar& rStb)
{
    return rStb.GetTextWidth(String::CreateFromAscii("XX"));
}


