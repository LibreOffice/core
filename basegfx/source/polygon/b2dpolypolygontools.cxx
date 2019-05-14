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

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <algorithm>
#include <numeric>

namespace basegfx
{
    namespace utils
    {
        B2DPolyPolygon correctOrientations(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval(rCandidate);
            const sal_uInt32 nCount(aRetval.count());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const B2DPolygon& aCandidate(rCandidate.getB2DPolygon(a));
                const B2VectorOrientation aOrientation(utils::getOrientation(aCandidate));
                sal_uInt32 nDepth(0);

                for(sal_uInt32 b(0); b < nCount; b++)
                {
                    if(b != a)
                    {
                        const B2DPolygon& aCompare(rCandidate.getB2DPolygon(b));

                        if(utils::isInside(aCompare, aCandidate, true))
                        {
                            nDepth++;
                        }
                    }
                }

                const bool bShallBeHole((nDepth & 0x00000001) == 1);
                const bool bIsHole(aOrientation == B2VectorOrientation::Negative);

                if(bShallBeHole != bIsHole && aOrientation != B2VectorOrientation::Neutral)
                {
                    B2DPolygon aFlipped(aCandidate);
                    aFlipped.flip();
                    aRetval.setB2DPolygon(a, aFlipped);
                }
            }

            return aRetval;
        }

