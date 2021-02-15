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

#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include "ChartSeriesPanel.hxx"
#include <ChartController.hxx>

#include <DataSeriesHelper.hxx>
#include <RegressionCurveHelper.hxx>
#include <StatisticsHelper.hxx>

using namespace css;
using namespace css::uno;

namespace chart::sidebar {

namespace {

bool isDataLabelVisible(const css::uno::Reference<css::frame::XModel>& xModel, const OUString& rCID)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return false;

    return DataSeriesHelper::hasDataLabelsAtSeries(xSeries);
}

void setDataLabelVisible(const css::uno::Reference<css::frame::XModel>& xModel, const OUString& rCID, bool bVisible)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

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

LabelPlacementMap const aLabelPlacementMap[] = {
    { 0, css::chart::DataLabelPlacement::TOP },
    { 1, css::chart::DataLabelPlacement::BOTTOM },
    { 2, css::chart::DataLabelPlacement::CENTER },
    { 3, css::chart::DataLabelPlacement::OUTSIDE },
    { 4, css::chart::DataLabelPlacement::INSIDE  },
    { 5, css::chart::DataLabelPlacement::NEAR_ORIGIN }
};

sal_Int32 getDataLabelPlacement(const css::uno::Reference<css::frame::XModel>& xModel,
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

    for (LabelPlacementMap const & i : aLabelPlacementMap)
    {
        if (i.nApi == nPlacement)
            return i.nPos;
    }

    return 0;
}

void setDataLabelPlacement(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, sal_Int32 nPos)
{
    css::uno::Reference< css::beans::XPropertySet > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return;

    sal_Int32 nApi = 0;
    for (LabelPlacementMap const & i : aLabelPlacementMap)
    {
        if (i.nPos == nPos)
        {
            nApi = i.nApi;
            break;
        }
    }

    xSeries->setPropertyValue("LabelPlacement", css::uno::Any(nApi));
}

bool isTrendlineVisible(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID)
{
    css::uno::Reference< css::chart2::XRegressionCurveContainer > xRegressionCurveContainer(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xRegressionCurveContainer.is())
        return false;

    return xRegressionCurveContainer->getRegressionCurves().hasElements();
}

void setTrendlineVisible(const css::uno::Reference<css::frame::XModel>&
        xModel, const OUString& rCID, bool bVisible)
{
    css::uno::Reference< css::chart2::XRegressionCurveContainer > xRegressionCurveContainer(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xRegressionCurveContainer.is())
        return;

    if (bVisible)
    {
        RegressionCurveHelper::addRegressionCurve(
                    SvxChartRegress::Linear,
                    xRegressionCurveContainer);
    }
    else
        RegressionCurveHelper::removeAllExceptMeanValueLine(
                xRegressionCurveContainer );

}

bool isErrorBarVisible(const css::uno::Reference<css::frame::XModel>& xModel,
                       const OUString& rCID, bool bYError)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return false;

    return StatisticsHelper::hasErrorBars(xSeries, bYError);
}

void setErrorBarVisible(const css::uno::Reference<css::frame::XModel>&
        xModel, const OUString& rCID, bool bYError, bool bVisible)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return;

    if (bVisible)
    {
        StatisticsHelper::addErrorBars( xSeries,
                    css::chart::ErrorBarStyle::STANDARD_DEVIATION,
                    bYError);
    }
    else
    {
        StatisticsHelper::removeErrorBars( xSeries, bYError );
    }
}

bool isPrimaryAxis(const css::uno::Reference<css::frame::XModel>&
        xModel, const OUString& rCID)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return true;

    return DataSeriesHelper::getAttachedAxisIndex(xSeries) == 0;
}

void setAttachedAxisType(const css::uno::Reference<css::frame::XModel>&
        xModel, const OUString& rCID, bool bPrimary)
{
    css::uno::Reference< css::beans::XPropertySet > xSeries(
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xSeries.is())
        return;

    sal_Int32 nIndex = bPrimary ? 0 : 1;
    xSeries->setPropertyValue("AttachedAxisIndex", css::uno::Any(nIndex));
}

