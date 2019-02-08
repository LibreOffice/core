/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <osl/diagnose.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

#include <vector>
#include <algorithm>
#include <memory>

#define SUBDIVIDE_FOR_CUT_TEST_COUNT        (50)

namespace basegfx
{
    namespace
    {

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

        typedef std::vector< temporaryPoint > temporaryPointVector;

        class temporaryPolygonData
        {
            B2DPolygon                              maPolygon;
            B2DRange                                maRange;
            temporaryPointVector                    maPoints;

        public:
            const B2DPolygon& getPolygon() const { return maPolygon; }
            void setPolygon(const B2DPolygon& rNew) { maPolygon = rNew; maRange = utils::getRange(maPolygon); }
            const B2DRange& getRange() const { return maRange; }
            temporaryPointVector& getTemporaryPointVector() { return maPoints; }
        };

        B2DPolygon mergeTemporaryPointsAndPolygon(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints)
        {
            // #i76891# mergeTemporaryPointsAndPolygon redesigned to be able to correctly handle
            // single edges with/without control points
            // #i101491# added counter for non-changing element count
            const sal_uInt32 nTempPointCount(rTempPoints.size());

            if(nTempPointCount)
            {
                B2DPolygon aRetval;
                const sal_uInt32 nCount(rCandidate.count());

                if(nCount)
                {
                    // sort temp points to assure increasing fCut values and increasing indices
                    std::sort(rTempPoints.begin(), rTempPoints.end());

                    // prepare loop
                    B2DCubicBezier aEdge;
                    sal_uInt32 nNewInd(0);

                    // add start point
                    aRetval.append(rCandidate.getB2DPoint(0));

                    for(sal_uInt32 a(0); a < nCount; a++)
                    {
                        // get edge
                        rCandidate.getBezierSegment(a, aEdge);

                        if(aEdge.isBezier())
                        {
                            // control vectors involved for this edge
                            double fLeftStart(0.0);

                            // now add all points targeted to be at this index
                            while (nNewInd < nTempPointCount && rTempPoints[nNewInd].getIndex() == a && fLeftStart < 1.0)
                            {
                                const temporaryPoint& rTempPoint = rTempPoints[nNewInd++];

                                // split curve segment. Splits need to come sorted and need to be < 1.0. Also,
                                // since original segment is consumed from left to right, the cut values need
                                // to be scaled to the remaining part
                                B2DCubicBezier aLeftPart;
                                const double fRelativeSplitPoint((rTempPoint.getCut() - fLeftStart) / (1.0 - fLeftStart));
                                aEdge.split(fRelativeSplitPoint, &aLeftPart, &aEdge);
                                fLeftStart = rTempPoint.getCut();

                                // add left bow
                                aRetval.appendBezierSegment(aLeftPart.getControlPointA(), aLeftPart.getControlPointB(), rTempPoint.getPoint());
                            }

                            // add remaining bow
                            aRetval.appendBezierSegment(aEdge.getControlPointA(), aEdge.getControlPointB(), aEdge.getEndPoint());
                        }
                        else
                        {
                            // add all points targeted to be at this index
                            while(nNewInd < nTempPointCount && rTempPoints[nNewInd].getIndex() == a)
                            {
                                const temporaryPoint& rTempPoint = rTempPoints[nNewInd++];
                                const B2DPoint& aNewPoint(rTempPoint.getPoint());

                                // do not add points double
                                if(!aRetval.getB2DPoint(aRetval.count() - 1).equal(aNewPoint))
                                {
                                    aRetval.append(aNewPoint);
                                }
                            }

                            // add edge end point
                            aRetval.append(aEdge.getEndPoint());
                        }
                    }
                }

                if(rCandidate.isClosed())
                {
                    // set closed flag and correct last point (which is added double now).
                    utils::closeWithGeometryChange(aRetval);
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        void adaptAndTransferCutsWithBezierSegment(
            const temporaryPointVector& rPointVector, const B2DPolygon& rPolygon,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            // assuming that the subdivision to create rPolygon used equidistant pieces
            // (as in adaptiveSubdivideByCount) it is now possible to calculate back the
            // cut positions in the polygon to relative cut positions on the original bezier
            // segment.
            const sal_uInt32 nTempPointCount(rPointVector.size());
            const sal_uInt32 nEdgeCount(rPolygon.count() ? rPolygon.count() - 1 : 0);

            if(nTempPointCount && nEdgeCount)
            {
                for(sal_uInt32 a(0); a < nTempPointCount; a++)
                {
                    const temporaryPoint& rTempPoint = rPointVector[a];
                    const double fCutPosInPolygon(static_cast<double>(rTempPoint.getIndex()) + rTempPoint.getCut());
                    const double fRelativeCutPos(fCutPosInPolygon / static_cast<double>(nEdgeCount));
                    rTempPoints.emplace_back(rTempPoint.getPoint(), nInd, fRelativeCutPos);
                }
            }
        }

    } // end of anonymous namespace
} // end of namespace basegfx

namespace basegfx
{
    namespace
    {

        // predefines for calls to this methods before method implementation

        void findCuts(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints);
        void findTouches(const B2DPolygon& rEdgePolygon, const B2DPolygon& rPointPolygon, temporaryPointVector& rTempPoints);
        void findCuts(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB);

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

                        if (fTools::betweenOrEqualEither(fCut, fZero, fOne))
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

                            if (fTools::betweenOrEqualEither(fCut2, fZero, fOne))
                            {
                                // cut is in range, add point. Two edges can have only one cut, but
                                // add a cut point to each list. The lists may be the same for
                                // self intersections.
                                const B2DPoint aCutPoint(interpolate(rCurrA, rNextA, fCut));
                                rTempPointsA.emplace_back(aCutPoint, nIndA, fCut);
                                rTempPointsB.emplace_back(aCutPoint, nIndB, fCut2);
                            }
                        }
                    }
                }
            }
        }

        void findCutsAndTouchesAndCommonForBezier(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            // #i76891#
            // This new method is necessary since in findEdgeCutsBezierAndEdge and in findEdgeCutsTwoBeziers
            // it is not sufficient to use findCuts() recursively. This will indeed find the cuts between the
            // segments of the two temporarily adaptive subdivided bezier segments, but not the touches or
            // equal points of them.
            // It would be possible to find the touches using findTouches(), but at last with common points
            // the adding of cut points (temporary points) would fail. But for these temporarily adaptive
            // subdivided bezier segments, common points may be not very likely, but the bug shows that it
            // happens.
            // Touch points are a little bit more likely than common points. All in all it is best to use
            // a specialized method here which can profit from knowing that it is working on a special
            // family of B2DPolygons: no curve segments included and not closed.
            OSL_ENSURE(!rCandidateA.areControlPointsUsed() && !rCandidateB.areControlPointsUsed(), "findCutsAndTouchesAndCommonForBezier only works with subdivided polygons (!)");
            OSL_ENSURE(!rCandidateA.isClosed() && !rCandidateB.isClosed(), "findCutsAndTouchesAndCommonForBezier only works with opened polygons (!)");
            const sal_uInt32 nPointCountA(rCandidateA.count());
            const sal_uInt32 nPointCountB(rCandidateB.count());

            if(nPointCountA > 1 && nPointCountB > 1)
            {
                const sal_uInt32 nEdgeCountA(nPointCountA - 1);
                const sal_uInt32 nEdgeCountB(nPointCountB - 1);
                B2DPoint aCurrA(rCandidateA.getB2DPoint(0));

                for(sal_uInt32 a(0); a < nEdgeCountA; a++)
                {
                    const B2DPoint aNextA(rCandidateA.getB2DPoint(a + 1));
                    const B2DRange aRangeA(aCurrA, aNextA);
                    B2DPoint aCurrB(rCandidateB.getB2DPoint(0));

                    for(sal_uInt32 b(0); b < nEdgeCountB; b++)
                    {
                        const B2DPoint aNextB(rCandidateB.getB2DPoint(b + 1));
                        const B2DRange aRangeB(aCurrB, aNextB);

                        if(aRangeA.overlaps(aRangeB))
                        {
                            // no null length edges
                            if(!(aCurrA.equal(aNextA) || aCurrB.equal(aNextB)))
                            {
                                const B2DVector aVecA(aNextA - aCurrA);
                                const B2DVector aVecB(aNextB - aCurrB);
                                double fCutA(aVecA.cross(aVecB));

                                if(!fTools::equalZero(fCutA))
                                {
                                    const double fZero(0.0);
                                    const double fOne(1.0);
                                    fCutA = (aVecB.getY() * (aCurrB.getX() - aCurrA.getX()) + aVecB.getX() * (aCurrA.getY() - aCurrB.getY())) / fCutA;

                                    // use range [0.0 .. 1.0[, thus in the loop, all direct aCurrA cuts will be registered
                                    // as 0.0 cut. The 1.0 cut will be registered in the next loop step
                                    if(fTools::moreOrEqual(fCutA, fZero) && fTools::less(fCutA, fOne))
                                    {
                                        // it's a candidate, but also need to test parameter value of cut on line 2
                                        double fCutB;

                                        // choose the more precise version
                                        if(fabs(aVecB.getX()) > fabs(aVecB.getY()))
                                        {
                                            fCutB = (aCurrA.getX() + (fCutA * aVecA.getX()) - aCurrB.getX()) / aVecB.getX();
                                        }
                                        else
                                        {
                                            fCutB = (aCurrA.getY() + (fCutA * aVecA.getY()) - aCurrB.getY()) / aVecB.getY();
                                        }

                                        // use range [0.0 .. 1.0[, thus in the loop, all direct aCurrA cuts will be registered
                                        // as 0.0 cut. The 1.0 cut will be registered in the next loop step
                                        if(fTools::moreOrEqual(fCutB, fZero) && fTools::less(fCutB, fOne))
                                        {
                                            // cut is in both ranges. Add points for A and B
                                            // #i111715# use fTools::equal instead of fTools::equalZero for better accuracy
                                            if(fTools::equal(fCutA, fZero))
                                            {
                                                // ignore for start point in first edge; this is handled
                                                // by outer methods and would just produce a double point
                                                if(a)
                                                {
                                                    rTempPointsA.emplace_back(aCurrA, a, 0.0);
                                                }
                                            }
                                            else
                                            {
                                                const B2DPoint aCutPoint(interpolate(aCurrA, aNextA, fCutA));
                                                rTempPointsA.emplace_back(aCutPoint, a, fCutA);
                                            }

                                            // #i111715# use fTools::equal instead of fTools::equalZero for better accuracy
                                            if(fTools::equal(fCutB, fZero))
                                            {
                                                // ignore for start point in first edge; this is handled
                                                // by outer methods and would just produce a double point
                                                if(b)
                                                {
                                                    rTempPointsB.emplace_back(aCurrB, b, 0.0);
                                                }
                                            }
                                            else
                                            {
                                                const B2DPoint aCutPoint(interpolate(aCurrB, aNextB, fCutB));
                                                rTempPointsB.emplace_back(aCutPoint, b, fCutB);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // prepare next step
                        aCurrB = aNextB;
                    }

                    // prepare next step
                    aCurrA = aNextA;
                }
            }
        }

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
            // Keep adaptiveSubdivideByCount due to needed quality
            aTempPolygonA.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygonA.append(rCubicA.getStartPoint());
            rCubicA.adaptiveSubdivideByCount(aTempPolygonA, SUBDIVIDE_FOR_CUT_TEST_COUNT);
            aTempPolygonEdge.append(rCurrB);
            aTempPolygonEdge.append(rNextB);

            // #i76891# using findCuts recursively is not sufficient here
            findCutsAndTouchesAndCommonForBezier(aTempPolygonA, aTempPolygonEdge, aTempPointVectorA, aTempPointVectorEdge);

            if(!aTempPointVectorA.empty())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorA, aTempPolygonA, nIndA, rTempPointsA);
            }

            // append remapped tempVector entries for edge to tempPoints for edge
            for(temporaryPoint & rTempPoint : aTempPointVectorEdge)
            {
                rTempPointsB.emplace_back(rTempPoint.getPoint(), nIndB, rTempPoint.getCut());
            }
        }

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
            // Keep adaptiveSubdivideByCount due to needed quality
            aTempPolygonA.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygonA.append(rCubicA.getStartPoint());
            rCubicA.adaptiveSubdivideByCount(aTempPolygonA, SUBDIVIDE_FOR_CUT_TEST_COUNT);
            aTempPolygonB.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygonB.append(rCubicB.getStartPoint());
            rCubicB.adaptiveSubdivideByCount(aTempPolygonB, SUBDIVIDE_FOR_CUT_TEST_COUNT);

            // #i76891# using findCuts recursively is not sufficient here
            findCutsAndTouchesAndCommonForBezier(aTempPolygonA, aTempPolygonB, aTempPointVectorA, aTempPointVectorB);

            if(!aTempPointVectorA.empty())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorA, aTempPolygonA, nIndA, rTempPointsA);
            }

            if(!aTempPointVectorB.empty())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVectorB, aTempPolygonB, nIndB, rTempPointsB);
            }
        }

        void findEdgeCutsOneBezier(
            const B2DCubicBezier& rCubicA,
            sal_uInt32 nInd, temporaryPointVector& rTempPoints)
        {
            // avoid expensive part of this method if possible
            // TODO: use hasAnyExtremum() method instead when it becomes available
            double fDummy;
            const bool bHasAnyExtremum = rCubicA.getMinimumExtremumPosition( fDummy );
            if( !bHasAnyExtremum )
                return;

            // find all self-intersections on the given bezier segment. Add an entry to the tempPoints
            // for each self intersection point with the cut value describing the relative position on given
            // bezier segment.
            B2DPolygon aTempPolygon;
            temporaryPointVector aTempPointVector;

            // create subdivided polygon and find cuts on it
            // Keep adaptiveSubdivideByCount due to needed quality
            aTempPolygon.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygon.append(rCubicA.getStartPoint());
            rCubicA.adaptiveSubdivideByCount(aTempPolygon, SUBDIVIDE_FOR_CUT_TEST_COUNT);
            findCuts(aTempPolygon, aTempPointVector);

            if(!aTempPointVector.empty())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVector, aTempPolygon, nInd, rTempPoints);
            }
        }

        void findCuts(const B2DPolygon& rCandidate, temporaryPointVector& rTempPoints)
        {
            // find out if there are edges with intersections (self-cuts). If yes, add
            // entries to rTempPoints accordingly
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);

                if(nEdgeCount)
                {
                    const bool bCurvesInvolved(rCandidate.areControlPointsUsed());

                    if(bCurvesInvolved)
                    {
                        B2DCubicBezier aCubicA;
                        B2DCubicBezier aCubicB;

                        for(sal_uInt32 a(0); a < nEdgeCount - 1; a++)
                        {
                            rCandidate.getBezierSegment(a, aCubicA);
                            aCubicA.testAndSolveTrivialBezier();
                            const bool bEdgeAIsCurve(aCubicA.isBezier());
                            const B2DRange aRangeA(aCubicA.getRange());

                            if(bEdgeAIsCurve)
                            {
                                // curved segments may have self-intersections, do not forget those (!)
                                findEdgeCutsOneBezier(aCubicA, a, rTempPoints);
                            }

                            for(sal_uInt32 b(a + 1); b < nEdgeCount; b++)
                            {
                                rCandidate.getBezierSegment(b, aCubicB);
                                aCubicB.testAndSolveTrivialBezier();
                                const B2DRange aRangeB(aCubicB.getRange());

                                // only overlapping segments need to be tested
                                // consecutive segments touch of course
                                bool bOverlap = false;
                                if( b > a+1)
                                    bOverlap = aRangeA.overlaps(aRangeB);
                                else
                                    bOverlap = aRangeA.overlapsMore(aRangeB);
                                if( bOverlap)
                                {
                                    const bool bEdgeBIsCurve(aCubicB.isBezier());
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
                        B2DPoint aCurrA(rCandidate.getB2DPoint(0));

                        for(sal_uInt32 a(0); a < nEdgeCount - 1; a++)
                        {
                            const B2DPoint aNextA(rCandidate.getB2DPoint(a + 1 == nPointCount ? 0 : a + 1));
                            const B2DRange aRangeA(aCurrA, aNextA);
                            B2DPoint aCurrB(rCandidate.getB2DPoint(a + 1));

                            for(sal_uInt32 b(a + 1); b < nEdgeCount; b++)
                            {
                                const B2DPoint aNextB(rCandidate.getB2DPoint(b + 1 == nPointCount ? 0 : b + 1));
                                const B2DRange aRangeB(aCurrB, aNextB);

                                // consecutive segments touch of course
                                bool bOverlap = false;
                                if( b > a+1)
                                    bOverlap = aRangeA.overlaps(aRangeB);
                                else
                                    bOverlap = aRangeA.overlapsMore(aRangeB);
                                if( bOverlap)
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

    } // end of anonymous namespace
} // end of namespace basegfx

namespace basegfx
{
    namespace
    {

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

                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    const B2DPoint aTestPoint(rPointPolygon.getB2DPoint(a));

                    if(aRange.isInside(aTestPoint))
                    {
                        if(!aTestPoint.equal(rCurr) && !aTestPoint.equal(rNext))
                        {
                            const B2DVector aTestVector(aTestPoint - rCurr);

                            if(areParallel(aEdgeVector, aTestVector))
                            {
                                const double fCut(bTestUsingX
                                    ? aTestVector.getX() / aEdgeVector.getX()
                                    : aTestVector.getY() / aEdgeVector.getY());
                                const double fZero(0.0);
                                const double fOne(1.0);

                                if(fTools::more(fCut, fZero) && fTools::less(fCut, fOne))
                                {
                                    rTempPoints.emplace_back(aTestPoint, nInd, fCut);
                                }
                            }
                        }
                    }
                }
            }
        }

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
            // Keep adaptiveSubdivideByCount due to needed quality
            aTempPolygon.reserve(SUBDIVIDE_FOR_CUT_TEST_COUNT + 8);
            aTempPolygon.append(rCubicA.getStartPoint());
            rCubicA.adaptiveSubdivideByCount(aTempPolygon, SUBDIVIDE_FOR_CUT_TEST_COUNT);
            findTouches(aTempPolygon, rPointPolygon, aTempPointVector);

            if(!aTempPointVector.empty())
            {
                // adapt tempVector entries to segment
                adaptAndTransferCutsWithBezierSegment(aTempPointVector, aTempPolygon, nInd, rTempPoints);
            }
        }

        void findTouches(const B2DPolygon& rEdgePolygon, const B2DPolygon& rPointPolygon, temporaryPointVector& rTempPoints)
        {
            // find out if points from rPointPolygon touch edges from rEdgePolygon. If yes,
            // add entries to rTempPoints
            const sal_uInt32 nPointCount(rPointPolygon.count());
            const sal_uInt32 nEdgePointCount(rEdgePolygon.count());

            if(nPointCount && nEdgePointCount)
            {
                const sal_uInt32 nEdgeCount(rEdgePolygon.isClosed() ? nEdgePointCount : nEdgePointCount - 1);
                B2DPoint aCurr(rEdgePolygon.getB2DPoint(0));

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    const sal_uInt32 nNextIndex((a + 1) % nEdgePointCount);
                    const B2DPoint aNext(rEdgePolygon.getB2DPoint(nNextIndex));

                    if(!aCurr.equal(aNext))
                    {
                        bool bHandleAsSimpleEdge(true);

                        if(rEdgePolygon.areControlPointsUsed())
                        {
                            const B2DPoint aNextControlPoint(rEdgePolygon.getNextControlPoint(a));
                            const B2DPoint aPrevControlPoint(rEdgePolygon.getPrevControlPoint(nNextIndex));
                            const bool bEdgeIsCurve(!aNextControlPoint.equal(aCurr) || !aPrevControlPoint.equal(aNext));

                            if(bEdgeIsCurve)
                            {
                                bHandleAsSimpleEdge = false;
                                const B2DCubicBezier aCubicA(aCurr, aNextControlPoint, aPrevControlPoint, aNext);
                                findTouchesOnCurve(aCubicA, rPointPolygon, a, rTempPoints);
                            }
                        }

                        if(bHandleAsSimpleEdge)
                        {
                            findTouchesOnEdge(aCurr, aNext, rPointPolygon, a, rTempPoints);
                        }
                    }

                    // next step
                    aCurr = aNext;
                }
            }
        }

    } // end of anonymous namespace
} // end of namespace basegfx

namespace basegfx
{
    namespace
    {

        void findCuts(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, temporaryPointVector& rTempPointsA, temporaryPointVector& rTempPointsB)
        {
            // find out if edges from both polygons cut. If so, add entries to rTempPoints which
            // should be added to the polygons accordingly
            const sal_uInt32 nPointCountA(rCandidateA.count());
            const sal_uInt32 nPointCountB(rCandidateB.count());

            if(nPointCountA && nPointCountB)
            {
                const sal_uInt32 nEdgeCountA(rCandidateA.isClosed() ? nPointCountA : nPointCountA - 1);
                const sal_uInt32 nEdgeCountB(rCandidateB.isClosed() ? nPointCountB : nPointCountB - 1);

                if(nEdgeCountA && nEdgeCountB)
                {
                    const bool bCurvesInvolved(rCandidateA.areControlPointsUsed() || rCandidateB.areControlPointsUsed());

                    if(bCurvesInvolved)
                    {
                        B2DCubicBezier aCubicA;
                        B2DCubicBezier aCubicB;

                        for(sal_uInt32 a(0); a < nEdgeCountA; a++)
                        {
                            rCandidateA.getBezierSegment(a, aCubicA);
                            aCubicA.testAndSolveTrivialBezier();
                            const bool bEdgeAIsCurve(aCubicA.isBezier());
                            const B2DRange aRangeA(aCubicA.getRange());

                            for(sal_uInt32 b(0); b < nEdgeCountB; b++)
                            {
                                rCandidateB.getBezierSegment(b, aCubicB);
                                aCubicB.testAndSolveTrivialBezier();
                                const B2DRange aRangeB(aCubicB.getRange());

                                // consecutive segments touch of course
                                bool bOverlap = false;
                                if( b > a+1)
                                    bOverlap = aRangeA.overlaps(aRangeB);
                                else
                                    bOverlap = aRangeA.overlapsMore(aRangeB);
                                if( bOverlap)
                                {
                                    const bool bEdgeBIsCurve(aCubicB.isBezier());
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
                        B2DPoint aCurrA(rCandidateA.getB2DPoint(0));

                        for(sal_uInt32 a(0); a < nEdgeCountA; a++)
                        {
                            const B2DPoint aNextA(rCandidateA.getB2DPoint(a + 1 == nPointCountA ? 0 : a + 1));
                            const B2DRange aRangeA(aCurrA, aNextA);
                            B2DPoint aCurrB(rCandidateB.getB2DPoint(0));

                            for(sal_uInt32 b(0); b < nEdgeCountB; b++)
                            {
                                const B2DPoint aNextB(rCandidateB.getB2DPoint(b + 1 == nPointCountB ? 0 : b + 1));
                                const B2DRange aRangeB(aCurrB, aNextB);

                                // consecutive segments touch of course
                                bool bOverlap = false;
                                if( b > a+1)
                                    bOverlap = aRangeA.overlaps(aRangeB);
                                else
                                    bOverlap = aRangeA.overlapsMore(aRangeB);
                                if( bOverlap)
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

    } // end of anonymous namespace
} // end of namespace basegfx

namespace basegfx
{
    namespace utils
    {

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

        B2DPolyPolygon addPointsAtCutsAndTouches(const B2DPolyPolygon& rCandidate)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount)
            {
                B2DPolyPolygon aRetval;

                if(nCount == 1)
                {
                    // remove self intersections
                    aRetval.append(addPointsAtCutsAndTouches(rCandidate.getB2DPolygon(0)));
                }
                else
                {
                    // first solve self cuts and self touches for all contained single polygons
                    std::unique_ptr<temporaryPolygonData[]> pTempData(new temporaryPolygonData[nCount]);
                    sal_uInt32 a, b;

                    for(a = 0; a < nCount; a++)
                    {
                        // use polygons with solved self intersections
                        pTempData[a].setPolygon(addPointsAtCutsAndTouches(rCandidate.getB2DPolygon(a)));
                    }

                    // now cuts and touches between the polygons
                    for(a = 0; a < nCount; a++)
                    {
                        for(b = 0; b < nCount; b++)
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
                    for(a = 0; a < nCount; a++)
                    {
                        aRetval.append(mergeTemporaryPointsAndPolygon(pTempData[a].getPolygon(), pTempData[a].getTemporaryPointVector()));
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPoint& rStart, const B2DPoint& rEnd)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount && !rStart.equal(rEnd))
            {
                const B2DRange aPolygonRange(rCandidate.getB2DRange());
                const B2DRange aEdgeRange(rStart, rEnd);

                if(aPolygonRange.overlaps(aEdgeRange))
                {
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nCount : nCount - 1);
                    temporaryPointVector aTempPoints;
                    temporaryPointVector aUnusedTempPoints;
                    B2DCubicBezier aCubic;

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        rCandidate.getBezierSegment(a, aCubic);
                        B2DRange aCubicRange(aCubic.getStartPoint(), aCubic.getEndPoint());

                        if(aCubic.isBezier())
                        {
                            aCubicRange.expand(aCubic.getControlPointA());
                            aCubicRange.expand(aCubic.getControlPointB());

                            if(aCubicRange.overlaps(aEdgeRange))
                            {
                                findEdgeCutsBezierAndEdge(aCubic, rStart, rEnd, a, 0, aTempPoints, aUnusedTempPoints);
                            }
                        }
                        else
                        {
                            if(aCubicRange.overlaps(aEdgeRange))
                            {
                                findEdgeCutsTwoEdges(aCubic.getStartPoint(), aCubic.getEndPoint(), rStart, rEnd, a, 0, aTempPoints, aUnusedTempPoints);
                            }
                        }
                    }

                    return mergeTemporaryPointsAndPolygon(rCandidate, aTempPoints);
                }
            }

            return rCandidate;
        }

        B2DPolygon addPointsAtCuts(const B2DPolygon& rCandidate, const B2DPolyPolygon& rPolyMask)
        {
            const sal_uInt32 nCountA(rCandidate.count());
            const sal_uInt32 nCountM(rPolyMask.count());

            if(nCountA && nCountM)
            {
                const B2DRange aRangeA(rCandidate.getB2DRange());
                const B2DRange aRangeM(rPolyMask.getB2DRange());

                if(aRangeA.overlaps(aRangeM))
                {
                    const sal_uInt32 nEdgeCountA(rCandidate.isClosed() ? nCountA : nCountA - 1);
                    temporaryPointVector aTempPointsA;
                    temporaryPointVector aUnusedTempPointsB;

                    for(sal_uInt32 m(0); m < nCountM; m++)
                    {
                        const B2DPolygon& aMask(rPolyMask.getB2DPolygon(m));
                        const sal_uInt32 nCountB(aMask.count());

                        if(nCountB)
                        {
                            B2DCubicBezier aCubicA;
                            B2DCubicBezier aCubicB;

                            for(sal_uInt32 a(0); a < nEdgeCountA; a++)
                            {
                                rCandidate.getBezierSegment(a, aCubicA);
                                const bool bCubicAIsCurve(aCubicA.isBezier());
                                B2DRange aCubicRangeA(aCubicA.getStartPoint(), aCubicA.getEndPoint());

                                if(bCubicAIsCurve)
                                {
                                    aCubicRangeA.expand(aCubicA.getControlPointA());
                                    aCubicRangeA.expand(aCubicA.getControlPointB());
                                }

                                for(sal_uInt32 b(0); b < nCountB; b++)
                                {
                                    aMask.getBezierSegment(b, aCubicB);
                                    const bool bCubicBIsCurve(aCubicB.isBezier());
                                    B2DRange aCubicRangeB(aCubicB.getStartPoint(), aCubicB.getEndPoint());

                                    if(bCubicBIsCurve)
                                    {
                                        aCubicRangeB.expand(aCubicB.getControlPointA());
                                        aCubicRangeB.expand(aCubicB.getControlPointB());
                                    }

                                    if(aCubicRangeA.overlaps(aCubicRangeB))
                                    {
                                        if(bCubicAIsCurve && bCubicBIsCurve)
                                        {
                                            findEdgeCutsTwoBeziers(aCubicA, aCubicB, a, b, aTempPointsA, aUnusedTempPointsB);
                                        }
                                        else if(bCubicAIsCurve)
                                        {
                                            findEdgeCutsBezierAndEdge(aCubicA, aCubicB.getStartPoint(), aCubicB.getEndPoint(), a, b, aTempPointsA, aUnusedTempPointsB);
                                        }
                                        else if(bCubicBIsCurve)
                                        {
                                            findEdgeCutsBezierAndEdge(aCubicB, aCubicA.getStartPoint(), aCubicA.getEndPoint(), b, a, aUnusedTempPointsB, aTempPointsA);
                                        }
                                        else
                                        {
                                            findEdgeCutsTwoEdges(aCubicA.getStartPoint(), aCubicA.getEndPoint(), aCubicB.getStartPoint(), aCubicB.getEndPoint(), a, b, aTempPointsA, aUnusedTempPointsB);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    return mergeTemporaryPointsAndPolygon(rCandidate, aTempPointsA);
                }
            }

            return rCandidate;
        }

    } // end of namespace utils
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
