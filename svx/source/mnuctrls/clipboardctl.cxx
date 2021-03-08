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

#include <sfx2/tbxctrl.hxx>
#include <svl/intitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/weldutils.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/clipfmtitem.hxx>

#include <svtools/insdlg.hxx>
#include <svx/svxids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


SFX_IMPL_TOOLBOX_CONTROL( SvxClipBoardControl, SfxVoidItem /*SfxUInt16Item*/ );


SvxClipBoardControl::SvxClipBoardControl(
        sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),
    bDisabled( false )
{
    addStatusListener( ".uno:ClipboardFormatItems");
    ToolBox& rBox = GetToolBox();
    rBox.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rBox.GetItemBits( nId ) );
    rBox.Invalidate();
}

SvxClipBoardControl::~SvxClipBoardControl()
{
}

void SvxClipBoardControl::CreatePopupWindow()
{
    const SvxClipboardFormatItem* pFmtItem = dynamic_cast<SvxClipboardFormatItem*>( pClipboardFmtItem.get()  );
    if ( pFmtItem )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "svx/ui/clipboardmenu.ui"));
        std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu("menu"));

        sal_uInt16 nCount = pFmtItem->Count();
        for (sal_uInt16 i = 0;  i < nCount;  ++i)
        {
            SotClipboardFormatId nFmtID =  pFmtItem->GetClipbrdFormatId( i );
            OUString aFmtStr( pFmtItem->GetClipbrdFormatName( i ) );
            if (aFmtStr.isEmpty())
              aFmtStr = SvPasteObjectHelper::GetSotFormatUIName( nFmtID );
            xPopup->append(OUString::number(static_cast<sal_uInt32>(nFmtID)), aFmtStr);
        }

        ToolBox& rBox = GetToolBox();
        ToolBoxItemId nId = GetId();
        rBox.SetItemDown( nId, true );

        ::tools::Rectangle aRect(rBox.GetItemRect(nId));
        weld::Window* pParent = weld::GetPopupParent(rBox, aRect);
        OString sResult = xPopup->popup_at_rect(pParent, aRect);

        rBox.SetItemDown( nId, false );

        SfxUInt32Item aItem(SID_CLIPBOARD_FORMAT_ITEMS, sResult.toUInt32());

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = "SelectedFormat";
        aItem.QueryValue( a );
        aArgs[0].Value = a;
        Dispatch( ".uno:ClipboardFormatItems",
                  aArgs );
    }

    GetToolBox().EndSelection();
}

void SvxClipBoardControl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SID_CLIPBOARD_FORMAT_ITEMS == nSID )
    {
        pClipboardFmtItem.reset();
        if ( eState >= SfxItemState::DEFAULT )
        {
            pClipboardFmtItem.reset( pState->Clone() );
            GetToolBox().SetItemBits( GetId(), GetToolBox().GetItemBits( GetId() ) | ToolBoxItemBits::DROPDOWN );
        }
        else if ( !bDisabled )
            GetToolBox().SetItemBits( GetId(), GetToolBox().GetItemBits( GetId() ) & ~ToolBoxItemBits::DROPDOWN );
        GetToolBox().Invalidate( GetToolBox().GetItemRect( GetId() ) );
    }
    else
    {
        // enable the item as a whole
        bDisabled = (GetItemState(pState) == SfxItemState::DISABLED);
        GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SfxItemState::DISABLED) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
