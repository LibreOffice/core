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

#undef SC_DLLIMPLEMENTATION

#include <textimportoptions.hxx>
#include <svx/langbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <i18nlangtag/languagetag.hxx>

ScTextImportOptionsDlg::ScTextImportOptionsDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/scalc/ui/textimportoptions.ui", "TextImportOptionsDialog")
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xRbAutomatic(m_xBuilder->weld_radio_button("automatic"))
    , m_xRbCustom(m_xBuilder->weld_radio_button("custom"))
    , m_xBtnConvertDate(m_xBuilder->weld_check_button("convertdata"))
    , m_xLbCustomLang(new LanguageBox(m_xBuilder->weld_combo_box("lang")))
{
    init();
}

ScTextImportOptionsDlg::~ScTextImportOptionsDlg()
{
}

LanguageType ScTextImportOptionsDlg::getLanguageType() const
{
    if (m_xRbAutomatic->get_active())
        return LANGUAGE_SYSTEM;

    return m_xLbCustomLang->get_active_id();
}

bool ScTextImportOptionsDlg::isDateConversionSet() const
{
    return m_xBtnConvertDate->get_active();
}

void ScTextImportOptionsDlg::init()
{
    m_xBtnOk->connect_clicked(LINK(this, ScTextImportOptionsDlg, OKHdl));
    Link<weld::ToggleButton&,void> aLink = LINK(this, ScTextImportOptionsDlg, RadioHdl);
    m_xRbAutomatic->connect_toggled(aLink);
    m_xRbCustom->connect_toggled(aLink);

    m_xRbAutomatic->set_active(true);

    m_xLbCustomLang->SetLanguageList(
        SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false);

    LanguageType eLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    m_xLbCustomLang->set_active_id(eLang);
    m_xLbCustomLang->set_sensitive(false);
}

IMPL_LINK_NOARG(ScTextImportOptionsDlg, OKHdl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK(ScTextImportOptionsDlg, RadioHdl, weld::ToggleButton&, rBtn, void)
{
    if (&rBtn == m_xRbAutomatic.get())
    {
        m_xLbCustomLang->set_sensitive(false);
    }
    else if (&rBtn == m_xRbCustom.get())
    {
        m_xLbCustomLang->set_sensitive(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
