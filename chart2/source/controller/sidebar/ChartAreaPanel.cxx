/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartAreaPanel.hxx"

#include "ChartController.hxx"

namespace chart { namespace sidebar {

namespace {

OUString getCID(css::uno::Reference<css::frame::XModel> xModel)
{
    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    css::uno::Any aAny = xSelectionSupplier->getSelection();
    if (!aAny.hasValue())
        return OUString();

    OUString aCID;
    aAny >>= aCID;

    return aCID;
}

css::uno::Reference<css::beans::XPropertySet> getPropSet(
        css::uno::Reference<css::frame::XModel> xModel)
{
    OUString aCID = getCID(xModel);
    return ObjectIdentifier::getObjectPropertySet(aCID, xModel);
}

}

VclPtr<vcl::Window> ChartAreaPanel::Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController)
{
    if (pParent == NULL)
        throw css::lang::IllegalArgumentException("no parent Window given to ChartAxisPanel::Create", NULL, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to ChartAxisPanel::Create", NULL, 1);

    return VclPtr<ChartAreaPanel>::Create(
                        pParent, rxFrame, pController);
}

ChartAreaPanel::ChartAreaPanel(vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController):
    svx::sidebar::AreaPropertyPanelBase(pParent, rxFrame),
    mxModel(pController->getModel()),
    mxListener(new ChartSidebarModifyListener(this))
{
}

ChartAreaPanel::~ChartAreaPanel()
{
    disposeOnce();
}

void ChartAreaPanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    AreaPropertyPanelBase::dispose();
}

void ChartAreaPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);
}

void ChartAreaPanel::setFillTransparence(const XFillTransparenceItem& /*rItem*/)
{

}

void ChartAreaPanel::setFillFloatTransparence(const XFillFloatTransparenceItem& /*rItem*/)
{

}

void ChartAreaPanel::setFillStyle(const XFillStyleItem& rItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("FillStyle", css::uno::makeAny(rItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndColor(const XFillStyleItem* pStyleItem,
        const XFillColorItem& rColorItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::makeAny(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("Color", css::uno::makeAny(rColorItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndGradient(const XFillStyleItem* pStyleItem,
        const XFillGradientItem& rGradientItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::makeAny(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("GradientName", css::uno::makeAny(rGradientItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndHatch(const XFillStyleItem* pStyleItem,
        const XFillHatchItem& rHatchItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::makeAny(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("HatchName", css::uno::makeAny(rHatchItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndBitmap(const XFillStyleItem* pStyleItem,
        const XFillBitmapItem& rBitmapItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::makeAny(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("FillBitmapName", css::uno::makeAny(rBitmapItem.GetValue()));
}

void ChartAreaPanel::updateData()
{
}

void ChartAreaPanel::modelInvalid()
{
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
