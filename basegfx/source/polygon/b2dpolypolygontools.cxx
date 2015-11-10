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
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <numeric>

namespace basegfx
{
    namespace tools
    {
        B2DPolyPolygon correctOrientations(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval(rCandidate);
            const sal_uInt32 nCount(aRetval.count());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                const B2VectorOrientation aOrientation(tools::getOrientation(aCandidate));
                sal_uInt32 nDepth(0L);

                for(sal_uInt32 b(0L); b < nCount; b++)
                {
                    if(b != a)
                    {
                        const B2DPolygon aCompare(rCandidate.getB2DPolygon(b));

                        if(tools::isInside(aCompare, aCandidate, true))
                        {
                            nDepth++;
                        }
                    }
                }

                const bool bShallBeHole(1L == (nDepth & 0x00000001));
                const bool bIsHole(B2VectorOrientation::Negative == aOrientation);

                if(bShallBeHole != bIsHole && B2VectorOrientation::Neutral != aOrientation)
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

            if(nCount > 1L)
            {
                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                    sal_uInt32 nDepth(0L);

                    for(sal_uInt32 b(0L); b < nCount; b++)
                    {
                        if(b != a)
                        {
                            const B2DPolygon aCompare(rCandidate.getB2DPolygon(b));

                            if(tools::isInside(aCompare, aCandidate, true))
                            {
                                nDepth++;
                            }
                        }
                    }

                    if(!nDepth)
                    {
                        B2DPolyPolygon aRetval(rCandidate);

                        if(a != 0L)
                        {
                            // exchange polygon a and polygon 0L
                            aRetval.setB2DPolygon(0L, aCandidate);
                            aRetval.setB2DPolygon(a, rCandidate.getB2DPolygon(0L));
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
                const sal_uInt32 nPolygonCount(rCandidate.count());
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    if(aCandidate.areControlPointsUsed())
                    {
                        aRetval.append(tools::adaptiveSubdivideByDistance(aCandidate, fDistanceBound));
                    }
                    else
                    {
                        aRetval.append(aCandidate);
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
                const sal_uInt32 nPolygonCount(rCandidate.count());
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    if(aCandidate.areControlPointsUsed())
                    {
                        aRetval.append(tools::adaptiveSubdivideByAngle(aCandidate, fAngleBound));
                    }
                    else
                    {
                        aRetval.append(aCandidate);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon adaptiveSubdivideByCount(const B2DPolyPolygon& rCandidate, sal_uInt32 nCount)
        {
            if(rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPolygonCount(rCandidate.count());
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    if(aCandidate.areControlPointsUsed())
                    {
                        aRetval.append(tools::adaptiveSubdivideByCount(aCandidate, nCount));
                    }
                    else
                    {
                        aRetval.append(aCandidate);
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
            const sal_uInt32 nPolygonCount(rCandidate.count());

            if(1L == nPolygonCount)
            {
                return isInside(rCandidate.getB2DPolygon(0L), rPoint, bWithBorder);
            }
            else
            {
                sal_Int32 nInsideCount(0L);

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aPolygon(rCandidate.getB2DPolygon(a));
                    const bool bInside(isInside(aPolygon, rPoint, bWithBorder));

                    if(bInside)
                    {
                        nInsideCount++;
                    }
                }

                return (nInsideCount % 2L);
            }
        }

        B2DRange getRange(const B2DPolyPolygon& rCandidate)
        {
            B2DRange aRetval;
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate = rCandidate.getB2DPolygon(a);
                aRetval.expand(tools::getRange(aCandidate));
            }

            return aRetval;
        }

        double getSignedArea(const B2DPolyPolygon& rCandidate)
        {
            double fRetval(0.0);
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                fRetval += tools::getSignedArea(aCandidate);
            }

            return fRetval;
        }

        double getArea(const B2DPolyPolygon& rCandidate)
        {
            return fabs(getSignedArea(rCandidate));
        }

        void applyLineDashing(const B2DPolyPolygon& rCandidate, const ::std::vector<double>& rDotDashArray, B2DPolyPolygon* pLineTarget, B2DPolyPolygon* pGapTarget, double fFullDashDotLen)
        {
            if(0.0 == fFullDashDotLen && rDotDashArray.size())
            {
                // calculate fFullDashDotLen from rDotDashArray
                fFullDashDotLen = ::std::accumulate(rDotDashArray.begin(), rDotDashArray.end(), 0.0);
            }

            if(rCandidate.count() && fFullDashDotLen > 0.0)
            {
                B2DPolyPolygon aLineTarget, aGapTarget;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    applyLineDashing(
                        aCandidate,
                        rDotDashArray,
                        pLineTarget ? &aLineTarget : nullptr,
                        pGapTarget ? &aGapTarget : nullptr,
                        fFullDashDotLen);

                    if(pLineTarget)
                    {
                        pLineTarget->append(aLineTarget);
                    }

                    if(pGapTarget)
                    {
                        pGapTarget->append(aGapTarget);
                    }
                }
            }
        }

        bool isInEpsilonRange(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                if(isInEpsilonRange(aCandidate, rTestPosition, fDistance))
                {
                    return true;
                }
            }

            return false;
        }

        B3DPolyPolygon createB3DPolyPolygonFromB2DPolyPolygon(const B2DPolyPolygon& rCandidate, double fZCoordinate)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B3DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(createB3DPolygonFromB2DPolygon(aCandidate, fZCoordinate));
            }

            return aRetval;
        }

        B2DPolyPolygon createB2DPolyPolygonFromB3DPolyPolygon(const B3DPolyPolygon& rCandidate, const B3DHomMatrix& rMat)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B3DPolygon aCandidate(rCandidate.getB3DPolygon(a));

                aRetval.append(createB2DPolygonFromB3DPolygon(aCandidate, rMat));
            }

