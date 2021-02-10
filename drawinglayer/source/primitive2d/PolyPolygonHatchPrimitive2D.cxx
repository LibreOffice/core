/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawinglayer/primitive2d/PolyPolygonHatchPrimitive2D.hxx>

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <rtl/ref.hxx>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
void PolyPolygonHatchPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (!getFillHatch().isDefault())
    {
        // create SubSequence with FillHatchPrimitive2D
        const basegfx::B2DRange aPolyPolygonRange(getB2DPolyPolygon().getB2DRange());
        rtl::Reference<FillHatchPrimitive2D> pNewHatch = new FillHatchPrimitive2D(
            aPolyPolygonRange, getDefinitionRange(), getBackgroundColor(), getFillHatch());
        const Primitive2DContainer aSubSequence{ pNewHatch };

        // create mask primitive
        rContainer.push_back(new MaskPrimitive2D(getB2DPolyPolygon(), aSubSequence));
    }
}

PolyPolygonHatchPrimitive2D::PolyPolygonHatchPrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::BColor& rBackgroundColor,
    const attribute::FillHatchAttribute& rFillHatch)
    : BufferedDecompositionPrimitive2D()
    , maPolyPolygon(rPolyPolygon)
    , maDefinitionRange(rPolyPolygon.getB2DRange())
    , maBackgroundColor(rBackgroundColor)
    , maFillHatch(rFillHatch)
{
}

PolyPolygonHatchPrimitive2D::PolyPolygonHatchPrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DRange& rDefinitionRange,
    const basegfx::BColor& rBackgroundColor, const attribute::FillHatchAttribute& rFillHatch)
    : BufferedDecompositionPrimitive2D()
    , maPolyPolygon(rPolyPolygon)
    , maDefinitionRange(rDefinitionRange)
    , maBackgroundColor(rBackgroundColor)
    , maFillHatch(rFillHatch)
{
}

bool PolyPolygonHatchPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PolyPolygonHatchPrimitive2D& rCompare
            = static_cast<const PolyPolygonHatchPrimitive2D&>(rPrimitive);

        return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                && getDefinitionRange() == rCompare.getDefinitionRange()
                && getBackgroundColor() == rCompare.getBackgroundColor()
                && getFillHatch() == rCompare.getFillHatch());
    }

    return false;
}

// provide unique ID
sal_uInt32 PolyPolygonHatchPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D;
}

} // end drawinglayer::primitive2d namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
