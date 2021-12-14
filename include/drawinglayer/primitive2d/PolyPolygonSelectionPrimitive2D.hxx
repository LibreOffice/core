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
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/color/bcolor.hxx>

namespace drawinglayer::primitive2d
{
/** PolyPolygonSelectionPrimitive2D class

    This primitive defines a tools::PolyPolygon which gets filled with a defined color
    and a defined transparence, but also gets extended ('grown') by the given
    discrete size (thus being a view-dependent primitive)
 */
class DRAWINGLAYER_DLLPUBLIC PolyPolygonSelectionPrimitive2D final
    : public DiscreteMetricDependentPrimitive2D
{
private:
    /// the tools::PolyPolygon geometry
    basegfx::B2DPolyPolygon maPolyPolygon;

    /// the color
    basegfx::BColor maColor;

    /// the transparence [0.0 .. 1.0]
    double mfTransparence;

    /// the discrete grow size ('pixels'), only positive values allowed
    double mfDiscreteGrow;

    /// draw polygons filled when fill is set
    bool mbFill : 1;

    /// local decomposition.
    virtual void
    create2DDecomposition(Primitive2DContainer& rContainer,
                          const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructor
    PolyPolygonSelectionPrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                    const basegfx::BColor& rColor, double fTransparence,
                                    double fDiscreteGrow, bool bFill);

    /// data read access
    const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
    const basegfx::BColor& getColor() const { return maColor; }
    double getTransparence() const { return mfTransparence; }
    double getDiscreteGrow() const { return mfDiscreteGrow; }
    bool getFill() const { return mbFill; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};
} // end of namespace primitive2d::drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
