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

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <vector>
#include <list>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        class TemporaryPoint
        {
        private:
            B2DPoint                            maPoint;        // the new point
            double                              mfCut;          // parametric cut description [0.0 .. 1.0]

        public:
            TemporaryPoint(
                const B2DPoint& rNewPoint,
                double fCut)
            :   maPoint(rNewPoint),
                mfCut(fCut)
            {
            }

            bool operator<(const TemporaryPoint& rComp) const
            {
                return (mfCut < rComp.mfCut);
            }

            const B2DPoint& getB2DPoint() const { return maPoint; }
            double getCut() const { return mfCut; }
        };

        class OriginalData
        {
            const B2DPolyPolygon&   mrPolyPoly;
            bool                    mbPositive;

        public:
            OriginalData(
                const B2DPolyPolygon& rPolyPoly,
                bool bPositive)
            :   mrPolyPoly(rPolyPoly),
                mbPositive(bPositive)
            {
            }

            const B2DPolyPolygon& getB2DPolyPolygon() const
            {
                return mrPolyPoly;
            }

            bool isPositive() const
            {
                return mbPositive;
            }
        };

        class Edge;
        class Node
        {
        private:
            B2DPoint            maB2DPoint;
            std::list< Edge* >  maEdges;

        public:
            Node(
                const B2DPoint& rB2DPoint)
            :   maB2DPoint(rB2DPoint),
                maEdges()
            {
            }

            bool operator<(const Node& rComp) const
            {
                if(maB2DPoint.getX() == rComp.maB2DPoint.getX())
                {
                    return maB2DPoint.getY() < rComp.maB2DPoint.getY();
                }

                return maB2DPoint.getX() < rComp.maB2DPoint.getX();
            }

            void add(Edge* pNew) { maEdges.push_back(pNew); }
            const B2DPoint& getB2DPoint() const { return maB2DPoint; }
            bool hasEdges() const { return !maEdges.empty(); }
            Edge* getEdge() { if(maEdges.empty()) return nullptr; return maEdges.front(); }
            void remove(Edge* pEdge) { maEdges.remove(pEdge); }
        };

        class Edge
        {
        private:
            Node*                           mpNodeA;
            Node*                           mpNodeB;
            B2DPoint                        maControlA;
            B2DPoint                        maControlB;
            const OriginalData&             mrOriginalData;
            std::vector< TemporaryPoint >   maTemporaryPoints;
            B2DRange                        maRange;
            bool                            mbIsBezier;

        public:
            Edge(
                Node& rNodeA,
                Node& rNodeB,
                const B2DPoint& rControlA,
                const B2DPoint& rControlB,
                const OriginalData& rOriginalData)
            :   mpNodeA(&rNodeA),
                mpNodeB(&rNodeB),
                maControlA(rControlA),
                maControlB(rControlB),
                mrOriginalData(rOriginalData),
                maTemporaryPoints(),
                maRange(rNodeA.getB2DPoint(), rNodeB.getB2DPoint()),
                mbIsBezier(rNodeA.getB2DPoint() != rControlA || rNodeB.getB2DPoint() != rControlB)
            {
                mpNodeA->add(this);
                mpNodeB->add(this);

                if(mbIsBezier)
                {
                    maRange.expand(maControlA);
                    maRange.expand(maControlB);
                }
            }

            Node& getNodeA() { return *mpNodeA; }
            Node& getNodeB() { return *mpNodeB; }
            const B2DPoint& getControlA() const { return maControlA; }
            const B2DPoint& getControlB() const { return maControlB; }
            Node& getOther(Node& rCaller) { if(&rCaller == mpNodeA) return *mpNodeB; return *mpNodeA; }
            const B2DPoint& getControl(Node& rCaller) const { if(&rCaller == mpNodeA) return maControlA; return maControlB; }
            const OriginalData& getOriginalData() const { return mrOriginalData; }
            void consume() { mpNodeA->remove(this); mpNodeB->remove(this); }
            bool isBezier() const { return mbIsBezier; }
            std::vector< TemporaryPoint >& getTemporaryPoints() { return maTemporaryPoints; }
            const B2DRange& getRange() const { return maRange; }

            void modifyNodeB(Node& rNodeB, const B2DPoint& rControlA, const B2DPoint& rControlB)
            {
                mpNodeB->remove(this);
                mpNodeB = &rNodeB;
                mpNodeB->add(this);

                maControlA = rControlA;
                maControlB = rControlB;

                maTemporaryPoints.clear();
                maRange.reset();
                maRange.expand(mpNodeA->getB2DPoint());
                maRange.expand(mpNodeB->getB2DPoint());
                mbIsBezier = mpNodeA->getB2DPoint() != maControlA || mpNodeB->getB2DPoint() != maControlB;

                if(mbIsBezier)
                {
                    maRange.expand(maControlA);
                    maRange.expand(maControlB);
                }
            }

            B2DPoint getCenter() const
            {
                if(mbIsBezier)
                {
                    const B2DCubicBezier aB2DCubicBezier(
                        mpNodeA->getB2DPoint(),
                        getControlA(),
                        getControlB(),
                        mpNodeB->getB2DPoint());

                    return aB2DCubicBezier.interpolatePoint(0.5);
                }
                else
                {
                    return (0.5 * (mpNodeA->getB2DPoint() + mpNodeB->getB2DPoint()));
                }
            }
        };

        class cutter
        {
        private:
            std::list< OriginalData >   maOriginalDatas;
            std::set< Node >            maNodes;
            std::vector< Edge* >        maEdges;

        public:
            cutter()
            :   maOriginalDatas(),
                maNodes(),
                maEdges()
            {
            }

            ~cutter()
            {
                while(!maEdges.empty())
                {
                    Edge* pCand(maEdges.back());
                    maEdges.pop_back();
                    delete pCand;
                }
            }

            void add(
                const B2DPolyPolygon& rCand,
                bool bPositive)
            {
                if(0 == rCand.count())
                {
                    return;
                }

                maOriginalDatas.emplace_back(
                    rCand,
                    bPositive);
                const OriginalData& rOriginalData(maOriginalDatas.back());

                for(sal_uInt32 a(0); a < rOriginalData.getB2DPolyPolygon().count(); a++)
                {
                    const B2DPolygon& rPoly(rOriginalData.getB2DPolyPolygon().getB2DPolygon(a));
                    const sal_uInt32 nCount(rPoly.count());

                    if(0 != nCount)
                    {
                        // create or find Node
                        Node* pFirst(&const_cast<Node&>(*maNodes.emplace(rPoly.getB2DPoint(0)).first));
                        Node* pLast(pFirst);

                        for(sal_uInt32 b(1); b < rPoly.count(); b++)
                        {
                            // create or find Node
                            Node* pNew(&const_cast<Node&>(*maNodes.emplace(rPoly.getB2DPoint(b)).first));

                            maEdges.push_back(
                                new Edge(
                                    *pLast,
                                    *pNew,
                                    rPoly.getNextControlPoint(b - 1),
                                    rPoly.getPrevControlPoint(b),
                                    rOriginalData));

                            pLast = pNew;
                        }

                        maEdges.push_back(
                            new Edge(
                                *pLast,
                                *pFirst,
                                rPoly.getNextControlPoint(nCount - 1),
                                rPoly.getPrevControlPoint(0),
                                rOriginalData));
                    }
                }
            }

            bool findTouchOnEdge(
                const B2DPoint& rStart,
                const B2DPoint& rEnd,
                const B2DPoint& rCandidate,
                double& rfCut)
            {
                const B2DVector aEdgeVector(rEnd - rStart);
                const bool bTestUsingX(fabs(aEdgeVector.getX()) > fabs(aEdgeVector.getY()));

                if(!rCandidate.equal(rStart) && !rCandidate.equal(rEnd))
                {
                    const B2DVector aTestVector(rCandidate - rStart);

                    if(areParallel(aEdgeVector, aTestVector))
                    {
                        rfCut = (bTestUsingX
                            ? aTestVector.getX() / aEdgeVector.getX()
                            : aTestVector.getY() / aEdgeVector.getY());
                        const double fZero(0.0);
                        const double fOne(1.0);

                        if(fTools::more(rfCut, fZero) && fTools::less(rfCut, fOne))
                        {
                            return true;
                        }
                    }
                }

                return false;
            }

            bool findCutOnEdge(
                const B2DPoint& rStartA,
                const B2DPoint& rEndA,
                const B2DPoint& rStartB,
                const B2DPoint& rEndB,
                double& rfCutA,
                double& rfCutB)
            {
                const bool bNullLengthA(rStartA.equal(rEndA));
                const bool bNullLengthB(rStartB.equal(rEndB));

                if(bNullLengthA || bNullLengthB)
                {
                    // at least one of the edges has zero-length
                    if(bNullLengthA && bNullLengthB)
                    {
                        // bot have zero-length, nothing to do
                    }
                    else if(bNullLengthA)
                    {
                        // PointA may lie on EdgeB
                        return findTouchOnEdge(
                            rStartB,
                            rEndB,
                            rStartA,
                            rfCutB);
                    }
                    else // if(bNullLengthB)
                    {
                        // PointB may lie on EdgeA
                        return findTouchOnEdge(
                            rStartA,
                            rEndA,
                            rStartB,
                            rfCutA);
                    }
                }
                else
                {
                    // no zero-length edges from here
                    // no common start/end points, this can be no cuts
                    if(!(rStartB.equal(rStartA) || rStartB.equal(rEndA) || rEndB.equal(rStartA) || rEndB.equal(rEndA)))
                    {
                        const B2DVector aVecA(rEndA - rStartA);
                        const B2DVector aVecB(rEndB - rStartB);
                        rfCutA = aVecA.cross(aVecB);

                        if(!fTools::equalZero(rfCutA))
                        {
                            const double fZero(0.0);
                            const double fOne(1.0);
                            rfCutA = (aVecB.getY() * (rStartB.getX() - rStartA.getX()) + aVecB.getX() * (rStartA.getY() - rStartB.getY())) / rfCutA;

                            if (fTools::betweenOrEqualEither(rfCutA, fZero, fOne))
                            {
                                // it's a candidate, but also need to test parameter value of cut on line 2
                                // choose the more precise version
                                if(fabs(aVecB.getX()) > fabs(aVecB.getY()))
                                {
                                    rfCutB = (rStartA.getX() + (rfCutA * aVecA.getX()) - rStartB.getX()) / aVecB.getX();
                                }
                                else
                                {
                                    rfCutB = (rStartA.getY() + (rfCutA * aVecA.getY()) - rStartB.getY()) / aVecB.getY();
                                }

                                if (fTools::betweenOrEqualEither(rfCutB, fZero, fOne))
                                {
                                    // cut is in range, add point. Two edges can have only one cut, but
                                    // add a cut point to each list. The lists may be the same for
                                    // self intersections.
                                    return true;
                                }
                            }
                        }
                    }
                }

                return false;
            }

            void splitEdge(Edge& rEdge)
            {
                if(!rEdge.getTemporaryPoints().empty())
                {
                    std::vector< TemporaryPoint > aTemporaryPoints(rEdge.getTemporaryPoints());
                    std::sort(aTemporaryPoints.begin(), aTemporaryPoints.end());
                    Edge* pCurrent(&rEdge);
                    double fLeftStart(0.0);

                    for(auto const& TempPoint : aTemporaryPoints)
                    {
                        if(!TempPoint.getB2DPoint().equal(pCurrent->getNodeA().getB2DPoint())
                            && !TempPoint.getB2DPoint().equal(pCurrent->getNodeB().getB2DPoint()))
                        {
                            B2DCubicBezier aB2DCubicBezierA(
                                pCurrent->getNodeA().getB2DPoint(),
                                pCurrent->getControlA(),
                                pCurrent->getControlB(),
                                pCurrent->getNodeB().getB2DPoint());
                            B2DCubicBezier aB2DCubicBezierB;
                            const double fRelativeSplitPoint((TempPoint.getCut() - fLeftStart) / (1.0 - fLeftStart));

                            aB2DCubicBezierA.split(
                                fRelativeSplitPoint,
                                &aB2DCubicBezierA,
                                &aB2DCubicBezierB);

                            // use the real common split-point here, that may differ from the
                            // calculated SplitPoint using fRelativeSplitPoint. Prepare a Vector
                            // to also correct associated control points
                            const B2DVector aDelta(TempPoint.getB2DPoint() - aB2DCubicBezierA.getEndPoint());

                            // create or find Node
                            Node* pSplit(&const_cast<Node&>(*maNodes.emplace(TempPoint.getB2DPoint()).first));
                            Node& rOldNodeB(pCurrent->getNodeB());

                            pCurrent->modifyNodeB(
                                *pSplit,
                                aB2DCubicBezierA.getControlPointA(),
                                aB2DCubicBezierA.getControlPointB() + aDelta);

                            Edge* pNewEdge(
                                new Edge(
                                    *pSplit,
                                    rOldNodeB,
                                    aB2DCubicBezierB.getControlPointA() + aDelta,
                                    aB2DCubicBezierB.getControlPointB(),
                                    pCurrent->getOriginalData()));

                            maEdges.push_back(pNewEdge);

                            fLeftStart = TempPoint.getCut();
                            pCurrent = pNewEdge;
                        }
                    }
                }
            }

            void findCuts(
                const B2DCubicBezier& rB2DCubicBezierA,
                const B2DCubicBezier& rB2DCubicBezierB,
                std::vector< double >& rCutsA,
                std::vector< double >& rCutsB,
                double fMinA,
                double fMaxA,
                double fMinB,
                double fMaxB,
                double fMinArea)
            {
                const B2DRange aRangeA(rB2DCubicBezierA.getRange());
                const B2DRange aRangeB(rB2DCubicBezierB.getRange());

                if(aRangeA.overlaps(aRangeB))
                {
                    const double fAreaA(aRangeA.getWidth() * aRangeA.getHeight());
                    const double fAreaB(aRangeB.getWidth() * aRangeB.getHeight());

                    if(fAreaA + fAreaB < fMinArea)
                    {
                        double fCutA(0.0);
                        double fCutB(0.0);

                        if(findCutOnEdge(
                            rB2DCubicBezierA.getStartPoint(),
                            rB2DCubicBezierA.getEndPoint(),
                            rB2DCubicBezierB.getStartPoint(),
                            rB2DCubicBezierB.getEndPoint(),
                            fCutA,
                            fCutB))
                        {
                            rCutsA.push_back(fMinA + ((fMaxA - fMinA) * fCutA));
                            rCutsB.push_back(fMinB + ((fMaxB - fMinB) * fCutB));
                        }
                    }
                    else
                    {
                        B2DCubicBezier aB2DCubicBezierLA;
                        B2DCubicBezier aB2DCubicBezierRA;
                        rB2DCubicBezierA.split(0.5, &aB2DCubicBezierLA, &aB2DCubicBezierRA);

                        B2DCubicBezier aB2DCubicBezierLB;
                        B2DCubicBezier aB2DCubicBezierRB;
                        rB2DCubicBezierB.split(0.5, &aB2DCubicBezierLB, &aB2DCubicBezierRB);

                        findCuts(
                            aB2DCubicBezierLA,
                            aB2DCubicBezierLB,
                            rCutsA,
                            rCutsB,
                            fMinA,
                            fMinA + ((fMaxA - fMinA) * 0.5),
                            fMinB,
                            fMinB + ((fMaxB - fMinB) * 0.5),
                            fMinArea);

                        findCuts(
                            aB2DCubicBezierRA,
                            aB2DCubicBezierLB,
                            rCutsA,
                            rCutsB,
                            fMinA + ((fMaxA - fMinA) * 0.5),
                            fMaxA,
                            fMinB,
                            fMinB + ((fMaxB - fMinB) * 0.5),
                            fMinArea);

                        findCuts(
                            aB2DCubicBezierLA,
                            aB2DCubicBezierRB,
                            rCutsA,
                            rCutsB,
                            fMinA,
                            fMinA + ((fMaxA - fMinA) * 0.5),
                            fMinB + ((fMaxB - fMinB) * 0.5),
                            fMaxB,
                            fMinArea);

                        findCuts(
                            aB2DCubicBezierRA,
                            aB2DCubicBezierRB,
                            rCutsA,
                            rCutsB,
                            fMinA + ((fMaxA - fMinA) * 0.5),
                            fMaxA,
                            fMinB + ((fMaxB - fMinB) * 0.5),
                            fMaxB,
                            fMinArea);
                    }
                }
            }

            void addPointsAtCuts(
                Edge& rEdgeA,
                Edge& rEdgeB)
            {
                if(&rEdgeA != &rEdgeB)
                {
                    if(rEdgeA.isBezier() || rEdgeB.isBezier())
                    {
                        const B2DCubicBezier aB2DCubicBezierA(
                            rEdgeA.getNodeA().getB2DPoint(),
                            rEdgeA.getControlA(),
                            rEdgeA.getControlB(),
                            rEdgeA.getNodeB().getB2DPoint());
                        const B2DCubicBezier aB2DCubicBezierB(
                            rEdgeB.getNodeA().getB2DPoint(),
                            rEdgeB.getControlA(),
                            rEdgeB.getControlB(),
                            rEdgeB.getNodeB().getB2DPoint());
                        const B2DRange aRangeA(aB2DCubicBezierA.getRange());
                        const B2DRange aRangeB(aB2DCubicBezierB.getRange());

                        if(aRangeA.overlaps(aRangeB))
                        {
                            std::vector< double > cutsA;
                            std::vector< double > cutsB;
                            const double fAreaA(aRangeA.getWidth() * aRangeA.getHeight());
                            const double fAreaB(aRangeB.getWidth() * aRangeB.getHeight());
                            const double fArea((fAreaA + fAreaB) * 0.0001);

                            findCuts(
                                aB2DCubicBezierA,
                                aB2DCubicBezierB,
                                cutsA,
                                cutsB,
                                0.0,
                                1.0,
                                0.0,
                                1.0,
                                fArea);

                            if(!cutsA.empty() && !cutsB.empty() && cutsA.size() == cutsB.size())
                            {
                                for(size_t a(0); a < cutsA.size(); a++)
                                {
                                    const double fCutA(cutsA[a]);
                                    const double fCutB(cutsB[a]);
                                    B2DPoint aPointA(aB2DCubicBezierA.getStartPoint());
                                    B2DPoint aPointB(aB2DCubicBezierB.getStartPoint());

                                    if(0.0 == fCutA)
                                    {
                                        // already set as default
                                    }
                                    else if(1.0 == fCutA)
                                    {
                                        aPointA = aB2DCubicBezierA.getEndPoint();
                                    }
                                    else
                                    {
                                        aPointA = aB2DCubicBezierA.interpolatePoint(fCutA);
                                    }

                                    if(0.0 == fCutB)
                                    {
                                        // already set as default
                                    }
                                    else if(1.0 == fCutB)
                                    {
                                        aPointB = aB2DCubicBezierB.getEndPoint();
                                    }
                                    else
                                    {
                                        aPointB = aB2DCubicBezierB.interpolatePoint(fCutB);
                                    }

                                    const B2DPoint aPoint(aPointA != aPointB
                                        ? (aPointA + aPointB) * 0.5
                                        : aPointA);

                                    rEdgeA.getTemporaryPoints().emplace_back(
                                        aPoint,
                                        cutsA[a]);
                                    rEdgeB.getTemporaryPoints().emplace_back(
                                        aPoint,
                                        cutsB[a]);
                                }
                            }
                        }
                    }
                    else
                    {
                        double fCutA(0.0);
                        double fCutB(0.0);

                        if(findCutOnEdge(
                            rEdgeA.getNodeA().getB2DPoint(),
                            rEdgeA.getNodeB().getB2DPoint(),
                            rEdgeB.getNodeA().getB2DPoint(),
                            rEdgeB.getNodeB().getB2DPoint(),
                            fCutA,
                            fCutB))
                        {
                            B2DPoint aPointA(rEdgeA.getNodeA().getB2DPoint());
                            B2DPoint aPointB(rEdgeB.getNodeA().getB2DPoint());

                            if(0.0 == fCutA)
                            {
                                // already set as default
                            }
                            else if(1.0 == fCutA)
                            {
                                aPointA = rEdgeA.getNodeB().getB2DPoint();
                            }
                            else
                            {
                                const B2DVector aVecA(rEdgeA.getNodeB().getB2DPoint() - rEdgeA.getNodeA().getB2DPoint());
                                aPointA += fCutA * aVecA;
                            }

                            if(0.0 == fCutB)
                            {
                                // already set as default
                            }
                            else if(1.0 == fCutB)
                            {
                                aPointB = rEdgeB.getNodeB().getB2DPoint();
                            }
                            else
                            {
                                const B2DVector aVecB(rEdgeB.getNodeB().getB2DPoint() - rEdgeB.getNodeA().getB2DPoint());
                                aPointB += fCutB * aVecB;
                            }

                            const B2DPoint aPoint(aPointA != aPointB
                                ? (aPointA + aPointB) * 0.5
                                : aPointA);

                            rEdgeA.getTemporaryPoints().emplace_back(
                                aPoint,
                                fCutA);
                            rEdgeB.getTemporaryPoints().emplace_back(
                                aPoint,
                                fCutB);
                        }
                    }
                }
            }

            void addPointsAtCuts(
                Edge& rEdge)
            {
                if(rEdge.isBezier())
                {
                    const B2DCubicBezier aB2DCubicBezier(
                        rEdge.getNodeA().getB2DPoint(),
                        rEdge.getControlA(),
                        rEdge.getControlB(),
                        rEdge.getNodeB().getB2DPoint());
                    std::vector< double > aAllResults;

                    aAllResults.reserve(4);
                    aB2DCubicBezier.getAllExtremumPositions(aAllResults);

                    const sal_uInt32 nCount(aAllResults.size());

                    if(0 != nCount)
                    {
                        std::sort(aAllResults.begin(), aAllResults.end());
                        std::vector< Node* > aNodes;
                        std::vector< Edge* > aEdges;

                        Node* pStartNode(new Node(rEdge.getNodeA().getB2DPoint()));
                        Node* pEndNode(new Node(rEdge.getNodeB().getB2DPoint()));
                        aNodes.push_back(pStartNode);
                        aNodes.push_back(pEndNode);
                        Edge* pCurrent(
                            new Edge(
                                *pStartNode,
                                *pEndNode,
                                rEdge.getControlA(),
                                rEdge.getControlB(),
                                rEdge.getOriginalData()));
                        aEdges.push_back(pCurrent);
                        double fLeftStart(0.0);

                        for(auto const& rResult : aAllResults)
                        {
                            B2DCubicBezier aB2DCubicBezierA(
                                pCurrent->getNodeA().getB2DPoint(),
                                pCurrent->getControlA(),
                                pCurrent->getControlB(),
                                pCurrent->getNodeB().getB2DPoint());
                            B2DCubicBezier aB2DCubicBezierB;
                            const double fRelativeSplitPoint((rResult - fLeftStart) / (1.0 - fLeftStart));

                            aB2DCubicBezierA.split(
                                fRelativeSplitPoint,
                                &aB2DCubicBezierA,
                                &aB2DCubicBezierB);

                            Node* pSplit(new Node(aB2DCubicBezierA.getEndPoint()));
                            aNodes.push_back(pSplit);
                            Node& rOldNodeB(pCurrent->getNodeB());

                            pCurrent->modifyNodeB(
                                *pSplit,
                                aB2DCubicBezierA.getControlPointA(),
                                aB2DCubicBezierA.getControlPointB());

                            Edge* pNewEdge(
                                new Edge(
                                    *pSplit,
                                    rOldNodeB,
                                    aB2DCubicBezierB.getControlPointA(),
                                    aB2DCubicBezierB.getControlPointB(),
                                    pCurrent->getOriginalData()));

                            aEdges.push_back(pNewEdge);

                            fLeftStart = rResult;
                            pCurrent = pNewEdge;
                        }

                        for(size_t a(0); a < aEdges.size(); a++)
                        {
                            for(size_t b(a + 1); b < aEdges.size(); b++)
                            {
                                // normal intersection between two edges
                                Edge& rEdgeA(*aEdges[a]);
                                Edge& rEdgeB(*aEdges[b]);

                                if(rEdgeA.getRange().overlaps(rEdgeB.getRange()))
                                {
                                    addPointsAtCuts(
                                        rEdgeA,
                                        rEdgeB);
                                }
                            }
                        }

                        for(size_t c(0); c < aEdges.size(); c++)
                        {
                            Edge& rEdge(*aEdges[c]);

                            if(!rEdge.getTemporaryPoints().empty())
                            {
                                const double fLeft(c - 1 < 0 ? 0.0 : aAllResults[c - 1]);
                                const double fRight(c + 1 > aAllResults.size() ? 1.0 : aAllResults[c]);
                                const double fScaleFactor(1.0 / (fRight - fLeft));

                                for(const auto& rTemporaryPoint : rEdge.getTemporaryPoints())
                                {
                                    const double fNewCut(fLeft + (rTemporaryPoint.getCut() * fScaleFactor));

                                    rEdge.getTemporaryPoints().emplace_back(
                                        rTemporaryPoint.getB2DPoint(),
                                        fNewCut);
                                }
                            }
                        }

                        while(!aEdges.empty())
                        {
                            Edge* pCand(aEdges.back());
                            aEdges.pop_back();
                            delete pCand;
                        }

                        while(!aNodes.empty())
                        {
                            Node* pCand(aNodes.back());
                            aNodes.pop_back();
                            delete pCand;
                        }
                    }
                }
            }

            void addPointsAtCuts()
            {
                for(size_t a(0); a < maEdges.size(); a++)
                {
                    for(size_t b(a); b < maEdges.size(); b++)
                    {
                        if(a == b)
                        {
                            // test self-intersection, only possible for
                            // bezier segments
                            Edge& rEdge(*maEdges[a]);

                            addPointsAtCuts(rEdge);
                        }
                        else
                        {
                            // normal intersection between two edges
                            Edge& rEdgeA(*maEdges[a]);
                            Edge& rEdgeB(*maEdges[b]);

                            if(rEdgeA.getRange().overlaps(rEdgeB.getRange()))
                            {
                                addPointsAtCuts(
                                    rEdgeA,
                                    rEdgeB);
                            }
                        }
                    }
                }

                for(size_t a(0); a < maEdges.size(); a++)
                {
                    splitEdge(*maEdges[a]);
                }
            }

            void consumeEdges()
            {
                for(const auto& Edge : maEdges)
                {
                    const B2DPoint aCenter(Edge->getCenter());

                    for(const auto& OriginalData : maOriginalDatas)
                    {
                        if(&OriginalData != &Edge->getOriginalData())
                        {
                            const bool bInside(utils::isInside(OriginalData.getB2DPolyPolygon(), aCenter, true));

                            if(bInside && OriginalData.isPositive())
                            {
                                // Edge can be deleted if inside one of the polygons
                                Edge->consume();
                                break;
                            }
                            else if(!bInside && !OriginalData.isPositive())
                            {
                                // Edge can be deleted if outside one of the polygons
                                Edge->consume();
                                break;
                            }
                        }
                    }
                }
            }

            B2DPolyPolygon extract()
            {
                B2DPolyPolygon aRetval;

                while(true)
                {
                    Node* pStartNode(nullptr);

                    for(std::set<Node>::iterator aNode(maNodes.begin()); nullptr == pStartNode && aNode != maNodes.end(); aNode++)
                    {
                        if(aNode->hasEdges())
                        {
                            pStartNode = &const_cast<Node&>(*aNode);
                        }
                    }

                    if(nullptr == pStartNode)
                    {
                        return aRetval;
                    }

                    B2DPolygon aNew;
                    Node* pCurrentNode(pStartNode);
                    aNew.append(pCurrentNode->getB2DPoint());

                    while(pCurrentNode->hasEdges())
                    {
                        Edge* pEdge(pCurrentNode->getEdge());
                        aNew.setNextControlPoint(aNew.count() - 1, pEdge->getControl(*pCurrentNode));
                        Node* pNextNode(&pEdge->getOther(*pCurrentNode));

                        if(pStartNode == pNextNode)
                        {
                            aNew.setPrevControlPoint(0, pEdge->getControl(*pNextNode));
                        }
                        else
                        {
                            aNew.append(pNextNode->getB2DPoint());
                            aNew.setPrevControlPoint(aNew.count() - 1, pEdge->getControl(*pNextNode));
                        }

                        pEdge->consume();
                        pCurrentNode = pNextNode;
                    }

                    aNew.setClosed(true);
                    aRetval.append(aNew);
                }

                return aRetval;
            }
        };
    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace utils
    {

        B2DPolyPolygon solveCrossovers(const B2DPolyPolygon& rCandidate)
        {
            if(0 != rCandidate.count())
            {
                cutter aCutter;

                aCutter.add(rCandidate, true);
                aCutter.addPointsAtCuts();

                return aCutter.extract();
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon stripNeutralPolygons(const B2DPolyPolygon& rCandidate)
        {
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0); a < rCandidate.count(); a++)
            {
                const B2DPolygon aCandidate(rCandidate.getB2DPolygon(a));

                if(utils::getOrientation(aCandidate) != B2VectorOrientation::Neutral)
                {
                    aRetval.append(aCandidate);
                }
            }

            return aRetval;
        }

        B2DPolyPolygon createNonzeroConform(const B2DPolyPolygon& rCandidate)
        {
            // remove all self-intersections and intersections
            B2DPolyPolygon aCandidate(basegfx::utils::solveCrossovers(rCandidate));

            // cleanup evtl. neutral polygons
            aCandidate = basegfx::utils::stripNeutralPolygons(aCandidate);

            // remove all polygons which have the same orientation as the polygon they are directly contained in
            const sal_uInt32 nCount(aCandidate.count());

            if(nCount > 1)
            {
                struct StripHelper
                {
                    B2DRange                                maRange;
                    sal_Int32                               mnDepth;
                    B2VectorOrientation                     meOrinetation;
                };

                sal_uInt32 a, b;
                std::vector< StripHelper > aHelpers;
                aHelpers.resize(nCount);

                for(a = 0; a < nCount; a++)
                {
                    const B2DPolygon aCand(aCandidate.getB2DPolygon(a));
                    StripHelper* pNewHelper = &(aHelpers[a]);
                    pNewHelper->maRange = utils::getRange(aCand);
                    pNewHelper->meOrinetation = utils::getOrientation(aCand);

                    // initialize with own orientation
                    pNewHelper->mnDepth = (pNewHelper->meOrinetation == B2VectorOrientation::Negative ? -1 : 1);
                }

                for(a = 0; a < nCount - 1; a++)
                {
                    const B2DPolygon aCandA(aCandidate.getB2DPolygon(a));
                    StripHelper& rHelperA = aHelpers[a];

                    for(b = a + 1; b < nCount; b++)
                    {
                        const B2DPolygon aCandB(aCandidate.getB2DPolygon(b));
                        StripHelper& rHelperB = aHelpers[b];
                        const bool bAInB(rHelperB.maRange.isInside(rHelperA.maRange) && utils::isInside(aCandB, aCandA, true));

                        if(bAInB)
                        {
                            // A is inside B, add orientation of B to A
                            rHelperA.mnDepth += (rHelperB.meOrinetation == B2VectorOrientation::Negative ? -1 : 1);
                        }

                        const bool bBInA(rHelperA.maRange.isInside(rHelperB.maRange) && utils::isInside(aCandA, aCandB, true));

                        if(bBInA)
                        {
                            // B is inside A, add orientation of A to B
                            rHelperB.mnDepth += (rHelperA.meOrinetation == B2VectorOrientation::Negative ? -1 : 1);
                        }
                    }
                }

                const B2DPolyPolygon aSource(aCandidate);
                aCandidate.clear();

                for(a = 0; a < nCount; a++)
                {
                    const StripHelper& rHelper = aHelpers[a];
                    // for contained unequal oriented polygons sum will be 0
                    // for contained equal it will be >=2 or <=-2
                    // for free polygons (not contained) it will be 1 or -1
                    // -> accept all which are >=-1 && <= 1
                    bool bAcceptEntry(rHelper.mnDepth >= -1 && rHelper.mnDepth <= 1);

                    if(bAcceptEntry)
                    {
                        aCandidate.append(aSource.getB2DPolygon(a));
                    }
                }
            }

            return aCandidate;
        }

        B2DPolyPolygon prepareForPolygonOperation(const B2DPolyPolygon& rCandidate)
        {
            cutter aCutter;

            aCutter.add(rCandidate, true);
            aCutter.addPointsAtCuts();

            B2DPolyPolygon aRetval(aCutter.extract());
            aRetval = stripNeutralPolygons(aRetval);
            return correctOrientations(aRetval);
        }

        B2DPolyPolygon solvePolygonOperationOr(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB)
        {
            if(!rCandidateA.count())
            {
                return rCandidateB;
            }
            else if(!rCandidateB.count())
            {
                return rCandidateA;
            }
            else
            {
                cutter aCutter;

                aCutter.add(rCandidateA, true);
                aCutter.add(rCandidateB, true);
                aCutter.addPointsAtCuts();
                aCutter.consumeEdges();

                return aCutter.extract();
            }
        }

        B2DPolyPolygon solvePolygonOperationXor(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB)
        {
            if(!rCandidateA.count())
            {
                return rCandidateB;
            }
            else if(!rCandidateB.count())
            {
                return rCandidateA;
            }
            else
            {
                static bool bCheckNewClip(true);

                if(bCheckNewClip)
                {
                    // XOR is pretty simple: By definition it is the simple concatenation of
                    // the single polygons since we imply XOR fill rule (even-odd).
                    B2DPolyPolygon aRetval(rCandidateA);

                    aRetval.append(rCandidateB);

                    return aRetval;
                }
                else
                {
                    // XOR is pretty simple: By definition it is the simple concatenation of
                    // the single polygons since we imply XOR fill rule. Make it intersection-free
                    // and correct orientations
                    B2DPolyPolygon aRetval(rCandidateA);

                    aRetval.append(rCandidateB);
                    aRetval = solveCrossovers(aRetval);
                    aRetval = stripNeutralPolygons(aRetval);

                    return correctOrientations(aRetval);
                }
            }
        }

        B2DPolyPolygon solvePolygonOperationAnd(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB)
        {
            if(!rCandidateA.count())
            {
                return B2DPolyPolygon();
            }
            else if(!rCandidateB.count())
            {
                return B2DPolyPolygon();
            }
            else
            {
                cutter aCutter;

                aCutter.add(rCandidateA, false);
                aCutter.add(rCandidateB, false);
                aCutter.addPointsAtCuts();
                aCutter.consumeEdges();

                return aCutter.extract();
            }
        }

        B2DPolyPolygon solvePolygonOperationDiff(const B2DPolyPolygon& rCandidateA, const B2DPolyPolygon& rCandidateB)
        {
            if(!rCandidateA.count())
            {
                return B2DPolyPolygon();
            }
            else if(!rCandidateB.count())
            {
                return rCandidateA;
            }
            else
            {
                cutter aCutter;

                aCutter.add(rCandidateA, false);
                aCutter.add(rCandidateB, true);
                aCutter.addPointsAtCuts();
                aCutter.consumeEdges();

                return aCutter.extract();
            }
        }

        B2DPolyPolygon mergeToSinglePolyPolygon(const B2DPolyPolygonVector& rInput)
        {
            cutter aCutter;

            for(const basegfx::B2DPolyPolygon& aCandidate : rInput)
            {
                aCutter.add(aCandidate, true);
            }

            aCutter.addPointsAtCuts();
            aCutter.consumeEdges();

            return aCutter.extract();
        }

    } // end of namespace utils
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
