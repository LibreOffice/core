/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmgreetingspage.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:05:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _MAILMERGEGREETINGSPAGE_HXX
#define _MAILMERGEGREETINGSPAGE_HXX

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _MAILMERGEHELPER_HXX
#include <mailmergehelper.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

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
    virtual sal_Bool    commitPage( CommitPageReason _eReason );
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


