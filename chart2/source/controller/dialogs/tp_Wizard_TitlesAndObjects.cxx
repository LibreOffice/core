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
#include <res_Titles.hxx>
#include <res_LegendPosition.hxx>
#include <ChartModel.hxx>
#include <Diagram.hxx>
#include <AxisHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <utility>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

TitlesAndObjectsTabPage::TitlesAndObjectsTabPage(weld::Container* pPage, weld::DialogController* pController,
                                                 rtl::Reference<::chart::ChartModel> xChartModel,
                                                 const uno::Reference< uno::XComponentContext >& xContext )
    : OWizardPage(pPage, pController, "modules/schart/ui/wizelementspage.ui", "WizElementsPage")
    , m_xTitleResources(new TitleResources(*m_xBuilder, false))
    , m_xLegendPositionResources(new LegendPositionResources(*m_xBuilder, xContext))
    , m_xChartModel(std::move(xChartModel))
    , m_xCC(xContext)
    , m_bCommitToModel(true)
    , m_aTimerTriggeredControllerLock( m_xChartModel )
    , m_xCB_Grid_X(m_xBuilder->weld_check_button("x"))
    , m_xCB_Grid_Y(m_xBuilder->weld_check_button("y"))
    , m_xCB_Grid_Z(m_xBuilder->weld_check_button("z"))
{
    m_xTitleResources->connect_changed( LINK( this, TitlesAndObjectsTabPage, ChangeEditHdl ));
    m_xLegendPositionResources->SetChangeHdl( LINK( this, TitlesAndObjectsTabPage, ChangeHdl ));

    m_xCB_Grid_X->connect_toggled( LINK( this, TitlesAndObjectsTabPage, ChangeCheckBoxHdl ));
    m_xCB_Grid_Y->connect_toggled( LINK( this, TitlesAndObjectsTabPage, ChangeCheckBoxHdl ));
    m_xCB_Grid_Z->connect_toggled( LINK( this, TitlesAndObjectsTabPage, ChangeCheckBoxHdl ));
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
        aTitleInput.readFromModel( m_xChartModel );
        m_xTitleResources->writeToResources( aTitleInput );
    }

    //init legend
    {
        m_xLegendPositionResources->writeToResources( m_xChartModel );
    }

    //init grid checkboxes
    {
        rtl::Reference< Diagram > xDiagram = m_xChartModel->getFirstChartDiagram();
        uno::Sequence< sal_Bool > aPossibilityList;
        uno::Sequence< sal_Bool > aExistenceList;
        AxisHelper::getAxisOrGridPossibilities( aPossibilityList, xDiagram, false );
        AxisHelper::getAxisOrGridExistence( aExistenceList, xDiagram, false );
        m_xCB_Grid_X->set_sensitive( aPossibilityList[0] );
        m_xCB_Grid_Y->set_sensitive( aPossibilityList[1] );
        m_xCB_Grid_Z->set_sensitive( aPossibilityList[2] );
        m_xCB_Grid_X->set_active( aExistenceList[0] );
        m_xCB_Grid_Y->set_active( aExistenceList[1] );
        m_xCB_Grid_Z->set_active( aExistenceList[2] );
    }

    m_bCommitToModel = true;
}

bool TitlesAndObjectsTabPage::commitPage( ::vcl::WizardTypes::CommitPageReason /*eReason*/ )
{
    if( m_xTitleResources->get_value_changed_from_saved() ) //titles may have changed in the meanwhile
        commitToModel();
    return true;//return false if this page should not be left
}

void TitlesAndObjectsTabPage::commitToModel()
{
    m_aTimerTriggeredControllerLock.startTimer();
    rtl::Reference<::chart::ChartModel>  xModel = m_xChartModel;

    ControllerLockGuardUNO aLockedControllers( xModel );

    //commit title changes to model
    {
        TitleDialogData aTitleOutput;
        m_xTitleResources->readFromResources( aTitleOutput );
        aTitleOutput.writeDifferenceToModel( xModel, m_xCC );
        m_xTitleResources->save_value();
    }

    //commit legend changes to model
    {
        m_xLegendPositionResources->writeToModel( xModel );
    }

    //commit grid changes to model
    {
        rtl::Reference< Diagram > xDiagram = xModel->getFirstChartDiagram();
        uno::Sequence< sal_Bool > aOldExistenceList;
        AxisHelper::getAxisOrGridExistence( aOldExistenceList, xDiagram, false );
        uno::Sequence< sal_Bool > aNewExistenceList(aOldExistenceList);
        sal_Bool* pNewExistenceList = aNewExistenceList.getArray();
        pNewExistenceList[0] = m_xCB_Grid_X->get_active();
        pNewExistenceList[1] = m_xCB_Grid_Y->get_active();
        pNewExistenceList[2] = m_xCB_Grid_Z->get_active();
        AxisHelper::changeVisibilityOfGrids( xDiagram
                , aOldExistenceList, aNewExistenceList );
    }
}

IMPL_LINK_NOARG(TitlesAndObjectsTabPage, ChangeCheckBoxHdl, weld::Toggleable&, void)
{
    ChangeHdl(nullptr);
}

IMPL_LINK_NOARG(TitlesAndObjectsTabPage, ChangeEditHdl, weld::Entry&, void)
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
