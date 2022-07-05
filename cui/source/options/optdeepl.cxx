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

#include "optdeepl.hxx"
#include <svtools/deeplcfg.hxx>

OptDeeplTabPage::OptDeeplTabPage(weld::Container* pPage,
                                               weld::DialogController* pController,
                                               const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/deepltabpage.ui", "OptDeeplPage", &rSet)
    , m_xAPIUrl(m_xBuilder->weld_entry("apiurl"))
    , m_xAuthKey(m_xBuilder->weld_entry("authkey"))
{

}

OptDeeplTabPage::~OptDeeplTabPage() {}

void OptDeeplTabPage::Reset(const SfxItemSet*)
{
    SvxDeeplOptions& rDeeplOptions = SvxDeeplOptions::Get();
    m_xAPIUrl->set_text(rDeeplOptions.getAPIUrl());
    m_xAuthKey->set_text(rDeeplOptions.getAuthKey());
}

bool OptDeeplTabPage::FillItemSet(SfxItemSet*)
{
    SvxDeeplOptions& rDeeplOptions = SvxDeeplOptions::Get();
    rDeeplOptions.setAPIUrl(m_xAPIUrl->get_text());
    rDeeplOptions.setAuthKey(m_xAuthKey->get_text());
    return false;
}

std::unique_ptr<SfxTabPage> OptDeeplTabPage::Create(weld::Container* pPage,
                                                           weld::DialogController* pController,
                                                           const SfxItemSet* rAttrSet)
{
    return std::make_unique<OptDeeplTabPage>(pPage, pController, *rAttrSet);
}
