/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dpolygontriangulator.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/polygon/b2dtrapezoid.hxx>
#include <basegfx/range/b1drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <list>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace trapezoidhelper
    {
        //////////////////////////////////////////////////////////////////////////////

        class TrDeSimpleEdge
        {
        protected:
            const B2DPoint*     mpStart;
            const B2DPoint*     mpEnd;

        public:
            TrDeSimpleEdge(
                const B2DPoint* pStart,
                const B2DPoint* pEnd)
            :   mpStart(pStart),
                mpEnd(pEnd)
            {
            }

            // data read access
            const B2DPoint& getStart() const { return *mpStart; }
            const B2DPoint& getEnd() const { return *mpEnd; }
        };

        //////////////////////////////////////////////////////////////////////////////
        // define vector of simple edges
        typedef ::std::vector< TrDeSimpleEdge > TrDeSimpleEdges;

        //////////////////////////////////////////////////////////////////////////////

        class TrDeEdgeEntry : public TrDeSimpleEdge
        {
        private:
            sal_uInt32          mnSortValue;

        public:
            // convenience data read access
            double getDeltaX() const { return mpEnd->getX() - mpStart->getX(); }
            double getDeltaY() const { return mpEnd->getY() - mpStart->getY(); }

            // convenience data read access
            sal_uInt32 getSortValue() const
            {
                if(0 != mnSortValue)
                    return mnSortValue;

                // get radiant; has to be in the range ]0.0 .. pi[, thus scale to full
                // sal_uInt32 range for maximum precision
                const double fRadiant(atan2(getDeltaY(), getDeltaX()) * (SAL_MAX_UINT32 / F_PI));

                // convert to sal_uInt32 value
                const_cast< TrDeEdgeEntry* >(this)->mnSortValue = sal_uInt32(fRadiant);

                return mnSortValue;
            }

            // constructor
            TrDeEdgeEntry(
                const B2DPoint* pStart,
                const B2DPoint* pEnd,
                sal_uInt32 nSortValue)
            :   TrDeSimpleEdge(pStart, pEnd),
                mnSortValue(nSortValue)
            {
                // no horizontal edges allowed, all neeed to traverse vertivally
                OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
            }

            // data write access
            void setStart( const B2DPoint* pNewStart)
            {
                if(mpStart != pNewStart)
                {
                    mpStart = pNewStart;

                    // no horizontal edges allowed, all neeed to traverse vertivally
                    OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
                }
            }

            void setEnd( const B2DPoint* pNewEnd)
            {
                if(mpEnd != pNewEnd)
                {
                    mpEnd = pNewEnd;

                    // no horizontal edges allowed, all neeed to traverse vertivally
                    OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
                }
            }

            // operator for sort support
            bool operator<(const TrDeEdgeEntry& rComp) const
            {
                if(fTools::equal(getStart().getY(), rComp.getStart().getY(), fTools::getSmallValue()))
                {
                    if(fTools::equal(getStart().getX(), rComp.getStart().getX(), fTools::getSmallValue()))
                    {
                        // when start points are equal, use the direction the edge is pointing
                        // to. That value is derived from atan2 in the range ]0.0 .. pi[ and scaled
                        // to sal_uint32 range for best precision. 0 means no angle, while SAL_MAX_UINT32
                        // means pi. Thus, the higher the value, the more left the edge resides.
                        return (getSortValue() > rComp.getSortValue());
                    }
                    else
                    {
                        return fTools::less(getStart().getX(), rComp.getStart().getX());
                    }
                }
                else
                {
                    return fTools::less(getStart().getY(), rComp.getStart().getY());
                }
            }

        };

        //////////////////////////////////////////////////////////////////////////////
        // define double linked list of edges (for fast random insert)
        typedef ::std::list< TrDeEdgeEntry > TrDeEdgeEntries;

    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace trapezoidhelper
    {
        class TrapezoidSubdivider
        {
        private:
            sal_uInt32                  mnEdgeEntryCount;
            TrDeEdgeEntries             maTrDeEdgeEntries;
            ::std::vector< B2DPoint >   maPoints;
            ::std::vector< B2DPoint* >  maNewPoints;

            void addEdgeSorted(
                TrDeEdgeEntries::iterator aCurrent,
                const TrDeEdgeEntry& rNewEdge)
            {
                // Loop while new entry is bigger, use operator<
                while(aCurrent != maTrDeEdgeEntries.end() && (*aCurrent) < rNewEdge)
                {
                    aCurrent++;
                }

                // Insert before first which is smaller or equal or at end
                maTrDeEdgeEntries.insert(aCurrent, rNewEdge);
                mnEdgeEntryCount++;
            }

            bool splitEdgeAtGivenPoint(
                TrDeEdgeEntries::reference aEdge,
                const B2DPoint& rCutPoint,
                TrDeEdgeEntries::iterator aCurrent)
            {
                if(aEdge.getStart().equal(rCutPoint, fTools::getSmallValue()))
                {
                    return false;
                }

                if(aEdge.getEnd().equal(rCutPoint, fTools::getSmallValue()))
                {
                    return false;
                }

                const double fOldDeltaYStart(rCutPoint.getY() - aEdge.getStart().getY());

                if(fTools::lessOrEqual(fOldDeltaYStart, 0.0))
                {
                    // do not split: the resulting edge would be horizontal
                    // correct it to new start point
                    aEdge.setStart(&rCutPoint);
                    return false;
                }

                const double fNewDeltaYStart(aEdge.getEnd().getY() - rCutPoint.getY());

                if(fTools::lessOrEqual(fNewDeltaYStart, 0.0))
                {
                    // do not split: the resulting edge would be horizontal
                    aEdge.setEnd(&rCutPoint);
                    return false;
                }

                // Create new entry
                const TrDeEdgeEntry aNewEdge(
                    &rCutPoint,
                    &aEdge.getEnd(),
                    aEdge.getSortValue());

                // Correct old entry
                aEdge.setEnd(&rCutPoint);

                // Insert sorted (to avoid new sort)
                addEdgeSorted(aCurrent, aNewEdge);

                return true;
            }

            bool testAndCorrectEdgeIntersection(
                TrDeEdgeEntries::reference aEdgeA,
                TrDeEdgeEntries::reference aEdgeB,
                TrDeEdgeEntries::iterator aCurrent)
            {
                // Exclude simple cases: same start or end point
                if(aEdgeA.getStart().equal(aEdgeB.getStart(), fTools::getSmallValue()))
                {
                    return false;
                }

                if(aEdgeA.getStart().equal(aEdgeB.getEnd(), fTools::getSmallValue()))
                {
                    return false;
                }

                if(aEdgeA.getEnd().equal(aEdgeB.getStart(), fTools::getSmallValue()))
                {
                    return false;
                }

                if(aEdgeA.getEnd().equal(aEdgeB.getEnd(), fTools::getSmallValue()))
                {
                    return false;
                }

                // Exclude simple cases: one of the edges has no length anymore
                if(aEdgeA.getStart().equal(aEdgeA.getEnd(), fTools::getSmallValue()))
                {
                    return false;
                }

                if(aEdgeB.getStart().equal(aEdgeB.getEnd(), fTools::getSmallValue()))
                {
                    return false;
                }

                // now check if one point is on the other edge
                const B2DVector aDeltaB(aEdgeB.getDeltaX(), aEdgeB.getDeltaY());

                if(tools::isPointOnEdge(aEdgeA.getStart(), aEdgeB.getStart(), aDeltaB))
                {
                    return splitEdgeAtGivenPoint(aEdgeB, aEdgeA.getStart(), aCurrent);
                }

                if(tools::isPointOnEdge(aEdgeA.getEnd(), aEdgeB.getStart(), aDeltaB))
                {
                    return splitEdgeAtGivenPoint(aEdgeB, aEdgeA.getEnd(), aCurrent);
                }

                const B2DVector aDeltaA(aEdgeA.getDeltaX(), aEdgeA.getDeltaY());

                if(tools::isPointOnEdge(aEdgeB.getStart(), aEdgeA.getStart(), aDeltaA))
                {
                    return splitEdgeAtGivenPoint(aEdgeA, aEdgeB.getStart(), aCurrent);
                }

                if(tools::isPointOnEdge(aEdgeB.getEnd(), aEdgeA.getStart(), aDeltaA))
                {
                    return splitEdgeAtGivenPoint(aEdgeA, aEdgeB.getEnd(), aCurrent);
                }

                // check for cut inside edges
                double fCutA(0.0);
                double fCutB(0.0);

                if(tools::findCut(
                    aEdgeA.getStart(), aDeltaA,
                    aEdgeB.getStart(), aDeltaB,
                    CUTFLAG_LINE,
                    &fCutA,
                    &fCutB))
                {
                    const double fSimpleLengthA(aDeltaA.getX() + aDeltaA.getY());
                    const double fSimpleLengthB(aDeltaB.getX() + aDeltaB.getY());
                    const bool bAIsLonger(fSimpleLengthA > fSimpleLengthB);
                    B2DPoint* pNewPoint = bAIsLonger
                        ? new B2DPoint(aEdgeA.getStart() + (fCutA * aDeltaA))
                        : new B2DPoint(aEdgeB.getStart() + (fCutB * aDeltaB));
                    bool bRetval(false);

                    bRetval = splitEdgeAtGivenPoint(aEdgeA, *pNewPoint, aCurrent);
                    bRetval |= splitEdgeAtGivenPoint(aEdgeB, *pNewPoint, aCurrent);

                    if(bRetval)
                    {
                        maNewPoints.push_back(pNewPoint);
                    }
                    else
                    {
                        delete pNewPoint;
                    }

                    return bRetval;
                }

                return false;
            }

            B2DPoint getCutPointForGivenY(
                const TrDeEdgeEntry& rEdge,
                double fGivenY)
            {
                // Calculate cut point locally (do not use interpolate) since it is numerically
                // necessary to guarantee the new, equal Y-coordinate
                const double fFactor((fGivenY - rEdge.getStart().getY()) / rEdge.getDeltaY());
                const double fDeltaXNew(fFactor * rEdge.getDeltaX());

                return B2DPoint(rEdge.getStart().getX() + fDeltaXNew, fGivenY);
            }

            void solveHorizontalEdges(TrDeSimpleEdges& rTrDeSimpleEdges)
            {
                if(rTrDeSimpleEdges.size() && maTrDeEdgeEntries.size())
                {
                    // there were horizontal edges. These can be excluded, but
                    // cuts with other edges need to be solved and added
                    sal_uInt32 a(0);

                    for(a = 0; a < rTrDeSimpleEdges.size(); a++)
                    {
                        // get horizontal edge as candidate; prepare it's range and fixed Y
                        const TrDeSimpleEdge& rHorEdge = rTrDeSimpleEdges[a];
                        const B1DRange aRange(rHorEdge.getStart().getX(), rHorEdge.getEnd().getX());
                        const double fFixedY(rHorEdge.getStart().getY());

                        // loop over edges
                        TrDeEdgeEntries::iterator aCurrent(maTrDeEdgeEntries.begin());

                        do
                        {
                            // get compare edge
                            TrDeEdgeEntries::reference aCompare(*aCurrent++);

                            if(fTools::moreOrEqual(aCompare.getStart().getY(), fFixedY))
                            {
                                // edge starts below horizontal edge, continue
                                continue;
                            }

                            if(fTools::lessOrEqual(aCompare.getEnd().getY(), fFixedY))
                            {
                                // edge ends above horizontal edge, continue
                                continue;
                            }

                            // vertical overlap, get horizontal range
                            const B1DRange aCompareRange(aCompare.getStart().getX(), aCompare.getEnd().getX());

                            if(aRange.overlaps(aCompareRange))
                            {
                                // possible cut, get cut point
                                const B2DPoint aSplit(getCutPointForGivenY(aCompare, fFixedY));

                                if(fTools::more(aSplit.getX(), aRange.getMinimum())
                                    && fTools::less(aSplit.getX(), aRange.getMaximum()))
                                {
                                    // cut is in XRange of horizontal edge, potenitally needed cut
                                    B2DPoint* pNewPoint = new B2DPoint(aSplit);

                                    if(splitEdgeAtGivenPoint(aCompare, *pNewPoint, aCurrent))
                                    {
                                        maNewPoints.push_back(pNewPoint);
                                    }
                                    else
                                    {
                                        delete pNewPoint;
                                    }
                                }
                            }
                        }
                        while(aCurrent != maTrDeEdgeEntries.end()
                            && fTools::less(aCurrent->getStart().getY(), fFixedY));
                    }
                }
            }

        public:
            TrapezoidSubdivider(
                const B2DPolyPolygon& rSourcePolyPolygon)
            :   mnEdgeEntryCount(0),
                maTrDeEdgeEntries(),
                maPoints(),
                maNewPoints()
            {
                const sal_uInt32 nPolygonCount(rSourcePolyPolygon.count());
                TrDeSimpleEdges aTrDeSimpleEdges;
                sal_uInt32 a(0), b(0);
                sal_uInt32 nAllPointCount(0);

                for(a = 0; a < nPolygonCount; a++)
                {
                    const B2DPolygon aPolygonCandidate(rSourcePolyPolygon.getB2DPolygon(a));
                    const sal_uInt32 nCount(aPolygonCandidate.count());

                    if(nCount > 2)
                    {
                        nAllPointCount += nCount;
                    }
                }

                if(nAllPointCount)
                {
                    maPoints.reserve(nAllPointCount);
                    sal_uInt32 nStartIndex(0);

                    for(a = 0; a < nPolygonCount; a++)
                    {
                        const B2DPolygon aPolygonCandidate(rSourcePolyPolygon.getB2DPolygon(a));
                        const sal_uInt32 nCount(aPolygonCandidate.count());

                        if(nCount > 2)
                        {
                            for(b = 0; b < nCount; b++)
                            {
                                maPoints.push_back(aPolygonCandidate.getB2DPoint(b));
                            }
                        }
                    }

                    // moved the edge construction to a 3rd loop; doing it in the 2nd loop is
                    // possible, but requires q working vector::reserve() implementation, else
                    // the vector will be reallocated and the pointers will be wrong
                    for(a = 0; a < nPolygonCount; a++)
                    {
                        const B2DPolygon aPolygonCandidate(rSourcePolyPolygon.getB2DPolygon(a));
                        const sal_uInt32 nCount(aPolygonCandidate.count());

                        if(nCount > 2)
                        {
                            B2DPoint* pPrev(&maPoints[maPoints.size() - 1]);

                            for(b = 0; b < nCount; b++)
                            {
                                B2DPoint* pCurr(&maPoints[nStartIndex++]);

                                if(fTools::equal(pPrev->getY(), pCurr->getY(), fTools::getSmallValue()))
                                {
                                    // horizontal edge, check for single point
                                    if(!fTools::equal(pPrev->getX(), pCurr->getX(), fTools::getSmallValue()))
                                    {
                                        // X-order not needed, just add
                                        aTrDeSimpleEdges.push_back(TrDeSimpleEdge(pPrev, pCurr));

                                        const double fMiddle((pPrev->getY() + pCurr->getY()) * 0.5);
                                        pPrev->setY(fMiddle);
                                        pCurr->setY(fMiddle);
                                    }
                                }
                                else
                                {
                                    // vertical edge, add with positive Y-direction
                                    if(pPrev->getY() < pCurr->getY())
                                    {
                                        maTrDeEdgeEntries.push_back(TrDeEdgeEntry(pPrev, pCurr, 0));
                                    }
                                    else
                                    {
                                        maTrDeEdgeEntries.push_back(TrDeEdgeEntry(pCurr, pPrev, 0));
                                    }

                                    mnEdgeEntryCount++;
                                }

                                pPrev = pCurr;
                            }
                        }
                    }
                }

                if(maTrDeEdgeEntries.size())
                {
                    maTrDeEdgeEntries.sort();

                    solveHorizontalEdges(aTrDeSimpleEdges);
                }
            }

            ~TrapezoidSubdivider()
            {
                const sal_uInt32 nCount(maNewPoints.size());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    delete maNewPoints[a];
                }
            }

            B2DTrapezoidVector Subdivide()
            {
                B2DTrapezoidVector aRetval;
                B1DRange aLeftRange;
                B1DRange aRightRange;

                if(!maTrDeEdgeEntries.empty())
                {
                    // measuring shows that the relation between edges and created trapezoids is
                    // mostly in the 1.0 range, thus reserve as much trapezoids as edges exist. Do
                    // not use maTrDeEdgeEntries.size() since that may be a non-constant time
                    // operation for Lists. Instead, use mnEdgeEntryCount which will contain the
                    // roughly counted adds to the List
                    aRetval.reserve(mnEdgeEntryCount);
                }

                while(!maTrDeEdgeEntries.empty())
                {
                    // Prepare current operator and get first edge
                    TrDeEdgeEntries::iterator aCurrent(maTrDeEdgeEntries.begin());
                    TrDeEdgeEntries::reference aLeft(*aCurrent++);

                    if(aCurrent == maTrDeEdgeEntries.end())
                    {
                        // Should not happen: No 2nd edge; consume the single edge
                        // and start next loop
                        maTrDeEdgeEntries.pop_front();
                        continue;
                    }

                    // get second edge
                    TrDeEdgeEntries::reference aRight(*aCurrent++);

                    if(!fTools::equal(aLeft.getStart().getY(), aRight.getStart().getY(), fTools::getSmallValue()))
                    {
                        // Should not happen: We have a 2nd edge, but YStart is on another
                        // line; consume the single edge and start next loop
                        maTrDeEdgeEntries.pop_front();
                        continue;
                    }

                    // aLeft and aRight build a thought trapezoid now. They have a common
                    // start line (same Y for start points). Potentially, one of the edges
                    // is longer than the other. It is only needed to look at the shorter
                    // length which build the potential traezoid. To do so, get the end points
                    // locally and adapt the evtl. longer one
                    B2DPoint aLeftEnd(aLeft.getEnd());
                    B2DPoint aRightEnd(aRight.getEnd());

                    // check if end points are on the same line. If yes, no adaption
                    // needs to be prepared
                    const bool bEndOnSameLine(fTools::equal(aLeftEnd.getY(), aRightEnd.getY(), fTools::getSmallValue()));
                    bool bLeftIsLonger(false);

                    if(!bEndOnSameLine)
                    {
                        // check which edge is longer and correct accordingly
                        bLeftIsLonger = fTools::more(aLeftEnd.getY(), aRightEnd.getY());

                        if(bLeftIsLonger)
                        {
                            aLeftEnd = getCutPointForGivenY(aLeft, aRightEnd.getY());
                        }
                        else
                        {
                            aRightEnd = getCutPointForGivenY(aRight, aLeftEnd.getY());
                        }
                    }

                    // check for same start and end points
                    const bool bSameStartPoint(aLeft.getStart().equal(aRight.getStart(), fTools::getSmallValue()));
                    const bool bSameEndPoint(aLeftEnd.equal(aRightEnd, fTools::getSmallValue()));

                    // check the simple case that the edges form a 'blind' edge
                    if(bSameStartPoint && bSameEndPoint)
                    {
                        // correct the longer edge if prepared
                        if(!bEndOnSameLine)
                        {
                            if(bLeftIsLonger)
                            {
                                B2DPoint* pNewPoint = new B2DPoint(aLeftEnd);

                                if(splitEdgeAtGivenPoint(aLeft, *pNewPoint, aCurrent))
                                {
                                    maNewPoints.push_back(pNewPoint);
                                }
                                else
                                {
                                    delete pNewPoint;
                                }
                            }
                            else
                            {
                                B2DPoint* pNewPoint = new B2DPoint(aRightEnd);

                                if(splitEdgeAtGivenPoint(aRight, *pNewPoint, aCurrent))
                                {
                                    maNewPoints.push_back(pNewPoint);
                                }
                                else
                                {
                                    delete pNewPoint;
                                }
                            }
                        }

                        // consume both edges and start next run
                        maTrDeEdgeEntries.pop_front();
                        maTrDeEdgeEntries.pop_front();

                        continue;
                    }

                    // check if the edges self-intersect. This can only happen when
                    // start and end point are different
                    bool bRangesSet(false);

                    if(!(bSameStartPoint || bSameEndPoint))
                    {
                        // get XRanges of edges
                        aLeftRange = B1DRange(aLeft.getStart().getX(), aLeftEnd.getX());
                        aRightRange = B1DRange(aRight.getStart().getX(), aRightEnd.getX());
                        bRangesSet = true;

                        // use fast range test first
                        if(aLeftRange.overlaps(aRightRange))
                        {
                            // real cut test and correction. If corrected,
                            // start new run
                            if(testAndCorrectEdgeIntersection(aLeft, aRight, aCurrent))
                            {
                                continue;
                            }
                        }
                    }

                    // now we need to check if there are intersections with other edges
                    // or if other edges start inside the candidate trapezoid
                    if(aCurrent != maTrDeEdgeEntries.end()
                        && fTools::less(aCurrent->getStart().getY(), aLeftEnd.getY()))
                    {
                        // get XRanges of edges
                        if(!bRangesSet)
                        {
                            aLeftRange = B1DRange(aLeft.getStart().getX(), aLeftEnd.getX());
                            aRightRange = B1DRange(aRight.getStart().getX(), aRightEnd.getX());
                            bRangesSet = true;
                        }

                        // build full XRange for fast check
                        B1DRange aAllRange(aLeftRange);
                        aAllRange.expand(aRightRange);

                        // prepare loop iterator; aCurrent needs to stay unchanged for
                        // eventual insertions of new EdgeNodes. Also prepare stop flag
                        TrDeEdgeEntries::iterator aLoop(aCurrent);
                        bool bDone(false);

                        do
                        {
                            // get compare edge and it's XRange
                            TrDeEdgeEntries::reference aCompare(*aLoop++);

                            // avoid edges using the same start point as one of
                            // the edges. These can neither have their start point
                            // in the thought edge nor cut with one of the edges
                            if(aCompare.getStart().equal(aRight.getStart(), fTools::getSmallValue()))
                            {
                                continue;
                            }

                            // get compare XRange
                            const B1DRange aCompareRange(aCompare.getStart().getX(), aCompare.getEnd().getX());

                            // use fast range test
                            if(aAllRange.overlaps(aCompareRange))
                            {
                                // check for start point inside thought trapezoid
                                if(fTools::more(aCompare.getStart().getY(), aLeft.getStart().getY()))
                                {
                                    // calculate the two possible split points at compare's Y
                                    const B2DPoint aSplitLeft(getCutPointForGivenY(aLeft, aCompare.getStart().getY()));
                                    const B2DPoint aSplitRight(getCutPointForGivenY(aRight, aCompare.getStart().getY()));

                                    // check for start point of aCompare being inside thought
                                    // trapezoid
                                    if(aCompare.getStart().getX() >= aSplitLeft.getX() &&
                                        aCompare.getStart().getX() <= aSplitRight.getX())
                                    {
                                        // is inside, correct and restart loop
                                        B2DPoint* pNewLeft = new B2DPoint(aSplitLeft);

                                        if(splitEdgeAtGivenPoint(aLeft, *pNewLeft, aCurrent))
                                        {
                                            maNewPoints.push_back(pNewLeft);
                                        }
                                        else
                                        {
                                            delete pNewLeft;
                                        }

                                        B2DPoint* pNewRight = new B2DPoint(aSplitRight);

                                        if(splitEdgeAtGivenPoint(aRight, *pNewRight, aCurrent))
                                        {
                                            maNewPoints.push_back(pNewRight);
                                        }
                                        else
                                        {
                                            delete pNewRight;
                                        }

                                        bDone = true;
                                    }
                                }

                                if(!bDone && aLeftRange.overlaps(aCompareRange))
                                {
                                    // test for concrete cut of compare edge with left edge
                                    bDone = testAndCorrectEdgeIntersection(aLeft, aCompare, aCurrent);
                                }

                                if(!bDone && aRightRange.overlaps(aCompareRange))
                                {
                                    // test for concrete cut of compare edge with Right edge
                                    bDone = testAndCorrectEdgeIntersection(aRight, aCompare, aCurrent);
                                }
                            }
                        }
                        while(!bDone
                            && aLoop != maTrDeEdgeEntries.end()
                            && fTools::less(aLoop->getStart().getY(), aLeftEnd.getY()));

                        if(bDone)
                        {
                            // something needed to be changed; start next loop
                            continue;
                        }
                    }

                    // when we get here, the intended trapezoid can be used. It needs to
                    // be corrected, eventually (if prepared); but this is no reason not to
                    // use it in the same loop iteration
                    if(!bEndOnSameLine)
                    {
                        if(bLeftIsLonger)
                        {
                            B2DPoint* pNewPoint = new B2DPoint(aLeftEnd);

                            if(splitEdgeAtGivenPoint(aLeft, *pNewPoint, aCurrent))
                            {
                                maNewPoints.push_back(pNewPoint);
                            }
                            else
                            {
                                delete pNewPoint;
                            }
                        }
                        else
                        {
                            B2DPoint* pNewPoint = new B2DPoint(aRightEnd);

                            if(splitEdgeAtGivenPoint(aRight, *pNewPoint, aCurrent))
                            {
                                maNewPoints.push_back(pNewPoint);
                            }
                            else
                            {
                                delete pNewPoint;
                            }
                        }
                    }

                    // the two edges start at the same Y, they use the same DeltaY, they
                    // do not cut themselves and not any other edge in range. Create a
                    // B2DTrapezoid and consume both edges
                    aRetval.push_back(
                        B2DTrapezoid(
                            aLeft.getStart().getX(),
                            aRight.getStart().getX(),
                            aLeft.getStart().getY(),
                            aLeftEnd.getX(),
                            aRightEnd.getX(),
                            aLeftEnd.getY()));

                    maTrDeEdgeEntries.pop_front();
                    maTrDeEdgeEntries.pop_front();
                }

                return aRetval;
            }
        };
    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    B2DTrapezoid::B2DTrapezoid(
        const double& rfTopXLeft,
        const double& rfTopXRight,
        const double& rfTopY,
        const double& rfBottomXLeft,
        const double& rfBottomXRight,
        const double& rfBottomY)
    :
        mfTopXLeft(rfTopXLeft),
        mfTopXRight(rfTopXRight),
        mfTopY(rfTopY),
        mfBottomXLeft(rfBottomXLeft),
        mfBottomXRight(rfBottomXRight),
        mfBottomY(rfBottomY)
    {
        if(rfTopXLeft > rfTopXRight)
        {
            std::swap(mfTopXLeft, mfTopXRight);
        }

        if(rfBottomXLeft > rfBottomXRight)
        {
            std::swap(mfBottomXLeft, mfBottomXRight);
        }
    }

    B2DPolygon B2DTrapezoid::getB2DPolygon() const
    {
        B2DPolygon aRetval;

        aRetval.append(B2DPoint(getTopXLeft(), getTopY()));
        aRetval.append(B2DPoint(getTopXRight(), getTopY()));
        aRetval.append(B2DPoint(getBottomXRight(), getBottomY()));
        aRetval.append(B2DPoint(getBottomXLeft(), getBottomY()));
        aRetval.setClosed(true);

        return aRetval;
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // convert SourcePolyPolygon to trapezoids
        B2DTrapezoidVector trapezoidSubdivide(const B2DPolyPolygon& rSourcePolyPolygon)
        {
            B2DPolyPolygon aSource(rSourcePolyPolygon);

            if(aSource.areControlPointsUsed())
            {
                aSource = aSource.getDefaultAdaptiveSubdivision();
            }

            trapezoidhelper::TrapezoidSubdivider aTrapezoidSubdivider(aSource);
            return aTrapezoidSubdivider.Subdivide();
        }
    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
