/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygontools.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:47:39 $
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
    namespace
    {
        class B2DExtraPolygonInfo
        {
            B2DRange                                maRange;
            sal_Int32                               mnDepth;
            B2VectorOrientation                     meOrinetation;

        public:
            B2DExtraPolygonInfo()
            {
            }

            void init(const B2DPolygon& rCandidate)
            {
                maRange = tools::getRange(rCandidate);
                meOrinetation = tools::getOrientation(rCandidate);
                mnDepth = (ORIENTATION_NEGATIVE == meOrinetation) ? -1L : 0L;
            }

            const B2DRange& getRange() const { return maRange; }
            B2VectorOrientation getOrientation() const { return meOrinetation; }

            sal_Int32 getDepth() const { return mnDepth; }

            void changeDepth(B2VectorOrientation eOrientation)
            {
                if(ORIENTATION_POSITIVE == eOrientation)
                {
                    mnDepth++;
                }
                else if(ORIENTATION_NEGATIVE == eOrientation)
                {
                    mnDepth--;
                }
            }
        };
    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // B2DPolyPolygon tools

//BFS08     void correctOrientations(B2DPolyPolygon& rCandidate)
        B2DPolyPolygon correctOrientations(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval;
            const sal_uInt32 nPolygonCount(rCandidate.count());
            sal_uInt32 nIndexOfOutmostPolygon(0L);
            bool bIndexOfOutmostPolygonSet(false);

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate = rCandidate.getB2DPolygon(a);

                if(aCandidate.count() > 2L)
                {
                    B2VectorOrientation aOrientation = tools::getOrientation(aCandidate);
                    bool bDoFlip(ORIENTATION_NEGATIVE == aOrientation);

                    // init values for depth and compare point for
                    // inside test. Since the ordering makes only sense when assuming
                    // that there are no intersections, the inside test is done with
                    // any point of the candidate, so teke the first one.
                    sal_uInt32 nDepth(0L);
                    const B2DPoint aTestPoint(aCandidate.getB2DPoint(0L));

                    // loop over other polygons and calculate depth
                    for(sal_uInt32 b(0L); b < nPolygonCount; b++)
                    {
                        if(b != a)
                        {
                            B2DPolygon aComparePolygon = rCandidate.getB2DPolygon(b);

                            if(tools::isInside(aComparePolygon, aTestPoint))
                            {
                                nDepth++;
                            }
                        }
                    }

                    // if nDepth is odd it is a hole
                    bool bIsHole(1L == (nDepth & 0x00000001));

                    // does polygon need to be flipped?
                    if((bDoFlip && !bIsHole) || (!bDoFlip && bIsHole))
                    {
                        aCandidate.flip();
                    }

                    // remember the index if it's the outmost polygon
                    if(!bIndexOfOutmostPolygonSet && 0L == nDepth)
                    {
                        bIndexOfOutmostPolygonSet = true;
                        nIndexOfOutmostPolygon = a;
                    }
                }

                // add to result
                aRetval.append(aCandidate);
            }

            // if the outmost polygon is not the first, move it in front
            if(bIndexOfOutmostPolygonSet && nIndexOfOutmostPolygon > 0L)
            {
                B2DPolygon aOutmostPolygon = rCandidate.getB2DPolygon(nIndexOfOutmostPolygon);
                aRetval.remove(nIndexOfOutmostPolygon);
                aRetval.insert(0L, aOutmostPolygon);
            }

            return aRetval;
        }

