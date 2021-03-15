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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart2/AxisOrientation.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include "ChartAxisPanel.hxx"
#include <ChartController.hxx>

using namespace css;
using namespace css::uno;

namespace chart::sidebar {

namespace {

bool isLabelShown(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return false;

    uno::Any aAny = xAxis->getPropertyValue("DisplayLabels");
    if (!aAny.hasValue())
        return false;

    bool bVisible = false;
    aAny >>= bVisible;
    return bVisible;
}

void setLabelShown(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, bool bVisible)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return;

    xAxis->setPropertyValue("DisplayLabels", css::uno::Any(bVisible));
}

struct AxisLabelPosMap
{
    sal_Int32 nPos;
    css::chart::ChartAxisLabelPosition ePos;
};

AxisLabelPosMap const aLabelPosMap[] = {
    { 0, css::chart::ChartAxisLabelPosition_NEAR_AXIS },
    { 1, css::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE },
    { 2, css::chart::ChartAxisLabelPosition_OUTSIDE_START },
    { 3, css::chart::ChartAxisLabelPosition_OUTSIDE_END }
};

sal_Int32 getLabelPosition(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return 0;

    uno::Any aAny = xAxis->getPropertyValue("LabelPosition");
    if (!aAny.hasValue())
        return 0;

    css::chart::ChartAxisLabelPosition ePos;
    aAny >>= ePos;
    for (AxisLabelPosMap const & i : aLabelPosMap)
    {
        if (i.ePos == ePos)
            return i.nPos;
    }

    return 0;
}

void setLabelPosition(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, sal_Int32 nPos)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return;

    css::chart::ChartAxisLabelPosition ePos;
    for (AxisLabelPosMap const & i : aLabelPosMap)
    {
        if (i.nPos == nPos)
            ePos = i.ePos;
    }

    xAxis->setPropertyValue("LabelPosition", css::uno::Any(ePos));
}

bool isReverse(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID)
{
    css::uno::Reference< css::chart2::XAxis > xAxis =
        ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return false;

    css::chart2::ScaleData aData = xAxis->getScaleData();

    return aData.Orientation == css::chart2::AxisOrientation_REVERSE;
}

void setReverse(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, bool bReverse)
{
    css::uno::Reference< css::chart2::XAxis > xAxis =
        ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return;

    css::chart2::ScaleData aData = xAxis->getScaleData();
    if (bReverse)
        aData.Orientation = css::chart2::AxisOrientation_REVERSE;
    else
        aData.Orientation = css::chart2::AxisOrientation_MATHEMATICAL;

    xAxis->setScaleData(aData);
}

OUString getCID(const css::uno::Reference<css::frame::XModel>& xModel)
{
    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    uno::Any aAny = xSelectionSupplier->getSelection();
    assert(aAny.hasValue());
    OUString aCID;
    aAny >>= aCID;
#if defined DBG_UTIL && !defined NDEBUG
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if(eType != OBJECTTYPE_AXIS)
        SAL_WARN("chart2","Selected item is not an axis");
#endif

    return aCID;
}

void setAxisRotation(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, double nVal)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return;

    xAxis->setPropertyValue("TextRotation", css::uno::Any(nVal));
}

double getAxisRotation(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return 0;

    css::uno::Any aAny = xAxis->getPropertyValue("TextRotation");
    double nVal = 0;
    aAny >>= nVal;
    return nVal;
}

}

ChartAxisPanel::ChartAxisPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
    : PanelLayout(pParent, "ChartAxisPanel", "modules/schart/ui/sidebaraxis.ui", rxFrame)
    , mxCBShowLabel(m_xBuilder->weld_check_button("checkbutton_show_label"))
    , mxCBReverse(m_xBuilder->weld_check_button("checkbutton_reverse"))
    , mxLBLabelPos(m_xBuilder->weld_combo_box("comboboxtext_label_position"))
    , mxGridLabel(m_xBuilder->weld_widget("label_props"))
    , mxNFRotation(m_xBuilder->weld_metric_spin_button("spinbutton1", FieldUnit::DEGREE))
    , mxModel(pController->getModel())
    , mxModifyListener(new ChartSidebarModifyListener(this))
    , mxSelectionListener(new ChartSidebarSelectionListener(this, OBJECTTYPE_AXIS))
    , mbModelValid(true)
{
    Initialize();

    m_pInitialFocusWidget = mxCBShowLabel.get();
}

