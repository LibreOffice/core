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
