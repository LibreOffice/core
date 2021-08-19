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
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/color/bcolor.hxx>

namespace drawinglayer::primitive2d
{
/** PolygonMarkerPrimitive2D class

    This primitive defines a two-colored marker hairline which is
    dashed with the given dash length. Since hairlines are view-dependent,
    this primitive is view-dependent, too.

    It will be decomposed to the needed PolygonHairlinePrimitive2D if
    not handled directly by a renderer.
 */
class DRAWINGLAYER_DLLPUBLIC PolygonMarkerPrimitive2D final
    : public BufferedDecompositionPrimitive2D
{
private:
    /// the marker hairline geometry
    basegfx::B2DPolygon maPolygon;

    /// the two colors
    basegfx::BColor maRGBColorA;
    basegfx::BColor maRGBColorB;

    /// the dash distance in 'pixels'
    double mfDiscreteDashLength;

    /// decomposition is view-dependent, remember last InverseObjectToViewTransformation
    basegfx::B2DHomMatrix maLastInverseObjectToViewTransformation;

    /// local decomposition.
    virtual void
    create2DDecomposition(Primitive2DContainer& rContainer,
                          const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructor
    PolygonMarkerPrimitive2D(const basegfx::B2DPolygon& rPolygon, const basegfx::BColor& rRGBColorA,
                             const basegfx::BColor& rRGBColorB, double fDiscreteDashLength);

    /// data read access
    const basegfx::B2DPolygon& getB2DPolygon() const { return maPolygon; }
    const basegfx::BColor& getRGBColorA() const { return maRGBColorA; }
    const basegfx::BColor& getRGBColorB() const { return maRGBColorB; }
    double getDiscreteDashLength() const { return mfDiscreteDashLength; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// Override standard getDecomposition to be view-dependent here
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

} // end of namespace primitive2d::drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
