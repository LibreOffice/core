/*************************************************************************
 *
 *  $RCSfile: b3dpolygontools.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-26 14:31:39 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_RANGE_B3DRANGE_HXX
#include <basegfx/range/b3drange.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        namespace tools
        {
            // B3DPolygon tools
            void checkClosed(polygon::B3DPolygon& rCandidate)
            {
                while(rCandidate.count() > 1L
                    && rCandidate.getB3DPoint(0L).equal(rCandidate.getB3DPoint(rCandidate.count() - 1L)))
                {
                    rCandidate.setClosed(sal_True);
                    rCandidate.remove(rCandidate.count() - 1L);
                }
            }

            // Get successor and predecessor indices. Returning the same index means there
            // is none. Same for successor.
            sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const ::basegfx::polygon::B3DPolygon& rCandidate)
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

            sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const ::basegfx::polygon::B3DPolygon& rCandidate)
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

            sal_uInt32 getIndexOfDifferentPredecessor(sal_uInt32 nIndex, const ::basegfx::polygon::B3DPolygon& rCandidate)
            {
                sal_uInt32 nNewIndex(nIndex);
                OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

                if(rCandidate.count() > 1)
                {
                    nNewIndex = getIndexOfPredecessor(nIndex, rCandidate);
                    ::basegfx::point::B3DPoint aPoint(rCandidate.getB3DPoint(nIndex));

                    while(nNewIndex != nIndex
                        && aPoint.equal(rCandidate.getB3DPoint(nNewIndex)))
                    {
                        nNewIndex = getIndexOfPredecessor(nNewIndex, rCandidate);
                    }
                }

                return nNewIndex;
            }

            sal_uInt32 getIndexOfDifferentSuccessor(sal_uInt32 nIndex, const ::basegfx::polygon::B3DPolygon& rCandidate)
            {
                sal_uInt32 nNewIndex(nIndex);
                OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

                if(rCandidate.count() > 1)
                {
                    nNewIndex = getIndexOfSuccessor(nIndex, rCandidate);
                    ::basegfx::point::B3DPoint aPoint(rCandidate.getB3DPoint(nIndex));

                    while(nNewIndex != nIndex
                        && aPoint.equal(rCandidate.getB3DPoint(nNewIndex)))
                    {
                        nNewIndex = getIndexOfSuccessor(nNewIndex, rCandidate);
                    }
                }

                return nNewIndex;
            }

            ::basegfx::range::B3DRange getRange(const ::basegfx::polygon::B3DPolygon& rCandidate)
            {
                ::basegfx::range::B3DRange aRetval;
                const sal_uInt32 nPointCount(rCandidate.count());

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const ::basegfx::point::B3DPoint aTestPoint(rCandidate.getB3DPoint(a));
                    aRetval.expand(aTestPoint);
                }

                return aRetval;
            }

            double getEdgeLength(const ::basegfx::polygon::B3DPolygon& rCandidate, sal_uInt32 nIndex)
            {
                OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");
                double fRetval(0.0);
                const sal_uInt32 nPointCount(rCandidate.count());

                if(nIndex < nPointCount)
                {
                    if(rCandidate.isClosed() || nIndex + 1 != nPointCount)
                    {
                        const sal_uInt32 nNextIndex(nIndex + 1 == nPointCount ? 0L : nIndex + 1L);
                        const ::basegfx::point::B3DPoint aCurrentPoint(rCandidate.getB3DPoint(nIndex));
                        const ::basegfx::point::B3DPoint aNextPoint(rCandidate.getB3DPoint(nNextIndex));
                        const ::basegfx::vector::B3DVector aVector(aNextPoint - aCurrentPoint);
                        fRetval = aVector.getLength();
                    }
                }

                return fRetval;
            }

            double getLength(const ::basegfx::polygon::B3DPolygon& rCandidate)
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
                    const ::basegfx::point::B3DPoint aCurrentPoint(rCandidate.getB3DPoint(a));
                    const ::basegfx::point::B3DPoint aNextPoint(rCandidate.getB3DPoint(nNextIndex));
                    const ::basegfx::vector::B3DVector aVector(aNextPoint - aCurrentPoint);
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

            ::basegfx::point::B3DPoint getPositionAbsolute(const ::basegfx::polygon::B3DPolygon& rCandidate, double fDistance, double fLength)
            {
                ::basegfx::point::B3DPoint aRetval;
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
                    aRetval = rCandidate.getB3DPoint(nIndex);

                    // if fDistance != 0.0, move that length on the edge. The edge
                    // length is in fEdgeLength.
                    if(!::basegfx::numeric::fTools::equalZero(fDistance))
                    {
                        sal_uInt32 nNextIndex(getIndexOfSuccessor(nIndex, rCandidate));
                        const ::basegfx::point::B3DPoint aNextPoint(rCandidate.getB3DPoint(nNextIndex));
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

            ::basegfx::point::B3DPoint getPositionRelative(const ::basegfx::polygon::B3DPolygon& rCandidate, double fDistance, double fLength)
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

        } // end of namespace tools
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof
