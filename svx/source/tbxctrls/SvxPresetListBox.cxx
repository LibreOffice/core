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

#include <svx/SvxPresetListBox.hxx>
#include <svx/xtable.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/image.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <vcl/menu.hxx>
#include <vcl/popupmenuwindow.hxx>

SvxPresetListBox::SvxPresetListBox(vcl::Window* pParent, WinBits nWinStyle)
    : ValueSet(pParent, nWinStyle),
      nColCount(3),
      aIconSize( Size(60,64) )
{
    SetEdgeBlending(true);
    SetExtraSpacing(4);
}


VCL_BUILDER_FACTORY_CONSTRUCTOR(SvxPresetListBox, WB_ITEMBORDER| WB_3DLOOK|WB_OWNERDRAWDECORATION|WB_TABSTOP)

void SvxPresetListBox::Resize()
{
    DrawLayout();
    WinBits aWinBits(GetStyle());
    aWinBits |= WB_VSCROLL;
    SetStyle(aWinBits);
    ValueSet::Resize();
}

void SvxPresetListBox::Command( const CommandEvent& rEvent )
{
    switch(rEvent.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            const sal_uInt16 nIndex = GetSelectItemId();
            if(nIndex > 0)
            {
                Point aPos(rEvent.GetMousePosPixel());
                VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/presetmenu.ui", "");
                VclPtr<PopupMenu> pMenu(aBuilder.get_menu("menu"));
                FloatingWindow* pMenuWindow = dynamic_cast<FloatingWindow*>(pMenu->GetWindow());
                if(pMenuWindow != nullptr)
                {
                    pMenuWindow->SetPopupModeFlags(
                    pMenuWindow->GetPopupModeFlags() | FloatWinPopupFlags::NoMouseUpClose);
                }
                pMenu->SetSelectHdl( LINK(this, SvxPresetListBox, OnMenuItemSelected) );
                pMenu->Execute(this,tools::Rectangle(aPos,Size(1,1)),PopupMenuFlags::ExecuteDown);
            }
        }
        break;
        default:
            ValueSet::Command( rEvent );
            break;
    }
}

void SvxPresetListBox::DrawLayout()
{
    SetColCount(getColumnCount());
    SetLineCount(5);
}
template< typename ListType, typename EntryType >
void SvxPresetListBox::FillPresetListBoxImpl(ListType & pList, sal_uInt32 nStartIndex)
{
    const Size aSize( GetIconSize() );
    BitmapEx aBitmap;
    for(long nIndex = 0; nIndex < pList.Count(); nIndex++, nStartIndex++)
    {
        aBitmap = pList.GetBitmapForPreview(nIndex, aSize);
        EntryType* pItem = static_cast<EntryType*>( pList.Get(nIndex) );
        InsertItem(nStartIndex, Image(aBitmap), pItem->GetName());
    }
}

void SvxPresetListBox::FillPresetListBox(XGradientList& pList, sal_uInt32 nStartIndex)
{
    FillPresetListBoxImpl< XGradientList, XGradientEntry>( pList, nStartIndex );
}

void SvxPresetListBox::FillPresetListBox(XHatchList& pList, sal_uInt32 nStartIndex)
{
    FillPresetListBoxImpl< XHatchList, XHatchEntry>( pList, nStartIndex );
}

void SvxPresetListBox::FillPresetListBox(XBitmapList& pList, sal_uInt32 nStartIndex)
{
    FillPresetListBoxImpl< XBitmapList, XBitmapEntry >( pList, nStartIndex );
}

void SvxPresetListBox::FillPresetListBox(XPatternList& pList, sal_uInt32 nStartIndex)
{
    FillPresetListBoxImpl< XPatternList, XBitmapEntry >( pList, nStartIndex );
}

IMPL_LINK(SvxPresetListBox, OnMenuItemSelected, Menu*, pMenu, bool)
{
    if( pMenu == nullptr )
    {
        OSL_ENSURE( pMenu != nullptr, "SvxPresetListBox::OnMenuItemSelected : illegal menu!" );
        return false;
    }
    pMenu->Deactivate();
    OString sIdent = pMenu->GetCurItemIdent();
    if (sIdent == "rename")
        maRenameHdl.Call(this);
    else if (sIdent == "delete")
        maDeleteHdl.Call(this);
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