ChartAxisPanel::~ChartAxisPanel()
{
    disposeOnce();
}

void ChartAxisPanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxModifyListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->removeSelectionChangeListener(mxSelectionListener);

    mxCBShowLabel.reset();
    mxCBReverse.reset();

    mxLBLabelPos.reset();
    mxGridLabel.reset();

    mxNFRotation.reset();

    PanelLayout::dispose();
}

void ChartAxisPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxModifyListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    updateData();

    Link<weld::ToggleButton&,void> aLink = LINK(this, ChartAxisPanel, CheckBoxHdl);
    mxCBShowLabel->connect_toggled(aLink);
    mxCBReverse->connect_toggled(aLink);

    Link<weld::MetricSpinButton&, void> aSpinButtonLink = LINK(this, ChartAxisPanel, TextRotationHdl);
    mxNFRotation->connect_value_changed(aSpinButtonLink);

    mxLBLabelPos->connect_changed(LINK(this, ChartAxisPanel, ListBoxHdl));
}

void ChartAxisPanel::updateData()
{
    if (!mbModelValid)
        return;

    OUString aCID = getCID(mxModel);
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType!=OBJECTTYPE_AXIS)
        return;

    SolarMutexGuard aGuard;

    mxCBShowLabel->set_active(isLabelShown(mxModel, aCID));
    mxCBReverse->set_active(isReverse(mxModel, aCID));

    mxLBLabelPos->set_active(getLabelPosition(mxModel, aCID));
    mxNFRotation->set_value(getAxisRotation(mxModel, aCID), FieldUnit::DEGREE);
}

VclPtr<vcl::Window> ChartAxisPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartAxisPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartAxisPanel::Create", nullptr, 1);

    return  VclPtr<ChartAxisPanel>::Create(
                        pParent, rxFrame, pController);
}

void ChartAxisPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartAxisPanel::HandleContextChange(
    const vcl::EnumContext& )
{
    updateData();
}

void ChartAxisPanel::NotifyItemUpdate(
    sal_uInt16 /*nSID*/,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/ )
{
}

void ChartAxisPanel::modelInvalid()
{
    mbModelValid = false;
}

void ChartAxisPanel::updateModel(
        css::uno::Reference<css::frame::XModel> xModel)
{
    if (mbModelValid)
    {
        css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
        xBroadcaster->removeModifyListener(mxModifyListener);
    }

    css::uno::Reference<css::view::XSelectionSupplier> oldSelectionSupplier(
        mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (oldSelectionSupplier.is()) {
        oldSelectionSupplier->removeSelectionChangeListener(mxSelectionListener.get());
    }

    mxModel = xModel;
    mbModelValid = true;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxModifyListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartAxisPanel::selectionChanged(bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

IMPL_LINK(ChartAxisPanel, CheckBoxHdl, weld::ToggleButton&, rCheckbox, void)
{
    OUString aCID = getCID(mxModel);
    bool bChecked = rCheckbox.get_active();

    if (&rCheckbox == mxCBShowLabel.get())
    {
        mxGridLabel->set_sensitive(bChecked);
        setLabelShown(mxModel, aCID, bChecked);
    }
    else if (&rCheckbox == mxCBReverse.get())
        setReverse(mxModel, aCID, bChecked);
}

IMPL_LINK_NOARG(ChartAxisPanel, ListBoxHdl, weld::ComboBox&, void)
{
    OUString aCID = getCID(mxModel);
    sal_Int32 nPos = mxLBLabelPos->get_active();

    setLabelPosition(mxModel, aCID, nPos);
}

IMPL_LINK(ChartAxisPanel, TextRotationHdl, weld::MetricSpinButton&, rMetricField, void)
{
    OUString aCID = getCID(mxModel);
    double nVal = rMetricField.get_value(FieldUnit::DEGREE);
    setAxisRotation(mxModel, aCID, nVal);
}

} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
