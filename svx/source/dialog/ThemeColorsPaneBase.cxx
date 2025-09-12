/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/dialog/ThemeColorsPaneBase.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/theme/Theme.hxx>
#include <svx/ColorSets.hxx>
#include <vcl/virdev.hxx>
#include <tools/color.hxx>

namespace svx
{
ThemeColorsPaneBase::ThemeColorsPaneBase(std::unique_ptr<weld::IconView> xIconView)
    : mxIconViewThemeColors(std::move(xIconView))
{
    if (mxIconViewThemeColors)
    {
        mxIconViewThemeColors->connect_selection_changed(
            LINK(this, ThemeColorsPaneBase, SelectionChangedHdl));
        mxIconViewThemeColors->connect_item_activated(
            LINK(this, ThemeColorsPaneBase, ItemActivatedHdl));
    }
}

ThemeColorsPaneBase::~ThemeColorsPaneBase() = default;

void ThemeColorsPaneBase::initColorSets(model::Theme* pTheme)
{
    maColorSets.clear();
    if (mxIconViewThemeColors)
        mxIconViewThemeColors->clear();

    if (pTheme)
        maColorSets.push_back(*pTheme->getColorSet());

    auto const& rColorSetVector = ColorSets::get().getColorSetVector();
    maColorSets.insert(maColorSets.end(), rColorSetVector.begin(), rColorSetVector.end());

    if (mxIconViewThemeColors)
    {
        for (size_t i = 0; i < maColorSets.size(); ++i)
        {
            auto const& rColorSet = maColorSets[i];
            VclPtr<VirtualDevice> pVirDev = CreateColorSetPreview(rColorSet);

            OUString sId = OUString::number(i);
            OUString sName = rColorSet.getName();
            mxIconViewThemeColors->insert(-1, &sName, &sId, pVirDev, nullptr);
        }

        if (!maColorSets.empty())
        {
            mxIconViewThemeColors->select(0);
            mpCurrentColorSet = std::make_shared<model::ColorSet>(maColorSets[0]);
        }
    }
}

VclPtr<VirtualDevice> ThemeColorsPaneBase::CreateColorSetPreview(const model::ColorSet& rColorSet)
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    const Size aSize(100, 50);
    pVDev->SetOutputSizePixel(aSize);

    const int nRows = 2;
    const int nCols = 4;
    const int nHorizontalPadding = 9;
    const int nVerticalPadding = 3;
    const int nMargin = 3;

    const int nAvailableWidth = aSize.Width() - (2 * nHorizontalPadding);
    const int nAvailableHeight = aSize.Height() - (2 * nVerticalPadding);
    const int nColorCellHeight = (nAvailableHeight - ((nRows - 1) * nMargin)) / nRows;
    const int nColorCellWidth = (nAvailableWidth - ((nCols - 1) * nMargin)) / nCols;

    // Draw border around entire color set
    pVDev->SetLineColor(COL_LIGHTGRAY);
    pVDev->DrawRect(tools::Rectangle(Point(0, 0), aSize));

    for (int i = 0; i < 8; ++i)
    {
        const int nCol = i / 2;
        const int nRow = i % 2;

        const int nX = nHorizontalPadding + (nCol * (nColorCellWidth + nMargin));
        const int nY = nVerticalPadding + (nRow * (nColorCellHeight + nMargin));

        const tools::Rectangle aRect(Point(nX, nY), Size(nColorCellWidth, nColorCellHeight));

        Color aColor = rColorSet.getColor(static_cast<model::ThemeColorType>(i + 2));

        pVDev->SetLineColor(COL_LIGHTGRAY);
        pVDev->SetFillColor(aColor);
        pVDev->DrawRect(aRect);
    }
    return pVDev;
}

IMPL_LINK_NOARG(ThemeColorsPaneBase, SelectionChangedHdl, weld::IconView&, void)
{
    if (!mxIconViewThemeColors)
        return;

    OUString sId = mxIconViewThemeColors->get_selected_id();
    if (sId.isEmpty())
        return;

    sal_uInt32 nIndex = sId.toUInt32();

    if (nIndex >= maColorSets.size())
        return;

    mpCurrentColorSet = std::make_shared<model::ColorSet>(maColorSets[nIndex]);
}

IMPL_LINK(ThemeColorsPaneBase, ItemActivatedHdl, weld::IconView&, /*rIconView*/, bool)
{
    SelectionChangedHdl(*mxIconViewThemeColors);
    onColorSetActivated();
    return true;
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
