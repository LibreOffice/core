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

#include "textimportoptions.hxx"
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>

ScTextImportOptionsDlg::ScTextImportOptionsDlg(vcl::Window* pParent)
    : ModalDialog(pParent, "TextImportOptionsDialog", "modules/scalc/ui/textimportoptions.ui")
{
    get(m_pBtnOk, "ok");
    get(m_pRbAutomatic, "automatic");
    get(m_pRbCustom, "custom");
    get(m_pBtnConvertDate, "convertdata");
    get(m_pLbCustomLang, "lang");
    init();
}

ScTextImportOptionsDlg::~ScTextImportOptionsDlg()
{
    disposeOnce();
}

void ScTextImportOptionsDlg::dispose()
{
    m_pBtnOk.clear();
    m_pRbAutomatic.clear();
    m_pRbCustom.clear();
    m_pLbCustomLang.clear();
    m_pBtnConvertDate.clear();
    ModalDialog::dispose();
}


short ScTextImportOptionsDlg::Execute()
{
    return ModalDialog::Execute();
}

LanguageType ScTextImportOptionsDlg::getLanguageType() const
{
    if (m_pRbAutomatic->IsChecked())
        return LANGUAGE_SYSTEM;

    return m_pLbCustomLang->GetSelectLanguage();
}

bool ScTextImportOptionsDlg::isDateConversionSet() const
{
    return m_pBtnConvertDate->IsChecked();
}

void ScTextImportOptionsDlg::init()
{
    Link<Button*,void> aLink = LINK( this, ScTextImportOptionsDlg, OKHdl );
    m_pBtnOk->SetClickHdl(aLink);
    aLink = LINK( this, ScTextImportOptionsDlg, RadioHdl );
    m_pRbAutomatic->SetClickHdl(aLink);
    m_pRbCustom->SetClickHdl(aLink);

    m_pRbAutomatic->Check();

    m_pLbCustomLang->SetLanguageList(
        SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false);

    LanguageType eLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    m_pLbCustomLang->SelectLanguage(eLang);
    m_pLbCustomLang->Disable();
}

IMPL_LINK_NOARG_TYPED(ScTextImportOptionsDlg, OKHdl, Button*, void)
{
    EndDialog(RET_OK);
}

IMPL_LINK_TYPED( ScTextImportOptionsDlg, RadioHdl, Button*, pBtn, void )
{
    if (pBtn == m_pRbAutomatic)
    {
        m_pLbCustomLang->Disable();
    }
    else if (pBtn == m_pRbCustom)
    {
        m_pLbCustomLang->Enable();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
