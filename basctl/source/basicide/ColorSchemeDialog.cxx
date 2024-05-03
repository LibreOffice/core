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

#include <basidesh.hxx>
#include <iderdll.hxx>
#include <ColorSchemeDialog.hxx>
#include <strings.hrc>
#include <vcl/weld.hxx>
#include <vcl/settings.hxx>
#include <tools/debug.hxx>
#include "baside2.hxx"
#include <officecfg/Office/BasicIDE.hxx>
#include <BasicColorConfig.hxx>
#include <memory>

namespace basctl
{
ColorSchemeDialog::ColorSchemeDialog(weld::Window* pParent,
                                     VclPtr<ModulWindowLayout> pModulWinLayout)
    : GenericDialogController(pParent, u"modules/BasicIDE/ui/colorscheme.ui"_ustr,
                              u"ColorSchemeDialog"_ustr)
    , m_pModulWinLayout(std::move(pModulWinLayout))
    , m_xSchemeList(m_xBuilder->weld_tree_view(u"schemelist"_ustr))
    , m_xUseAppCollors(m_xBuilder->weld_radio_button(u"opt_appcolors"_ustr))
    , m_xUseScheme(m_xBuilder->weld_radio_button(u"opt_usescheme"_ustr))
    , m_xOk(m_xBuilder->weld_button(u"btn_ok"_ustr))
{
    m_pColorConfig = GetShell()->GetColorConfig();
    m_sSelectedSchemeId = m_pColorConfig->GetCurrentColorSchemeName();

    m_xSchemeList->set_size_request(m_xSchemeList->get_approximate_digit_width() * 30,
                                    m_xSchemeList->get_height_rows(10));
    m_xSchemeList->set_selection_mode(SelectionMode::Single);

    Init();
}

ColorSchemeDialog::~ColorSchemeDialog() {}

void ColorSchemeDialog::Init()
{
    m_xOk->connect_clicked(LINK(this, ColorSchemeDialog, BtnOkHdl));
    m_xUseAppCollors->connect_toggled(LINK(this, ColorSchemeDialog, OptionHdl));
    m_xUseScheme->connect_toggled(LINK(this, ColorSchemeDialog, OptionHdl));
    m_xSchemeList->connect_changed(LINK(this, ColorSchemeDialog, SelectHdl));

    // Populate the list with available color schemes
    for (auto const& rName : m_pColorConfig->GetColorSchemeNames())
    {
        // Default schemes (preinstalled with LO) have TranslateIds
        if (m_pColorConfig->IsDefaultScheme(rName))
        {
            m_xSchemeList->append(rName, IDEResId(m_pColorConfig->GetSchemeTranslateId(rName)));
        }
        else
        {
            m_xSchemeList->append(rName, rName);
        }
    }
    m_xSchemeList->make_sorted();

    // Set initial selection in the dialog
    m_xUseAppCollors->set_active(true);
    if (m_sSelectedSchemeId == DEFAULT_SCHEME)
    {
        // The "Application Colors" theme is being used
        m_xSchemeList->set_sensitive(false);
    }
    else
    {
        // Check if the scheme exists
        if (comphelper::findValue(m_pColorConfig->GetColorSchemeNames(), m_sSelectedSchemeId) != -1)
        {
            m_xUseScheme->set_active(true);
            m_xSchemeList->select_id(m_sSelectedSchemeId);
        }
        else
        {
            m_xSchemeList->set_sensitive(false);
        }
    }
}

IMPL_LINK_NOARG(ColorSchemeDialog, BtnOkHdl, weld::Button&, void)
{
    // Collect selected theme
    if (m_xUseAppCollors->get_active())
        m_sSelectedSchemeId = DEFAULT_SCHEME;
    else
        m_sSelectedSchemeId = m_xSchemeList->get_selected_id();

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ColorSchemeDialog, OptionHdl, weld::Toggleable&, void)
{
    if (m_xUseAppCollors->get_active())
    {
        m_xSchemeList->set_sensitive(false);
        m_pModulWinLayout->ApplyColorSchemeToCurrentWindow(DEFAULT_SCHEME);
        m_sSelectedSchemeId = DEFAULT_SCHEME;
    }

    if (m_xUseScheme->get_active())
    {
        m_xSchemeList->set_sensitive(true);
        // Always select a item to avoid having nothing selected
        if (m_sSelectedSchemeId == DEFAULT_SCHEME)
        {
            // If the default color scheme was selected, then choose first entry in the list
            m_xSchemeList->select_id(m_xSchemeList->get_id(0));
            m_pModulWinLayout->ApplyColorSchemeToCurrentWindow(m_xSchemeList->get_id(0));
        }
        else
        {
            // If a color scheme was active, select it in the list
            m_xSchemeList->select_id(m_sSelectedSchemeId);
            m_pModulWinLayout->ApplyColorSchemeToCurrentWindow(m_sSelectedSchemeId);
        }
    }
}

IMPL_LINK_NOARG(ColorSchemeDialog, SelectHdl, weld::TreeView&, void)
{
    // Apply the selected scheme only for the current ModulWinLayout for preview
    // Only if the user presses OK the scheme will be applied to all ModulWinLayout(s)
    m_sSelectedSchemeId = m_xSchemeList->get_selected_id();
    m_pModulWinLayout->ApplyColorSchemeToCurrentWindow(m_sSelectedSchemeId);
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
