/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygoncutandtouch.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:46:07 $
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

#ifndef _BGFX_POLYGON_CUTANDTOUCH_HXX
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        class temporaryPoint
        {
            B2DPoint                            maPoint;
            sal_uInt32                          mnIndex;
            double                              mfQuadDistance;

        public:
            temporaryPoint(const B2DPoint& rOldPoint, const B2DPoint& rNewPoint, sal_uInt32 nIndex)
            :   maPoint(rNewPoint),
                mnIndex(nIndex)
            {
                const B2DVector aDist(rNewPoint - rOldPoint);
                mfQuadDistance = (aDist.getX() * aDist.getX() + aDist.getY() * aDist.getY());
            }

            bool operator<(const temporaryPoint& rComp) const
            {
                if(mnIndex == rComp.mnIndex)
                {
                    return (mfQuadDistance < rComp.mfQuadDistance);
                }

                return (mnIndex < rComp.mnIndex);
            }

            const B2DPoint& getPoint() const { return maPoint; }
            const sal_uInt32 getIndex() const { return mnIndex; }
        };

        ////////////////////////////////////////////////////////////////////////////////

        typedef ::std::vector< temporaryPoint > temporaryPointVector;

        ////////////////////////////////////////////////////////////////////////////////

        class temporaryPolygonData
        {
            B2DPolygon                              maPolygon;
            B2DRange                                maRange;
            temporaryPointVector                    maPoints;

        public:
            const B2DPolygon& getPolygon() const { return maPolygon; }
            void setPolygon(const B2DPolygon& rNew) { maPolygon = rNew; maRange = tools::getRange(maPolygon); }
            const B2DRange& getRange() const { return maRange; }
            temporaryPointVector& getTemporaryPointVector() { return maPoints; }
        };

        ////////////////////////////////////////////////////////////////////////////////

        void findCuts(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount > 3L)
            {
                const double fZero(0.0);
                const double fOne(1.0);
                B2DPoint aCurrentA(rCandidate.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < nCount - 1L; a++)
                {
                    // start edge is aCurrentA, aNextA
                    const B2DPoint aNextA(rCandidate.getB2DPoint(a + 1L));

                    if(!aCurrentA.equal(aNextA))
                    {
                        const B2DVector aVecA(aNextA - aCurrentA);
                        const B2DRange aRangeA(aCurrentA, aNextA);
                        B2DPoint aCurrentB(aNextA);

                        for(sal_uInt32 b(a + 1L); b < nCount; b++)
                        {
                            // compare edge is aCurrentB, aNextB
                            const sal_uInt32 nNextIndB(tools::getIndexOfSuccessor(b, rCandidate));
                            const B2DPoint aNextB(rCandidate.getB2DPoint(nNextIndB));

                            if(!aCurrentB.equal(aNextB))
                            {
                                const B2DRange aRangeB(aCurrentB, aNextB);

                                if(aRangeA.overlaps(aRangeB))
                                {
                                    if(!(aCurrentB.equal(aCurrentA) || aCurrentB.equal(aNextA) || aNextB.equal(aCurrentA) || aNextB.equal(aNextA)))
                                    {
                                        const B2DVector aVecB(aNextB - aCurrentB);
                                        double fCut(aVecA.cross(aVecB));

                                        if(!fTools::equalZero(fCut))
                                        {
                                            fCut = (aVecB.getY() * (aCurrentB.getX() - aCurrentA.getX()) + aVecB.getX() * (aCurrentA.getY() - aCurrentB.getY())) / fCut;

                                            if(fTools::more(fCut, fZero) && fTools::less(fCut, fOne))
                                            {
                                                // it's a candidate, but also need to test parameter value
                                                // of cut on line 2
                                                double fCut2;

                                                // choose the more precise version
                                                if(fabs(aVecB.getX()) > fabs(aVecB.getY()))
                                                {
                                                    fCut2 = (aCurrentA.getX() + (fCut * aVecA.getX()) - aCurrentB.getX()) / aVecB.getX();
                                                }
                                                else
                                                {
                                                    fCut2 = (aCurrentA.getY() + (fCut * aVecA.getY()) - aCurrentB.getY()) / aVecB.getY();
                                                }

                                                if(fTools::more(fCut2, fZero) && fTools::less(fCut2, fOne))
                                                {
                                                    B2DPoint aCutPoint(interpolate(aCurrentA, aNextA, fCut));
                                                    rTempPoints.push_back(temporaryPoint(aCurrentA, aCutPoint, a));
                                                    rTempPoints.push_back(temporaryPoint(aCurrentB, aCutPoint, b));
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // prepare next compare edge
                            aCurrentB = aNextB;
                        }
                    }

                    // prepare next start edge
                    aCurrentA = aNextA;
                }
            }
        }

        void findTouches(const B2DPolygon& rEdgePolygon, const B2DPolygon& rPointPolygon, temporaryPointVector& rTempPoints)
        {
            // find out if points from rPointPolygon touch edges from rEdgePolygon. If yes,
            // add entries to rTempPoints which should be added to rEdgePolygon
            const sal_uInt32 nEdgeCount(rEdgePolygon.count());
            const sal_uInt32 nPointCount(rPointPolygon.count());

            if(nEdgeCount && nPointCount)
            {
                const double fZero(0.0);
                const double fOne(1.0);
                B2DPoint aCurrent(rEdgePolygon.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                {
                    // edge is aCurrent, aNext
                    const sal_uInt32 nNextInd(tools::getIndexOfSuccessor(a, rEdgePolygon));
                    const B2DPoint aNext(rEdgePolygon.getB2DPoint(nNextInd));

                    if(!aNext.equal(aCurrent))
                    {
                        const B2DRange aRange(aCurrent, aNext);

                        if(aCurrent != aNext)
                        {
                            const B2DVector aEdgeVector(aNext - aCurrent);
                            bool bTestUsingX(fabs(aEdgeVector.getX()) > fabs(aEdgeVector.getY()));

                            for(sal_uInt32 b(0L); b < nPointCount; b++)
                            {
                                const B2DPoint aTestPoint(rPointPolygon.getB2DPoint(b));

                                if(aRange.isInside(aTestPoint))
                                {
                                    if(!(aTestPoint.equal(aCurrent) || aTestPoint.equal(aNext)))
                                    {
                                        const B2DVector aTestVector(aTestPoint - aCurrent);

                                        if(areParallel(aEdgeVector, aTestVector))
                                        {
                                            const double fParamTestOnCurr((bTestUsingX)
                                                ? aTestVector.getX() / aEdgeVector.getX()
                                                : aTestVector.getY() / aEdgeVector.getY());

                                            if(fTools::more(fParamTestOnCurr, fZero) && fTools::less(fParamTestOnCurr, fOne))
                                            {
                                                rTempPoints.push_back(temporaryPoint(aCurrent, aTestPoint, a));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // prepare next edge
                    aCurrent = aNext;
                }
            }
        }

        void findCuts(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            // find out if edges from both polygons cut. If so, add entries to rTempPoints which
            // should be added to the polygons accordingly
            const sal_uInt32 nCountA(rCandidateA.count());
            const sal_uInt32 nCountB(rCandidateB.count());

            if(nCountA && nCountB)
            {
                const double fZero(0.0);
                const double fOne(1.0);
                B2DPoint aCurrentA(rCandidateA.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < nCountA; a++)
                {
                    // edge A is aCurrentA, aNextA
                    const sal_uInt32 nNextIndA(tools::getIndexOfSuccessor(a, rCandidateA));
                    const B2DPoint aNextA(rCandidateA.getB2DPoint(nNextIndA));

                    if(!aCurrentA.equal(aNextA))
                    {
                        const B2DVector aVecA(aNextA - aCurrentA);
                        const B2DRange aRangeA(aCurrentA, aNextA);
                        B2DPoint aCurrentB(rCandidateB.getB2DPoint(0L));

                        for(sal_uInt32 b(0L); b < nCountB; b++)
                        {
                            // edge B is aCurrentB, aNextB
                            const sal_uInt32 nNextIndB(tools::getIndexOfSuccessor(b, rCandidateB));
                            const B2DPoint aNextB(rCandidateB.getB2DPoint(nNextIndB));

                            if(!aCurrentB.equal(aNextB))
                            {
                                const B2DRange aRangeB(aCurrentB, aNextB);

                                if(aRangeA.overlaps(aRangeB))
                                {
                                    if(!(aCurrentB.equal(aCurrentA) || aCurrentB.equal(aNextA) || aNextB.equal(aCurrentA) || aNextB.equal(aNextA)))
                                    {
                                        const B2DVector aVecB(aNextB - aCurrentB);
                                        double fCut(aVecA.cross(aVecB));

                                        if(!fTools::equalZero(fCut))
                                        {
                                            fCut = (aVecB.getY() * (aCurrentB.getX() - aCurrentA.getX()) + aVecB.getX() * (aCurrentA.getY() - aCurrentB.getY())) / fCut;

                                            if(fTools::more(fCut, fZero) && fTools::less(fCut, fOne))
                                            {
                                                // it's a candidate, but also need to test parameter value
                                                // of cut on line 2
                                                double fCut2;

                                                // choose the more precise version
                                                if(fabs(aVecB.getX()) > fabs(aVecB.getY()))
                                                {
                                                    fCut2 = (aCurrentA.getX() + (fCut * aVecA.getX()) - aCurrentB.getX()) / aVecB.getX();
                                                }
                                                else
                                                {
                                                    fCut2 = (aCurrentA.getY() + (fCut * aVecA.getY()) - aCurrentB.getY()) / aVecB.getY();
                                                }

                                                if(fTools::more(fCut2, fZero) && fTools::less(fCut2, fOne))
                                                {
                                                    B2DPoint aCutPoint(interpolate(aCurrentA, aNextA, fCut));
                                                    rTempPointsA.push_back(temporaryPoint(aCurrentA, aCutPoint, a));
                                                    rTempPointsB.push_back(temporaryPoint(aCurrentB, aCutPoint, b));
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // prepare next edge
                            aCurrentB = aNextB;
                        }
                    }

                    // prepare next edge
                    aCurrentA = aNextA;
                }
            }
        }

        B2DPolygon mergeTemporaryPointsAndPolygon(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints)
        {
            if(rTempPoints.size())
            {
                // add found cut and touch points
                B2DPolygon aRetval;
                sal_uInt32 nNewInd(0L);
                const sal_uInt32 nCount(rCandidate.count());

                // first sort by indices
                ::std::sort(rTempPoints.begin(), rTempPoints.end());

                // now merge new polygon by indices
                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    // first add original point
                    aRetval.append(rCandidate.getB2DPoint(a));

                    // now add all points targeted to be at this index
                    while(nNewInd < rTempPoints.size() && rTempPoints[nNewInd].getIndex() == a)
                    {
                        aRetval.append(rTempPoints[nNewInd++].getPoint());
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }
    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        B2DPolygon addPointsAtCutsAndTouches(const B2DPolygon& rCandidate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                OSL_ENSURE(false, "addPointsAtCutsAndTouches: works not for curves (!)");
                B2DPolygon aCandidate = tools::adaptiveSubdivideByAngle(rCandidate);
                return addPointsAtCutsAndTouches(aCandidate);
            }

            const sal_uInt32 nCount(rCandidate.count());
            temporaryPointVector aTempPoints;

            if(nCount > 2L)
            {
                findTouches(rCandidate, rCandidate, aTempPoints);
            }

            if(nCount > 3L)
            {
                findCuts(rCandidate, aTempPoints);
            }

            if(aTempPoints.size())
            {
                B2DPolygon aRetval(mergeTemporaryPointsAndPolygon(rCandidate, aTempPoints));

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rCandidate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                OSL_ENSURE(false, "addPointsAtCutsAndTouches: works not for curves (!)");
                B2DPolyPolygon aCandidate = tools::adaptiveSubdivideByAngle(rCandidate);
                return addPointsAtCutsAndTouches(aCandidate);
            }

            const sal_uInt32 nCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            if(1L == nCount)
            {
                // only one Polygon contained, solve self cuts and self touches
                const B2DPolygon aInput(rCandidate.getB2DPolygon(0L));
                const B2DPolygon aResult(addPointsAtCutsAndTouches(aInput));

                aRetval.append(aResult);
            }
            else
            {
                // first solve self cuts and self touches for all contained single polygons
                temporaryPolygonData *pTempData = new temporaryPolygonData[nCount];
                sal_uInt32 a, b;

                for(a = 0L; a < nCount; a++)
                {
                    pTempData[a].setPolygon(addPointsAtCutsAndTouches(rCandidate.getB2DPolygon(a)));
                }

                // now cuts and touches between the polygons
                for(a = 0L; a < nCount; a++)
                {
                    for(b = 0L; b < nCount; b++)
                    {
                        if(a != b)
                        {
                            // look for touches, compare each edge polygon to all other points
                            if(pTempData[a].getRange().overlaps(pTempData[b].getRange()))
                            {
                                findTouches(pTempData[a].getPolygon(), pTempData[b].getPolygon(), pTempData[a].getTemporaryPointVector());
                            }
                        }

                        if(a < b)
                        {
                            // look for cuts, compare each edge polygon to following ones
                            if(pTempData[a].getRange().overlaps(pTempData[b].getRange()))
                            {
                                findCuts(pTempData[a].getPolygon(), pTempData[b].getPolygon(), pTempData[a].getTemporaryPointVector(), pTempData[b].getTemporaryPointVector());
                            }
                        }
                    }
                }

                // consolidate the result
                for(a = 0L; a < nCount; a++)
                {
                    if(pTempData[a].getTemporaryPointVector().size())
                    {
                        aRetval.append(mergeTemporaryPointsAndPolygon(pTempData[a].getPolygon(), pTempData[a].getTemporaryPointVector()));
                    }
                    else
                    {
                        aRetval.append(pTempData[a].getPolygon());
                    }
                }

                delete[] pTempData;
            }

            return aRetval;
        }

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
