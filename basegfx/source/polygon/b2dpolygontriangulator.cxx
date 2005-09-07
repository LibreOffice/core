/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygontriangulator.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:46:38 $
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

#ifndef _BGFX_POLYGON_B2DPOLYGONTRIANGULATOR_HXX
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

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
            :   mpNext(0L),
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

            ~EdgeEntry()
            {
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

            bool operator!=(const EdgeEntry& rComp) const
            {
                return !(*this == rComp);
            }

            const B2DPoint& getStart() const { return maStart; }
            const B2DPoint& getEnd() const { return maEnd; }

            EdgeEntry* getNext() const { return mpNext; }
            void setNext(EdgeEntry* pNext) { mpNext = pNext; }
        };

        //////////////////////////////////////////////////////////////////////////////

        typedef ::std::vector< EdgeEntry > EdgeEntries;
        typedef ::std::vector< EdgeEntry* > EdgeEntryPointers;

        //////////////////////////////////////////////////////////////////////////////

        class Triangulator
        {
            EdgeEntry*                                      mpList;
            EdgeEntries                                     maStartEntries;
            EdgeEntryPointers                               maNewEdgeEntries;
            B2DPolygon                                      maResult;

            void handleClosingEdge(const B2DPoint& rStart, const B2DPoint& rEnd);
            bool CheckPointInTriangle(EdgeEntry* pEdgeA, EdgeEntry* pEdgeB, const B2DPoint& rTestPoint);
            void createTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC);

        public:
            Triangulator(const B2DPolyPolygon& rCandidate);
            ~Triangulator();

            const B2DPolygon getResult() const { return maResult; }
        };

        void Triangulator::handleClosingEdge(const B2DPoint& rStart, const B2DPoint& rEnd)
        {
            // create an entry, else the comparison might use the wrong edges
            EdgeEntry aNew(rStart, rEnd);
            EdgeEntry* pCurr = mpList;
            EdgeEntry* pPrev = 0L;

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
                maNewEdgeEntries.push_back(pNew);
                pCurr = mpList;
                pPrev = 0L;

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

        bool Triangulator::CheckPointInTriangle(EdgeEntry* pEdgeA, EdgeEntry* pEdgeB, const B2DPoint& rTestPoint)
        {
            // inside triangle or on edge?
            if(tools::isPointInTriangle(pEdgeA->getStart(), pEdgeA->getEnd(), pEdgeB->getEnd(), rTestPoint, true))
            {
                // but not on point
                if(!rTestPoint.equal(pEdgeA->getEnd()) && !rTestPoint.equal(pEdgeB->getEnd()))
                {
                    // found point in triangle -> split triangle inserting two edges
                    EdgeEntry* pStart = new EdgeEntry(pEdgeA->getStart(), rTestPoint);
                    EdgeEntry* pEnd = new EdgeEntry(*pStart);
                    maNewEdgeEntries.push_back(pStart);
                    maNewEdgeEntries.push_back(pEnd);

                    pStart->setNext(pEnd);
                    pEnd->setNext(pEdgeA->getNext());
                    pEdgeA->setNext(pStart);

                    return false;
                }
            }

            return true;
        }

        void Triangulator::createTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC)
        {
            maResult.append(rA);
            maResult.append(rB);
            maResult.append(rC);
        }

        // consume as long as there are edges
        Triangulator::Triangulator(const B2DPolyPolygon& rCandidate)
        :   mpList(0L)
        {
            // add all available edges to the single linked local list which will be sorted
            // by Y,X,atan2 when adding nodes
            if(rCandidate.count())
            {
                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    const B2DPolygon aPolygonCandidate(rCandidate.getB2DPolygon(a));
                    const sal_uInt32 nCount(aPolygonCandidate.count());

                    if(nCount > 2L)
                    {
                        B2DPoint aPrevPnt(aPolygonCandidate.getB2DPoint(nCount - 1L));

                        for(sal_uInt32 b(0L); b < nCount; b++)
                        {
                            B2DPoint aNextPnt(aPolygonCandidate.getB2DPoint(b));

                            if( !aPrevPnt.equal(aNextPnt) )
                            {
                                maStartEntries.push_back(EdgeEntry(aPrevPnt, aNextPnt));
                            }

                            aPrevPnt = aNextPnt;
                        }
                    }
                }

                if(maStartEntries.size())
                {
                    // sort initial list
                    ::std::sort(maStartEntries.begin(), maStartEntries.end());

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

                        if(ORIENTATION_NEUTRAL == getOrientation(aLeft, aRight))
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
                                // look for end point in triange
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
                    EdgeEntry* pEdge = mpList;
                    mpList = mpList->getNext();
                }
            }
        }

        Triangulator::~Triangulator()
        {
            EdgeEntryPointers::iterator aIter(maNewEdgeEntries.begin());

            while(aIter != maNewEdgeEntries.end())
            {
                delete (*aIter++);
            }
        }

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace triangulator
    {
        B2DPolygon triangulate(const B2DPolygon& rCandidate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                OSL_ENSURE(false, "triangulate: works not for curves (!)");
                B2DPolygon aCandidate = tools::adaptiveSubdivideByAngle(rCandidate);
                return triangulate(aCandidate);
            }

            B2DPolygon aRetval;

            // remove double and neutral points
            B2DPolygon aCandidate(rCandidate);
            aCandidate.removeDoublePoints();
            aCandidate = tools::removeNeutralPoints(aCandidate);

            if(2L == aCandidate.count())
            {
                // candidate IS a triangle, just append
                aRetval.append(aCandidate);
            }
            else if(aCandidate.count() > 2L)
            {
                if(tools::isConvex(aCandidate))
                {
                    // polygon is convex, just use a triangle fan
                    tools::addTriangleFan(aCandidate, aRetval);
                }
                else
                {
                    // polygon is concave.
                    B2DPolyPolygon aPolyPolygon;
                    aPolyPolygon.append(rCandidate);
                    Triangulator aTriangulator(aPolyPolygon);
                    aRetval = aTriangulator.getResult();
                }
            }

            return aRetval;
        }

        B2DPolygon triangulate(const B2DPolyPolygon& rCandidate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                OSL_ENSURE(false, "triangulate: works not for curves (!)");
                B2DPolyPolygon aCandidate = tools::adaptiveSubdivideByAngle(rCandidate);
                return triangulate(aCandidate);
            }

            B2DPolygon aRetval;

            if(1L == rCandidate.count())
            {
                // single polygon -> single polygon triangulation
                const B2DPolygon aSinglePolygon(rCandidate.getB2DPolygon(0L));
                aRetval = triangulate(aSinglePolygon);
            }
            else
            {
                Triangulator aTriangulator(rCandidate);
                aRetval = aTriangulator.getResult();
            }

            return aRetval;
        }
    } // end of namespace triangulator
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
