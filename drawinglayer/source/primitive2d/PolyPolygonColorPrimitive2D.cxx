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

#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <utility>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
PolyPolygonColorPrimitive2D::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon aPolyPolygon,
                                                         const basegfx::BColor& rBColor)
    : maPolyPolygon(std::move(aPolyPolygon))
    , maBColor(rBColor)
{
}

bool PolyPolygonColorPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const PolyPolygonColorPrimitive2D& rCompare
            = static_cast<const PolyPolygonColorPrimitive2D&>(rPrimitive);

        return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                && getBColor() == rCompare.getBColor());
    }

    return false;
}

basegfx::B2DRange PolyPolygonColorPrimitive2D::getB2DRange(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // return range
    return basegfx::utils::getRange(getB2DPolyPolygon());
}

// provide unique ID
sal_uInt32 PolyPolygonColorPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D;
}

FilledRectanglePrimitive2D::FilledRectanglePrimitive2D(const basegfx::B2DRange& rB2DRange,
                                                       const basegfx::BColor& rBColor)
    : BasePrimitive2D()
    , maB2DRange(rB2DRange)
    , maBColor(rBColor)
{
}

bool FilledRectanglePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const FilledRectanglePrimitive2D& rCompare(
            static_cast<const FilledRectanglePrimitive2D&>(rPrimitive));

        return (getB2DRange() == rCompare.getB2DRange() && getBColor() == rCompare.getBColor());
    }

    return false;
}

basegfx::B2DRange FilledRectanglePrimitive2D::getB2DRange(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    return getB2DRange();
}

sal_uInt32 FilledRectanglePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_FILLEDRECTANGLEPRIMITIVE2D;
}

void FilledRectanglePrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (getB2DRange().isEmpty())
    {
        // no geometry, done
        return;
    }

    const basegfx::B2DPolygon aPolygon(basegfx::utils::createPolygonFromRect(getB2DRange()));
    Primitive2DContainer aSequence
        = { new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon), getBColor()) };
    rVisitor.visit(aSequence);
}

} // end drawinglayer::primitive2d namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
