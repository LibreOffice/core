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
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>

namespace drawinglayer::primitive2d
{
/** PolyPolygonHatchPrimitive2D class

    This primitive defines a tools::PolyPolygon filled with a hatch. The
    decomosition will create a MaskPrimitive2D containing a
    FillHatchPrimitive2D.
 */
class DRAWINGLAYER_DLLPUBLIC PolyPolygonHatchPrimitive2D final
    : public BufferedDecompositionPrimitive2D
{
private:
    /// the tools::PolyPolygon geometry
    basegfx::B2DPolyPolygon maPolyPolygon;

    /// the definition range
    basegfx::B2DRange maDefinitionRange;

    /// the hatch background color (if used)
    basegfx::BColor maBackgroundColor;

    /// the hatch definition
    attribute::FillHatchAttribute maFillHatch;

    /// local decomposition.
    virtual void
    create2DDecomposition(Primitive2DContainer& rContainer,
                          const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructors. The one without definition range will use output range as definition range
    PolyPolygonHatchPrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                const basegfx::BColor& rBackgroundColor,
                                const attribute::FillHatchAttribute& rFillHatch);
    PolyPolygonHatchPrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                const basegfx::B2DRange& rDefinitionRange,
                                const basegfx::BColor& rBackgroundColor,
                                const attribute::FillHatchAttribute& rFillHatch);

    /// data read access
    const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
    const basegfx::B2DRange& getDefinitionRange() const { return maDefinitionRange; }
    const basegfx::BColor& getBackgroundColor() const { return maBackgroundColor; }
    const attribute::FillHatchAttribute& getFillHatch() const { return maFillHatch; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

} // end of namespace primitive2d::drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
