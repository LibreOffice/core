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

} // namespace desktop

#endif // #ifndef _PAGES_HXX_

