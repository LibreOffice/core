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

#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>

#include "ChartAxisPanel.hxx"
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

using namespace css;
using namespace css::uno;
using ::sfx2::sidebar::Theme;

namespace chart { namespace sidebar {

namespace {

bool isLabelShown(css::uno::Reference<css::frame::XModel> xModel,
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

void setLabelShown(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID, bool bVisible)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return;

    xAxis->setPropertyValue("DisplayLabels", css::uno::makeAny(bVisible));
}

struct AxisLabelPosMap
{
    sal_Int32 nPos;
    css::chart::ChartAxisLabelPosition ePos;
};

AxisLabelPosMap aLabelPosMap[] = {
    { 0, css::chart::ChartAxisLabelPosition_NEAR_AXIS },
    { 1, css::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE },
    { 2, css::chart::ChartAxisLabelPosition_OUTSIDE_START },
    { 3, css::chart::ChartAxisLabelPosition_OUTSIDE_END }
};

sal_Int32 getLabelPosition(css::uno::Reference<css::frame::XModel> xModel,
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
    for (size_t i = 0; i < SAL_N_ELEMENTS(aLabelPosMap); ++i)
    {
        if (aLabelPosMap[i].ePos == ePos)
            return aLabelPosMap[i].nPos;
    }

    return 0;
}

void setLabelPosition(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID, sal_Int32 nPos)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return;

    css::chart::ChartAxisLabelPosition ePos;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aLabelPosMap); ++i)
    {
        if (aLabelPosMap[i].nPos == nPos)
            ePos = aLabelPosMap[i].ePos;
    }

    xAxis->setPropertyValue("LabelPosition", css::uno::makeAny(ePos));
}

bool isReverse(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID)
{
    css::uno::Reference< css::chart2::XAxis > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return false;

    css::chart2::ScaleData aData = xAxis->getScaleData();

    return aData.Orientation == css::chart2::AxisOrientation_REVERSE;
}

void setReverse(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID, bool bReverse)
{
    css::uno::Reference< css::chart2::XAxis > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return;

    css::chart2::ScaleData aData = xAxis->getScaleData();
    if (bReverse)
        aData.Orientation = css::chart2::AxisOrientation_REVERSE;
    else
        aData.Orientation = css::chart2::AxisOrientation_MATHEMATICAL;

    xAxis->setScaleData(aData);
}

OUString getCID(css::uno::Reference<css::frame::XModel> xModel)
{
    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    uno::Any aAny = xSelectionSupplier->getSelection();
    assert(aAny.hasValue());
    OUString aCID;
    aAny >>= aCID;
#ifdef DBG_UTIL
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    assert(eType == OBJECTTYPE_AXIS);
#endif

    return aCID;
}

void setAxisRotation(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID, double nVal)
{
    css::uno::Reference< css::beans::XPropertySet > xAxis(
        ObjectIdentifier::getAxisForCID(rCID, xModel), uno::UNO_QUERY );

    if (!xAxis.is())
        return;

    xAxis->setPropertyValue("TextRotation", css::uno::makeAny(nVal));
}

double getAxisRotation(css::uno::Reference<css::frame::XModel> xModel,
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
  : PanelLayout(pParent, "ChartAxisPanel", "modules/schart/ui/sidebaraxis.ui", rxFrame),
    mxFrame(rxFrame),
    mxModel(pController->getModel()),
    mxModifyListener(new ChartSidebarModifyListener(this)),
    mxSelectionListener(new ChartSidebarSelectionListener(this, OBJECTTYPE_AXIS)),
    mbModelValid(true)
{
    get(mpCBShowLabel, "checkbutton_show_label");
    get(mpCBReverse, "checkbutton_reverse");

    get(mpLBLabelPos, "comboboxtext_label_position");
    get(mpNFRotation, "spinbutton1");
    get(mpGridLabel, "label_props");

    Initialize();
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

    mpCBShowLabel.clear();
    mpCBReverse.clear();

    mpLBLabelPos.clear();
    mpGridLabel.clear();

    mpNFRotation.clear();

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

    Link<Button*,void> aLink = LINK(this, ChartAxisPanel, CheckBoxHdl);
    mpCBShowLabel->SetClickHdl(aLink);
    mpCBReverse->SetClickHdl(aLink);

    Link<Edit&, void> aSpinButtonLink = LINK(this, ChartAxisPanel, TextRotationHdl);
    mpNFRotation->SetModifyHdl(aSpinButtonLink);

    mpLBLabelPos->SetSelectHdl(LINK(this, ChartAxisPanel, ListBoxHdl));
}

void ChartAxisPanel::updateData()
{
    if (!mbModelValid)
        return;

    OUString aCID = getCID(mxModel);
    SolarMutexGuard aGuard;

    mpCBShowLabel->Check(isLabelShown(mxModel, aCID));
    mpCBReverse->Check(isReverse(mxModel, aCID));

    mpLBLabelPos->SelectEntryPos(getLabelPosition(mxModel, aCID));
    mpNFRotation->SetValue(getAxisRotation(mxModel, aCID));
}

VclPtr<vcl::Window> ChartAxisPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to ChartAxisPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartAxisPanel::Create", NULL, 1);

    return  VclPtr<ChartAxisPanel>::Create(
                        pParent, rxFrame, pController);
}

void ChartAxisPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartAxisPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& )
{
    updateData();
}

void ChartAxisPanel::NotifyItemUpdate(
    sal_uInt16 /*nSID*/,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/,
    const bool )
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

void ChartAxisPanel::SelectionInvalid()
{
}

IMPL_LINK_TYPED(ChartAxisPanel, CheckBoxHdl, Button*, pButton, void)
{
    CheckBox* pCheckbox = static_cast<CheckBox*>(pButton);
    OUString aCID = getCID(mxModel);
    bool bChecked = pCheckbox->IsChecked();

    if (pCheckbox == mpCBShowLabel.get())
    {
        mpGridLabel->Enable(bChecked);
        setLabelShown(mxModel, aCID, bChecked);
    }
    else if (pCheckbox == mpCBReverse.get())
        setReverse(mxModel, aCID, bChecked);
}

IMPL_LINK_NOARG_TYPED(ChartAxisPanel, ListBoxHdl, ListBox&, void)
{
    OUString aCID = getCID(mxModel);
    sal_Int32 nPos = mpLBLabelPos->GetSelectEntryPos();

    setLabelPosition(mxModel, aCID, nPos);
}

IMPL_LINK_TYPED(ChartAxisPanel, TextRotationHdl, Edit&, rMetricField, void)
{
    OUString aCID = getCID(mxModel);
    double nVal = static_cast<NumericField&>(rMetricField).GetValue();
    setAxisRotation(mxModel, aCID, nVal);
}

}} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
