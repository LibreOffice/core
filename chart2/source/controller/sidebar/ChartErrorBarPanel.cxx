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

#include <sfx2/sidebar/ControlFactory.hxx>

#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

#include "ChartErrorBarPanel.hxx"
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

namespace chart { namespace sidebar {

namespace {

enum class ErrorBarDirection
{
    POSITIVE,
    NEGATIVE
};

css::uno::Reference<css::beans::XPropertySet> getErrorBarPropSet(
        const css::uno::Reference<css::frame::XModel>& xModel, const OUString& rCID)
{
    return ObjectIdentifier::getObjectPropertySet(rCID, xModel);
}

bool showPositiveError(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return false;

    css::uno::Any aAny = xPropSet->getPropertyValue("ShowPositiveError");

    if (!aAny.hasValue())
        return false;

    bool bShow = false;
    aAny >>= bShow;
    return bShow;
}

bool showNegativeError(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return false;

    css::uno::Any aAny = xPropSet->getPropertyValue("ShowNegativeError");

    if (!aAny.hasValue())
        return false;

    bool bShow = false;
    aAny >>= bShow;
    return bShow;
}

void setShowPositiveError(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, bool bShow)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("ShowPositiveError", css::uno::Any(bShow));
}

void setShowNegativeError(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, bool bShow)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("ShowNegativeError", css::uno::Any(bShow));
}

struct ErrorBarTypeMap
{
    sal_Int32 nPos;
    sal_Int32 nApi;
};

ErrorBarTypeMap aErrorBarType[] = {
    { 0, css::chart::ErrorBarStyle::ABSOLUTE },
    { 1, css::chart::ErrorBarStyle::RELATIVE },
    { 2, css::chart::ErrorBarStyle::FROM_DATA },
    { 3, css::chart::ErrorBarStyle::STANDARD_DEVIATION },
    { 4, css::chart::ErrorBarStyle::STANDARD_ERROR },
    { 5, css::chart::ErrorBarStyle::VARIANCE},
    { 6, css::chart::ErrorBarStyle::ERROR_MARGIN },
};

sal_Int32 getTypePos(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return 0;

    css::uno::Any aAny = xPropSet->getPropertyValue("ErrorBarStyle");

    if (!aAny.hasValue())
        return 0;

    sal_Int32 nApi = 0;
    aAny >>= nApi;

    for (ErrorBarTypeMap & i : aErrorBarType)
    {
        if (i.nApi == nApi)
            return i.nPos;
    }

    return 0;
}

void setTypePos(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, sal_Int32 nPos)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    sal_Int32 nApi = 0;
    for (ErrorBarTypeMap & i : aErrorBarType)
    {
        if (i.nPos == nPos)
            nApi = i.nApi;
    }

    xPropSet->setPropertyValue("ErrorBarStyle", css::uno::Any(nApi));
}

double getValue(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, ErrorBarDirection eDir)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return 0;

    OUString aName = "PositiveError";
    if (eDir == ErrorBarDirection::NEGATIVE)
        aName = "NegativeError";

    css::uno::Any aAny = xPropSet->getPropertyValue(aName);

    if (!aAny.hasValue())
        return 0;

    double nVal = 0;
    aAny >>= nVal;

    return nVal;
}

void setValue(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rCID, double nVal, ErrorBarDirection eDir)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    OUString aName = "PositiveError";
    if (eDir == ErrorBarDirection::NEGATIVE)
        aName = "NegativeError";

    xPropSet->setPropertyValue(aName, css::uno::Any(nVal));
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
#ifdef DBG_UTIL
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    assert(eType == OBJECTTYPE_DATA_ERRORS_X ||
            eType == OBJECTTYPE_DATA_ERRORS_Y ||
            eType == OBJECTTYPE_DATA_ERRORS_Z);
#endif

    return aCID;
}

}

ChartErrorBarPanel::ChartErrorBarPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
  : PanelLayout(pParent, "ChartErrorBarPanel", "modules/schart/ui/sidebarerrorbar.ui", rxFrame),
    mxModel(pController->getModel()),
    mxListener(new ChartSidebarModifyListener(this)),
    mbModelValid(true)
{

    get(mpRBPosAndNeg, "radiobutton_positive_negative");
    get(mpRBPos, "radiobutton_positive");
    get(mpRBNeg, "radiobutton_negative");

    get(mpLBType, "comboboxtext_type");

    get(mpMFPos, "spinbutton_pos");
    get(mpMFNeg, "spinbutton_neg");

    Initialize();
}

ChartErrorBarPanel::~ChartErrorBarPanel()
{
    disposeOnce();
}

void ChartErrorBarPanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    mpRBPosAndNeg.clear();
    mpRBPos.clear();
    mpRBNeg.clear();

    mpLBType.clear();

    mpMFPos.clear();
    mpMFNeg.clear();

    PanelLayout::dispose();
}

void ChartErrorBarPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);
    mpRBNeg->Check(false);
    mpRBPos->Check(false);
    mpRBPosAndNeg->Check(false);

    updateData();

    Link<RadioButton&,void> aLink = LINK(this, ChartErrorBarPanel, RadioBtnHdl);
    mpRBPosAndNeg->SetToggleHdl(aLink);
    mpRBPos->SetToggleHdl(aLink);
    mpRBNeg->SetToggleHdl(aLink);

    mpLBType->SetSelectHdl(LINK(this, ChartErrorBarPanel, ListBoxHdl));

    Link<Edit&,void> aLink2 = LINK(this, ChartErrorBarPanel, NumericFieldHdl);
    mpMFPos->SetModifyHdl(aLink2);
    mpMFNeg->SetModifyHdl(aLink2);
}

void ChartErrorBarPanel::updateData()
{
    if (!mbModelValid)
        return;

    OUString aCID = getCID(mxModel);
    bool bPos = showPositiveError(mxModel, aCID);
    bool bNeg = showNegativeError(mxModel, aCID);

    SolarMutexGuard aGuard;

    if (bPos && bNeg)
        mpRBPosAndNeg->Check();
    else if (bPos)
        mpRBPos->Check();
    else if (bNeg)
        mpRBNeg->Check();

    sal_Int32 nTypePos = getTypePos(mxModel, aCID);
    mpLBType->SelectEntryPos(nTypePos);

    if (nTypePos <= 1)
    {
        if (bPos)
            mpMFPos->Enable();
        else
            mpMFPos->Disable();

        if (bNeg)
            mpMFNeg->Enable();
        else
            mpMFNeg->Disable();

        double nValPos = getValue(mxModel, aCID, ErrorBarDirection::POSITIVE);
        double nValNeg = getValue(mxModel, aCID, ErrorBarDirection::NEGATIVE);

        mpMFPos->SetValue(nValPos);
        mpMFNeg->SetValue(nValNeg);
    }
    else
    {
        mpMFPos->Disable();
        mpMFNeg->Disable();
    }
}

VclPtr<vcl::Window> ChartErrorBarPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartErrorBarPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartErrorBarPanel::Create", nullptr, 1);

    return  VclPtr<ChartErrorBarPanel>::Create(
                        pParent, rxFrame, pController);
}

void ChartErrorBarPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartErrorBarPanel::HandleContextChange(
    const vcl::EnumContext& )
{
    updateData();
}

void ChartErrorBarPanel::NotifyItemUpdate(
    sal_uInt16 /*nSID*/,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/,
    const bool )
{
}

void ChartErrorBarPanel::modelInvalid()
{
    mbModelValid = false;
}

void ChartErrorBarPanel::updateModel(
        css::uno::Reference<css::frame::XModel> xModel)
{
    if (mbModelValid)
    {
        css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
        xBroadcaster->removeModifyListener(mxListener);
    }

    mxModel = xModel;
    mbModelValid = true;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);
}

IMPL_LINK_NOARG(ChartErrorBarPanel, RadioBtnHdl, RadioButton&, void)
{
    OUString aCID = getCID(mxModel);
    bool bPos = mpRBPosAndNeg->IsChecked() || mpRBPos->IsChecked();
    bool bNeg = mpRBPosAndNeg->IsChecked() || mpRBNeg->IsChecked();

    setShowPositiveError(mxModel, aCID, bPos);
    setShowNegativeError(mxModel, aCID, bNeg);
}

IMPL_LINK_NOARG(ChartErrorBarPanel, ListBoxHdl, ListBox&, void)
{
    OUString aCID = getCID(mxModel);
    sal_Int32 nPos = mpLBType->GetSelectEntryPos();

    setTypePos(mxModel, aCID, nPos);
}

IMPL_LINK(ChartErrorBarPanel, NumericFieldHdl, Edit&, rMetricField, void)
{
    OUString aCID = getCID(mxModel);
    double nVal = static_cast<NumericField&>(rMetricField).GetValue();
    if (&rMetricField == mpMFPos.get())
        setValue(mxModel, aCID, nVal, ErrorBarDirection::POSITIVE);
    else if (&rMetricField == mpMFNeg.get())
        setValue(mxModel, aCID, nVal, ErrorBarDirection::NEGATIVE);
}

}} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
