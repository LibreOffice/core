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

#include <string>

#include <tools/shl.hxx>
#include <svl/intitem.hxx>
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

ScTbxInsertCtrl::ScTbxInsertCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx  ) :
        SfxToolBoxControl( nSlotId, nId, rTbx ),
        nLastSlotId(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

ScTbxInsertCtrl::~ScTbxInsertCtrl()
{
}

void ScTbxInsertCtrl::StateChanged( sal_uInt16 /* nSID */, SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {

        const SfxUInt16Item* pItem = PTR_CAST( SfxUInt16Item, pState );
        if(pItem)
        {
            nLastSlotId = pItem->GetValue();
            sal_uInt16 nImageId = nLastSlotId ? nLastSlotId : GetSlotId();
            OUString aSlotURL( "slot:" );
            aSlotURL += OUString::number( nImageId);
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
    sal_uInt16 nSlotId = GetSlotId();
    if (nSlotId == SID_TBXCTL_INSERT)
    {
        OUString aInsertBarResStr( "private:resource/toolbar/insertbar" );
        createAndPositionSubToolBar( aInsertBarResStr );
    }
    else if (nSlotId == SID_TBXCTL_INSCELLS)
    {
        OUString aInsertCellsBarResStr( "private:resource/toolbar/insertcellsbar" );
        createAndPositionSubToolBar( aInsertCellsBarResStr );
    }
    else
    {
        OUString aInsertObjectBarResStr( "private:resource/toolbar/insertobjectbar" );
        createAndPositionSubToolBar( aInsertObjectBarResStr );
    }
    return NULL;
}

SfxPopupWindowType ScTbxInsertCtrl::GetPopupWindowType() const
{
    return nLastSlotId ? SFX_POPUPWINDOW_ONTIMEOUT : SFX_POPUPWINDOW_ONCLICK;
}

void ScTbxInsertCtrl::Select( sal_Bool /* bMod1 */ )
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
