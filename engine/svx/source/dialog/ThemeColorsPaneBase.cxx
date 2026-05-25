/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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

void ThemeColorsPaneBase::initColorSets(const model::Theme* pTheme)
{
    maColorSets.clear();

    OUString sDocumentThemeName;
    if (pTheme)
    {
        maColorSets.push_back(*pTheme->getColorSet());
        sDocumentThemeName = maColorSets.front().getName();
    }

    // Re-scan the configured theme paths each time the pane opens. In
    // COKit/Online the singleton was first initialized during forkit
    // preinit with a throwaway temp $(userurl), and the kit's
    // asyncInstallPresets places user .theme files into the real
    // $(userurl)/themes asynchronously, possibly after the kit's
    // SECOND_INIT has already run. Re-scanning here ensures both the
    // user theme folder and the list of available themes are current.
    ColorSets::get().init();

    // Skip the singleton entry that matches the document's current
    // theme so it is not shown twice in the icon view: the document's
    // (possibly customized) copy stays at index 0, the pristine
    // singleton duplicate is dropped.
    auto const& rColorSetVector = ColorSets::get().getColorSetVector();
    for (model::ColorSet const& rColorSet : rColorSetVector)
    {
        if (rColorSet.getName() != sDocumentThemeName)
            maColorSets.push_back(rColorSet);
    }

    if (mxIconViewThemeColors)
    {
        mxIconViewThemeColors->freeze();
        mxIconViewThemeColors->clear();

        // The legacy "LibreOffice" theme is kept in the model so documents that
        // already reference it keep working, but it is not offered as a choice
        // in the UI. "Office" is the fallback selection in its place.
        static constexpr std::u16string_view aHiddenThemeName = u"LibreOffice";
        static constexpr std::u16string_view aFallbackDefaultName = u"Office";

        int nVisiblePos = 0;
        int nFirstVisiblePos = -1;
        size_t nFirstVisibleIndex = 0;
        int nFallbackPos = -1;
        size_t nFallbackIndex = 0;
        for (size_t i = 0; i < maColorSets.size(); ++i)
        {
            model::ColorSet const& rColorSet = maColorSets[i];
            if (rColorSet.getName() == aHiddenThemeName)
                continue;

            ScopedVclPtr<VirtualDevice> pVirDev = CreateColorSetPreview(rColorSet);

            Bitmap aBitmap(pVirDev->GetBitmap(Point(0, 0), pVirDev->GetOutputSizePixel()));
            OUString sId = OUString::number(i);
            OUString sName = rColorSet.getName();
            mxIconViewThemeColors->insert(-1, &sName, &sId, &aBitmap, nullptr);

            if (nFirstVisiblePos < 0)
            {
                nFirstVisiblePos = nVisiblePos;
                nFirstVisibleIndex = i;
            }
            if (nFallbackPos < 0 && rColorSet.getName() == aFallbackDefaultName)
            {
                nFallbackPos = nVisiblePos;
                nFallbackIndex = i;
            }
            ++nVisiblePos;
        }

        mxIconViewThemeColors->thaw();

        if (nFirstVisiblePos >= 0)
        {
            // If the document's current theme was hidden, fall back to "Office".
            const bool bDocumentThemeHidden
                = pTheme && !maColorSets.empty() && maColorSets[0].getName() == aHiddenThemeName;
            int nSelectedPos = nFirstVisiblePos;
            size_t nSelectedIndex = nFirstVisibleIndex;
            if (bDocumentThemeHidden && nFallbackPos >= 0)
            {
                nSelectedPos = nFallbackPos;
                nSelectedIndex = nFallbackIndex;
            }
            mxIconViewThemeColors->select(nSelectedPos);
            mpCurrentColorSet = std::make_shared<model::ColorSet>(maColorSets[nSelectedIndex]);
        }
    }
}

ScopedVclPtr<VirtualDevice>
ThemeColorsPaneBase::CreateColorSetPreview(const model::ColorSet& rColorSet)
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
