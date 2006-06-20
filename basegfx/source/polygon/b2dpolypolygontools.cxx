/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygontools.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:44:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#endif

#include <numeric>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // B2DPolyPolygon tools
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
            B2DPolyPolygon aRetval(rCandidate);

            if(aRetval.areControlPointsUsed())
            {
                const sal_uInt32 nPolygonCount(aRetval.count());

                for(sal_uInt32 a(0L); aRetval.areControlPointsUsed() && a < nPolygonCount; a++)
                {
                    B2DPolygon aCandidate = aRetval.getB2DPolygon(a);

                    if(aCandidate.areControlPointsUsed())
                    {
                        aCandidate = tools::adaptiveSubdivideByDistance(aCandidate, fDistanceBound);
                        aRetval.setB2DPolygon(a, aCandidate);
                    }
                }
            }

            return aRetval;
        }

        B2DPolyPolygon adaptiveSubdivideByAngle(const B2DPolyPolygon& rCandidate, double fAngleBound)
        {
            B2DPolyPolygon aRetval(rCandidate);

            if(aRetval.areControlPointsUsed())
            {
                const sal_uInt32 nPolygonCount(aRetval.count());

                for(sal_uInt32 a(0L); aRetval.areControlPointsUsed() && a < nPolygonCount; a++)
                {
                    B2DPolygon aCandidate = aRetval.getB2DPolygon(a);

                    if(aCandidate.areControlPointsUsed())
                    {
                        aCandidate = tools::adaptiveSubdivideByAngle(aCandidate, fAngleBound);
                        aRetval.setB2DPolygon(a, aCandidate);
                    }
                }
            }

            return aRetval;
        }

        // #i37443#
        B2DPolyPolygon adaptiveSubdivideByCount(const B2DPolyPolygon& rCandidate, sal_uInt32 nCount)
        {
            B2DPolyPolygon aRetval(rCandidate);

            if(aRetval.areControlPointsUsed())
            {
                const sal_uInt32 nPolygonCount(aRetval.count());

                for(sal_uInt32 a(0L); aRetval.areControlPointsUsed() && a < nPolygonCount; a++)
                {
                    B2DPolygon aCandidate = aRetval.getB2DPolygon(a);

                    if(aCandidate.areControlPointsUsed())
                    {
                        aCandidate = tools::adaptiveSubdivideByCount(aCandidate, nCount);
                        aRetval.setB2DPolygon(a, aCandidate);
                    }
                }
            }

            return aRetval;
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
            if(rCandidate.areControlVectorsUsed())
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

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof
