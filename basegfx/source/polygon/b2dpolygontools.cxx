/*************************************************************************
 *
 *  $RCSfile: b2dpolygontools.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:30:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
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

#include <numeric>

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

        // Get index of outmost point (e.g. biggest X and biggest Y)
        sal_uInt32 getIndexOfOutmostPoint(const B2DPolygon& rCandidate)
        {
            sal_uInt32 nRetval(0L);

            if(rCandidate.count())
            {
                B2DPoint aOutmostPoint(rCandidate.getB2DPoint(0L));

                for(sal_uInt32 a(1L); a < rCandidate.count(); a++)
                {
                    B2DPoint rPoint(rCandidate.getB2DPoint(a));

                    if(fTools::more(rPoint.getX(), aOutmostPoint.getX()))
                    {
                        nRetval = a;
                        aOutmostPoint = rPoint;
                    }
                    else
                    {
                        if(fTools::more(rPoint.getY(), aOutmostPoint.getY()))
                        {
                            nRetval = a;
                            aOutmostPoint = rPoint;
                        }
                    }
                }
            }

            return nRetval;
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

        sal_uInt32 getIndexOfDifferentPredecessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate)
        {
            sal_uInt32 nNewIndex(nIndex);
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(rCandidate.count() > 1)
            {
                nNewIndex = getIndexOfPredecessor(nIndex, rCandidate);
                B2DPoint aPoint(rCandidate.getB2DPoint(nIndex));

                while(nNewIndex != nIndex
                    && aPoint.equal(rCandidate.getB2DPoint(nNewIndex)))
                {
                    nNewIndex = getIndexOfPredecessor(nNewIndex, rCandidate);
                }
            }

            return nNewIndex;
        }

        sal_uInt32 getIndexOfDifferentSuccessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate)
        {
            sal_uInt32 nNewIndex(nIndex);
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(rCandidate.count() > 1)
            {
                nNewIndex = getIndexOfSuccessor(nIndex, rCandidate);
                B2DPoint aPoint(rCandidate.getB2DPoint(nIndex));

                while(nNewIndex != nIndex
                    && aPoint.equal(rCandidate.getB2DPoint(nNewIndex)))
                {
                    nNewIndex = getIndexOfSuccessor(nNewIndex, rCandidate);
                }
            }

            return nNewIndex;
        }

        B2VectorOrientation getOrientation(const B2DPolygon& rCandidate)
        {
            B2VectorOrientation eRetval(ORIENTATION_NEUTRAL);

            if(rCandidate.count() > 2)
            {
                sal_uInt32 nIndex = getIndexOfOutmostPoint(rCandidate);
                eRetval = getPointOrientation(rCandidate, nIndex);
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

                        // call adaptive subdivide
                        ::basegfx::adaptiveSubdivideByDistance(aRetval, aBezier, fBound);
                    }
                    else
                    {
                        // no vectors used, add point
                        aRetval.append(rCandidate.getB2DPoint(a));
                    }
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

                        // generate AngleBound
                        double fBound(fAngleBound);

                        // make sure angle bound is not too small
                        if(fTools::less(fAngleBound, 0.1))
                        {
                            fAngleBound = 0.1;
                        }

                        // call adaptive subdivide
                        ::basegfx::adaptiveSubdivideByAngle(aRetval, aBezier, fBound);
                    }
                    else
                    {
                        // no vectors used, add point
                        aRetval.append(rCandidate.getB2DPoint(a));
                    }
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
            bool bRetval(false);
            const sal_uInt32 nPointCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPointCount; a++)
            {
                const B2DPoint aCurrentPoint(rCandidate.getB2DPoint(a));

                if(bWithBorder && aCurrentPoint.equal(rPoint))
                {
                    return true;
                }

                // cross-over in Y?
                const B2DPoint aPreviousPoint(rCandidate.getB2DPoint((!a) ? nPointCount - 1L : a - 1L));
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

                        if(bWithBorder && fTools::more(fCompare, rPoint.getX()))
                        {
                            bRetval = !bRetval;
                        }
                        else if(fTools::moreOrEqual(fCompare, rPoint.getX()))
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

        double getArea(const B2DPolygon& rCandidate)
        {
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
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");
            double fRetval(0.0);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nIndex < nPointCount)
            {
                if(rCandidate.isClosed() || nIndex + 1 != nPointCount)
                {
                    const sal_uInt32 nNextIndex(nIndex + 1 == nPointCount ? 0L : nIndex + 1L);
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
            // This method may also be implemented using a loop over getEdgeLength, but
            // since this would cause a lot of sqare roots to be solved it is much better
            // to sum up the quadrats first and then use a singe suare root (if necessary)
            double fRetval(0.0);
            const sal_uInt32 nPointCount(rCandidate.count());
            const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);

            for(sal_uInt32 a(0L); a < nLoopCount; a++)
            {
                const sal_uInt32 nNextIndex(a + 1 == nPointCount ? 0L : a + 1L);
                const B2DPoint aCurrentPoint(rCandidate.getB2DPoint(a));
                const B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                const B2DVector aVector(aNextPoint - aCurrentPoint);
                fRetval += aVector.scalar(aVector);
            }

            if(!fTools::equalZero(fRetval))
            {
                const double fOne(1.0);

                if(!fTools::equal(fOne, fRetval))
                {
                    fRetval = sqrt(fRetval);
                }
            }

            return fRetval;
        }

        B2DPoint getPositionAbsolute(const B2DPolygon& rCandidate, double fDistance, double fLength)
        {
            B2DPoint aRetval;
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1L)
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
                        nIndex = nPointCount - 1L;
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

                        if(fTools::moreOrEqual(fDistance, fEdgeLength))
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
                        fRelative = fDistance / fEdgeLength;
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

        B2VectorOrientation getPointOrientation(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");
            B2VectorOrientation eRetval(ORIENTATION_NEUTRAL);

            if(rCandidate.count() > 2)
            {
                sal_uInt32 nIndPrev = getIndexOfDifferentPredecessor(nIndex, rCandidate);

                if(nIndPrev != nIndex)
                {
                    sal_uInt32 nIndNext = getIndexOfDifferentSuccessor(nIndex, rCandidate);

                    if(nIndNext != nIndex && nIndNext != nIndPrev)
                    {
                        B2DPoint aPoint(rCandidate.getB2DPoint(nIndex));
                        B2DVector aVecPrev(rCandidate.getB2DPoint(nIndPrev) - aPoint);
                        B2DVector aVecNext(rCandidate.getB2DPoint(nIndNext) - aPoint);
                        eRetval = ::basegfx::getOrientation(aVecPrev, aVecNext);
                    }
                }
            }

            return eRetval;
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
            B2DPolyPolygon aRetval;

            if(0.0 == fFullDashDotLen && raDashDotArray.size())
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
                        if(nDashDotIndex % 2)
                        {
                            B2DPolygon aResult;

                            // add start point
                            if(fPosOnVector == 0.0)
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
                    if((fLength > 0.0) && (nDashDotIndex % 2))
                    {
                        B2DPolygon aResult;

                        // add start and end point
                        const B2DPoint aPosA(aStart + (aVector * fPosOnVector));
                        aResult.append(aPosA);
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
    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
