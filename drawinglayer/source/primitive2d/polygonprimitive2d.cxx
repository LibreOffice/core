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

#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <com/sun/star/drawing/LineCap.hpp>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
PolygonHairlinePrimitive2D::PolygonHairlinePrimitive2D(const basegfx::B2DPolygon& rPolygon,
                                                       const basegfx::BColor& rBColor)
    : maPolygon(rPolygon)
    , maBColor(rBColor)
{
}

bool PolygonHairlinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const PolygonHairlinePrimitive2D& rCompare
            = static_cast<const PolygonHairlinePrimitive2D&>(rPrimitive);

        return (getB2DPolygon() == rCompare.getB2DPolygon() && getBColor() == rCompare.getBColor());
    }

    return false;
}

basegfx::B2DRange
PolygonHairlinePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    // this is a hairline, thus the line width is view-dependent. Get range of polygon
    // as base size
    basegfx::B2DRange aRetval(getB2DPolygon().getB2DRange());

    if (!aRetval.isEmpty())
    {
        // Calculate view-dependent hairline width
        const basegfx::B2DVector aDiscreteSize(
            rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
        const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

        if (basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
        {
            aRetval.grow(fDiscreteHalfLineWidth);
        }
    }

    // return range
    return aRetval;
}

// provide unique ID
sal_uInt32 PolygonHairlinePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D;
}

void PolygonMarkerPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
{
    // calculate logic DashLength
    const basegfx::B2DVector aDashVector(rViewInformation.getInverseObjectToViewTransformation()
                                         * basegfx::B2DVector(getDiscreteDashLength(), 0.0));
    const double fLogicDashLength(aDashVector.getX());

    if (fLogicDashLength > 0.0 && !getRGBColorA().equal(getRGBColorB()))
    {
        // apply dashing; get line and gap snippets
        std::vector<double> aDash;
        basegfx::B2DPolyPolygon aDashedPolyPolyA;
        basegfx::B2DPolyPolygon aDashedPolyPolyB;

        aDash.push_back(fLogicDashLength);
        aDash.push_back(fLogicDashLength);
        basegfx::utils::applyLineDashing(getB2DPolygon(), aDash, &aDashedPolyPolyA,
                                         &aDashedPolyPolyB, 2.0 * fLogicDashLength);

        rContainer.push_back(new PolyPolygonHairlinePrimitive2D(aDashedPolyPolyA, getRGBColorA()));
        rContainer.push_back(new PolyPolygonHairlinePrimitive2D(aDashedPolyPolyB, getRGBColorB()));
    }
    else
    {
        rContainer.push_back(new PolygonHairlinePrimitive2D(getB2DPolygon(), getRGBColorA()));
    }
}

PolygonMarkerPrimitive2D::PolygonMarkerPrimitive2D(const basegfx::B2DPolygon& rPolygon,
                                                   const basegfx::BColor& rRGBColorA,
                                                   const basegfx::BColor& rRGBColorB,
                                                   double fDiscreteDashLength)
    : maPolygon(rPolygon)
    , maRGBColorA(rRGBColorA)
    , maRGBColorB(rRGBColorB)
    , mfDiscreteDashLength(fDiscreteDashLength)
{
}

bool PolygonMarkerPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PolygonMarkerPrimitive2D& rCompare
            = static_cast<const PolygonMarkerPrimitive2D&>(rPrimitive);

        return (getB2DPolygon() == rCompare.getB2DPolygon()
                && getRGBColorA() == rCompare.getRGBColorA()
                && getRGBColorB() == rCompare.getRGBColorB()
                && getDiscreteDashLength() == rCompare.getDiscreteDashLength());
    }

    return false;
}

