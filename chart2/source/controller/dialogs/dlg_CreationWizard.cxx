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

#include <dlg_CreationWizard.hxx>
#include <ResId.hxx>
#include <strings.hrc>
#include <helpids.h>
#include <ChartModel.hxx>

#include "tp_ChartType.hxx"
#include "tp_RangeChooser.hxx"
#include "tp_Wizard_TitlesAndObjects.hxx"
#include "tp_DataSource.hxx"
#include <ChartTypeTemplateProvider.hxx>
#include "DialogModel.hxx"

using namespace css;

using vcl::RoadmapWizardTypes::WizardPath;

namespace chart
{
#define PATH_FULL   1
#define STATE_FIRST        0
#define STATE_CHARTTYPE    STATE_FIRST
#define STATE_SIMPLE_RANGE 1
#define STATE_DATA_SERIES  2
#define STATE_OBJECTS      3
#define STATE_LAST         STATE_OBJECTS

CreationWizard::CreationWizard(weld::Window* pParent, const uno::Reference<frame::XModel>& xChartModel,
                               const uno::Reference<uno::XComponentContext>& xContext)
    : vcl::RoadmapWizardMachine(pParent)
    , m_xChartModel(xChartModel,uno::UNO_QUERY)
    , m_xComponentContext(xContext)
    , m_pTemplateProvider(nullptr)
    , m_aTimerTriggeredControllerLock(xChartModel)
    , m_bCanTravel(true)
{
    m_pDialogModel.reset(new DialogModel(m_xChartModel, m_xComponentContext));
    defaultButton(WizardButtonFlags::FINISH);

    setTitleBase(SchResId(STR_DLG_CHART_WIZARD));

    // tdf#134386 set m_pTemplateProvider before creating any other pages
    m_pTemplateProvider = static_cast<ChartTypeTabPage*>(GetOrCreatePage(STATE_CHARTTYPE));
    assert(m_pTemplateProvider && "must exist");
    m_pDialogModel->setTemplate(m_pTemplateProvider->getCurrentTemplate());

    WizardPath aPath = {
        STATE_CHARTTYPE,
        STATE_SIMPLE_RANGE,
        STATE_DATA_SERIES,
        STATE_OBJECTS
    };

    declarePath(PATH_FULL, aPath);

    SetRoadmapHelpId(HID_SCH_WIZARD_ROADMAP);

    if (!m_pDialogModel->getModel().isDataFromSpreadsheet())
    {
        enableState(STATE_SIMPLE_RANGE, false);
        enableState(STATE_DATA_SERIES, false);
    }

    // Call ActivatePage, to create and activate the first page
    ActivatePage();

    m_xAssistant->set_current_page(0);
}

CreationWizard::~CreationWizard() = default;

std::unique_ptr<BuilderPage> CreationWizard::createPage(WizardState nState)
{
    std::unique_ptr<vcl::OWizardPage> xRet;

    OString sIdent(OString::number(nState));
    weld::Container* pPageContainer = m_xAssistant->append_page(sIdent);

    switch( nState )
    {
        case STATE_CHARTTYPE:
        {
            m_aTimerTriggeredControllerLock.startTimer();
            xRet = std::make_unique<ChartTypeTabPage>(pPageContainer, this, m_xChartModel);
            break;
        }
        case STATE_SIMPLE_RANGE:
        {
            m_aTimerTriggeredControllerLock.startTimer();
            xRet = std::make_unique<RangeChooserTabPage>(pPageContainer, this, *m_pDialogModel, m_pTemplateProvider);
            break;
        }
        case STATE_DATA_SERIES:
        {
            m_aTimerTriggeredControllerLock.startTimer();
            xRet = std::make_unique<DataSourceTabPage>(pPageContainer, this, *m_pDialogModel, m_pTemplateProvider);
            break;
        }
        case STATE_OBJECTS:
        {
            xRet = std::make_unique<TitlesAndObjectsTabPage>(pPageContainer, this, m_xChartModel, m_xComponentContext);
            m_aTimerTriggeredControllerLock.startTimer();
            break;
        }
        default:
            break;
    }

    if (xRet)
        xRet->SetPageTitle(OUString()); //remove title of pages to not get them in the wizard title

    return xRet;
}

bool CreationWizard::leaveState( WizardState /*_nState*/ )
{
    return m_bCanTravel;
}

vcl::WizardTypes::WizardState CreationWizard::determineNextState( WizardState nCurrentState ) const
{
    if( !m_bCanTravel )
        return WZS_INVALID_STATE;
    if( nCurrentState == STATE_LAST )
        return WZS_INVALID_STATE;
    vcl::WizardTypes::WizardState nNextState = nCurrentState + 1;
    while( !isStateEnabled( nNextState ) && nNextState <= STATE_LAST )
        ++nNextState;
    return (nNextState==STATE_LAST+1) ? WZS_INVALID_STATE : nNextState;
}

void CreationWizard::enterState(WizardState nState)
{
    m_aTimerTriggeredControllerLock.startTimer();
    enableButtons( WizardButtonFlags::PREVIOUS, nState > STATE_FIRST );
    enableButtons( WizardButtonFlags::NEXT, nState < STATE_LAST );
    if( isStateEnabled( nState ))
        vcl::RoadmapWizardMachine::enterState(nState);
}

void CreationWizard::setInvalidPage(BuilderPage* pTabPage)
{
    if (pTabPage == m_pCurTabPage)
        m_bCanTravel = false;
}

void CreationWizard::setValidPage(BuilderPage* pTabPage)
{
    if (pTabPage == m_pCurTabPage)
        m_bCanTravel = true;
}

OUString CreationWizard::getStateDisplayName( WizardState nState ) const
{
    const char* pResId = nullptr;
    switch( nState )
    {
    case STATE_CHARTTYPE:
        pResId = STR_PAGE_CHARTTYPE;
        break;
    case STATE_SIMPLE_RANGE:
        pResId = STR_PAGE_DATA_RANGE;
        break;
    case STATE_DATA_SERIES:
        pResId = STR_OBJECT_DATASERIES_PLURAL;
        break;
    case STATE_OBJECTS:
        pResId = STR_PAGE_CHART_ELEMENTS;
        break;
    default:
        break;
    }
    if (!pResId)
        return OUString();
    return SchResId(pResId);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
