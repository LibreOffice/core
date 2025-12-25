/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/gen.hxx>
#include <drawingml/chart/stylemodel.hxx>
#include <docmodel/text/TextBodyProps.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;

namespace oox::drawingml::chart
{
model::StyleEntry StyleEntryModel::toStyleEntry([[maybe_unused]] oox::core::XmlFilterBase& rFilter)
{
    std::shared_ptr<model::ShapeBase> pShapePr;
    // Down-cast to ShapeBase* and deep copy so the shared_ptr can have ownership
    if (mxShapeProp)
    {
        pShapePr = std::make_shared<model::ShapeBase>(
            *static_cast<model::ShapeBase*>(mxShapeProp.get()));
    }

    std::shared_ptr<model::TextCharPropsBase> pCharPr;
    // Down-cast to TextCharPropsBase* and deep copy so the shared_ptr can have ownership
    if (mrTextCharacterProperties)
    {
        pCharPr = std::make_shared<model::TextCharPropsBase>(
            *static_cast<model::TextCharPropsBase*>(mrTextCharacterProperties.get()));
    }

    std::shared_ptr<model::TextBodyProps> pBodyPr;
    if (mxBodyPr)
    {
        // Down-cast to TextBodyProps* and deep copy so the shared_ptr can have ownership
        pBodyPr = std::make_shared<model::TextBodyProps>(
            *static_cast<model::TextBodyProps*>(mxBodyPr.get()));
    }

    return model::StyleEntry(mxLnRef, mfLineWidthScale, mxFillRef, mxEffectRef, mxFontRef,
                             std::move(pShapePr), std::move(pCharPr), std::move(pBodyPr));
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