basegfx::B2DRange
PolygonMarkerPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    // this is a hairline, thus the line width is view-dependent. Get range of polygon
    // as base size
    basegfx::B2DRange aRetval(getB2DPolygon().getB2DRange());

    if (!aRetval.isEmpty())
    {
        // Calculate view-dependent hairline width
        const basegfx::B2DVector aDiscreteSize(
            rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
        const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

        if (basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
        {
            aRetval.grow(fDiscreteHalfLineWidth);
        }
    }

    // return range
    return aRetval;
}

void PolygonMarkerPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    bool bNeedNewDecomposition(false);

    if (!getBuffered2DDecomposition().empty())
    {
        if (rViewInformation.getInverseObjectToViewTransformation()
            != maLastInverseObjectToViewTransformation)
        {
            bNeedNewDecomposition = true;
        }
    }

    if (bNeedNewDecomposition)
    {
        // conditions of last local decomposition have changed, delete
        const_cast<PolygonMarkerPrimitive2D*>(this)->setBuffered2DDecomposition(
            Primitive2DContainer());
    }

    if (getBuffered2DDecomposition().empty())
    {
        // remember last used InverseObjectToViewTransformation
        PolygonMarkerPrimitive2D* pThat = const_cast<PolygonMarkerPrimitive2D*>(this);
        pThat->maLastInverseObjectToViewTransformation
            = rViewInformation.getInverseObjectToViewTransformation();
    }

    // use parent implementation
    BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
}

// provide unique ID
sal_uInt32 PolygonMarkerPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D;
}

} // end of namespace

namespace drawinglayer::primitive2d
{
void PolygonStrokePrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (!getB2DPolygon().count())
        return;

    // #i102241# try to simplify before usage
    const basegfx::B2DPolygon aB2DPolygon(basegfx::utils::simplifyCurveSegments(getB2DPolygon()));
    basegfx::B2DPolyPolygon aHairLinePolyPolygon;

    if (getStrokeAttribute().isDefault() || 0.0 == getStrokeAttribute().getFullDotDashLen())
    {
        // no line dashing, just copy
        aHairLinePolyPolygon.append(aB2DPolygon);
    }
    else
    {
        // apply LineStyle
        basegfx::utils::applyLineDashing(aB2DPolygon, getStrokeAttribute().getDotDashArray(),
                                         &aHairLinePolyPolygon, nullptr,
                                         getStrokeAttribute().getFullDotDashLen());
    }

    const sal_uInt32 nCount(aHairLinePolyPolygon.count());

    if (!getLineAttribute().isDefault() && getLineAttribute().getWidth())
    {
        // create fat line data
        const double fHalfLineWidth(getLineAttribute().getWidth() / 2.0);
        const basegfx::B2DLineJoin aLineJoin(getLineAttribute().getLineJoin());
        const css::drawing::LineCap aLineCap(getLineAttribute().getLineCap());
        basegfx::B2DPolyPolygon aAreaPolyPolygon;
        const double fMiterMinimumAngle(getLineAttribute().getMiterMinimumAngle());

        for (sal_uInt32 a(0); a < nCount; a++)
        {
            // New version of createAreaGeometry; now creates bezier polygons
            aAreaPolyPolygon.append(basegfx::utils::createAreaGeometry(
                aHairLinePolyPolygon.getB2DPolygon(a), fHalfLineWidth, aLineJoin, aLineCap,
                basegfx::deg2rad(12.5) /* default fMaxAllowedAngle*/,
                0.4 /* default fMaxPartOfEdge*/, fMiterMinimumAngle));
        }

        // create primitive
        for (sal_uInt32 b(0); b < aAreaPolyPolygon.count(); b++)
        {
            // put into single polyPolygon primitives to make clear that this is NOT meant
            // to be painted as a single tools::PolyPolygon (XORed as fill rule). Alternatively, a
            // melting process may be used here one day.
            const basegfx::B2DPolyPolygon aNewPolyPolygon(aAreaPolyPolygon.getB2DPolygon(b));
            const basegfx::BColor aColor(getLineAttribute().getColor());
            rContainer.push_back(new PolyPolygonColorPrimitive2D(aNewPolyPolygon, aColor));
        }
    }
    else
    {
        rContainer.push_back(new PolyPolygonHairlinePrimitive2D(aHairLinePolyPolygon,
                                                                getLineAttribute().getColor()));
    }
}

PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(
    const basegfx::B2DPolygon& rPolygon, const attribute::LineAttribute& rLineAttribute,
    const attribute::StrokeAttribute& rStrokeAttribute)
    : maPolygon(rPolygon)
    , maLineAttribute(rLineAttribute)
    , maStrokeAttribute(rStrokeAttribute)
{
    // MM01: keep these - these are no curve-decompposers but just checks
    // simplify curve segments: moved here to not need to use it
    // at VclPixelProcessor2D::tryDrawPolygonStrokePrimitive2DDirect
    maPolygon = basegfx::utils::simplifyCurveSegments(maPolygon);
}

PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(const basegfx::B2DPolygon& rPolygon,
                                                   const attribute::LineAttribute& rLineAttribute)
    : maPolygon(rPolygon)
    , maLineAttribute(rLineAttribute)
{
    // MM01: keep these - these are no curve-decompposers but just checks
    // simplify curve segments: moved here to not need to use it
    // at VclPixelProcessor2D::tryDrawPolygonStrokePrimitive2DDirect
    maPolygon = basegfx::utils::simplifyCurveSegments(maPolygon);
}

bool PolygonStrokePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PolygonStrokePrimitive2D& rCompare
            = static_cast<const PolygonStrokePrimitive2D&>(rPrimitive);

        return (getB2DPolygon() == rCompare.getB2DPolygon()
                && getLineAttribute() == rCompare.getLineAttribute()
                && getStrokeAttribute() == rCompare.getStrokeAttribute());
    }

    return false;
}

basegfx::B2DRange
PolygonStrokePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DRange aRetval;

    if (getLineAttribute().getWidth())
    {
        bool bUseDecomposition(false);

        if (basegfx::B2DLineJoin::Miter == getLineAttribute().getLineJoin())
        {
            // if line is mitered, use parent call since mitered line
            // geometry may use more space than the geometry grown by half line width
            bUseDecomposition = true;
        }

        if (!bUseDecomposition && css::drawing::LineCap_SQUARE == getLineAttribute().getLineCap())
        {
            // when drawing::LineCap_SQUARE is used the below method to grow the polygon
            // range by half line width will not work, so use decomposition. Interestingly,
            // the grow method below works perfectly for LineCap_ROUND since the grow is in
            // all directions and the rounded cap needs the same grow in all directions independent
            // from its orientation. Unfortunately this is not the case for drawing::LineCap_SQUARE
            bUseDecomposition = true;
        }

        if (bUseDecomposition)
        {
            // get correct range by using the decomposition fallback, reasons see above cases

            // ofz#947 to optimize calculating the range, turn any slow dashes into a solid line
            // when just calculating bounds
            attribute::StrokeAttribute aOrigStrokeAttribute = maStrokeAttribute;
            const_cast<PolygonStrokePrimitive2D*>(this)->maStrokeAttribute
                = attribute::StrokeAttribute();
            aRetval = BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
            const_cast<PolygonStrokePrimitive2D*>(this)->maStrokeAttribute = aOrigStrokeAttribute;
        }
        else
        {
            // for all other B2DLINEJOIN_* get the range from the base geometry
            // and expand by half the line width
            aRetval = getB2DPolygon().getB2DRange();
            aRetval.grow(getLineAttribute().getWidth() * 0.5);
        }
    }
    else
    {
        // this is a hairline, thus the line width is view-dependent. Get range of polygon
        // as base size
        aRetval = getB2DPolygon().getB2DRange();

        if (!aRetval.isEmpty())
        {
            // Calculate view-dependent hairline width
            const basegfx::B2DVector aDiscreteSize(
                rViewInformation.getInverseObjectToViewTransformation()
                * basegfx::B2DVector(1.0, 0.0));
            const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

            if (basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
            {
                aRetval.grow(fDiscreteHalfLineWidth);
            }
        }
    }

    return aRetval;
}

