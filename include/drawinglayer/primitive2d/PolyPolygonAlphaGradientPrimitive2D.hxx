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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
// #include <basegfx/numeric/ftools.hxx>

namespace drawinglayer::primitive2d
{
/** PolyPolygonAlphaGradientPrimitive2D class

    This is a simple extension to PolyPolygonColorPrimitive2D
    that allows to directly define an alpha gradient for the PolyPolygon
    to be represented, additionally to the color.

    It will be decomposed simply to PolyPolygonColorPrimitive2D,
    maybe embedded to a FillGradientPrimitive2D if
    needed, so no changes have to be done to any primitive processor.

    OTOH e.g. SDPR implementations *may* use this directly if they
    are capable to draw a filled PolyPolygon with transparency gradient
    directly (e.g. CairoPixelProcessor2D)
 */
class DRAWINGLAYER_DLLPUBLIC PolyPolygonAlphaGradientPrimitive2D final
    : public BufferedDecompositionPrimitive2D
{
private:
    /// the tools::PolyPolygon geometry
    basegfx::B2DPolyPolygon maPolyPolygon;

    /// the polygon fill color
    basegfx::BColor maBColor;

    /// alphaGradient definition
    attribute::FillGradientAttribute maAlphaGradient;

    /// create local decomposition
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructor
    PolyPolygonAlphaGradientPrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                        const basegfx::BColor& rBColor,
                                        const attribute::FillGradientAttribute& rAlphaGradient);

    /// data read access
    const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
    const basegfx::BColor& getBColor() const { return maBColor; }
    const attribute::FillGradientAttribute& getAlphaGradient() const { return maAlphaGradient; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get B2Drange
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
