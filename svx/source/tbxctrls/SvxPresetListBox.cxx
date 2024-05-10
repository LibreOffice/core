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
#include <vcl/commandevent.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>

SvxPresetListBox::SvxPresetListBox(std::unique_ptr<weld::ScrolledWindow> pWindow)
    : ValueSet(std::move(pWindow))
    , aIconSize(60, 64)
{
    SetEdgeBlending(true);
}

void SvxPresetListBox::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    ValueSet::SetDrawingArea(pDrawingArea);
    SetStyle(GetStyle() | WB_ITEMBORDER);
}

void SvxPresetListBox::Resize()
{
    DrawLayout();
    WinBits aWinBits(GetStyle());
    aWinBits |= WB_VSCROLL;
    SetStyle(aWinBits);
    ValueSet::Resize();
}

bool SvxPresetListBox::Command(const CommandEvent& rEvent)
{
    if (rEvent.GetCommand() != CommandEventId::ContextMenu)
        return CustomWidgetController::Command(rEvent);
    const sal_uInt16 nIndex = GetSelectedItemId();
    if (nIndex > 0)
    {
        std::unique_ptr<weld::Builder> xBuilder(
            Application::CreateBuilder(GetDrawingArea(), u"svx/ui/presetmenu.ui"_ustr));
        std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu(u"menu"_ustr));
        OnMenuItemSelected(xMenu->popup_at_rect(
            GetDrawingArea(), tools::Rectangle(rEvent.GetMousePosPixel(), Size(1, 1))));
        return true;
    }
    return false;
}

void SvxPresetListBox::DrawLayout()
{
    SetColCount(nColCount);
    SetLineCount(5);
}

template <typename ListType, typename EntryType>
void SvxPresetListBox::FillPresetListBoxImpl(ListType& pList, sal_uInt32 nStartIndex)
{
    const Size aSize(GetIconSize());
    BitmapEx aBitmap;
    for (tools::Long nIndex = 0; nIndex < pList.Count(); nIndex++, nStartIndex++)
    {
        aBitmap = pList.GetBitmapForPreview(nIndex, aSize);
        EntryType* pItem = static_cast<EntryType*>(pList.Get(nIndex));
        InsertItem(nStartIndex, Image(aBitmap), pItem->GetName());
    }
}

void SvxPresetListBox::FillPresetListBox(XGradientList& pList, sal_uInt32 nStartIndex)
{
    FillPresetListBoxImpl<XGradientList, XGradientEntry>(pList, nStartIndex);
}

void SvxPresetListBox::FillPresetListBox(XHatchList& pList, sal_uInt32 nStartIndex)
{
    FillPresetListBoxImpl<XHatchList, XHatchEntry>(pList, nStartIndex);
}

void SvxPresetListBox::FillPresetListBox(XBitmapList& pList, sal_uInt32 nStartIndex)
{
    FillPresetListBoxImpl<XBitmapList, XBitmapEntry>(pList, nStartIndex);
}

void SvxPresetListBox::FillPresetListBox(XPatternList& pList, sal_uInt32 nStartIndex)
{
    FillPresetListBoxImpl<XPatternList, XBitmapEntry>(pList, nStartIndex);
}

void SvxPresetListBox::OnMenuItemSelected(std::u16string_view rIdent)
{
    if (rIdent == u"rename")
        maRenameHdl.Call(this);
    else if (rIdent == u"delete")
        maDeleteHdl.Call(this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
