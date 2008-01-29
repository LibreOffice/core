/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pages.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:30:51 $
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

#ifndef _PAGES_HXX_
#define _PAGES_HXX_

#include <vcl/tabpage.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/scrbar.hxx>
#include <svtools/wizardmachine.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/lstner.hxx>
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
    OEMType checkOEM();
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
    BOOL            mbEndReached;
    Link            maEndReachedHdl;
    Link            maScrolledHdl;

public:
    LicenseView( Window* pParent, const ResId& rResId );
    ~LicenseView();

    void ScrollDown( ScrollType eScroll );

    BOOL IsEndReached() const;
    BOOL EndReached() const { return mbEndReached; }
    void SetEndReached( BOOL bEnd ) { mbEndReached = bEnd; }

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
    virtual sal_Bool determineNextButtonState();
    virtual void ActivatePage();
};

class MigrationPage : public svt::OWizardPage
{
private:
    FixedText m_ftHead;
    FixedText m_ftBody;
    CheckBox m_cbMigration;
    sal_Bool m_bMigrationDone;
public:
    MigrationPage( svt::OWizardMachine* parent, const ResId& resid);
    virtual sal_Bool commitPage(COMMIT_REASON _eReason);

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
    virtual sal_Bool commitPage(COMMIT_REASON _eReason);
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
    virtual sal_Bool commitPage(COMMIT_REASON _eReason);

protected:
    virtual void ActivatePage();
};


class RegistrationPage : public svt::OWizardPage
{
private:
    FixedText m_ftHeader;
    FixedText m_ftBody;
    FixedImage m_fiImage;
    RadioButton m_rbNow;
    RadioButton m_rbLater;
    RadioButton m_rbNever;
    RadioButton m_rbReg;
    FixedLine m_flSeparator;
    FixedText m_ftEnd;
    sal_Bool    m_bNeverVisible;

    void updateButtonStates();
    void impl_retrieveConfigurationData();

public:
    RegistrationPage( svt::OWizardMachine* parent, const ResId& resid);
    virtual sal_Bool commitPage(COMMIT_REASON _eReason);
protected:
    virtual sal_Bool determineNextButtonState();
    virtual void ActivatePage();
};

}
#endif
