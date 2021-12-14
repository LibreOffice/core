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

#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>

namespace drawinglayer::primitive2d
{
/** PolygonWavePrimitive2D class

    This primitive defines a waveline based on a PolygonStrokePrimitive2D
    where the wave is defined by wave width and wave length.
 */
class PolygonWavePrimitive2D final : public PolygonStrokePrimitive2D
{
private:
    /// wave definition
    double mfWaveWidth;
    double mfWaveHeight;

    /// local decomposition.
    virtual void
    create2DDecomposition(Primitive2DContainer& rContainer,
                          const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructor
    PolygonWavePrimitive2D(const basegfx::B2DPolygon& rPolygon,
                           const attribute::LineAttribute& rLineAttribute,
                           const attribute::StrokeAttribute& rStrokeAttribute, double fWaveWidth,
                           double fWaveHeight);

    /// constructor without stroking
    PolygonWavePrimitive2D(const basegfx::B2DPolygon& rPolygon,
                           const attribute::LineAttribute& rLineAttribute, double fWaveWidth,
                           double fWaveHeight);

    /// data read access
    double getWaveWidth() const { return mfWaveWidth; }
    double getWaveHeight() const { return mfWaveHeight; }

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
