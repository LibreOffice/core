/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include "ChartAreaPanel.hxx"

#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <ViewElementListProvider.hxx>
#include <PropertyHelper.hxx>

#include <chartview/DrawModelWrapper.hxx>
#include <com/sun/star/chart2/XDiagram.hpp>

#include <sfx2/weldutils.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/unomid.hxx>
#include <vcl/svapp.hxx>

#include <svx/tbcontrl.hxx>

namespace chart::sidebar {

namespace {

SvxColorToolBoxControl* getColorToolBoxControl(const ToolbarUnoDispatcher& rColorDispatch)
{
    css::uno::Reference<css::frame::XToolbarController> xController = rColorDispatch.GetControllerForCommand(u".uno:FillColor"_ustr);
    SvxColorToolBoxControl* pToolBoxColorControl = dynamic_cast<SvxColorToolBoxControl*>(xController.get());
    return pToolBoxColorControl;
}

OUString getCID(const rtl::Reference<::chart::ChartModel>& xModel)
{
    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    css::uno::Any aAny = xSelectionSupplier->getSelection();
    if (!aAny.hasValue())
    {
        // if no selection, default to diagram wall so sidebar can show some editable properties
        ChartController* pController = dynamic_cast<ChartController*>(xController.get());
        if (pController)
        {
            pController->select( css::uno::Any( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, u"" ) ) );
            xSelectionSupplier = css::uno::Reference<css::view::XSelectionSupplier>(xController, css::uno::UNO_QUERY);
            if (xSelectionSupplier.is())
                aAny = xSelectionSupplier->getSelection();
        }

        if (!aAny.hasValue())
            return OUString();
    }

    OUString aCID;
    aAny >>= aCID;

