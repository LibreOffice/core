/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

