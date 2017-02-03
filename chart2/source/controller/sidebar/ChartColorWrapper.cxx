/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartColorWrapper.hxx"

#include "ChartController.hxx"

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
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    assert(xPropSet.is());

    xPropSet->setPropertyValue(maPropertyName, css::uno::Any(rColor.first.GetColor()));
}

void ChartColorWrapper::updateModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    mxModel = xModel;
}

void ChartColorWrapper::updateData()
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    if (!xPropSet.is())
        return;

    css::util::URL aUrl;
    aUrl.Complete = ".uno:FillColor";

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL = aUrl;
    aEvent.IsEnabled = true;
    aEvent.State = xPropSet->getPropertyValue(maPropertyName);
    mpControl->statusChanged(aEvent);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
