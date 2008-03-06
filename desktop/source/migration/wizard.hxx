/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wizard.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:50:06 $
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
