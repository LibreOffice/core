/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartColorWrapper.hxx"

#include <ObjectIdentifier.hxx>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <svx/tbcontrl.hxx>

namespace chart { namespace sidebar {

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
    if (!mxModel.is())
    {
        SAL_WARN("chart2", "Invalid reference to mxModel");
        return;
    }

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
    static const OUStringLiteral aLineColor = "LineColor";
    static const OUStringLiteral aCommands[2] = {".uno:XLineColor", ".uno:FillColor"};

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

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
