/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/dialog/ThemeDialog.hxx>
#include <svx/dialog/ThemeColorEditDialog.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/theme/Theme.hxx>
#include <svx/ColorSets.hxx>
#include <vcl/svapp.hxx>
#include <svx/colorbox.hxx>
#include <comphelper/lok.hxx>

namespace svx
{
ThemeDialog::ThemeDialog(weld::Window* pParent, model::Theme* pTheme)
    : GenericDialogController(pParent, "svx/ui/themedialog.ui", "ThemeDialog")
    , mpWindow(pParent)
    , mpTheme(pTheme)
    , mxValueSetThemeColors(new svx::ThemeColorValueSet)
    , mxValueSetThemeColorsWindow(
          new weld::CustomWeld(*m_xBuilder, "valueset_theme_colors", *mxValueSetThemeColors))
    , mxAdd(m_xBuilder->weld_button("button_add"))
{
    mxValueSetThemeColors->SetColCount(3);
    mxValueSetThemeColors->SetLineCount(4);
    mxValueSetThemeColors->SetColor(Application::GetSettings().GetStyleSettings().GetFaceColor());
    mxValueSetThemeColors->SetDoubleClickHdl(LINK(this, ThemeDialog, DoubleClickValueSetHdl));
    mxValueSetThemeColors->SetSelectHdl(LINK(this, ThemeDialog, SelectItem));

    mxAdd->connect_clicked(LINK(this, ThemeDialog, ButtonClicked));

    initColorSets();

    if (!maColorSets.empty())
    {
        mxValueSetThemeColors->SelectItem(1); // ItemId 1, position 0
        mpCurrentColorSet = std::make_shared<model::ColorSet>(maColorSets[0]);
    }
}

ThemeDialog::~ThemeDialog() = default;

void ThemeDialog::initColorSets()
{
    if (mpTheme)
        maColorSets.push_back(*mpTheme->getColorSet());

    auto const& rColorSetVector = ColorSets::get().getColorSetVector();
    maColorSets.insert(maColorSets.end(), rColorSetVector.begin(), rColorSetVector.end());

    for (auto const& rColorSet : maColorSets)
    {
        mxValueSetThemeColors->insert(rColorSet);
    }

    mxValueSetThemeColors->SetOptimalSize();
}

IMPL_LINK_NOARG(ThemeDialog, DoubleClickValueSetHdl, ValueSet*, void)
{
    SelectItem(nullptr);
    if (!comphelper::LibreOfficeKit::isActive())
        m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ThemeDialog, SelectItem, ValueSet*, void)
{
    sal_uInt32 nItemId = mxValueSetThemeColors->GetSelectedItemId();
    if (!nItemId)
        return;

    sal_uInt32 nIndex = nItemId - 1;

    if (nIndex >= maColorSets.size())
        return;

    mpCurrentColorSet = std::make_shared<model::ColorSet>(maColorSets[nIndex]);
}

void ThemeDialog::runThemeColorEditDialog()
{
    auto pDialog = std::make_shared<svx::ThemeColorEditDialog>(mpWindow, *mpCurrentColorSet);
    std::shared_ptr<DialogController> xKeepAlive(shared_from_this());
    weld::DialogController::runAsync(pDialog, [this, xKeepAlive, pDialog](sal_uInt32 nResult) {
        if (nResult != RET_OK)
            return;
        auto aColorSet = pDialog->getColorSet();
        if (!aColorSet.getName().isEmpty())
        {
            ColorSets::get().insert(aColorSet, ColorSets::IdenticalNameAction::AutoRename);
            maColorSets.clear();
            mxValueSetThemeColors->Clear();

            initColorSets();

            mxValueSetThemeColors->SelectItem(maColorSets.size() - 1);
            mpCurrentColorSet
                = std::make_shared<model::ColorSet>(maColorSets[maColorSets.size() - 1]);
        }
    });
}

IMPL_LINK(ThemeDialog, ButtonClicked, weld::Button&, rButton, void)
{
    if (mpCurrentColorSet && mxAdd.get() == &rButton)
    {
        runThemeColorEditDialog();
    }
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
