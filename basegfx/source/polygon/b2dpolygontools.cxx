/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygontools.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:46:23 $
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

#include <numeric>

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
            else
            {
                return 0L;
            }
        }

        B2VectorOrientation getOrientation(const B2DPolygon& rCandidate)
        {
            B2VectorOrientation eRetval(ORIENTATION_NEUTRAL);

            if(rCandidate.count() > 2L)
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
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "removeIntersections: ATM works not for curves (!)");
            B2DPolyPolygon aRetval;

            if(rCandidate.count() > 2L)
            {
                B2DPolyPolygonCutter aCutter;

                B2DPolygon aPreparedCandidate(rCandidate);
                aPreparedCandidate.removeDoublePoints();
                aCutter.addPolygon(aPreparedCandidate);
                aCutter.removeSelfIntersections();

                aRetval = aCutter.getPolyPolygon();

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
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "isInside: ATM works not for curves (!)");
            bool bRetval(false);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 2L)
            {
                B2DPoint aPreviousPoint(rCandidate.getB2DPoint(nPointCount - 1L));

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aCurrentPoint(rCandidate.getB2DPoint(a));

                    // test epsilon around points and line
                    if(isPointOnLine(aPreviousPoint, aCurrentPoint, rPoint, true))
                    {
                        return bWithBorder;
                    }

                    // cross-over in Y?
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
                        }
                        else
                        {
                            const double fCompare =
                                aCurrentPoint.getX() - (aCurrentPoint.getY() - rPoint.getY()) *
                                (aPreviousPoint.getX() - aCurrentPoint.getX()) /
                                (aPreviousPoint.getY() - aCurrentPoint.getY());

                            if(fTools::moreOrEqual(fCompare, rPoint.getX()))
                            {
                                bRetval = !bRetval;
                            }
                        }
                    }

                    // prepare next step
                    aPreviousPoint = aCurrentPoint;
                }
            }

            return bRetval;
        }

        bool isInside(const B2DPolygon& rCandidate, const B2DPolygon& rPolygon, bool bWithBorder)
        {
            const sal_uInt32 nPointCount(rPolygon.count());

            for(sal_uInt32 a(0L); a < nPointCount; a++)
            {
                const B2DPoint aTestPoint(rPolygon.getB2DPoint(a));

                if(!isInside(rCandidate, aTestPoint, bWithBorder))
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
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "getSignedArea: ATM works not for curves (!)");
            double fRetval(0.0);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 2)
            {
                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aPreviousPoint(rCandidate.getB2DPoint((!a) ? nPointCount - 1L : a - 1L));
                    const B2DPoint aCurrentPoint(rCandidate.getB2DPoint(a));

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

            if(rCandidate.count() > 2)
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
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");
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
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "isInEpsilonRange: ATM works not for curves (!)");

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

        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect )
        {
            B2DPolygon aRet;

            const double aX1( rRect.getMinX() );
            const double aX2( rRect.getMaxX() );
            const double aY1( rRect.getMinY() );
            const double aY2( rRect.getMaxY() );

            aRet.append( B2DPoint( aX1, aY1 ) );
            aRet.append( B2DPoint( aX2, aY1 ) );
            aRet.append( B2DPoint( aX2, aY2 ) );
            aRet.append( B2DPoint( aX1, aY2 ) );
            aRet.setClosed( true );

            return aRet;
        }

        B2DPolygon createPolygonFromCircle( const B2DPoint& rCenter, double nRadius )
        {
            return createPolygonFromEllipse( rCenter, nRadius, nRadius );
        }

        B2DPolygon createPolygonFromEllipse( const B2DPoint& rCenter, double nRadiusX, double nRadiusY )
        {
            B2DPolygon aRet;

            const double aX( rCenter.getX() );
            const double aY( rCenter.getY() );

            const double nKappa( (M_SQRT2-1.0)*4.0/3.0 );
            const double nlX( nRadiusX * nKappa );
            const double nlY( nRadiusY * nKappa );

            aRet.append( B2DPoint( aX,          aY-nRadiusY ) );
            aRet.append( B2DPoint( aX+nRadiusX, aY ) );
            aRet.append( B2DPoint( aX,          aY+nRadiusY ) );
            aRet.append( B2DPoint( aX-nRadiusX, aY ) );

            aRet.setControlPointA( 0, B2DPoint( aX+nlX,         aY-nRadiusY ) );
            aRet.setControlPointB( 0, B2DPoint( aX+nRadiusX,    aY-nlY ) );

            aRet.setControlPointA( 1, B2DPoint( aX+nRadiusX,    aY+nlY ) );
            aRet.setControlPointB( 1, B2DPoint( aX+nlX,         aY+nRadiusY ) );

            aRet.setControlPointA( 2, B2DPoint( aX-nlX,         aY+nRadiusY ) );
            aRet.setControlPointB( 2, B2DPoint( aX-nRadiusX,    aY+nlY ) );

            aRet.setControlPointA( 3, B2DPoint( aX-nRadiusX,    aY-nlY ) );
            aRet.setControlPointB( 3, B2DPoint( aX-nlX,         aY-nRadiusY ) );

            aRet.setClosed( true );

            return aRet;
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

        bool isRectangle( const B2DPolygon& rPoly )
        {
            if( rPoly.count() != 4 ||
                !rPoly.isClosed() )
            {
                return false;
            }

            const ::basegfx::B2DPoint& rPoint0( rPoly.getB2DPoint(0) );
            const ::basegfx::B2DPoint& rPoint1( rPoly.getB2DPoint(1) );

            bool bEdgeVertical( rPoint0.getX() == rPoint1.getX() );
            bool bEdgeHorizontal( rPoint0.getY() == rPoint1.getY() );

            if( !bEdgeVertical && !bEdgeHorizontal )
                return false; // oblique vertex - for sure no rect

            bool bNullVertex( bEdgeVertical && bEdgeHorizontal );

            for( sal_Int32 i=1; i<5; ++i )
            {
                const ::basegfx::B2DPoint& rPoint0( rPoly.getB2DPoint( i%4) );
                const ::basegfx::B2DPoint& rPoint1( rPoly.getB2DPoint( (i+1)%4 ) );

                const bool bCurrEdgeVertical( rPoint0.getX() == rPoint1.getX() );
                const bool bCurrEdgeHorizontal( rPoint0.getY() == rPoint1.getY() );

                if( !bCurrEdgeVertical && !bCurrEdgeHorizontal )
                    return false; // oblique vertex - for sure no rect

                const bool bCurrNullVertex( bCurrEdgeVertical && bCurrEdgeHorizontal );

                // direction change from last vertex?
                if( !bNullVertex && !bCurrNullVertex &&
                    (bEdgeVertical==bCurrEdgeVertical ||
                     bEdgeHorizontal==bCurrEdgeHorizontal) )
                {
                    // nope, for sure no rect
                    return false;
                }

                // might still be a rect - note that this code will
                // accept all configurations of collinear points as
                // rectangles, because they are representable as an
                // axis-aligned rect.
                bEdgeVertical   = bCurrEdgeVertical;
                bEdgeHorizontal = bCurrEdgeHorizontal;
                bNullVertex     = bCurrNullVertex;
            }

            return true;
        }

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
