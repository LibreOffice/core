/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dpolypolygontools.cxx,v $
 * $Revision: 1.17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
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

        B2DPolyPolygon removeIntersections(const B2DPolyPolygon& rCandidate)
        {
            return SolveCrossovers(rCandidate, false);
        }

        B2DPolyPolygon removeAllIntersections(const B2DPolyPolygon& rCandidate)
        {
            return SolveCrossovers(rCandidate);
        }

        B2DPolyPolygon removeNeutralPolygons(const B2DPolyPolygon& rCandidate, bool bUseOr)
        {
            B2DPolyPolygon aRetval(rCandidate);

            aRetval = StripNeutralPolygons(aRetval);
            aRetval = StripDispensablePolygons(aRetval, !bUseOr);

            return aRetval;
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
                    const sal_Bool bInside(isInside(aPolygon, rPoint, bWithBorder));

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

        B2DPolyPolygon applyLineDashing(const B2DPolyPolygon& rCandidate, const ::std::vector<double>& raDashDotArray, double fFullDashDotLen)
        {
            B2DPolyPolygon aRetval;

            if(0.0 == fFullDashDotLen && raDashDotArray.size())
            {
                // calculate fFullDashDotLen from raDashDotArray
                fFullDashDotLen = ::std::accumulate(raDashDotArray.begin(), raDashDotArray.end(), 0.0);
            }

            if(rCandidate.count() && fFullDashDotLen > 0.0)
            {
                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    B2DPolygon aCandidate = rCandidate.getB2DPolygon(a);
                    aRetval.append(applyLineDashing(aCandidate, raDashDotArray, fFullDashDotLen));
                }
            }

            return aRetval;
        }

        B2DPolyPolygon mergeDashedLines(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval;
            const sal_uInt32 nPolygonCount(rCandidate.count());

            if(nPolygonCount)
            {
                B2DPolygon aMergePolygon;

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    if(aMergePolygon.count())
                    {
                        B2DPolygon aNewCandidate = rCandidate.getB2DPolygon(a);

                        if(aNewCandidate.count())
                        {
                            // does aNewCandidate start where aMergePolygon ends?
                            if(aNewCandidate.getB2DPoint(0L) == aMergePolygon.getB2DPoint(aMergePolygon.count() - 1L))
                            {
                                // copy remaining points to aMergePolygon
                                for(sal_uInt32 b(1L); b < aNewCandidate.count(); b++)
                                {
                                    aMergePolygon.append(aNewCandidate.getB2DPoint(b));
                                }
                            }
                            else
                            {
                                // new start point, add aMergePolygon
                                aRetval.append(aMergePolygon);

                                // set aMergePolygon to the new polygon
                                aMergePolygon = aNewCandidate;
                            }
                        }
                    }
                    else
                    {
                        // set aMergePolygon to the new polygon
                        aMergePolygon = rCandidate.getB2DPolygon(a);
                    }
                }

                // append the last used merge polygon
                if(aMergePolygon.count())
                {
                    aRetval.append(aMergePolygon);
                }

                // test if last and first need to be appended, too
                if(aRetval.count() > 1)
                {
                    B2DPolygon aFirst = aRetval.getB2DPolygon(0L);
                    B2DPolygon aLast = aRetval.getB2DPolygon(aRetval.count() - 1L);

                    if(aFirst.getB2DPoint(0L) == aLast.getB2DPoint(aLast.count() - 1L))
                    {
                        // copy remaining points to aLast
                        for(sal_uInt32 a(1L); a < aFirst.count(); a++)
                        {
                            aLast.append(aFirst.getB2DPoint(a));
                        }

                        // create new retval
                        B2DPolyPolygon aNewRetval;

                        // copy the unchanged part polygons
                        for(sal_uInt32 b(1L); b < aRetval.count() - 1L; b++)
                        {
                            aNewRetval.append(aRetval.getB2DPolygon(b));
                        }

                        // append new part polygon
                        aNewRetval.append(aLast);

                        // use as return value
                        aRetval = aNewRetval;
                    }
                }
            }

            return aRetval;
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

        B2DPolyPolygon rotateAroundPoint(const B2DPolyPolygon& rCandidate, const B2DPoint& rCenter, double fAngle)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                aRetval.append(rotateAroundPoint(aCandidate, rCenter, fAngle));
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

        B2DPolyPolygon setContinuity(const B2DPolyPolygon& rCandidate, B2VectorContinuity eContinuity)
        {
            if(rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPolygonCount(rCandidate.count());
                B2DPolyPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                    aRetval.append(setContinuity(aCandidate, eContinuity));
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
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

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
