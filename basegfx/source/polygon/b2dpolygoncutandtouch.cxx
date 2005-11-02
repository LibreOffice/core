/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygoncutandtouch.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:57:57 $
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

#ifndef _BGFX_CURVE_B2DCUBICBEZIER_HXX
#include <basegfx/curve/b2dcubicbezier.hxx>
#endif

#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////
// defines

#define SUBDIVIDE_FOR_CUT_TEST_COUNT        (50)

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        ////////////////////////////////////////////////////////////////////////////////

        class temporaryPoint
        {
            B2DPoint                            maPoint;        // the new point
            sal_uInt32                          mnIndex;        // index after which to insert
            double                              mfCut;          // parametric cut description [0.0 .. 1.0]

        public:
            temporaryPoint(const B2DPoint& rNewPoint, sal_uInt32 nIndex, double fCut)
            :   maPoint(rNewPoint),
                mnIndex(nIndex),
                mfCut(fCut)
            {
            }

            bool operator<(const temporaryPoint& rComp) const
            {
                if(mnIndex == rComp.mnIndex)
                {
                    return (mfCut < rComp.mfCut);
                }

                return (mnIndex < rComp.mnIndex);
            }

            const B2DPoint& getPoint() const { return maPoint; }
            sal_uInt32 getIndex() const { return mnIndex; }
            double getCut() const { return mfCut; }
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

        B2DPolygon mergeTemporaryPointsAndPolygon(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints)
        {
            if(rTempPoints.size())
            {
                B2DPolygon aRetval;
                sal_uInt32 nNewInd(0L);
                const sal_uInt32 nCount(rCandidate.count());
                const bool bCurvesInvolved(rCandidate.areControlPointsUsed());

                // sort by indices to assure increasing fCut values and increasing indices
                ::std::sort(rTempPoints.begin(), rTempPoints.end());

                // add found cut and touch points
                if(bCurvesInvolved)
                {
                    // merge new polygon by indices
                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        // get cubic bezier snippet
                        B2DCubicBezier aCubicBezier(
                            rCandidate.getB2DPoint(a),
                            rCandidate.getControlVectorA(a),
                            rCandidate.getControlVectorB(a),
                            rCandidate.getB2DPoint(a + 1L == nCount ? 0L : a + 1L));
                        double fLeftStart(0.0);

                        // add original point
                        aRetval.append(aCubicBezier.getStartPoint());

                        if(aCubicBezier.isBezier())
                        {
                            // if really bezier, copy control vectors to added point, too
                            const sal_uInt32 nRetvalIndex(aRetval.count() - 1L);
                            aRetval.setControlPointA(nRetvalIndex, aCubicBezier.getControlPointA());
                            aRetval.setControlPointB(nRetvalIndex, aCubicBezier.getControlPointB());
                        }

                        // now add all points targeted to be at this index
                        while(nNewInd < rTempPoints.size() && rTempPoints[nNewInd].getIndex() == a)
                        {
                            const temporaryPoint& rTempPoint = rTempPoints[nNewInd++];

                            // split curve segment. Splits need to come sorted and need to be < 1.0. Also,
                            // since original segment is consumed from left to right, the cut values need
                            // to be scaled to the remaining part
                            B2DCubicBezier aLeftPart;
                            const double fRelativeSplitPoint((rTempPoint.getCut() - fLeftStart) / (1.0 - fLeftStart));
                            aCubicBezier.split(fRelativeSplitPoint, aLeftPart, aCubicBezier);
                            fLeftStart = rTempPoint.getCut();

                            // correct vectors on last added point
                            const sal_uInt32 nRetvalCount(aRetval.count());
                            aRetval.setControlPointA(nRetvalCount - 1L, aLeftPart.getControlPointA());
                            aRetval.setControlPointB(nRetvalCount - 1L, aLeftPart.getControlPointB());

                            // append new point, use point from rTempPoint for numerical reasons. This
                            // will guarantee the usage of exactly the same point in different curves
                            aRetval.append(rTempPoint.getPoint());

                            // set new vectors for newly added point
                            aRetval.setControlPointA(nRetvalCount, aCubicBezier.getControlPointA());
                            aRetval.setControlPointB(nRetvalCount, aCubicBezier.getControlPointB());
                        }
                    }
                }
                else
                {
                    // merge new polygon by indices
                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        // add original point
                        aRetval.append(rCandidate.getB2DPoint(a));

                        // add all points targeted to be at this index
                        while(nNewInd < rTempPoints.size() && rTempPoints[nNewInd].getIndex() == a)
                        {
                            const temporaryPoint& rTempPoint = rTempPoints[nNewInd++];
                            const B2DPoint aNewPoint(rTempPoint.getPoint());

                            // do not add points double
                            if(!aRetval.getB2DPoint(aRetval.count() - 1L).equal(aNewPoint))
                            {
                                aRetval.append(aNewPoint);
                            }
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

        ////////////////////////////////////////////////////////////////////////////////

        void adaptAndTransferCutsWithBezierSegment(
            const temporaryPointVector& rPointVector, const B2DPolygon& rPolygon,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            // assuming that the subdivision to create rPolygon used aequidistant pieces
            // (as in adaptiveSubdivideByCount) it is now possible to calculate back the
            // cut positions in the polygon to relative cut positions on the original bezier
            // segment.
            const sal_uInt32 nTempPointCount(rPointVector.size());
            const sal_uInt32 nEdgeCount(rPolygon.count() ? rPolygon.count() - 1L : 0L);

            if(nTempPointCount && nEdgeCount)
            {
                for(sal_uInt32 a(0L); a < nTempPointCount; a++)
                {
                    const temporaryPoint& rTempPoint = rPointVector[a];
                    const double fCutPosInPolygon((double)rTempPoint.getIndex() + rTempPoint.getCut());
                    const double fRelativeCutPos(fCutPosInPolygon / (double)nEdgeCount);
                    rTempPoints.push_back(temporaryPoint(rTempPoint.getPoint(), nInd, fRelativeCutPos));
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        ////////////////////////////////////////////////////////////////////////////////
        // predefines for calls to this methods before method implementation

        void findCuts(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints);
        void findTouches(const B2DPolygon& rEdgePolygon, const B2DPolygon& rPointPolygon, temporaryPointVector& rTempPoints);
        void findCuts(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB);

        ////////////////////////////////////////////////////////////////////////////////

        void findEdgeCutsTwoEdges(
            const B2DPoint& rCurrA, const B2DPoint& rNextA,
            const B2DPoint& rCurrB, const B2DPoint& rNextB,
            sal_uInt32 nIndA, sal_uInt32 nIndB,
            temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            // no null length edges
            if(!(rCurrA.equal(rNextA) || rCurrB.equal(rNextB)))
            {
                // no common start/end points, this can be no cuts
                if(!(rCurrB.equal(rCurrA) || rCurrB.equal(rNextA) || rNextB.equal(rCurrA) || rNextB.equal(rNextA)))
                {
                    const B2DVector aVecA(rNextA - rCurrA);
                    const B2DVector aVecB(rNextB - rCurrB);
                    double fCut(aVecA.cross(aVecB));

                    if(!fTools::equalZero(fCut))
                    {
                        const double fZero(0.0);
                        const double fOne(1.0);
                        fCut = (aVecB.getY() * (rCurrB.getX() - rCurrA.getX()) + aVecB.getX() * (rCurrA.getY() - rCurrB.getY())) / fCut;

                        if(fTools::more(fCut, fZero) && fTools::less(fCut, fOne))
                        {
                            // it's a candidate, but also need to test parameter value of cut on line 2
                            double fCut2;

                            // choose the more precise version
                            if(fabs(aVecB.getX()) > fabs(aVecB.getY()))
                            {
                                fCut2 = (rCurrA.getX() + (fCut * aVecA.getX()) - rCurrB.getX()) / aVecB.getX();
                            }
                            else
                            {
                                fCut2 = (rCurrA.getY() + (fCut * aVecA.getY()) - rCurrB.getY()) / aVecB.getY();
                            }

                            if(fTools::more(fCut2, fZero) && fTools::less(fCut2, fOne))
                            {
                                // cut is in range, add point. Two edges can have only one cut, but
                                // add a cut point to each list. The lists may be the same for
                                // self intersections.
                                const B2DPoint aCutPoint(interpolate(rCurrA, rNextA, fCut));
                                rTempPointsA.push_back(temporaryPoint(aCutPoint, nIndA, fCut));
                                rTempPointsB.push_back(temporaryPoint(aCutPoint, nIndB, fCut2));
                            }
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        void findEdgeCutsBezierAndEdge(
            const B2DCubicBezier& rCubicA,
            const B2DPoint& rCurrB, const B2DPoint& rNextB,
            sal_uInt32 nIndA, sal_uInt32 nIndB,
            temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            // find all cuts between given bezier segment and edge. Add an entry to the tempPoints
            // for each common point with the cut value describing the relative position on given
            // bezier segment and edge.
            B2DPolygon aTempPolygonA;
            B2DPolygon aTempPolygonEdge;
            temporaryPointVector aTempPointVectorA;
            temporaryPointVector aTempPointVectorEdge;

            // create subdivided polygons and find cuts between them
            rCubicA.adaptiveSubdivideByCount(aTempPolygonA, SUBDIVIDE_FOR_CUT_TEST_COUNT, true);
            aTempPolygonEdge.append(rCurrB);
            aTempPolygonEdge.append(rNextB);
            findCuts(aTempPolygonA, aTempPolygonEdge, aTempPointVectorA, aTempPointVectorEdge);

            if(aTempPointVectorA.size())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorA, aTempPolygonA, nIndA, rTempPointsA);
            }

            // append remapped tempVector entries for edge to tempPoints for edge
            for(sal_uInt32 a(0L); a < aTempPointVectorEdge.size(); a++)
            {
                const temporaryPoint& rTempPoint = aTempPointVectorEdge[a];
                rTempPointsB.push_back(temporaryPoint(rTempPoint.getPoint(), nIndB, rTempPoint.getCut()));
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        void findEdgeCutsTwoBeziers(
            const B2DCubicBezier& rCubicA,
            const B2DCubicBezier& rCubicB,
            sal_uInt32 nIndA, sal_uInt32 nIndB,
            temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            // find all cuts between the two given bezier segments. Add an entry to the tempPoints
            // for each common point with the cut value describing the relative position on given
            // bezier segments.
            B2DPolygon aTempPolygonA;
            B2DPolygon aTempPolygonB;
            temporaryPointVector aTempPointVectorA;
            temporaryPointVector aTempPointVectorB;

            // create subdivided polygons and find cuts between them
            rCubicA.adaptiveSubdivideByCount(aTempPolygonA, SUBDIVIDE_FOR_CUT_TEST_COUNT, true);
            rCubicB.adaptiveSubdivideByCount(aTempPolygonB, SUBDIVIDE_FOR_CUT_TEST_COUNT, true);
            findCuts(aTempPolygonA, aTempPolygonB, aTempPointVectorA, aTempPointVectorB);

            if(aTempPointVectorA.size())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorA, aTempPolygonA, nIndA, rTempPointsA);
            }

            if(aTempPointVectorB.size())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorB, aTempPolygonB, nIndB, rTempPointsB);
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        void findEdgeCutsOneBezier(
            const B2DCubicBezier& rCubicA,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            // find all self-intersections on the given bezier segment. Add an entry to the tempPoints
            // for each self intersection point with the cut value describing the relative position on given
            // bezier segment.
            B2DPolygon aTempPolygon;
            temporaryPointVector aTempPointVector;

            // create subdivided polygon and find cuts on it
            rCubicA.adaptiveSubdivideByCount(aTempPolygon, SUBDIVIDE_FOR_CUT_TEST_COUNT, true);
            findCuts(aTempPolygon, aTempPointVector);

            if(aTempPointVector.size())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVector, aTempPolygon, nInd, rTempPoints);
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        void findCuts(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints)
        {
            // find out if there are edges with intersections (self-cuts). If yes, add
            // entries to rTempPoints accordingly
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);

                if(nEdgeCount)
                {
                    const bool bCurvesInvolved(rCandidate.areControlPointsUsed());

                    if(bCurvesInvolved)
                    {
                        for(sal_uInt32 a(0L); a < nEdgeCount - 1L; a++)
                        {
                            const B2DCubicBezier aCubicA(
                                rCandidate.getB2DPoint(a),
                                rCandidate.getControlVectorA(a),
                                rCandidate.getControlVectorB(a),
                                rCandidate.getB2DPoint(a + 1L == nPointCount ? 0L : a + 1L));
                            const bool bEdgeAIsCurve(aCubicA.isBezier());
                            const B2DRange aRangeA(aCubicA.getRange());

                            for(sal_uInt32 b(a + 1L); b < nEdgeCount; b++)
                            {
                                const B2DCubicBezier aCubicB(
                                    rCandidate.getB2DPoint(b),
                                    rCandidate.getControlVectorA(b),
                                    rCandidate.getControlVectorB(b),
                                    rCandidate.getB2DPoint(b + 1L == nPointCount ? 0L : b + 1L));
                                const bool bEdgeBIsCurve(aCubicB.isBezier());
                                const B2DRange aRangeB(aCubicB.getRange());

                                if(aRangeA.overlaps(aRangeB))
                                {
                                    if(bEdgeAIsCurve && bEdgeBIsCurve)
                                    {
                                        // test for bezier-bezier cuts
                                        findEdgeCutsTwoBeziers(aCubicA, aCubicB, a, b, rTempPoints, rTempPoints);
                                    }
                                    else if(bEdgeAIsCurve)
                                    {
                                        // test for bezier-edge cuts
                                        findEdgeCutsBezierAndEdge(aCubicA, aCubicB.getStartPoint(), aCubicB.getEndPoint(), a, b, rTempPoints, rTempPoints);
                                    }
                                    else if(bEdgeBIsCurve)
                                    {
                                        // test for bezier-edge cuts
                                        findEdgeCutsBezierAndEdge(aCubicB, aCubicA.getStartPoint(), aCubicA.getEndPoint(), b, a, rTempPoints, rTempPoints);
                                    }
                                    else
                                    {
                                        // test for simple edge-edge cuts
                                        findEdgeCutsTwoEdges(aCubicA.getStartPoint(), aCubicA.getEndPoint(), aCubicB.getStartPoint(), aCubicB.getEndPoint(),
                                            a, b, rTempPoints, rTempPoints);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        B2DPoint aCurrA(rCandidate.getB2DPoint(0L));

                        for(sal_uInt32 a(0L); a < nEdgeCount - 1L; a++)
                        {
                            const B2DPoint aNextA(rCandidate.getB2DPoint(a + 1L == nPointCount ? 0L : a + 1L));
                            const B2DRange aRangeA(aCurrA, aNextA);
                            B2DPoint aCurrB(rCandidate.getB2DPoint(a + 1L));

                            for(sal_uInt32 b(a + 1L); b < nEdgeCount; b++)
                            {
                                const B2DPoint aNextB(rCandidate.getB2DPoint(b + 1L == nPointCount ? 0L : b + 1L));
                                const B2DRange aRangeB(aCurrB, aNextB);

                                if(aRangeA.overlaps(aRangeB))
                                {
                                    findEdgeCutsTwoEdges(aCurrA, aNextA, aCurrB, aNextB, a, b, rTempPoints, rTempPoints);
                                }

                                // prepare next step
                                aCurrB = aNextB;
                            }

                            // prepare next step
                            aCurrA = aNextA;
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        ////////////////////////////////////////////////////////////////////////////////

        void findTouchesOnEdge(
            const B2DPoint& rCurr, const B2DPoint& rNext, const B2DPolygon& rPointPolygon,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            // find out if points from rPointPolygon are positioned on given edge. If Yes, add
            // points there to represent touches (which may be enter or leave nodes later).
            const sal_uInt32 nPointCount(rPointPolygon.count());

            if(nPointCount)
            {
                const B2DRange aRange(rCurr, rNext);
                const B2DVector aEdgeVector(rNext - rCurr);
                bool bTestUsingX(fabs(aEdgeVector.getX()) > fabs(aEdgeVector.getY()));

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aTestPoint(rPointPolygon.getB2DPoint(a));

                    if(aRange.isInside(aTestPoint))
                    {
                        if(!aTestPoint.equal(rCurr) && !aTestPoint.equal(rNext))
                        {
                            const B2DVector aTestVector(aTestPoint - rCurr);

                            if(areParallel(aEdgeVector, aTestVector))
                            {
                                const double fCut((bTestUsingX)
                                    ? aTestVector.getX() / aEdgeVector.getX()
                                    : aTestVector.getY() / aEdgeVector.getY());
                                const double fZero(0.0);
                                const double fOne(1.0);

                                if(fTools::more(fCut, fZero) && fTools::less(fCut, fOne))
                                {
                                    rTempPoints.push_back(temporaryPoint(aTestPoint, nInd, fCut));
                                }
                            }
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        void findTouchesOnCurve(
            const B2DCubicBezier& rCubicA, const B2DPolygon& rPointPolygon,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            // find all points from rPointPolygon which touch the given bezier segment. Add an entry
            // for each touch to the given pointVector. The cut for that entry is the relative position on
            // the given bezier segment.
            B2DPolygon aTempPolygon;
            temporaryPointVector aTempPointVector;

            // create subdivided polygon and find cuts on it
            rCubicA.adaptiveSubdivideByCount(aTempPolygon, SUBDIVIDE_FOR_CUT_TEST_COUNT, true);
            findTouches(aTempPolygon, rPointPolygon, aTempPointVector);

            if(aTempPointVector.size())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVector, aTempPolygon, nInd, rTempPoints);
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        void findTouches(const B2DPolygon& rEdgePolygon, const B2DPolygon& rPointPolygon, temporaryPointVector& rTempPoints)
        {
            // find out if points from rPointPolygon touch edges from rEdgePolygon. If yes,
            // add entries to rTempPoints
            const sal_uInt32 nPointCount(rPointPolygon.count());
            const sal_uInt32 nEdgePointCount(rEdgePolygon.count());

            if(nPointCount && nEdgePointCount)
            {
                const sal_uInt32 nEdgeCount(rEdgePolygon.isClosed() ? nEdgePointCount : nEdgePointCount - 1L);

                for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                {
                    const B2DPoint aCurr(rEdgePolygon.getB2DPoint(a));
                    const B2DPoint aNext(rEdgePolygon.getB2DPoint(a + 1L == nEdgePointCount ? 0L : a + 1L));

                    if(!aCurr.equal(aNext))
                    {
                        bool bHandleAsSimpleEdge(true);

                        if(rEdgePolygon.areControlPointsUsed())
                        {
                            const B2DVector aCVecA(rEdgePolygon.getControlVectorA(a));
                            const B2DVector aCVecB(rEdgePolygon.getControlVectorB(a));
                            const bool bEdgeIsCurve(!aCVecA.equalZero() || !aCVecB.equalZero());

                            if(bEdgeIsCurve)
                            {
                                bHandleAsSimpleEdge = false;
                                const B2DCubicBezier aCubicA(aCurr, aCVecA, aCVecB, aNext);
                                findTouchesOnCurve(aCubicA, rPointPolygon, a, rTempPoints);
                            }
                        }

                        if(bHandleAsSimpleEdge)
                        {
                            findTouchesOnEdge(aCurr, aNext, rPointPolygon, a, rTempPoints);
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        ////////////////////////////////////////////////////////////////////////////////

        void findCuts(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            // find out if edges from both polygons cut. If so, add entries to rTempPoints which
            // should be added to the polygons accordingly
            const sal_uInt32 nPointCountA(rCandidateA.count());
            const sal_uInt32 nPointCountB(rCandidateB.count());

            if(nPointCountA && nPointCountB)
            {
                const sal_uInt32 nEdgeCountA(rCandidateA.isClosed() ? nPointCountA : nPointCountA - 1L);
                const sal_uInt32 nEdgeCountB(rCandidateB.isClosed() ? nPointCountB : nPointCountB - 1L);

                if(nEdgeCountA && nEdgeCountB)
                {
                    const bool bCurvesInvolved(rCandidateA.areControlPointsUsed() || rCandidateB.areControlPointsUsed());

                    if(bCurvesInvolved)
                    {
                        for(sal_uInt32 a(0L); a < nEdgeCountA; a++)
                        {
                            const B2DCubicBezier aCubicA(
                                rCandidateA.getB2DPoint(a),
                                rCandidateA.getControlVectorA(a),
                                rCandidateA.getControlVectorB(a),
                                rCandidateA.getB2DPoint(a + 1L == nPointCountA ? 0L : a + 1L));
                            const bool bEdgeAIsCurve(aCubicA.isBezier());
                            const B2DRange aRangeA(aCubicA.getRange());

                            for(sal_uInt32 b(0L); b < nEdgeCountB; b++)
                            {
                                const B2DCubicBezier aCubicB(
                                    rCandidateB.getB2DPoint(b),
                                    rCandidateB.getControlVectorA(b),
                                    rCandidateB.getControlVectorB(b),
                                    rCandidateB.getB2DPoint(b + 1L == nPointCountB ? 0L : b + 1L));
                                const bool bEdgeBIsCurve(aCubicB.isBezier());
                                const B2DRange aRangeB(aCubicB.getRange());

                                if(aRangeA.overlaps(aRangeB))
                                {
                                    if(bEdgeAIsCurve && bEdgeBIsCurve)
                                    {
                                        // test for bezier-bezier cuts
                                        findEdgeCutsTwoBeziers(aCubicA, aCubicB, a, b, rTempPointsA, rTempPointsB);
                                    }
                                    else if(bEdgeAIsCurve)
                                    {
                                        // test for bezier-edge cuts
                                        findEdgeCutsBezierAndEdge(aCubicA, aCubicB.getStartPoint(), aCubicB.getEndPoint(), a, b, rTempPointsA, rTempPointsB);
                                    }
                                    else if(bEdgeBIsCurve)
                                    {
                                        // test for bezier-edge cuts
                                        findEdgeCutsBezierAndEdge(aCubicB, aCubicA.getStartPoint(), aCubicA.getEndPoint(), b, a, rTempPointsB, rTempPointsA);
                                    }
                                    else
                                    {
                                        // test for simple edge-edge cuts
                                        findEdgeCutsTwoEdges(aCubicA.getStartPoint(), aCubicA.getEndPoint(), aCubicB.getStartPoint(), aCubicB.getEndPoint(),
                                            a, b, rTempPointsA, rTempPointsB);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        B2DPoint aCurrA(rCandidateA.getB2DPoint(0L));

                        for(sal_uInt32 a(0L); a < nEdgeCountA; a++)
                        {
                            const B2DPoint aNextA(rCandidateA.getB2DPoint(a + 1L == nPointCountA ? 0L : a + 1L));
                            const B2DRange aRangeA(aCurrA, aNextA);
                            B2DPoint aCurrB(rCandidateB.getB2DPoint(0L));

                            for(sal_uInt32 b(0L); b < nEdgeCountB; b++)
                            {
                                const B2DPoint aNextB(rCandidateB.getB2DPoint(b + 1L == nPointCountB ? 0L : b + 1L));
                                const B2DRange aRangeB(aCurrB, aNextB);

                                if(aRangeA.overlaps(aRangeB))
                                {
                                    // test for simple edge-edge cuts
                                    findEdgeCutsTwoEdges(aCurrA, aNextA, aCurrB, aNextB, a, b, rTempPointsA, rTempPointsB);
                                }

                                // prepare next step
                                aCurrB = aNextB;
                            }

                            // prepare next step
                            aCurrA = aNextA;
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        ////////////////////////////////////////////////////////////////////////////////

        B2DPolygon addPointsAtCutsAndTouches(const B2DPolygon& rCandidate)
        {
            if(rCandidate.count())
            {
                temporaryPointVector aTempPoints;

                findTouches(rCandidate, rCandidate, aTempPoints);
                findCuts(rCandidate, aTempPoints);

                return mergeTemporaryPointsAndPolygon(rCandidate, aTempPoints);
            }
            else
            {
                return rCandidate;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rCandidate, bool bSelfIntersections)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount)
            {
                B2DPolyPolygon aRetval;

                if(1L == nCount)
                {
                    if(bSelfIntersections)
                    {
                        // remove self intersections
                        aRetval.append(addPointsAtCutsAndTouches(rCandidate.getB2DPolygon(0L)));
                    }
                    else
                    {
                        // copy source
                        aRetval = rCandidate;
                    }
                }
                else
                {
                    // first solve self cuts and self touches for all contained single polygons
                    temporaryPolygonData *pTempData = new temporaryPolygonData[nCount];
                    sal_uInt32 a, b;

                    for(a = 0L; a < nCount; a++)
                    {
                        if(bSelfIntersections)
                        {
                            // use polygons with solved self intersections
                            pTempData[a].setPolygon(addPointsAtCutsAndTouches(rCandidate.getB2DPolygon(a)));
                        }
                        else
                        {
                            // copy given polygons
                            pTempData[a].setPolygon(rCandidate.getB2DPolygon(a));
                        }
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
                        aRetval.append(mergeTemporaryPointsAndPolygon(pTempData[a].getPolygon(), pTempData[a].getTemporaryPointVector()));
                    }

                    delete[] pTempData;
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        B2DPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rMask, const B2DPolygon& rCandidate)
        {
            if(rCandidate.count())
            {
                temporaryPointVector aTempPoints;
                temporaryPointVector aTempPointsUnused;

                for(sal_uInt32 a(0L); a < rMask.count(); a++)
                {
                    const B2DPolygon aPartMask(rMask.getB2DPolygon(a));

                    findTouches(rCandidate, aPartMask, aTempPoints);
                    findCuts(rCandidate, aPartMask, aTempPoints, aTempPointsUnused);
                }

                return mergeTemporaryPointsAndPolygon(rCandidate, aTempPoints);
            }
            else
            {
                return rCandidate;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rMask, const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                aRetval.append(addPointsAtCutsAndTouches(rMask, rCandidate.getB2DPolygon(a)));
            }

            return aRetval;
        }

        ////////////////////////////////////////////////////////////////////////////////

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
