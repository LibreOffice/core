/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygontools.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:43:59 $
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

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_CURVE_B2DCUBICBEZIER_HXX
#include <basegfx/curve/b2dcubicbezier.hxx>
#endif

#ifndef _BGFX_CURVE_B2DBEZIERTOOLS_HXX
#include <basegfx/curve/b2dbeziertools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#include <numeric>
#include <limits>

// #i37443#
#define ANGLE_BOUND_START_VALUE     (2.25)
#define ANGLE_BOUND_MINIMUM_VALUE   (0.1)
#define COUNT_SUBDIVIDE_DEFAULT     (4L)
#ifdef DBG_UTIL
static double fAngleBoundStartValue = ANGLE_BOUND_START_VALUE;
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // B2DPolygon tools
        void checkClosed(B2DPolygon& rCandidate)
        {
            while(rCandidate.count() > 1L
                && rCandidate.getB2DPoint(0L).equal(rCandidate.getB2DPoint(rCandidate.count() - 1L)))
            {
                rCandidate.setClosed(true);
                rCandidate.remove(rCandidate.count() - 1L);
            }
        }

        // Get successor and predecessor indices. Returning the same index means there
        // is none. Same for successor.
        sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(nIndex)
            {
                return nIndex - 1L;
            }
            else if(rCandidate.count())
            {
                return rCandidate.count() - 1L;
            }
            else
            {
                return nIndex;
            }
        }

        sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(nIndex + 1L < rCandidate.count())
            {
                return nIndex + 1L;
            }
            else if(nIndex + 1L == rCandidate.count())
            {
                return 0L;
            }
            else
            {
                return nIndex;
            }
        }

        B2VectorOrientation getOrientation(const B2DPolygon& rCandidate)
        {
            B2VectorOrientation eRetval(ORIENTATION_NEUTRAL);

            if(rCandidate.count() > 2L || rCandidate.areControlVectorsUsed())
            {
                const double fSignedArea(getSignedArea(rCandidate));

                if(fSignedArea > 0.0)
                {
                    eRetval = ORIENTATION_POSITIVE;
                }
                else if(fSignedArea < 0.0)
                {
                    eRetval = ORIENTATION_NEGATIVE;
                }
            }

            return eRetval;
        }

        B2VectorContinuity getContinuityInPoint(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");
            B2VectorContinuity eRetval(CONTINUITY_NONE);

            if(rCandidate.count() > 1L && rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPrevInd(getIndexOfPredecessor(nIndex, rCandidate));
                const B2DVector aForwardVector(rCandidate.getControlVectorA(nIndex));
                const B2DVector aBackVector(rCandidate.getControlPointB(nPrevInd) - rCandidate.getB2DPoint(nIndex));

                eRetval = getContinuity(aBackVector, aForwardVector);
            }

            return eRetval;
        }

        B2DPolyPolygon removeIntersections(const B2DPolygon& rCandidate, bool bKeepOrientations)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count() > 2L)
            {
                aRetval = SolveCrossovers(rCandidate);

                if(bKeepOrientations && aRetval.count() > 1L)
                {
                    // there were cuts done, correct orientations
                    const B2VectorOrientation aOriginalOrientation(getOrientation(rCandidate));
                    B2DPolyPolygon aNewRetval;

                    for(sal_uInt32 a(0L); a < aRetval.count(); a++)
                    {
                        B2DPolygon aCandidate = aRetval.getB2DPolygon(a);
                        const B2VectorOrientation aOrientation(getOrientation(aCandidate));

                        if(aOriginalOrientation != aOrientation
                            && ORIENTATION_NEUTRAL != aOrientation)
                        {
                            aCandidate.flip();
                        }

                        aNewRetval.append(aCandidate);
                    }

                    aRetval = aNewRetval;
                }
            }
            else
            {
                aRetval.append(rCandidate);
            }

            return aRetval;
        }

        B2DPolygon adaptiveSubdivideByDistance(const B2DPolygon& rCandidate, double fDistanceBound)
        {
            B2DPolygon aRetval(rCandidate);

            if(aRetval.areControlPointsUsed())
            {
                const sal_uInt32 nPointCount(rCandidate.isClosed() ? rCandidate.count() : rCandidate.count() - 1L);
                aRetval.clear();

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DVector aVectorA(rCandidate.getControlVectorA(a));
                    const B2DVector aVectorB(rCandidate.getControlVectorB(a));

                    if(!aVectorA.equalZero() || !aVectorB.equalZero())
                    {
                        // vectors are used, get points
                        const sal_uInt32 nNext(getIndexOfSuccessor(a, rCandidate));
                        B2DPoint aPointA(rCandidate.getB2DPoint(a));
                        B2DPoint aPointB(rCandidate.getB2DPoint(nNext));

                        // build CubicBezier segment
                        B2DCubicBezier aBezier(
                            aPointA, B2DPoint(aPointA + aVectorA), B2DPoint(aPointA + aVectorB), aPointB);

                        // generate DistanceBound
                        double fBound;

                        if(0.0 == fDistanceBound)
                        {
                            // If not set, use B2DCubicBezier functionality to guess a rough
                            // value
                            const double fRoughLength((aBezier.getEdgeLength() + aBezier.getControlPolygonLength()) / 2.0);

                            // take 1/100th of the rouch curve length
                            fBound = fRoughLength * 0.01;
                        }
                        else
                        {
                            // use given bound value
                            fBound = fDistanceBound;
                        }

                        // make sure bound value is not too small. The base units are 1/100th mm, thus
                        // just make sure it's not smaller then 1/100th of that
                        if(fBound < 0.01)
                        {
                            fBound = 0.01;
                        }

                        // call adaptive subdivide, do not add last point
                        adaptiveSubdivideByDistance(aRetval, aBezier, fBound, false);
                    }
                    else
                    {
                        // no vectors used, add point
                        aRetval.append(rCandidate.getB2DPoint(a));
                    }
                }

                // add last point if not closed. If the last point had vectors,
                // they are lost since they make no sense.
                if(!rCandidate.isClosed())
                {
                    aRetval.append(rCandidate.getB2DPoint(rCandidate.count() - 1));
                }

                // check closed flag, aRetval was cleared and thus it may be invalid.
                if(aRetval.isClosed() != rCandidate.isClosed())
                {
                    aRetval.setClosed(rCandidate.isClosed());
                }
            }

            return aRetval;
        }

        B2DPolygon adaptiveSubdivideByAngle(const B2DPolygon& rCandidate, double fAngleBound)
        {
            B2DPolygon aRetval(rCandidate);

            if(aRetval.areControlPointsUsed())
            {
                const sal_uInt32 nPointCount(rCandidate.isClosed() ? rCandidate.count() : rCandidate.count() - 1L);
                aRetval.clear();

                // #i37443# prepare convenient AngleBound if none was given
                if(0.0 == fAngleBound)
                {
#ifdef DBG_UTIL
                    fAngleBound = fAngleBoundStartValue;
#else
                    fAngleBound = ANGLE_BOUND_START_VALUE;
#endif
                }
                else if(fTools::less(fAngleBound, ANGLE_BOUND_MINIMUM_VALUE))
                {
                    fAngleBound = 0.1;
                }

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DVector aVectorA(rCandidate.getControlVectorA(a));
                    const B2DVector aVectorB(rCandidate.getControlVectorB(a));

                    if(!aVectorA.equalZero() || !aVectorB.equalZero())
                    {
                        // vectors are used, get points
                        const sal_uInt32 nNext(getIndexOfSuccessor(a, rCandidate));
                        B2DPoint aPointA(rCandidate.getB2DPoint(a));
                        B2DPoint aPointB(rCandidate.getB2DPoint(nNext));

                        // build CubicBezier segment
                        B2DCubicBezier aBezier(
                            aPointA, B2DPoint(aPointA + aVectorA), B2DPoint(aPointA + aVectorB), aPointB);

                        // call adaptive subdivide, do not add last point
                        // #i37443# allow unsharpen the criteria
                        aBezier.adaptiveSubdivideByAngle(aRetval, fAngleBound, false, true);
                    }
                    else
                    {
                        // no vectors used, add point
                        aRetval.append(rCandidate.getB2DPoint(a));
                    }
                }

                // add last point if not closed. If the last point had vectors,
                // they are lost since they make no sense.
                if(!rCandidate.isClosed())
                {
                    aRetval.append(rCandidate.getB2DPoint(rCandidate.count() - 1));
                }

                // check closed flag, aRetval was cleared and thus it may be invalid.
                if(aRetval.isClosed() != rCandidate.isClosed())
                {
                    aRetval.setClosed(rCandidate.isClosed());
                }
            }

            return aRetval;
        }

        B2DPolygon adaptiveSubdivideByCount(const B2DPolygon& rCandidate, sal_uInt32 nCount)
        {
            B2DPolygon aRetval(rCandidate);

            if(aRetval.areControlPointsUsed())
            {
                const sal_uInt32 nPointCount(rCandidate.isClosed() ? rCandidate.count() : rCandidate.count() - 1L);
                aRetval.clear();

                // #i37443# prepare convenient count if none was given
                if(0L == nCount)
                {
                    nCount = COUNT_SUBDIVIDE_DEFAULT;
                }

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DVector aVectorA(rCandidate.getControlVectorA(a));
                    const B2DVector aVectorB(rCandidate.getControlVectorB(a));

                    if(!aVectorA.equalZero() || !aVectorB.equalZero())
                    {
                        // vectors are used, get points
                        const sal_uInt32 nNext(getIndexOfSuccessor(a, rCandidate));
                        B2DPoint aPointA(rCandidate.getB2DPoint(a));
                        B2DPoint aPointB(rCandidate.getB2DPoint(nNext));

                        // build CubicBezier segment
                        B2DCubicBezier aBezier(
                            aPointA, B2DPoint(aPointA + aVectorA), B2DPoint(aPointA + aVectorB), aPointB);

                        // call adaptive subdivide, do not add last point
                        aBezier.adaptiveSubdivideByCount(aRetval, nCount, false);
                    }
                    else
                    {
                        // no vectors used, add point
                        aRetval.append(rCandidate.getB2DPoint(a));
                    }
                }

                // add last point if not closed. If the last point had vectors,
                // they are lost since they make no sense.
                if(!rCandidate.isClosed())
                {
                    aRetval.append(rCandidate.getB2DPoint(rCandidate.count() - 1));
                }

                // check closed flag, aRetval was cleared and thus it may be invalid.
                if(aRetval.isClosed() != rCandidate.isClosed())
                {
                    aRetval.setClosed(rCandidate.isClosed());
                }
            }

            return aRetval;
        }

        bool isInside(const B2DPolygon& rCandidate, const B2DPoint& rPoint, bool bWithBorder)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? adaptiveSubdivideByCount(rCandidate, 6L) : rCandidate);
            bool bRetval(false);
            const sal_uInt32 nPointCount(aCandidate.count());

            for(sal_uInt32 a(0L); a < nPointCount; a++)
            {
                const B2DPoint aCurrentPoint(aCandidate.getB2DPoint(a));

                if(bWithBorder && aCurrentPoint.equal(rPoint))
                {
                    return true;
                }

                // cross-over in Y?
                const B2DPoint aPreviousPoint(aCandidate.getB2DPoint((!a) ? nPointCount - 1L : a - 1L));
                const bool bCompYA(fTools::more(aPreviousPoint.getY(), rPoint.getY()));
                const bool bCompYB(fTools::more(aCurrentPoint.getY(), rPoint.getY()));

                if(bCompYA != bCompYB)
                {
                    const bool bCompXA(fTools::more(aPreviousPoint.getX(), rPoint.getX()));
                    const bool bCompXB(fTools::more(aCurrentPoint.getX(), rPoint.getX()));

                    if(bCompXA == bCompXB)
                    {
                        if(bCompXA)
                        {
                            bRetval = !bRetval;
                        }
                        else
                        {
                            // it may still be a touch with a vertical line when bWithBorder==true,
                            // check for it. If it is, return true
                            if(bWithBorder
                                && fTools::equal(aPreviousPoint.getX(), rPoint.getX())
                                && fTools::equal(aCurrentPoint.getX(), rPoint.getX()))
                            {
                                return true;
                            }
                        }
                    }
                    else
                    {
                        const double fCompare =
                            aCurrentPoint.getX() - (aCurrentPoint.getY() - rPoint.getY()) *
                            (aPreviousPoint.getX() - aCurrentPoint.getX()) /
                            (aPreviousPoint.getY() - aCurrentPoint.getY());

                        if(bWithBorder && fTools::equal(fCompare, rPoint.getX()))
                        {
                            // point on line, when bWithBorder=true, all is done
                            return true;
                        }
                        else if(fTools::more(fCompare, rPoint.getX()))
                        {
                            bRetval = !bRetval;
                        }
                    }
                }
            }

            return bRetval;
        }

        bool isInside(const B2DPolygon& rCandidate, const B2DPolygon& rPolygon, bool bWithBorder)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? adaptiveSubdivideByCount(rCandidate, 6L) : rCandidate);
            const B2DPolygon aPolygon(rPolygon.areControlPointsUsed() ? adaptiveSubdivideByCount(rPolygon, 6L) : rPolygon);
            const sal_uInt32 nPointCount(aPolygon.count());

            for(sal_uInt32 a(0L); a < nPointCount; a++)
            {
                const B2DPoint aTestPoint(aPolygon.getB2DPoint(a));

                if(!isInside(aCandidate, aTestPoint, bWithBorder))
                {
                    return false;
                }
            }

            return true;
        }

        B2DRange getRange(const B2DPolygon& rCandidate)
        {
            B2DRange aRetval;
            const sal_uInt32 nPointCount(rCandidate.count());

            if(rCandidate.areControlPointsUsed())
            {
                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aTestPoint(rCandidate.getB2DPoint(a));
                    const B2DVector aVectorA(rCandidate.getControlVectorA(a));
                    const B2DVector aVectorB(rCandidate.getControlVectorB(a));
                    aRetval.expand(aTestPoint);

                    if(!aVectorA.equalZero())
                    {
                        aRetval.expand(aTestPoint + aVectorA);
                    }

                    if(!aVectorB.equalZero())
                    {
                        aRetval.expand(aTestPoint + aVectorB);
                    }
                }
            }
            else
            {
                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aTestPoint(rCandidate.getB2DPoint(a));
                    aRetval.expand(aTestPoint);
                }
            }

            return aRetval;
        }

        double getSignedArea(const B2DPolygon& rCandidate)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? adaptiveSubdivideByCount(rCandidate, 6L) : rCandidate);
            double fRetval(0.0);
            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount > 2)
            {
                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aPreviousPoint(aCandidate.getB2DPoint((!a) ? nPointCount - 1L : a - 1L));
                    const B2DPoint aCurrentPoint(aCandidate.getB2DPoint(a));

                    fRetval += aPreviousPoint.getX() * aCurrentPoint.getY();
                    fRetval -= aPreviousPoint.getY() * aCurrentPoint.getX();
                }

                fRetval /= 2.0;
            }

            return fRetval;
        }

        double getArea(const B2DPolygon& rCandidate)
        {
            double fRetval(0.0);

            if(rCandidate.count() > 2 || rCandidate.areControlPointsUsed())
            {
                fRetval = getSignedArea(rCandidate);
                const double fZero(0.0);

                if(fTools::less(fRetval, fZero))
                {
                    fRetval = -fRetval;
                }
            }

            return fRetval;
        }

        double getEdgeLength(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "getEdgeLength: ATM works not for curves (!)");
            OSL_ENSURE(nIndex < rCandidate.count(), "getEdgeLength: Access to polygon out of range (!)");
            double fRetval(0.0);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nIndex < nPointCount)
            {
                if(rCandidate.isClosed() || ((nIndex + 1L) != nPointCount))
                {
                    const sal_uInt32 nNextIndex(getIndexOfSuccessor(nIndex, rCandidate));
                    const B2DPoint aCurrentPoint(rCandidate.getB2DPoint(nIndex));
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                    const B2DVector aVector(aNextPoint - aCurrentPoint);
                    fRetval = aVector.getLength();
                }
            }

            return fRetval;
        }

        double getLength(const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "getLength: ATM works not for curves (!)");
            double fRetval(0.0);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1L)
            {
                const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);

                for(sal_uInt32 a(0L); a < nLoopCount; a++)
                {
                    const sal_uInt32 nNextIndex(getIndexOfSuccessor(a, rCandidate));
                    const B2DPoint aCurrentPoint(rCandidate.getB2DPoint(a));
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                    const B2DVector aVector(aNextPoint - aCurrentPoint);
                    fRetval += aVector.getLength();
                }
            }

            return fRetval;
        }

        B2DPoint getPositionAbsolute(const B2DPolygon& rCandidate, double fDistance, double fLength)
        {
            B2DPoint aRetval;
            const sal_uInt32 nPointCount(rCandidate.count());
            const sal_uInt32 nPointCountMinusOne(nPointCount - 1L);

            if( 1L == nPointCount )
            {
                // only one point (i.e. no edge) - simply take that point
                aRetval = rCandidate.getB2DPoint(0);
            }
            else if(nPointCount > 1L)
            {
                sal_uInt32 nIndex(0L);
                bool bIndexDone(false);
                const double fZero(0.0);
                double fEdgeLength(fZero);

                // get length if not given
                if(fTools::equalZero(fLength))
                {
                    fLength = getLength(rCandidate);
                }

                // handle fDistance < 0.0
                if(fTools::less(fDistance, fZero))
                {
                    if(rCandidate.isClosed())
                    {
                        // if fDistance < 0.0 increment with multiple of fLength
                        sal_uInt32 nCount(sal_uInt32(-fDistance / fLength));
                        fDistance += double(nCount + 1L) * fLength;
                    }
                    else
                    {
                        // crop to polygon start
                        fDistance = fZero;
                        bIndexDone = true;
                    }
                }

                // handle fDistance >= fLength
                if(fTools::moreOrEqual(fDistance, fLength))
                {
                    if(rCandidate.isClosed())
                    {
                        // if fDistance >= fLength decrement with multiple of fLength
                        sal_uInt32 nCount(sal_uInt32(fDistance / fLength));
                        fDistance -= (double)(nCount) * fLength;
                    }
                    else
                    {
                        // crop to polygon end
                        fDistance = fZero;
                        nIndex = nPointCountMinusOne;
                        bIndexDone = true;
                    }
                }

                // look for correct index. fDistance is now [0.0 .. fLength[
                if(!bIndexDone)
                {
                    do
                    {
                        // get length of next edge
                        fEdgeLength = getEdgeLength(rCandidate, nIndex);

                        // edge found must be on the half-open range
                        // [0,fEdgeLength).
                        // Note that in theory, we cannot move beyond
                        // the last polygon point, since fDistance>=fLength
                        // is checked above. Unfortunately, with floating-
                        // point calculations, this case might happen.
                        // Handled by nIndex check below
                        if( nIndex < nPointCountMinusOne &&
                            fDistance >= fEdgeLength )
                        {
                            // go to next edge
                            fDistance -= fEdgeLength;
                            nIndex++;
                        }
                        else
                        {
                            // it's on this edge, stop
                            bIndexDone = true;
                        }
                    } while (!bIndexDone);
                }

                // get the point using nIndex
                aRetval = rCandidate.getB2DPoint(nIndex);

                // if fDistance != 0.0, move that length on the edge. The edge
                // length is in fEdgeLength.
                if(!fTools::equalZero(fDistance))
                {
                    sal_uInt32 nNextIndex(getIndexOfSuccessor(nIndex, rCandidate));
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                    double fRelative(fZero);

                    if(!fTools::equalZero(fEdgeLength))
                    {
                        // clamp fRelative to [0,1] range. Borderline cases
                        // can happen, since this is floating point arithmetic.
                        fRelative = ::std::max(0.0,
                                               ::std::min(1.0,
                                                          fDistance / fEdgeLength) );
                    }

                    // add calculated average value to the return value
                    aRetval = interpolate(aRetval, aNextPoint, fRelative);
                }
            }

            return aRetval;
        }

        B2DPoint getPositionRelative(const B2DPolygon& rCandidate, double fDistance, double fLength)
        {
            // get length if not given
            if(fTools::equalZero(fLength))
            {
                fLength = getLength(rCandidate);
            }

            // multiply fDistance with real length to get absolute position and
            // use getPositionAbsolute
            return getPositionAbsolute(rCandidate, fDistance * fLength, fLength);
        }

        B2DPolygon getSnippetAbsolute(const B2DPolygon& rCandidate, double fFrom, double fTo, double fLength)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "getSnippetAbsolute: ATM works not for curves (!)");
            B2DPolygon aRetval;
            const sal_uInt32 nPointCount(rCandidate.count());

            // get length if not given
            if(fTools::equalZero(fLength))
            {
                fLength = getLength(rCandidate);
            }

            // test and correct fFrom
            if(fFrom < 0.0)
            {
                fFrom = 0.0;
            }

            // test and correct fTo
            if(fTo > fLength)
            {
                fTo = fLength;
            }

            // test and correct relationship of fFrom, fTo
            if(fFrom > fTo)
            {
                fFrom = fTo = (fFrom + fTo) / 2.0;
            }

            if(0.0 == fFrom && fTo == fLength)
            {
                // result is the whole polygon
                aRetval = rCandidate;
            }
            else
            {
                double fPositionOfStart(0.0);
                bool bStartDone(false);
                bool bEndDone(false);

                for(sal_uInt32 a(0L); !(bStartDone && bEndDone) && a < nPointCount; a++)
                {
                    const sal_uInt32 nNextIndex(getIndexOfSuccessor(a, rCandidate));
                    const B2DPoint aStart(rCandidate.getB2DPoint(a));
                    const B2DPoint aEnd(rCandidate.getB2DPoint(nNextIndex));
                    const B2DVector aEdgeVector(aEnd - aStart);
                    const double fEdgeLength(aEdgeVector.getLength());

                    if(!bStartDone)
                    {
                        if(0.0 == fFrom)
                        {
                            aRetval.append(aStart);
                            bStartDone = true;
                        }
                        else if(fFrom >= fPositionOfStart && fFrom < fPositionOfStart + fEdgeLength)
                        {
                            // calculate and add start point
                            if(0.0 != fEdgeLength)
                            {
                                aRetval.append(interpolate(aStart, aEnd, (fFrom - fPositionOfStart) / fEdgeLength));
                            }
                            else
                            {
                                aRetval.append(aStart);
                            }

                            bStartDone = true;

                            // if same point, end is done, too.
                            if(fFrom == fTo)
                            {
                                bEndDone = true;
                            }
                        }
                    }

                    if(!bEndDone && fTo >= fPositionOfStart && fTo < fPositionOfStart + fEdgeLength)
                    {
                        // calculate and add end point
                        if(0.0 != fEdgeLength)
                        {
                            aRetval.append(interpolate(aStart, aEnd, (fTo - fPositionOfStart) / fEdgeLength));
                        }
                        else
                        {
                            aRetval.append(aEnd);
                        }

                        bEndDone = true;
                    }

                    if(!bEndDone)
                    {
                        if(bStartDone)
                        {
                            // add segments end point
                            aRetval.append(aEnd);
                        }

                        // increment fPositionOfStart
                        fPositionOfStart += fEdgeLength;
                    }
                }
            }

            return aRetval;
        }

        B2DPolygon getSnippetRelative(const B2DPolygon& rCandidate, double fFrom, double fTo, double fLength)
        {
            // get length if not given
            if(fTools::equalZero(fLength))
            {
                fLength = getLength(rCandidate);
            }

            // multiply distances with real length to get absolute position and
            // use getSnippetAbsolute
            return getSnippetAbsolute(rCandidate, fFrom * fLength, fTo * fLength, fLength);
        }

        CutFlagValue findCut(
            const B2DPolygon& rCandidate,
            sal_uInt32 nIndex1, sal_uInt32 nIndex2,
            CutFlagValue aCutFlags,
            double* pCut1, double* pCut2)
        {
            CutFlagValue aRetval(CUTFLAG_NONE);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nIndex1 < nPointCount && nIndex2 < nPointCount && nIndex1 != nIndex2)
            {
                sal_uInt32 nEnd1(getIndexOfSuccessor(nIndex1, rCandidate));
                sal_uInt32 nEnd2(getIndexOfSuccessor(nIndex2, rCandidate));

                const B2DPoint aStart1(rCandidate.getB2DPoint(nIndex1));
                const B2DPoint aEnd1(rCandidate.getB2DPoint(nEnd1));
                const B2DVector aVector1(aEnd1 - aStart1);

                const B2DPoint aStart2(rCandidate.getB2DPoint(nIndex2));
                const B2DPoint aEnd2(rCandidate.getB2DPoint(nEnd2));
                const B2DVector aVector2(aEnd2 - aStart2);

                aRetval = findCut(
                    aStart1, aVector1, aStart2, aVector2,
                    aCutFlags, pCut1, pCut2);
            }

            return aRetval;
        }

        CutFlagValue findCut(
            const B2DPolygon& rCandidate1, sal_uInt32 nIndex1,
            const B2DPolygon& rCandidate2, sal_uInt32 nIndex2,
            CutFlagValue aCutFlags,
            double* pCut1, double* pCut2)
        {
            CutFlagValue aRetval(CUTFLAG_NONE);
            const sal_uInt32 nPointCount1(rCandidate1.count());
            const sal_uInt32 nPointCount2(rCandidate2.count());

            if(nIndex1 < nPointCount1 && nIndex2 < nPointCount2)
            {
                sal_uInt32 nEnd1(getIndexOfSuccessor(nIndex1, rCandidate1));
                sal_uInt32 nEnd2(getIndexOfSuccessor(nIndex2, rCandidate2));

                const B2DPoint aStart1(rCandidate1.getB2DPoint(nIndex1));
                const B2DPoint aEnd1(rCandidate1.getB2DPoint(nEnd1));
                const B2DVector aVector1(aEnd1 - aStart1);

                const B2DPoint aStart2(rCandidate2.getB2DPoint(nIndex2));
                const B2DPoint aEnd2(rCandidate2.getB2DPoint(nEnd2));
                const B2DVector aVector2(aEnd2 - aStart2);

                aRetval = findCut(
                    aStart1, aVector1, aStart2, aVector2,
                    aCutFlags, pCut1, pCut2);
            }

            return aRetval;
        }

        CutFlagValue findCut(
            const B2DPoint& rEdge1Start, const B2DVector& rEdge1Delta,
            const B2DPoint& rEdge2Start, const B2DVector& rEdge2Delta,
            CutFlagValue aCutFlags,
            double* pCut1, double* pCut2)
        {
            CutFlagValue aRetval(CUTFLAG_NONE);
            double fCut1(0.0);
            double fCut2(0.0);
            bool bFinished(!((bool)(aCutFlags & CUTFLAG_ALL)));

            // test for same points?
            if(!bFinished
                && (aCutFlags & (CUTFLAG_START1|CUTFLAG_END1))
                && (aCutFlags & (CUTFLAG_START2|CUTFLAG_END2)))
            {
                // same startpoint?
                if(!bFinished && (aCutFlags & (CUTFLAG_START1|CUTFLAG_START2)) == (CUTFLAG_START1|CUTFLAG_START2))
                {
                    if(rEdge1Start.equal(rEdge2Start))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_START1|CUTFLAG_START2);
                    }
                }

                // same endpoint?
                if(!bFinished && (aCutFlags & (CUTFLAG_END1|CUTFLAG_END2)) == (CUTFLAG_END1|CUTFLAG_END2))
                {
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(aEnd1.equal(aEnd2))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_END1|CUTFLAG_END2);
                        fCut1 = fCut2 = 1.0;
                    }
                }

                // startpoint1 == endpoint2?
                if(!bFinished && (aCutFlags & (CUTFLAG_START1|CUTFLAG_END2)) == (CUTFLAG_START1|CUTFLAG_END2))
                {
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(rEdge1Start.equal(aEnd2))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_START1|CUTFLAG_END2);
                        fCut1 = 0.0;
                        fCut2 = 1.0;
                    }
                }

                // startpoint2 == endpoint1?
                if(!bFinished&& (aCutFlags & (CUTFLAG_START2|CUTFLAG_END1)) == (CUTFLAG_START2|CUTFLAG_END1))
                {
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);

                    if(rEdge2Start.equal(aEnd1))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_START2|CUTFLAG_END1);
                        fCut1 = 1.0;
                        fCut2 = 0.0;
                    }
                }
            }

            if(!bFinished && (aCutFlags & CUTFLAG_LINE))
            {
                if(!bFinished && (aCutFlags & CUTFLAG_START1))
                {
                    // start1 on line 2 ?
                    if(isPointOnEdge(rEdge1Start, rEdge2Start, rEdge2Delta, &fCut2))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_LINE|CUTFLAG_START1);
                    }
                }

                if(!bFinished && (aCutFlags & CUTFLAG_START2))
                {
                    // start2 on line 1 ?
                    if(isPointOnEdge(rEdge2Start, rEdge1Start, rEdge1Delta, &fCut1))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_LINE|CUTFLAG_START2);
                    }
                }

                if(!bFinished && (aCutFlags & CUTFLAG_END1))
                {
                    // end1 on line 2 ?
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);

                    if(isPointOnEdge(aEnd1, rEdge2Start, rEdge2Delta, &fCut2))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_LINE|CUTFLAG_END1);
                    }
                }

                if(!bFinished && (aCutFlags & CUTFLAG_END2))
                {
                    // end2 on line 1 ?
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(isPointOnEdge(aEnd2, rEdge1Start, rEdge1Delta, &fCut1))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_LINE|CUTFLAG_END2);
                    }
                }

                if(!bFinished)
                {
                    // cut in line1, line2 ?
                    fCut1 = (rEdge1Delta.getX() * rEdge2Delta.getY()) - (rEdge1Delta.getY() * rEdge2Delta.getX());

                    if(!fTools::equalZero(fCut1))
                    {
                        fCut1 = (rEdge2Delta.getY() * (rEdge2Start.getX() - rEdge1Start.getX())
                            + rEdge2Delta.getX() * (rEdge1Start.getY() - rEdge2Start.getY())) / fCut1;

                        const double fZero(0.0);
                        const double fOne(1.0);

                        // inside parameter range edge1 AND fCut2 is calcable
                        if(fTools::more(fCut1, fZero) && fTools::less(fCut1, fOne)
                            && (!fTools::equalZero(rEdge2Delta.getX()) || !fTools::equalZero(rEdge2Delta.getY())))
                        {
                            // take the mopre precise calculation of the two possible
                            if(fabs(rEdge2Delta.getX()) > fabs(rEdge2Delta.getY()))
                            {
                                fCut2 = (rEdge1Start.getX() + fCut1
                                    * rEdge1Delta.getX() - rEdge2Start.getX()) / rEdge2Delta.getX();
                            }
                            else
                            {
                                fCut2 = (rEdge1Start.getY() + fCut1
                                    * rEdge1Delta.getY() - rEdge2Start.getY()) / rEdge2Delta.getY();
                            }

                            // inside parameter range edge2, too
                            if(fTools::more(fCut2, fZero) && fTools::less(fCut2, fOne))
                            {
                                bFinished = true;
                                aRetval = CUTFLAG_LINE;
                            }
                        }
                    }
                }
            }

            // copy values if wanted
            if(pCut1)
            {
                *pCut1 = fCut1;
            }

            if(pCut2)
            {
                *pCut2 = fCut2;
            }

            return aRetval;
        }

        bool isPointOnEdge(
            const B2DPoint& rPoint,
            const B2DPoint& rEdgeStart,
            const B2DVector& rEdgeDelta,
            double* pCut)
        {
            bool bDeltaXIsZero(fTools::equalZero(rEdgeDelta.getX()));
            bool bDeltaYIsZero(fTools::equalZero(rEdgeDelta.getY()));
            const double fZero(0.0);
            const double fOne(1.0);

            if(bDeltaXIsZero && bDeltaYIsZero)
            {
                // no line, just a point
                return false;
            }
            else if(bDeltaXIsZero)
            {
                // vertical line
                if(fTools::equal(rPoint.getX(), rEdgeStart.getX()))
                {
                    double fValue = (rPoint.getY() - rEdgeStart.getY()) / rEdgeDelta.getY();

                    if(fTools::more(fValue, fZero) && fTools::less(fValue, fOne))
                    {
                        if(pCut)
                        {
                            *pCut = fValue;
                        }

                        return true;
                    }
                }
            }
            else if(bDeltaYIsZero)
            {
                // horizontal line
                if(fTools::equal(rPoint.getY(), rEdgeStart.getY()))
                {
                    double fValue = (rPoint.getX() - rEdgeStart.getX()) / rEdgeDelta.getX();

                    if(fTools::more(fValue, fZero)
                        && fTools::less(fValue, fOne))
                    {
                        if(pCut)
                        {
                            *pCut = fValue;
                        }

                        return true;
                    }
                }
            }
            else
            {
                // any angle line
                double fTOne = (rPoint.getX() - rEdgeStart.getX()) / rEdgeDelta.getX();
                double fTTwo = (rPoint.getY() - rEdgeStart.getY()) / rEdgeDelta.getY();

                if(fTools::equal(fTOne, fTTwo))
                {
                    // same parameter representation, point is on line. Take
                    // middle value for better results
                    double fValue = (fTOne + fTTwo) / 2.0;

                    if(fTools::more(fValue, fZero) && fTools::less(fValue, fOne))
                    {
                        // point is inside line bounds, too
                        if(pCut)
                        {
                            *pCut = fValue;
                        }

                        return true;
                    }
                }
            }

            return false;
        }

        B2DPolyPolygon applyLineDashing(const B2DPolygon& rCandidate, const ::std::vector<double>& raDashDotArray, double fFullDashDotLen)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "applyLineDashing: ATM works not for curves (!)");
            B2DPolyPolygon aRetval;

            if(0.0 == fFullDashDotLen && rCandidate.count() && raDashDotArray.size())
            {
                // calculate fFullDashDotLen from raDashDotArray
                fFullDashDotLen = ::std::accumulate(raDashDotArray.begin(), raDashDotArray.end(), 0.0);
            }

            if(rCandidate.count() && fFullDashDotLen > 0.0)
            {
                // prepare candidate, evtl. remove curves
                B2DPolygon aCandidate(rCandidate);

                if(aCandidate.areControlVectorsUsed())
                {
                    aCandidate = adaptiveSubdivideByAngle(aCandidate);
                }

                const sal_uInt32 nCount(aCandidate.isClosed() ? aCandidate.count() : aCandidate.count() - 1L);
                sal_uInt32 nDashDotIndex(0L);
                double fDashDotLength(raDashDotArray[nDashDotIndex]);

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    const sal_uInt32 nNextIndex(getIndexOfSuccessor(a, aCandidate));
                    const B2DPoint aStart(aCandidate.getB2DPoint(a));
                    const B2DPoint aEnd(aCandidate.getB2DPoint(nNextIndex));
                    B2DVector aVector(aEnd - aStart);
                    double fLength(aVector.getLength());
                    double fPosOnVector(0.0);

                    // normalize vector for further usage as multiplication base
                    aVector.normalize();

                    while(fLength >= fDashDotLength)
                    {
                        // handle [fPosOnVector .. fPosOnVector+fDashDotLength]
                        if(!(nDashDotIndex % 2L))
                        {
                            B2DPolygon aResult;

                            // add start point
                            if(0.0 == fPosOnVector)
                            {
                                aResult.append(aStart);
                            }
                            else
                            {
                                aResult.append( B2DPoint(aStart + (aVector * fPosOnVector)) );
                            }

                            // add end point
                            aResult.append( B2DPoint(aStart + (aVector * (fPosOnVector + fDashDotLength))) );

                            // add line to PolyPolygon
                            aRetval.append(aResult);
                        }

                        // consume from fDashDotLength
                        fPosOnVector += fDashDotLength;
                        fLength -= fDashDotLength;
                        nDashDotIndex = (nDashDotIndex + 1L) % raDashDotArray.size();
                        fDashDotLength = raDashDotArray[nDashDotIndex];
                    }

                    // handle [fPosOnVector .. fPosOnVector+fLength (bzw. end)]
                    if((fLength > 0.0) && (!(nDashDotIndex % 2L)))
                    {
                        B2DPolygon aResult;

                        // add start point
                        if(0.0 == fPosOnVector)
                        {
                            aResult.append(aStart);
                        }
                        else
                        {
                            // add modified start
                            aResult.append( B2DPoint(aStart + (aVector * fPosOnVector)) );
                        }

                        // add end point
                        aResult.append(aEnd);

                        // add line to PolyPolygon
                        aRetval.append(aResult);
                    }

                    // consume from fDashDotLength
                    fDashDotLength -= fLength;
                }
            }

            return aRetval;
        }

        // test if point is inside epsilon-range around an edge defined
        // by the two given points. Can be used for HitTesting. The epsilon-range
        // is defined to be the rectangle centered to the given edge, using height
        // 2 x fDistance, and the circle around both points with radius fDistance.
        bool isInEpsilonRange(const B2DPoint& rEdgeStart, const B2DPoint& rEdgeEnd, const B2DPoint& rTestPosition, double fDistance)
        {
            // build edge vector
            const B2DVector aEdge(rEdgeEnd - rEdgeStart);
            bool bDeltaXIsZero(fTools::equalZero(aEdge.getX()));
            bool bDeltaYIsZero(fTools::equalZero(aEdge.getY()));
            bool bDoDistanceTestStart(false);
            bool bDoDistanceTestEnd(false);

            if(bDeltaXIsZero && bDeltaYIsZero)
            {
                // no edge, just a point. Do one of the distance tests.
                bDoDistanceTestStart = true;
            }
            else
            {
                // edge has a length. Create perpendicular vector.
                const B2DVector aPerpend(getPerpendicular(aEdge));
                double fCut(
                    (aPerpend.getY() * (rTestPosition.getX() - rEdgeStart.getX())
                    + aPerpend.getX() * (rEdgeStart.getY() - rTestPosition.getY())) /
                    (aEdge.getX() * aEdge.getX() + aEdge.getY() * aEdge.getY()));

                const double fZero(0.0);
                const double fOne(1.0);

                if(fTools::less(fCut, fZero))
                {
                    // left of rEdgeStart
                    bDoDistanceTestStart = true;
                }
                else if(fTools::more(fCut, fOne))
                {
                    // right of rEdgeEnd
                    bDoDistanceTestEnd = true;
                }
                else
                {
                    // inside line [0.0 .. 1.0]
                    const B2DPoint aCutPoint(interpolate(rEdgeStart, rEdgeEnd, fCut));
                    const double fDeltaX(rTestPosition.getX() - aCutPoint.getX());
                    const double fDeltaY(rTestPosition.getY() - aCutPoint.getY());
                    const double fDistanceSquare(fDeltaX * fDeltaX + fDeltaY * fDeltaY);

                    if(fDistanceSquare <= fDistance * fDistance)
                    {
                        return true;
                    }
                    else
                    {
                        return sal_False;
                    }
                }
            }

            if(bDoDistanceTestStart)
            {
                const double fDeltaX(rTestPosition.getX() - rEdgeStart.getX());
                const double fDeltaY(rTestPosition.getY() - rEdgeStart.getY());
                const double fDistanceSquare(fDeltaX * fDeltaX + fDeltaY * fDeltaY);

                if(fDistanceSquare <= fDistance * fDistance)
                {
                    return true;
                }
            }
            else if(bDoDistanceTestEnd)
            {
                const double fDeltaX(rTestPosition.getX() - rEdgeEnd.getX());
                const double fDeltaY(rTestPosition.getY() - rEdgeEnd.getY());
                const double fDistanceSquare(fDeltaX * fDeltaX + fDeltaY * fDeltaY);

                if(fDistanceSquare <= fDistance * fDistance)
                {
                    return true;
                }
            }

            return false;
        }

        // test if point is inside epsilon-range around the given Polygon. Can be used
        // for HitTesting. The epsilon-range is defined to be the tube around the polygon
        // with distance fDistance and rounded edges (start and end point).
        bool isInEpsilonRange(const B2DPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance)
        {
            if(rCandidate.areControlPointsUsed())
            {
                // call myself recursively with subdivided input
                const B2DPolygon aCandidate(adaptiveSubdivideByAngle(rCandidate));
                return isInEpsilonRange(aCandidate, rTestPosition, fDistance);
            }
            else
            {
                if(rCandidate.count())
                {
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? rCandidate.count() : rCandidate.count() - 1L);

                    for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                    {
                        B2DPoint aStart(rCandidate.getB2DPoint(a));
                        const sal_uInt32 nNextIndex(getIndexOfSuccessor(a, rCandidate));
                        B2DPoint aEnd(rCandidate.getB2DPoint(nNextIndex));

                        if(isInEpsilonRange(aStart, aEnd, rTestPosition, fDistance))
                        {
                            return true;
                        }
                    }
                }

                return false;
            }
        }

        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect, double fRadius )
        {
            const double fZero(0.0);
            const double fOne(1.0);

            if(fTools::lessOrEqual(fRadius, fZero))
            {
                // no radius, use rectangle
                return createPolygonFromRect( rRect );
            }
            else if(fTools::moreOrEqual(fRadius, fOne))
            {
                // full radius, use ellipse
                const B2DPoint aCenter(rRect.getCenter());
                const double fRadiusX(rRect.getWidth() / 2.0);
                const double fRadiusY(rRect.getHeight() / 2.0);

                return createPolygonFromEllipse( aCenter, fRadiusX, fRadiusY );
            }
            else
            {
                // create rectangle with two radii between ]0.0 .. 1.0[
                return createPolygonFromRect( rRect, fRadius, fRadius );
            }
        }

        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect, double fRadiusX, double fRadiusY )
        {
            const double fZero(0.0);
            const double fOne(1.0);

            // crop to useful values
            if(fTools::less(fRadiusX, fZero))
            {
                fRadiusX = fZero;
            }
            else if(fTools::more(fRadiusX, fOne))
            {
                fRadiusX = fOne;
            }

            if(fTools::less(fRadiusY, fZero))
            {
                fRadiusY = fZero;
            }
            else if(fTools::more(fRadiusY, fOne))
            {
                fRadiusY = fOne;
            }

            if(fZero == fRadiusX || fZero == fRadiusY)
            {
                // at least in one direction no radius, use rectangle
                return createPolygonFromRect( rRect );
            }
            else if(fOne == fRadiusX && fOne == fRadiusY)
            {
                // in both directions full radius, use ellipse
                const B2DPoint aCenter(rRect.getCenter());
                const double fRectRadiusX(rRect.getWidth() / 2.0);
                const double fRectRadiusY(rRect.getHeight() / 2.0);

                return createPolygonFromEllipse( aCenter, fRectRadiusX, fRectRadiusY );
            }
            else
            {
                B2DPolygon aRetval;
                const double fBowX((rRect.getWidth() / 2.0) * fRadiusX);
                const double fBowY((rRect.getHeight() / 2.0) * fRadiusY);
                const double fKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);

                // create first bow
                const B2DPoint aBottomRight(rRect.getMaxX(), rRect.getMaxY());
                aRetval.append(aBottomRight + B2DPoint(0.0, -fBowY));
                aRetval.append(aBottomRight + B2DPoint(-fBowX, 0.0));
                aRetval.setControlPointA(0L, interpolate(aRetval.getB2DPoint(0L), aBottomRight, fKappa));
                aRetval.setControlPointB(0L, interpolate(aRetval.getB2DPoint(1L), aBottomRight, fKappa));

                // create second bow
                const B2DPoint aBottomLeft(rRect.getMinX(), rRect.getMaxY());
                aRetval.append(aBottomLeft + B2DPoint(fBowX, 0.0));
                aRetval.append(aBottomLeft + B2DPoint(0.0, -fBowY));
                aRetval.setControlPointA(2L, interpolate(aRetval.getB2DPoint(2L), aBottomLeft, fKappa));
                aRetval.setControlPointB(2L, interpolate(aRetval.getB2DPoint(3L), aBottomLeft, fKappa));

                // create third bow
                const B2DPoint aTopLeft(rRect.getMinX(), rRect.getMinY());
                aRetval.append(aTopLeft + B2DPoint(0.0, fBowY));
                aRetval.append(aTopLeft + B2DPoint(fBowX, 0.0));
                aRetval.setControlPointA(4L, interpolate(aRetval.getB2DPoint(4L), aTopLeft, fKappa));
                aRetval.setControlPointB(4L, interpolate(aRetval.getB2DPoint(5L), aTopLeft, fKappa));

                // create forth bow
                const B2DPoint aTopRight(rRect.getMaxX(), rRect.getMinY());
                aRetval.append(aTopRight + B2DPoint(-fBowX, 0.0));
                aRetval.append(aTopRight + B2DPoint(0.0, fBowY));
                aRetval.setControlPointA(6L, interpolate(aRetval.getB2DPoint(6L), aTopRight, fKappa));
                aRetval.setControlPointB(6L, interpolate(aRetval.getB2DPoint(7L), aTopRight, fKappa));

                // close
                aRetval.setClosed( true );

                // remove double created points if there is extreme radius
                if(fOne == fRadiusX || fOne == fRadiusY)
                {
                    aRetval.removeDoublePoints();
                }

                return aRetval;
            }
        }

        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect )
        {
            B2DPolygon aRetval;

            aRetval.append( B2DPoint( rRect.getMinX(), rRect.getMinY() ) );
            aRetval.append( B2DPoint( rRect.getMaxX(), rRect.getMinY() ) );
            aRetval.append( B2DPoint( rRect.getMaxX(), rRect.getMaxY() ) );
            aRetval.append( B2DPoint( rRect.getMinX(), rRect.getMaxY() ) );

            // close
            aRetval.setClosed( true );

            return aRetval;
        }

        B2DPolygon createPolygonFromCircle( const B2DPoint& rCenter, double fRadius )
        {
            return createPolygonFromEllipse( rCenter, fRadius, fRadius );
        }

        void appendUnitCircleQuadrant(B2DPolygon& rPolygon, sal_uInt32 nQuadrant, bool bEndPoint)
        {
            const double fZero(0.0);
            const double fOne(1.0);
            const double fKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);
            const sal_uInt32 nIndex(rPolygon.count());

            // create closed unit-circle with 4 segments
            switch(nQuadrant)
            {
                case 0 : // first quadrant
                {
                    rPolygon.append(B2DPoint(fOne, fZero));
                    rPolygon.setControlPointA(nIndex, B2DPoint(fOne, fKappa));
                    rPolygon.setControlPointB(nIndex, B2DPoint(fKappa, fOne));

                    if(bEndPoint)
                    {
                        rPolygon.append(B2DPoint(fZero, fOne));
                    }

                    break;
                }
                case 1 : // second quadrant
                {
                    rPolygon.append(B2DPoint(fZero, fOne));
                    rPolygon.setControlPointA(nIndex, B2DPoint(-fKappa, fOne));
                    rPolygon.setControlPointB(nIndex, B2DPoint(-fOne, fKappa));

                    if(bEndPoint)
                    {
                        rPolygon.append(B2DPoint(-fOne, fZero));
                    }

                    break;
                }
                case 2 : // third quadrant
                {
                    rPolygon.append(B2DPoint(-fOne, fZero));
                    rPolygon.setControlPointA(nIndex, B2DPoint(-fOne, -fKappa));
                    rPolygon.setControlPointB(nIndex, B2DPoint(-fKappa, -fOne));

                    if(bEndPoint)
                    {
                        rPolygon.append(B2DPoint(fZero, -fOne));
                    }

                    break;
                }
                default : // last quadrant
                {
                    rPolygon.append(B2DPoint(fZero, -fOne));
                    rPolygon.setControlPointA(nIndex, B2DPoint(fKappa, -fOne));
                    rPolygon.setControlPointB(nIndex, B2DPoint(fOne, -fKappa));

                    if(bEndPoint)
                    {
                        rPolygon.append(B2DPoint(fOne, fZero));
                    }

                    break;
                }
            }
        }

        B2DPolygon createPolygonFromUnitCircle()
        {
            B2DPolygon aRetval;

            // create unit-circle with all 4 segments, close it
            appendUnitCircleQuadrant(aRetval, 0L, false);
            appendUnitCircleQuadrant(aRetval, 1L, false);
            appendUnitCircleQuadrant(aRetval, 2L, false);
            appendUnitCircleQuadrant(aRetval, 3L, false);
            aRetval.setClosed(true);

            return aRetval;
        }

        B2DPolygon createPolygonFromEllipse( const B2DPoint& rCenter, double fRadiusX, double fRadiusY )
        {
            const double fOne(1.0);
            B2DPolygon aRetval(createPolygonFromUnitCircle());

            // transformation necessary?
            const sal_Bool bScale(!fTools::equal(fRadiusX, fOne) || !fTools::equal(fRadiusY, fOne));
            const sal_Bool bTranslate(!rCenter.equalZero());

            if(bScale || bTranslate)
            {
                B2DHomMatrix aMatrix;

                if(bScale)
                {
                    aMatrix.scale(fRadiusX, fRadiusY);
                }

                if(bTranslate)
                {
                    aMatrix.translate(rCenter.getX(), rCenter.getY());
                }

                aRetval.transform(aMatrix);
            }

            return aRetval;
        }

        void appendUnitCircleQuadrantSegment(B2DPolygon& rPolygon, sal_uInt32 nQuadrant, double fStart, double fEnd, bool bEndPoint)
        {
            OSL_ENSURE(fStart >= 0.0 && fStart <= 1.0, "appendUnitCircleQuadrant: Access out of range (!)");
            OSL_ENSURE(fEnd >= 0.0 && fEnd <= 1.0, "appendUnitCircleQuadrant: Access out of range (!)");
            OSL_ENSURE(fEnd >= fStart, "appendUnitCircleQuadrant: Access out of range (!)");
            const double fOne(1.0);
            const bool bStartIsZero(fTools::equalZero(fStart));
            const bool bEndIsOne(fTools::equal(fEnd, fOne));

            if(bStartIsZero && bEndIsOne)
            {
                // add completely
                appendUnitCircleQuadrant(rPolygon, nQuadrant, bEndPoint);
            }
            else
            {
                // split and add
                B2DPolygon aQuadrant;
                appendUnitCircleQuadrant(aQuadrant, nQuadrant, true);
                const bool bStartEndEqual(fTools::equal(fStart, fEnd));

                if(bStartEndEqual && bEndPoint)
                {
                    if(bStartIsZero)
                    {
                        // both zero, add start point
                        rPolygon.append(aQuadrant.getB2DPoint(0L));
                    }
                    else if(bEndIsOne)
                    {
                        // both one, add end point
                        rPolygon.append(aQuadrant.getB2DPoint(1L));
                    }
                    else
                    {
                        // both equal but not zero, add split point
                        B2DCubicBezier aCubicBezier(aQuadrant.getB2DPoint(0L), aQuadrant.getControlPointA(0L), aQuadrant.getControlPointB(0L), aQuadrant.getB2DPoint(1L));
                        B2DCubicBezier aCubicBezierWaste;

                        aCubicBezier.split(fStart, aCubicBezier, aCubicBezierWaste);

                        rPolygon.append(aCubicBezier.getEndPoint());
                    }
                }
                else
                {
                    B2DCubicBezier aCubicBezier(aQuadrant.getB2DPoint(0L), aQuadrant.getControlPointA(0L), aQuadrant.getControlPointB(0L), aQuadrant.getB2DPoint(1L));
                    B2DCubicBezier aCubicBezierWaste;

                    if(!bEndIsOne)
                    {
                        aCubicBezier.split(fEnd, aCubicBezier, aCubicBezierWaste);

                        if(!bStartIsZero)
                        {
                            fStart /= fEnd;
                        }
                    }

                    if(!bStartIsZero)
                    {
                        aCubicBezier.split(fStart, aCubicBezierWaste, aCubicBezier);
                    }

                    const sal_uInt32 nIndex(rPolygon.count());
                    rPolygon.append(aCubicBezier.getStartPoint());
                    rPolygon.setControlPointA(nIndex, aCubicBezier.getControlPointA());
                    rPolygon.setControlPointB(nIndex, aCubicBezier.getControlPointB());

                    if(bEndPoint)
                    {
                        rPolygon.append(aCubicBezier.getEndPoint());
                    }
                }
            }
        }

        B2DPolygon createPolygonFromUnitEllipseSegment( double fStart, double fEnd )
        {
            B2DPolygon aRetval;

            if(fTools::less(fStart, 0.0))
            {
                fStart = 0.0;
            }

            if(fTools::more(fStart, F_2PI))
            {
                fStart = F_2PI;
            }

            if(fTools::less(fEnd, 0.0))
            {
                fEnd = 0.0;
            }

            if(fTools::more(fEnd, F_2PI))
            {
                fEnd = F_2PI;
            }

            const sal_uInt32 nQuadrantStart(sal_uInt32(fStart / F_PI2) % 4L);
            const sal_uInt32 nQuadrantEnd(sal_uInt32(fEnd / F_PI2) % 4L);
            sal_uInt32 nCurrentQuadrant(nQuadrantStart);
            bool bStartDone(false);
            bool bEndDone(false);

            do
            {
                if(!bStartDone && nQuadrantStart == nCurrentQuadrant)
                {
                    if(nQuadrantStart == nQuadrantEnd && fTools::moreOrEqual(fEnd, fStart))
                    {
                        // both in one quadrant and defining the complete segment, create start to end
                        double fSplitOffsetStart((fStart - (nCurrentQuadrant * F_PI2)) / F_PI2);
                        double fSplitOffsetEnd((fEnd - (nCurrentQuadrant * F_PI2)) / F_PI2);
                        appendUnitCircleQuadrantSegment(aRetval, nCurrentQuadrant, fSplitOffsetStart, fSplitOffsetEnd, true);
                        bStartDone = bEndDone = true;
                    }
                    else
                    {
                        // create start to quadrant end
                        const double fSplitOffsetStart((fStart - (nCurrentQuadrant * F_PI2)) / F_PI2);
                        appendUnitCircleQuadrantSegment(aRetval, nCurrentQuadrant, fSplitOffsetStart, 1.0, false);
                        bStartDone = true;
                    }
                }
                else if(!bEndDone && nQuadrantEnd == nCurrentQuadrant)
                {
                    // create quadrant start to end
                    const double fSplitOffsetEnd((fEnd - (nCurrentQuadrant * F_PI2)) / F_PI2);
                    appendUnitCircleQuadrantSegment(aRetval, nCurrentQuadrant, 0.0, fSplitOffsetEnd, true);
                    bEndDone = true;
                }
                else
                {
                    // add quadrant completely
                    appendUnitCircleQuadrant(aRetval, nCurrentQuadrant, false);
                }

                // next step
                nCurrentQuadrant = (nCurrentQuadrant + 1L) % 4L;
            }
            while(!(bStartDone && bEndDone));

            return aRetval;
        }

        B2DPolygon createPolygonFromEllipseSegment( const B2DPoint& rCenter, double fRadiusX, double fRadiusY, double fStart, double fEnd )
        {
            B2DPolygon aRetval(createPolygonFromUnitEllipseSegment(fStart, fEnd));

            // transformation necessary?
            const double fOne(1.0);
            const sal_Bool bScale(!fTools::equal(fRadiusX, fOne) || !fTools::equal(fRadiusY, fOne));
            const sal_Bool bTranslate(!rCenter.equalZero());

            if(bScale || bTranslate)
            {
                B2DHomMatrix aMatrix;

                if(bScale)
                {
                    aMatrix.scale(fRadiusX, fRadiusY);
                }

                if(bTranslate)
                {
                    aMatrix.translate(rCenter.getX(), rCenter.getY());
                }

                aRetval.transform(aMatrix);
            }

            return aRetval;
        }

        bool hasNeutralPoints(const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "hasNeutralPoints: ATM works not for curves (!)");

            if(rCandidate.count() > 2L)
            {
                B2DPoint aPrevPoint(rCandidate.getB2DPoint(rCandidate.count() - 1L));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    const bool bLast(a + 1L == rCandidate.count());
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint(bLast ? 0L : a + 1L));
                    const B2DVector aPrevVec(aPrevPoint - aCurrPoint);
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aOrientation(getOrientation(aNextVec, aPrevVec));

                    if(ORIENTATION_NEUTRAL == aOrientation)
                    {
                        // current has neutral orientation
                        return true;
                    }
                    else
                    {
                        // prepare next
                        if(!bLast)
                        {
                            aPrevPoint = aCurrPoint;
                            aCurrPoint = aNextPoint;
                        }
                    }
                }
            }

            return false;
        }

        B2DPolygon removeNeutralPoints(const B2DPolygon& rCandidate)
        {
            if(hasNeutralPoints(rCandidate))
            {
                B2DPolygon aRetval;
                B2DPoint aPrevPoint(rCandidate.getB2DPoint(rCandidate.count() - 1L));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    const bool bLast(a + 1L == rCandidate.count());
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint(bLast ? 0L : a + 1L));
                    const B2DVector aPrevVec(aPrevPoint - aCurrPoint);
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aOrientation(getOrientation(aNextVec, aPrevVec));

                    if(ORIENTATION_NEUTRAL == aOrientation)
                    {
                        // current has neutral orientation, leave it out and prepare next
                        if(!bLast)
                        {
                            aCurrPoint = aNextPoint;
                        }
                    }
                    else
                    {
                        // add current point
                        aRetval.append(aCurrPoint);

                        // prepare next
                        if(!bLast)
                        {
                            aPrevPoint = aCurrPoint;
                            aCurrPoint = aNextPoint;
                        }
                    }
                }

                while(aRetval.count() && ORIENTATION_NEUTRAL == getOrientationForIndex(aRetval, 0L))
                {
                    aRetval.remove(0L);
                }

                // copy closed state
                aRetval.setClosed(rCandidate.isClosed());

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        bool isConvex(const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "isConvex: ATM works not for curves (!)");

            if(rCandidate.count() > 2L)
            {
                const B2DPoint aPrevPoint(rCandidate.getB2DPoint(rCandidate.count() - 1L));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0L));
                B2DVector aCurrVec(aPrevPoint - aCurrPoint);
                B2VectorOrientation aOrientation(ORIENTATION_NEUTRAL);

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    const bool bLast(a + 1L == rCandidate.count());
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint(bLast ? 0L : a + 1L));
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aCurrentOrientation(getOrientation(aNextVec, aCurrVec));

                    if(ORIENTATION_NEUTRAL == aOrientation)
                    {
                        // set start value, maybe neutral again
                        aOrientation = aCurrentOrientation;
                    }
                    else
                    {
                        if(ORIENTATION_NEUTRAL != aCurrentOrientation && aCurrentOrientation != aOrientation)
                        {
                            // different orientations found, that's it
                            return false;
                        }
                    }

                    // prepare next
                    if(!bLast)
                    {
                        aCurrPoint = aNextPoint;
                        aCurrVec = -aNextVec;
                    }
                }
            }

            return true;
        }

        B2VectorOrientation getOrientationForIndex(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getOrientationForIndex: index out of range (!)");
            const B2DPoint aPrev(rCandidate.getB2DPoint(getIndexOfPredecessor(nIndex, rCandidate)));
            const B2DPoint aCurr(rCandidate.getB2DPoint(nIndex));
            const B2DPoint aNext(rCandidate.getB2DPoint(getIndexOfSuccessor(nIndex, rCandidate)));
            const B2DVector aBack(aPrev - aCurr);
            const B2DVector aForw(aNext - aCurr);

            return getOrientation(aForw, aBack);
        }

        bool isPointOnLine(const B2DPoint& rStart, const B2DPoint& rEnd, const B2DPoint& rCandidate, bool bWithPoints)
        {
            if(rCandidate.equal(rStart) || rCandidate.equal(rEnd))
            {
                // candidate is in epsilon around start or end -> inside
                return bWithPoints;
            }
            else if(rStart.equal(rEnd))
            {
                // start and end are equal, but candidate is outside their epsilon -> outside
                return false;
            }
            else
            {
                const B2DVector aEdgeVector(rEnd - rStart);
                const B2DVector aTestVector(rCandidate - rStart);

                if(areParallel(aEdgeVector, aTestVector))
                {
                    const double fZero(0.0);
                    const double fOne(1.0);
                    const double fParamTestOnCurr(fabs(aEdgeVector.getX()) > fabs(aEdgeVector.getY())
                        ? aTestVector.getX() / aEdgeVector.getX()
                        : aTestVector.getY() / aEdgeVector.getY());

                    if(fTools::more(fParamTestOnCurr, fZero) && fTools::less(fParamTestOnCurr, fOne))
                    {
                        return true;
                    }
                }

                return false;
            }
        }

        bool isPointInTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC, const B2DPoint& rCandidate, bool bWithBorder)
        {
            if(arePointsOnSameSideOfLine(rA, rB, rC, rCandidate, bWithBorder))
            {
                if(arePointsOnSameSideOfLine(rB, rC, rA, rCandidate, bWithBorder))
                {
                    if(arePointsOnSameSideOfLine(rC, rA, rB, rCandidate, bWithBorder))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        bool arePointsOnSameSideOfLine(const B2DPoint& rStart, const B2DPoint& rEnd, const B2DPoint& rCandidateA, const B2DPoint& rCandidateB, bool bWithLine)
        {
            const B2DVector aLineVector(rEnd - rStart);
            const B2DVector aVectorToA(rEnd - rCandidateA);
            const double fCrossA(aLineVector.cross(aVectorToA));

            if(fTools::equalZero(fCrossA))
            {
                // one point on the line
                return bWithLine;
            }

            const B2DVector aVectorToB(rEnd - rCandidateB);
            const double fCrossB(aLineVector.cross(aVectorToB));

            if(fTools::equalZero(fCrossB))
            {
                // one point on the line
                return bWithLine;
            }

            // return true if they both have the same sign
            return ((fCrossA > 0.0) == (fCrossB > 0.0));
        }

        void addTriangleFan(const B2DPolygon& rCandidate, B2DPolygon& rTarget)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount > 2L)
            {
                const B2DPoint aStart(rCandidate.getB2DPoint(0L));
                B2DPoint aLast(rCandidate.getB2DPoint(1L));

                for(sal_uInt32 a(2L); a < nCount; a++)
                {
                    const B2DPoint aCurrent(rCandidate.getB2DPoint(a));
                    rTarget.append(aStart);
                    rTarget.append(aLast);
                    rTarget.append(aCurrent);

                    // prepare next
                    aLast = aCurrent;
                }
            }
        }

        namespace
        {
            /// return 0 for input of 0, -1 for negative and 1 for positive input
            inline int lcl_sgn( const double n )
            {
                return n == 0.0 ? 0 : 1 - 2*::rtl::math::isSignBitSet(n);
            }
        }

        bool isRectangle( const B2DPolygon& rPoly )
        {
            // polygon must be closed to resemble a rect, and contain
            // at least four points.
            if( !rPoly.isClosed() ||
                rPoly.count() < 4 )
            {
                return false;
            }

            // number of 90 degree turns the polygon has taken
            int nNumTurns(0);

            int  nVerticalEdgeType=0;
            int  nHorizontalEdgeType=0;
            bool bNullVertex(true);
            bool bCWPolygon(false);  // when true, polygon is CW
                                     // oriented, when false, CCW
            bool bOrientationSet(false); // when false, polygon
                                         // orientation has not yet
                                         // been determined.

            // scan all _edges_ (which involves coming back to point 0
            // for the last edge - thus the modulo operation below)
            const sal_Int32 nCount( rPoly.count() );
            for( sal_Int32 i=0; i<nCount; ++i )
            {
                const B2DPoint& rPoint0( rPoly.getB2DPoint(i % nCount) );
                const B2DPoint& rPoint1( rPoly.getB2DPoint((i+1) % nCount) );

                // is 0 for zero direction vector, 1 for south edge and -1
                // for north edge (standard screen coordinate system)
                int nCurrVerticalEdgeType( lcl_sgn( rPoint1.getY() - rPoint0.getY() ) );

                // is 0 for zero direction vector, 1 for east edge and -1
                // for west edge (standard screen coordinate system)
                int nCurrHorizontalEdgeType( lcl_sgn(rPoint1.getX() - rPoint0.getX()) );

                if( nCurrVerticalEdgeType && nCurrHorizontalEdgeType )
                    return false; // oblique edge - for sure no rect

                const bool bCurrNullVertex( !nCurrVerticalEdgeType && !nCurrHorizontalEdgeType );

                // current vertex is equal to previous - just skip,
                // until we have a real edge
                if( bCurrNullVertex )
                    continue;

                // if previous edge has two identical points, because
                // no previous edge direction was available, simply
                // take this first non-null edge as the start
                // direction. That's what will happen here, if
                // bNullVertex is false
                if( !bNullVertex )
                {
                    // 2D cross product - is 1 for CW and -1 for CCW turns
                    const int nCrossProduct( nHorizontalEdgeType*nCurrVerticalEdgeType -
                                             nVerticalEdgeType*nCurrHorizontalEdgeType );

                    if( !nCrossProduct )
                        continue; // no change in orientation -
                                  // collinear edges - just go on

                    // if polygon orientation is not set, we'll
                    // determine it now
                    if( !bOrientationSet )
                    {
                        bCWPolygon = nCrossProduct == 1;
                        bOrientationSet = true;
                    }
                    else
                    {
                        // if current turn orientation is not equal
                        // initial orientation, this is not a
                        // rectangle (as rectangles have consistent
                        // orientation).
                        if( (nCrossProduct == 1) != bCWPolygon )
                            return false;
                    }

                    ++nNumTurns;

                    // More than four 90 degree turns are an
                    // indication that this must not be a rectangle.
                    if( nNumTurns > 4 )
                        return false;
                }

                // store current state for the next turn
                nVerticalEdgeType   = nCurrVerticalEdgeType;
                nHorizontalEdgeType = nCurrHorizontalEdgeType;
                bNullVertex         = false; // won't reach this line,
                                             // if bCurrNullVertex is
                                             // true - see above
            }

            return true;
        }

        B3DPolygon createB3DPolygonFromB2DPolygon(const B2DPolygon& rCandidate, double fZCoordinate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                // call myself recursively with subdivided input
                const B2DPolygon aCandidate(adaptiveSubdivideByAngle(rCandidate));
                return createB3DPolygonFromB2DPolygon(aCandidate, fZCoordinate);
            }
            else
            {
                B3DPolygon aRetval;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    B2DPoint aPoint(rCandidate.getB2DPoint(a));
                    aRetval.append(B3DPoint(aPoint.getX(), aPoint.getY(), fZCoordinate));
                }

                // copy closed state
                aRetval.setClosed(rCandidate.isClosed());

                return aRetval;
            }
        }

        B2DPolygon createB2DPolygonFromB3DPolygon(const B3DPolygon& rCandidate, const B3DHomMatrix& rMat)
        {
            B2DPolygon aRetval;
            const sal_uInt32 nCount(rCandidate.count());
            const bool bIsIdentity(rMat.isIdentity());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                B3DPoint aCandidate(rCandidate.getB3DPoint(a));

                if(!bIsIdentity)
                {
                    aCandidate *= rMat;
                }

                aRetval.append(B2DPoint(aCandidate.getX(), aCandidate.getY()));
            }

            // copy closed state
            aRetval.setClosed(rCandidate.isClosed());

            return aRetval;
        }

        double getSmallestDistancePointToEdge(const B2DPoint& rPointA, const B2DPoint& rPointB, const B2DPoint& rTestPoint, double& rCut)
        {
            if(rPointA.equal(rPointB))
            {
                const B2DVector aVector(rTestPoint - rPointA);
                return aVector.getLength();
            }
            else
            {
                // get the relative cut value on line vector (Vector1) for cut with perpendicular through TestPoint
                const B2DVector aVector1(rPointB - rPointA);
                const B2DVector aVector2(rTestPoint - rPointA);
                const double fDividend((aVector2.getX() * aVector1.getX()) + (aVector2.getY() * aVector1.getY()));
                const double fDivisor((aVector1.getX() * aVector1.getX()) + (aVector1.getY() * aVector1.getY()));
                const double fCut(fDividend / fDivisor);

                if(fCut < 0.0)
                {
                    // not in line range, get distance to PointA
                    rCut = 0.0;
                    return aVector2.getLength();
                }
                else if(fCut > 1.0)
                {
                    // not in line range, get distance to PointB
                    rCut = 1.0;
                    const B2DVector aVector(rTestPoint - rPointB);
                    return aVector.getLength();
                }
                else
                {
                    // in line range
                    const B2DPoint aCutPoint(rPointA + fCut * aVector1);
                    const B2DVector aVector(rTestPoint - aCutPoint);
                    rCut = fCut;
                    return aVector.getLength();
                }
            }
        }

        double getSmallestDistancePointToPolygon(const B2DPolygon& rCandidate, const B2DPoint& rTestPoint, sal_uInt32& rEdgeIndex, double& rCut)
        {
            double fRetval(DBL_MAX);

            if(rCandidate.count() > 1L)
            {
                const double fZero(0.0);
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? rCandidate.count() : rCandidate.count() - 1L);

                for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                {
                    const B2DPoint aPointA(rCandidate.getB2DPoint(a));
                    const B2DPoint aPointB(rCandidate.getB2DPoint(getIndexOfSuccessor(a, rCandidate)));
                    double fEdgeDist;
                    double fNewCut;

                    if(rCandidate.areControlVectorsUsed())
                    {
                        const B2DVector aVectorA(rCandidate.getControlVectorA(a));
                        const B2DVector aVectorB(rCandidate.getControlVectorB(a));

                        if(aVectorA.equalZero() && aVectorB.equalZero())
                        {
                            fEdgeDist = getSmallestDistancePointToEdge(aPointA, aPointB, rTestPoint, fNewCut);
                        }
                        else
                        {
                            B2DCubicBezier aBezier(aPointA, B2DPoint(aPointA + aVectorA), B2DPoint(aPointA + aVectorB), aPointB);
                            fEdgeDist = aBezier.getSmallestDistancePointToBezierSegment(rTestPoint, fNewCut);
                        }
                    }
                    else
                    {
                        fEdgeDist = getSmallestDistancePointToEdge(aPointA, aPointB, rTestPoint, fNewCut);
                    }

                    if(DBL_MAX == fRetval || fEdgeDist < fRetval)
                    {
                        fRetval = fEdgeDist;
                        rEdgeIndex = a;
                        rCut = fNewCut;

                        if(fTools::equal(fRetval, fZero))
                        {
                            // already found zero distance, cannot get better. Ensure numerical zero value and end loop.
                            fRetval = 0.0;
                            break;
                        }
                    }
                }

                if(1.0 == rCut)
                {
                    // correct rEdgeIndex when not last point
                    if(rCandidate.isClosed())
                    {
                        rEdgeIndex = getIndexOfSuccessor(rEdgeIndex, rCandidate);
                        rCut = 0.0;
                    }
                    else
                    {
                        if(rEdgeIndex != nEdgeCount - 1L)
                        {
                            rEdgeIndex++;
                            rCut = 0.0;
                        }
                    }
                }
            }

            return fRetval;
        }

        B2DPoint distort(const B2DPoint& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight)
        {
            if(fTools::equalZero(rOriginal.getWidth()) || fTools::equalZero(rOriginal.getHeight()))
            {
                return rCandidate;
            }
            else
            {
                const double fRelativeX((rCandidate.getX() - rOriginal.getMinX()) / rOriginal.getWidth());
                const double fRelativeY((rCandidate.getY() - rOriginal.getMinY()) / rOriginal.getHeight());
                const double fOneMinusRelativeX(1.0 - fRelativeX);
                const double fOneMinusRelativeY(1.0 - fRelativeY);
                const double fNewX((fOneMinusRelativeY) * ((fOneMinusRelativeX) * rTopLeft.getX() + fRelativeX * rTopRight.getX()) +
                    fRelativeY * ((fOneMinusRelativeX) * rBottomLeft.getX() + fRelativeX * rBottomRight.getX()));
                const double fNewY((fOneMinusRelativeX) * ((fOneMinusRelativeY) * rTopLeft.getY() + fRelativeY * rBottomLeft.getY()) +
                    fRelativeX * ((fOneMinusRelativeY) * rTopRight.getY() + fRelativeY * rBottomRight.getY()));

                return B2DPoint(fNewX, fNewY);
            }
        }

        B2DPolygon distort(const B2DPolygon& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount && 0.0 != rOriginal.getWidth() && 0.0 != rOriginal.getHeight())
            {
                B2DPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    aRetval.append(distort(rCandidate.getB2DPoint(a), rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));

                    if(rCandidate.areControlVectorsUsed())
                    {
                        if(!rCandidate.getControlVectorA(a).equalZero())
                        {
                            aRetval.setControlPointA(a, distort(rCandidate.getControlPointA(a), rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));
                        }

                        if(!rCandidate.getControlVectorB(a).equalZero())
                        {
                            aRetval.setControlPointB(a, distort(rCandidate.getControlPointB(a), rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));
                        }
                    }
                }

                aRetval.setClosed(rCandidate.isClosed());
                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolygon rotateAroundPoint(const B2DPolygon& rCandidate, const B2DPoint& rCenter, double fAngle)
        {
            const sal_uInt32 nPointCount(rCandidate.count());
            B2DPolygon aRetval(rCandidate);

            if(nPointCount)
            {
                B2DHomMatrix aMatrix;

                aMatrix.translate(-rCenter.getX(), -rCenter.getY());
                aMatrix.rotate(fAngle);
                aMatrix.translate(rCenter.getX(), rCenter.getY());

                aRetval.transform(aMatrix);
            }

            return aRetval;
        }

        B2DPolygon expandToCurve(const B2DPolygon& rCandidate)
        {
            B2DPolygon aRetval(rCandidate);

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                expandToCurveInPoint(aRetval, a);
            }

            return aRetval;
        }

        bool expandToCurveInPoint(B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "expandToCurveInPoint: Access to polygon out of range (!)");
            bool bRetval(false);

            if(rCandidate.count())
            {
                // look for predecessor
                const sal_uInt32 nPrev(getIndexOfPredecessor(nIndex, rCandidate));

                if(nPrev != nIndex && rCandidate.getControlVectorB(nPrev).equalZero())
                {
                    rCandidate.setControlPointB(nPrev, interpolate(rCandidate.getB2DPoint(nIndex), rCandidate.getB2DPoint(nPrev), 1.0 / 3.0));
                    bRetval = true;
                }

                // look for successor
                const sal_uInt32 nNext(getIndexOfSuccessor(nIndex, rCandidate));

                if(nNext != nIndex && rCandidate.getControlVectorA(nIndex).equalZero())
                {
                    rCandidate.setControlPointA(nIndex, interpolate(rCandidate.getB2DPoint(nIndex), rCandidate.getB2DPoint(nNext), 1.0 / 3.0));
                    bRetval = true;
                }
            }

            return bRetval;
        }

        B2DPolygon setContinuity(const B2DPolygon& rCandidate, B2VectorContinuity eContinuity)
        {
            B2DPolygon aRetval(rCandidate);

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                setContinuityInPoint(aRetval, a, eContinuity);
            }

            return aRetval;
        }

        bool setContinuityInPoint(B2DPolygon& rCandidate, sal_uInt32 nIndex, B2VectorContinuity eContinuity)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "setContinuityInPoint: Access to polygon out of range (!)");
            bool bRetval(false);

            if(rCandidate.count())
            {
                const sal_uInt32 nPrev(getIndexOfPredecessor(nIndex, rCandidate));
                const sal_uInt32 nNext(getIndexOfSuccessor(nIndex, rCandidate));

                if(nPrev != nIndex && nNext != nIndex)
                {
                    const B2DVector aControlVectorB(rCandidate.getControlVectorB(nPrev));
                    const B2DVector aControlVectorA(rCandidate.getControlVectorA(nIndex));
                    const B2DPoint aCurrentPoint(rCandidate.getB2DPoint(nIndex));

                    switch(eContinuity)
                    {
                        case CONTINUITY_NONE :
                        {
                            if(!aControlVectorB.equalZero())
                            {
                                rCandidate.setControlPointB(nPrev, interpolate(aCurrentPoint, rCandidate.getB2DPoint(nPrev), 1.0 / 3.0));
                                bRetval = true;
                            }

                            if(!aControlVectorA.equalZero())
                            {
                                rCandidate.setControlPointA(nIndex, interpolate(aCurrentPoint, rCandidate.getB2DPoint(nNext), 1.0 / 3.0));
                                bRetval = true;
                            }

                            break;
                        }
                        case CONTINUITY_C1 :
                        {
                            if(!aControlVectorB.equalZero() && !aControlVectorA.equalZero())
                            {
                                B2DVector aVectorPrev(rCandidate.getControlPointB(nPrev) - aCurrentPoint);
                                B2DVector aVectorNext(aControlVectorA);
                                const double fLenPrev(aVectorPrev.getLength());
                                const double fLenNext(aVectorNext.getLength());
                                aVectorPrev.normalize();
                                aVectorNext.normalize();
                                const B2VectorOrientation aOrientation(getOrientation(aVectorPrev, aVectorNext));

                                if(ORIENTATION_NEUTRAL == aOrientation)
                                {
                                    // already parallel, check length
                                    if(fTools::equal(fLenPrev, fLenNext))
                                    {
                                        // this would be even C2, but we want C1. Use the lengths of the corresponding edges.
                                        const double fLenPrevEdge(B2DVector(rCandidate.getB2DPoint(nPrev) - aCurrentPoint).getLength() * (1.0 / 3.0));
                                        const double fLenNextEdge(B2DVector(rCandidate.getB2DPoint(nNext) - aCurrentPoint).getLength() * (1.0 / 3.0));

                                        rCandidate.setControlPointB(nPrev, aCurrentPoint + (aVectorPrev * fLenPrevEdge));
                                        rCandidate.setControlPointA(nIndex, aCurrentPoint + (aVectorNext * fLenNextEdge));

                                        bRetval = true;
                                    }
                                }
                                else
                                {
                                    // not parallel, set vectors and length
                                    const B2DVector aNormalizedPerpendicular(getNormalizedPerpendicular(aVectorPrev + aVectorNext));

                                    if(ORIENTATION_POSITIVE == aOrientation)
                                    {
                                        rCandidate.setControlPointB(nPrev, aCurrentPoint - (aNormalizedPerpendicular * fLenPrev));
                                        rCandidate.setControlPointA(nIndex, aCurrentPoint + (aNormalizedPerpendicular * fLenNext));
                                    }
                                    else
                                    {
                                        rCandidate.setControlPointB(nPrev, aCurrentPoint + (aNormalizedPerpendicular * fLenPrev));
                                        rCandidate.setControlPointA(nIndex, aCurrentPoint - (aNormalizedPerpendicular * fLenNext));
                                    }

                                    bRetval = true;
                                }
                            }
                            break;
                        }
                        case CONTINUITY_C2 :
                        {
                            if(!aControlVectorB.equalZero() && !aControlVectorA.equalZero())
                            {
                                B2DVector aVectorPrev(rCandidate.getControlPointB(nPrev) - aCurrentPoint);
                                B2DVector aVectorNext(aControlVectorA);
                                const double fCommonLength((aVectorPrev.getLength() + aVectorNext.getLength()) / 2.0);
                                aVectorPrev.normalize();
                                aVectorNext.normalize();
                                const B2VectorOrientation aOrientation(getOrientation(aVectorPrev, aVectorNext));

                                if(ORIENTATION_NEUTRAL == aOrientation)
                                {
                                    // already parallel, set length. Use one direction for better numerical correctness
                                    const B2DVector aScaledDirection(aVectorPrev * fCommonLength);

                                    rCandidate.setControlPointB(nPrev, aCurrentPoint + aScaledDirection);
                                    rCandidate.setControlPointA(nIndex, aCurrentPoint - aScaledDirection);
                                }
                                else
                                {
                                    // not parallel, set vectors and length
                                    const B2DVector aNormalizedPerpendicular(getNormalizedPerpendicular(aVectorPrev + aVectorNext));
                                    const B2DVector aPerpendicular(aNormalizedPerpendicular * fCommonLength);

                                    if(ORIENTATION_POSITIVE == aOrientation)
                                    {
                                        rCandidate.setControlPointB(nPrev, aCurrentPoint - aPerpendicular);
                                        rCandidate.setControlPointA(nIndex, aCurrentPoint + aPerpendicular);
                                    }
                                    else
                                    {
                                        rCandidate.setControlPointB(nPrev, aCurrentPoint + aPerpendicular);
                                        rCandidate.setControlPointA(nIndex, aCurrentPoint - aPerpendicular);
                                    }
                                }

                                bRetval = true;
                            }
                            break;
                        }
                    }
                }
            }

            return bRetval;
        }

        bool isPolyPolygonEqualRectangle( const ::basegfx::B2DPolyPolygon& rPolyPoly,
                                          const ::basegfx::B2DRange&       rRect )
        {
            // exclude some cheap cases first
            if( rPolyPoly.count() != 1 )
                return false;

            // fill array with rectangle vertices
            const ::basegfx::B2DPoint aPoints[] =
              {
                  ::basegfx::B2DPoint(rRect.getMinX(),rRect.getMinY()),
                  ::basegfx::B2DPoint(rRect.getMaxX(),rRect.getMinY()),
                  ::basegfx::B2DPoint(rRect.getMaxX(),rRect.getMaxY()),
                  ::basegfx::B2DPoint(rRect.getMinX(),rRect.getMaxY())
              };

            const ::basegfx::B2DPolygon& rPoly( rPolyPoly.getB2DPolygon(0) );
            const sal_uInt32 nCount( rPoly.count() );
            const double epsilon = ::std::numeric_limits<double>::epsilon();

            for(unsigned int j=0; j<4; ++j)
            {
                const ::basegfx::B2DPoint &p1 = aPoints[j];
                const ::basegfx::B2DPoint &p2 = aPoints[(j+1)%4];
                bool bPointOnBoundary = false;
                for( sal_uInt32 i=0; i<nCount; ++i )
                {
                    const ::basegfx::B2DPoint p(rPoly.getB2DPoint(i));

                    //     1 | x0 y0 1 |
                    // A = - | x1 y1 1 |
                    //     2 | x2 y2 1 |
                    double fDoubleArea = p2.getX()*p.getY() -
                                         p2.getY()*p.getX() -
                                         p1.getX()*p.getY() +
                                         p1.getY()*p.getX() +
                                         p1.getX()*p2.getY() -
                                         p1.getY()*p2.getX();

                    if(fDoubleArea < epsilon)
                    {
                        bPointOnBoundary=true;
                        break;
                    }
                }
                if(!(bPointOnBoundary))
                    return false;
            }

            return true;
        }

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
