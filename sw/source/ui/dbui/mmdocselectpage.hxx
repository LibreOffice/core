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

#include <svtools/wizardmachine.hxx>
#include <mailmergehelper.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
class SwMailMergeWizard;

class SwMailMergeDocSelectPage : public svt::OWizardPage
{
    VclPtr<RadioButton>        m_pCurrentDocRB;
    VclPtr<RadioButton>        m_pNewDocRB;
    VclPtr<RadioButton>        m_pLoadDocRB;
    VclPtr<RadioButton>        m_pLoadTemplateRB;
    VclPtr<RadioButton>        m_pRecentDocRB;

    VclPtr<PushButton>         m_pBrowseDocPB;
    VclPtr<PushButton>         m_pBrowseTemplatePB;

    VclPtr<ListBox>            m_pRecentDocLB;

    OUString            m_sLoadFileName;
    OUString            m_sLoadTemplateName;

    VclPtr<SwMailMergeWizard>  m_pWizard;

    DECL_LINK_TYPED(DocSelectHdl, Button*, void);
    DECL_LINK_TYPED(FileSelectHdl, Button*, void);

    virtual bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;

public:
        SwMailMergeDocSelectPage( SwMailMergeWizard* _pParent);
        virtual ~SwMailMergeDocSelectPage();
    virtual void dispose() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
