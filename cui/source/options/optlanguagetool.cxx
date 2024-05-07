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
constexpr OUString LANGUAGETOOL_DEFAULT_URL = u"https://api.languagetool.org/v2"_ustr;
constexpr OUString LANGUAGETOOLPLUS_DEFAULT_URL = u"https://api.languagetoolplus.com/v2"_ustr;

OptLanguageToolTabPage::OptLanguageToolTabPage(weld::Container* pPage,
                                               weld::DialogController* pController,
                                               const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/langtoolconfigpage.ui"_ustr, u"OptLangToolPage"_ustr,
                 &rSet)
    , m_xBaseURLED(m_xBuilder->weld_entry(u"baseurl"_ustr))
    , m_xBaseURLImg(m_xBuilder->weld_widget(u"lockbaseurl"_ustr))
    , m_xUsernameED(m_xBuilder->weld_entry(u"username"_ustr))
    , m_xUsernameImg(m_xBuilder->weld_widget(u"lockusername"_ustr))
    , m_xApiKeyED(m_xBuilder->weld_entry(u"apikey"_ustr))
    , m_xApiKeyImg(m_xBuilder->weld_widget(u"lockapikey"_ustr))
    , m_xRestProtocol(m_xBuilder->weld_entry(u"restprotocol"_ustr))
    , m_xRestProtocolImg(m_xBuilder->weld_widget(u"lockrestprotocol"_ustr))
    , m_xActivateBox(m_xBuilder->weld_check_button(u"activate"_ustr))
    , m_xActivateBoxImg(m_xBuilder->weld_widget(u"lockactivate"_ustr))
    , m_xSSLDisableVerificationBox(m_xBuilder->weld_check_button(u"verifyssl"_ustr))
    , m_xSSLDisableVerificationBoxImg(m_xBuilder->weld_widget(u"lockverifyssl"_ustr))
    , m_xApiSettingsFrame(m_xBuilder->weld_frame(u"apisettings"_ustr))
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
    m_xActivateBox->set_sensitive(!LanguageToolCfg::IsEnabled::isReadOnly());
    m_xActivateBoxImg->set_visible(LanguageToolCfg::IsEnabled::isReadOnly());
    m_xSSLDisableVerificationBox->set_active(!LanguageToolCfg::SSLCertVerify::get());
    m_xSSLDisableVerificationBox->set_sensitive(!LanguageToolCfg::SSLCertVerify::isReadOnly());
    m_xSSLDisableVerificationBoxImg->set_visible(LanguageToolCfg::SSLCertVerify::isReadOnly());
}

IMPL_LINK_NOARG(OptLanguageToolTabPage, CheckHdl, weld::Toggleable&, void)
{
    EnableControls(m_xActivateBox->get_active());
}

void OptLanguageToolTabPage::Reset(const SfxItemSet*)
{
    // tdf#150494 If no URL has been set, use the default URL
    OUString aUsername = LanguageToolCfg::Username::get().value_or("");
    OUString aApiKey = LanguageToolCfg::ApiKey::get().value_or("");
    OUString aBaseURL = LanguageToolCfg::BaseURL::get().value_or(
        (aUsername.isEmpty() && aApiKey.isEmpty()) ? LANGUAGETOOL_DEFAULT_URL
                                                   : LANGUAGETOOLPLUS_DEFAULT_URL);

    m_xBaseURLED->set_text(aBaseURL);
    m_xBaseURLED->set_sensitive(!LanguageToolCfg::BaseURL::isReadOnly());
    m_xBaseURLImg->set_visible(LanguageToolCfg::BaseURL::isReadOnly());

    m_xUsernameED->set_text(aUsername);
    m_xUsernameED->set_sensitive(!LanguageToolCfg::Username::isReadOnly());
    m_xUsernameImg->set_visible(LanguageToolCfg::Username::isReadOnly());

    m_xApiKeyED->set_text(aApiKey);
    m_xApiKeyED->set_sensitive(!LanguageToolCfg::ApiKey::isReadOnly());
    m_xApiKeyImg->set_visible(LanguageToolCfg::ApiKey::isReadOnly());

    m_xRestProtocol->set_text(LanguageToolCfg::RestProtocol::get().value_or(""));
    m_xRestProtocol->set_sensitive(!LanguageToolCfg::RestProtocol::isReadOnly());
    m_xRestProtocolImg->set_visible(LanguageToolCfg::RestProtocol::isReadOnly());

    m_xSSLDisableVerificationBox->set_active(!LanguageToolCfg::SSLCertVerify::get());
    m_xSSLDisableVerificationBox->set_sensitive(!LanguageToolCfg::SSLCertVerify::isReadOnly());
    m_xSSLDisableVerificationBoxImg->set_visible(LanguageToolCfg::SSLCertVerify::isReadOnly());
}

OUString OptLanguageToolTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = {
        u"langtoolsettings"_ustr, u"disclaimer"_ustr,  u"apisettingsheader"_ustr, u"base"_ustr,
        u"urldesc"_ustr,          u"usernamelbl"_ustr, u"usernamedesc"_ustr,      u"apikeylbl"_ustr,
        u"apikeydesc"_ustr,       u"restlbl"_ustr,     u"restdesc"_ustr
    };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"activate"_ustr, u"verifyssl"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    if (const auto& pString = m_xBuilder->weld_link_button(u"policy"_ustr))
        sAllStrings += pString->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool OptLanguageToolTabPage::FillItemSet(SfxItemSet*)
{
    auto batch(comphelper::ConfigurationChanges::create());

    // tdf#150494 If no URL has been set, then save the default URL
    // tdf#159395 If Username and ApiKey are set, then save the default URL for paid service
    OUString aBaseURL = m_xBaseURLED->get_text();
    OUString aUsername = m_xUsernameED->get_text();
    OUString aApiKey = m_xApiKeyED->get_text();

    if (aBaseURL.isEmpty()
        || (aUsername.isEmpty() && aApiKey.isEmpty() && aBaseURL == LANGUAGETOOLPLUS_DEFAULT_URL))
        aBaseURL = LANGUAGETOOL_DEFAULT_URL;

    if (!aUsername.isEmpty() && !aApiKey.isEmpty() && aBaseURL == LANGUAGETOOL_DEFAULT_URL)
        aBaseURL = LANGUAGETOOLPLUS_DEFAULT_URL;

    LanguageToolCfg::BaseURL::set(aBaseURL, batch);
    LanguageToolCfg::Username::set(aUsername, batch);
    LanguageToolCfg::ApiKey::set(aApiKey, batch);
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