            return aRetval;
        }

        double getSmallestDistancePointToPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPoint& rTestPoint, sal_uInt32& rPolygonIndex, sal_uInt32& rEdgeIndex, double& rCut)
        {
            double fRetval(DBL_MAX);
            const double fZero(0.0);
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                sal_uInt32 nNewEdgeIndex;
                double fNewCut(0.0);
                const double fNewDistance(getSmallestDistancePointToPolygon(aCandidate, rTestPoint, nNewEdgeIndex, fNewCut));

                if(DBL_MAX == fRetval || fNewDistance < fRetval)
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
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(distort(aCandidate, rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));
            }

            return aRetval;
        }

        B2DPolyPolygon expandToCurve(const B2DPolyPolygon& rCandidate)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(expandToCurve(aCandidate));
            }

            return aRetval;
        }

        B2DPolyPolygon growInNormalDirection(const B2DPolyPolygon& rCandidate, double fValue)
        {
            if(0.0 != fValue)
            {
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    aRetval.append(growInNormalDirection(rCandidate.getB2DPolygon(a), fValue));
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        void correctGrowShrinkPolygonPair(SAL_UNUSED_PARAMETER B2DPolyPolygon& /*rOriginal*/, SAL_UNUSED_PARAMETER B2DPolyPolygon& /*rGrown*/)
        {
            //TODO!
        }

        B2DPolyPolygon reSegmentPolyPolygon(const B2DPolyPolygon& rCandidate, sal_uInt32 nSegments)
        {
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(reSegmentPolygon(rCandidate.getB2DPolygon(a), nSegments));
            }

            return aRetval;
        }

        B2DPolyPolygon interpolate(const B2DPolyPolygon& rOld1, const B2DPolyPolygon& rOld2, double t)
        {
            OSL_ENSURE(rOld1.count() == rOld2.count(), "B2DPolyPolygon interpolate: Different geometry (!)");
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rOld1.count(); a++)
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

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    aRetval.append(simplifyCurveSegments(rCandidate.getB2DPolygon(a)));
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

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(snapPointsOfHorizontalOrVerticalEdges(rCandidate.getB2DPolygon(a)));
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
                nNumber=clamp<sal_uInt32>(nNumber,'0','9') - '0';
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

                    if( start.getX() == end.getX() )
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
            const css::drawing::PointSequenceSequence& rPointSequenceSequenceSource,
            bool bCheckClosed)
        {
            B2DPolyPolygon aRetval;
            const css::drawing::PointSequence* pPointSequence = rPointSequenceSequenceSource.getConstArray();
            const css::drawing::PointSequence* pPointSeqEnd = pPointSequence + rPointSequenceSequenceSource.getLength();

            for(;pPointSequence != pPointSeqEnd; pPointSequence++)
            {
                const B2DPolygon aNewPolygon = UnoPointSequenceToB2DPolygon(*pPointSequence, bCheckClosed);
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

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(a));

                    B2DPolygonToUnoPointSequence(aPolygon, *pPointSequence);
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
            const css::drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoordsSource,
            bool bCheckClosed)
        {
            B2DPolyPolygon aRetval;
            const sal_uInt32 nSequenceCount((sal_uInt32)rPolyPolygonBezierCoordsSource.Coordinates.getLength());

            if(nSequenceCount)
            {
                OSL_ENSURE(nSequenceCount == (sal_uInt32)rPolyPolygonBezierCoordsSource.Flags.getLength(),
                    "UnoPolyPolygonBezierCoordsToB2DPolyPolygon: unequal number of Points and Flags (!)");
                const css::drawing::PointSequence* pPointSequence = rPolyPolygonBezierCoordsSource.Coordinates.getConstArray();
                const css::drawing::FlagSequence* pFlagSequence = rPolyPolygonBezierCoordsSource.Flags.getConstArray();

                for(sal_uInt32 a(0); a < nSequenceCount; a++)
                {
                    const B2DPolygon aNewPolygon(UnoPolygonBezierCoordsToB2DPolygon(
                        *pPointSequence,
                        *pFlagSequence,
                        bCheckClosed));

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
                rPolyPolygonBezierCoordsRetval.Coordinates.realloc((sal_Int32)nCount);
                rPolyPolygonBezierCoordsRetval.Flags.realloc((sal_Int32)nCount);

                // get pointers to arrays
                css::drawing::PointSequence* pPointSequence = rPolyPolygonBezierCoordsRetval.Coordinates.getArray();
                css::drawing::FlagSequence*  pFlagSequence = rPolyPolygonBezierCoordsRetval.Flags.getArray();

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const B2DPolygon aSource(rPolyPolygon.getB2DPolygon(a));

                    B2DPolygonToUnoPolygonBezierCoords(
                        aSource,
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

    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
