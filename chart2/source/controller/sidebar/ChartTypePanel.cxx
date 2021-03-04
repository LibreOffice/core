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

#include "ChartTypePanel.hxx"
#include <TimerTriggeredControllerLock.hxx>

#include <ChartController.hxx>
#include <ChartModelHelper.hxx>
#include <ChartResourceGroups.hxx>
#include <ChartTypeDialogController.hxx>
#include <DiagramHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <svtools/valueset.hxx>
#include <tools/diagnose_ex.h>

using namespace css;
using namespace css::uno;

namespace chart::sidebar
{
ChartTypePanel::ChartTypePanel(weld::Widget* pParent, ::chart::ChartController* pController)
    : PanelLayout(pParent, "ChartTypePanel", "modules/schart/ui/sidebartype.ui")
    , maContext()
    , mxModel(pController->getModel())
    , mxListener(new ChartSidebarModifyListener(this))
    , mbModelValid(true)
    , m_pDim3DLookResourceGroup(new Dim3DLookResourceGroup(m_xBuilder.get()))
    , m_pStackingResourceGroup(new StackingResourceGroup(m_xBuilder.get()))
    , m_pSplineResourceGroup(
          new SplineResourceGroup(m_xBuilder.get(), pController->GetChartFrame()))
    , m_pGeometryResourceGroup(new GeometryResourceGroup(m_xBuilder.get()))
    , m_pSortByXValuesResourceGroup(new SortByXValuesResourceGroup(m_xBuilder.get()))
    , m_xChartModel(mxModel, css::uno::UNO_QUERY_THROW)
    , m_aChartTypeDialogControllerList(0)
    , m_pCurrentMainType(nullptr)
    , m_nChangingCalls(0)
    , m_aTimerTriggeredControllerLock(m_xChartModel)
    , m_xMainTypeList(m_xBuilder->weld_combo_box("cmb_chartType"))
    , m_xSubTypeList(new ValueSet(m_xBuilder->weld_scrolled_window("subtypewin", true)))
    , m_xSubTypeListWin(new weld::CustomWeld(*m_xBuilder, "subtype", *m_xSubTypeList))
{
    Size aSize(m_xSubTypeList->GetDrawingArea()->get_ref_device().LogicToPixel(
        Size(120, 40), MapMode(MapUnit::MapAppFont)));
    m_xSubTypeListWin->set_size_request(aSize.Width(), aSize.Height());

    m_xMainTypeList->connect_changed(LINK(this, ChartTypePanel, SelectMainTypeHdl));
    m_xSubTypeList->SetSelectHdl(LINK(this, ChartTypePanel, SelectSubTypeHdl));

    m_xSubTypeList->SetStyle(m_xSubTypeList->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER
                             | WB_NAMEFIELD | WB_FLATVALUESET | WB_3DLOOK);
    m_xSubTypeList->SetColCount(4);
    m_xSubTypeList->SetLineCount(1);

    bool bEnableComplexChartTypes = true;
    uno::Reference<beans::XPropertySet> xProps(m_xChartModel, uno::UNO_QUERY);
    if (xProps.is())
    {
        try
        {
            xProps->getPropertyValue("EnableComplexChartTypes") >>= bEnableComplexChartTypes;
        }
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("chart2", "");
        }
    }

