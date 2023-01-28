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
#include <svtools/languagetoolcfg.hxx>
#include <sal/log.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

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
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    EnableControls(rLanguageOpts.getEnabled());

    // tdf#150494 Set default values as placeholder text
    m_xBaseURLED->set_placeholder_text(CuiResId(RID_LANGUAGETOOL_LEAVE_EMPTY));
    m_xUsernameED->set_placeholder_text(CuiResId(RID_LANGUAGETOOL_LEAVE_EMPTY));
    m_xApiKeyED->set_placeholder_text(CuiResId(RID_LANGUAGETOOL_LEAVE_EMPTY));
}

OptLanguageToolTabPage::~OptLanguageToolTabPage() {}

void OptLanguageToolTabPage::EnableControls(bool bEnable)
{
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    rLanguageOpts.setEnabled(bEnable);
    m_xApiSettingsFrame->set_visible(bEnable);
    m_xActivateBox->set_active(bEnable);
    m_xSSLDisableVerificationBox->set_active(rLanguageOpts.getSSLVerification() != true);
}

IMPL_LINK_NOARG(OptLanguageToolTabPage, CheckHdl, weld::Toggleable&, void)
{
    EnableControls(m_xActivateBox->get_active());
}

void OptLanguageToolTabPage::Reset(const SfxItemSet*)
{
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();

    // tdf#150494 If no URL has been set, use the default URL
    OUString aBaseURL = rLanguageOpts.getBaseURL();
    if (aBaseURL.isEmpty())
        m_xBaseURLED->set_text(LANGUAGETOOL_DEFAULT_URL);
    else
        m_xBaseURLED->set_text(rLanguageOpts.getBaseURL());

    m_xUsernameED->set_text(rLanguageOpts.getUsername());
    m_xApiKeyED->set_text(rLanguageOpts.getApiKey());
    m_xRestProtocol->set_text(rLanguageOpts.getRestProtocol());
    m_xSSLDisableVerificationBox->set_active(rLanguageOpts.getSSLVerification() != true);
}

bool OptLanguageToolTabPage::FillItemSet(SfxItemSet*)
{
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();

    // tdf#150494 If no URL has been set, then save the default URL
    OUString aBaseURL = m_xBaseURLED->get_text();
    if (aBaseURL.isEmpty())
        rLanguageOpts.setBaseURL(LANGUAGETOOL_DEFAULT_URL);
    else
        rLanguageOpts.setBaseURL(aBaseURL);

    rLanguageOpts.setUsername(m_xUsernameED->get_text());
    rLanguageOpts.setApiKey(m_xApiKeyED->get_text());
    rLanguageOpts.setRestProtocol(m_xRestProtocol->get_text());
    rLanguageOpts.setSSLVerification(m_xSSLDisableVerificationBox->get_active() != true);
    return false;
}

std::unique_ptr<SfxTabPage> OptLanguageToolTabPage::Create(weld::Container* pPage,
                                                           weld::DialogController* pController,
                                                           const SfxItemSet* rAttrSet)
{
    return std::make_unique<OptLanguageToolTabPage>(pPage, pController, *rAttrSet);
}