css::uno::Reference<css::chart2::XChartType> getChartType(
        const css::uno::Reference<css::frame::XModel>& xModel)
{
    css::uno::Reference<css::chart2::XChartDocument> xChartDoc (xModel, css::uno::UNO_QUERY);
    css::uno::Reference<css::chart2::XDiagram> xDiagram = xChartDoc->getFirstDiagram();
    css::uno::Reference< css::chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, UNO_QUERY_THROW );
    css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());
    css::uno::Reference< css::chart2::XChartTypeContainer > xChartTypeContainer( xCooSysSequence[0], UNO_QUERY_THROW );
    css::uno::Sequence< css::uno::Reference< css::chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );
    return xChartTypeSequence[0];
}

OUString getSeriesLabel(const css::uno::Reference<css::frame::XModel>& xModel, const OUString& rCID)
{
    css::uno::Reference< css::chart2::XDataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return OUString();

    css::uno::Reference<css::chart2::XChartType> xChartType = getChartType(xModel);
    return DataSeriesHelper::getDataSeriesLabel(xSeries, xChartType->getRoleOfSequenceForSeriesLabel());
}

OUString getCID(const css::uno::Reference<css::frame::XModel>& xModel)
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

    if (aCID.isEmpty())
        return OUString();

#if defined DBG_UTIL && !defined NDEBUG
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType != OBJECTTYPE_DATA_SERIES &&
         eType != OBJECTTYPE_DATA_POINT &&
         eType != OBJECTTYPE_DATA_CURVE)
        SAL_WARN("chart2","Selected item is not a chart series");
#endif

    return aCID;
}

}

ChartSeriesPanel::ChartSeriesPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
    : PanelLayout(pParent, "ChartSeriesPanel", "modules/schart/ui/sidebarseries.ui", rxFrame)
    , mxCBLabel(m_xBuilder->weld_check_button("checkbutton_label"))
    , mxCBTrendline(m_xBuilder->weld_check_button("checkbutton_trendline"))
    , mxCBXError(m_xBuilder->weld_check_button("checkbutton_x_error"))
    , mxCBYError(m_xBuilder->weld_check_button("checkbutton_y_error"))
    , mxRBPrimaryAxis(m_xBuilder->weld_radio_button("radiobutton_primary_axis"))
    , mxRBSecondaryAxis(m_xBuilder->weld_radio_button("radiobutton_secondary_axis"))
    , mxBoxLabelPlacement(m_xBuilder->weld_widget("datalabel_box"))
    , mxLBLabelPlacement(m_xBuilder->weld_combo_box("comboboxtext_label"))
    , mxFTSeriesName(m_xBuilder->weld_label("label_series_name"))
    , mxFTSeriesTemplate(m_xBuilder->weld_label("label_series_tmpl"))
    , mxModel(pController->getModel())
    , mxListener(new ChartSidebarModifyListener(this))
    , mxSelectionListener(new ChartSidebarSelectionListener(this, OBJECTTYPE_DATA_SERIES))
    , mbModelValid(true)
{
    Initialize();

    m_pInitialFocusWidget = mxCBLabel.get();
}

ChartSeriesPanel::~ChartSeriesPanel()
{
    disposeOnce();
}

void ChartSeriesPanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->removeSelectionChangeListener(mxSelectionListener);

    mxCBLabel.reset();
    mxCBTrendline.reset();
    mxCBXError.reset();
    mxCBYError.reset();

    mxRBPrimaryAxis.reset();
    mxRBSecondaryAxis.reset();

    mxBoxLabelPlacement.reset();
    mxLBLabelPlacement.reset();

    mxFTSeriesName.reset();
    mxFTSeriesTemplate.reset();

    PanelLayout::dispose();
}

void ChartSeriesPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    updateData();

    Link<weld::ToggleButton&,void> aLink = LINK(this, ChartSeriesPanel, CheckBoxHdl);
    mxCBLabel->connect_toggled(aLink);
    mxCBTrendline->connect_toggled(aLink);
    mxCBXError->connect_toggled(aLink);
    mxCBYError->connect_toggled(aLink);

    Link<weld::ToggleButton&,void> aLink2 = LINK(this, ChartSeriesPanel, RadioBtnHdl);
    mxRBPrimaryAxis->connect_toggled(aLink2);
    mxRBSecondaryAxis->connect_toggled(aLink2);

    mxLBLabelPlacement->connect_changed(LINK(this, ChartSeriesPanel, ListBoxHdl));
}