    m_aChartTypeDialogControllerList.push_back(std::make_unique<ColumnChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(std::make_unique<BarChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(std::make_unique<PieChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(std::make_unique<AreaChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(std::make_unique<LineChartDialogController>());
    if (bEnableComplexChartTypes)
    {
        m_aChartTypeDialogControllerList.push_back(std::make_unique<XYChartDialogController>());
        m_aChartTypeDialogControllerList.push_back(std::make_unique<BubbleChartDialogController>());
    }
    m_aChartTypeDialogControllerList.push_back(std::make_unique<NetChartDialogController>());
    if (bEnableComplexChartTypes)
    {
        m_aChartTypeDialogControllerList.push_back(std::make_unique<StockChartDialogController>());
    }
    m_aChartTypeDialogControllerList.push_back(
        std::make_unique<CombiColumnLineChartDialogController>());

    for (auto const& elem : m_aChartTypeDialogControllerList)
    {
        m_xMainTypeList->append("", elem->getName(), elem->getImage());
        elem->setChangeListener(this);
    }

    m_pDim3DLookResourceGroup->setChangeListener(this);
    m_pStackingResourceGroup->setChangeListener(this);
    m_pSplineResourceGroup->setChangeListener(this);
    m_pGeometryResourceGroup->setChangeListener(this);
    m_pSortByXValuesResourceGroup->setChangeListener(this);

    Initialize();
}

ChartTypePanel::~ChartTypePanel()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel,
                                                                    css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    //delete all dialog controller
    m_aChartTypeDialogControllerList.clear();

    //delete all resource helpers
    m_pDim3DLookResourceGroup.reset();
    m_pStackingResourceGroup.reset();
    m_pSplineResourceGroup.reset();
    m_pGeometryResourceGroup.reset();
    m_pSortByXValuesResourceGroup.reset();
    m_xSubTypeListWin.reset();
    m_xSubTypeList.reset();

    m_xSubTypeListWin.reset();
    m_xSubTypeList.reset();
    m_xMainTypeList.reset();
}

IMPL_LINK_NOARG(ChartTypePanel, SelectMainTypeHdl, weld::ComboBox&, void) { selectMainType(); }

IMPL_LINK_NOARG(ChartTypePanel, SelectSubTypeHdl, ValueSet*, void)
{
    if (m_pCurrentMainType)
    {
        ChartTypeParameter aParameter(getCurrentParameter());
        m_pCurrentMainType->adjustParameterToSubType(aParameter);
        fillAllControls(aParameter, false);
        commitToModel(aParameter);
    }
}

void ChartTypePanel::Initialize()
{
    if (!m_xChartModel.is())
        return;
    uno::Reference<lang::XMultiServiceFactory> xTemplateManager(
        m_xChartModel->getChartTypeManager(), uno::UNO_QUERY);
    uno::Reference<css::chart2::XDiagram> xDiagram(ChartModelHelper::findDiagram(m_xChartModel));
    DiagramHelper::tTemplateWithServiceName aTemplate
        = DiagramHelper::getTemplateForDiagram(xDiagram, xTemplateManager);
    OUString aServiceName(aTemplate.second);

    bool bFound = false;

    sal_uInt16 nM = 0;
    for (auto const& elem : m_aChartTypeDialogControllerList)
    {
        if (elem->isSubType(aServiceName))
        {
            bFound = true;

            m_xMainTypeList->set_active(nM);
            showAllControls(*elem);
            uno::Reference<beans::XPropertySet> xTemplateProps(aTemplate.first, uno::UNO_QUERY);
            ChartTypeParameter aParameter
                = elem->getChartTypeParameterForService(aServiceName, xTemplateProps);
            m_pCurrentMainType = getSelectedMainType();

            //set ThreeDLookScheme
            aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme(xDiagram);
            if (!aParameter.b3DLook && aParameter.eThreeDLookScheme != ThreeDLookScheme_Realistic)
                aParameter.eThreeDLookScheme = ThreeDLookScheme_Realistic;

            try
            {
                uno::Reference<beans::XPropertySet> xPropSet(xDiagram, uno::UNO_QUERY_THROW);
                xPropSet->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES)
                    >>= aParameter.bSortByXValues;
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }

            fillAllControls(aParameter);
            if (m_pCurrentMainType)
                m_pCurrentMainType->fillExtraControls(m_xChartModel, xTemplateProps);
            break;
        }
        ++nM;
    }

    if (!bFound)
    {
        m_xSubTypeList->Hide();
        m_pDim3DLookResourceGroup->showControls(false);
        m_pStackingResourceGroup->showControls(false);
        m_pSplineResourceGroup->showControls(false);
        m_pGeometryResourceGroup->showControls(false);
        m_pSortByXValuesResourceGroup->showControls(false);
    }
}

void ChartTypePanel::updateData()
{
    // Chart Type related
    if (!m_xChartModel.is())
        return;
    uno::Reference<lang::XMultiServiceFactory> xTemplateManager(
        m_xChartModel->getChartTypeManager(), uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(m_xChartModel);
    uno::Reference<css::chart2::XDiagram> xDiagram(ChartModelHelper::findDiagram(xModel));
    DiagramHelper::tTemplateWithServiceName aTemplate
        = DiagramHelper::getTemplateForDiagram(xDiagram, xTemplateManager);
    OUString aServiceName(aTemplate.second);

    sal_uInt16 nM = 0;
    for (auto const& elem : m_aChartTypeDialogControllerList)
    {
        if (elem->isSubType(aServiceName))
        {
            //m_pMainTypeList->SelectEntryPos(nM);
            //m_pMainTypeList->select_entry_region(nM, nM);
            break;
        }
        ++nM;
    }
}

void ChartTypePanel::DataChanged(const DataChangedEvent& rEvent)
{
    PanelLayout::DataChanged(rEvent);
    updateData();
}

void ChartTypePanel::HandleContextChange(const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
    updateData();
}

void ChartTypePanel::modelInvalid() { mbModelValid = false; }

void ChartTypePanel::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    if (mbModelValid)
    {
        css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel,
                                                                        css::uno::UNO_QUERY_THROW);
        xBroadcaster->removeModifyListener(mxListener);
    }

    mxModel = xModel;
    mbModelValid = true;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel,
                                                                       css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);
}

uno::Reference<css::chart2::XChartTypeTemplate> ChartTypePanel::getCurrentTemplate() const
{
    if (m_pCurrentMainType && m_xChartModel.is())
    {
        ChartTypeParameter aParameter(getCurrentParameter());
        m_pCurrentMainType->adjustParameterToSubType(aParameter);
        uno::Reference<lang::XMultiServiceFactory> xTemplateManager(
            m_xChartModel->getChartTypeManager(), uno::UNO_QUERY);
        return m_pCurrentMainType->getCurrentTemplate(aParameter, xTemplateManager);
    }
    return nullptr;
}

ChartTypeDialogController* ChartTypePanel::getSelectedMainType()
{
    ChartTypeDialogController* pTypeController = nullptr;
    auto nM = static_cast<std::vector<ChartTypeDialogController*>::size_type>(
        m_xMainTypeList->get_active());
    if (nM < m_aChartTypeDialogControllerList.size())
        pTypeController = m_aChartTypeDialogControllerList[nM].get();
    return pTypeController;
}

void ChartTypePanel::showAllControls(ChartTypeDialogController& rTypeController)
{
    m_xMainTypeList->show();
    m_xSubTypeList->Show();

    bool bShow = rTypeController.shouldShow_3DLookControl();
    m_pDim3DLookResourceGroup->showControls(bShow);
    bShow = rTypeController.shouldShow_StackingControl();
    m_pStackingResourceGroup->showControls(bShow);
    bShow = rTypeController.shouldShow_SplineControl();
    m_pSplineResourceGroup->showControls(bShow);
    bShow = rTypeController.shouldShow_GeometryControl();
    m_pGeometryResourceGroup->showControls(bShow);
    bShow = rTypeController.shouldShow_SortByXValuesResourceGroup();
    m_pSortByXValuesResourceGroup->showControls(bShow);
    rTypeController.showExtraControls(m_xBuilder.get());
}

