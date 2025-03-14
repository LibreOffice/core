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

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonMarkerPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonWavePrimitive2D.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <utility>

using namespace com::sun::star;

namespace
{
void implGrowHairline(basegfx::B2DRange& rRange,
                      const drawinglayer::geometry::ViewInformation2D& rViewInformation)
{
    if (!rRange.isEmpty())
    {
        // Calculate view-dependent hairline width
        const basegfx::B2DVector aDiscreteSize(
            rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
        const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

        if (fDiscreteHalfLineWidth > 0.0)
        {
            rRange.grow(fDiscreteHalfLineWidth);
        }
    }
}
}

namespace drawinglayer::primitive2d
{
PolygonHairlinePrimitive2D::PolygonHairlinePrimitive2D(basegfx::B2DPolygon aPolygon,
                                                       const basegfx::BColor& rBColor)
    : maPolygon(std::move(aPolygon))
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

    // Calculate and grow by view-dependent hairline width
    implGrowHairline(aRetval, rViewInformation);

    // return range
    return aRetval;
}

// provide unique ID
sal_uInt32 PolygonHairlinePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D;
}

SingleLinePrimitive2D::SingleLinePrimitive2D(const basegfx::B2DPoint& rStart,
                                             const basegfx::B2DPoint& rEnd,
                                             const basegfx::BColor& rBColor)
    : BasePrimitive2D()
    , maStart(rStart)
    , maEnd(rEnd)
    , maBColor(rBColor)
{
}

bool SingleLinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const SingleLinePrimitive2D& rCompare(
            static_cast<const SingleLinePrimitive2D&>(rPrimitive));

        return (getStart() == rCompare.getStart() && getEnd() == rCompare.getEnd()
                && getBColor() == rCompare.getBColor());
    }

    return false;
}

basegfx::B2DRange
SingleLinePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DRange aRetval(getStart(), getEnd());

    // Calculate and grow by view-dependent hairline width
    implGrowHairline(aRetval, rViewInformation);

    return aRetval;
}

sal_uInt32 SingleLinePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SINGLELINEPRIMITIVE2D;
}

void SingleLinePrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (getStart() == getEnd())
    {
        // single point
        Primitive2DContainer aSequence = { new PointArrayPrimitive2D(
            std::vector<basegfx::B2DPoint>{ getStart() }, getBColor()) };
        rVisitor.visit(aSequence);
    }
    else
    {
        // line
        Primitive2DContainer aSequence = { new PolygonHairlinePrimitive2D(
            basegfx::B2DPolygon{ getStart(), getEnd() }, getBColor()) };
        rVisitor.visit(aSequence);
    }
}

LineRectanglePrimitive2D::LineRectanglePrimitive2D(const basegfx::B2DRange& rB2DRange,
                                                   const basegfx::BColor& rBColor)
    : BasePrimitive2D()
    , maB2DRange(rB2DRange)
    , maBColor(rBColor)
{
}

bool LineRectanglePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const LineRectanglePrimitive2D& rCompare(
            static_cast<const LineRectanglePrimitive2D&>(rPrimitive));

        return (getB2DRange() == rCompare.getB2DRange() && getBColor() == rCompare.getBColor());
    }

    return false;
}

basegfx::B2DRange
LineRectanglePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DRange aRetval(getB2DRange());

    // Calculate and grow by view-dependent hairline width
    implGrowHairline(aRetval, rViewInformation);

    return aRetval;
}

sal_uInt32 LineRectanglePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_LINERECTANGLEPRIMITIVE2D;
}

void LineRectanglePrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (getB2DRange().isEmpty())
    {
        // no geometry, done
        return;
    }

    const basegfx::B2DPolygon aPolygon(basegfx::utils::createPolygonFromRect(getB2DRange()));
    Primitive2DContainer aSequence = { new PolygonHairlinePrimitive2D(aPolygon, getBColor()) };
    rVisitor.visit(aSequence);
}

Primitive2DReference PolygonMarkerPrimitive2D::create2DDecomposition(
    const geometry::ViewInformation2D& rViewInformation) const
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

        Primitive2DContainer aContainer;
        aContainer.push_back(
            new PolyPolygonHairlinePrimitive2D(std::move(aDashedPolyPolyA), getRGBColorA()));
        aContainer.push_back(
            new PolyPolygonHairlinePrimitive2D(std::move(aDashedPolyPolyB), getRGBColorB()));
        return new GroupPrimitive2D(std::move(aContainer));
    }
    else
    {
        return new PolygonHairlinePrimitive2D(getB2DPolygon(), getRGBColorA());
    }
}

