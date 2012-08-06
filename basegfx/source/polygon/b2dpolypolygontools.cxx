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

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <osl/diagnose.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

#include <numeric>

//////////////////////////////////////////////////////////////////////////////

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
                const bool bIsHole(ORIENTATION_NEGATIVE == aOrientation);

                if(bShallBeHole != bIsHole && ORIENTATION_NEUTRAL != aOrientation)
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
                        pLineTarget ? &aLineTarget : 0,
                        pGapTarget ? &aGapTarget : 0,
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
                double fNewCut;
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

    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
