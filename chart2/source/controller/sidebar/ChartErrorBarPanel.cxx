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
using ::sfx2::sidebar::Theme;

namespace chart { namespace sidebar {

namespace {

css::uno::Reference<css::beans::XPropertySet> getErrorBarPropSet(
        css::uno::Reference<css::frame::XModel> xModel, const OUString& rCID)
{
    return ObjectIdentifier::getObjectPropertySet(rCID, xModel);
}

bool showPositiveError(css::uno::Reference<css::frame::XModel> xModel,
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

bool showNegativeError(css::uno::Reference<css::frame::XModel> xModel,
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

void setShowPositiveError(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID, bool bShow)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("ShowPositiveError", css::uno::makeAny(bShow));
}

void setShowNegativeError(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rCID, bool bShow)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getErrorBarPropSet(xModel, rCID);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("ShowNegativeError", css::uno::makeAny(bShow));
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
    mxFrame(rxFrame),
    mxModel(pController->getModel()),
    mxListener(new ChartSidebarModifyListener(this))
{

    get(mpRBPosAndNeg, "radiobutton_positive_negative");
    get(mpRBPos, "radiobutton_positive");
    get(mpRBNeg, "radiobutton_negative");

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

    PanelLayout::dispose();
}

void ChartErrorBarPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);

    updateData();

    Link<> aLink = LINK(this, ChartErrorBarPanel, RadioBtnHdl);
    mpRBPosAndNeg->SetToggleHdl(aLink);
    mpRBPos->SetToggleHdl(aLink);
    mpRBNeg->SetToggleHdl(aLink);
}

void ChartErrorBarPanel::updateData()
{
    OUString aCID = getCID(mxModel);
    bool bPos = showPositiveError(mxModel, aCID);
    bool bNeg = showNegativeError(mxModel, aCID);

    SolarMutexGuard aGuard;

    if (bPos && bNeg)
        mpRBPosAndNeg->Check(true);
    else if (bPos)
        mpRBPos->Check(true);
    else if (bNeg)
        mpRBNeg->Check(true);
}

VclPtr<vcl::Window> ChartErrorBarPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to ChartErrorBarPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartErrorBarPanel::Create", NULL, 1);

    return  VclPtr<ChartErrorBarPanel>::Create(
                        pParent, rxFrame, pController);
}

void ChartErrorBarPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartErrorBarPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& )
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
}

IMPL_LINK_NOARG(ChartErrorBarPanel, RadioBtnHdl)
{
    OUString aCID = getCID(mxModel);
    bool bPos = mpRBPosAndNeg->IsChecked() || mpRBPos->IsChecked();
    bool bNeg = mpRBPosAndNeg->IsChecked() || mpRBNeg->IsChecked();

    setShowPositiveError(mxModel, aCID, bPos);
    setShowNegativeError(mxModel, aCID, bNeg);
}

}} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
