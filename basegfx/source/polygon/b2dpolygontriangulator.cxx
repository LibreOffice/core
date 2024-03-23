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

#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <algorithm>

namespace basegfx
{
    namespace
    {
        class EdgeEntry
        {
            EdgeEntry*                              mpNext;
            B2DPoint                                maStart;
            B2DPoint                                maEnd;
            double                                  mfAtan2;

        public:
            EdgeEntry(const B2DPoint& rStart, const B2DPoint& rEnd)
            :   mpNext(nullptr),
                maStart(rStart),
                maEnd(rEnd),
                mfAtan2(0.0)
            {
                // make sure edge goes down. If horizontal, let it go to the right (left-handed).
                bool bSwap(false);

                if(::basegfx::fTools::equal(maStart.getY(), maEnd.getY()))
                {
                    if(maStart.getX() > maEnd.getX())
                    {
                        bSwap = true;
                    }
                }
                else if(maStart.getY() > maEnd.getY())
                {
                    bSwap = true;
                }

                if(bSwap)
                {
                    maStart = rEnd;
                    maEnd = rStart;
                }

                mfAtan2 = atan2(maEnd.getY() - maStart.getY(), maEnd.getX() - maStart.getX());
            }

            bool operator<(const EdgeEntry& rComp) const
            {
                if(::basegfx::fTools::equal(maStart.getY(), rComp.maStart.getY()))
                {
                    if(::basegfx::fTools::equal(maStart.getX(), rComp.maStart.getX()))
                    {
                        // same in x and y -> same start point. Sort emitting vectors from left to right.
                        return (mfAtan2 > rComp.mfAtan2);
                    }

                    return (maStart.getX() < rComp.maStart.getX());
                }

                return (maStart.getY() < rComp.maStart.getY());
            }

            bool operator==(const EdgeEntry& rComp) const
            {
                return (maStart.equal(rComp.maStart) && maEnd.equal(rComp.maEnd));
            }

            const B2DPoint& getStart() const { return maStart; }
            const B2DPoint& getEnd() const { return maEnd; }

            EdgeEntry* getNext() const { return mpNext; }
            void setNext(EdgeEntry* pNext) { mpNext = pNext; }
        };

        typedef std::vector< EdgeEntry > EdgeEntries;

        class Triangulator
        {
            EdgeEntry*                                      mpList;
            EdgeEntries                                     maStartEntries;
            std::vector< std::unique_ptr<EdgeEntry> >       maNewEdgeEntries;
            triangulator::B2DTriangleVector                 maResult;

            void handleClosingEdge(const B2DPoint& rStart, const B2DPoint& rEnd);
            bool CheckPointInTriangle(EdgeEntry* pEdgeA, EdgeEntry const * pEdgeB, const B2DPoint& rTestPoint);
            void createTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC);

        public:
            explicit Triangulator(const B2DPolyPolygon& rCandidate);

            const triangulator::B2DTriangleVector& getResult() const { return maResult; }
        };

        void Triangulator::handleClosingEdge(const B2DPoint& rStart, const B2DPoint& rEnd)
        {
            // create an entry, else the comparison might use the wrong edges
            EdgeEntry aNew(rStart, rEnd);
            EdgeEntry* pCurr = mpList;
            EdgeEntry* pPrev = nullptr;

            while(pCurr
                && pCurr->getStart().getY() <= aNew.getStart().getY()
                && *pCurr != aNew)
            {
                pPrev = pCurr;
                pCurr = pCurr->getNext();
            }

            if(pCurr && *pCurr == aNew)
            {
                // found closing edge, remove
                if(pPrev)
                {
                    pPrev->setNext(pCurr->getNext());
                }
                else
                {
                    mpList = pCurr->getNext();
                }
            }
            else
            {
                // insert closing edge
                EdgeEntry* pNew = new EdgeEntry(aNew);
                maNewEdgeEntries.emplace_back(pNew);
                pCurr = mpList;
                pPrev = nullptr;

                while(pCurr && *pCurr < *pNew)
                {
                    pPrev = pCurr;
                    pCurr = pCurr->getNext();
                }

                if(pPrev)
                {
                    pNew->setNext(pPrev->getNext());
                    pPrev->setNext(pNew);
                }
                else
                {
                    pNew->setNext(mpList);
                    mpList = pNew;
                }
            }
        }

        bool Triangulator::CheckPointInTriangle(EdgeEntry* pEdgeA, EdgeEntry const * pEdgeB, const B2DPoint& rTestPoint)
        {
            // inside triangle or on edge?
            if(!utils::isPointInTriangle(pEdgeA->getStart(), pEdgeA->getEnd(), pEdgeB->getEnd(), rTestPoint, true))
                return true;

            // but not on point
            if(!rTestPoint.equal(pEdgeA->getEnd()) && !rTestPoint.equal(pEdgeB->getEnd()))
            {
                // found point in triangle -> split triangle inserting two edges
                EdgeEntry* pStart = new EdgeEntry(pEdgeA->getStart(), rTestPoint);
                EdgeEntry* pEnd = new EdgeEntry(*pStart);
                maNewEdgeEntries.emplace_back(pStart);
                maNewEdgeEntries.emplace_back(pEnd);

                pStart->setNext(pEnd);
                pEnd->setNext(pEdgeA->getNext());
                pEdgeA->setNext(pStart);

                return false;
            }

            return true;
        }

        void Triangulator::createTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC)
        {
            maResult.emplace_back(
                rA,
                rB,
                rC);
        }

        // consume as long as there are edges
        Triangulator::Triangulator(const B2DPolyPolygon& rCandidate)
        :   mpList(nullptr)
        {
            // add all available edges to the single linked local list which will be sorted
            // by Y,X,atan2 when adding nodes
            if(rCandidate.count())
            {
                for(const auto& rPolygonCandidate : rCandidate)
                {
                    const sal_uInt32 nCount {rPolygonCandidate.count()};

                    if(nCount > 2)
                    {
                        B2DPoint aPrevPnt(rPolygonCandidate.getB2DPoint(nCount - 1));

                        for(sal_uInt32 b(0); b < nCount; b++)
                        {
                            B2DPoint aNextPnt(rPolygonCandidate.getB2DPoint(b));

                            if( !aPrevPnt.equal(aNextPnt) )
                            {
                                maStartEntries.emplace_back(aPrevPnt, aNextPnt);
                            }

                            aPrevPnt = aNextPnt;
                        }
                    }
                }

                if(!maStartEntries.empty())
                {
                    // sort initial list
                    std::sort(maStartEntries.begin(), maStartEntries.end());

                    // insert to own simply linked list
                    EdgeEntries::iterator aPos(maStartEntries.begin());
                    mpList = &(*aPos++);
                    EdgeEntry* pLast = mpList;

                    while(aPos != maStartEntries.end())
                    {
                        EdgeEntry* pEntry = &(*aPos++);
                        pLast->setNext(pEntry);
                        pLast = pEntry;
                    }
                }
            }

            while(mpList)
            {
                if(mpList->getNext() && mpList->getNext()->getStart().equal(mpList->getStart()))
                {
                    // next candidate. There are two edges and start point is equal.
                    // Length is not zero.
                    EdgeEntry* pEdgeA = mpList;
                    EdgeEntry* pEdgeB = pEdgeA->getNext();

                    if( pEdgeA->getEnd().equal(pEdgeB->getEnd()) )
                    {
                        // start and end equal -> neutral triangle, delete both
                        mpList = pEdgeB->getNext();
                    }
                    else
                    {
                        const B2DVector aLeft(pEdgeA->getEnd() - pEdgeA->getStart());
                        const B2DVector aRight(pEdgeB->getEnd() - pEdgeA->getStart());

                        if(getOrientation(aLeft, aRight) == B2VectorOrientation::Neutral)
                        {
                            // edges are parallel and have different length -> neutral triangle,
                            // delete both edges and handle closing edge
                            mpList = pEdgeB->getNext();
                            handleClosingEdge(pEdgeA->getEnd(), pEdgeB->getEnd());
                        }
                        else
                        {
                            // not parallel, look for points inside
                            B2DRange aRange(pEdgeA->getStart(), pEdgeA->getEnd());
                            aRange.expand(pEdgeB->getEnd());
                            EdgeEntry* pTestEdge = pEdgeB->getNext();
                            bool bNoPointInTriangle(true);

                            // look for start point in triangle
                            while(bNoPointInTriangle && pTestEdge)
                            {
                                if(aRange.getMaxY() < pTestEdge->getStart().getY())
                                {
                                    // edge is below test range and edges are sorted -> stop looking
                                    break;
                                }
                                else
                                {
                                    // do not look for edges with same start point, they are sorted and cannot end inside.
                                    if(!pTestEdge->getStart().equal(pEdgeA->getStart()))
                                    {
                                        if(aRange.isInside(pTestEdge->getStart()))
                                        {
                                            bNoPointInTriangle = CheckPointInTriangle(pEdgeA, pEdgeB, pTestEdge->getStart());
                                        }
                                    }
                                }

                                // next candidate
                                pTestEdge = pTestEdge->getNext();
                            }

                            if(bNoPointInTriangle)
                            {
                                // look for end point in triangle
                                pTestEdge = pEdgeB->getNext();

                                while(bNoPointInTriangle && pTestEdge)
                                {
                                    if(aRange.getMaxY() < pTestEdge->getStart().getY())
                                    {
                                        // edge is below test range and edges are sorted -> stop looking
                                        break;
                                    }
                                    else
                                    {
                                        // do not look for edges with same end point, they are sorted and cannot end inside.
                                        if(!pTestEdge->getEnd().equal(pEdgeA->getStart()))
                                        {
                                            if(aRange.isInside(pTestEdge->getEnd()))
                                            {
                                                bNoPointInTriangle = CheckPointInTriangle(pEdgeA, pEdgeB, pTestEdge->getEnd());
                                            }
                                        }
                                    }

                                    // next candidate
                                    pTestEdge = pTestEdge->getNext();
                                }
                            }

                            if(bNoPointInTriangle)
                            {
                                // create triangle, remove edges, handle closing edge
                                mpList = pEdgeB->getNext();
                                createTriangle(pEdgeA->getStart(), pEdgeB->getEnd(), pEdgeA->getEnd());
                                handleClosingEdge(pEdgeA->getEnd(), pEdgeB->getEnd());
                            }
                        }
                    }
                }
                else
                {
                    // only one entry at start point, delete it
                    mpList = mpList->getNext();
                }
            }
        }

    } // end of anonymous namespace
} // end of namespace basegfx