//BFS08     void removeIntersections(B2DPolyPolygon& rCandidate,
//BFS08         bool bForceOrientation, bool bInvertRemove)
//BFS08     {
//BFS08         B2DPolyPolygonCutter aCutter;
//BFS08
//BFS08         aCutter.addPolyPolygon(rCandidate, bForceOrientation);
//BFS08         aCutter.removeSelfIntersections();
//BFS08         aCutter.removeDoubleIntersections();
//BFS08         aCutter.removeIncludedPolygons(!bInvertRemove);
//BFS08         rCandidate.clear();
//BFS08         aCutter.getPolyPolygon(rCandidate);
//BFS08     }
        B2DPolyPolygon removeIntersections(const B2DPolyPolygon& rCandidate)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "removeIntersections: ATM works not for curves (!)");
            B2DPolyPolygon aRetval;

            if(rCandidate.count() > 1L)
            {
                B2DPolyPolygonCutter aCutter;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    B2DPolygon aCandidate = rCandidate.getB2DPolygon(a);
                    aCandidate.removeDoublePoints();
                    aRetval.append(aCandidate);
                }

                aCutter.addPolyPolygon(aRetval);
                aCutter.removeDoubleIntersections();

                aRetval = aCutter.getPolyPolygon();
            }
            else
            {
                aRetval = rCandidate;
            }

            return aRetval;
        }

        B2DPolyPolygon removeAllIntersections(const B2DPolyPolygon& rCandidate)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "removeAllIntersections: ATM works not for curves (!)");
            B2DPolyPolygon aRetval;

            if(rCandidate.count() > 1L)
            {
                B2DPolyPolygonCutter aCutter;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    B2DPolyPolygon aCandidate = removeIntersections(rCandidate.getB2DPolygon(a), true);
                    aRetval.append(aCandidate);
                }

                aCutter.addPolyPolygon(aRetval);
                aCutter.removeDoubleIntersections();

                aRetval = aCutter.getPolyPolygon();
            }
            else
            {
                aRetval = rCandidate;
            }

            return aRetval;
        }

        B2DPolyPolygon removeNeutralPolygons(const B2DPolyPolygon& rCandidate, bool bUseOr)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "removeNeutralOrientedPolygons: ATM works not for curves (!)");
            B2DPolyPolygon aRetval;
            B2DPolyPolygon aLocalCandidate;
            sal_uInt32 nCount(rCandidate.count());
            sal_uInt32 a;

            // sort out neutral polygons
            for(a = 0L; a < nCount; a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));
                const B2VectorOrientation aOrientation(getOrientation(aCandidate));

                if(ORIENTATION_NEUTRAL != aOrientation)
                {
                    aLocalCandidate.append(aCandidate);
                }
            }

            // get new count
            nCount = aLocalCandidate.count();

            if(nCount > 1L)
            {
                B2DExtraPolygonInfo* pPolygonInfos = new B2DExtraPolygonInfo[nCount];

                // initialize polygon infos
                for(a = 0L; a < nCount; a++)
                {
                    const B2DPolygon aCandidate(aLocalCandidate.getB2DPolygon(a));
                    pPolygonInfos[a].init(aCandidate);
                }

                // get all includes
                for(a = 0L; a < nCount; a++)
                {
                    B2DExtraPolygonInfo& rInfoA = pPolygonInfos[a];

                    for(sal_uInt32 b(0L); b < nCount; b++)
                    {
                        B2DExtraPolygonInfo& rInfoB = pPolygonInfos[b];

                        if(a != b && rInfoA.getRange().isInside(rInfoB.getRange()))
                        {
                            // volume B in A, test pA, pB for inclusion, with border
                            const B2DPolygon aCandidateA(aLocalCandidate.getB2DPolygon(a));
                            const B2DPolygon aCandidateB(aLocalCandidate.getB2DPolygon(b));

                            if(isInside(aCandidateA, aCandidateB, true))
                            {
                                // pB is inside pA
                                rInfoB.changeDepth(rInfoA.getOrientation());
                            }
                        }
                    }
                }

                // copy non-removables
                for(a = 0L; a < nCount; a++)
                {
                    B2DExtraPolygonInfo& rInfo = pPolygonInfos[a];

                    if(bUseOr)
                    {
                        if(rInfo.getDepth() == 0L)
                        {
                            const B2DPolygon aCandidate(aLocalCandidate.getB2DPolygon(a));
                            aRetval.append(aCandidate);
                        }
                    }
                    else
                    {
                        if(rInfo.getDepth() >= 1L)
                        {
                            const B2DPolygon aCandidate(aLocalCandidate.getB2DPolygon(a));
                            aRetval.append(aCandidate);
                        }
                    }
                }

                // delete infos
                delete[] pPolygonInfos;
            }
            else
            {
                aRetval = aLocalCandidate;
            }

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
                                for(sal_uInt32 a(1L); a < aNewCandidate.count(); a++)
                                {
                                    aMergePolygon.append(aNewCandidate.getB2DPoint(a));
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
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "isInEpsilonRange: ATM works not for curves (!)");
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                B2DPolygon aCandidate = rCandidate.getB2DPolygon(a);

                if(isInEpsilonRange(aCandidate, rTestPosition, fDistance))
                {
                    return true;
                }
            }

            return false;
        }

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof
