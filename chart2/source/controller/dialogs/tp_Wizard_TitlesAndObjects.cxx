/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "tp_Wizard_TitlesAndObjects.hxx"
#include "tp_Wizard_TitlesAndObjects.hrc"
#include "Strings.hrc"
#include "res_Titles.hxx"
#include "res_LegendPosition.hxx"
#include "ResId.hxx"
#include "HelpIds.hrc"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "LegendHelper.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "ControllerLockGuard.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;


TitlesAndObjectsTabPage::TitlesAndObjectsTabPage( svt::OWizardMachine* pParent
        , const uno::Reference< XChartDocument >& xChartModel
        , const uno::Reference< uno::XComponentContext >& xContext )
        : OWizardPage( pParent, SchResId(TP_WIZARD_TITLEANDOBJECTS) )
        , m_aFT_TitleDescription( this, SchResId( FT_TITLEDESCRIPTION ) )
        , m_aFL_Vertical( this, SchResId( FL_VERTICAL ) )
        , m_apTitleResources( new TitleResources(this,false) )
        , m_apLegendPositionResources( new LegendPositionResources(this,xContext) )
        , m_aFL_Grids( this, SchResId( FL_GRIDS ) )
        , m_aCB_Grid_X( this, SchResId( CB_X_SECONDARY ) )
        , m_aCB_Grid_Y( this, SchResId( CB_Y_SECONDARY ) )
        , m_aCB_Grid_Z( this, SchResId( CB_Z_SECONDARY ) )
        , m_xChartModel( xChartModel )
        , m_xCC( xContext )
        , m_bCommitToModel( true )
        , m_aTimerTriggeredControllerLock( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY ) )
{
    FreeResource();

    this->SetText( SCH_RESSTR( STR_PAGE_CHART_ELEMENTS ) );

    Font aFont( m_aFT_TitleDescription.GetControlFont() );
    aFont.SetWeight( WEIGHT_BOLD );
    m_aFT_TitleDescription.SetControlFont( aFont );

    m_aCB_Grid_X.SetHelpId( HID_SCH_CB_XGRID );
    m_aCB_Grid_Y.SetHelpId( HID_SCH_CB_YGRID );
    m_aCB_Grid_Z.SetHelpId( HID_SCH_CB_ZGRID );

    m_apTitleResources->SetUpdateDataHdl( LINK( this, TitlesAndObjectsTabPage, ChangeHdl ));
    m_apLegendPositionResources->SetChangeHdl( LINK( this, TitlesAndObjectsTabPage, ChangeHdl ));

    m_aCB_Grid_X.SetToggleHdl( LINK( this, TitlesAndObjectsTabPage, ChangeHdl ));
    m_aCB_Grid_Y.SetToggleHdl( LINK( this, TitlesAndObjectsTabPage, ChangeHdl ));
    m_aCB_Grid_Z.SetToggleHdl( LINK( this, TitlesAndObjectsTabPage, ChangeHdl ));
}

TitlesAndObjectsTabPage::~TitlesAndObjectsTabPage()
{
}

void TitlesAndObjectsTabPage::initializePage()
{
    m_bCommitToModel = false;

    //init titles
    {
        TitleDialogData aTitleInput;
        aTitleInput.readFromModel( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY) );
        m_apTitleResources->writeToResources( aTitleInput );
    }

    //init legend
    {
        m_apLegendPositionResources->writeToResources( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY) );
    }

    //init grid checkboxes
    {
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( m_xChartModel );
        uno::Sequence< sal_Bool > aPossibilityList;
        uno::Sequence< sal_Bool > aExistenceList;
        AxisHelper::getAxisOrGridPossibilities( aPossibilityList, xDiagram, sal_False );
        AxisHelper::getAxisOrGridExcistence( aExistenceList, xDiagram, sal_False );
        m_aCB_Grid_X.Enable( aPossibilityList[0] );
        m_aCB_Grid_Y.Enable( aPossibilityList[1] );
        m_aCB_Grid_Z.Enable( aPossibilityList[2] );
        m_aCB_Grid_X.Check( aExistenceList[0] );
        m_aCB_Grid_Y.Check( aExistenceList[1] );
        m_aCB_Grid_Z.Check( aExistenceList[2] );
    }

    m_bCommitToModel = true;
}

sal_Bool TitlesAndObjectsTabPage::commitPage( ::svt::WizardTypes::CommitPageReason /*eReason*/ )
{
    if( m_apTitleResources->IsModified() ) //titles may have changed in the meanwhile
        commitToModel();
    return sal_True;//return false if this page should not be left
}

void TitlesAndObjectsTabPage::commitToModel()
{
    m_aTimerTriggeredControllerLock.startTimer();
    uno::Reference< frame::XModel >  xModel( m_xChartModel, uno::UNO_QUERY);

    ControllerLockGuard aLockedControllers( xModel );

    //commit title changes to model
    {
        TitleDialogData aTitleOutput;
        m_apTitleResources->readFromResources( aTitleOutput );
        aTitleOutput.writeDifferenceToModel( xModel, m_xCC );
        m_apTitleResources->ClearModifyFlag();
    }

    //commit legend changes to model
    {
        m_apLegendPositionResources->writeToModel( xModel );
    }

    //commit grid changes to model
    {
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( xModel );
        uno::Sequence< sal_Bool > aOldExistenceList;
        AxisHelper::getAxisOrGridExcistence( aOldExistenceList, xDiagram, sal_False );
        uno::Sequence< sal_Bool > aNewExistenceList(aOldExistenceList);
        aNewExistenceList[0]=m_aCB_Grid_X.IsChecked();
        aNewExistenceList[1]=m_aCB_Grid_Y.IsChecked();
        aNewExistenceList[2]=m_aCB_Grid_Z.IsChecked();
        AxisHelper::changeVisibilityOfGrids( xDiagram
                , aOldExistenceList, aNewExistenceList, m_xCC );
    }
}

IMPL_LINK_NOARG(TitlesAndObjectsTabPage, ChangeHdl)
{
    if( m_bCommitToModel )
        commitToModel();
    return 0;
}

bool TitlesAndObjectsTabPage::canAdvance() const
{
    return false;
}


//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
