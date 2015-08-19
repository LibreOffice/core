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

}

ChartColorWrapper::ChartColorWrapper(
        css::uno::Reference<css::frame::XModel> xModel):
    mxModel(xModel),
    maPropertyName("FillColor")
{
}

void ChartColorWrapper::operator()(const OUString& rCommand, const Color& rColor)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    assert(xPropSet.is());

    xPropSet->setPropertyValue(maPropertyName, css::uno::makeAny(rColor.GetColor()));
}

void ChartColorWrapper::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    mxModel = xModel;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