void ChartSeriesPanel::updateData()
{
    if (!mbModelValid)
        return;

    OUString aCID = getCID(mxModel);
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType!=OBJECTTYPE_DATA_SERIES &&
          eType != OBJECTTYPE_DATA_POINT &&
          eType != OBJECTTYPE_DATA_CURVE)
        return;

    SolarMutexGuard aGuard;
    bool bLabelVisible = isDataLabelVisible(mxModel, aCID);
    mxCBLabel->set_active(bLabelVisible);
    mxCBTrendline->set_active(isTrendlineVisible(mxModel, aCID));
    mxCBXError->set_active(isErrorBarVisible(mxModel, aCID, false));
    mxCBYError->set_active(isErrorBarVisible(mxModel, aCID, true));

    bool bPrimaryAxis = isPrimaryAxis(mxModel, aCID);
    mxRBPrimaryAxis->set_active(bPrimaryAxis);
    mxRBSecondaryAxis->set_active(!bPrimaryAxis);

    mxBoxLabelPlacement->set_sensitive(bLabelVisible);
    mxLBLabelPlacement->set_active(getDataLabelPlacement(mxModel, aCID));

    OUString aFrameLabel = mxFTSeriesTemplate->get_label();
    aFrameLabel = aFrameLabel.replaceFirst("%1", getSeriesLabel(mxModel, aCID));
    mxFTSeriesName->set_label(aFrameLabel);
}

VclPtr<vcl::Window> ChartSeriesPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartSeriesPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartSeriesPanel::Create", nullptr, 1);

    return  VclPtr<ChartSeriesPanel>::Create(
                        pParent, rxFrame, pController);
}

void ChartSeriesPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartSeriesPanel::HandleContextChange(
    const vcl::EnumContext& )
{
    updateData();
}

void ChartSeriesPanel::NotifyItemUpdate(
    sal_uInt16 /*nSID*/,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/ )
{
}

void ChartSeriesPanel::modelInvalid()
{
    mbModelValid = false;
}

void ChartSeriesPanel::updateModel(
        css::uno::Reference<css::frame::XModel> xModel)
{
    if (mbModelValid)
    {
        css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
        xBroadcaster->removeModifyListener(mxListener);
    }

    css::uno::Reference<css::view::XSelectionSupplier> oldSelectionSupplier(
        mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (oldSelectionSupplier.is()) {
        oldSelectionSupplier->removeSelectionChangeListener(mxSelectionListener.get());
    }

    mxModel = xModel;
    mbModelValid = true;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartSeriesPanel::selectionChanged(bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

IMPL_LINK(ChartSeriesPanel, CheckBoxHdl, weld::ToggleButton&, rCheckBox, void)
{
    bool bChecked = rCheckBox.get_active();
    OUString aCID = getCID(mxModel);
    if (&rCheckBox == mxCBLabel.get())
        setDataLabelVisible(mxModel, aCID, bChecked);
    else if (&rCheckBox == mxCBTrendline.get())
        setTrendlineVisible(mxModel, aCID, bChecked);
    else if (&rCheckBox == mxCBXError.get())
        setErrorBarVisible(mxModel, aCID, false, bChecked);
    else if (&rCheckBox == mxCBYError.get())
        setErrorBarVisible(mxModel, aCID, true, bChecked);
}

IMPL_LINK_NOARG(ChartSeriesPanel, RadioBtnHdl, weld::ToggleButton&, void)
{
    OUString aCID = getCID(mxModel);
    bool bChecked = mxRBPrimaryAxis->get_active();

    setAttachedAxisType(mxModel, aCID, bChecked);
}

IMPL_LINK_NOARG(ChartSeriesPanel, ListBoxHdl, weld::ComboBox&, void)
{
    OUString aCID = getCID(mxModel);

    sal_Int32 nPos = mxLBLabelPlacement->get_active();
    setDataLabelPlacement(mxModel, aCID, nPos);
}

} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
