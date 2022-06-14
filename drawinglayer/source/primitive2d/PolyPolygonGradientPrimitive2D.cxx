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

#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <rtl/ref.hxx>
#include <utility>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
void PolyPolygonGradientPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (!getFillGradient().isDefault())
    {
        // create SubSequence with FillGradientPrimitive2D
        const basegfx::B2DRange aPolyPolygonRange(getB2DPolyPolygon().getB2DRange());
        rtl::Reference<FillGradientPrimitive2D> pNewGradient = new FillGradientPrimitive2D(
            aPolyPolygonRange, getDefinitionRange(), getFillGradient());
        Primitive2DContainer aSubSequence{ pNewGradient };

        // create mask primitive
        rContainer.push_back(new MaskPrimitive2D(getB2DPolyPolygon(), std::move(aSubSequence)));
    }
}

PolyPolygonGradientPrimitive2D::PolyPolygonGradientPrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon, attribute::FillGradientAttribute aFillGradient)
    : maPolyPolygon(rPolyPolygon)
    , maDefinitionRange(rPolyPolygon.getB2DRange())
    , maFillGradient(std::move(aFillGradient))
{
}

PolyPolygonGradientPrimitive2D::PolyPolygonGradientPrimitive2D(
    basegfx::B2DPolyPolygon aPolyPolygon, const basegfx::B2DRange& rDefinitionRange,
    attribute::FillGradientAttribute aFillGradient)
    : maPolyPolygon(std::move(aPolyPolygon))
    , maDefinitionRange(rDefinitionRange)
    , maFillGradient(std::move(aFillGradient))
{
}

bool PolyPolygonGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PolyPolygonGradientPrimitive2D& rCompare
            = static_cast<const PolyPolygonGradientPrimitive2D&>(rPrimitive);

        return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                && getDefinitionRange() == rCompare.getDefinitionRange()
                && getFillGradient() == rCompare.getFillGradient());
    }

    return false;
}

// provide unique ID
sal_uInt32 PolyPolygonGradientPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D;
}

} // end drawinglayer::primitive2d namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
