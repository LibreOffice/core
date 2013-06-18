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



#ifndef _WIZARD_HXX_
#define _WIZARD_HXX_

#include <rtl/ustring.hxx>
#include <svtools/roadmapwizard.hxx>
#include <vcl/throbber.hxx>
#include <tools/resid.hxx>

namespace desktop
{

class WizardResId : public ResId
{
public:
    WizardResId( sal_uInt16 nId );
};

class FirstStartWizard : public svt::RoadmapWizard
{

public:
    static const WizardState STATE_WELCOME;
    static const WizardState STATE_LICENSE;
    static const WizardState STATE_MIGRATION;
    static const WizardState STATE_USER;
    static const WizardState STATE_UPDATE_CHECK;
    static const WizardState STATE_REGISTRATION;

    static ResMgr* pResMgr;
    static ResMgr* GetResManager();

    FirstStartWizard( Window* pParent, sal_Bool bLicenseNeedsAcceptance, const rtl::OUString &rLicensePath );

    virtual short Execute();
    virtual long PreNotify( NotifyEvent& rNEvt );

    void DisableButtonsWhileMigration();

private:
    sal_Bool m_bOverride;
    WizardState _currentState;
    WizardState m_lastState;
    ::svt::RoadmapWizardTypes::PathId m_aDefaultPath;
    ::svt::RoadmapWizardTypes::PathId m_aMigrationPath;
    String m_sNext;
    String m_sCancel;
    sal_Bool m_bDone;
    sal_Bool m_bLicenseNeedsAcceptance;
    sal_Bool m_bLicenseWasAccepted;
    sal_Bool m_bAutomaticUpdChk;
    Link m_lnkCancel;
    Throbber  m_aThrobber;

    rtl::OUString m_aLicensePath;

    void storeAcceptDate();
    void setPatchLevel();

    DECL_LINK(DeclineHdl, PushButton*);

    void cleanOldOfficeRegKeys();
    sal_Bool showOnlineUpdatePage();
    ::svt::RoadmapWizardTypes::PathId defineWizardPagesDependingFromContext();

protected:
    // from svt::WizardMachine
    virtual TabPage* createPage(WizardState _nState);
    virtual sal_Bool prepareLeaveCurrentState( CommitPageReason _eReason );
    virtual sal_Bool leaveState(WizardState _nState );
    virtual sal_Bool onFinish();
    virtual void     enterState(WizardState _nState);

    // from svt::RoadmapWizard
    virtual String       getStateDisplayName( WizardState _nState ) const;
};
}
#endif
