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

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <svtools/borderhelper.hxx>
#include <numeric>
#include <algorithm>

namespace drawinglayer {

namespace {

void moveLine(basegfx::B2DPolygon& rPoly, double fGap, const basegfx::B2DVector& rVector)
{
    if (basegfx::fTools::equalZero(rVector.getX()))
    {
        basegfx::B2DHomMatrix aMat(1, 0, fGap, 0, 1, 0);
        rPoly.transform(aMat);
    }
    else if (basegfx::fTools::equalZero(rVector.getY()))
    {
        basegfx::B2DHomMatrix aMat(1, 0, 0, 0, 1, fGap);
        rPoly.transform(aMat);
    }
}

primitive2d::Primitive2DReference makeHairLinePrimitive(
    const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd, const basegfx::B2DVector& rVector,
    const basegfx::BColor& rColor, double fGap)
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(rStart);
    aPolygon.append(rEnd);
    moveLine(aPolygon, fGap, rVector);

    return primitive2d::Primitive2DReference(new primitive2d::PolygonHairlinePrimitive2D(aPolygon, rColor));
}

primitive2d::Primitive2DReference makeSolidLinePrimitive(
    const basegfx::B2DPolyPolygon& rClipRegion, const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd,
    const basegfx::B2DVector& rVector, const basegfx::BColor& rColor, double fLineWidth, double fGap)
{
    const basegfx::B2DVector aPerpendicular = basegfx::getPerpendicular(rVector);
    const basegfx::B2DVector aLineWidthOffset = ((fLineWidth + 1.0) * 0.5) * aPerpendicular;

    basegfx::B2DPolygon aPolygon;
    aPolygon.append(rStart + aLineWidthOffset);
    aPolygon.append(rEnd + aLineWidthOffset);
    aPolygon.append(rEnd - aLineWidthOffset);
    aPolygon.append(rStart - aLineWidthOffset);
    aPolygon.setClosed(true);

    moveLine(aPolygon, fGap, rVector);

    basegfx::B2DPolyPolygon aClipped =
        basegfx::tools::clipPolygonOnPolyPolygon(aPolygon, rClipRegion, true, false);

    if (aClipped.count())
        aPolygon = aClipped.getB2DPolygon(0);

    return primitive2d::Primitive2DReference(
        new primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon), rColor));
}

}

    // fdo#49438: heuristic pseudo hack
    static bool lcl_UseHairline(double const fW,
            basegfx::B2DPoint const& rStart, basegfx::B2DPoint const& rEnd,
            geometry::ViewInformation2D const& rViewInformation)
    {
        basegfx::B2DTuple scale;
        basegfx::B2DTuple translation;
        double fRotation;
        double fShear;
        rViewInformation.getObjectToViewTransformation().decompose(
                scale, translation, fRotation, fShear);
        double const fScale(
            (rEnd.getX() - rStart.getX() > rEnd.getY() - rStart.getY())
                ? scale.getY() : scale.getX());
        return (fW * fScale < 0.51);
    }

    static double lcl_GetCorrectedWidth(double const fW,
            basegfx::B2DPoint const& rStart, basegfx::B2DPoint const& rEnd,
            geometry::ViewInformation2D const& rViewInformation)
    {
        return (lcl_UseHairline(fW, rStart, rEnd, rViewInformation)) ? 0.0 : fW;
    }

    namespace primitive2d
    {
        double BorderLinePrimitive2D::getWidth(
            geometry::ViewInformation2D const& rViewInformation) const
        {
            return lcl_GetCorrectedWidth(mfLeftWidth, getStart(), getEnd(),
                        rViewInformation)
                 + lcl_GetCorrectedWidth(mfDistance, getStart(), getEnd(),
                         rViewInformation)
                 + lcl_GetCorrectedWidth(mfRightWidth, getStart(), getEnd(),
                         rViewInformation);
        }

        basegfx::B2DPolyPolygon BorderLinePrimitive2D::getClipPolygon(
            geometry::ViewInformation2D const& rViewInformation) const
        {
            basegfx::B2DPolygon clipPolygon;

            // Get the vectors
            basegfx::B2DVector aVector( getEnd() - getStart() );
            aVector.normalize();
            const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

            // Get the points
            const double fWidth(getWidth(rViewInformation));
            const basegfx::B2DVector aLeftOff(
                    aPerpendicular * (-0.5 * std::max(fWidth, 1.0)));
            const basegfx::B2DVector aRightOff(
                    aPerpendicular * (0.5 * std::max(fWidth, 1.0)));

            const basegfx::B2DVector aSLVector( aLeftOff - ( getExtendLeftStart() * aVector ) );
            clipPolygon.append( basegfx::B2DPoint( getStart() + aSLVector * 2.0 ) );

            clipPolygon.append( getStart( ) );

            const basegfx::B2DVector aSRVector( aRightOff - ( getExtendRightStart() * aVector ) );
            clipPolygon.append( basegfx::B2DPoint( getStart() + aSRVector * 2.0 ) );

            const basegfx::B2DVector aERVector( aRightOff + ( getExtendRightEnd() * aVector ) );
            clipPolygon.append( basegfx::B2DPoint( getEnd() + aERVector * 2.0 ) );

            clipPolygon.append( getEnd( ) );

            const basegfx::B2DVector aELVector( aLeftOff + ( getExtendLeftEnd() * aVector ) );
            clipPolygon.append( basegfx::B2DPoint( getEnd() + aELVector * 2.0 ) );

            clipPolygon.setClosed( true );

            return basegfx::B2DPolyPolygon( clipPolygon );
        }

        Primitive2DSequence BorderLinePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence xRetval;

            if(!getStart().equal(getEnd()) && ( isInsideUsed() || isOutsideUsed() ) )
            {
                // get data and vectors
                basegfx::B2DVector aVector(getEnd() - getStart());
                aVector.normalize();
                const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

                const basegfx::B2DPolyPolygon& aClipRegion =
                    getClipPolygon(rViewInformation);

                if(isOutsideUsed() && isInsideUsed())
                {
                    const double fExt = getWidth(rViewInformation);  // Extend a lot: it'll be clipped later.
                    const basegfx::B2DPoint aTmpStart(getStart() - (fExt * aVector));
                    const basegfx::B2DPoint aTmpEnd(getEnd() + (fExt * aVector));

                    xRetval.realloc(2);

                    {
                        // "inside" line
                        double fWidth = getLeftWidth();
                        basegfx::BColor aColor = getRGBColorLeft();
                        bool bIsHairline = lcl_UseHairline(
                                fWidth, getStart(), getEnd(), rViewInformation);
                        fWidth = lcl_GetCorrectedWidth(fWidth,
                                    getStart(), getEnd(), rViewInformation);

                        if (bIsHairline)
                            xRetval[0] = makeHairLinePrimitive(getStart(), getEnd(), aVector, aColor, 0.0);
                        else
                            xRetval[0] = makeSolidLinePrimitive(
                                aClipRegion, aTmpStart, aTmpEnd, aVector, aColor, fWidth, 0.0);
                    }

                    {
                        // "outside" line
                        double fWidth = getRightWidth();
                        basegfx::BColor aColor = getRGBColorRight();
                        bool bIsHairline = lcl_UseHairline(
                                fWidth, getStart(), getEnd(), rViewInformation);
                        fWidth = lcl_GetCorrectedWidth(fWidth,
                                    getStart(), getEnd(), rViewInformation);

                        if (bIsHairline)
                            xRetval[1] = makeHairLinePrimitive(getStart(), getEnd(), aVector, aColor, mfDistance);
                        else
                            xRetval[1] = makeSolidLinePrimitive(
                                aClipRegion, aTmpStart, aTmpEnd, aVector, aColor, fWidth, mfDistance);
                    }
                }
                else
                {
                    // single line, create geometry
                    basegfx::B2DPolygon aPolygon;
                    const double fExt = getWidth(rViewInformation);  // Extend a lot: it'll be clipped after
                    const basegfx::B2DPoint aTmpStart(getStart() - (fExt * aVector));
                    const basegfx::B2DPoint aTmpEnd(getEnd() + (fExt * aVector));

                    // Get which is the line to show
                    bool bIsSolidline = isSolidLine();
                    double nWidth = getLeftWidth();
                    basegfx::BColor aColor = getRGBColorLeft();
                    if ( basegfx::fTools::equal( 0.0, mfLeftWidth ) )
                    {
                        nWidth = getRightWidth();
                        aColor = getRGBColorRight();
                    }
                    bool const bIsHairline = lcl_UseHairline(
                            nWidth, getStart(), getEnd(), rViewInformation);
                    nWidth = lcl_GetCorrectedWidth(nWidth,
                                getStart(), getEnd(), rViewInformation);

                    if(bIsHairline && bIsSolidline)
                    {
                        // create hairline primitive
                        aPolygon.append( getStart() );
                        aPolygon.append( getEnd() );

                        xRetval.realloc(1);
                        xRetval[0] = Primitive2DReference(new PolygonHairlinePrimitive2D(
                            aPolygon,
                            aColor));
                    }
                    else
                    {
                        // create filled polygon primitive
                        const basegfx::B2DVector aLineWidthOffset(((nWidth + 1) * 0.5) * aPerpendicular);

                        aPolygon.append( aTmpStart );
                        aPolygon.append( aTmpEnd );

                        basegfx::B2DPolyPolygon aDashed =
                            svtools::ApplyLineDashing(aPolygon, getStyle(), mfPatternScale*10.0);

                        for (sal_uInt32 i = 0; i < aDashed.count(); i++ )
                        {
                            basegfx::B2DPolygon aDash = aDashed.getB2DPolygon( i );
                            basegfx::B2DPoint aDashStart = aDash.getB2DPoint( 0 );
                            basegfx::B2DPoint aDashEnd = aDash.getB2DPoint( aDash.count() - 1 );

                            basegfx::B2DPolygon aDashPolygon;
                            aDashPolygon.append( aDashStart + aLineWidthOffset );
                            aDashPolygon.append( aDashEnd + aLineWidthOffset );
                            aDashPolygon.append( aDashEnd - aLineWidthOffset );
                            aDashPolygon.append( aDashStart - aLineWidthOffset );
                            aDashPolygon.setClosed( true );

                            basegfx::B2DPolyPolygon aClipped = basegfx::tools::clipPolygonOnPolyPolygon(
                                aDashPolygon, aClipRegion, true, false );

                            if ( aClipped.count() )
                                aDashed.setB2DPolygon( i, aClipped.getB2DPolygon( 0 ) );
                        }

                        sal_uInt32 n = aDashed.count();
                        xRetval.realloc(n);
                        for (sal_uInt32 i = 0; i < n; ++i)
                        {
                            basegfx::B2DPolygon aDash = aDashed.getB2DPolygon(i);
                            if (bIsHairline)
                            {
                                // Convert a rectanglar polygon into a line.
                                basegfx::B2DPolygon aDash2;
                                basegfx::B2DRange aRange = aDash.getB2DRange();
                                aDash2.append(basegfx::B2DPoint(aRange.getMinX(), aRange.getMinY()));
                                aDash2.append(basegfx::B2DPoint(aRange.getMaxX(), aRange.getMinY()));
                                xRetval[i] = Primitive2DReference(
                                    new PolygonHairlinePrimitive2D(aDash2, aColor));
                            }
                            else
                            {
                                xRetval[i] = Primitive2DReference(
                                    new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aDash), aColor));
                            }
                        }
                    }
                }
            }

            return xRetval;
        }

        BorderLinePrimitive2D::BorderLinePrimitive2D(
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            double fLeftWidth,
            double fDistance,
            double fRightWidth,
            double fExtendLeftStart,
            double fExtendLeftEnd,
            double fExtendRightStart,
            double fExtendRightEnd,
            const basegfx::BColor& rRGBColorRight,
            const basegfx::BColor& rRGBColorLeft,
            const basegfx::BColor& rRGBColorGap,
            bool bHasGapColor,
            const short nStyle,
            double fPatternScale)
        :   BufferedDecompositionPrimitive2D(),
            maStart(rStart),
            maEnd(rEnd),
            mfLeftWidth(fLeftWidth),
            mfDistance(fDistance),
            mfRightWidth(fRightWidth),
            mfExtendLeftStart(fExtendLeftStart),
            mfExtendLeftEnd(fExtendLeftEnd),
            mfExtendRightStart(fExtendRightStart),
            mfExtendRightEnd(fExtendRightEnd),
            maRGBColorRight(rRGBColorRight),
            maRGBColorLeft(rRGBColorLeft),
            maRGBColorGap(rRGBColorGap),
            mbHasGapColor(bHasGapColor),
            mnStyle(nStyle),
            mfPatternScale(fPatternScale)
        {
        }

        bool BorderLinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const BorderLinePrimitive2D& rCompare = (BorderLinePrimitive2D&)rPrimitive;

                return (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd()
                    && getLeftWidth() == rCompare.getLeftWidth()
                    && getDistance() == rCompare.getDistance()
                    && getRightWidth() == rCompare.getRightWidth()
                    && getExtendLeftStart() == rCompare.getExtendLeftStart()
                    && getExtendLeftEnd() == rCompare.getExtendLeftEnd()
                    && getExtendRightStart() == rCompare.getExtendRightStart()
                    && getExtendRightEnd() == rCompare.getExtendRightEnd()
                    && getRGBColorRight() == rCompare.getRGBColorRight()
                    && getRGBColorLeft() == rCompare.getRGBColorLeft()
                    && getRGBColorGap() == rCompare.getRGBColorGap()
                    && hasGapColor() == rCompare.hasGapColor()
                    && getStyle() == rCompare.getStyle()
                    && getPatternScale() == rCompare.getPatternScale());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(BorderLinePrimitive2D, PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
