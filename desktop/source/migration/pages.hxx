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

#ifndef _PAGES_HXX_
#define _PAGES_HXX_

#include <vcl/tabpage.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/throbber.hxx>
#include <svtools/wizardmachine.hxx>
#include <svtools/svmedit.hxx>
#include <svl/lstner.hxx>
#include <svtools/xtextedt.hxx>

namespace desktop
{
class WelcomePage : public svt::OWizardPage
{
private:
    FixedText m_ftHead;
    FixedText m_ftBody;
    svt::OWizardMachine *m_pParent;
    sal_Bool m_bLicenseNeedsAcceptance;
    enum OEMType
    {
        OEM_NONE, OEM_NORMAL, OEM_EXTENDED
    };
    bool bIsEvalVersion;
    bool bNoEvalText;
    void checkEval();


public:
    WelcomePage( svt::OWizardMachine* parent, const ResId& resid, sal_Bool bLicenseNeedsAcceptance );
protected:
    virtual void ActivatePage();
};

class LicenseView : public MultiLineEdit, public SfxListener
{
    sal_Bool            mbEndReached;
    Link            maEndReachedHdl;
    Link            maScrolledHdl;

public:
    LicenseView( Window* pParent, const ResId& rResId );
    ~LicenseView();

    void ScrollDown( ScrollType eScroll );

    sal_Bool IsEndReached() const;
    sal_Bool EndReached() const { return mbEndReached; }
    void SetEndReached( sal_Bool bEnd ) { mbEndReached = bEnd; }

    void SetEndReachedHdl( const Link& rHdl )  { maEndReachedHdl = rHdl; }
    const Link& GetAutocompleteHdl() const { return maEndReachedHdl; }

    void SetScrolledHdl( const Link& rHdl )  { maScrolledHdl = rHdl; }
    const Link& GetScrolledHdl() const { return maScrolledHdl; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

protected:
    using MultiLineEdit::Notify;
};

class LicensePage : public svt::OWizardPage
{
private:
    svt::OWizardMachine *m_pParent;
    FixedText m_ftHead;
    FixedText m_ftBody1;
    FixedText m_ftBody1Txt;
    FixedText m_ftBody2;
    FixedText m_ftBody2Txt;
    LicenseView m_mlLicense;
    PushButton m_pbDown;
    sal_Bool m_bLicenseRead;
public:
    LicensePage( svt::OWizardMachine* parent, const ResId& resid, const rtl::OUString &rLicensePath );
private:
    DECL_LINK(PageDownHdl, PushButton*);
    DECL_LINK(EndReachedHdl, LicenseView*);
    DECL_LINK(ScrolledHdl, LicenseView*);
protected:
    virtual bool canAdvance() const;
    virtual void ActivatePage();
};

class MigrationPage : public svt::OWizardPage
{
private:
    FixedText m_ftHead;
    FixedText m_ftBody;
    CheckBox  m_cbMigration;
    Throbber& m_rThrobber;
    sal_Bool m_bMigrationDone;
public:
    MigrationPage( svt::OWizardMachine* parent, const ResId& resid, Throbber& i_throbber );
    virtual sal_Bool commitPage( svt::WizardTypes::CommitPageReason _eReason );

protected:
    virtual void ActivatePage();
};

class UserPage : public svt::OWizardPage
{
private:
    FixedText m_ftHead;
    FixedText m_ftBody;
    FixedText m_ftFirst;
    Edit m_edFirst;
    FixedText m_ftLast;
    Edit m_edLast;
    FixedText m_ftInitials;
    Edit m_edInitials;
    FixedText m_ftFather;
    Edit m_edFather;
    LanguageType m_lang;

public:
    UserPage( svt::OWizardMachine* parent, const ResId& resid);
    virtual sal_Bool commitPage( svt::WizardTypes::CommitPageReason _eReason );
protected:
    virtual void ActivatePage();
};

class UpdateCheckPage : public svt::OWizardPage
{
private:
    FixedText m_ftHead;
    FixedText m_ftBody;
    CheckBox m_cbUpdateCheck;
public:
    UpdateCheckPage( svt::OWizardMachine* parent, const ResId& resid);
    virtual sal_Bool commitPage( svt::WizardTypes::CommitPageReason _eReason );

protected:
    virtual void ActivatePage();
};


class RegistrationPage : public svt::OWizardPage
{
private:
    FixedText   m_ftHeader;
    FixedText   m_ftBody;
    RadioButton m_rbNow;
    RadioButton m_rbLater;
    RadioButton m_rbNever;
    FixedLine   m_flSeparator;
    FixedText   m_ftEnd;

    sal_Bool    m_bNeverVisible;

    void updateButtonStates();
    void impl_retrieveConfigurationData();

protected:
    virtual bool canAdvance() const;
    virtual void ActivatePage();

    virtual sal_Bool commitPage( svt::WizardTypes::CommitPageReason _eReason );

public:
    RegistrationPage( Window* parent, const ResId& resid);

    enum RegistrationMode
    {
        rmNow,      // register now
        rmLater,    // register later
        rmNever     // register never
    };

    RegistrationMode    getRegistrationMode() const;
    void                prepareSingleMode();
    inline String       getSingleModeTitle() const { return m_ftHeader.GetText(); }

    static bool         hasReminderDateCome();
    static void         executeSingleMode();
};

} // namespace desktop

#endif // #ifndef _PAGES_HXX_

