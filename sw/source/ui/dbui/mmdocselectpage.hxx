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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_MMDOCSELECTPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_MMDOCSELECTPAGE_HXX

#include <vcl/wizardmachine.hxx>
#include <vcl/weld.hxx>

class SwMailMergeWizard;

class SwMailMergeDocSelectPage : public vcl::OWizardPage
{
    OUString m_sLoadFileName;
    OUString m_sLoadTemplateName;

    SwMailMergeWizard* m_pWizard;

    std::unique_ptr<weld::RadioButton> m_xCurrentDocRB;
    std::unique_ptr<weld::RadioButton> m_xNewDocRB;
    std::unique_ptr<weld::RadioButton> m_xLoadDocRB;
    std::unique_ptr<weld::RadioButton> m_xLoadTemplateRB;
    std::unique_ptr<weld::RadioButton> m_xRecentDocRB;
    std::unique_ptr<weld::Button> m_xBrowseDocPB;
    std::unique_ptr<weld::Button> m_xBrowseTemplatePB;
    std::unique_ptr<weld::ComboBox> m_xRecentDocLB;
    std::unique_ptr<weld::Label> m_xDataSourceWarningFT;
    std::unique_ptr<weld::Button> m_xExchangeDatabasePB;

    DECL_LINK(DocSelectHdl, weld::ToggleButton&, void);
    DECL_LINK(FileSelectHdl, weld::Button&, void);
    DECL_LINK(ExchangeDatabaseHdl, weld::Button&, void);

    virtual bool commitPage(::vcl::WizardTypes::CommitPageReason _eReason) override;

public:
    SwMailMergeDocSelectPage(weld::Container* pPage, SwMailMergeWizard* pWizard);
    virtual ~SwMailMergeDocSelectPage() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
