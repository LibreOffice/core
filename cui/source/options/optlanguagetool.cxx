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

#include "optlanguagetool.hxx"
#include <officecfg/Office/Linguistic.hxx>
#include <sal/log.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

using LanguageToolCfg = officecfg::Office::Linguistic::GrammarChecking::LanguageTool;
constexpr OUStringLiteral LANGUAGETOOL_DEFAULT_URL = u"https://api.languagetool.org/v2";

OptLanguageToolTabPage::OptLanguageToolTabPage(weld::Container* pPage,
                                               weld::DialogController* pController,
                                               const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/langtoolconfigpage.ui", "OptLangToolPage", &rSet)
    , m_xBaseURLED(m_xBuilder->weld_entry("baseurl"))
    , m_xUsernameED(m_xBuilder->weld_entry("username"))
    , m_xApiKeyED(m_xBuilder->weld_entry("apikey"))
    , m_xRestProtocol(m_xBuilder->weld_entry("restprotocol"))
    , m_xActivateBox(m_xBuilder->weld_check_button("activate"))
    , m_xSSLDisableVerificationBox(m_xBuilder->weld_check_button("verifyssl"))
    , m_xApiSettingsFrame(m_xBuilder->weld_frame("apisettings"))
{
    m_xActivateBox->connect_toggled(LINK(this, OptLanguageToolTabPage, CheckHdl));
    EnableControls(LanguageToolCfg::IsEnabled::get());

    // tdf#150494 Set default values as placeholder text
    m_xBaseURLED->set_placeholder_text(CuiResId(RID_LANGUAGETOOL_LEAVE_EMPTY));
    m_xUsernameED->set_placeholder_text(CuiResId(RID_LANGUAGETOOL_LEAVE_EMPTY));
    m_xApiKeyED->set_placeholder_text(CuiResId(RID_LANGUAGETOOL_LEAVE_EMPTY));
    m_xRestProtocol->set_placeholder_text(CuiResId(RID_LANGUAGETOOL_REST_LEAVE_EMPTY));
}

OptLanguageToolTabPage::~OptLanguageToolTabPage() {}

void OptLanguageToolTabPage::EnableControls(bool bEnable)
{
    if (bEnable != LanguageToolCfg::IsEnabled::get())
    {
        auto batch(comphelper::ConfigurationChanges::create());
        LanguageToolCfg::IsEnabled::set(bEnable, batch);
        batch->commit();
    }
    m_xApiSettingsFrame->set_visible(bEnable);
    m_xActivateBox->set_active(bEnable);
    m_xSSLDisableVerificationBox->set_active(!LanguageToolCfg::SSLCertVerify::get());
}

IMPL_LINK_NOARG(OptLanguageToolTabPage, CheckHdl, weld::Toggleable&, void)
{
    EnableControls(m_xActivateBox->get_active());
}

void OptLanguageToolTabPage::Reset(const SfxItemSet*)
{
    // tdf#150494 If no URL has been set, use the default URL
    OUString aBaseURL = LanguageToolCfg::BaseURL::get().value_or("");
    if (aBaseURL.isEmpty())
        m_xBaseURLED->set_text(LANGUAGETOOL_DEFAULT_URL);
    else
        m_xBaseURLED->set_text(aBaseURL);

    m_xUsernameED->set_text(LanguageToolCfg::Username::get().value_or(""));
    m_xApiKeyED->set_text(LanguageToolCfg::ApiKey::get().value_or(""));
    m_xRestProtocol->set_text(LanguageToolCfg::RestProtocol::get().value_or(""));
    m_xSSLDisableVerificationBox->set_active(!LanguageToolCfg::SSLCertVerify::get());
}

bool OptLanguageToolTabPage::FillItemSet(SfxItemSet*)
{
    auto batch(comphelper::ConfigurationChanges::create());

    // tdf#150494 If no URL has been set, then save the default URL
    OUString aBaseURL = m_xBaseURLED->get_text();
    if (aBaseURL.isEmpty())
        LanguageToolCfg::BaseURL::set(LANGUAGETOOL_DEFAULT_URL, batch);
    else
        LanguageToolCfg::BaseURL::set(aBaseURL, batch);

    LanguageToolCfg::Username::set(m_xUsernameED->get_text(), batch);
    LanguageToolCfg::ApiKey::set(m_xApiKeyED->get_text(), batch);
    LanguageToolCfg::RestProtocol::set(m_xRestProtocol->get_text(), batch);
    LanguageToolCfg::SSLCertVerify::set(!m_xSSLDisableVerificationBox->get_active(), batch);
    batch->commit();
    return false;
}

std::unique_ptr<SfxTabPage> OptLanguageToolTabPage::Create(weld::Container* pPage,
                                                           weld::DialogController* pController,
                                                           const SfxItemSet* rAttrSet)
{
    return std::make_unique<OptLanguageToolTabPage>(pPage, pController, *rAttrSet);
}
