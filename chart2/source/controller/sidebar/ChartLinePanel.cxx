/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartLinePanel.hxx"

#include "PropertyHelper.hxx"
#include "ChartController.hxx"

#include <svx/xlnwtit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/unomid.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>

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

css::uno::Any getLineDash(
        css::uno::Reference<css::frame::XModel> xModel, const OUString& rDashName)
{
    css::uno::Reference<css::lang::XMultiServiceFactory> xFact(xModel, css::uno::UNO_QUERY);
    css::uno::Reference<css::container::XNameAccess> xNameAccess(
            xFact->createInstance("com.sun.star.drawing.DashTable"),
            css::uno::UNO_QUERY );
    if(xNameAccess.is())
    {
        if (!xNameAccess->hasByName(rDashName))
            return css::uno::Any();

        return xNameAccess->getByName(rDashName);
    }

    return css::uno::Any();
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

VclPtr<vcl::Window> ChartLinePanel::Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController)
{
    if (pParent == NULL)
        throw css::lang::IllegalArgumentException("no parent Window given to ChartAxisPanel::Create", NULL, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to ChartAxisPanel::Create", NULL, 1);

    return VclPtr<ChartLinePanel>::Create(
                        pParent, rxFrame, pController);
}

ChartLinePanel::ChartLinePanel(vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController):
    svx::sidebar::LinePropertyPanelBase(pParent, rxFrame),
    mxModel(pController->getModel()),
    mxListener(new ChartSidebarModifyListener(this)),
    mxSelectionListener(new ChartSidebarSelectionListener(this)),
    mbUpdate(true)
{
    std::vector<ObjectType> aAcceptedTypes { OBJECTTYPE_PAGE, OBJECTTYPE_DIAGRAM, OBJECTTYPE_DATA_SERIES, OBJECTTYPE_TITLE, OBJECTTYPE_LEGEND};
    mxSelectionListener->setAcceptedTypes(aAcceptedTypes);
    Initialize();
}

ChartLinePanel::~ChartLinePanel()
{
    disposeOnce();
}

void ChartLinePanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->removeSelectionChangeListener(mxSelectionListener.get());

    LinePropertyPanelBase::dispose();
}

void ChartLinePanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener.get());

    setMapUnit(SFX_MAPUNIT_100TH_MM);
    updateData();
}

void ChartLinePanel::updateData()
{
    if (!mbUpdate)
        return;

    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    sal_uInt16 nLineTransparence = 0;
    xPropSet->getPropertyValue("LineTransparence") >>= nLineTransparence;
    XLineTransparenceItem aLineTransparenceItem(nLineTransparence);
    updateLineTransparence(false, true, &aLineTransparenceItem);

    css::drawing::LineStyle eStyle = css::drawing::LineStyle_SOLID;
    xPropSet->getPropertyValue("LineStyle") >>= eStyle;
    XLineStyleItem aStyleItem(eStyle);
    updateLineStyle(false, true, &aStyleItem);

    css::uno::Any aLineDashName = xPropSet->getPropertyValue("LineDashName");
    OUString aDashName;
    aLineDashName >>= aDashName;
    css::uno::Any aLineDash = getLineDash(mxModel, aDashName);
    XLineDashItem aDashItem;
    aDashItem.PutValue(aLineDash, MID_LINEDASH);
    updateLineDash(false, true, &aDashItem);
}

void ChartLinePanel::modelInvalid()
{
}

void ChartLinePanel::selectionChanged(bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

void ChartLinePanel::SelectionInvalid()
{
}

void ChartLinePanel::updateModel(
        css::uno::Reference<css::frame::XModel> xModel)
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    mxModel = xModel;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener.get());
}

void ChartLinePanel::setLineStyle(const XLineStyleItem& rItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getPropSet(mxModel);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("LineStyle", css::uno::makeAny(rItem.GetValue()));
}

void ChartLinePanel::setLineDash(const XLineDashItem& rItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getPropSet(mxModel);

    if (!xPropSet.is())
        return;

    css::uno::Any aAny;
    rItem.QueryValue(aAny, MID_LINEDASH);
    OUString aDashName = PropertyHelper::addLineDashUniqueNameToTable(aAny,
            css::uno::Reference<css::lang::XMultiServiceFactory>(mxModel, css::uno::UNO_QUERY),
            "");
    xPropSet->setPropertyValue("LineDash", aAny);
    xPropSet->setPropertyValue("LineDashName", css::uno::makeAny(aDashName));
}

void ChartLinePanel::setLineEndStyle(const XLineEndItem* /*pItem*/)
{
}

void ChartLinePanel::setLineStartStyle(const XLineStartItem* /*pItem*/)
{
}

void ChartLinePanel::setLineJoint(const XLineJointItem* pItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getPropSet(mxModel);

    if (!xPropSet.is())
        return;

    if (pItem)
        xPropSet->setPropertyValue("LineJoint", css::uno::makeAny(pItem->GetValue()));
}

void ChartLinePanel::setLineCap(const XLineCapItem* /*pItem*/)
{
}

void ChartLinePanel::setLineTransparency(const XLineTransparenceItem& rItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getPropSet(mxModel);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("LineTransparence", css::uno::makeAny(rItem.GetValue()));
}

void ChartLinePanel::setLineWidth(const XLineWidthItem& rItem)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet =
        getPropSet(mxModel);

    if (!xPropSet.is())
        return;

    xPropSet->setPropertyValue("LineWidth", css::uno::makeAny(rItem.GetValue()));
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
