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
#include "precompiled_sc.hxx"

// System - Includes -----------------------------------------------------

#include <string>


// INCLUDE ---------------------------------------------------------------

#include <tools/shl.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/imagemgr.hxx>
#include <vcl/toolbox.hxx>

#include "tbinsert.hxx"
#include "tbinsert.hrc"
#include "global.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"

// -----------------------------------------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( ScTbxInsertCtrl, SfxUInt16Item);

//------------------------------------------------------------------
//
//  ToolBox - Controller
//
//------------------------------------------------------------------

ScTbxInsertCtrl::ScTbxInsertCtrl( USHORT nSlotId, USHORT nId, ToolBox& rTbx  ) :
        SfxToolBoxControl( nSlotId, nId, rTbx ),
        nLastSlotId(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

ScTbxInsertCtrl::~ScTbxInsertCtrl()
{
}

void ScTbxInsertCtrl::StateChanged( USHORT /* nSID */, SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {

        const SfxUInt16Item* pItem = PTR_CAST( SfxUInt16Item, pState );
        if(pItem)
        {
            nLastSlotId = pItem->GetValue();
            USHORT nImageId = nLastSlotId ? nLastSlotId : GetSlotId();
            rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
            aSlotURL += rtl::OUString::valueOf( sal_Int32( nImageId ));
            Image aImage = GetImage( m_xFrame,
                                     aSlotURL,
                                     hasBigImages()
                                     );
            GetToolBox().SetItemImage(GetId(), aImage);
        }
    }
}

SfxPopupWindow* ScTbxInsertCtrl::CreatePopupWindow()
{
    USHORT nSlotId = GetSlotId();
    if (nSlotId == SID_TBXCTL_INSERT)
    {
        rtl::OUString aInsertBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertbar" ));
        createAndPositionSubToolBar( aInsertBarResStr );
    }
    else if (nSlotId == SID_TBXCTL_INSCELLS)
    {
        rtl::OUString aInsertCellsBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertcellsbar" ));
        createAndPositionSubToolBar( aInsertCellsBarResStr );
    }
    else
    {
        rtl::OUString aInsertObjectBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertobjectbar" ));
        createAndPositionSubToolBar( aInsertObjectBarResStr );
    }
    return NULL;
}

SfxPopupWindowType ScTbxInsertCtrl::GetPopupWindowType() const
{
    return nLastSlotId ? SFX_POPUPWINDOW_ONTIMEOUT : SFX_POPUPWINDOW_ONCLICK;
}

void ScTbxInsertCtrl::Select( BOOL /* bMod1 */ )
{
    SfxViewShell*   pCurSh( SfxViewShell::Current() );
    SfxDispatcher*  pDispatch( 0 );

    if ( pCurSh )
    {
        SfxViewFrame*   pViewFrame = pCurSh->GetViewFrame();
        if ( pViewFrame )
            pDispatch = pViewFrame->GetDispatcher();
    }

    if ( pDispatch )
        pDispatch->Execute(nLastSlotId);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
