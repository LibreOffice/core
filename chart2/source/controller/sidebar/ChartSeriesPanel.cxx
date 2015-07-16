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

}

ChartSeriesPanel::ChartSeriesPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    ChartController* pController)
  : PanelLayout(pParent, "ChartSeriesPanel", "modules/schart/ui/sidebarseries.ui", rxFrame),
    mxFrame(rxFrame),
    maContext(),
    mpBindings(pBindings),
    mxModel(pController->getModel()),
    mxListener(new ChartSidebarModifyListener(this))
{
    get(mpCBLabel, "checkbutton_label");
    get(mpCBTrendline, "checkbutton_trendline");
    get(mpCBXError, "checkbutton_x_error");
    get(mpCBYError, "checkbutton_y_error");

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
}

void ChartSeriesPanel::updateData()
{
    css::uno::Reference<css::frame::XController> xController(mxModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return;

    uno::Any aAny = xSelectionSupplier->getSelection();
    assert(aAny.hasValue());
    OUString aCID;
    aAny >>= aCID;
#ifdef DBG_UTIL
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    assert(eType == OBJECTTYPE_DATA_SERIES);
#endif
    SolarMutexGuard aGuard;
    mpCBLabel->Check(isDataLabelVisible(mxModel, aCID));
    mpCBTrendline->Check(isTrendlineVisible(mxModel, aCID));
}

VclPtr<vcl::Window> ChartSeriesPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings, ChartController* pController)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to ChartSeriesPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartSeriesPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to ChartSeriesPanel::Create", NULL, 2);

    return  VclPtr<ChartSeriesPanel>::Create(
                        pParent, rxFrame, pBindings, pController);
}

void ChartSeriesPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartSeriesPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if(maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
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

IMPL_LINK(ChartSeriesPanel, CheckBoxHdl, CheckBox*, pCheckBox)
{
    bool bChecked = pCheckBox->IsChecked();
    css::uno::Reference<css::frame::XController> xController(mxModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return 0;

    uno::Any aAny = xSelectionSupplier->getSelection();
    assert(aAny.hasValue());
    OUString aCID;
    aAny >>= aCID;
#ifdef DBG_UTIL
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    assert(eType == OBJECTTYPE_DATA_SERIES);
#endif
    if (pCheckBox == mpCBLabel.get())
        setDataLabelVisible(mxModel, aCID, bChecked);
    else if (pCheckBox == mpCBTrendline.get())
        setTrendlineVisible(mxModel, aCID, bChecked);

    return 0;
}

}} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
