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
#include <sfx2/app.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/gen.hxx>
#include <svl/intitem.hxx>
#include <sot/exchange.hxx>
#include <svl/eitem.hxx>
#include <vcl/toolbox.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/clipfmtitem.hxx>

#include <svtools/insdlg.hxx>
#include <svx/svxids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

/////////////////////////////////////////////////////////////////

SFX_IMPL_TOOLBOX_CONTROL( SvxClipBoardControl, SfxVoidItem /*SfxUInt16Item*/ );


SvxClipBoardControl::SvxClipBoardControl(
        sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pClipboardFmtItem( 0 ),
     pPopup     (0),
    nItemId     (nId),
    bDisabled( sal_False )
{
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ClipboardFormatItems" )));
    ToolBox& rBox = GetToolBox();
    rBox.SetItemBits( nId, TIB_DROPDOWN | rBox.GetItemBits( nId ) );
    rBox.Invalidate();
}


SvxClipBoardControl::~SvxClipBoardControl()
{
    DelPopup();
    delete pClipboardFmtItem;
}


SfxPopupWindow* SvxClipBoardControl::CreatePopupWindow()
{
    const SvxClipboardFmtItem* pFmtItem = PTR_CAST( SvxClipboardFmtItem, pClipboardFmtItem );
    if ( pFmtItem )
    {
        if (pPopup)
            pPopup->Clear();
        else
            pPopup = new PopupMenu;

        sal_uInt16 nCount = pFmtItem->Count();
        for (sal_uInt16 i = 0;  i < nCount;  ++i)
        {
            sal_uIntPtr nFmtID =  pFmtItem->GetClipbrdFormatId( i );
            String aFmtStr( pFmtItem->GetClipbrdFormatName( i ) );
            if (!aFmtStr.Len())
              aFmtStr = SvPasteObjectHelper::GetSotFormatUIName( nFmtID );
            pPopup->InsertItem( (sal_uInt16)nFmtID, aFmtStr );
        }

        ToolBox& rBox = GetToolBox();
        sal_uInt16 nId = GetId();
        rBox.SetItemDown( nId, sal_True );

        pPopup->Execute( &rBox, rBox.GetItemRect( nId ),
            (rBox.GetAlign() == WINDOWALIGN_TOP || rBox.GetAlign() == WINDOWALIGN_BOTTOM) ?
                POPUPMENU_EXECUTE_DOWN : POPUPMENU_EXECUTE_RIGHT );

        rBox.SetItemDown( nId, sal_False );

        SfxUInt32Item aItem( SID_CLIPBOARD_FORMAT_ITEMS, pPopup->GetCurItemId() );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SelectedFormat" ));
        aItem.QueryValue( a );
        aArgs[0].Value = a;
        Dispatch( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ClipboardFormatItems" )),
                  aArgs );
    }

    GetToolBox().EndSelection();
    DelPopup();
    return 0;
}


SfxPopupWindowType SvxClipBoardControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}


void SvxClipBoardControl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SID_CLIPBOARD_FORMAT_ITEMS == nSID )
    {
        DELETEZ( pClipboardFmtItem );
        if ( eState >= SFX_ITEM_AVAILABLE )
        {
            pClipboardFmtItem = pState->Clone();
            GetToolBox().SetItemBits( GetId(), GetToolBox().GetItemBits( GetId() ) | TIB_DROPDOWN );
        }
        else if ( !bDisabled )
            GetToolBox().SetItemBits( GetId(), GetToolBox().GetItemBits( GetId() ) & ~TIB_DROPDOWN );
        GetToolBox().Invalidate( GetToolBox().GetItemRect( GetId() ) );
    }
    else
    {
        // enable the item as a whole
        bDisabled = (GetItemState(pState) == SFX_ITEM_DISABLED);
        GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
    }
}


void SvxClipBoardControl::DelPopup()
{
    if(pPopup)
    {
        delete pPopup;
        pPopup = 0;
    }
}


/////////////////////////////////////////////////////////////////