PolygonMarkerPrimitive2D::PolygonMarkerPrimitive2D(basegfx::B2DPolygon aPolygon,
                                                   const basegfx::BColor& rRGBColorA,
                                                   const basegfx::BColor& rRGBColorB,
                                                   double fDiscreteDashLength)
    : maPolygon(std::move(aPolygon))
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

        if (fDiscreteHalfLineWidth > 0.0)
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

    if (hasBuffered2DDecomposition())
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
        const_cast<PolygonMarkerPrimitive2D*>(this)->setBuffered2DDecomposition(nullptr);
    }

    if (!hasBuffered2DDecomposition())
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
Primitive2DReference PolygonStrokePrimitive2D::create2DDecomposition(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (!getB2DPolygon().count())
        return nullptr;

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
        Primitive2DContainer aContainer;
        for (sal_uInt32 b(0); b < aAreaPolyPolygon.count(); b++)
        {
            // put into single polyPolygon primitives to make clear that this is NOT meant
            // to be painted as a single tools::PolyPolygon (XORed as fill rule). Alternatively, a
            // melting process may be used here one day.
            basegfx::B2DPolyPolygon aNewPolyPolygon(aAreaPolyPolygon.getB2DPolygon(b));
            const basegfx::BColor aColor(getLineAttribute().getColor());
            aContainer.push_back(
                new PolyPolygonColorPrimitive2D(std::move(aNewPolyPolygon), aColor));
        }
        return new GroupPrimitive2D(std::move(aContainer));
    }
    else
    {
        return new PolyPolygonHairlinePrimitive2D(std::move(aHairLinePolyPolygon),
                                                  getLineAttribute().getColor());
    }
}

PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(basegfx::B2DPolygon aPolygon,
                                                   const attribute::LineAttribute& rLineAttribute,
                                                   attribute::StrokeAttribute aStrokeAttribute)
    : maPolygon(std::move(aPolygon))
    , maLineAttribute(rLineAttribute)
    , maStrokeAttribute(std::move(aStrokeAttribute))
    , maBufferedRange()
{
    // MM01: keep these - these are no curve-decompposers but just checks
    // simplify curve segments: moved here to not need to use it
    // at VclPixelProcessor2D::tryDrawPolygonStrokePrimitive2DDirect
    maPolygon = basegfx::utils::simplifyCurveSegments(maPolygon);
}

PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(basegfx::B2DPolygon aPolygon,
                                                   const attribute::LineAttribute& rLineAttribute)
    : maPolygon(std::move(aPolygon))
    , maLineAttribute(rLineAttribute)
    , maBufferedRange()
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
    if (!maBufferedRange.isEmpty())
    {
        // use the view-independent, buffered B2DRange
        return maBufferedRange;
    }

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

            // NOTE: I thought about using [sqrt(2) * 0.5] a a factor for LineCap_SQUARE and not
            // set bUseDecomposition. I even tried that it works. Then an auto-test failing showed
            // not only that view-dependent stuff needs to be considered (what is done for the
            // hairline case below), *BUT* also e.g. conversions to PNG/exports use the B2DRange
            // of the geometry, so:
            // - expanding by 1/2 LineWidth is OK for rounded
            // - expanding by more (like sqrt(2) * 0.5 * LineWidth) immediately extends the size
            //   of e.g. geometry converted to PNG, plus many similar cases that cannot be thought
            //   of in advance.
            // This means that converting those thought-experiment examples in (4) will and do lead
            // to bigger e.g. Bitmap conversion(s), not avoiding but painting the free space. That
            // could only be done by correctly and fully decomposing the geometry, including
            // stroke, and accepting the cost...
            bUseDecomposition = true;
        }

        if (bUseDecomposition)
        {
            // get correct range by using the decomposition fallback, reasons see above cases

            // It is not a good idea to temporarily (re)set the PolygonStrokePrimitive2D
            // to default. While it is understandable to use the speed advantage, it is
            // bad for quite some reasons:
            //
            // (1) As described in include/drawinglayer/primitive2d/baseprimitive2d.hxx
            //     a Primitive is "noncopyable to make clear that a primitive is a read-only
            //     instance and copying or changing values is not intended". This is the base
            //     assumption for many decisions for Primitive handling.
            // (2) For example, that the decomposition is *always* re-usable. It cannot change
            //     and is correct when it already exists since the values the decomposition is
            //     based on cannot change.
            // (3) If this *is* done (like it was here) and the Primitive is derived from
            //     BufferedDecompositionPrimitive2D and thus buffers it's decomposition,
            //     the risk is that in this case the *wrong* decomposition will be used by
            //     other PrimitiveProcessors. Maybe not by the VclPixelProcessor2D/VclProcessor2D
            //     since it handles this primitive directly - not even sure for all cases.
            //     Sooner or later another PrimitiveProcessor will re-use this wrong temporary
            //     decomposition, and as an error, a non-stroked line will be painted/used.
            // (4) The B2DRange is not strictly defined as minimal bound for the geometry,
            //     but it should be as small/tight as possible. Making it larger risks more
            //     area to be invalidated (repaint) and processed (all geometric stuff,l may
            //     include future and existing exports to other formats which are or will be
            //     implemented as PrimitiveProcessor). It is easy to imagine cases with much
            //     too large B2DRange - a line with a pattern that would solve to a single
            //     small start-rectangle and rest is empty, or a circle with a stroke that
            //     makes only a quarter of it visible.
            //
            // The reason to do this is speed, what is a good argument. But speed should
            // only be used if the pair of [correctness/speed] does not sacrifice the correctness
            // over the speed.
            // Luckily there are alternatives to solve this and to keep [correctness/speed]
            // valid:
            //
            // (a) Reset the temporary decomposition after having const-casted and
            //     changed maStrokeAttribute.
            //     Disadvantage: More const-cast hacks, plus this temporary decomposition
            //     will be potentially done repeatedly (every time
            //     PolygonStrokePrimitive2D::getB2DRange is called)
            // (b) Use a temporary, local PolygonStrokePrimitive2D here, with neutral
            //     PolygonStrokePrimitive2D and call ::getB2DRange() at it. That way
            //     the buffered decomposition will not be harmed.
            //     Disadvantage: Same as (a), decomposition will be potentially done repeatedly
            // (c) Use a temporary, local PolygonStrokePrimitive2D and buffer B2DRange
            //     locally for this Primitive. Due to (1)/(2) this cannot change, so
            //     when calculated once it is totally legal to use it.
            //
            // Thus here I would use (c): It accepts the disadvantages of (4) over speed, but
            // avoids the errors/problems from (1-4).
            // Additional argument for this: The hairline case below *also* uses the full
            // B2DRange of the polygon, ignoring an evtl. stroke, so (4) applies
            if (!getStrokeAttribute().isDefault())
            {
                // only do this if StrokeAttribute is used, else recursion may happen (!)
                const rtl::Reference<primitive2d::PolygonStrokePrimitive2D>
                    aTemporaryPrimitiveWithoutStroke(new primitive2d::PolygonStrokePrimitive2D(
                        getB2DPolygon(), getLineAttribute()));
                maBufferedRange
                    = aTemporaryPrimitiveWithoutStroke
                          ->BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
            }
            else
            {
                // fallback to normal decompose, that result can be used for visualization
                // later, too. Still buffer B2DRange in maBufferedRange, so it needs to be
                // merged into one B2DRange only once
                maBufferedRange = BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
            }
        }
        else
        {
            // for all other B2DLINEJOIN_* get the range from the base geometry
            // and expand by half the line width.
            maBufferedRange = getB2DPolygon().getB2DRange();
            maBufferedRange.grow(getLineAttribute().getWidth() * 0.5);
        }

        return maBufferedRange;
    }

    // It is a hairline, thus the line width is view-dependent. Get range of polygon
    // as base size.
    // CAUTION: Since a hairline *is* view-dependent,
    // - either use maBufferedRange, additionally remember view-dependent
    //   factor & reset if that changes
    // - or do not buffer for hairline -> not really needed, the range is buffered
    //   in the B2DPolygon, no decomposition is needed and a simple grow is cheap
    basegfx::B2DRange aHairlineRange = getB2DPolygon().getB2DRange();

    if (!aHairlineRange.isEmpty())
    {
        // Calculate view-dependent hairline width
        const basegfx::B2DVector aDiscreteSize(
            rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
        const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

        if (fDiscreteHalfLineWidth > 0.0)
        {
            aHairlineRange.grow(fDiscreteHalfLineWidth);
        }
    }

    return aHairlineRange;
}

// provide unique ID
sal_uInt32 PolygonStrokePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D;
}

Primitive2DReference PolygonWavePrimitive2D::create2DDecomposition(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (!getB2DPolygon().count())
        return nullptr;

    const bool bHasWidth(!basegfx::fTools::equalZero(getWaveWidth()));
    const bool bHasHeight(!basegfx::fTools::equalZero(getWaveHeight()));

    if (bHasWidth && bHasHeight)
    {
        // create waveline curve
        basegfx::B2DPolygon aWaveline(
            basegfx::utils::createWaveline(getB2DPolygon(), getWaveWidth(), getWaveHeight()));
        return new PolygonStrokePrimitive2D(std::move(aWaveline), getLineAttribute(),
                                            getStrokeAttribute());
    }
    else
    {
        // flat waveline, decompose to simple line primitive
        return new PolygonStrokePrimitive2D(getB2DPolygon(), getLineAttribute(),
                                            getStrokeAttribute());
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
    if (getWaveHeight() > 0.0)
    {
        aRetval.grow(getWaveHeight());
    }

    // if line width, grow by it
    if (getLineAttribute().getWidth() > 0.0)
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

Primitive2DReference PolygonStrokeArrowPrimitive2D::create2DDecomposition(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
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
    Primitive2DContainer aContainer;
    aContainer.push_back(new PolygonStrokePrimitive2D(std::move(aLocalPolygon), getLineAttribute(),
                                                      getStrokeAttribute()));

    if (aArrowA.count())
    {
        aContainer.push_back(
            new PolyPolygonColorPrimitive2D(std::move(aArrowA), getLineAttribute().getColor()));
    }

    if (aArrowB.count())
    {
        aContainer.push_back(
            new PolyPolygonColorPrimitive2D(std::move(aArrowB), getLineAttribute().getColor()));
    }
    return new GroupPrimitive2D(std::move(aContainer));
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
