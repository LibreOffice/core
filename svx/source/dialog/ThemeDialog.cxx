/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/dialog/ThemeDialog.hxx>
#include <docmodel/theme/ThemeColor.hxx>
#include <vcl/svapp.hxx>

namespace svx
{
ThemeDialog::ThemeDialog(weld::Window* pParent, svx::Theme* pTheme,
                         std::shared_ptr<IThemeColorChanger> const& pChanger)
    : GenericDialogController(pParent, "svx/ui/themedialog.ui", "ThemeDialog")
    , mpTheme(pTheme)
    , mpChanger(pChanger)
    , mxValueSetThemeColors(new svx::ThemeColorValueSet)
    , mxValueSetThemeColorsWindow(
          new weld::CustomWeld(*m_xBuilder, "valueset_theme_colors", *mxValueSetThemeColors))
{
    mxValueSetThemeColors->SetColCount(2);
    mxValueSetThemeColors->SetLineCount(6);
    mxValueSetThemeColors->SetColor(Application::GetSettings().GetStyleSettings().GetFaceColor());
    mxValueSetThemeColors->SetDoubleClickHdl(LINK(this, ThemeDialog, DoubleClickValueSetHdl));

    maColorSets.init();
    maColorSets.insert(*mpTheme->GetColorSet());

    for (auto const& rColorSet : maColorSets.getColorSets())
    {
        mxValueSetThemeColors->insert(rColorSet);
    }

    mxValueSetThemeColors->SetOptimalSize();

    if (!maColorSets.getColorSets().empty())
        mxValueSetThemeColors->SelectItem(1); // ItemId 1, position 0
}

ThemeDialog::~ThemeDialog() = default;

IMPL_LINK_NOARG(ThemeDialog, DoubleClickValueSetHdl, ValueSet*, void) { DoubleClickHdl(); }

void ThemeDialog::DoubleClickHdl()
{
    sal_uInt32 nItemId = mxValueSetThemeColors->GetSelectedItemId();
    if (!nItemId)
        return;

    sal_uInt32 nIndex = nItemId - 1;

    svx::ColorSet const& rColorSet = maColorSets.getColorSet(nIndex);

    mpChanger->apply(rColorSet);
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
