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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_MMGREETINGSPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_MMGREETINGSPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <svtools/stdctrl.hxx>
#include <mailmergehelper.hxx>
#include <svtools/svmedit.hxx>

class SwMailMergeWizard;

class SwGreetingsHandler
{
protected:
    VclPtr<CheckBox>           m_pGreetingLineCB;

    VclPtr<CheckBox>           m_pPersonalizedCB;

    VclPtr<FixedText>          m_pFemaleFT;
    VclPtr<ListBox>            m_pFemaleLB;
    VclPtr<PushButton>         m_pFemalePB;

    VclPtr<FixedText>          m_pMaleFT;
    VclPtr<ListBox>            m_pMaleLB;
    VclPtr<PushButton>         m_pMalePB;

    VclPtr<FixedText>          m_pFemaleFI;
    VclPtr<FixedText>          m_pFemaleColumnFT;
    VclPtr<ListBox>            m_pFemaleColumnLB;
    VclPtr<FixedText>          m_pFemaleFieldFT;
    VclPtr<ComboBox>           m_pFemaleFieldCB;

    VclPtr<FixedText>          m_pNeutralFT;
    VclPtr<ComboBox>           m_pNeutralCB;

    bool                m_bIsTabPage;

    VclPtr<SwMailMergeWizard>  m_pWizard;

    ~SwGreetingsHandler() {}

    DECL_LINK_TYPED(IndividualHdl_Impl, Button*, void);
    DECL_LINK_TYPED(GreetingHdl_Impl, Button*, void);

    void    Contains(bool bContainsGreeting);
    virtual void    UpdatePreview();
};

class SwMailMergeGreetingsPage : public svt::OWizardPage,
                                    public SwGreetingsHandler
{
    VclPtr<FixedText>          m_pPreviewFI;
    VclPtr<SwAddressPreview>   m_pPreviewWIN;
    VclPtr<PushButton>         m_pAssignPB;
    VclPtr<FixedText>          m_pDocumentIndexFI;
    VclPtr<PushButton>         m_pPrevSetIB;
    VclPtr<PushButton>         m_pNextSetIB;

    OUString            m_sDocument;

    DECL_LINK_TYPED(ContainsHdl_Impl, Button*, void);
    DECL_LINK_TYPED(InsertDataHdl_Impl, Button*, void);
    DECL_LINK(GreetingSelectHdl_Impl, void *);
    DECL_LINK_TYPED(GreetingSelectListBoxHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(AssignHdl_Impl, Button*, void);

    virtual void        UpdatePreview() override;
    virtual void        ActivatePage() override;
    virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;
public:
        SwMailMergeGreetingsPage( SwMailMergeWizard* _pParent);
        virtual ~SwMailMergeGreetingsPage();
    virtual void dispose() override;

};

class SwMailBodyDialog : public SfxModalDialog, public SwGreetingsHandler
{
    VclPtr<FixedText>           m_pBodyFT;
    VclPtr<VclMultiLineEdit>    m_pBodyMLE;

    VclPtr<OKButton>            m_pOK;

    DECL_LINK_TYPED(ContainsHdl_Impl, Button*, void);
    DECL_LINK_TYPED(OKHdl, Button*, void);
public:
    SwMailBodyDialog(vcl::Window* pParent, SwMailMergeWizard* pWizard);
    virtual ~SwMailBodyDialog();
    virtual void dispose() override;

    void            SetBody(const OUString& rBody ) {m_pBodyMLE->SetText(rBody);}
    OUString        GetBody() const {return m_pBodyMLE->GetText();}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
