/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ChartColorPalettes.hxx>
#include <ChartColorPaletteHelper.hxx>
#include <vcl/event.hxx>

namespace chart
{
constexpr tools::Long BORDER = ChartColorPaletteLayout::ItemBorder;
constexpr tools::Long SIZE = ChartColorPaletteLayout::ItemSize;

void ChartColorPalettes::insert(ChartColorPalette const& rColorSet)
{
    maColorSets.push_back(rColorSet);
    InsertItem(maColorSets.size());
}

const ChartColorPalette* ChartColorPalettes::getPalette(const sal_uInt32 nItem) const
{
    if (maColorSets.size() > nItem)
    {
        return &maColorSets[nItem];
    }
    return nullptr;
}

void ChartColorPalettes::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    ValueSet::SetDrawingArea(pDrawingArea);
    SetStyle(WB_TABSTOP | WB_DOUBLEBORDER | WB_FLATVALUESET);
    SetItemWidth(BORDER * 6 + SIZE * ChartColorPaletteSize / 2);
    SetItemHeight(BORDER * 5 + SIZE * 2);
}

void ChartColorPalettes::UserDraw(const UserDrawEvent& rUserDrawEvent)
{
    vcl::RenderContext* pDev = rUserDrawEvent.GetRenderContext();
    const tools::Rectangle aDrawArea = rUserDrawEvent.GetRect();
    const sal_uInt16 nItemId = rUserDrawEvent.GetItemId();
    ChartColorPalette const& rColorSet = maColorSets[nItemId - 1];
    ChartColorPaletteHelper::renderColorPalette(pDev, aDrawArea, rColorSet, false);
}

void ChartColorPalettes::StyleUpdated()
{
    SetFormat();
    Invalidate();
    ValueSet::StyleUpdated();
}

bool ChartColorPalettes::MouseMove(const MouseEvent& rMEvt)
{
    bool bRes = ValueSet::MouseMove(rMEvt);
    maMouseMoveHdl.Call(rMEvt);
    return bRes;
}

void ChartColorPalettes::setMouseMoveHdl(const MouseEventHandler& rLink) { maMouseMoveHdl = rLink; }

} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
