/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartLinePanel.hxx"

#include <ChartController.hxx>
#include <ChartModel.hxx>

#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>

#include <svx/linectrl.hxx>
#include <svx/tbcontrl.hxx>
#include <sfx2/weldutils.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>

#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

namespace chart::sidebar {

namespace {

SvxLineStyleToolBoxControl* getLineStyleToolBoxControl(const ToolbarUnoDispatcher& rToolBoxColor)
{
    css::uno::Reference<css::frame::XToolbarController> xController = rToolBoxColor.GetControllerForCommand(".uno:XLineStyle");
    SvxLineStyleToolBoxControl* pToolBoxLineStyleControl = dynamic_cast<SvxLineStyleToolBoxControl*>(xController.get());
    return pToolBoxLineStyleControl;
}

SvxColorToolBoxControl* getColorToolBoxControl(const ToolbarUnoDispatcher& rToolBoxLineStyle)
{
    css::uno::Reference<css::frame::XToolbarController> xController = rToolBoxLineStyle.GetControllerForCommand(".uno:XLineColor");
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
        return OUString();

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

std::unique_ptr<PanelLayout> ChartLinePanel::Create(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException("no parent Window given to ChartAxisPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to ChartAxisPanel::Create", nullptr, 1);

    return std::make_unique<ChartLinePanel>(pParent, rxFrame, pController);
}

ChartLinePanel::ChartLinePanel(weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController):
    svx::sidebar::LinePropertyPanelBase(pParent, rxFrame),
    mxModel(pController->getChartModel()),
    mxListener(new ChartSidebarModifyListener(this)),
    mxSelectionListener(new ChartSidebarSelectionListener(this)),
    mbUpdate(true),
    mbModelValid(true),
    maLineColorWrapper(mxModel, getColorToolBoxControl(*mxColorDispatch), "LineColor"),
    maLineStyleWrapper(mxModel, getLineStyleToolBoxControl(*mxLineStyleDispatch))
{
    disableArrowHead();
    std::vector<ObjectType> aAcceptedTypes { OBJECTTYPE_PAGE, OBJECTTYPE_DIAGRAM,
        OBJECTTYPE_DATA_SERIES, OBJECTTYPE_DATA_POINT,
        OBJECTTYPE_TITLE, OBJECTTYPE_LEGEND, OBJECTTYPE_DATA_CURVE,
        OBJECTTYPE_DATA_AVERAGE_LINE, OBJECTTYPE_AXIS};
    mxSelectionListener->setAcceptedTypes(std::move(aAcceptedTypes));
    Initialize();
}

ChartLinePanel::~ChartLinePanel()
{
    doUpdateModel(nullptr);
}

void ChartLinePanel::Initialize()
{
    mxModel->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    SvxColorToolBoxControl* pToolBoxColor = getColorToolBoxControl(*mxColorDispatch);
    pToolBoxColor->setColorSelectFunction(maLineColorWrapper);

    SvxLineStyleToolBoxControl* pToolBoxLineStyle = getLineStyleToolBoxControl(*mxLineStyleDispatch);
    pToolBoxLineStyle->setLineStyleSelectFunction(maLineStyleWrapper);

    setMapUnit(MapUnit::Map100thMM);
    updateData();
}

void ChartLinePanel::updateData()
{
    if (!mbUpdate || !mbModelValid)
        return;

    SolarMutexGuard aGuard;
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    sal_uInt16 nLineTransparence = 0;
    xPropSet->getPropertyValue("LineTransparence") >>= nLineTransparence;
    XLineTransparenceItem aLineTransparenceItem(nLineTransparence);
    updateLineTransparence(false, true, &aLineTransparenceItem);

    sal_uInt32 nWidth = 0;
    xPropSet->getPropertyValue("LineWidth") >>= nWidth;
    XLineWidthItem aWidthItem(nWidth);
    updateLineWidth(false, true, &aWidthItem);

    maLineStyleWrapper.updateData();
    maLineColorWrapper.updateData();
}

void ChartLinePanel::modelInvalid()
{
    mbModelValid = false;
}

void ChartLinePanel::selectionChanged(bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

void ChartLinePanel::doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel)
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

    maLineStyleWrapper.updateModel(mxModel);
    maLineColorWrapper.updateModel(mxModel);

    mxModel->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartLinePanel::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    ::chart::ChartModel* pModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}

void ChartLinePanel::setLineTransparency(const XLineTransparenceItem& rItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getPropSet(mxModel);

    if (!xPropSet.is())
        return;

    PreventUpdate aPreventUpdate(mbUpdate);
    xPropSet->setPropertyValue("LineTransparence", css::uno::Any(rItem.GetValue()));
}

void ChartLinePanel::setLineWidth(const XLineWidthItem& rItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getPropSet(mxModel);

    if (!xPropSet.is())
        return;

    PreventUpdate aPreventUpdate(mbUpdate);
    xPropSet->setPropertyValue("LineWidth", css::uno::Any(rItem.GetValue()));
}

void ChartLinePanel::updateLineWidth(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem)
{
    LinePropertyPanelBase::updateLineWidth(bDisabled, bSetOrDefault, pItem);

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (comphelper::LibreOfficeKit::isActive() && pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                        ".uno:LineWidth=" + OString::number(mnWidthCoreValue));
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
