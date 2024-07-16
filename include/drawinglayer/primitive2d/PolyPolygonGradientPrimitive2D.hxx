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
#include <drawinglayer/attribute/fillgradientattribute.hxx>

namespace drawinglayer::primitive2d
{
/** PolyPolygonColorPrimitive2D class

    This primitive defines a tools::PolyPolygon filled with a gradient. The
    decomosition will create a MaskPrimitive2D containing a
    FillGradientPrimitive2D.
 */
class DRAWINGLAYER_DLLPUBLIC PolyPolygonGradientPrimitive2D
    : public BufferedDecompositionPrimitive2D
{
private:
    /// the tools::PolyPolygon geometry
    basegfx::B2DPolyPolygon maPolyPolygon;

    /// the definition range
    basegfx::B2DRange maDefinitionRange;

    /// the gradient definition
    attribute::FillGradientAttribute maFillGradient;

    /// local decomposition.
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructors. The one without definition range will use output range as definition range
    PolyPolygonGradientPrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                   attribute::FillGradientAttribute rFillGradient);
    PolyPolygonGradientPrimitive2D(basegfx::B2DPolyPolygon aPolyPolygon,
                                   const basegfx::B2DRange& rDefinitionRange,
                                   attribute::FillGradientAttribute aFillGradient);

    /// data read access
    const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
    const basegfx::B2DRange& getDefinitionRange() const { return maDefinitionRange; }
    const attribute::FillGradientAttribute& getFillGradient() const { return maFillGradient; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

// helper primitive that can be used to directly express RGBA
// gradient definitions. It will be decomposed to a combined
// TransparencePrimitive2D if not handled directly. Use the
// already existing PolyPolygonGradientPrimitive2D as base class,
// only the additional FillGradientAlpha needs to be added.
// NOTE: FillGradientAlpha *has* to fulfil the
// 'sameDefinitionThanAlpha' condition defined by the check
// method with the same name
class DRAWINGLAYER_DLLPUBLIC PolyPolygonRGBAGradientPrimitive2D final
    : public PolyPolygonGradientPrimitive2D
{
private:
    /// the gradient alpha definition
    attribute::FillGradientAttribute maFillGradientAlpha;

    /// local decomposition.
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructors. The one without definition range will use output range as definition range
    PolyPolygonRGBAGradientPrimitive2D(basegfx::B2DPolyPolygon aPolyPolygon,
                                       const basegfx::B2DRange& rDefinitionRange,
                                       attribute::FillGradientAttribute aFillGradient,
                                       attribute::FillGradientAttribute aFillGradientAlpha);

    /// data read access
    const attribute::FillGradientAttribute& getFillGradientAlpha() const
    {
        return maFillGradientAlpha;
    }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

} // end of namespace primitive2d::drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
