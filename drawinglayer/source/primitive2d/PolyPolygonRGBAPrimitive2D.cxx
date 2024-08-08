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

#include <drawinglayer/primitive2d/PolyPolygonRGBAPrimitive2D.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
Primitive2DReference PolyPolygonRGBAPrimitive2D::create2DDecomposition(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (basegfx::fTools::equal(getTransparency(), 1.0))
    {
        // completely transparent, done
        return nullptr;
    }

    if (0 == getB2DPolyPolygon().count())
    {
        // no geometry, done
        return nullptr;
    }

    if (basegfx::fTools::equalZero(getTransparency()))
    {
        // no transparency, use simple PolyPolygonColorPrimitive2D
        return Primitive2DReference{ new PolyPolygonColorPrimitive2D(getB2DPolyPolygon(),
                                                                     getBColor()) };
    }

    // default: embed to UnifiedTransparencePrimitive2D
    Primitive2DContainer aContent{ new PolyPolygonColorPrimitive2D(getB2DPolyPolygon(),
                                                                   getBColor()) };
    return Primitive2DReference{ new UnifiedTransparencePrimitive2D(std::move(aContent),
                                                                    getTransparency()) };
}

PolyPolygonRGBAPrimitive2D::PolyPolygonRGBAPrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                                       const basegfx::BColor& rBColor,
                                                       double fTransparency)
    : maPolyPolygon(rPolyPolygon)
    , maBColor(rBColor)
    , mfTransparency(std::max(0.0, std::min(1.0, fTransparency)))
{
}

bool PolyPolygonRGBAPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PolyPolygonRGBAPrimitive2D& rCompare
            = static_cast<const PolyPolygonRGBAPrimitive2D&>(rPrimitive);

        return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                && getBColor() == rCompare.getBColor()
                && basegfx::fTools::equal(getTransparency(), rCompare.getTransparency()));
    }

    return false;
}

basegfx::B2DRange PolyPolygonRGBAPrimitive2D::getB2DRange(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // return range - without decompose
    return basegfx::utils::getRange(getB2DPolyPolygon());
}

// provide unique ID
sal_uInt32 PolyPolygonRGBAPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYPOLYGONRGBAPRIMITIVE2D;
}
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
