/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ChartGradientPalettes.hxx>
#include <ChartGradientPaletteHelper.hxx>
#include <vcl/event.hxx>
#include <vcl/virdev.hxx>

namespace chart
{
constexpr tools::Long BORDER = ChartGradientPaletteLayout::ItemBorder;
constexpr tools::Long SIZE = ChartGradientPaletteLayout::ItemSize;

ChartGradientPalettes::ChartGradientPalettes(weld::Builder& rBuilder, const OUString& id,
                                             const OUString& winId)
    : mrBuilder(rBuilder)
    , mxIconView(mrBuilder.weld_icon_view(id))
    , mxWindow(mrBuilder.weld_scrolled_window(winId))
    , mnHighlightedItemId(0)
{
    mxIconView->connect_mouse_move(LINK(this, ChartGradientPalettes, OnMouseMove));
    mxIconView->connect_query_tooltip(LINK(this, ChartGradientPalettes, OnQueryTooltip));
}

void ChartGradientPalettes::SetSelectHdl(const Link<weld::IconView&, bool>& rLink)
{
    mxIconView->connect_item_activated(rLink);
}

sal_uInt16 ChartGradientPalettes::GetSelectedItemId()
{
    OUString sId = mxIconView->get_selected_id();
    if (sId.isEmpty())
        return 0;
    return sId.toUInt32();
}

void ChartGradientPalettes::SelectItem(sal_uInt16 nItemId) { mxIconView->select(nItemId - 1); }

void ChartGradientPalettes::SetNoSelection() { mxIconView->unselect_all(); }

bool ChartGradientPalettes::IsNoSelection() { return mxIconView->get_selected_id().isEmpty(); }

void ChartGradientPalettes::GrabFocus() { mxIconView->grab_focus(); }

void ChartGradientPalettes::insert(basegfx::BGradient const& rGradientSet)
{
    maGradientSets.push_back(rGradientSet);
}

const basegfx::BGradient* ChartGradientPalettes::getPalette(const sal_uInt32 nItem) const
{
    if (maGradientSets.size() > nItem)
    {
        return &maGradientSets[nItem];
    }
    return nullptr;
}

sal_uInt16 ChartGradientPalettes::GetHighlightedItemId() { return mnHighlightedItemId; }

void ChartGradientPalettes::setMouseMoveHdl(const MouseEventHandler& rLink)
{
    maMouseMoveHdl = rLink;
}

void ChartGradientPalettes::Fill()
{
    constexpr sal_uInt32 nColumns = 4;
    static constexpr Size aSize(2 * BORDER + SIZE, 2 * BORDER + SIZE);

    sal_uInt32 nRows = maGradientSets.size() / nColumns;
    if (maGradientSets.size() % nColumns != 0)
        ++nRows;

    mxIconView->set_item_width(aSize.getWidth());
    mxIconView->set_size_request(nColumns * (aSize.getWidth() + 8),
                                 nRows * (aSize.getHeight() + 8));

    mxIconView->clear();

    mxIconView->freeze();

    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    pVDev->SetOutputSizePixel(aSize);

    tools::Rectangle aDrawArea(0, 0, aSize.getWidth(), aSize.getHeight());
    for (size_t i = 0; i < maGradientSets.size(); ++i)
    {
        const basegfx::BGradient& rGradientSet = maGradientSets[i];
        pVDev->Erase();
        ChartGradientPaletteHelper::renderGradientItem(pVDev, aDrawArea, rGradientSet, true);
        OUString sId = OUString::number(i + 1);
        OUString sName = "Palette " + OUString::number(i + 1);
        mxIconView->insert(-1, &sName, &sId, pVDev, nullptr);
    }

    mxIconView->thaw();
    mnHighlightedItemId = 0;
}

IMPL_LINK(ChartGradientPalettes, OnQueryTooltip, const weld::TreeIter&, rIter, OUString)
{
    OUString sId = mxIconView->get_id(rIter);
    mnHighlightedItemId = sId.isEmpty() ? 0 : static_cast<sal_uInt16>(sId.toUInt32());

    // Suppress actual tooltip text.
    return {};
}

IMPL_LINK(ChartGradientPalettes, OnMouseMove, const MouseEvent&, rMouseEvent, bool)
{
    if (rMouseEvent.IsLeaveWindow())
        mnHighlightedItemId = 0;
    if (maMouseMoveHdl.IsSet())
        return maMouseMoveHdl.Call(rMouseEvent);
    return false;
}

} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
