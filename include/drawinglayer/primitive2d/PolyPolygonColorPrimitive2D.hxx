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

namespace drawinglayer::primitive2d
{
/** PolyPolygonColorPrimitive2D class

    This primitive defines a tools::PolyPolygon filled with a single color.
    This is one of the non-decomposable primitives, so a renderer
    should process it.
 */
class DRAWINGLAYER_DLLPUBLIC PolyPolygonColorPrimitive2D final : public BasePrimitive2D
{
private:
    /// the tools::PolyPolygon geometry
    basegfx::B2DPolyPolygon maPolyPolygon;

    /// the polygon fill color
    basegfx::BColor maBColor;

public:
    /// constructor
    PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon aPolyPolygon,
                                const basegfx::BColor& rBColor);

    /// data read access
    const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
    const basegfx::BColor& getBColor() const { return maBColor; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

/** FilledRectanglePrimitive2D class

    Tooling: This primitive defines a simple rectangle. It is
    sometimes useful for simpler tasks and decomposes to a
    more generalized PolyPolygonColorPrimitive2D (see above)
*/
class UNLESS_MERGELIBS(DRAWINGLAYER_DLLPUBLIC) FilledRectanglePrimitive2D final
    : public BasePrimitive2D
{
private:
    /// the  geometry
    basegfx::B2DRange maB2DRange;

    /// the fill color
    basegfx::BColor maBColor;

public:
    /// constructor
    FilledRectanglePrimitive2D(const basegfx::B2DRange& rB2DRange, const basegfx::BColor& rBColor);

    /// data read access
    const basegfx::B2DRange& getB2DRange() const { return maB2DRange; }
    const basegfx::BColor& getBColor() const { return maBColor; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;

    /// return as PolyPolygonColorPrimitive2D
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;
};

} // end of namespace primitive2d::drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
