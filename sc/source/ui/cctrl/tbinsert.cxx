/*************************************************************************
 *
 *  $RCSfile: tbinsert.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 12:53:48 $
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

// System - Includes -----------------------------------------------------

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <tools/shl.hxx>
#include <svtools/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/imagemgr.hxx>

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

__EXPORT ScTbxInsertCtrl::~ScTbxInsertCtrl()
{
}

void __EXPORT ScTbxInsertCtrl::StateChanged( USHORT nSID, SfxItemState eState,
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
                                     hasBigImages(),
                                     GetToolBox().GetDisplayBackground().GetColor().IsDark() );
/*
            Image aImage = GetBindings().GetImageManager()->GetImage( nImageId,
                           GetToolBox().GetDisplayBackground().GetColor().IsDark(),
                           SC_MOD() );
*/
            GetToolBox().SetItemImage(GetId(), aImage);
        }
    }
}

SfxPopupWindow* __EXPORT ScTbxInsertCtrl::CreatePopupWindow()
{
    USHORT nWinResId, nTbxResId;
    USHORT nSlotId = GetSlotId();
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
    pWin->StartPopupMode(&GetToolBox(), TRUE);
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

void __EXPORT ScTbxInsertCtrl::Select( BOOL bMod1 )
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

ScTbxInsertPopup::ScTbxInsertPopup( USHORT nId, WindowAlign eNewAlign,
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

    USHORT nLastSlotId = pBox->GetCurItemId();
    SfxUInt16Item aItem( GetId(), nLastSlotId );
    SfxDispatcher* pDisp = GetBindings().GetDispatcher();
    pDisp->Execute( GetId(), SFX_CALLMODE_SYNCHRON, &aItem, 0L );
    pDisp->Execute( nLastSlotId, SFX_CALLMODE_ASYNCHRON );
    return 0;
}

IMPL_LINK(ScTbxInsertPopup, TbxClickHdl, ToolBox*, pBox)
{
    USHORT nLastSlotId = pBox->GetCurItemId();
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


