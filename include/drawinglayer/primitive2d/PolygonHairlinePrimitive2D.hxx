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
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/color/bcolor.hxx>

namespace drawinglayer::primitive2d
{
/** PolygonHairlinePrimitive2D class

    This primitive defines a Hairline. Since hairlines are view-dependent,
    this primitive is view-dependent, too.

    This is one of the non-decomposable primitives, so a renderer
    should process it.
 */
class DRAWINGLAYER_DLLPUBLIC PolygonHairlinePrimitive2D final : public BasePrimitive2D
{
private:
    /// the hairline geometry
    basegfx::B2DPolygon maPolygon;

    /// the hairline color
    basegfx::BColor maBColor;

public:
    /// constructor
    PolygonHairlinePrimitive2D(const basegfx::B2DPolygon& rPolygon, const basegfx::BColor& rBColor);

    /// data read access
    const basegfx::B2DPolygon& getB2DPolygon() const { return maPolygon; }
    const basegfx::BColor& getBColor() const { return maBColor; }

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
