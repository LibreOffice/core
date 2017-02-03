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
#include "PropertyHelper.hxx"

#include "chartview/DrawModelWrapper.hxx"

#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/unomid.hxx>
#include <vcl/svapp.hxx>

#include <svx/tbcontrl.hxx>

namespace chart { namespace sidebar {

namespace {

SvxColorToolBoxControl* getColorToolBoxControl(sfx2::sidebar::SidebarToolBox* pToolBoxColor)
{
    css::uno::Reference<css::frame::XToolbarController> xController = pToolBoxColor->GetFirstController();
    SvxColorToolBoxControl* pToolBoxColorControl = dynamic_cast<SvxColorToolBoxControl*>(xController.get());
    return pToolBoxColorControl;
}

OUString getCID(const css::uno::Reference<css::frame::XModel>& xModel)
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
        const css::uno::Reference<css::frame::XModel>& xModel)
{
    OUString aCID = getCID(xModel);
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        ObjectIdentifier::getObjectPropertySet(aCID, xModel);

    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType == OBJECTTYPE_DIAGRAM)
    {
        css::uno::Reference<css::chart2::XDiagram> xDiagram(
                xPropSet, css::uno::UNO_QUERY);
        if (!xDiagram.is())
            return xPropSet;

        xPropSet.set(xDiagram->getWall());
    }

