/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/SvxColorValueSet.hxx>
#include <svx/xtable.hxx>
#include <vcl/svapp.hxx>

//////////////////////////////////////////////////////////////////////////////

SvxColorValueSet::SvxColorValueSet(Window* _pParent, WinBits nWinStyle)
:   ValueSet(_pParent, nWinStyle)
{
    SetEdgeBlending(true);
}

SvxColorValueSet::SvxColorValueSet(Window* _pParent, const ResId& rResId)
:   ValueSet(_pParent, rResId)
{
    SetEdgeBlending(true);
}

sal_uInt32 SvxColorValueSet::getMaxRowCount() const
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    return rStyleSettings.GetColorValueSetMaximumRowCount();
}

sal_uInt32 SvxColorValueSet::getEntryEdgeLength() const
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    return rStyleSettings.GetListBoxPreviewDefaultPixelSize().Height() + 1;
}

sal_uInt32 SvxColorValueSet::getColumnCount() const
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    return rStyleSettings.GetColorValueSetColumnCount();
}

void SvxColorValueSet::addEntriesForXColorList(const XColorListSharedPtr aXColorList, sal_uInt32 nStartIndex)
{
    const sal_uInt32 nColorCount(aXColorList ? aXColorList->Count() : 0);

    for(sal_uInt32 nIndex(0); nIndex < nColorCount; nIndex++, nStartIndex++)
    {
        const XColorEntry* pEntry = aXColorList->GetColor(nIndex);

        if(pEntry)
        {
            InsertItem(static_cast< sal_uInt16 >(nStartIndex), pEntry->GetColor(), pEntry->GetName());
        }
        else
        {
            OSL_ENSURE(false, "OOps, XColorList with empty entries (!)");
        }
    }
}

Size SvxColorValueSet::layoutAllVisible(sal_uInt32 nEntryCount)
{
    if(!nEntryCount)
    {
        nEntryCount++;
    }

    const sal_uInt32 nRowCount(basegfx::fround(ceil(double(nEntryCount)/getColumnCount())));
    const Size aItemSize(getEntryEdgeLength() - 2, getEntryEdgeLength() - 2);
    const WinBits aWinBits(GetStyle() & ~WB_VSCROLL);

    if(nRowCount > getMaxRowCount())
    {
        SetStyle(aWinBits|WB_VSCROLL);
    }
    else
    {
        SetStyle(aWinBits);
    }

    SetColCount(static_cast< sal_uInt16 >(getColumnCount()));
    SetLineCount(static_cast< sal_uInt16 >(std::min(nRowCount, getMaxRowCount())));
    SetItemWidth(aItemSize.Width());
    SetItemHeight(aItemSize.Height());

    return CalcWindowSizePixel(aItemSize);
}

Size SvxColorValueSet::layoutToGivenHeight(sal_uInt32 nHeight, sal_uInt32 nEntryCount)
{
    if(!nEntryCount)
    {
        nEntryCount++;
    }

    const Size aItemSize(getEntryEdgeLength(), getEntryEdgeLength());
    const WinBits aWinBits(GetStyle() & ~WB_VSCROLL);

    // get size with all fields disabled
    const WinBits aWinBitsNoScrollNoFields(GetStyle() & ~(WB_VSCROLL|WB_NAMEFIELD|WB_NONEFIELD));
    SetStyle(aWinBitsNoScrollNoFields);
    const Size aSizeNoScrollNoFields(CalcWindowSizePixel(aItemSize, static_cast< sal_uInt16 >(getColumnCount())));

    // get size with all needed fields
    SetStyle(aWinBits);
    Size aNewSize(CalcWindowSizePixel(aItemSize, static_cast< sal_uInt16 >(getColumnCount())));

    // evtl. activate vertical scroll
    const bool bAdaptHeight(static_cast< sal_uInt32 >(aNewSize.Height()) > nHeight);

    if(bAdaptHeight)
    {
        SetStyle(aWinBits|WB_VSCROLL);
        aNewSize = CalcWindowSizePixel(aItemSize, static_cast< sal_uInt16 >(getColumnCount()));
    }

    // calculate field height and available height for requested height
    const sal_uInt32 nFieldHeight(aNewSize.Height() - aSizeNoScrollNoFields.Height());
    const sal_uInt32 nAvailableHeight(nHeight >= nFieldHeight ? nHeight - nFieldHeight : 0);

    // calculate how many lines can be shown there
    const Size aItemSizePixel(CalcItemSizePixel(aItemSize));
    const sal_uInt32 nLineCount((nAvailableHeight + aItemSizePixel.Height() - 1) / aItemSizePixel.Height());

    // set height to wanted height
    aNewSize.Height() = nHeight;

    SetItemWidth(aItemSize.Width());
    SetItemHeight(aItemSize.Height());
    SetColCount(static_cast< sal_uInt16 >(getColumnCount()));
    SetLineCount(static_cast< sal_uInt16 >(nLineCount));

    return aNewSize;
}

//////////////////////////////////////////////////////////////////////////////
// eof