        B2DPolyPolygon correctOutmostPolygon(const B2DPolyPolygon& rCandidate)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount > 1)
            {
                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const B2DPolygon& aCandidate(rCandidate.getB2DPolygon(a));
                    sal_uInt32 nDepth(0);

                    for(sal_uInt32 b(0); b < nCount; b++)
                    {
                        if(b != a)
                        {
                            const B2DPolygon& aCompare(rCandidate.getB2DPolygon(b));

                            if(utils::isInside(aCompare, aCandidate, true))
                            {
                                nDepth++;
                            }
                        }
                    }

                    if(!nDepth)
                    {
                        B2DPolyPolygon aRetval(rCandidate);

                        if(a != 0)
                        {
                            // exchange polygon a and polygon 0
                            aRetval.setB2DPolygon(0, aCandidate);
                            aRetval.setB2DPolygon(a, rCandidate.getB2DPolygon(0));
                        }

                        // exit
                        return aRetval;
                    }
                }
            }

            return rCandidate;
        }

        B2DPolyPolygon adaptiveSubdivideByDistance(const B2DPolyPolygon& rCandidate, double fDistanceBound)
        {
            if(rCandidate.areControlPointsUsed())
            {
                B2DPolyPolygon aRetval;

                for(auto const& rPolygon : rCandidate)
                {
                    if(rPolygon.areControlPointsUsed())
                    {
                        aRetval.append(utils::adaptiveSubdivideByDistance(rPolygon, fDistanceBound));
                    }
                    else
                    {
                        aRetval.append(rPolygon);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon adaptiveSubdivideByAngle(const B2DPolyPolygon& rCandidate, double fAngleBound)
        {
            if(rCandidate.areControlPointsUsed())
            {
                B2DPolyPolygon aRetval;

                for(auto const& rPolygon : rCandidate)
                {
                    if(rPolygon.areControlPointsUsed())
                    {
                        aRetval.append(utils::adaptiveSubdivideByAngle(rPolygon, fAngleBound));
                    }
                    else
                    {
                        aRetval.append(rPolygon);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        bool isInside(const B2DPolyPolygon& rCandidate, const B2DPoint& rPoint, bool bWithBorder)
        {
            if(rCandidate.count() == 1)
            {
                return isInside(rCandidate.getB2DPolygon(0), rPoint, bWithBorder);
            }
            else
            {
                sal_Int32 nInsideCount = std::count_if(rCandidate.begin(), rCandidate.end(), [rPoint, bWithBorder](B2DPolygon polygon){ return isInside(polygon, rPoint, bWithBorder); });

                return (nInsideCount % 2);
            }
        }

        B2DRange getRange(const B2DPolyPolygon& rCandidate)
        {
            B2DRange aRetval;

            for(auto const& rPolygon : rCandidate)
            {
                aRetval.expand(utils::getRange(rPolygon));
            }

            return aRetval;
        }

        double getSignedArea(const B2DPolyPolygon& rCandidate)
        {
            double fRetval(0.0);

            for(auto const& rPolygon : rCandidate)
            {
                fRetval += utils::getSignedArea(rPolygon);
            }

            return fRetval;
        }

        double getArea(const B2DPolyPolygon& rCandidate)
        {
            return fabs(getSignedArea(rCandidate));
        }

        void applyLineDashing(const B2DPolyPolygon& rCandidate, const std::vector<double>& rDotDashArray, B2DPolyPolygon* pLineTarget, double fFullDashDotLen)
        {
            if(fFullDashDotLen == 0.0 && !rDotDashArray.empty())
            {
                // calculate fFullDashDotLen from rDotDashArray
                fFullDashDotLen = std::accumulate(rDotDashArray.begin(), rDotDashArray.end(), 0.0);
            }

            if(rCandidate.count() && fFullDashDotLen > 0.0)
            {
                B2DPolyPolygon aLineTarget;

                for(auto const& rPolygon : rCandidate)
                {
                    applyLineDashing(
                        rPolygon,
                        rDotDashArray,
                        pLineTarget ? &aLineTarget : nullptr,
                        nullptr,
                        fFullDashDotLen);

                    if(pLineTarget)
                    {
                        pLineTarget->append(aLineTarget);
                    }
                }
            }
        }

        bool isInEpsilonRange(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance)
        {
            for(auto const& rPolygon : rCandidate)
            {
                if(isInEpsilonRange(rPolygon, rTestPosition, fDistance))
                {
                    return true;
                }
            }

            return false;
        }

        B3DPolyPolygon createB3DPolyPolygonFromB2DPolyPolygon(const B2DPolyPolygon& rCandidate, double fZCoordinate)
        {
            B3DPolyPolygon aRetval;

            for(auto const& rPolygon : rCandidate)
            {
                aRetval.append(createB3DPolygonFromB2DPolygon(rPolygon, fZCoordinate));
            }

            return aRetval;
        }

        B2DPolyPolygon createB2DPolyPolygonFromB3DPolyPolygon(const B3DPolyPolygon& rCandidate, const B3DHomMatrix& rMat)
        {
            B2DPolyPolygon aRetval;

            for(auto const& rPolygon : rCandidate)
            {
                aRetval.append(createB2DPolygonFromB3DPolygon(rPolygon, rMat));
            }

            return aRetval;
        }

        double getSmallestDistancePointToPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPoint, sal_uInt32& rPolygonIndex, sal_uInt32& rEdgeIndex, double& rCut)
        {
            double fRetval(DBL_MAX);
            const double fZero(0.0);
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0); a < nPolygonCount; a++)
            {
                const B2DPolygon& aCandidate(rCandidate.getB2DPolygon(a));
                sal_uInt32 nNewEdgeIndex;
                double fNewCut(0.0);
                const double fNewDistance(getSmallestDistancePointToPolygon(aCandidate, rTestPoint, nNewEdgeIndex, fNewCut));

                if(fRetval == DBL_MAX || fNewDistance < fRetval)
                {
                    fRetval = fNewDistance;
                    rPolygonIndex = a;
                    rEdgeIndex = nNewEdgeIndex;
                    rCut = fNewCut;

                    if(fTools::equal(fRetval, fZero))
                    {
                        // already found zero distance, cannot get better. Ensure numerical zero value and end loop.
                        fRetval = 0.0;
                        break;
                    }
                }
            }

            return fRetval;
        }

        B2DPolyPolygon distort(const B2DPolyPolygon& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight)
        {
            B2DPolyPolygon aRetval;

            for(auto const& rPolygon : rCandidate)
            {
                aRetval.append(distort(rPolygon, rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));
            }

            return aRetval;
        }

        B2DPolyPolygon expandToCurve(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval;

            for(auto const& rPolygon : rCandidate)
            {
                aRetval.append(expandToCurve(rPolygon));
            }

            return aRetval;
        }

        B2DPolyPolygon growInNormalDirection(const B2DPolyPolygon& rCandidate, double fValue)
        {
            if(fValue != 0.0)
            {
                B2DPolyPolygon aRetval;

                for(auto const& rPolygon : rCandidate)
                {
                    aRetval.append(growInNormalDirection(rPolygon, fValue));
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon reSegmentPolyPolygon(const B2DPolyPolygon& rCandidate, sal_uInt32 nSegments)
        {
            B2DPolyPolygon aRetval;

            for(auto const& rPolygon : rCandidate)
            {
                aRetval.append(reSegmentPolygon(rPolygon, nSegments));
            }

            return aRetval;
        }

        B2DPolyPolygon interpolate(const B2DPolyPolygon& rOld1, const B2DPolyPolygon& rOld2, double t)
        {
            OSL_ENSURE(rOld1.count() == rOld2.count(), "B2DPolyPolygon interpolate: Different geometry (!)");
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0); a < rOld1.count(); a++)
            {
                aRetval.append(interpolate(rOld1.getB2DPolygon(a), rOld2.getB2DPolygon(a), t));
            }

            return aRetval;
        }

        bool isRectangle( const B2DPolyPolygon& rPoly )
        {
            // exclude some cheap cases first
            if( rPoly.count() != 1 )
                return false;

            return isRectangle( rPoly.getB2DPolygon(0) );
        }

        // #i76891#
        B2DPolyPolygon simplifyCurveSegments(const B2DPolyPolygon& rCandidate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                B2DPolyPolygon aRetval;

                for(auto const& rPolygon : rCandidate)
                {
                    aRetval.append(simplifyCurveSegments(rPolygon));
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon snapPointsOfHorizontalOrVerticalEdges(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval;

            for(auto const& rPolygon : rCandidate)
            {
                aRetval.append(snapPointsOfHorizontalOrVerticalEdges(rPolygon));
            }

            return aRetval;
        }

        B2DPolyPolygon createSevenSegmentPolyPolygon(sal_Char nNumber, bool bLitSegments)
        {
            // config here
            // {
            const double fTotalSize=1.0;
            const double fPosMiddleSegment=0.6;
            const double fSegmentEndChopHoriz=0.08;
            const double fSegmentEndChopVert =0.04;
            // }
            // config here

            const double fLeft=0.0;
            const double fRight=fTotalSize;
            const double fTop=0.0;
            const double fMiddle=fPosMiddleSegment;
            const double fBottom=fTotalSize;

            // from 0 to 5: pair of segment corner coordinates

            // segment corner indices are these:

            //   0 - 1
            //   |   |
            //   2 - 3
            //   |   |
            //   4 - 5

            static const double corners[] =
            {
                fLeft,  fTop,
                fRight, fTop,
                fLeft,  fMiddle,
                fRight, fMiddle,
                fLeft,  fBottom,
                fRight, fBottom
            };

            // from 0 to 9: which segments are 'lit' for this number?

            // array denotes graph edges to traverse, with -1 means
            // stop (the vertices are the corner indices from above):
            //     0
            //     -
            // 1 |   | 2
            //     - 3
            // 4 |   | 5
            //     -
            //     6

            static const int numbers[] =
            {
                1, 1, 1, 0, 1, 1, 1, // 0
                0, 0, 1, 0, 0, 1, 0, // 1
                1, 0, 1, 1, 1, 0, 1, // 2
                1, 0, 1, 1, 0, 1, 1, // 3
                0, 1, 1, 1, 0, 1, 0, // 4
                1, 1, 0, 1, 0, 1, 1, // 5
                1, 1, 0, 1, 1, 1, 1, // 6
                1, 0, 1, 0, 0, 1, 0, // 1
                1, 1, 1, 1, 1, 1, 1, // 8
                1, 1, 1, 1, 0, 1, 1, // 9
                0, 0, 0, 1, 0, 0, 0, // '-'
                1, 1, 0, 1, 1, 0, 1, // 'E'
            };

            // maps segment index to two corner ids:
            static const int index2corner[] =
            {
                0, 2,  // 0
                0, 4,  // 1
                2, 6,  // 2
                4, 6,  // 3
                4, 8,  // 4
                6, 10, // 5
                8, 10, // 6
            };

            B2DPolyPolygon aRes;
            if( nNumber == '-' )
            {
                nNumber = 10;
            }
            else if( nNumber == 'E' )
            {
                nNumber = 11;
            }
            else if( nNumber == '.' )
            {
                if( bLitSegments )
                    aRes.append(createPolygonFromCircle(B2DPoint(fTotalSize/2, fTotalSize),
                                                        fSegmentEndChopHoriz));
                return aRes;
            }
            else
            {
                nNumber=std::clamp<sal_uInt32>(nNumber,'0','9') - '0';
            }

            B2DPolygon aCurrSegment;
            const size_t sliceSize=SAL_N_ELEMENTS(numbers)/12;
            const int* pCurrSegment=numbers + nNumber*sliceSize;
            for( size_t i=0; i<sliceSize; i++, pCurrSegment++)
            {
                if( !(*pCurrSegment ^ int(bLitSegments)) )
                {
                    const size_t j=2*i;
                    aCurrSegment.clear();
                    B2DPoint start(corners[index2corner[j]],
                                   corners[index2corner[j]+1]  );
                    B2DPoint end  (corners[index2corner[j+1]],
                                   corners[index2corner[j+1]+1]);

                    if( rtl::math::approxEqual(start.getX(), end.getX()) )
                    {
                        start.setY(start.getY()+fSegmentEndChopVert);
                        end.setY(end.getY()-fSegmentEndChopVert);
                    }
                    else
                    {
                        start.setX(start.getX()+fSegmentEndChopHoriz);
                        end.setX(end.getX()-fSegmentEndChopHoriz);
                    }

                    aCurrSegment.append(start);
                    aCurrSegment.append(end);
                }
                aRes.append(aCurrSegment);
            }

            return aRes;
        }

        // converters for css::drawing::PointSequence

        B2DPolyPolygon UnoPointSequenceSequenceToB2DPolyPolygon(
            const css::drawing::PointSequenceSequence& rPointSequenceSequenceSource)
        {
            B2DPolyPolygon aRetval;
            const css::drawing::PointSequence* pPointSequence = rPointSequenceSequenceSource.getConstArray();
            const css::drawing::PointSequence* pPointSeqEnd = pPointSequence + rPointSequenceSequenceSource.getLength();

            for(;pPointSequence != pPointSeqEnd; pPointSequence++)
            {
                const B2DPolygon aNewPolygon = UnoPointSequenceToB2DPolygon(*pPointSequence);
                aRetval.append(aNewPolygon);
            }

            return aRetval;
        }

        void B2DPolyPolygonToUnoPointSequenceSequence(
            const B2DPolyPolygon& rPolyPolygon,
            css::drawing::PointSequenceSequence& rPointSequenceSequenceRetval)
        {
            const sal_uInt32 nCount(rPolyPolygon.count());

            if(nCount)
            {
                rPointSequenceSequenceRetval.realloc(nCount);
                css::drawing::PointSequence* pPointSequence = rPointSequenceSequenceRetval.getArray();

                for(auto const& rPolygon : rPolyPolygon)
                {
                    B2DPolygonToUnoPointSequence(rPolygon, *pPointSequence);
                    pPointSequence++;
                }
            }
            else
            {
                rPointSequenceSequenceRetval.realloc(0);
            }
        }

        // converters for css::drawing::PolyPolygonBezierCoords (curved polygons)

        B2DPolyPolygon UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
            const css::drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoordsSource)
        {
            B2DPolyPolygon aRetval;
            const sal_uInt32 nSequenceCount(static_cast<sal_uInt32>(rPolyPolygonBezierCoordsSource.Coordinates.getLength()));

            if(nSequenceCount)
            {
                OSL_ENSURE(nSequenceCount == static_cast<sal_uInt32>(rPolyPolygonBezierCoordsSource.Flags.getLength()),
                    "UnoPolyPolygonBezierCoordsToB2DPolyPolygon: unequal number of Points and Flags (!)");
                const css::drawing::PointSequence* pPointSequence = rPolyPolygonBezierCoordsSource.Coordinates.getConstArray();
                const css::drawing::FlagSequence* pFlagSequence = rPolyPolygonBezierCoordsSource.Flags.getConstArray();

                for(sal_uInt32 a(0); a < nSequenceCount; a++)
                {
                    const B2DPolygon aNewPolygon(UnoPolygonBezierCoordsToB2DPolygon(
                        *pPointSequence,
                        *pFlagSequence));

                    pPointSequence++;
                    pFlagSequence++;
                    aRetval.append(aNewPolygon);
                }
            }

            return aRetval;
        }

        void B2DPolyPolygonToUnoPolyPolygonBezierCoords(
            const B2DPolyPolygon& rPolyPolygon,
            css::drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoordsRetval)
        {
            const sal_uInt32 nCount(rPolyPolygon.count());

            if(nCount)
            {
                // prepare return value memory
                rPolyPolygonBezierCoordsRetval.Coordinates.realloc(static_cast<sal_Int32>(nCount));
                rPolyPolygonBezierCoordsRetval.Flags.realloc(static_cast<sal_Int32>(nCount));

                // get pointers to arrays
                css::drawing::PointSequence* pPointSequence = rPolyPolygonBezierCoordsRetval.Coordinates.getArray();
                css::drawing::FlagSequence*  pFlagSequence = rPolyPolygonBezierCoordsRetval.Flags.getArray();

                for(auto const& rSource : rPolyPolygon)
                {
                    B2DPolygonToUnoPolygonBezierCoords(
                        rSource,
                        *pPointSequence,
                        *pFlagSequence);
                    pPointSequence++;
                    pFlagSequence++;
                }
            }
            else
            {
                rPolyPolygonBezierCoordsRetval.Coordinates.realloc(0);
                rPolyPolygonBezierCoordsRetval.Flags.realloc(0);
            }
        }

    } // end of namespace utils
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
