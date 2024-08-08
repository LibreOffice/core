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

#include <drawinglayer/primitive2d/PolyPolygonAlphaGradientPrimitive2D.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonRGBAPrimitive2D.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/utils/bgradient.hxx>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
Primitive2DReference PolyPolygonAlphaGradientPrimitive2D::create2DDecomposition(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (0 == getB2DPolyPolygon().count())
    {
        // no geometry, done
        return nullptr;
    }

    if (getAlphaGradient().isDefault())
    {
        // default is a single ColorStop at 0.0 with black (0, 0, 0). The
        // luminance is then 0.0, too -> not transparent at all
        return new PolyPolygonColorPrimitive2D(getB2DPolyPolygon(), getBColor());
    }

    basegfx::BColor aSingleColor;
    if (getAlphaGradient().getColorStops().isSingleColor(aSingleColor))
    {
        // no real transparence gradient, only unified alpha,
        // we can use PolyPolygonRGBAPrimitive2D
        return new PolyPolygonRGBAPrimitive2D(getB2DPolyPolygon(), getBColor(),
                                              aSingleColor.luminance());
    }

    // transparency gradient is a real gradient, create TransparencePrimitive2D
    Primitive2DContainer aContent{ new PolyPolygonColorPrimitive2D(getB2DPolyPolygon(),
                                                                   getBColor()) };

    Primitive2DContainer aAlpha{ new FillGradientPrimitive2D(
        basegfx::utils::getRange(getB2DPolyPolygon()), getAlphaGradient()) };

    return new TransparencePrimitive2D(std::move(aContent), std::move(aAlpha));
}

PolyPolygonAlphaGradientPrimitive2D::PolyPolygonAlphaGradientPrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::BColor& rBColor,
    const attribute::FillGradientAttribute& rAlphaGradient)
    : maPolyPolygon(rPolyPolygon)
    , maBColor(rBColor)
    , maAlphaGradient(rAlphaGradient)
{
}

bool PolyPolygonAlphaGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PolyPolygonAlphaGradientPrimitive2D& rCompare
            = static_cast<const PolyPolygonAlphaGradientPrimitive2D&>(rPrimitive);

        return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                && getBColor() == rCompare.getBColor()
                && getAlphaGradient() == rCompare.getAlphaGradient());
    }

    return false;
}

basegfx::B2DRange PolyPolygonAlphaGradientPrimitive2D::getB2DRange(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // return range - without decompose
    return basegfx::utils::getRange(getB2DPolyPolygon());
}

// provide unique ID
sal_uInt32 PolyPolygonAlphaGradientPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYPOLYGONALPHAGRADIENTPRIMITIVE2D;
}
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