// provide unique ID
sal_uInt32 PolygonStrokePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D;
}

void PolygonWavePrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (!getB2DPolygon().count())
        return;

    const bool bHasWidth(!basegfx::fTools::equalZero(getWaveWidth()));
    const bool bHasHeight(!basegfx::fTools::equalZero(getWaveHeight()));

    if (bHasWidth && bHasHeight)
    {
        // create waveline curve
        const basegfx::B2DPolygon aWaveline(
            basegfx::utils::createWaveline(getB2DPolygon(), getWaveWidth(), getWaveHeight()));
        rContainer.push_back(
            new PolygonStrokePrimitive2D(aWaveline, getLineAttribute(), getStrokeAttribute()));
    }
    else
    {
        // flat waveline, decompose to simple line primitive
        rContainer.push_back(new PolygonStrokePrimitive2D(getB2DPolygon(), getLineAttribute(),
                                                          getStrokeAttribute()));
    }
}

PolygonWavePrimitive2D::PolygonWavePrimitive2D(const basegfx::B2DPolygon& rPolygon,
                                               const attribute::LineAttribute& rLineAttribute,
                                               const attribute::StrokeAttribute& rStrokeAttribute,
                                               double fWaveWidth, double fWaveHeight)
    : PolygonStrokePrimitive2D(rPolygon, rLineAttribute, rStrokeAttribute)
    , mfWaveWidth(fWaveWidth)
    , mfWaveHeight(fWaveHeight)
{
    if (mfWaveWidth < 0.0)
    {
        mfWaveWidth = 0.0;
    }

    if (mfWaveHeight < 0.0)
    {
        mfWaveHeight = 0.0;
    }
}

PolygonWavePrimitive2D::PolygonWavePrimitive2D(const basegfx::B2DPolygon& rPolygon,
                                               const attribute::LineAttribute& rLineAttribute,
                                               double fWaveWidth, double fWaveHeight)
    : PolygonStrokePrimitive2D(rPolygon, rLineAttribute)
    , mfWaveWidth(fWaveWidth)
    , mfWaveHeight(fWaveHeight)
{
    if (mfWaveWidth < 0.0)
    {
        mfWaveWidth = 0.0;
    }

    if (mfWaveHeight < 0.0)
    {
        mfWaveHeight = 0.0;
    }
}

bool PolygonWavePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (PolygonStrokePrimitive2D::operator==(rPrimitive))
    {
        const PolygonWavePrimitive2D& rCompare
            = static_cast<const PolygonWavePrimitive2D&>(rPrimitive);

        return (getWaveWidth() == rCompare.getWaveWidth()
                && getWaveHeight() == rCompare.getWaveHeight());
    }

    return false;
}

basegfx::B2DRange
PolygonWavePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    // get range of parent
    basegfx::B2DRange aRetval(PolygonStrokePrimitive2D::getB2DRange(rViewInformation));

    // if WaveHeight, grow by it
    if (basegfx::fTools::more(getWaveHeight(), 0.0))
    {
        aRetval.grow(getWaveHeight());
    }

    // if line width, grow by it
    if (basegfx::fTools::more(getLineAttribute().getWidth(), 0.0))
    {
        aRetval.grow(getLineAttribute().getWidth() * 0.5);
    }

    return aRetval;
}

// provide unique ID
sal_uInt32 PolygonWavePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D;
}

void PolygonStrokeArrowPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // copy local polygon, it may be changed
    basegfx::B2DPolygon aLocalPolygon(getB2DPolygon());
    aLocalPolygon.removeDoublePoints();
    basegfx::B2DPolyPolygon aArrowA;
    basegfx::B2DPolyPolygon aArrowB;

    if (!aLocalPolygon.isClosed() && aLocalPolygon.count() > 1)
    {
        // apply arrows
        const double fPolyLength(basegfx::utils::getLength(aLocalPolygon));
        double fStart(0.0);
        double fEnd(0.0);
        double fStartOverlap(0.0);
        double fEndOverlap(0.0);

        if (!getStart().isDefault() && getStart().isActive())
        {
            // create start arrow primitive and consume
            aArrowA = basegfx::utils::createAreaGeometryForLineStartEnd(
                aLocalPolygon, getStart().getB2DPolyPolygon(), true, getStart().getWidth(),
                fPolyLength, getStart().isCentered() ? 0.5 : 0.0, &fStart);

            // create some overlapping, compromise between straight and peaked markers
            // for marker width 0.3cm and marker line width 0.02cm
            fStartOverlap = getStart().getWidth() / 15.0;
        }

        if (!getEnd().isDefault() && getEnd().isActive())
        {
            // create end arrow primitive and consume
            aArrowB = basegfx::utils::createAreaGeometryForLineStartEnd(
                aLocalPolygon, getEnd().getB2DPolyPolygon(), false, getEnd().getWidth(),
                fPolyLength, getEnd().isCentered() ? 0.5 : 0.0, &fEnd);

            // create some overlapping
            fEndOverlap = getEnd().getWidth() / 15.0;
        }

        if (0.0 != fStart || 0.0 != fEnd)
        {
            // build new poly, consume something from old poly
            aLocalPolygon
                = basegfx::utils::getSnippetAbsolute(aLocalPolygon, fStart - fStartOverlap,
                                                     fPolyLength - fEnd + fEndOverlap, fPolyLength);
        }
    }

    // add shaft
    rContainer.push_back(
        new PolygonStrokePrimitive2D(aLocalPolygon, getLineAttribute(), getStrokeAttribute()));

    if (aArrowA.count())
    {
        rContainer.push_back(
            new PolyPolygonColorPrimitive2D(aArrowA, getLineAttribute().getColor()));
    }

    if (aArrowB.count())
    {
        rContainer.push_back(
            new PolyPolygonColorPrimitive2D(aArrowB, getLineAttribute().getColor()));
    }
}

PolygonStrokeArrowPrimitive2D::PolygonStrokeArrowPrimitive2D(
    const basegfx::B2DPolygon& rPolygon, const attribute::LineAttribute& rLineAttribute,
    const attribute::StrokeAttribute& rStrokeAttribute,
    const attribute::LineStartEndAttribute& rStart, const attribute::LineStartEndAttribute& rEnd)
    : PolygonStrokePrimitive2D(rPolygon, rLineAttribute, rStrokeAttribute)
    , maStart(rStart)
    , maEnd(rEnd)
{
}

PolygonStrokeArrowPrimitive2D::PolygonStrokeArrowPrimitive2D(
    const basegfx::B2DPolygon& rPolygon, const attribute::LineAttribute& rLineAttribute,
    const attribute::LineStartEndAttribute& rStart, const attribute::LineStartEndAttribute& rEnd)
    : PolygonStrokePrimitive2D(rPolygon, rLineAttribute)
    , maStart(rStart)
    , maEnd(rEnd)
{
}

bool PolygonStrokeArrowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (PolygonStrokePrimitive2D::operator==(rPrimitive))
    {
        const PolygonStrokeArrowPrimitive2D& rCompare
            = static_cast<const PolygonStrokeArrowPrimitive2D&>(rPrimitive);

        return (getStart() == rCompare.getStart() && getEnd() == rCompare.getEnd());
    }

    return false;
}

basegfx::B2DRange PolygonStrokeArrowPrimitive2D::getB2DRange(
    const geometry::ViewInformation2D& rViewInformation) const
{
    if (getStart().isActive() || getEnd().isActive())
    {
        // use decomposition when line start/end is used
        return BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
    }
    else
    {
        // get range from parent
        return PolygonStrokePrimitive2D::getB2DRange(rViewInformation);
    }
}

// provide unique ID
sal_uInt32 PolygonStrokeArrowPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
