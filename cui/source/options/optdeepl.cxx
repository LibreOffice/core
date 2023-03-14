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
#include <officecfg/Office/Linguistic.hxx>

OptDeeplTabPage::OptDeeplTabPage(weld::Container* pPage, weld::DialogController* pController,
                                 const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optdeeplpage.ui", "OptDeeplPage", &rSet)
    , m_xAPIUrl(m_xBuilder->weld_entry("apiurl"))
    , m_xAuthKey(m_xBuilder->weld_entry("authkey"))
{
}

OptDeeplTabPage::~OptDeeplTabPage() {}

void OptDeeplTabPage::Reset(const SfxItemSet*)
{
    m_xAPIUrl->set_text(*officecfg::Office::Linguistic::Translation::Deepl::ApiURL::get());
    m_xAuthKey->set_text(*officecfg::Office::Linguistic::Translation::Deepl::AuthKey::get());
}

bool OptDeeplTabPage::FillItemSet(SfxItemSet*)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Linguistic::Translation::Deepl::ApiURL::set(m_xAPIUrl->get_text(), batch);
    officecfg::Office::Linguistic::Translation::Deepl::AuthKey::set(m_xAuthKey->get_text(), batch);
    batch->commit();
    return false;
}

std::unique_ptr<SfxTabPage> OptDeeplTabPage::Create(weld::Container* pPage,
                                                    weld::DialogController* pController,
                                                    const SfxItemSet* rAttrSet)
{
    return std::make_unique<OptDeeplTabPage>(pPage, pController, *rAttrSet);
}
