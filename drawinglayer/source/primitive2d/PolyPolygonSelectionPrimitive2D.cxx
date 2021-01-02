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

#include <drawinglayer/primitive2d/PolyPolygonSelectionPrimitive2D.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
void PolyPolygonSelectionPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, VisitingParameters const& /*rParameters*/) const
{
    if (getTransparence() >= 1.0 || !getB2DPolyPolygon().count())
        return;

    Primitive2DContainer aRetval;

    if (getFill() && getB2DPolyPolygon().isClosed())
    {
        // create fill primitive
        const Primitive2DReference aFill(
            new PolyPolygonColorPrimitive2D(getB2DPolyPolygon(), getColor()));

        aRetval = Primitive2DContainer{ aFill };
    }

    if (getDiscreteGrow() > 0.0)
    {
        const attribute::LineAttribute aLineAttribute(getColor(),
                                                      getDiscreteGrow() * getDiscreteUnit() * 2.0);
        const Primitive2DReference aFatLine(
            new PolyPolygonStrokePrimitive2D(getB2DPolyPolygon(), aLineAttribute));

        aRetval.push_back(aFatLine);
    }

    // embed filled to transparency (if used)
    if (!aRetval.empty() && getTransparence() > 0.0)
    {
        const Primitive2DReference aTrans(
            new UnifiedTransparencePrimitive2D(aRetval, getTransparence()));

        aRetval = Primitive2DContainer{ aTrans };
    }

    rContainer.insert(rContainer.end(), aRetval.begin(), aRetval.end());
}

PolyPolygonSelectionPrimitive2D::PolyPolygonSelectionPrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::BColor& rColor,
    double fTransparence, double fDiscreteGrow, bool bFill)
    : DiscreteMetricDependentPrimitive2D()
    , maPolyPolygon(rPolyPolygon)
    , maColor(rColor)
    , mfTransparence(fTransparence)
    , mfDiscreteGrow(fabs(fDiscreteGrow))
    , mbFill(bFill)
{
}

bool PolyPolygonSelectionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
    {
        const PolyPolygonSelectionPrimitive2D& rCompare
            = static_cast<const PolyPolygonSelectionPrimitive2D&>(rPrimitive);

        return (
            getB2DPolyPolygon() == rCompare.getB2DPolyPolygon() && getColor() == rCompare.getColor()
            && getTransparence() == rCompare.getTransparence()
            && getDiscreteGrow() == rCompare.getDiscreteGrow() && getFill() == rCompare.getFill());
    }

    return false;
}

basegfx::B2DRange
PolyPolygonSelectionPrimitive2D::getB2DRange(VisitingParameters const& rParameters) const
{
    basegfx::B2DRange aRetval(basegfx::utils::getRange(getB2DPolyPolygon()));

    if (getDiscreteGrow() > 0.0)
    {
        // get the current DiscreteUnit (not sure if getDiscreteUnit() is updated here, better go safe way)
        const double fDiscreteUnit(
            (rParameters.getViewInformation().getInverseObjectToViewTransformation()
             * basegfx::B2DVector(1.0, 0.0))
                .getLength());

        aRetval.grow(fDiscreteUnit * getDiscreteGrow());
    }

    return aRetval;
}

// provide unique ID
sal_uInt32 PolyPolygonSelectionPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYPOLYGONSELECTIONPRIMITIVE2D;
}

} // end drawinglayer::primitive2d namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
