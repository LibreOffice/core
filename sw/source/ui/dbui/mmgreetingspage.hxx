/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _MAILMERGEGREETINGSPAGE_HXX
#define _MAILMERGEGREETINGSPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <sfx2/basedlgs.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <svtools/stdctrl.hxx>
#include <mailmergehelper.hxx>
#include <svtools/svmedit.hxx>

class SwMailMergeWizard;
/*-- 17.05.2004 14:51:45---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwGreetingsHandler
{
    friend class SwMailBodyDialog;
    friend class SwMailMergeGreetingsPage;
    CheckBox*           m_pGreetingLineCB;

    CheckBox*           m_pPersonalizedCB;

    FixedText*          m_pFemaleFT;
    ListBox*            m_pFemaleLB;
    PushButton*         m_pFemalePB;

    FixedText*          m_pMaleFT;
    ListBox*            m_pMaleLB;
    PushButton*         m_pMalePB;

    FixedInfo*          m_pFemaleFI;
    FixedText*          m_pFemaleColumnFT;
    ListBox*            m_pFemaleColumnLB;
    FixedText*          m_pFemaleFieldFT;
    ComboBox*           m_pFemaleFieldCB;

    FixedText*          m_pNeutralFT;
    ComboBox*           m_pNeutralCB;

    bool                m_bIsTabPage;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(IndividualHdl_Impl, CheckBox*);
    DECL_LINK(GreetingHdl_Impl, PushButton*);

    void    Contains(sal_Bool bContainsGreeting);
    virtual void    UpdatePreview();
};
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeGreetingsPage : public svt::OWizardPage,
                                    public SwGreetingsHandler
{
    SwBoldFixedInfo     m_aHeaderFI;

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

    FixedInfo           m_aPreviewFI;
    SwAddressPreview    m_aPreviewWIN;
    PushButton          m_aAssignPB;
    FixedInfo           m_aDocumentIndexFI;
    ImageButton         m_aPrevSetIB;
    ImageButton         m_aNextSetIB;

    String              m_sDocument;

    DECL_LINK(ContainsHdl_Impl, CheckBox*);
    DECL_LINK(InsertDataHdl_Impl, ImageButton*);
    DECL_LINK(GreetingSelectHdl_Impl, ListBox*);
    DECL_LINK(AssignHdl_Impl, PushButton*);

    virtual void    UpdatePreview();
    virtual void        ActivatePage();
    virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
public:
        SwMailMergeGreetingsPage( SwMailMergeWizard* _pParent);
        ~SwMailMergeGreetingsPage();

};
/*-- 17.05.2004 14:45:43---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    DECL_LINK(OKHdl, PushButton*);
public:
    SwMailBodyDialog(Window* pParent, SwMailMergeWizard* pWizard);
    ~SwMailBodyDialog();

    void            SetBody(const String& rBody ) {m_aBodyMLE.SetText(rBody);}
    String          GetBody() const {return m_aBodyMLE.GetText();}
};
#endif


