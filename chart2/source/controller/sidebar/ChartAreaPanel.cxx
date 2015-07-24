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
#include "ViewElementListProvider.hxx"

#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>

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

ViewElementListProvider getViewElementListProvider( css::uno::Reference<css::frame::XModel> xModel)
{
    css::uno::Reference<css::frame::XController>xController = xModel->getCurrentController();
    if (!xController.is())
        throw std::exception();

    ChartController* pController = dynamic_cast<ChartController*>(xController.get());
    if (!pController)
        throw std::exception();

    ViewElementListProvider aProvider = pController->getViewElementListProvider();
    return aProvider;
}

XGradient getXGradientForName(css::uno::Reference<css::frame::XModel> xModel,
        const OUString& rName)
{
    try
    {
        ViewElementListProvider aProvider = getViewElementListProvider(xModel);
        XGradientListRef aRef = aProvider.GetGradientList();
        size_t n = aRef->Count();
        for (size_t i = 0; i < n; ++i)
        {
            XGradientEntry* pGradient = aRef->GetGradient(i);
            if (!pGradient)
                continue;

            if (pGradient->GetName() == rName)
                return XGradient(pGradient->GetGradient());
        }
    }
    catch (...)
    {
        // ignore exception
    }

    return XGradient();
}

XHatch getXHatchFromName(css::uno::Reference<css::frame::XModel> xModel,
        OUString& rName)
{
    try
    {
        ViewElementListProvider aProvider = getViewElementListProvider(xModel);
        XHatchListRef aRef = aProvider.GetHatchList();
        size_t n = aRef->Count();
        for (size_t i = 0; i < n; ++i)
        {
            XHatchEntry* pHatch = aRef->GetHatch(i);
            if (!pHatch)
                continue;

            if (pHatch->GetName().equalsIgnoreAsciiCase(rName))
            {
                // we need to update the hatch name
                rName = pHatch->GetName();
                return XHatch(pHatch->GetHatch());
            }
        }
    }
    catch (...)
    {
        // ignore exception
    }

    return XHatch();
}

class PreventUpdate
{
public:
    PreventUpdate(bool& bUpdate):
        mbUpdate(bUpdate)
    {
        mbUpdate = false;
    }

    ~PreventUpdate()
    {
        mbUpdate = true;
    }

private:
    bool& mbUpdate;
};

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
    mxListener(new ChartSidebarModifyListener(this)),
    mxSelectionListener(new ChartSidebarSelectionListener(this)),
    mbUpdate(true)
{
    Initialize();
}

ChartAreaPanel::~ChartAreaPanel()
{
    disposeOnce();
}

void ChartAreaPanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->removeSelectionChangeListener(mxSelectionListener);

    AreaPropertyPanelBase::dispose();
}

void ChartAreaPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartAreaPanel::setFillTransparence(const XFillTransparenceItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("Transparency", css::uno::makeAny(rItem.GetValue()));
}

void ChartAreaPanel::setFillFloatTransparence(
        const XFillFloatTransparenceItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("TransparencyGradientName", css::uno::makeAny(rItem.GetValue()));
}

void ChartAreaPanel::setFillStyle(const XFillStyleItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
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
    PreventUpdate aProtector(mbUpdate);
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
    PreventUpdate aProtector(mbUpdate);
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
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::makeAny(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("FillBitmapName", css::uno::makeAny(rBitmapItem.GetValue()));
}

void ChartAreaPanel::updateData()
{
    if (!mbUpdate)
        return;

    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    css::drawing::FillStyle eFillStyle = css::drawing::FillStyle_SOLID;
    xPropSet->getPropertyValue("FillStyle") >>= eFillStyle;
    XFillStyleItem aFillStyleItem(eFillStyle);
    updateFillStyle(false, true, &aFillStyleItem);

    sal_uInt16 nFillTransparence = 0;
    xPropSet->getPropertyValue("Transparency") >>= nFillTransparence;
    SfxUInt16Item aTransparenceItem(0, nFillTransparence);
    updateFillTransparence(false, true, &aTransparenceItem);

    OUString aGradientName;
    xPropSet->getPropertyValue("GradientName") >>= aGradientName;
    XGradient xGradient = getXGradientForName(mxModel, aGradientName);
    XFillGradientItem aGradientItem(aGradientName, xGradient);
    updateFillGradient(false, true, &aGradientItem);

    OUString aHatchName;
    xPropSet->getPropertyValue("HatchName") >>= aHatchName;
    XHatch xHatch = getXHatchFromName(mxModel, aHatchName);
    XFillHatchItem aHatchItem(aHatchName, xHatch);
    updateFillHatch(false, true, &aHatchItem);
}

void ChartAreaPanel::modelInvalid()
{
}

void ChartAreaPanel::selectionChanged(bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

void ChartAreaPanel::SelectionInvalid()
{
}

void ChartAreaPanel::updateModel(
        css::uno::Reference<css::frame::XModel> xModel)
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    mxModel = xModel;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
