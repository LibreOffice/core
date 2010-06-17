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

#include "udlg_module.hxx"
#include "udlg_global.hrc"
#include "roadmapskeleton.hrc"
#include "roadmapskeleton.hxx"
#include "roadmapskeletonpages.hxx"

//........................................................................
namespace udlg
{
//........................................................................

#define STATE_WELCOME       0
#define STATE_PREPARE       1
#define STATE_SETUP         2
#define STATE_FINISH        3

#define PATH_DEFAULT        1

    //====================================================================
    //= RoadmapSkeletonDialog
    //====================================================================
    //--------------------------------------------------------------------
    RoadmapSkeletonDialog::RoadmapSkeletonDialog( const ::comphelper::ComponentContext& _rContext, Window* _pParent )
        :RoadmapSkeletonDialog_Base( _pParent, UdlgResId( DLG_ROADMAP_SKELETON ) )
        ,m_aContext( _rContext )
    {
        String sTitlePrepare( UdlgResId( STR_STATE_WELCOME ) );
        String sTitleStoreAs( UdlgResId( STR_STATE_PREPARE ) );
        String sTitleMigrate( UdlgResId( STR_STATE_SETUP ) );
        String sTitleSummary( UdlgResId( STR_STATE_FINISH ) );
        FreeResource();

        describeState( STATE_WELCOME,    sTitlePrepare, &WelcomePage::Create     );
        describeState( STATE_PREPARE,    sTitleStoreAs, &PreparationPage::Create );
        describeState( STATE_SETUP,      sTitleMigrate, &SetupPage::Create       );
        describeState( STATE_FINISH,     sTitleSummary, &FinishPage::Create      );

        declarePath( PATH_DEFAULT, STATE_WELCOME, STATE_PREPARE, STATE_SETUP, STATE_FINISH, WZS_INVALID_STATE );

        SetPageSizePixel( LogicToPixel( ::Size( TAB_PAGE_WIDTH, TAB_PAGE_HEIGHT ), MAP_APPFONT ) );
        ShowButtonFixedLine( true );
        SetRoadmapInteractive( true );
        enableAutomaticNextButtonState();
        defaultButton( WZB_NEXT );
        enableButtons( WZB_FINISH, true );
        ActivatePage();
    }

    //--------------------------------------------------------------------
    RoadmapSkeletonDialog::~RoadmapSkeletonDialog()
    {
    }

    //--------------------------------------------------------------------
    void RoadmapSkeletonDialog::enterState( WizardState _nState )
    {
        RoadmapSkeletonDialog_Base::enterState( _nState );
    }

    //--------------------------------------------------------------------
    sal_Bool RoadmapSkeletonDialog::prepareLeaveCurrentState( CommitPageReason _eReason )
    {
        return RoadmapSkeletonDialog_Base::prepareLeaveCurrentState( _eReason );
    }

    //--------------------------------------------------------------------
    sal_Bool RoadmapSkeletonDialog::leaveState( WizardState _nState )
    {
        return RoadmapSkeletonDialog_Base::leaveState( _nState );
    }

    //--------------------------------------------------------------------
    RoadmapSkeletonDialog::WizardState RoadmapSkeletonDialog::determineNextState( WizardState _nCurrentState ) const
    {
        return RoadmapSkeletonDialog_Base::determineNextState( _nCurrentState );
    }

    //--------------------------------------------------------------------
    sal_Bool RoadmapSkeletonDialog::onFinish()
    {
        return RoadmapSkeletonDialog_Base::onFinish();
    }

//........................................................................
} // namespace udlg
//........................................................................