    return xPropSet;
}

ChartController* getController(const css::uno::Reference<css::frame::XModel>& xModel)
{
    css::uno::Reference<css::frame::XController>xController = xModel->getCurrentController();
    if (!xController.is())
        throw std::exception();

    ChartController* pController = dynamic_cast<ChartController*>(xController.get());
    if (!pController)
        throw std::exception();

    return pController;
}

ViewElementListProvider getViewElementListProvider( const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartController* pController = getController(xModel);
    ViewElementListProvider aProvider = pController->getViewElementListProvider();
    return aProvider;
}

DrawModelWrapper* getDrawModelWrapper(const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartController* pController = getController(xModel);
    return pController->GetDrawModelWrapper();
}

XGradient getXGradientForName(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rName)
{
    try
    {
        ViewElementListProvider aProvider = getViewElementListProvider(xModel);
        XGradientListRef aRef = aProvider.GetGradientList();
        size_t n = aRef->Count();
        for (size_t i = 0; i < n; ++i)
        {
            const XGradientEntry* pGradient = aRef->GetGradient(i);
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

XFillFloatTransparenceItem getXTransparencyGradientForName(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rName)
{
    css::uno::Reference<css::lang::XMultiServiceFactory> xFact(xModel, css::uno::UNO_QUERY);
    css::uno::Reference<css::container::XNameAccess> xNameAccess(
            xFact->createInstance("com.sun.star.drawing.TransparencyGradientTable"), css::uno::UNO_QUERY);
    if (!xNameAccess.is())
        return XFillFloatTransparenceItem();

    if (!xNameAccess->hasByName(rName))
        return XFillFloatTransparenceItem();

    css::uno::Any aAny = xNameAccess->getByName(rName);

    XFillFloatTransparenceItem aItem;
    aItem.SetName(rName);
    aItem.PutValue(aAny, MID_FILLGRADIENT);
    aItem.SetEnabled(true);

    return aItem;
}

XHatch getXHatchFromName(const css::uno::Reference<css::frame::XModel>& xModel,
        OUString& rName)
{
    try
    {
        ViewElementListProvider aProvider = getViewElementListProvider(xModel);
        XHatchListRef aRef = aProvider.GetHatchList();
        size_t n = aRef->Count();
        for (size_t i = 0; i < n; ++i)
        {
            const XHatchEntry* pHatch = aRef->GetHatch(i);
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

GraphicObject getXBitmapFromName(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rName)
{
    try
    {
        ViewElementListProvider aProvider = getViewElementListProvider(xModel);
        XBitmapListRef aRef = aProvider.GetBitmapList();
        size_t n = aRef->Count();
        for (size_t i = 0; i < n; ++i)
        {
            const XBitmapEntry* pBitmap = aRef->GetBitmap(i);
            if (!pBitmap)
                continue;

            if (pBitmap->GetName().equalsIgnoreAsciiCase(rName))
            {
                return GraphicObject(pBitmap->GetGraphicObject());
            }
        }
    }
    catch (...)
    {
        // ignore exception
    }

    return GraphicObject();
}

class PreventUpdate
{
public:
    explicit PreventUpdate(bool& bUpdate):
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
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException("no parent Window given to ChartAxisPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to ChartAxisPanel::Create", nullptr, 1);

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
    mbUpdate(true),
    mbModelValid(true),
    maFillColorWrapper(mxModel, getColorToolBoxControl(mpToolBoxColor.get()), "FillColor")
{
    std::vector<ObjectType> aAcceptedTypes { OBJECTTYPE_PAGE, OBJECTTYPE_DIAGRAM,
        OBJECTTYPE_DATA_SERIES, OBJECTTYPE_DATA_POINT,
        OBJECTTYPE_TITLE, OBJECTTYPE_LEGEND};
    mxSelectionListener->setAcceptedTypes(aAcceptedTypes);
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
        xSelectionSupplier->removeSelectionChangeListener(mxSelectionListener.get());

    AreaPropertyPanelBase::dispose();
}

void ChartAreaPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener.get());

    SvxColorToolBoxControl* pToolBoxColor = getColorToolBoxControl(mpToolBoxColor.get());
    pToolBoxColor->setColorSelectFunction(maFillColorWrapper);

    updateData();
}

void ChartAreaPanel::setFillTransparence(const XFillTransparenceItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("FillTransparence", css::uno::Any(rItem.GetValue()));
}

void ChartAreaPanel::setFillFloatTransparence(
        const XFillFloatTransparenceItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (!rItem.IsEnabled())
    {
        xPropSet->setPropertyValue("FillTransparenceGradientName", css::uno::Any(OUString()));
        return;
    }

    OUString aName = rItem.GetName();
    css::uno::Any aGradientVal;
    rItem.QueryValue(aGradientVal, MID_FILLGRADIENT);
    OUString aNewName = PropertyHelper::addTransparencyGradientUniqueNameToTable(aGradientVal, css::uno::Reference<css::lang::XMultiServiceFactory>(mxModel, css::uno::UNO_QUERY_THROW), aName);
    xPropSet->setPropertyValue("FillTransparenceGradientName", css::uno::Any(aNewName));
}

void ChartAreaPanel::setFillStyle(const XFillStyleItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("FillStyle", css::uno::Any(rItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndColor(const XFillStyleItem* pStyleItem,
        const XFillColorItem& rColorItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::Any(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("FillColor", css::uno::Any(rColorItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndGradient(const XFillStyleItem* pStyleItem,
        const XFillGradientItem& rGradientItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::Any(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("FillGradientName", css::uno::Any(rGradientItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndHatch(const XFillStyleItem* pStyleItem,
        const XFillHatchItem& rHatchItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::Any(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("FillHatchName", css::uno::Any(rHatchItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndBitmap(const XFillStyleItem* pStyleItem,
        const XFillBitmapItem& rBitmapItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue("FillStyle", css::uno::Any(pStyleItem->GetValue()));
    xPropSet->setPropertyValue("FillBitmapName", css::uno::Any(rBitmapItem.GetValue()));
}

void ChartAreaPanel::updateData()
{
    if (!mbUpdate || !mbModelValid)
        return;

    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    css::uno::Reference<css::beans::XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
    if (!xInfo.is())
        return;

    SolarMutexGuard aGuard;
    if (xInfo->hasPropertyByName("FillStyle"))
    {
        css::drawing::FillStyle eFillStyle = css::drawing::FillStyle_SOLID;
        xPropSet->getPropertyValue("FillStyle") >>= eFillStyle;
        XFillStyleItem aFillStyleItem(eFillStyle);
        updateFillStyle(false, true, &aFillStyleItem);
    }

    if (xInfo->hasPropertyByName("FillTransparence"))
    {
        sal_uInt16 nFillTransparence = 0;
        xPropSet->getPropertyValue("FillTransparence") >>= nFillTransparence;
        SfxUInt16Item aTransparenceItem(0, nFillTransparence);
        updateFillTransparence(false, true, &aTransparenceItem);
    }

    if (xInfo->hasPropertyByName("FillGradientName"))
    {
       OUString aGradientName;
       xPropSet->getPropertyValue("FillGradientName") >>= aGradientName;
       XGradient aGradient = getXGradientForName(mxModel, aGradientName);
       XFillGradientItem aGradientItem(aGradientName, aGradient);
       updateFillGradient(false, true, &aGradientItem);
    }

    if (xInfo->hasPropertyByName("FillHatchName"))
    {
        OUString aHatchName;
        xPropSet->getPropertyValue("FillHatchName") >>= aHatchName;
        XHatch aHatch = getXHatchFromName(mxModel, aHatchName);
        XFillHatchItem aHatchItem(aHatchName, aHatch);
        updateFillHatch(false, true, &aHatchItem);
    }

    if (xInfo->hasPropertyByName("FillBitmapName"))
    {
        OUString aBitmapName;
        xPropSet->getPropertyValue("FillBitmapName") >>= aBitmapName;
        GraphicObject aBitmap = getXBitmapFromName(mxModel, aBitmapName);
        XFillBitmapItem aBitmapItem(aBitmapName, aBitmap);
        XFillBitmapItem* pBitmapItem = nullptr;
        DrawModelWrapper* pModelWrapper = nullptr;
        try
        {
            pModelWrapper = getDrawModelWrapper(mxModel);
            if (pModelWrapper)
            {
                pBitmapItem = aBitmapItem.checkForUniqueItem(&pModelWrapper->getSdrModel());
            }
        }
        catch (...)
        {
        }
        updateFillBitmap(false, true, pBitmapItem ? pBitmapItem : &aBitmapItem);
        delete pBitmapItem;
    }

    if (xInfo->hasPropertyByName("FillTransparenceGradientName"))
    {
        OUString aFillFloatTransparenceName;
        xPropSet->getPropertyValue("FillTransparenceGradientName") >>= aFillFloatTransparenceName;
        XFillFloatTransparenceItem aFillFloatTransparenceItem = getXTransparencyGradientForName(mxModel, aFillFloatTransparenceName);
        updateFillFloatTransparence(false, true, &aFillFloatTransparenceItem);

        maFillColorWrapper.updateData();
    }

    if (xInfo->hasPropertyByName("FillColor"))
    {
        sal_uInt32 nFillColor = 0;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        XFillColorItem aFillColorItem("", Color(nFillColor));
        updateFillColor(true, &aFillColorItem);
    }
}

void ChartAreaPanel::modelInvalid()
{
    mbModelValid = false;
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
    if (mbModelValid)
    {
        css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
        xBroadcaster->removeModifyListener(mxListener);
    }

    mxModel = xModel;
    mbModelValid = true;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener.get());
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
