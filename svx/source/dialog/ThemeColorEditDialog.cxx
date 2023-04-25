/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/dialog/ThemeColorEditDialog.hxx>
#include <vcl/svapp.hxx>
#include <svx/colorbox.hxx>

namespace svx
{
ThemeColorEditDialog::ThemeColorEditDialog(weld::Window* pParent, model::ColorSet& rColorSet)
    : GenericDialogController(pParent, "svx/ui/themecoloreditdialog.ui", "ThemeColorEditDialog")
    , maColorSet(rColorSet)
    , mxThemeColorsNameEntry(m_xBuilder->weld_entry("entryThemeColorsName"))
    , mxDark1(new ColorListBox(m_xBuilder->weld_menu_button("buttonDark1"),
                               [pParent] { return pParent; }))
    , mxLight1(new ColorListBox(m_xBuilder->weld_menu_button("buttonLight1"),
                                [pParent] { return pParent; }))
    , mxDark2(new ColorListBox(m_xBuilder->weld_menu_button("buttonDark2"),
                               [pParent] { return pParent; }))
    , mxLight2(new ColorListBox(m_xBuilder->weld_menu_button("buttonLight2"),
                                [pParent] { return pParent; }))
    , mxAccent1(new ColorListBox(m_xBuilder->weld_menu_button("buttonAccent1"),
                                 [pParent] { return pParent; }))
    , mxAccent2(new ColorListBox(m_xBuilder->weld_menu_button("buttonAccent2"),
                                 [pParent] { return pParent; }))
    , mxAccent3(new ColorListBox(m_xBuilder->weld_menu_button("buttonAccent3"),
                                 [pParent] { return pParent; }))
    , mxAccent4(new ColorListBox(m_xBuilder->weld_menu_button("buttonAccent4"),
                                 [pParent] { return pParent; }))
    , mxAccent5(new ColorListBox(m_xBuilder->weld_menu_button("buttonAccent5"),
                                 [pParent] { return pParent; }))
    , mxAccent6(new ColorListBox(m_xBuilder->weld_menu_button("buttonAccent6"),
                                 [pParent] { return pParent; }))
    , mxHyperlink(new ColorListBox(m_xBuilder->weld_menu_button("buttonHyperlink"),
                                   [pParent] { return pParent; }))
    , mxFollowHyperlink(new ColorListBox(m_xBuilder->weld_menu_button("buttonFollowHyperlink"),
                                         [pParent] { return pParent; }))
{
    mxThemeColorsNameEntry->set_text(rColorSet.getName());
    mxDark1->SelectEntry(rColorSet.getColor(model::ThemeColorType::Dark1));
    mxLight1->SelectEntry(rColorSet.getColor(model::ThemeColorType::Light1));
    mxDark2->SelectEntry(rColorSet.getColor(model::ThemeColorType::Dark2));
    mxLight2->SelectEntry(rColorSet.getColor(model::ThemeColorType::Light2));
    mxAccent1->SelectEntry(rColorSet.getColor(model::ThemeColorType::Accent1));
    mxAccent2->SelectEntry(rColorSet.getColor(model::ThemeColorType::Accent2));
    mxAccent3->SelectEntry(rColorSet.getColor(model::ThemeColorType::Accent3));
    mxAccent4->SelectEntry(rColorSet.getColor(model::ThemeColorType::Accent4));
    mxAccent5->SelectEntry(rColorSet.getColor(model::ThemeColorType::Accent5));
    mxAccent6->SelectEntry(rColorSet.getColor(model::ThemeColorType::Accent6));
    mxHyperlink->SelectEntry(rColorSet.getColor(model::ThemeColorType::Hyperlink));
    mxFollowHyperlink->SelectEntry(rColorSet.getColor(model::ThemeColorType::FollowedHyperlink));
}

ThemeColorEditDialog::~ThemeColorEditDialog() = default;

model::ColorSet ThemeColorEditDialog::getColorSet()
{
    OUString aName = mxThemeColorsNameEntry->get_text();

    model::ColorSet aColorSet(aName);

    if (!aName.isEmpty())
    {
        aColorSet.add(model::ThemeColorType::Dark1, mxDark1->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Light1, mxLight1->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Dark2, mxDark2->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Light2, mxLight2->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Accent1, mxAccent1->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Accent2, mxAccent2->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Accent3, mxAccent3->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Accent4, mxAccent4->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Accent5, mxAccent5->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Accent6, mxAccent6->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::Hyperlink, mxHyperlink->GetSelectEntryColor());
        aColorSet.add(model::ThemeColorType::FollowedHyperlink,
                      mxFollowHyperlink->GetSelectEntryColor());
    }
    return aColorSet;
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
