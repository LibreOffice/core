/*************************************************************************
 *
 *  $RCSfile: b2dpolygontools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-06 16:30:29 $
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

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        namespace tools
        {
            // B2DPolygon tools
            void checkClosed(polygon::B2DPolygon& rCandidate)
            {
                while(rCandidate.count() > 1L)
                {
                    sal_Bool bFirstLastPointEqual(rCandidate.getB2DPoint(0L).equal(rCandidate.getB2DPoint(rCandidate.count() - 1L)));

                    if(bFirstLastPointEqual)
                    {
                        rCandidate.setClosed(sal_True);
                        rCandidate.remove(rCandidate.count() - 1L);
                    }
                }
            }

            // Get index of outmost point (e.g. biggest X and biggest Y)
            sal_uInt32 getIndexOfOutmostPoint(const ::basegfx::polygon::B2DPolygon& rCandidate)
            {
                sal_uInt32 nRetval(0L);

                if(rCandidate.count())
                {
                    ::basegfx::point::B2DPoint aOutmostPoint(rCandidate.getB2DPoint(0L));

                    for(sal_uInt32 a(1L); a < rCandidate.count(); a++)
                    {
                        ::basegfx::point::B2DPoint rPoint(rCandidate.getB2DPoint(a));

                        if(::basegfx::numeric::fTools::more(rPoint.getX(), aOutmostPoint.getX()))
                        {
                            nRetval = a;
                            aOutmostPoint = rPoint;
                        }
                        else
                        {
                            if(::basegfx::numeric::fTools::more(rPoint.getY(), aOutmostPoint.getY()))
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
            sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const ::basegfx::polygon::B2DPolygon& rCandidate)
            {
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

            sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const ::basegfx::polygon::B2DPolygon& rCandidate)
            {
                if(nIndex + 1L < rCandidate.count())
                {
                    return nIndex + 1L;
                }
                else
                {
                    return 0L;
                }
            }

            sal_uInt32 getIndexOfDifferentPredecessor(sal_uInt32 nIndex, const ::basegfx::polygon::B2DPolygon& rCandidate)
            {
                if(rCandidate.count() > 1)
                {
                    sal_uInt32 nNewIndex = getIndexOfPredecessor(nIndex, rCandidate);
                    ::basegfx::point::B2DPoint aPoint(rCandidate.getB2DPoint(nIndex));

                    while(nNewIndex != nIndex
                        && aPoint.equal(rCandidate.getB2DPoint(nNewIndex)))
                    {
                        nNewIndex = getIndexOfPredecessor(nNewIndex, rCandidate);
                    }
                }

                return nIndex;
            }

            sal_uInt32 getIndexOfDifferentSuccessor(sal_uInt32 nIndex, const ::basegfx::polygon::B2DPolygon& rCandidate)
            {
                if(rCandidate.count() > 1)
                {
                    sal_uInt32 nNewIndex = getIndexOfSuccessor(nIndex, rCandidate);
                    ::basegfx::point::B2DPoint aPoint(rCandidate.getB2DPoint(nIndex));

                    while(nNewIndex != nIndex
                        && aPoint.equal(rCandidate.getB2DPoint(nNewIndex)))
                    {
                        nNewIndex = getIndexOfSuccessor(nNewIndex, rCandidate);
                    }
                }

                return nIndex;
            }

            ::basegfx::vector::B2DVectorOrientation getOrientation(const ::basegfx::polygon::B2DPolygon& rCandidate)
            {
                ::basegfx::vector::B2DVectorOrientation eRetval(::basegfx::vector::ORIENTATION_NEUTRAL);

                if(rCandidate.count() > 2)
                {
                    sal_uInt32 nIndex = getIndexOfOutmostPoint(rCandidate);
                    eRetval = getPointOrientation(rCandidate, nIndex);
                }

                return eRetval;
            }

            sal_Bool isInside(const ::basegfx::polygon::B2DPolygon& rCandidate, const ::basegfx::point::B2DPoint& rPoint)
            {
                sal_Bool bRetval(sal_False);
                const sal_uInt32 nPointCount(rCandidate.count());

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const ::basegfx::point::B2DPoint aCurrentPoint(rCandidate.getB2DPoint(a));
                    const ::basegfx::point::B2DPoint aPreviousPoint(rCandidate.getB2DPoint((!a) ? nPointCount - 1L : a - 1L));

                    // cross-over in Y?
                    const sal_Bool bCompYA(::basegfx::numeric::fTools::more(aPreviousPoint.getY(), rPoint.getY()));
                    const sal_Bool bCompYB(::basegfx::numeric::fTools::more(aCurrentPoint.getY(), rPoint.getY()));

                    if(bCompYA != bCompYB)
                    {
                        const sal_Bool bCompXA(::basegfx::numeric::fTools::more(aPreviousPoint.getX(), rPoint.getX()));
                        const sal_Bool bCompXB(::basegfx::numeric::fTools::more(aCurrentPoint.getX(), rPoint.getX()));

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

                            if(::basegfx::numeric::fTools::more(fCompare, rPoint.getX()))
                            {
                                bRetval = !bRetval;
                            }
                        }
                    }
                }

                return bRetval;
            }

            sal_Bool isInside(const ::basegfx::polygon::B2DPolygon& rCandidate, const ::basegfx::polygon::B2DPolygon& rPolygon)
            {
                const sal_uInt32 nPointCount(rPolygon.count());

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const ::basegfx::point::B2DPoint aTestPoint(rPolygon.getB2DPoint(a));

                    if(!isInside(rCandidate, aTestPoint))
                    {
                        return sal_False;
                    }
                }

                return sal_True;
            }

            ::basegfx::range::B2DRange getRange(const ::basegfx::polygon::B2DPolygon& rCandidate)
            {
                ::basegfx::range::B2DRange aRetval;
                const sal_uInt32 nPointCount(rCandidate.count());

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const ::basegfx::point::B2DPoint aTestPoint(rCandidate.getB2DPoint(a));
                    aRetval.expand(aTestPoint);
                }

                return aRetval;
            }

            double getArea(const ::basegfx::polygon::B2DPolygon& rCandidate)
            {
                double fRetval(0.0);
                const sal_uInt32 nPointCount(rCandidate.count());

                if(nPointCount > 2)
                {
                    for(sal_uInt32 a(0L); a < nPointCount; a++)
                    {
                        const ::basegfx::point::B2DPoint aPreviousPoint(rCandidate.getB2DPoint((!a) ? nPointCount - 1L : a - 1L));
                        const ::basegfx::point::B2DPoint aCurrentPoint(rCandidate.getB2DPoint(a));

                        fRetval += aPreviousPoint.getX() * aCurrentPoint.getY();
                        fRetval -= aPreviousPoint.getY() * aCurrentPoint.getX();
                    }

                    fRetval /= 2.0;

                    const double fZero(0.0);

                    if(::basegfx::numeric::fTools::less(fRetval, fZero))
                    {
                        fRetval = -fRetval;
                    }
                }

                return fRetval;
            }

            double getEdgeLength(const ::basegfx::polygon::B2DPolygon& rCandidate, sal_uInt32 nIndex)
            {
                double fRetval(0.0);
                const sal_uInt32 nPointCount(rCandidate.count());

                if(nIndex < nPointCount)
                {
                    if(rCandidate.isClosed() || nIndex + 1 != nPointCount)
                    {
                        const sal_uInt32 nNextIndex(nIndex + 1 == nPointCount ? 0L : nIndex + 1L);
                        const ::basegfx::point::B2DPoint aCurrentPoint(rCandidate.getB2DPoint(nIndex));
                        const ::basegfx::point::B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                        const ::basegfx::vector::B2DVector aVector(aNextPoint - aCurrentPoint);
                        fRetval = aVector.getLength();
                    }
                }

                return fRetval;
            }

            double getLength(const ::basegfx::polygon::B2DPolygon& rCandidate)
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
                    const ::basegfx::point::B2DPoint aCurrentPoint(rCandidate.getB2DPoint(a));
                    const ::basegfx::point::B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                    const ::basegfx::vector::B2DVector aVector(aNextPoint - aCurrentPoint);
                    fRetval += aVector.scalar(aVector);
                }

                if(!::basegfx::numeric::fTools::equalZero(fRetval))
                {
                    const double fOne(1.0);

                    if(!::basegfx::numeric::fTools::equal(fOne, fRetval))
                    {
                        fRetval = sqrt(fRetval);
                    }
                }

                return fRetval;
            }

            ::basegfx::point::B2DPoint getPositionAbsolute(const ::basegfx::polygon::B2DPolygon& rCandidate, double fDistance, double fLength)
            {
                ::basegfx::point::B2DPoint aRetval;
                const sal_uInt32 nPointCount(rCandidate.count());

                if(nPointCount > 1L)
                {
                    sal_uInt32 nIndex(0L);
                    sal_Bool bIndexDone(sal_False);
                    const double fZero(0.0);
                    double fEdgeLength(fZero);

                    // get length if not given
                    if(::basegfx::numeric::fTools::equalZero(fLength))
                    {
                        fLength = getLength(rCandidate);
                    }

                    // handle fDistance < 0.0
                    if(::basegfx::numeric::fTools::less(fDistance, fZero))
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
                            bIndexDone = sal_True;
                        }
                    }

                    // handle fDistance >= fLength
                    if(::basegfx::numeric::fTools::moreOrEqual(fDistance, fLength))
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
                            bIndexDone = sal_True;
                        }
                    }

                    // look for correct index. fDistance is now [0.0 .. fLength[
                    if(!bIndexDone)
                    {
                        do
                        {
                            // get length of next edge
                            fEdgeLength = getEdgeLength(rCandidate, nIndex);

                            if(::basegfx::numeric::fTools::moreOrEqual(fDistance, fEdgeLength))
                            {
                                // go to next edge
                                fDistance -= fEdgeLength;
                                nIndex++;
                            }
                            else
                            {
                                // it's on this edge, stop
                                bIndexDone = sal_True;
                            }
                        } while (!bIndexDone);
                    }

                    // get the point using nIndex
                    aRetval = rCandidate.getB2DPoint(nIndex);

                    // if fDistance != 0.0, move that length on the edge. The edge
                    // length is in fEdgeLength.
                    if(!::basegfx::numeric::fTools::equalZero(fDistance))
                    {
                        sal_uInt32 nNextIndex(getIndexOfSuccessor(nIndex, rCandidate));
                        const ::basegfx::point::B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                        double fRelative(fZero);

                        if(!::basegfx::numeric::fTools::equalZero(fEdgeLength))
                        {
                            fRelative = fDistance / fEdgeLength;
                        }

                        // add calculated average value to the return value
                        aRetval += ::basegfx::tuple::average(aRetval, aNextPoint, fRelative);
                    }
                }

                return aRetval;
            }

            ::basegfx::point::B2DPoint getPositionRelative(const ::basegfx::polygon::B2DPolygon& rCandidate, double fDistance, double fLength)
            {
                // get length if not given
                if(::basegfx::numeric::fTools::equalZero(fLength))
                {
                    fLength = getLength(rCandidate);
                }

                // multiply fDistance with real length to get absolute position and
                // use getPositionAbsolute
                return getPositionAbsolute(rCandidate, fDistance * fLength, fLength);
            }

            ::basegfx::vector::B2DVectorOrientation getPointOrientation(const ::basegfx::polygon::B2DPolygon& rCandidate, sal_uInt32 nIndex)
            {
                ::basegfx::vector::B2DVectorOrientation eRetval(::basegfx::vector::ORIENTATION_NEUTRAL);

                if(rCandidate.count() > 2)
                {
                    sal_uInt32 nIndPrev = getIndexOfDifferentPredecessor(nIndex, rCandidate);

                    if(nIndPrev != nIndex)
                    {
                        sal_uInt32 nIndNext = getIndexOfDifferentSuccessor(nIndex, rCandidate);

                        if(nIndNext != nIndex && nIndNext != nIndPrev)
                        {
                            ::basegfx::point::B2DPoint aPoint(rCandidate.getB2DPoint(nIndex));
                            ::basegfx::vector::B2DVector aVecPrev(rCandidate.getB2DPoint(nIndPrev) - aPoint);
                            ::basegfx::vector::B2DVector aVecNext(rCandidate.getB2DPoint(nIndNext) - aPoint);
                            eRetval = ::basegfx::vector::getOrientation(aVecPrev, aVecNext);
                        }
                    }
                }

                return eRetval;
            }

            CutFlagValue findCut(
                const ::basegfx::polygon::B2DPolygon& rCandidate,
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

                    const ::basegfx::point::B2DPoint aStart1(rCandidate.getB2DPoint(nIndex1));
                    const ::basegfx::point::B2DPoint aEnd1(rCandidate.getB2DPoint(nEnd1));
                    const ::basegfx::vector::B2DVector aVector1(aEnd1 - aStart1);

                    const ::basegfx::point::B2DPoint aStart2(rCandidate.getB2DPoint(nIndex2));
                    const ::basegfx::point::B2DPoint aEnd2(rCandidate.getB2DPoint(nEnd2));
                    const ::basegfx::vector::B2DVector aVector2(aEnd2 - aStart2);

                    aRetval = findCut(
                        aStart1, aVector1, aStart2, aVector2,
                        aCutFlags, pCut1, pCut2);
                }

                return aRetval;
            }

            CutFlagValue findCut(
                const ::basegfx::polygon::B2DPolygon& rCandidate1, sal_uInt32 nIndex1,
                const ::basegfx::polygon::B2DPolygon& rCandidate2, sal_uInt32 nIndex2,
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

                    const ::basegfx::point::B2DPoint aStart1(rCandidate1.getB2DPoint(nIndex1));
                    const ::basegfx::point::B2DPoint aEnd1(rCandidate1.getB2DPoint(nEnd1));
                    const ::basegfx::vector::B2DVector aVector1(aEnd1 - aStart1);

                    const ::basegfx::point::B2DPoint aStart2(rCandidate2.getB2DPoint(nIndex2));
                    const ::basegfx::point::B2DPoint aEnd2(rCandidate2.getB2DPoint(nEnd2));
                    const ::basegfx::vector::B2DVector aVector2(aEnd2 - aStart2);

                    aRetval = findCut(
                        aStart1, aVector1, aStart2, aVector2,
                        aCutFlags, pCut1, pCut2);
                }

                return aRetval;
            }

            CutFlagValue findCut(
                const ::basegfx::point::B2DPoint& rEdge1Start, const ::basegfx::vector::B2DVector& rEdge1Delta,
                const ::basegfx::point::B2DPoint& rEdge2Start, const ::basegfx::vector::B2DVector& rEdge2Delta,
                CutFlagValue aCutFlags,
                double* pCut1, double* pCut2)
            {
                CutFlagValue aRetval(CUTFLAG_NONE);
                double fCut1(0.0);
                double fCut2(0.0);
                sal_Bool bFinished(!((sal_Bool)(aCutFlags & CUTFLAG_ALL)));

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
                            bFinished = sal_True;
                            aRetval = (CUTFLAG_START1|CUTFLAG_START2);
                        }
                    }

                    // same endpoint?
                    if(!bFinished && (aCutFlags & (CUTFLAG_END1|CUTFLAG_END2)) == (CUTFLAG_END1|CUTFLAG_END2))
                    {
                        const ::basegfx::point::B2DPoint aEnd1(rEdge1Start + rEdge1Delta);
                        const ::basegfx::point::B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                        if(aEnd1.equal(aEnd2))
                        {
                            bFinished = sal_True;
                            aRetval = (CUTFLAG_END1|CUTFLAG_END2);
                            fCut1 = fCut2 = 1.0;
                        }
                    }

                    // startpoint1 == endpoint2?
                    if(!bFinished && (aCutFlags & (CUTFLAG_START1|CUTFLAG_END2)) == (CUTFLAG_START1|CUTFLAG_END2))
                    {
                        const ::basegfx::point::B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                        if(rEdge1Start.equal(aEnd2))
                        {
                            bFinished = sal_True;
                            aRetval = (CUTFLAG_START1|CUTFLAG_END2);
                            fCut1 = 0.0;
                            fCut2 = 1.0;
                        }
                    }

                    // startpoint2 == endpoint1?
                    if(!bFinished&& (aCutFlags & (CUTFLAG_START2|CUTFLAG_END1)) == (CUTFLAG_START2|CUTFLAG_END1))
                    {
                        const ::basegfx::point::B2DPoint aEnd1(rEdge1Start + rEdge1Delta);

                        if(rEdge2Start.equal(aEnd1))
                        {
                            bFinished = sal_True;
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
                            bFinished = sal_True;
                            aRetval = (CUTFLAG_LINE|CUTFLAG_START1);
                        }
                    }

                    if(!bFinished && (aCutFlags & CUTFLAG_START2))
                    {
                        // start2 on line 1 ?
                        if(isPointOnEdge(rEdge2Start, rEdge1Start, rEdge1Delta, &fCut1))
                        {
                            bFinished = sal_True;
                            aRetval = (CUTFLAG_LINE|CUTFLAG_START2);
                        }
                    }

                    if(!bFinished && (aCutFlags & CUTFLAG_END1))
                    {
                        // end1 on line 2 ?
                        const ::basegfx::point::B2DPoint aEnd1(rEdge1Start + rEdge1Delta);

                        if(isPointOnEdge(aEnd1, rEdge2Start, rEdge2Delta, &fCut2))
                        {
                            bFinished = sal_True;
                            aRetval = (CUTFLAG_LINE|CUTFLAG_END1);
                        }
                    }

                    if(!bFinished && (aCutFlags & CUTFLAG_END2))
                    {
                        // end2 on line 1 ?
                        const ::basegfx::point::B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                        if(isPointOnEdge(aEnd2, rEdge1Start, rEdge1Delta, &fCut1))
                        {
                            bFinished = sal_True;
                            aRetval = (CUTFLAG_LINE|CUTFLAG_END2);
                        }
                    }

                    if(!bFinished)
                    {
                        // cut in line1, line2 ?
                        fCut1 = (rEdge1Delta.getX() * rEdge2Delta.getY()) - (rEdge1Delta.getY() * rEdge2Delta.getX());

                        if(!::basegfx::numeric::fTools::equalZero(fCut1))
                        {
                            fCut1 = (rEdge2Delta.getY() * (rEdge2Start.getX() - rEdge1Start.getX())
                                + rEdge2Delta.getX() * (rEdge1Start.getY() - rEdge2Start.getY())) / fCut1;

                            const double fZero(0.0);
                            const double fOne(1.0);

                            // inside parameter range edge1 AND fCut2 is calcable
                            if(::basegfx::numeric::fTools::more(fCut1, fZero) && ::basegfx::numeric::fTools::less(fCut1, fOne)
                                && (!::basegfx::numeric::fTools::equalZero(rEdge2Delta.getX()) || !::basegfx::numeric::fTools::equalZero(rEdge2Delta.getY())))
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
                                if(::basegfx::numeric::fTools::more(fCut2, fZero) && ::basegfx::numeric::fTools::less(fCut2, fOne))
                                {
                                    bFinished = sal_True;
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

            sal_Bool isPointOnEdge(
                const ::basegfx::point::B2DPoint& rPoint,
                const ::basegfx::point::B2DPoint& rEdgeStart,
                const ::basegfx::vector::B2DVector& rEdgeDelta,
                double* pCut)
            {
                sal_Bool bDeltaXIsZero(::basegfx::numeric::fTools::equalZero(rEdgeDelta.getX()));
                sal_Bool bDeltaYIsZero(::basegfx::numeric::fTools::equalZero(rEdgeDelta.getY()));
                const double fZero(0.0);
                const double fOne(1.0);

                if(bDeltaXIsZero && bDeltaYIsZero)
                {
                    // no line, just a point
                    return sal_False;
                }
                else if(bDeltaXIsZero)
                {
                    // vertical line
                    if(::basegfx::numeric::fTools::equal(rPoint.getX(), rEdgeStart.getX()))
                    {
                        double fValue = (rPoint.getY() - rEdgeStart.getY()) / rEdgeDelta.getY();

                        if(::basegfx::numeric::fTools::more(fValue, fZero) && ::basegfx::numeric::fTools::less(fValue, fOne))
                        {
                            if(pCut)
                            {
                                *pCut = fValue;
                            }

                            return sal_True;
                        }
                    }
                }
                else if(bDeltaYIsZero)
                {
                    // horizontal line
                    if(::basegfx::numeric::fTools::equal(rPoint.getY(), rEdgeStart.getY()))
                    {
                        double fValue = (rPoint.getX() - rEdgeStart.getX()) / rEdgeDelta.getX();

                        if(::basegfx::numeric::fTools::more(fValue, fZero)
                            && ::basegfx::numeric::fTools::less(fValue, fOne))
                        {
                            if(pCut)
                            {
                                *pCut = fValue;
                            }

                            return sal_True;
                        }
                    }
                }
                else
                {
                    // any angle line
                    double fTOne = (rPoint.getX() - rEdgeStart.getX()) / rEdgeDelta.getX();
                    double fTTwo = (rPoint.getY() - rEdgeStart.getY()) / rEdgeDelta.getY();

                    if(::basegfx::numeric::fTools::equal(fTOne, fTTwo))
                    {
                        // same parameter representation, point is on line. Take
                        // middle value for better results
                        double fValue = (fTOne + fTTwo) / 2.0;

                        if(::basegfx::numeric::fTools::more(fValue, fZero) && ::basegfx::numeric::fTools::less(fValue, fOne))
                        {
                            // point is inside line bounds, too
                            if(pCut)
                            {
                                *pCut = fValue;
                            }

                            return sal_True;
                        }
                    }
                }

                return sal_False;
            }
        } // end of namespace tools
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof
