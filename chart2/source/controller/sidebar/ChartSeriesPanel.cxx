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

#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include "ChartSeriesPanel.hxx"
#include "ChartController.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/toolbox.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <comphelper/processfactory.hxx>

#include "ChartModel.hxx"
#include "DataSeriesHelper.hxx"
#include "RegressionCurveHelper.hxx"
#include "StatisticsHelper.hxx"

using namespace css;
using namespace css::uno;
using ::sfx2::sidebar::Theme;

namespace chart { namespace sidebar {

namespace {

bool isDataLabelVisible(css::uno::Reference<css::frame::XModel> xModel, const OUString& rCID)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return false;

    return DataSeriesHelper::hasDataLabelsAtSeries(xSeries);
}

void setDataLabelVisible(css::uno::Reference<css::frame::XModel> xModel, const OUString& rCID, bool bVisible)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return;

    if (bVisible)
        DataSeriesHelper::insertDataLabelsToSeriesAndAllPoints(xSeries);
    else
        DataSeriesHelper::deleteDataLabelsFromSeriesAndAllPoints(xSeries);
}

struct LabelPlacementMap
{
    sal_Int32 nPos;
    sal_Int32 nApi;
};

LabelPlacementMap aLabelPlacementMap[] = {
    { 0, css::chart::DataLabelPlacement::TOP },
    { 1, css::chart::DataLabelPlacement::BOTTOM },
    { 2, css::chart::DataLabelPlacement::CENTER },
    { 3, css::chart::DataLabelPlacement::OUTSIDE },
    { 4, css::chart::DataLabelPlacement::INSIDE  },
    { 5, css::chart::DataLabelPlacement::NEAR_ORIGIN }
};

sal_Int32 getDataLabelPlacement(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID)
{
    css::uno::Reference< css::beans::XPropertySet > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return 0;

    css::uno::Any aAny = xSeries->getPropertyValue("LabelPlacement");
    if (!aAny.hasValue())
        return 0;

    sal_Int32 nPlacement = 0;
    aAny >>= nPlacement;

    for (size_t i = 0; i < SAL_N_ELEMENTS(aLabelPlacementMap); ++i)
    {
        if (aLabelPlacementMap[i].nApi == nPlacement)
            return aLabelPlacementMap[i].nPos;
    }

    return 0;
}

void setDataLabelPlacement(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID, sal_Int32 nPos)
{
    css::uno::Reference< css::beans::XPropertySet > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return;

    sal_Int32 nApi = 0;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aLabelPlacementMap); ++i)
    {
        if (aLabelPlacementMap[i].nPos == nPos)
        {
            nApi = aLabelPlacementMap[i].nApi;
            break;
        }
    }

    xSeries->setPropertyValue("LabelPlacement", css::uno::makeAny(nApi));
}

bool isTrendlineVisible(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID)
{
    css::uno::Reference< css::chart2::XRegressionCurveContainer > xRegressionCurveContainer(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xRegressionCurveContainer.is())
        return false;

    return xRegressionCurveContainer->getRegressionCurves().getLength() != 0;
}

void setTrendlineVisible(css::uno::Reference<css::frame::XModel>
        xModel, const OUString& rCID, bool bVisible)
{
    css::uno::Reference< css::chart2::XRegressionCurveContainer > xRegressionCurveContainer(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xRegressionCurveContainer.is())
        return;

    if (bVisible)
    {
        /* code */
        uno::Reference< chart2::XRegressionCurve > xCurve =
            RegressionCurveHelper::addRegressionCurve(
                    CHREGRESS_LINEAR,
                    xRegressionCurveContainer,
                    comphelper::getProcessComponentContext());
    }
    else
        RegressionCurveHelper::removeAllExceptMeanValueLine(
                xRegressionCurveContainer );

}

bool isErrorBarVisible(css::uno::Reference<css::frame::XModel>
        xModel, const OUString& rCID, bool bYError)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return false;

    return StatisticsHelper::hasErrorBars(xSeries, bYError);
}

void setErrorBarVisible(css::uno::Reference<css::frame::XModel>
        xModel, const OUString& rCID, bool bYError, bool bVisible)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return;

    if (bVisible)
    {
                StatisticsHelper::addErrorBars( xSeries, comphelper::getProcessComponentContext(),
                    css::chart::ErrorBarStyle::STANDARD_DEVIATION,
                    bYError);
    }
    else
    {
        StatisticsHelper::removeErrorBars( xSeries, bYError );
    }
}

bool isPrimaryAxis(css::uno::Reference<css::frame::XModel>
        xModel, const OUString& rCID)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return true;

    return DataSeriesHelper::getAttachedAxisIndex(xSeries) == 0;
}

void setAttachedAxisType(css::uno::Reference<css::frame::XModel>
        xModel, const OUString& rCID, bool bPrimary)
{
    css::uno::Reference< css::beans::XPropertySet > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return;

    sal_Int32 nIndex = bPrimary ? 0 : 1;
    xSeries->setPropertyValue("AttachedAxisIndex", css::uno::makeAny(nIndex));
}

css::uno::Reference<css::chart2::XChartType> getChartType(
        css::uno::Reference<css::frame::XModel> xModel)
{
    css::uno::Reference<css::chart2::XChartDocument> xChartDoc (xModel, css::uno::UNO_QUERY);
    css::uno::Reference<css::chart2::XDiagram> xDiagram = xChartDoc->getFirstDiagram();
    css::uno::Reference< css::chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, UNO_QUERY_THROW );
    css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());
    css::uno::Reference< css::chart2::XChartTypeContainer > xChartTypeContainer( xCooSysSequence[0], UNO_QUERY_THROW );
    css::uno::Sequence< css::uno::Reference< css::chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );
    return xChartTypeSequence[0];
}

OUString getSeriesLabel(css::uno::Reference<css::frame::XModel> xModel, const OUString& rCID)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return OUString();

    css::uno::Reference<css::chart2::XChartType> xChartType = getChartType(xModel);
    return DataSeriesHelper::getDataSeriesLabel(xSeries, xChartType->getRoleOfSequenceForSeriesLabel());
}

OUString getCID(css::uno::Reference<css::frame::XModel> xModel)
{
    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    uno::Any aAny = xSelectionSupplier->getSelection();
    if (!aAny.hasValue())
        return OUString();

    OUString aCID;
    aAny >>= aCID;
#ifdef DBG_UTIL
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    assert(eType == OBJECTTYPE_DATA_SERIES);
#endif

    return aCID;
}

}

ChartSeriesPanel::ChartSeriesPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
  : PanelLayout(pParent, "ChartSeriesPanel", "modules/schart/ui/sidebarseries.ui", rxFrame),
    mxFrame(rxFrame),
    mxModel(pController->getModel()),
    mxListener(new ChartSidebarModifyListener(this))
{
    get(mpCBLabel, "checkbutton_label");
    get(mpCBTrendline, "checkbutton_trendline");
    get(mpCBXError, "checkbutton_x_error");
    get(mpCBYError, "checkbutton_y_error");

    get(mpRBPrimaryAxis, "radiobutton_primary_axis");
    get(mpRBSecondaryAxis, "radiobutton_secondary_axis");

    get(mpLBLabelPlacement, "comboboxtext_label");

    get(mpFTSeriesName, "label_series_name");

    Initialize();
}

ChartSeriesPanel::~ChartSeriesPanel()
{
    disposeOnce();
}

void ChartSeriesPanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    mpCBLabel.clear();
    mpCBTrendline.clear();
    mpCBXError.clear();
    mpCBYError.clear();

    mpRBPrimaryAxis.clear();
    mpRBSecondaryAxis.clear();

    mpLBLabelPlacement.clear();

    mpFTSeriesName.clear();

    PanelLayout::dispose();
}

void ChartSeriesPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);

    updateData();

    Link<> aLink = LINK(this, ChartSeriesPanel, CheckBoxHdl);
    mpCBLabel->SetClickHdl(aLink);
    mpCBTrendline->SetClickHdl(aLink);
    mpCBXError->SetClickHdl(aLink);
    mpCBYError->SetClickHdl(aLink);

    aLink = LINK(this, ChartSeriesPanel, RadioBtnHdl);
    mpRBPrimaryAxis->SetToggleHdl(aLink);
    mpRBSecondaryAxis->SetToggleHdl(aLink);

    mpLBLabelPlacement->SetSelectHdl(LINK(this, ChartSeriesPanel, ListBoxHdl));
}

void ChartSeriesPanel::updateData()
{
    OUString aCID = getCID(mxModel);
    SolarMutexGuard aGuard;
    bool bLabelVisible = isDataLabelVisible(mxModel, aCID);
    mpCBLabel->Check(bLabelVisible);
    mpCBTrendline->Check(isTrendlineVisible(mxModel, aCID));
    mpCBXError->Check(isErrorBarVisible(mxModel, aCID, false));
    mpCBYError->Check(isErrorBarVisible(mxModel, aCID, true));

    bool bPrimaryAxis = isPrimaryAxis(mxModel, aCID);
    mpRBPrimaryAxis->Check(bPrimaryAxis);
    mpRBSecondaryAxis->Check(!bPrimaryAxis);

    mpLBLabelPlacement->Enable(bLabelVisible);
    mpLBLabelPlacement->SelectEntryPos(getDataLabelPlacement(mxModel, aCID));

    mpFTSeriesName->SetText(getSeriesLabel(mxModel, aCID));
}

VclPtr<vcl::Window> ChartSeriesPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to ChartSeriesPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartSeriesPanel::Create", NULL, 1);

    return  VclPtr<ChartSeriesPanel>::Create(
                        pParent, rxFrame, pController);
}

void ChartSeriesPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartSeriesPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& )
{
    updateData();
}

void ChartSeriesPanel::NotifyItemUpdate(
    sal_uInt16 /*nSID*/,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/,
    const bool )
{
}

void ChartSeriesPanel::modelInvalid()
{

}

void ChartSeriesPanel::updateModel(
        css::uno::Reference<css::frame::XModel> xModel)
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    mxModel = xModel;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);
}

IMPL_LINK(ChartSeriesPanel, CheckBoxHdl, CheckBox*, pCheckBox)
{
    bool bChecked = pCheckBox->IsChecked();
    OUString aCID = getCID(mxModel);
    if (pCheckBox == mpCBLabel.get())
        setDataLabelVisible(mxModel, aCID, bChecked);
    else if (pCheckBox == mpCBTrendline.get())
        setTrendlineVisible(mxModel, aCID, bChecked);
    else if (pCheckBox == mpCBXError.get())
        setErrorBarVisible(mxModel, aCID, false, bChecked);
    else if (pCheckBox == mpCBYError.get())
        setErrorBarVisible(mxModel, aCID, true, bChecked);

    return 0;
}

IMPL_LINK_NOARG(ChartSeriesPanel, RadioBtnHdl)
{
    OUString aCID = getCID(mxModel);
    bool bChecked = mpRBPrimaryAxis->IsChecked();

    setAttachedAxisType(mxModel, aCID, bChecked);

    return 0;
}

IMPL_LINK_NOARG(ChartSeriesPanel, ListBoxHdl)
{
    OUString aCID = getCID(mxModel);

    sal_Int32 nPos = mpLBLabelPlacement->GetSelectEntryPos();
    setDataLabelPlacement(mxModel, aCID, nPos);

    return 0;
}

}} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
