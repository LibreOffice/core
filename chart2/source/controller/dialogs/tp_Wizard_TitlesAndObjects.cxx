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
#include "strings.hrc"
#include "res_Titles.hxx"
#include "res_LegendPosition.hxx"
#include "ResId.hxx"
#include "HelpIds.hrc"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "LegendHelper.hxx"
#include "ControllerLockGuard.hxx"

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

TitlesAndObjectsTabPage::TitlesAndObjectsTabPage( svt::OWizardMachine* pParent
        , const uno::Reference< XChartDocument >& xChartModel
        , const uno::Reference< uno::XComponentContext >& xContext )
        : OWizardPage(pParent, "WizElementsPage", "modules/schart/ui/wizelementspage.ui")
        , m_xTitleResources(new TitleResources(*this, false))
        , m_xLegendPositionResources(new LegendPositionResources(*this, xContext))
        , m_xChartModel(xChartModel)
        , m_xCC(xContext)
        , m_bCommitToModel(true)
        , m_aTimerTriggeredControllerLock( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY ) )
{
    get(m_pCB_Grid_X, "x");
    get(m_pCB_Grid_Y, "y");
    get(m_pCB_Grid_Z, "z");

    m_xTitleResources->SetUpdateDataHdl( LINK( this, TitlesAndObjectsTabPage, ChangeEditHdl ));
    m_xLegendPositionResources->SetChangeHdl( LINK( this, TitlesAndObjectsTabPage, ChangeHdl ));

    m_pCB_Grid_X->SetToggleHdl( LINK( this, TitlesAndObjectsTabPage, ChangeCheckBoxHdl ));
    m_pCB_Grid_Y->SetToggleHdl( LINK( this, TitlesAndObjectsTabPage, ChangeCheckBoxHdl ));
    m_pCB_Grid_Z->SetToggleHdl( LINK( this, TitlesAndObjectsTabPage, ChangeCheckBoxHdl ));
}

TitlesAndObjectsTabPage::~TitlesAndObjectsTabPage()
{
    disposeOnce();
}

void TitlesAndObjectsTabPage::dispose()
{
    m_pCB_Grid_X.clear();
    m_pCB_Grid_Y.clear();
    m_pCB_Grid_Z.clear();
    OWizardPage::dispose();
}

void TitlesAndObjectsTabPage::initializePage()
{
    m_bCommitToModel = false;

    //init titles
    {
        TitleDialogData aTitleInput;
        aTitleInput.readFromModel( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY) );
        m_xTitleResources->writeToResources( aTitleInput );
    }

    //init legend
    {
        m_xLegendPositionResources->writeToResources( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY) );
    }

    //init grid checkboxes
    {
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( m_xChartModel );
        uno::Sequence< sal_Bool > aPossibilityList;
        uno::Sequence< sal_Bool > aExistenceList;
        AxisHelper::getAxisOrGridPossibilities( aPossibilityList, xDiagram, false );
        AxisHelper::getAxisOrGridExcistence( aExistenceList, xDiagram, false );
        m_pCB_Grid_X->Enable( aPossibilityList[0] );
        m_pCB_Grid_Y->Enable( aPossibilityList[1] );
        m_pCB_Grid_Z->Enable( aPossibilityList[2] );
        m_pCB_Grid_X->Check( aExistenceList[0] );
        m_pCB_Grid_Y->Check( aExistenceList[1] );
        m_pCB_Grid_Z->Check( aExistenceList[2] );
    }

    m_bCommitToModel = true;
}

bool TitlesAndObjectsTabPage::commitPage( ::svt::WizardTypes::CommitPageReason /*eReason*/ )
{
    if( m_xTitleResources->IsModified() ) //titles may have changed in the meanwhile
        commitToModel();
    return true;//return false if this page should not be left
}

void TitlesAndObjectsTabPage::commitToModel()
{
    m_aTimerTriggeredControllerLock.startTimer();
    uno::Reference< frame::XModel >  xModel( m_xChartModel, uno::UNO_QUERY);

    ControllerLockGuardUNO aLockedControllers( xModel );

    //commit title changes to model
    {
        TitleDialogData aTitleOutput;
        m_xTitleResources->readFromResources( aTitleOutput );
        aTitleOutput.writeDifferenceToModel( xModel, m_xCC );
        m_xTitleResources->ClearModifyFlag();
    }

    //commit legend changes to model
    {
        m_xLegendPositionResources->writeToModel( xModel );
    }

    //commit grid changes to model
    {
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( xModel );
        uno::Sequence< sal_Bool > aOldExistenceList;
        AxisHelper::getAxisOrGridExcistence( aOldExistenceList, xDiagram, false );
        uno::Sequence< sal_Bool > aNewExistenceList(aOldExistenceList);
        aNewExistenceList[0] = m_pCB_Grid_X->IsChecked();
        aNewExistenceList[1] = m_pCB_Grid_Y->IsChecked();
        aNewExistenceList[2] = m_pCB_Grid_Z->IsChecked();
        AxisHelper::changeVisibilityOfGrids( xDiagram
                , aOldExistenceList, aNewExistenceList, m_xCC );
    }
}


IMPL_LINK_NOARG(TitlesAndObjectsTabPage, ChangeCheckBoxHdl, CheckBox&, void)
{
    ChangeHdl(nullptr);
}
IMPL_LINK_NOARG(TitlesAndObjectsTabPage, ChangeEditHdl, Edit&, void)
{
    ChangeHdl(nullptr);
}
IMPL_LINK_NOARG(TitlesAndObjectsTabPage, ChangeHdl, LinkParamNone*, void)
{
    if( m_bCommitToModel )
        commitToModel();
}

bool TitlesAndObjectsTabPage::canAdvance() const
{
    return false;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
