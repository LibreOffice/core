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

#include "ChartColorWrapper.hxx"

#include <ObjectIdentifier.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <svx/linectrl.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/xlndsit.hxx>
#include <svx/unomid.hxx>

namespace chart::sidebar {

namespace {

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

}

ChartColorWrapper::ChartColorWrapper(
        css::uno::Reference<css::frame::XModel> const & xModel,
        SvxColorToolBoxControl* pControl,
        const OUString& rName):
    mxModel(xModel),
    mpControl(pControl),
    maPropertyName(rName)
{
}

void ChartColorWrapper::operator()(const OUString& , const NamedColor& rColor)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);

    if (!xPropSet.is())
    {
        SAL_WARN("chart2", "Invalid reference to xPropSet");
        return;
    }

    xPropSet->setPropertyValue(maPropertyName, css::uno::makeAny(rColor.first));
}

void ChartColorWrapper::updateModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    mxModel = xModel;
}

void ChartColorWrapper::updateData()
{
    static const OUStringLiteral aLineColor = u"LineColor";
    static const std::u16string_view aCommands[2] = {u".uno:XLineColor", u".uno:FillColor"};

    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    css::util::URL aUrl;
    aUrl.Complete = (maPropertyName == aLineColor) ? aCommands[0] : aCommands[1];

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL = aUrl;
    aEvent.IsEnabled = true;
    aEvent.State = xPropSet->getPropertyValue(maPropertyName);
    mpControl->statusChanged(aEvent);
}

ChartLineStyleWrapper::ChartLineStyleWrapper(
        css::uno::Reference<css::frame::XModel> const & xModel,
        SvxLineStyleToolBoxControl* pControl)
    : mxModel(xModel)
    , mpControl(pControl)
{
}

void ChartLineStyleWrapper::updateModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    mxModel = xModel;
}

namespace
{
    css::uno::Any getLineDash(
            const css::uno::Reference<css::frame::XModel>& xModel, const OUString& rDashName)
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
}

void ChartLineStyleWrapper::updateData()
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    css::util::URL aUrl;
    aUrl.Complete = ".uno:XLineStyle";

    css::frame::FeatureStateEvent aEvent;
    aEvent.IsEnabled = true;

    aEvent.FeatureURL = aUrl;
    aEvent.State = xPropSet->getPropertyValue("LineStyle");
    mpControl->statusChanged(aEvent);

    aUrl.Complete = ".uno:LineDash";

    auto aLineDashName = xPropSet->getPropertyValue("LineDashName");
    OUString aDashName;
    aLineDashName >>= aDashName;
    css::uno::Any aLineDash = getLineDash(mxModel, aDashName);
    XLineDashItem aDashItem;
    aDashItem.PutValue(aLineDash, MID_LINEDASH);

    aEvent.FeatureURL = aUrl;
    aDashItem.QueryValue(aEvent.State);
    mpControl->statusChanged(aEvent);
}

bool ChartLineStyleWrapper::operator()(const OUString& rCommand, const css::uno::Any& rValue)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);

    if (!xPropSet.is())
    {
        SAL_WARN("chart2", "Invalid reference to xPropSet");
        return false;
    }

    if (rCommand == ".uno:XLineStyle")
    {
        xPropSet->setPropertyValue("LineStyle", rValue);
        return true;
    }
    else if (rCommand == ".uno:LineDash")
    {
        XLineDashItem aDashItem;
        aDashItem.PutValue(rValue, 0);
        css::uno::Any aAny;
        aDashItem.QueryValue(aAny, MID_LINEDASH);
        OUString aDashName = PropertyHelper::addLineDashUniqueNameToTable(aAny,
                css::uno::Reference<css::lang::XMultiServiceFactory>(mxModel, css::uno::UNO_QUERY),
                "");
        xPropSet->setPropertyValue("LineDash", aAny);
        xPropSet->setPropertyValue("LineDashName", css::uno::Any(aDashName));
        return true;
    }
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
