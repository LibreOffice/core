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

#include <svx/SvxColorValueSet.hxx>
#include <svx/xtable.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/diagnose.h>

#include <svx/uiobject.hxx>

SvxColorValueSet::SvxColorValueSet(std::unique_ptr<weld::ScrolledWindow> pWindow)
    : ValueSet(std::move(pWindow))
{
    SetEdgeBlending(true);
}

FactoryFunction SvxColorValueSet::GetUITestFactory() const
{
    return SvxColorValueSetUIObject::create;
}

sal_uInt32 SvxColorValueSet::getEntryEdgeLength()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    return rStyleSettings.GetListBoxPreviewDefaultPixelSize().Height() + 1;
}

sal_uInt32 SvxColorValueSet::getColumnCount()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    return rStyleSettings.GetColorValueSetColumnCount();
}

void SvxColorValueSet::addEntriesForXColorList(const XColorList& rXColorList, sal_uInt32 nStartIndex)
{
    const sal_uInt32 nColorCount(rXColorList.Count());

    for(sal_uInt32 nIndex(0); nIndex < nColorCount; nIndex++, nStartIndex++)
    {
        const XColorEntry* pEntry = rXColorList.GetColor(nIndex);

        if(pEntry)
        {
            InsertItem(nStartIndex, pEntry->GetColor(), pEntry->GetName());
        }
        else
        {
            OSL_ENSURE(false, "OOps, XColorList with empty entries (!)");
        }
    }
}

void SvxColorValueSet::addEntriesForColorSet(const std::set<Color>& rColorSet, std::u16string_view rNamePrefix)
{
    sal_uInt32 nStartIndex = 1;
    if(!rNamePrefix.empty())
    {
        for(const auto& rColor : rColorSet)
        {
            InsertItem(nStartIndex, rColor, rNamePrefix + OUString::number(nStartIndex));
            nStartIndex++;
        }
    }
    else
    {
        for(const auto& rColor : rColorSet)
        {
            InsertItem(nStartIndex, rColor, u""_ustr);
            nStartIndex++;
        }
    }
}

Size SvxColorValueSet::layoutAllVisible(sal_uInt32 nEntryCount)
{
    if(!nEntryCount)
    {
        nEntryCount++;
    }

    const sal_uInt32 nRowCount(ceil(double(nEntryCount)/SvxColorValueSet::getColumnCount()));
    const Size aItemSize(SvxColorValueSet::getEntryEdgeLength() - 2, SvxColorValueSet::getEntryEdgeLength() - 2);
    const WinBits aWinBits(GetStyle() & ~WB_VSCROLL);

    if (nRowCount > SvxColorValueSet::getMaxRowCount())
    {
        SetStyle(aWinBits|WB_VSCROLL);
    }
    else
    {
        SetStyle(aWinBits);
    }

    SetColCount(SvxColorValueSet::getColumnCount());
    SetLineCount(std::min(nRowCount, SvxColorValueSet::getMaxRowCount()));
    SetItemWidth(aItemSize.Width());
    SetItemHeight(aItemSize.Height());

    return CalcWindowSizePixel(aItemSize);
}

void SvxColorValueSet::Resize()
{
    layoutToGivenHeight(GetOutputSizePixel().Height(), GetItemCount());
    ValueSet::Resize();
}

void SvxColorValueSet::layoutToGivenHeight(sal_uInt32 nHeight, sal_uInt32 nEntryCount)
{
    if(!nEntryCount)
    {
        nEntryCount++;
    }

    const Size aItemSize(SvxColorValueSet::getEntryEdgeLength() - 2, SvxColorValueSet::getEntryEdgeLength() - 2);
    const WinBits aWinBits(GetStyle() & ~WB_VSCROLL);

    // get size with all fields disabled
    const WinBits aWinBitsNoScrollNoFields(GetStyle() & ~(WB_VSCROLL|WB_NAMEFIELD|WB_NONEFIELD));
    SetStyle(aWinBitsNoScrollNoFields);
    const Size aSizeNoScrollNoFields(CalcWindowSizePixel(aItemSize, SvxColorValueSet::getColumnCount()));

    // get size with all needed fields
    SetStyle(aWinBits);
    Size aNewSize(CalcWindowSizePixel(aItemSize, SvxColorValueSet::getColumnCount()));

    const Size aItemSizePixel(CalcItemSizePixel(aItemSize));
    // calculate field height and available height for requested height
    const sal_uInt32 nFieldHeight(aNewSize.Height() - aSizeNoScrollNoFields.Height());
    const sal_uInt32 nAvailableHeight(nHeight >= nFieldHeight ? nHeight - nFieldHeight + aItemSizePixel.Height() - 1 : 0);

    // calculate how many lines can be shown there
    const sal_uInt32 nLineCount(nAvailableHeight / aItemSizePixel.Height());
    const sal_uInt32 nLineMax(ceil(double(nEntryCount)/SvxColorValueSet::getColumnCount()));

    if(nLineMax > nLineCount)
    {
        SetStyle(aWinBits|WB_VSCROLL);
    }

    SetItemWidth(aItemSize.Width());
    SetItemHeight(aItemSize.Height());
    SetColCount(SvxColorValueSet::getColumnCount());
    SetLineCount(nLineCount);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
