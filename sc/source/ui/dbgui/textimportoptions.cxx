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
    : GenericDialogController(pParent, u"modules/scalc/ui/textimportoptions.ui"_ustr, u"TextImportOptionsDialog"_ustr)
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xRbAutomatic(m_xBuilder->weld_radio_button(u"automatic"_ustr))
    , m_xRbCustom(m_xBuilder->weld_radio_button(u"custom"_ustr))
    , m_xCkbConvertDate(m_xBuilder->weld_check_button(u"convertdata"_ustr))
    , m_xCkbConvertScientific(m_xBuilder->weld_check_button(u"convertscientificnotation"_ustr))
    , m_xCkbKeepAsking(m_xBuilder->weld_check_button(u"keepasking"_ustr))
    , m_xLbCustomLang(new SvxLanguageBox(m_xBuilder->weld_combo_box(u"lang"_ustr)))
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
    return m_xCkbConvertDate->get_active();
}

bool ScTextImportOptionsDlg::isScientificConversionSet() const
{
    return m_xCkbConvertScientific->get_active();
}

bool ScTextImportOptionsDlg::isKeepAskingSet() const
{
    return m_xCkbKeepAsking->get_active();
}

void ScTextImportOptionsDlg::init()
{
    m_xBtnOk->connect_clicked(LINK(this, ScTextImportOptionsDlg, OKHdl));
    Link<weld::Toggleable&,void> aLink = LINK(this, ScTextImportOptionsDlg, RadioCheckHdl);
    m_xRbAutomatic->connect_toggled(aLink);
    m_xRbCustom->connect_toggled(aLink);
    m_xCkbConvertDate->connect_toggled(aLink);
    m_xCkbConvertScientific->connect_toggled(aLink);

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

IMPL_LINK(ScTextImportOptionsDlg, RadioCheckHdl, weld::Toggleable&, rBtn, void)
{
    if (&rBtn == m_xRbAutomatic.get())
    {
        m_xLbCustomLang->set_sensitive(false);
    }
    else if (&rBtn == m_xRbCustom.get())
    {
        m_xLbCustomLang->set_sensitive(true);
    }
    else if (&rBtn == m_xCkbConvertDate.get())
    {
        if (m_xCkbConvertDate->get_active())
        {
            m_xCkbConvertScientific->set_active(true);
            m_xCkbConvertScientific->set_sensitive(false);
        }
        else
        {
            m_xCkbConvertScientific->set_sensitive(true);
        }
    }
    else if (&rBtn == m_xCkbConvertScientific.get())
    {
        assert( !m_xCkbConvertDate->get_active() && "ScTextImportOptionsDlg::RadioCheckHdl - scientific option disabled if Detect numbers active" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
