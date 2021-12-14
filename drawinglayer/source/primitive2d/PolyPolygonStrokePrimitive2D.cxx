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

#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
void PolyPolygonStrokePrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    const basegfx::B2DPolyPolygon aPolyPolygon(getB2DPolyPolygon());
    const sal_uInt32 nCount(aPolyPolygon.count());

    if (nCount)
    {
        for (sal_uInt32 a(0); a < nCount; a++)
        {
            rContainer.push_back(new PolygonStrokePrimitive2D(
                aPolyPolygon.getB2DPolygon(a), getLineAttribute(), getStrokeAttribute()));
        }
    }
}

PolyPolygonStrokePrimitive2D::PolyPolygonStrokePrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon, const attribute::LineAttribute& rLineAttribute,
    const attribute::StrokeAttribute& rStrokeAttribute)
    : maPolyPolygon(rPolyPolygon)
    , maLineAttribute(rLineAttribute)
    , maStrokeAttribute(rStrokeAttribute)
{
}

PolyPolygonStrokePrimitive2D::PolyPolygonStrokePrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon, const attribute::LineAttribute& rLineAttribute)
    : maPolyPolygon(rPolyPolygon)
    , maLineAttribute(rLineAttribute)
{
}

bool PolyPolygonStrokePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PolyPolygonStrokePrimitive2D& rCompare
            = static_cast<const PolyPolygonStrokePrimitive2D&>(rPrimitive);

        return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                && getLineAttribute() == rCompare.getLineAttribute()
                && getStrokeAttribute() == rCompare.getStrokeAttribute());
    }

    return false;
}

basegfx::B2DRange PolyPolygonStrokePrimitive2D::getB2DRange(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // get range of it (subdivided)
    basegfx::B2DRange aRetval(basegfx::utils::getRange(getB2DPolyPolygon()));

    // if width, grow by line width
    if (getLineAttribute().getWidth())
    {
        aRetval.grow(getLineAttribute().getWidth() / 2.0);
    }

    return aRetval;
}

// provide unique ID
sal_uInt32 PolyPolygonStrokePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D;
}

} // end drawinglayer::primitive2d namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