void ChartTypePanel::fillAllControls(const ChartTypeParameter& rParameter,
                                     bool bAlsoResetSubTypeList)
{
    m_nChangingCalls++;
    if (m_pCurrentMainType && bAlsoResetSubTypeList)
    {
        m_pCurrentMainType->fillSubTypeList(*m_xSubTypeList, rParameter);
    }
    m_xSubTypeList->SelectItem(static_cast<sal_uInt16>(rParameter.nSubTypeIndex));
    m_pDim3DLookResourceGroup->fillControls(rParameter);
    m_pStackingResourceGroup->fillControls(rParameter);
    m_pSplineResourceGroup->fillControls(rParameter);
    m_pGeometryResourceGroup->fillControls(rParameter);
    m_pSortByXValuesResourceGroup->fillControls(rParameter);
    m_nChangingCalls--;
}

ChartTypeParameter ChartTypePanel::getCurrentParameter() const
{
    ChartTypeParameter aParameter;
    aParameter.nSubTypeIndex = static_cast<sal_Int32>(m_xSubTypeList->GetSelectedItemId());
    m_pDim3DLookResourceGroup->fillParameter(aParameter);
    m_pStackingResourceGroup->fillParameter(aParameter);
    m_pSplineResourceGroup->fillParameter(aParameter);
    m_pGeometryResourceGroup->fillParameter(aParameter);
    m_pSortByXValuesResourceGroup->fillParameter(aParameter);
    return aParameter;
}

void ChartTypePanel::stateChanged()
{
    if (m_nChangingCalls)
        return;
    m_nChangingCalls++;

    ChartTypeParameter aParameter(getCurrentParameter());
    if (m_pCurrentMainType)
    {
        m_pCurrentMainType->adjustParameterToSubType(aParameter);
        m_pCurrentMainType->adjustSubTypeAndEnableControls(aParameter);
    }
    commitToModel(aParameter);

    //detect the new ThreeDLookScheme
    uno::Reference<css::chart2::XDiagram> xDiagram = ChartModelHelper::findDiagram(m_xChartModel);
    aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme(xDiagram);
    try
    {
        uno::Reference<beans::XPropertySet> xPropSet(xDiagram, uno::UNO_QUERY_THROW);
        xPropSet->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= aParameter.bSortByXValues;
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    //the controls have to be enabled/disabled accordingly
    fillAllControls(aParameter);

    m_nChangingCalls--;
}

void ChartTypePanel::commitToModel(const ChartTypeParameter& rParameter)
{
    if (!m_pCurrentMainType)
        return;

    m_aTimerTriggeredControllerLock.startTimer();
    m_pCurrentMainType->commitToModel(rParameter, m_xChartModel);
}

void ChartTypePanel::selectMainType()
{
    ChartTypeParameter aParameter(getCurrentParameter());

    if (m_pCurrentMainType)
    {
        m_pCurrentMainType->adjustParameterToSubType(aParameter);
        m_pCurrentMainType->hideExtraControls();
    }

    m_pCurrentMainType = getSelectedMainType();
    if (!m_pCurrentMainType)
        return;

    showAllControls(*m_pCurrentMainType);

    m_pCurrentMainType->adjustParameterToMainType(aParameter);
    commitToModel(aParameter);
    //detect the new ThreeDLookScheme
    aParameter.eThreeDLookScheme
        = ThreeDHelper::detectScheme(ChartModelHelper::findDiagram(m_xChartModel));
    if (!aParameter.b3DLook && aParameter.eThreeDLookScheme != ThreeDLookScheme_Realistic)
        aParameter.eThreeDLookScheme = ThreeDLookScheme_Realistic;

    uno::Reference<css::chart2::XDiagram> xDiagram = ChartModelHelper::findDiagram(m_xChartModel);
    try
    {
        uno::Reference<beans::XPropertySet> xPropSet(xDiagram, uno::UNO_QUERY_THROW);
        xPropSet->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= aParameter.bSortByXValues;
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    fillAllControls(aParameter);
    uno::Reference<beans::XPropertySet> xTemplateProps(getCurrentTemplate(), uno::UNO_QUERY);
    m_pCurrentMainType->fillExtraControls(m_xChartModel, xTemplateProps);
}
} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
