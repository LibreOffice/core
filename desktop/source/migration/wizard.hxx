/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wizard.hxx,v $
 * $Revision: 1.11 $
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

#ifndef _WIZARD_HXX_
#define _WIZARD_HXX_

#include <rtl/ustring.hxx>
#include <svtools/roadmapwizard.hxx>
#include <vcl/window.hxx>
#include <tools/resid.hxx>

namespace desktop
{

class WizardResId : public ResId
{
public:
    WizardResId( USHORT nId );
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

    void overrideCheck(sal_Bool bOverride);

private:
    sal_Bool m_bOverride;
    WizardState _currentState;
    ::svt::RoadmapWizardTypes::PathId m_aDefaultPath;
    ::svt::RoadmapWizardTypes::PathId m_aMigrationPath;
    String m_sNext;
    String m_sCancel;
    sal_Bool m_bDone;
    sal_Bool m_bLicenseNeedsAcceptance;
    sal_Bool m_bLicenseWasAccepted;
    sal_Bool m_bAutomaticUpdChk;
    Link m_lnkCancel;

    rtl::OUString m_aLicensePath;

    void storeAcceptDate();
    void disableWizard();
    void enableQuickstart();

    DECL_LINK(DeclineHdl, PushButton*);

    void cleanOldOfficeRegKeys();
    sal_Bool showOnlineUpdatePage();
    ::svt::RoadmapWizardTypes::PathId defineWizardPagesDependingFromContext();

protected:
    // from svt::WizardMachine
    virtual TabPage* createPage(WizardState _nState);
    virtual sal_Bool prepareLeaveCurrentState( CommitPageReason _eReason );
    virtual sal_Bool leaveState(WizardState _nState );
    virtual sal_Bool onFinish(sal_Int32 _nResult);
    virtual void     enterState(WizardState _nState);

    // from svt::RoadmapWizard
    virtual String       getStateDisplayName( WizardState _nState ) const;
};
}
#endif
