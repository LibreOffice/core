/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
        // helper class to hold a simple ege. This is only used for horizontal edges
        // currently, thus the YPositions will be equal. I did not create a special
        // class for this since holdingthe pointers is more effective and also can be
        // used as baseclass for the traversing edges

        class TrDeSimpleEdge
        {
        protected:
            // pointers to start and end point
            const B2DPoint*     mpStart;
            const B2DPoint*     mpEnd;

        public:
            // constructor
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
        // helper class for holding a traversing edge. It will always have some
        // distance in YPos. The slope (in a numerically useful form, see comments) is
        // hold and used in SortValue to allow sorting traversing edges by Y, X and slope
        // (in that order)

        class TrDeEdgeEntry : public TrDeSimpleEdge
        {
        private:
            // the slope in a numerical useful form for sorting
            sal_uInt32          mnSortValue;

        public:
            // convenience data read access
            double getDeltaX() const { return mpEnd->getX() - mpStart->getX(); }
            double getDeltaY() const { return mpEnd->getY() - mpStart->getY(); }

            // convenience data read access. SortValue is created on demand since
            // it is not always used
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

            // constructor. SortValue can be given when known, use zero otherwise
            TrDeEdgeEntry(
                const B2DPoint* pStart,
                const B2DPoint* pEnd,
                sal_uInt32 nSortValue = 0)
            :   TrDeSimpleEdge(pStart, pEnd),
                mnSortValue(nSortValue)
            {
                // force traversal of deltaY downward
                if(mpEnd->getY() < mpStart->getY())
                {
                    std::swap(mpStart, mpEnd);
                }

                // no horizontal edges allowed, all neeed to traverse vertically
                OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
            }

            // data write access to StartPoint
            void setStart( const B2DPoint* pNewStart)
            {
                OSL_ENSURE(0 != pNewStart, "No null pointer allowed here (!)");

                if(mpStart != pNewStart)
                {
                    mpStart = pNewStart;

                    // no horizontal edges allowed, all neeed to traverse vertivally
                    OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
                }
            }

            // data write access to EndPoint
            void setEnd( const B2DPoint* pNewEnd)
            {
                OSL_ENSURE(0 != pNewEnd, "No null pointer allowed here (!)");

                if(mpEnd != pNewEnd)
                {
                    mpEnd = pNewEnd;

                    // no horizontal edges allowed, all neeed to traverse vertivally
                    OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
                }
            }

            // operator for sort support. Sort by Y, X and slope (in that order)
            bool operator<(const TrDeEdgeEntry& rComp) const
            {
                if(fTools::equal(getStart().getY(), rComp.getStart().getY(), fTools::getSmallValue()))
                {
                    if(fTools::equal(getStart().getX(), rComp.getStart().getX(), fTools::getSmallValue()))
                    {
                        // when start points are equal, use the direction the edge is pointing
                        // to. That value is created on demand and derived from atan2 in the
                        // range ]0.0 .. pi[ (without extremas, we always have a deltaY in this
                        // class) and scaled to sal_uInt32 range for best precision. 0 means no angle,
                        // while SAL_MAX_UINT32 means pi. Thus, the higher the value, the more left
                        // the edge traverses.
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

            // method for cut support
            B2DPoint getCutPointForGivenY(double fGivenY)
            {
                // Calculate cut point locally (do not use interpolate) since it is numerically
                // necessary to guarantee the new, equal Y-coordinate
                const double fFactor((fGivenY - getStart().getY()) / getDeltaY());
                const double fDeltaXNew(fFactor * getDeltaX());

                return B2DPoint(getStart().getX() + fDeltaXNew, fGivenY);
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
        // helper class to handle the complete trapezoid subdivision of a PolyPolygon
        class TrapezoidSubdivider
        {
        private:
            // local data
            sal_uInt32                  mnInitialEdgeEntryCount;
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
                    ++aCurrent;
                }

                // Insert before first which is smaller or equal or at end
                maTrDeEdgeEntries.insert(aCurrent, rNewEdge);
            }

            bool splitEdgeAtGivenPoint(
                TrDeEdgeEntries::reference aEdge,
                const B2DPoint& rCutPoint,
                TrDeEdgeEntries::iterator aCurrent)
            {
                // do not create edges without deltaY: do not split when start is identical
                if(aEdge.getStart().equal(rCutPoint, fTools::getSmallValue()))
                {
                    return false;
                }

                // do not create edges without deltaY: do not split when end is identical
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
                    // correct it to new end point
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

                // check if one point is on the other edge (a touch, not a cut)
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

                // check for cut inside edges. Use both t-values to choose the more precise
                // one later
                double fCutA(0.0);
                double fCutB(0.0);

                if(tools::findCut(
                    aEdgeA.getStart(), aDeltaA,
                    aEdgeB.getStart(), aDeltaB,
                    CUTFLAG_LINE,
                    &fCutA,
                    &fCutB))
                {
                    // use a simple metric (length criteria) for choosing the numerically
                    // better cut
                    const double fSimpleLengthA(aDeltaA.getX() + aDeltaA.getY());
                    const double fSimpleLengthB(aDeltaB.getX() + aDeltaB.getY());
                    const bool bAIsLonger(fSimpleLengthA > fSimpleLengthB);
                    B2DPoint* pNewPoint = bAIsLonger
                        ? new B2DPoint(aEdgeA.getStart() + (fCutA * aDeltaA))
                        : new B2DPoint(aEdgeB.getStart() + (fCutB * aDeltaB));
                    bool bRetval(false);

                    // try to split both edges
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

            void solveHorizontalEdges(TrDeSimpleEdges& rTrDeSimpleEdges)
            {
                if(rTrDeSimpleEdges.size() && maTrDeEdgeEntries.size())
                {
                    // there were horizontal edges. These can be excluded, but
                    // cuts with other edges need to be solved and added before
                    // ignoring them
                    sal_uInt32 a(0);

                    for(a = 0; a < rTrDeSimpleEdges.size(); a++)
                    {
                        // get horizontal edge as candidate; prepare it's range and fixed Y
                        const TrDeSimpleEdge& rHorEdge = rTrDeSimpleEdges[a];
                        const B1DRange aRange(rHorEdge.getStart().getX(), rHorEdge.getEnd().getX());
                        const double fFixedY(rHorEdge.getStart().getY());

                        // loop over traversing edges
                        TrDeEdgeEntries::iterator aCurrent(maTrDeEdgeEntries.begin());

                        do
                        {
                            // get compare edge
                            TrDeEdgeEntries::reference aCompare(*aCurrent++);

                            if(fTools::lessOrEqual(aCompare.getEnd().getY(), fFixedY))
                            {
                                // edge ends above horizontal edge, continue
                                continue;
                            }

                            if(fTools::moreOrEqual(aCompare.getStart().getY(), fFixedY))
                            {
                                // edge starts below horizontal edge, continue
                                continue;
                            }

                            // vertical overlap, get horizontal range
                            const B1DRange aCompareRange(aCompare.getStart().getX(), aCompare.getEnd().getX());

                            if(aRange.overlaps(aCompareRange))
                            {
                                // possible cut, get cut point
                                const B2DPoint aSplit(aCompare.getCutPointForGivenY(fFixedY));

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
            explicit TrapezoidSubdivider(
                const B2DPolyPolygon& rSourcePolyPolygon)
            :   mnInitialEdgeEntryCount(0),
                maTrDeEdgeEntries(),
                maPoints(),
                maNewPoints()
            {
                B2DPolyPolygon aSource(rSourcePolyPolygon);
                const sal_uInt32 nPolygonCount(rSourcePolyPolygon.count());
                TrDeSimpleEdges aTrDeSimpleEdges;
                sal_uInt32 a(0), b(0);
                sal_uInt32 nAllPointCount(0);

                // ensure there are no curves used
                if(aSource.areControlPointsUsed())
                {
                    aSource = aSource.getDefaultAdaptiveSubdivision();
                }

                for(a = 0; a < nPolygonCount; a++)
                {
                    // 1st run: count points
                    const B2DPolygon aPolygonCandidate(aSource.getB2DPolygon(a));
                    const sal_uInt32 nCount(aPolygonCandidate.count());

                    if(nCount > 2)
                    {
                        nAllPointCount += nCount;
                    }
                }

                if(nAllPointCount)
                {
                    // reserve needed points. CAUTION: maPoints size is NOT to be changed anymore
                    // after 2nd loop since pointers to it are used in the edges
                    maPoints.reserve(nAllPointCount);

                    for(a = 0; a < nPolygonCount; a++)
                    {
                        // 2nd run: add points
                        const B2DPolygon aPolygonCandidate(aSource.getB2DPolygon(a));
                        const sal_uInt32 nCount(aPolygonCandidate.count());

                        if(nCount > 2)
                        {
                            for(b = 0; b < nCount; b++)
                            {
                                maPoints.push_back(aPolygonCandidate.getB2DPoint(b));
                            }
                        }
                    }

                    // Moved the edge construction to a 3rd run: doing it in the 2nd run is
                    // possible(and i used it), but requires a working vector::reserve()
                    // implementation, else the vector will be reallocated and the pointers
                    // in the edges may be wrong. Security first here.
                    sal_uInt32 nStartIndex(0);

                    for(a = 0; a < nPolygonCount; a++)
                    {
                        const B2DPolygon aPolygonCandidate(aSource.getB2DPolygon(a));
                        const sal_uInt32 nCount(aPolygonCandidate.count());

                        if(nCount > 2)
                        {
                            // get the last point of the current polygon
                            B2DPoint* pPrev(&maPoints[nCount + nStartIndex - 1]);

                            for(b = 0; b < nCount; b++)
                            {
                                // get next point
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
                                    // vertical edge. Positive Y-direction is guaranteed by the
                                    // TrDeEdgeEntry constructor
                                    maTrDeEdgeEntries.push_back(TrDeEdgeEntry(pPrev, pCurr, 0));
                                    mnInitialEdgeEntryCount++;
                                }

                                // prepare next step
                                pPrev = pCurr;
                            }
                        }
                    }
                }

                if(!maTrDeEdgeEntries.empty())
                {
                    // single and initial sort of traversing edges
                    maTrDeEdgeEntries.sort();

                    // solve horizontal edges if there are any detected
                    solveHorizontalEdges(aTrDeSimpleEdges);
                }
            }

            ~TrapezoidSubdivider()
            {
                // delete the extra points created for cuts
                const sal_uInt32 nCount(maNewPoints.size());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    delete maNewPoints[a];
                }
            }

            void Subdivide(B2DTrapezoidVector& ro_Result)
            {
                // This is the central subdivider. The strategy is to use the first two entries
                // from the traversing edges as a potential trapezoid and do the needed corrections
                // and adaptions on the way.
                //
                // There always must be two edges with the same YStart value: When adding the polygons
                // in the constructor, there is always a topmost point from which two edges start; when
                // the topmost is an edge, there is a start and end of this edge from which two edges
                // start. All cases have two edges with same StartY (QED).
                //
                // Based on this these edges get corrected when:
                // - one is longer than the other
                // - they intersect
                // - they intersect with other edges
                // - another edge starts inside the thought trapezoid
                //
                // All this cases again produce a valid state so that the first two edges have a common
                // Ystart again. Some cases lead to a restart of the process, some allow consuming the
                // edges and create the intended trapezoid.
                //
                // Be careful when doing chages here: It is essential to keep all possible paths
                // in valid states and to be numerically correct. This is especially needed e.g.
                // by using fTools::equal(..) in the more robust small-value incarnation.
                B1DRange aLeftRange;
                B1DRange aRightRange;

                if(!maTrDeEdgeEntries.empty())
                {
                    // measuring shows that the relation between edges and created trapezoids is
                    // mostly in the 1:1 range, thus reserve as much trapezoids as edges exist. Do
                    // not use maTrDeEdgeEntries.size() since that may be a non-constant time
                    // operation for Lists. Instead, use mnInitialEdgeEntryCount which will contain
                    // the roughly counted adds to the List
                    ro_Result.reserve(ro_Result.size() + mnInitialEdgeEntryCount);
                }

                while(!maTrDeEdgeEntries.empty())
                {
                    // Prepare current operator and get first edge
                    TrDeEdgeEntries::iterator aCurrent(maTrDeEdgeEntries.begin());
                    TrDeEdgeEntries::reference aLeft(*aCurrent++);

                    if(aCurrent == maTrDeEdgeEntries.end())
                    {
                        // Should not happen: No 2nd edge; consume the single edge
                        // to not have an endless loop and start next. During development
                        // i constantly had breakpoints here, so i am sure enough to add an
                        // assertion here
                        OSL_FAIL("Trapeziod decomposer in illegal state (!)");
                        maTrDeEdgeEntries.pop_front();
                        continue;
                    }

                    // get second edge
                    TrDeEdgeEntries::reference aRight(*aCurrent++);

                    if(!fTools::equal(aLeft.getStart().getY(), aRight.getStart().getY(), fTools::getSmallValue()))
                    {
                        // Should not happen: We have a 2nd edge, but YStart is on another
                        // line; consume the single edge to not have an endless loop and start
                        // next. During development i constantly had breakpoints here, so i am
                        // sure enough to add an assertion here
                        OSL_FAIL("Trapeziod decomposer in illegal state (!)");
                        maTrDeEdgeEntries.pop_front();
                        continue;
                    }

                    // aLeft and aRight build a thought trapezoid now. They have a common
                    // start line (same Y for start points). Potentially, one of the edges
                    // is longer than the other. It is only needed to look at the shorter
                    // length which build the potential trapezoid. To do so, get the end points
                    // locally and adapt the evtl. longer one. Use only aLeftEnd and aRightEnd
                    // from here on, not the aLeft.getEnd() or aRight.getEnd() accesses.
                    B2DPoint aLeftEnd(aLeft.getEnd());
                    B2DPoint aRightEnd(aRight.getEnd());

                    // check if end points are on the same line. If yes, no adaption
                    // needs to be prepared. Also remember which one actually is longer.
                    const bool bEndOnSameLine(fTools::equal(aLeftEnd.getY(), aRightEnd.getY(), fTools::getSmallValue()));
                    bool bLeftIsLonger(false);

                    if(!bEndOnSameLine)
                    {
                        // check which edge is longer and correct accordingly
                        bLeftIsLonger = fTools::more(aLeftEnd.getY(), aRightEnd.getY());

                        if(bLeftIsLonger)
                        {
                            aLeftEnd = aLeft.getCutPointForGivenY(aRightEnd.getY());
                        }
                        else
                        {
                            aRightEnd = aRight.getCutPointForGivenY(aLeftEnd.getY());
                        }
                    }

                    // check for same start and end points
                    const bool bSameStartPoint(aLeft.getStart().equal(aRight.getStart(), fTools::getSmallValue()));
                    const bool bSameEndPoint(aLeftEnd.equal(aRightEnd, fTools::getSmallValue()));

                    // check the simple case that the edges form a 'blind' edge (deadend)
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
                            // real cut test and correction. If correction was needed,
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
                        }

                        // build full XRange for fast check
                        B1DRange aAllRange(aLeftRange);
                        aAllRange.expand(aRightRange);

                        // prepare loop iterator; aCurrent needs to stay unchanged for
                        // eventual sorted insertions of new EdgeNodes. Also prepare stop flag
                        TrDeEdgeEntries::iterator aLoop(aCurrent);
                        bool bDone(false);

                        do
                        {
                            // get compare edge and it's XRange
                            TrDeEdgeEntries::reference aCompare(*aLoop++);

                            // avoid edges using the same start point as one of
                            // the edges. These can neither have their start point
                            // in the thought trapezoid nor cut with one of the edges
                            if(aCompare.getStart().equal(aRight.getStart(), fTools::getSmallValue()))
                            {
                                continue;
                            }

                            // get compare XRange
                            const B1DRange aCompareRange(aCompare.getStart().getX(), aCompare.getEnd().getX());

                            // use fast range test first
                            if(aAllRange.overlaps(aCompareRange))
                            {
                                // check for start point inside thought trapezoid
                                if(fTools::more(aCompare.getStart().getY(), aLeft.getStart().getY()))
                                {
                                    // calculate the two possible split points at compare's Y
                                    const B2DPoint aSplitLeft(aLeft.getCutPointForGivenY(aCompare.getStart().getY()));
                                    const B2DPoint aSplitRight(aRight.getCutPointForGivenY(aCompare.getStart().getY()));

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
                                            bDone = true;
                                        }
                                        else
                                        {
                                            delete pNewLeft;
                                        }

                                        B2DPoint* pNewRight = new B2DPoint(aSplitRight);

                                        if(splitEdgeAtGivenPoint(aRight, *pNewRight, aCurrent))
                                        {
                                            maNewPoints.push_back(pNewRight);
                                            bDone = true;
                                        }
                                        else
                                        {
                                            delete pNewRight;
                                        }
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
                    ro_Result.push_back(
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
    :   mfTopXLeft(rfTopXLeft),
        mfTopXRight(rfTopXRight),
        mfTopY(rfTopY),
        mfBottomXLeft(rfBottomXLeft),
        mfBottomXRight(rfBottomXRight),
        mfBottomY(rfBottomY)
    {
        // guarantee mfTopXRight >= mfTopXLeft
        if(mfTopXLeft > mfTopXRight)
        {
            std::swap(mfTopXLeft, mfTopXRight);
        }

        // guarantee mfBottomXRight >= mfBottomXLeft
        if(mfBottomXLeft > mfBottomXRight)
        {
            std::swap(mfBottomXLeft, mfBottomXRight);
        }

        // guarantee mfBottomY >= mfTopY
        if(mfTopY > mfBottomY)
        {
            std::swap(mfTopY, mfBottomY);
            std::swap(mfTopXLeft, mfBottomXLeft);
            std::swap(mfTopXRight, mfBottomXRight);
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
        // convert Source PolyPolygon to trapezoids
        void trapezoidSubdivide(B2DTrapezoidVector& ro_Result, const B2DPolyPolygon& rSourcePolyPolygon)
        {
            trapezoidhelper::TrapezoidSubdivider aTrapezoidSubdivider(rSourcePolyPolygon);

            aTrapezoidSubdivider.Subdivide(ro_Result);
        }

        void createLineTrapezoidFromEdge(
            B2DTrapezoidVector& ro_Result,
            const B2DPoint& rPointA,
            const B2DPoint& rPointB,
            double fLineWidth)
        {
            if(fTools::lessOrEqual(fLineWidth, 0.0))
            {
                // no line witdh
                return;
            }

            if(rPointA.equal(rPointB, fTools::getSmallValue()))
            {
                // points are equal, no edge
                return;
            }

            const double fHalfLineWidth(0.5 * fLineWidth);

            if(fTools::equal(rPointA.getX(), rPointB.getX(), fTools::getSmallValue()))
            {
                // vertical line
                const double fLeftX(rPointA.getX() - fHalfLineWidth);
                const double fRightX(rPointA.getX() + fHalfLineWidth);

                ro_Result.push_back(
                    B2DTrapezoid(
                        fLeftX,
                        fRightX,
                        std::min(rPointA.getY(), rPointB.getY()),
                        fLeftX,
                        fRightX,
                        std::max(rPointA.getY(), rPointB.getY())));
            }
            else if(fTools::equal(rPointA.getY(), rPointB.getY(), fTools::getSmallValue()))
            {
                // horizontal line
                const double fLeftX(std::min(rPointA.getX(), rPointB.getX()));
                const double fRightX(std::max(rPointA.getX(), rPointB.getX()));

                ro_Result.push_back(
                    B2DTrapezoid(
                        fLeftX,
                        fRightX,
                        rPointA.getY() - fHalfLineWidth,
                        fLeftX,
                        fRightX,
                        rPointA.getY() + fHalfLineWidth));
            }
            else
            {
                // diagonal line
                // create perpendicular vector
                const B2DVector aDelta(rPointB - rPointA);
                B2DVector aPerpendicular(-aDelta.getY(), aDelta.getX());
                aPerpendicular.setLength(fHalfLineWidth);

                // create StartLow, StartHigh, EndLow and EndHigh
                const B2DPoint aStartLow(rPointA + aPerpendicular);
                const B2DPoint aStartHigh(rPointA - aPerpendicular);
                const B2DPoint aEndHigh(rPointB - aPerpendicular);
                const B2DPoint aEndLow(rPointB + aPerpendicular);

                // create EdgeEntries
                basegfx::trapezoidhelper::TrDeEdgeEntries aTrDeEdgeEntries;

                aTrDeEdgeEntries.push_back(basegfx::trapezoidhelper::TrDeEdgeEntry(&aStartLow, &aStartHigh, 0));
                aTrDeEdgeEntries.push_back(basegfx::trapezoidhelper::TrDeEdgeEntry(&aStartHigh, &aEndHigh, 0));
                aTrDeEdgeEntries.push_back(basegfx::trapezoidhelper::TrDeEdgeEntry(&aEndHigh, &aEndLow, 0));
                aTrDeEdgeEntries.push_back(basegfx::trapezoidhelper::TrDeEdgeEntry(&aEndLow, &aStartLow, 0));
                aTrDeEdgeEntries.sort();

                // here we know we have exactly four edges, and they do not cut, touch or
                // intersect. This makes processing much easier. Get the first two as start
                // edges for the thought trapezoid
                basegfx::trapezoidhelper::TrDeEdgeEntries::iterator aCurrent(aTrDeEdgeEntries.begin());
                basegfx::trapezoidhelper::TrDeEdgeEntries::reference aLeft(*aCurrent++);
                basegfx::trapezoidhelper::TrDeEdgeEntries::reference aRight(*aCurrent++);
                const bool bEndOnSameLine(fTools::equal(aLeft.getEnd().getY(), aRight.getEnd().getY(), fTools::getSmallValue()));

                if(bEndOnSameLine)
                {
                    // create two triangle trapezoids
                    ro_Result.push_back(
                        B2DTrapezoid(
                            aLeft.getStart().getX(),
                            aRight.getStart().getX(),
                            aLeft.getStart().getY(),
                            aLeft.getEnd().getX(),
                            aRight.getEnd().getX(),
                            aLeft.getEnd().getY()));

                    basegfx::trapezoidhelper::TrDeEdgeEntries::reference aLeft2(*aCurrent++);
                    basegfx::trapezoidhelper::TrDeEdgeEntries::reference aRight2(*aCurrent++);

                    ro_Result.push_back(
                        B2DTrapezoid(
                            aLeft2.getStart().getX(),
                            aRight2.getStart().getX(),
                            aLeft2.getStart().getY(),
                            aLeft2.getEnd().getX(),
                            aRight2.getEnd().getX(),
                            aLeft2.getEnd().getY()));
                }
                else
                {
                    // create three trapezoids. Check which edge is longer and
                    // correct accordingly
                    const bool bLeftIsLonger(fTools::more(aLeft.getEnd().getY(), aRight.getEnd().getY()));

                    if(bLeftIsLonger)
                    {
                        basegfx::trapezoidhelper::TrDeEdgeEntries::reference aRight2(*aCurrent++);
                        basegfx::trapezoidhelper::TrDeEdgeEntries::reference aLeft2(*aCurrent++);
                        const B2DPoint aSplitLeft(aLeft.getCutPointForGivenY(aRight.getEnd().getY()));
                        const B2DPoint aSplitRight(aRight2.getCutPointForGivenY(aLeft.getEnd().getY()));

                        ro_Result.push_back(
                            B2DTrapezoid(
                                aLeft.getStart().getX(),
                                aRight.getStart().getX(),
                                aLeft.getStart().getY(),
                                aSplitLeft.getX(),
                                aRight.getEnd().getX(),
                                aRight.getEnd().getY()));

                        ro_Result.push_back(
                            B2DTrapezoid(
                                aSplitLeft.getX(),
                                aRight.getEnd().getX(),
                                aRight.getEnd().getY(),
                                aLeft2.getStart().getX(),
                                aSplitRight.getX(),
                                aLeft2.getStart().getY()));

                        ro_Result.push_back(
                            B2DTrapezoid(
                                aLeft2.getStart().getX(),
                                aSplitRight.getX(),
                                aLeft2.getStart().getY(),
                                aLeft2.getEnd().getX(),
                                aRight2.getEnd().getX(),
                                aLeft2.getEnd().getY()));
                    }
                    else
                    {
                        basegfx::trapezoidhelper::TrDeEdgeEntries::reference aLeft2(*aCurrent++);
                        basegfx::trapezoidhelper::TrDeEdgeEntries::reference aRight2(*aCurrent++);
                        const B2DPoint aSplitRight(aRight.getCutPointForGivenY(aLeft.getEnd().getY()));
                        const B2DPoint aSplitLeft(aLeft2.getCutPointForGivenY(aRight.getEnd().getY()));

                        ro_Result.push_back(
                            B2DTrapezoid(
                                aLeft.getStart().getX(),
                                aRight.getStart().getX(),
                                aLeft.getStart().getY(),
                                aLeft.getEnd().getX(),
                                aSplitRight.getX(),
                                aLeft.getEnd().getY()));

                        ro_Result.push_back(
                            B2DTrapezoid(
                                aLeft.getEnd().getX(),
                                aSplitRight.getX(),
                                aLeft.getEnd().getY(),
                                aSplitLeft.getX(),
                                aRight.getEnd().getX(),
                                aRight2.getStart().getY()));

                        ro_Result.push_back(
                            B2DTrapezoid(
                                aSplitLeft.getX(),
                                aRight.getEnd().getX(),
                                aRight2.getStart().getY(),
                                aLeft2.getEnd().getX(),
                                aRight2.getEnd().getX(),
                                aLeft2.getEnd().getY()));
                    }
                }
            }
        }

        void createLineTrapezoidFromB2DPolygon(
            B2DTrapezoidVector& ro_Result,
            const B2DPolygon& rPolygon,
            double fLineWidth)
        {
            if(fTools::lessOrEqual(fLineWidth, 0.0))
            {
                return;
            }

            // ensure there are no curves used
            B2DPolygon aSource(rPolygon);

            if(aSource.areControlPointsUsed())
            {
            const double fPrecisionFactor = 0.25;
                aSource = adaptiveSubdivideByDistance( aSource, fLineWidth * fPrecisionFactor );
            }

            const sal_uInt32 nPointCount(aSource.count());

            if(!nPointCount)
            {
                return;
            }

            const sal_uInt32 nEdgeCount(aSource.isClosed() ? nPointCount : nPointCount - 1);
            B2DPoint aCurrent(aSource.getB2DPoint(0));

            ro_Result.reserve(ro_Result.size() + (3 * nEdgeCount));

            for(sal_uInt32 a(0); a < nEdgeCount; a++)
            {
                const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                const B2DPoint aNext(aSource.getB2DPoint(nNextIndex));

                createLineTrapezoidFromEdge(ro_Result, aCurrent, aNext, fLineWidth);
                aCurrent = aNext;
            }
        }

        void createLineTrapezoidFromB2DPolyPolygon(
            B2DTrapezoidVector& ro_Result,
            const B2DPolyPolygon& rPolyPolygon,
            double fLineWidth)
        {
            if(fTools::lessOrEqual(fLineWidth, 0.0))
            {
                return;
            }

            // ensure there are no curves used
            B2DPolyPolygon aSource(rPolyPolygon);

            if(aSource.areControlPointsUsed())
            {
                aSource = aSource.getDefaultAdaptiveSubdivision();
            }

            const sal_uInt32 nCount(aSource.count());

            if(!nCount)
            {
                return;
            }

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                createLineTrapezoidFromB2DPolygon(
                    ro_Result,
                    aSource.getB2DPolygon(a),
                    fLineWidth);
            }
        }

    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
