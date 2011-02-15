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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers



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

ScTbxInsertCtrl::ScTbxInsertCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx  ) :
        SfxToolBoxControl( nSlotId, nId, rTbx ),
        nLastSlotId(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

__EXPORT ScTbxInsertCtrl::~ScTbxInsertCtrl()
{
}

void __EXPORT ScTbxInsertCtrl::StateChanged( sal_uInt16 /* nSID */, SfxItemState eState,
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
            rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
            aSlotURL += rtl::OUString::valueOf( sal_Int32( nImageId ));
            Image aImage = GetImage( m_xFrame,
                                     aSlotURL,
                                     hasBigImages(),
                                     GetToolBox().GetSettings().GetStyleSettings().GetHighContrastMode() );
            GetToolBox().SetItemImage(GetId(), aImage);
        }
    }
}

SfxPopupWindow* __EXPORT ScTbxInsertCtrl::CreatePopupWindow()
{
//    sal_uInt16 nWinResId, nTbxResId;
    sal_uInt16 nSlotId = GetSlotId();
    if (nSlotId == SID_TBXCTL_INSERT)
    {
        rtl::OUString aInsertBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertbar" ));
        createAndPositionSubToolBar( aInsertBarResStr );
//      nWinResId = RID_TBXCTL_INSERT;
//      nTbxResId = RID_TOOLBOX_INSERT;
    }
    else if (nSlotId == SID_TBXCTL_INSCELLS)
    {
        rtl::OUString aInsertCellsBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertcellsbar" ));
        createAndPositionSubToolBar( aInsertCellsBarResStr );
//      nWinResId = RID_TBXCTL_INSCELLS;
//      nTbxResId = RID_TOOLBOX_INSCELLS;
    }
    else /* SID_TBXCTL_INSOBJ */
    {
        rtl::OUString aInsertObjectBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertobjectbar" ));
        createAndPositionSubToolBar( aInsertObjectBarResStr );
//      nWinResId = RID_TBXCTL_INSOBJ;
//      nTbxResId = RID_TOOLBOX_INSOBJ;
    }
/*
    WindowAlign eNewAlign = ( GetToolBox().IsHorizontal() ) ? WINDOWALIGN_LEFT : WINDOWALIGN_TOP;
    ScTbxInsertPopup *pWin = new ScTbxInsertPopup( nSlotId, eNewAlign,
                                    ScResId(nWinResId), ScResId(nTbxResId), GetBindings() );
    pWin->StartPopupMode(&GetToolBox(), sal_True);
    pWin->StartSelection();
    pWin->Show();
    return pWin;
*/
    return NULL;
}

SfxPopupWindowType __EXPORT ScTbxInsertCtrl::GetPopupWindowType() const
{
    return nLastSlotId ? SFX_POPUPWINDOW_ONTIMEOUT : SFX_POPUPWINDOW_ONCLICK;
}

void __EXPORT ScTbxInsertCtrl::Select( sal_Bool /* bMod1 */ )
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
/*
//------------------------------------------------------------------
//
//  Popup - Window
//
//------------------------------------------------------------------

ScTbxInsertPopup::ScTbxInsertPopup( sal_uInt16 nId, WindowAlign eNewAlign,
                        const ResId& rRIdWin, const ResId& rRIdTbx,
                        SfxBindings& rBindings ) :
                SfxPopupWindow  ( nId, rRIdWin, rBindings),
                aTbx            ( this, GetBindings(), rRIdTbx ),
                aRIdWinTemp(rRIdWin),
                aRIdTbxTemp(rRIdTbx)
{
    aTbx.UseDefault();
    FreeResource();

    aTbx.GetToolBox().SetAlign( eNewAlign );
    if (eNewAlign == WINDOWALIGN_LEFT || eNewAlign == WINDOWALIGN_RIGHT)
        SetText( EMPTY_STRING );

    Size aSize = aTbx.CalcWindowSizePixel();
    aTbx.SetPosSizePixel( Point(), aSize );
    SetOutputSizePixel( aSize );
    aTbx.GetToolBox().SetSelectHdl( LINK(this, ScTbxInsertPopup, TbxSelectHdl));
    aTbxClickHdl = aTbx.GetToolBox().GetClickHdl();
    aTbx.GetToolBox().SetClickHdl(  LINK(this, ScTbxInsertPopup, TbxClickHdl));
}

ScTbxInsertPopup::~ScTbxInsertPopup()
{
}

SfxPopupWindow* __EXPORT ScTbxInsertPopup::Clone() const
{
    return new ScTbxInsertPopup( GetId(), aTbx.GetToolBox().GetAlign(),
                                    aRIdWinTemp, aRIdTbxTemp,
                                    (SfxBindings&) GetBindings() );
}

void ScTbxInsertPopup::StartSelection()
{
    aTbx.GetToolBox().StartSelection();
}

IMPL_LINK(ScTbxInsertPopup, TbxSelectHdl, ToolBox*, pBox)
{
    EndPopupMode();

    sal_uInt16 nLastSlotId = pBox->GetCurItemId();
    SfxUInt16Item aItem( GetId(), nLastSlotId );
    SfxDispatcher* pDisp = GetBindings().GetDispatcher();
    pDisp->Execute( GetId(), SFX_CALLMODE_SYNCHRON, &aItem, 0L );
    pDisp->Execute( nLastSlotId, SFX_CALLMODE_ASYNCHRON );
    return 0;
}

IMPL_LINK(ScTbxInsertPopup, TbxClickHdl, ToolBox*, pBox)
{
    sal_uInt16 nLastSlotId = pBox->GetCurItemId();
    SfxUInt16Item aItem( GetId(), nLastSlotId );
    GetBindings().GetDispatcher()->Execute( GetId(), SFX_CALLMODE_SYNCHRON, &aItem, 0L );
    if(aTbxClickHdl.IsSet())
        aTbxClickHdl.Call(pBox);
    return 0;
}

void __EXPORT ScTbxInsertPopup::PopupModeEnd()
{
    aTbx.GetToolBox().EndSelection();
    SfxPopupWindow::PopupModeEnd();
}
*/


