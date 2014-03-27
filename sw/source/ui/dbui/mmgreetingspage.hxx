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
    CheckBox*           m_pGreetingLineCB;

    CheckBox*           m_pPersonalizedCB;

    FixedText*          m_pFemaleFT;
    ListBox*            m_pFemaleLB;
    PushButton*         m_pFemalePB;

    FixedText*          m_pMaleFT;
    ListBox*            m_pMaleLB;
    PushButton*         m_pMalePB;

    FixedText*          m_pFemaleFI;
    FixedText*          m_pFemaleColumnFT;
    ListBox*            m_pFemaleColumnLB;
    FixedText*          m_pFemaleFieldFT;
    ComboBox*           m_pFemaleFieldCB;

    FixedText*          m_pNeutralFT;
    ComboBox*           m_pNeutralCB;

    bool                m_bIsTabPage;

    SwMailMergeWizard*  m_pWizard;

    ~SwGreetingsHandler() {}

    DECL_LINK(IndividualHdl_Impl, void *);
    DECL_LINK(GreetingHdl_Impl, PushButton*);

    void    Contains(sal_Bool bContainsGreeting);
    virtual void    UpdatePreview();
};

class SwMailMergeGreetingsPage : public svt::OWizardPage,
                                    public SwGreetingsHandler
{
    FixedText*          m_pPreviewFI;
    SwAddressPreview*   m_pPreviewWIN;
    PushButton*         m_pAssignPB;
    FixedText*          m_pDocumentIndexFI;
    PushButton*         m_pPrevSetIB;
    PushButton*         m_pNextSetIB;

    OUString            m_sDocument;

    DECL_LINK(ContainsHdl_Impl, CheckBox*);
    DECL_LINK(InsertDataHdl_Impl, ImageButton*);
    DECL_LINK(GreetingSelectHdl_Impl, void *);
    DECL_LINK(AssignHdl_Impl, PushButton*);

    virtual void        UpdatePreview() SAL_OVERRIDE;
    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;
public:
        SwMailMergeGreetingsPage( SwMailMergeWizard* _pParent);
        ~SwMailMergeGreetingsPage();

};

class SwMailBodyDialog : public SfxModalDialog, public SwGreetingsHandler
{
    CheckBox            m_aGreetingLineCB;

    CheckBox            m_aPersonalizedCB;

    FixedText           m_aFemaleFT;
    ListBox             m_aFemaleLB;
    PushButton          m_aFemalePB;

    FixedText           m_aMaleFT;
    ListBox             m_aMaleLB;
    PushButton          m_aMalePB;

    FixedInfo           m_aFemaleFI;
    FixedText           m_aFemaleColumnFT;
    ListBox             m_aFemaleColumnLB;
    FixedText           m_aFemaleFieldFT;
    ComboBox            m_aFemaleFieldCB;

    FixedText           m_aNeutralFT;
    ComboBox            m_aNeutralCB;

    FixedText           m_aBodyFT;
    MultiLineEdit       m_aBodyMLE;
    FixedLine           m_aSeparatorFL;

    OKButton            m_aOK;
    CancelButton        m_aCancel;
    HelpButton          m_aHelp;

    DECL_LINK(ContainsHdl_Impl, CheckBox*);
    DECL_LINK(OKHdl, void *);
public:
    SwMailBodyDialog(Window* pParent, SwMailMergeWizard* pWizard);
    ~SwMailBodyDialog();

    void            SetBody(const OUString& rBody ) {m_aBodyMLE.SetText(rBody);}
    OUString        GetBody() const {return m_aBodyMLE.GetText();}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
