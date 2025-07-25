/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/dialog/ThemeDialog.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/theme/Theme.hxx>
#include <svx/ColorSets.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/lok.hxx>
#include <vcl/virdev.hxx>

namespace svx
{
ThemeDialog::ThemeDialog(weld::Window* pParent, model::Theme* pTheme)
    : GenericDialogController(pParent, u"svx/ui/themedialog.ui"_ustr, u"ThemeDialog"_ustr)
    , mpTheme(pTheme)
    , mxIconViewThemeColors(m_xBuilder->weld_icon_view(u"iconview_theme_colors"_ustr))
    , mxAdd(m_xBuilder->weld_button(u"button_add"_ustr))
{
    mxIconViewThemeColors->connect_item_activated(LINK(this, ThemeDialog, ItemActivatedHdl));
    mxIconViewThemeColors->connect_selection_changed(LINK(this, ThemeDialog, SelectionChangedHdl));

    mxAdd->connect_clicked(LINK(this, ThemeDialog, ButtonClicked));

    initColorSets();

    if (!maColorSets.empty())
    {
        mxIconViewThemeColors->select(0);
        mpCurrentColorSet = std::make_shared<model::ColorSet>(maColorSets[0]);
    }
}

ThemeDialog::~ThemeDialog()
{
    if (mxSubDialog)
        mxSubDialog->response(RET_CANCEL);
}

void ThemeDialog::initColorSets()
{
    if (mpTheme)
        maColorSets.push_back(*mpTheme->getColorSet());

    auto const& rColorSetVector = ColorSets::get().getColorSetVector();
    maColorSets.insert(maColorSets.end(), rColorSetVector.begin(), rColorSetVector.end());

    for (size_t i = 0; i < maColorSets.size(); ++i)
    {
        auto const& rColorSet = maColorSets[i];
        VclPtr<VirtualDevice> pVirDev = CreateColorSetPreview(rColorSet);
        BitmapEx aBitmapEx(pVirDev->GetBitmap(Point(0, 0), pVirDev->GetOutputSizePixel()));

        OUString sId = OUString::number(i);
        OUString sName = rColorSet.getName();
        mxIconViewThemeColors->insert(-1, &sName, &sId, &aBitmapEx, nullptr);
    }
}

VclPtr<VirtualDevice> ThemeDialog::CreateColorSetPreview(const model::ColorSet& rColorSet)
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

IMPL_LINK(ThemeDialog, ItemActivatedHdl, weld::IconView&, iter, bool)
{
    SelectionChangedHdl(iter);
    if (!comphelper::LibreOfficeKit::isActive())
        m_xDialog->response(RET_OK);
    return true;
}

IMPL_LINK_NOARG(ThemeDialog, SelectionChangedHdl, weld::IconView&, void)
{
    OUString sId = mxIconViewThemeColors->get_selected_id();
    if (sId.isEmpty())
        return;

    sal_uInt32 nIndex = sId.toUInt32();

    if (nIndex >= maColorSets.size())
        return;

    mpCurrentColorSet = std::make_shared<model::ColorSet>(maColorSets[nIndex]);
}

void ThemeDialog::runThemeColorEditDialog()
{
    if (mxSubDialog)
        return;

    mxSubDialog = std::make_shared<svx::ThemeColorEditDialog>(getDialog(), *mpCurrentColorSet);

    weld::DialogController::runAsync(mxSubDialog, [this](sal_uInt32 nResult) {
        if (nResult != RET_OK)
        {
            mxAdd->set_sensitive(true);
            mxSubDialog = nullptr;
            return;
        }
        auto aColorSet = mxSubDialog->getColorSet();
        if (!aColorSet.getName().isEmpty())
        {
            ColorSets::get().insert(aColorSet);
            maColorSets.clear();
            mxIconViewThemeColors->clear();

            initColorSets();

            mxIconViewThemeColors->select(maColorSets.size() - 1);
            mpCurrentColorSet
                = std::make_shared<model::ColorSet>(maColorSets[maColorSets.size() - 1]);
        }
        mxAdd->set_sensitive(true);
        mxSubDialog = nullptr;
    });
}

IMPL_LINK(ThemeDialog, ButtonClicked, weld::Button&, rButton, void)
{
    mxAdd->set_sensitive(false);
    if (mpCurrentColorSet && mxAdd.get() == &rButton)
    {
        runThemeColorEditDialog();
    }
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
