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

#include <editeng/colritem.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/svxids.hrc>

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
        css::uno::Reference<css::frame::XModel> xModel,
        SvxColorToolBoxControl* pControl,
        const OUString& rName):
    mxModel(xModel),
    mpControl(pControl),
    maPropertyName(rName)
{
}

void ChartColorWrapper::operator()(const OUString& , const Color& rColor)
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet = getPropSet(mxModel);
    assert(xPropSet.is());

    xPropSet->setPropertyValue(maPropertyName, css::uno::makeAny(rColor.GetColor()));
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

    css::uno::Any aAny = xPropSet->getPropertyValue(maPropertyName);
    sal_uInt32 nColor = 0;
    aAny >>= nColor;
    Color aColor(nColor);

    SvxColorItem aItem(aColor, SID_ATTR_FILL_COLOR);
    mpControl->StateChanged(SID_ATTR_FILL_COLOR, SfxItemState::SET, &aItem);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
