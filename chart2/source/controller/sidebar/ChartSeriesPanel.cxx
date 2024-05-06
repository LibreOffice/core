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
#include <com/sun/star/chart/DataLabelPlacement.hpp>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include "ChartSeriesPanel.hxx"
#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <ChartType.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <Diagram.hxx>
#include <RegressionCurveHelper.hxx>
#include <RegressionCurveModel.hxx>
#include <StatisticsHelper.hxx>
#include <BaseCoordinateSystem.hxx>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace chart::sidebar {

namespace {

bool isDataLabelVisible(const rtl::Reference<::chart::ChartModel>& xModel, std::u16string_view rCID)
{
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return false;

    return DataSeriesHelper::hasDataLabelsAtSeries(xSeries);
}

void setDataLabelVisible(const rtl::Reference<::chart::ChartModel>& xModel, std::u16string_view rCID, bool bVisible)
{
    rtl::Reference< DataSeries > xSeries =
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

sal_Int32 getDataLabelPlacement(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return 0;

    css::uno::Any aAny = xSeries->getPropertyValue(u"LabelPlacement"_ustr);
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

void setDataLabelPlacement(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, sal_Int32 nPos)
{
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

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

    xSeries->setPropertyValue(u"LabelPlacement"_ustr, css::uno::Any(nApi));
}

bool isTrendlineVisible(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    rtl::Reference< DataSeries > xRegressionCurveContainer =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xRegressionCurveContainer.is())
        return false;

    return !xRegressionCurveContainer->getRegressionCurves2().empty();
}

void setTrendlineVisible(const rtl::Reference<::chart::ChartModel>&
        xModel, std::u16string_view rCID, bool bVisible)
{
    rtl::Reference< DataSeries > xRegressionCurveContainer =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

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

bool isErrorBarVisible(const rtl::Reference<::chart::ChartModel>& xModel,
                       std::u16string_view rCID, bool bYError)
{
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return false;

    return StatisticsHelper::hasErrorBars(xSeries, bYError);
}

void setErrorBarVisible(const rtl::Reference<::chart::ChartModel>&
        xModel, std::u16string_view rCID, bool bYError, bool bVisible)
{
    rtl::Reference< DataSeries > xSeries =
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

bool isPrimaryAxis(const rtl::Reference<::chart::ChartModel>&
        xModel, std::u16string_view rCID)
{
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return true;

    return DataSeriesHelper::getAttachedAxisIndex(xSeries) == 0;
}

void setAttachedAxisType(const rtl::Reference<::chart::ChartModel>&
        xModel, std::u16string_view rCID, bool bPrimary)
{
    const rtl::Reference<DataSeries> xDataSeries = ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xDataSeries.is())
        return;

    rtl::Reference<Diagram> xDiagram = xModel->getFirstChartDiagram();
    xDiagram->attachSeriesToAxis(bPrimary, xDataSeries, comphelper::getProcessComponentContext());
}

rtl::Reference<ChartType> getChartType(
        const rtl::Reference<::chart::ChartModel>& xModel)
{
    rtl::Reference<Diagram> xDiagram = xModel->getFirstChartDiagram();
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & xCooSysSequence( xDiagram->getBaseCoordinateSystems());
    return xCooSysSequence[0]->getChartTypes2()[0];
}

OUString getSeriesLabel(const rtl::Reference<::chart::ChartModel>& xModel, std::u16string_view rCID)
{
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID(rCID, xModel);

    if (!xSeries.is())
        return OUString();

    rtl::Reference<ChartType> xChartType = getChartType(xModel);
    return xSeries->getLabelForRole(xChartType->getRoleOfSequenceForSeriesLabel());
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
    weld::Widget* pParent,
    ChartController* pController)
    : PanelLayout(pParent, u"ChartSeriesPanel"_ustr, u"modules/schart/ui/sidebarseries.ui"_ustr)
    , mxCBLabel(m_xBuilder->weld_check_button(u"checkbutton_label"_ustr))
    , mxCBTrendline(m_xBuilder->weld_check_button(u"checkbutton_trendline"_ustr))
    , mxCBXError(m_xBuilder->weld_check_button(u"checkbutton_x_error"_ustr))
    , mxCBYError(m_xBuilder->weld_check_button(u"checkbutton_y_error"_ustr))
    , mxRBPrimaryAxis(m_xBuilder->weld_radio_button(u"radiobutton_primary_axis"_ustr))
    , mxRBSecondaryAxis(m_xBuilder->weld_radio_button(u"radiobutton_secondary_axis"_ustr))
    , mxBoxLabelPlacement(m_xBuilder->weld_widget(u"datalabel_box"_ustr))
    , mxLBLabelPlacement(m_xBuilder->weld_combo_box(u"comboboxtext_label"_ustr))
    , mxFTSeriesName(m_xBuilder->weld_label(u"label_series_name"_ustr))
    , mxFTSeriesTemplate(m_xBuilder->weld_label(u"label_series_tmpl"_ustr))
    , mxModel(pController->getChartModel())
    , mxListener(new ChartSidebarModifyListener(this))
    , mxSelectionListener(new ChartSidebarSelectionListener(this, OBJECTTYPE_DATA_SERIES))
    , mbModelValid(true)
{
    Initialize();
}

ChartSeriesPanel::~ChartSeriesPanel()
{
    doUpdateModel(nullptr);

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
}

void ChartSeriesPanel::Initialize()
{
    mxModel->addModifyListener(mxListener);
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    updateData();

    Link<weld::Toggleable&,void> aLink = LINK(this, ChartSeriesPanel, CheckBoxHdl);
    mxCBLabel->connect_toggled(aLink);
    mxCBTrendline->connect_toggled(aLink);
    mxCBXError->connect_toggled(aLink);
    mxCBYError->connect_toggled(aLink);

    Link<weld::Toggleable&,void> aLink2 = LINK(this, ChartSeriesPanel, RadioBtnHdl);
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

std::unique_ptr<PanelLayout> ChartSeriesPanel::Create (
    weld::Widget* pParent,
    ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(u"no parent Window given to ChartSeriesPanel::Create"_ustr, nullptr, 0);

    return std::make_unique<ChartSeriesPanel>(pParent, pController);
}

void ChartSeriesPanel::DataChanged(const DataChangedEvent& rEvent)
{
    PanelLayout::DataChanged(rEvent);
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

void ChartSeriesPanel::doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel)
{
    if (mbModelValid)
    {
        mxModel->removeModifyListener(mxListener);
    }

    css::uno::Reference<css::view::XSelectionSupplier> oldSelectionSupplier(
        mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (oldSelectionSupplier.is()) {
        oldSelectionSupplier->removeSelectionChangeListener(mxSelectionListener);
    }

    mxModel = xModel;
    mbModelValid = mxModel.is();

    if (!mbModelValid)
        return;

    mxModel->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartSeriesPanel::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    ::chart::ChartModel* pModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}

void ChartSeriesPanel::selectionChanged(bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

IMPL_LINK(ChartSeriesPanel, CheckBoxHdl, weld::Toggleable&, rCheckBox, void)
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

IMPL_LINK_NOARG(ChartSeriesPanel, RadioBtnHdl, weld::Toggleable&, void)
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