    return aCID;
}

css::uno::Reference<css::beans::XPropertySet> getPropSet(
        const rtl::Reference<::chart::ChartModel>& xModel)
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

XFillGradientItem getXGradientForName(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rName)
{
    css::uno::Reference<css::lang::XMultiServiceFactory> xFact(xModel, css::uno::UNO_QUERY);
    css::uno::Reference<css::container::XNameAccess> xNameAccess(
            xFact->createInstance(u"com.sun.star.drawing.GradientTable"_ustr), css::uno::UNO_QUERY);
    if (!xNameAccess.is())
        return XFillGradientItem();

    if (!xNameAccess->hasByName(rName))
        return XFillGradientItem();

    css::uno::Any aAny = xNameAccess->getByName(rName);

    XFillGradientItem aItem;
    aItem.SetName(rName);
    aItem.PutValue(aAny, MID_FILLGRADIENT);

    return aItem;

}

XFillFloatTransparenceItem getXTransparencyGradientForName(const css::uno::Reference<css::frame::XModel>& xModel,
        const OUString& rName)
{
    css::uno::Reference<css::lang::XMultiServiceFactory> xFact(xModel, css::uno::UNO_QUERY);
    css::uno::Reference<css::container::XNameAccess> xNameAccess(
            xFact->createInstance(u"com.sun.star.drawing.TransparencyGradientTable"_ustr), css::uno::UNO_QUERY);
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
                return pHatch->GetHatch();
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
        std::u16string_view rName)
{
    try
    {
        ViewElementListProvider aProvider = getViewElementListProvider(xModel);
        XBitmapListRef aBmpRef = aProvider.GetBitmapList();
        XPatternListRef aPatRef = aProvider.GetPatternList();

        size_t n = aBmpRef->Count();
        for (size_t i = 0; i < n; ++i)
        {
            const XBitmapEntry* pBitmap = aBmpRef->GetBitmap(i);
            if (!pBitmap)
                continue;

            if (pBitmap->GetName().equalsIgnoreAsciiCase(rName))
            {
                return pBitmap->GetGraphicObject();
            }
        }

        // perhaps it's a pattern
        size_t m = aPatRef->Count();
        for (size_t i = 0; i < m; ++i)
        {
            const XBitmapEntry* pBitmap = aPatRef->GetBitmap(i);
            if (!pBitmap)
                continue;

            if (pBitmap->GetName().equalsIgnoreAsciiCase(rName))
            {
                return pBitmap->GetGraphicObject();
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

std::unique_ptr<PanelLayout> ChartAreaPanel::Create(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(u"no parent Window given to ChartAxisPanel::Create"_ustr, nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException(u"no XFrame given to ChartAxisPanel::Create"_ustr, nullptr, 1);

    return std::make_unique<ChartAreaPanel>(pParent, rxFrame, pController);
}

ChartAreaPanel::ChartAreaPanel(weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController):
    svx::sidebar::AreaPropertyPanelBase(pParent, rxFrame),
    mxModel(pController->getChartModel()),
    mxListener(new ChartSidebarModifyListener(this)),
    mxSelectionListener(new ChartSidebarSelectionListener(this)),
    mbUpdate(true),
    mbModelValid(true),
    maFillColorWrapper(mxModel, getColorToolBoxControl(*mxColorDispatch), u"FillColor"_ustr)
{
    std::vector<ObjectType> aAcceptedTypes { OBJECTTYPE_PAGE, OBJECTTYPE_DIAGRAM,
        OBJECTTYPE_DATA_SERIES, OBJECTTYPE_DATA_POINT,
        OBJECTTYPE_TITLE, OBJECTTYPE_LEGEND};
    mxSelectionListener->setAcceptedTypes(std::move(aAcceptedTypes));
    Initialize();
}

ChartAreaPanel::~ChartAreaPanel()
{
    doUpdateModel(nullptr);
}

void ChartAreaPanel::Initialize()
{
    mxModel->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    SvxColorToolBoxControl* pToolBoxColor = getColorToolBoxControl(*mxColorDispatch);
    pToolBoxColor->setColorSelectFunction(maFillColorWrapper);

    updateData();
}

void ChartAreaPanel::setFillTransparence(const XFillTransparenceItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue(u"FillTransparence"_ustr, css::uno::Any(rItem.GetValue()));
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
        xPropSet->setPropertyValue(u"FillTransparenceGradientName"_ustr, css::uno::Any(OUString()));
        return;
    }

    const OUString& aName = rItem.GetName();
    css::uno::Any aGradientVal;
    rItem.QueryValue(aGradientVal, MID_FILLGRADIENT);
    OUString aNewName = PropertyHelper::addTransparencyGradientUniqueNameToTable(aGradientVal, mxModel, aName);
    xPropSet->setPropertyValue(u"FillTransparenceGradientName"_ustr, css::uno::Any(aNewName));
}

void ChartAreaPanel::setFillStyle(const XFillStyleItem& rItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue(u"FillStyle"_ustr, css::uno::Any(rItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndColor(const XFillStyleItem* pStyleItem,
        const XFillColorItem& rColorItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue(u"FillStyle"_ustr, css::uno::Any(pStyleItem->GetValue()));
    xPropSet->setPropertyValue(u"FillColor"_ustr, css::uno::Any(rColorItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndGradient(const XFillStyleItem* pStyleItem,
        const XFillGradientItem& rGradientItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue(u"FillStyle"_ustr, css::uno::Any(pStyleItem->GetValue()));

    const OUString& aName = rGradientItem.GetName();
    css::uno::Any aGradientVal;
    rGradientItem.QueryValue(aGradientVal, MID_FILLGRADIENT);
    OUString aNewName = PropertyHelper::addGradientUniqueNameToTable(aGradientVal, mxModel, aName);
    xPropSet->setPropertyValue(u"FillGradientName"_ustr, css::uno::Any(aNewName));
}

void ChartAreaPanel::setFillStyleAndHatch(const XFillStyleItem* pStyleItem,
        const XFillHatchItem& rHatchItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue(u"FillStyle"_ustr, css::uno::Any(pStyleItem->GetValue()));
    xPropSet->setPropertyValue(u"FillHatchName"_ustr, css::uno::Any(rHatchItem.GetValue()));
}

void ChartAreaPanel::setFillStyleAndBitmap(const XFillStyleItem* pStyleItem,
        const XFillBitmapItem& rBitmapItem)
{
    PreventUpdate aProtector(mbUpdate);
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    if (pStyleItem)
        xPropSet->setPropertyValue(u"FillStyle"_ustr, css::uno::Any(pStyleItem->GetValue()));

    css::uno::Any aBitmap;
    rBitmapItem.QueryValue(aBitmap, MID_BITMAP);
    const OUString& aPreferredName = rBitmapItem.GetName();
    aBitmap <<= PropertyHelper::addBitmapUniqueNameToTable(aBitmap, mxModel, aPreferredName);
    xPropSet->setPropertyValue(u"FillBitmapName"_ustr, aBitmap);
}

void ChartAreaPanel::setFillUseBackground(const XFillStyleItem* pStyleItem,
                                          const XFillUseSlideBackgroundItem& /*rItem*/)
{
    setFillStyle(*pStyleItem);
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
    if (xInfo->hasPropertyByName(u"FillStyle"_ustr))
    {
        css::drawing::FillStyle eFillStyle = css::drawing::FillStyle_SOLID;
        xPropSet->getPropertyValue(u"FillStyle"_ustr) >>= eFillStyle;
        XFillStyleItem aFillStyleItem(eFillStyle);
        updateFillStyle(false, true, &aFillStyleItem);
    }

    if (xInfo->hasPropertyByName(u"FillTransparence"_ustr))
    {
        sal_uInt16 nFillTransparence = 0;
        xPropSet->getPropertyValue(u"FillTransparence"_ustr) >>= nFillTransparence;
        SfxUInt16Item aTransparenceItem(0, nFillTransparence);
        updateFillTransparence(false, true, &aTransparenceItem);
    }

    if (xInfo->hasPropertyByName(u"FillGradientName"_ustr))
    {
        OUString aGradientName;
        xPropSet->getPropertyValue(u"FillGradientName"_ustr) >>= aGradientName;
        XFillGradientItem aGradientItem = getXGradientForName(mxModel, aGradientName);
        updateFillGradient(false, true, &aGradientItem);
    }

    if (xInfo->hasPropertyByName(u"FillHatchName"_ustr))
    {
        OUString aHatchName;
        xPropSet->getPropertyValue(u"FillHatchName"_ustr) >>= aHatchName;
        XHatch aHatch = getXHatchFromName(mxModel, aHatchName);
        XFillHatchItem aHatchItem(aHatchName, aHatch);
        updateFillHatch(false, true, &aHatchItem);
    }

    if (xInfo->hasPropertyByName(u"FillBitmapName"_ustr))
    {
        OUString aBitmapName;
        xPropSet->getPropertyValue(u"FillBitmapName"_ustr) >>= aBitmapName;
        GraphicObject aBitmap = getXBitmapFromName(mxModel, aBitmapName);
        XFillBitmapItem aBitmapItem(aBitmapName, aBitmap);
        std::unique_ptr<XFillBitmapItem> pBitmapItem;
        try
        {
            DrawModelWrapper* pModelWrapper = getDrawModelWrapper(mxModel);
            if (pModelWrapper)
            {
                pBitmapItem = aBitmapItem.checkForUniqueItem(&pModelWrapper->getSdrModel());
            }
        }
        catch (...)
        {
        }
        updateFillBitmap(false, true, pBitmapItem ? pBitmapItem.get() : &aBitmapItem);
    }

    if (xInfo->hasPropertyByName(u"FillTransparenceGradientName"_ustr))
    {
        OUString aFillFloatTransparenceName;
        xPropSet->getPropertyValue(u"FillTransparenceGradientName"_ustr) >>= aFillFloatTransparenceName;
        XFillFloatTransparenceItem aFillFloatTransparenceItem = getXTransparencyGradientForName(mxModel, aFillFloatTransparenceName);
        updateFillFloatTransparence(false, true, &aFillFloatTransparenceItem);

        maFillColorWrapper.updateData();
    }

    if (xInfo->hasPropertyByName(u"FillColor"_ustr))
    {
        sal_uInt32 nFillColor = 0;
        xPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
        XFillColorItem aFillColorItem(u""_ustr, Color(ColorTransparency, nFillColor));
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

void ChartAreaPanel::doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel)
{
    if (mbModelValid)
    {
        mxModel->removeModifyListener(mxListener);

        css::uno::Reference<css::view::XSelectionSupplier> oldSelectionSupplier(
            mxModel->getCurrentController(), css::uno::UNO_QUERY);
        if (oldSelectionSupplier.is()) {
            oldSelectionSupplier->removeSelectionChangeListener(mxSelectionListener);
        }
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

void ChartAreaPanel::updateModel( css::uno::Reference<css::frame::XModel> xModel)
{
    ::chart::ChartModel* pModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
