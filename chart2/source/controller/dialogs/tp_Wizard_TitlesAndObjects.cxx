/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

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

    this->SetText( String( SchResId( STR_PAGE_CHART_ELEMENTS ) ) );

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

IMPL_LINK( TitlesAndObjectsTabPage, ChangeHdl, void *, EMPTYARG )
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