namespace basegfx::triangulator
{
        B2DTriangleVector triangulate(const B2DPolygon& rCandidate)
        {
            B2DTriangleVector aRetval;

            // subdivide locally (triangulate does not work with beziers), remove double and neutral points
            B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? utils::adaptiveSubdivideByAngle(rCandidate) : rCandidate);
            aCandidate.removeDoublePoints();
            aCandidate = utils::removeNeutralPoints(aCandidate);

            if(aCandidate.count() == 2)
            {
                // candidate IS a triangle, just append
                aRetval.emplace_back(
                    aCandidate.getB2DPoint(0),
                    aCandidate.getB2DPoint(1),
                    aCandidate.getB2DPoint(2));
            }
            else if(aCandidate.count() > 2)
            {
                if(utils::isConvex(aCandidate))
                {
                    // polygon is convex, just use a triangle fan
                    utils::addTriangleFan(aCandidate, aRetval);
                }
                else
                {
                    // polygon is concave.
                    const B2DPolyPolygon aCandPolyPoly(aCandidate);
                    Triangulator aTriangulator(aCandPolyPoly);

                    aRetval = aTriangulator.getResult();
                }
            }

            return aRetval;
        }

        B2DTriangleVector triangulate(const B2DPolyPolygon& rCandidate)
        {
            B2DTriangleVector aRetval;

            // subdivide locally (triangulate does not work with beziers)
            B2DPolyPolygon aCandidate(rCandidate.areControlPointsUsed() ? utils::adaptiveSubdivideByAngle(rCandidate) : rCandidate);

            if(aCandidate.count() == 1)
            {
                // single polygon -> single polygon triangulation
                const B2DPolygon& aSinglePolygon(aCandidate.getB2DPolygon(0));

                aRetval = triangulate(aSinglePolygon);
            }
            else
            {
                Triangulator aTriangulator(aCandidate);

                aRetval = aTriangulator.getResult();
            }

            return aRetval;
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
