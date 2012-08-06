/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
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
                const double fWidth(getWidth(rViewInformation));
                basegfx::B2DVector aVector(getEnd() - getStart());
                aVector.normalize();
                const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

                const basegfx::B2DPolyPolygon& aClipRegion =
                    getClipPolygon(rViewInformation);

                if(isOutsideUsed() && isInsideUsed())
                {
                    const double fExt = getWidth(rViewInformation);  // Extend a lot: it'll be clipped after

                    // both used, double line definition. Create left and right offset
                    xRetval.realloc(2);
                    sal_uInt32 nInsert(0);

                    basegfx::B2DPolygon aGap;

                    {
                        // create geometry for left
                        const basegfx::B2DVector aLeftOff(aPerpendicular * (0.5 * (lcl_GetCorrectedWidth(mfLeftWidth, getStart(), getEnd(), rViewInformation) - fWidth + 1)));
                        const basegfx::B2DPoint aTmpStart(getStart() + aLeftOff - ( fExt * aVector));
                        const basegfx::B2DPoint aTmpEnd(getEnd() + aLeftOff + ( fExt * aVector));
                        basegfx::B2DPolygon aLeft;

                        if (lcl_UseHairline(mfLeftWidth, getStart(), getEnd(),
                                    rViewInformation))
                        {
                            // create hairline primitive
                            aLeft.append(aTmpStart);
                            aLeft.append(aTmpEnd);

                            basegfx::B2DPolyPolygon const aClipped =
                                basegfx::tools::clipPolygonOnPolyPolygon(
                                    aLeft, aClipRegion, true, true);

                            xRetval[nInsert++] =
                                new PolyPolygonHairlinePrimitive2D(
                                    aClipped,
                                    getRGBColorLeft());

                            aGap.append( getStart() - getExtendLeftStart() * aVector );
                            aGap.append( getEnd() + getExtendLeftEnd() * aVector );
                        }
                        else
                        {
                            // create filled polygon primitive. Already tried to create thick lines
                            // with the correct LineWidth, but this leads to problems when no AA
                            // is available and fat line special case reductions between 0.5 < x < 2.5 line widths
                            // are executed due to the FilledPolygon-do-not-paint-their-bottom-and-right-lines.
                            const basegfx::B2DVector aLineWidthOffset((lcl_GetCorrectedWidth(mfLeftWidth, getStart(), getEnd(), rViewInformation) * 0.5) * aPerpendicular);

                            aLeft.append(aTmpStart + aLineWidthOffset);
                            aLeft.append(aTmpEnd + aLineWidthOffset);
                            aLeft.append(aTmpEnd - aLineWidthOffset);
                            aLeft.append(aTmpStart - aLineWidthOffset);
                            aLeft.setClosed(true);

                            basegfx::B2DPolyPolygon aClipped = basegfx::tools::clipPolygonOnPolyPolygon(
                                    aLeft, aClipRegion, true, false );

                            aGap.append( aTmpStart + aLineWidthOffset );
                            aGap.append( aTmpEnd + aLineWidthOffset );

                            xRetval[nInsert++] = Primitive2DReference(new PolyPolygonColorPrimitive2D(
                                aClipped, getRGBColorLeft()));
                        }
                    }

                    {
                        // create geometry for right
                        const basegfx::B2DVector aRightOff(aPerpendicular * (0.5 * (fWidth - lcl_GetCorrectedWidth(mfRightWidth, getStart(), getEnd(), rViewInformation) + 1)));
                        const basegfx::B2DPoint aTmpStart(getStart() + aRightOff - ( fExt * aVector));
                        const basegfx::B2DPoint aTmpEnd(getEnd() + aRightOff + ( fExt * aVector));
                        basegfx::B2DPolygon aRight;

                        if (lcl_UseHairline(mfRightWidth, getStart(), getEnd(),
                                    rViewInformation))
                        {
                            // create hairline primitive
                            aRight.append(aTmpStart);
                            aRight.append(aTmpEnd);

                            basegfx::B2DPolyPolygon const aClipped =
                                basegfx::tools::clipPolygonOnPolyPolygon(
                                    aRight, aClipRegion, true, true);

                            xRetval[nInsert++] =
                                new PolyPolygonHairlinePrimitive2D(
                                    aClipped,
                                    getRGBColorRight());

                            aGap.append( getStart() - getExtendRightStart() * aVector );
                            aGap.append( getEnd() + getExtendRightEnd() * aVector );
                        }
                        else
                        {
                            // create filled polygon primitive
                            const basegfx::B2DVector aLineWidthOffset((lcl_GetCorrectedWidth(mfRightWidth, getStart(), getEnd(), rViewInformation) * 0.5) * aPerpendicular);

                            aRight.append(aTmpStart + aLineWidthOffset);
                            aRight.append(aTmpEnd + aLineWidthOffset);
                            aRight.append(aTmpEnd - aLineWidthOffset);
                            aRight.append(aTmpStart - aLineWidthOffset);
                            aRight.setClosed(true);

                            basegfx::B2DPolyPolygon aClipped = basegfx::tools::clipPolygonOnPolyPolygon(
                                    aRight, aClipRegion, true, false );

                            xRetval[nInsert++] = Primitive2DReference(new PolyPolygonColorPrimitive2D(
                                aClipped, getRGBColorRight()));

                            aGap.append( aTmpEnd - aLineWidthOffset );
                            aGap.append( aTmpStart - aLineWidthOffset );
                        }
                    }

                    if (hasGapColor() && aGap.count() == 4)
                    {
                        xRetval.realloc( xRetval.getLength() + 1 );
                        // create geometry for filled gap
                        aGap.setClosed( true );

                        basegfx::B2DPolyPolygon aClipped = basegfx::tools::clipPolygonOnPolyPolygon(
                                aGap, aClipRegion, true, false );

                        xRetval[nInsert++] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
                              aClipped, getRGBColorGap() ) );
                    }
                }
                else
                {
                    // single line, create geometry
                    basegfx::B2DPolygon aPolygon;
                    const double fExt = getWidth(rViewInformation);  // Extend a lot: it'll be clipped after
                    const basegfx::B2DPoint aTmpStart(getStart() - (fExt * aVector));
                    const basegfx::B2DPoint aTmpEnd(getEnd() + (fExt * aVector));
                    xRetval.realloc(1);

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

                        xRetval[0] = Primitive2DReference(new PolygonHairlinePrimitive2D(
                            aPolygon,
                            aColor));
                    }
                    else
                    {
                        // create filled polygon primitive
                        const basegfx::B2DVector aLineWidthOffset(((nWidth + 1) * 0.5) * aPerpendicular);
                        basegfx::B2DVector aScale( rViewInformation.getInverseObjectToViewTransformation() * aVector );

                        aPolygon.append( aTmpStart );
                        aPolygon.append( aTmpEnd );

                        basegfx::B2DPolyPolygon aDashed = svtools::ApplyLineDashing(
                               aPolygon, getStyle(), MAP_PIXEL, aScale.getLength() );
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

                        xRetval[0] = Primitive2DReference(new PolyPolygonColorPrimitive2D(
                                basegfx::B2DPolyPolygon( aDashed ), aColor));
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
            const short nStyle)
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
            mnStyle(nStyle)
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
                    && getStyle() == rCompare.getStyle());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(BorderLinePrimitive2D, PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
