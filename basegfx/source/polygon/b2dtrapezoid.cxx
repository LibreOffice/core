/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/polygon/b2dtrapezoid.hxx>
#include <basegfx/range/b1drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <list>



namespace basegfx
{
    namespace trapezoidhelper
    {
        
        
        
        
        

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

            
            const B2DPoint& getStart() const { return *mpStart; }
            const B2DPoint& getEnd() const { return *mpEnd; }
        };

        
        

        typedef ::std::vector< TrDeSimpleEdge > TrDeSimpleEdges;

        
        
        
        
        

        class TrDeEdgeEntry : public TrDeSimpleEdge
        {
        private:
            
            sal_uInt32          mnSortValue;

        public:
            
            double getDeltaX() const { return mpEnd->getX() - mpStart->getX(); }
            double getDeltaY() const { return mpEnd->getY() - mpStart->getY(); }

            
            
            sal_uInt32 getSortValue() const
            {
                if(0 != mnSortValue)
                    return mnSortValue;

                
                
                const double fRadiant(atan2(getDeltaY(), getDeltaX()) * (SAL_MAX_UINT32 / F_PI));

                
                const_cast< TrDeEdgeEntry* >(this)->mnSortValue = sal_uInt32(fRadiant);

                return mnSortValue;
            }

            
            TrDeEdgeEntry(
                const B2DPoint* pStart,
                const B2DPoint* pEnd,
                sal_uInt32 nSortValue = 0)
            :   TrDeSimpleEdge(pStart, pEnd),
                mnSortValue(nSortValue)
            {
                
                if(mpEnd->getY() < mpStart->getY())
                {
                    std::swap(mpStart, mpEnd);
                }

                
                OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
            }

            
            void setStart( const B2DPoint* pNewStart)
            {
                OSL_ENSURE(0 != pNewStart, "No null pointer allowed here (!)");

                if(mpStart != pNewStart)
                {
                    mpStart = pNewStart;

                    
                    OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
                }
            }

            
            void setEnd( const B2DPoint* pNewEnd)
            {
                OSL_ENSURE(0 != pNewEnd, "No null pointer allowed here (!)");

                if(mpEnd != pNewEnd)
                {
                    mpEnd = pNewEnd;

                    
                    OSL_ENSURE(mpEnd->getY() > mpStart->getY(), "Illegal TrDeEdgeEntry constructed (!)");
                }
            }

            
            bool operator<(const TrDeEdgeEntry& rComp) const
            {
                if(fTools::equal(getStart().getY(), rComp.getStart().getY(), fTools::getSmallValue()))
                {
                    if(fTools::equal(getStart().getX(), rComp.getStart().getX(), fTools::getSmallValue()))
                    {
                        
                        
                        
                        
                        
                        
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

            
            B2DPoint getCutPointForGivenY(double fGivenY)
            {
                
                
                const double fFactor((fGivenY - getStart().getY()) / getDeltaY());
                const double fDeltaXNew(fFactor * getDeltaX());

                return B2DPoint(getStart().getX() + fDeltaXNew, fGivenY);
            }
        };

        
        

        typedef ::std::list< TrDeEdgeEntry > TrDeEdgeEntries;

    } 
} 



namespace basegfx
{
    namespace trapezoidhelper
    {
        

        
        class PointBlockAllocator
        {
            static const size_t nBlockSize = 32;
            size_t nCurPoint;
            B2DPoint *mpPointBase;
            
            B2DPoint maFirstStackBlock[nBlockSize];
            std::vector< B2DPoint * > maBlocks;
        public:
            PointBlockAllocator() :
                nCurPoint( nBlockSize ),
                mpPointBase( maFirstStackBlock )
            {
            }

            ~PointBlockAllocator()
            {
                while(maBlocks.size() > 0)
                {
                    delete [] maBlocks.back();
                    maBlocks.pop_back();
                }
            }

            B2DPoint *allocatePoint()
            {
                if(nCurPoint >= nBlockSize)
                {
                    mpPointBase = new B2DPoint[nBlockSize];
                    maBlocks.push_back(mpPointBase);
                    nCurPoint = 0;
                }
                return mpPointBase + nCurPoint++;
            }

            B2DPoint *allocatePoint(const B2DTuple &rPoint)
            {
                B2DPoint *pPoint = allocatePoint();
                *pPoint = rPoint;
                return pPoint;
            }

            
            void freeIfLast(B2DPoint *pPoint)
            {
                
                if ( nCurPoint > 0 && pPoint == mpPointBase + nCurPoint - 1 )
                    nCurPoint--;
            }
        };

        
        class TrapezoidSubdivider
        {
        private:
            
            sal_uInt32                  mnInitialEdgeEntryCount;
            TrDeEdgeEntries             maTrDeEdgeEntries;
            ::std::vector< B2DPoint >   maPoints;
            
            PointBlockAllocator         maNewPoints;

            void addEdgeSorted(
                TrDeEdgeEntries::iterator aCurrent,
                const TrDeEdgeEntry& rNewEdge)
            {
                
                while(aCurrent != maTrDeEdgeEntries.end() && (*aCurrent) < rNewEdge)
                {
                    ++aCurrent;
                }

                
                maTrDeEdgeEntries.insert(aCurrent, rNewEdge);
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
                    
                    
                    aEdge.setStart(&rCutPoint);
                    return false;
                }

                const double fNewDeltaYStart(aEdge.getEnd().getY() - rCutPoint.getY());

                if(fTools::lessOrEqual(fNewDeltaYStart, 0.0))
                {
                    
                    
                    aEdge.setEnd(&rCutPoint);
                    return false;
                }

                
                const TrDeEdgeEntry aNewEdge(
                    &rCutPoint,
                    &aEdge.getEnd(),
                    aEdge.getSortValue());

                
                aEdge.setEnd(&rCutPoint);

                
                addEdgeSorted(aCurrent, aNewEdge);

                return true;
            }

            bool testAndCorrectEdgeIntersection(
                TrDeEdgeEntries::reference aEdgeA,
                TrDeEdgeEntries::reference aEdgeB,
                TrDeEdgeEntries::iterator aCurrent)
            {
                
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

                
                if(aEdgeA.getStart().equal(aEdgeA.getEnd(), fTools::getSmallValue()))
                {
                    return false;
                }

                if(aEdgeB.getStart().equal(aEdgeB.getEnd(), fTools::getSmallValue()))
                {
                    return false;
                }

                
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
                        ? maNewPoints.allocatePoint(aEdgeA.getStart() + (fCutA * aDeltaA))
                        : maNewPoints.allocatePoint(aEdgeB.getStart() + (fCutB * aDeltaB));

                    
                    bool bRetval = splitEdgeAtGivenPoint(aEdgeA, *pNewPoint, aCurrent);
                    bRetval |= splitEdgeAtGivenPoint(aEdgeB, *pNewPoint, aCurrent);

                    if(!bRetval)
                        maNewPoints.freeIfLast(pNewPoint);

                    return bRetval;
                }

                return false;
            }

            void solveHorizontalEdges(TrDeSimpleEdges& rTrDeSimpleEdges)
            {
                if(rTrDeSimpleEdges.size() && maTrDeEdgeEntries.size())
                {
                    
                    
                    
                    sal_uInt32 a(0);

                    for(a = 0; a < rTrDeSimpleEdges.size(); a++)
                    {
                        
                        const TrDeSimpleEdge& rHorEdge = rTrDeSimpleEdges[a];
                        const B1DRange aRange(rHorEdge.getStart().getX(), rHorEdge.getEnd().getX());
                        const double fFixedY(rHorEdge.getStart().getY());

                        
                        TrDeEdgeEntries::iterator aCurrent(maTrDeEdgeEntries.begin());

                        do
                        {
                            
                            TrDeEdgeEntries::reference aCompare(*aCurrent++);

                            if(fTools::lessOrEqual(aCompare.getEnd().getY(), fFixedY))
                            {
                                
                                continue;
                            }

                            if(fTools::moreOrEqual(aCompare.getStart().getY(), fFixedY))
                            {
                                
                                continue;
                            }

                            
                            const B1DRange aCompareRange(aCompare.getStart().getX(), aCompare.getEnd().getX());

                            if(aRange.overlaps(aCompareRange))
                            {
                                
                                const B2DPoint aSplit(aCompare.getCutPointForGivenY(fFixedY));

                                if(fTools::more(aSplit.getX(), aRange.getMinimum())
                                    && fTools::less(aSplit.getX(), aRange.getMaximum()))
                                {
                                    
                                    B2DPoint* pNewPoint = maNewPoints.allocatePoint(aSplit);

                                    if(!splitEdgeAtGivenPoint(aCompare, *pNewPoint, aCurrent))
                                    {
                                        maNewPoints.freeIfLast(pNewPoint);
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

                
                if(aSource.areControlPointsUsed())
                {
                    aSource = aSource.getDefaultAdaptiveSubdivision();
                }

                for(a = 0; a < nPolygonCount; a++)
                {
                    
                    const B2DPolygon aPolygonCandidate(aSource.getB2DPolygon(a));
                    const sal_uInt32 nCount(aPolygonCandidate.count());

                    if(nCount > 2)
                    {
                        nAllPointCount += nCount;
                    }
                }

                if(nAllPointCount)
                {
                    
                    
                    maPoints.reserve(nAllPointCount);

                    for(a = 0; a < nPolygonCount; a++)
                    {
                        
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

                    
                    
                    
                    
                    sal_uInt32 nStartIndex(0);

                    for(a = 0; a < nPolygonCount; a++)
                    {
                        const B2DPolygon aPolygonCandidate(aSource.getB2DPolygon(a));
                        const sal_uInt32 nCount(aPolygonCandidate.count());

                        if(nCount > 2)
                        {
                            
                            B2DPoint* pPrev(&maPoints[nCount + nStartIndex - 1]);

                            for(b = 0; b < nCount; b++)
                            {
                                
                                B2DPoint* pCurr(&maPoints[nStartIndex++]);

                                if(fTools::equal(pPrev->getY(), pCurr->getY(), fTools::getSmallValue()))
                                {
                                    
                                    if(!fTools::equal(pPrev->getX(), pCurr->getX(), fTools::getSmallValue()))
                                    {
                                        
                                        aTrDeSimpleEdges.push_back(TrDeSimpleEdge(pPrev, pCurr));

                                        const double fMiddle((pPrev->getY() + pCurr->getY()) * 0.5);
                                        pPrev->setY(fMiddle);
                                        pCurr->setY(fMiddle);
                                    }
                                }
                                else
                                {
                                    
                                    
                                    maTrDeEdgeEntries.push_back(TrDeEdgeEntry(pPrev, pCurr, 0));
                                    mnInitialEdgeEntryCount++;
                                }

                                
                                pPrev = pCurr;
                            }
                        }
                    }
                }

                if(!maTrDeEdgeEntries.empty())
                {
                    
                    maTrDeEdgeEntries.sort();

                    
                    solveHorizontalEdges(aTrDeSimpleEdges);
                }
            }

            void Subdivide(B2DTrapezoidVector& ro_Result)
            {
                
                
                
                //
                
                
                
                
                //
                
                
                
                
                
                //
                
                
                
                //
                
                
                
                B1DRange aLeftRange;
                B1DRange aRightRange;

                if(!maTrDeEdgeEntries.empty())
                {
                    
                    
                    
                    
                    
                    ro_Result.reserve(ro_Result.size() + mnInitialEdgeEntryCount);
                }

                while(!maTrDeEdgeEntries.empty())
                {
                    
                    TrDeEdgeEntries::iterator aCurrent(maTrDeEdgeEntries.begin());
                    TrDeEdgeEntries::reference aLeft(*aCurrent++);

                    if(aCurrent == maTrDeEdgeEntries.end())
                    {
                        
                        
                        
                        
                        OSL_FAIL("Trapeziod decomposer in illegal state (!)");
                        maTrDeEdgeEntries.pop_front();
                        continue;
                    }

                    
                    TrDeEdgeEntries::reference aRight(*aCurrent++);

                    if(!fTools::equal(aLeft.getStart().getY(), aRight.getStart().getY(), fTools::getSmallValue()))
                    {
                        
                        
                        
                        
                        OSL_FAIL("Trapeziod decomposer in illegal state (!)");
                        maTrDeEdgeEntries.pop_front();
                        continue;
                    }

                    
                    
                    
                    
                    
                    
                    B2DPoint aLeftEnd(aLeft.getEnd());
                    B2DPoint aRightEnd(aRight.getEnd());

                    
                    
                    const bool bEndOnSameLine(fTools::equal(aLeftEnd.getY(), aRightEnd.getY(), fTools::getSmallValue()));
                    bool bLeftIsLonger(false);

                    if(!bEndOnSameLine)
                    {
                        
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

                    
                    const bool bSameStartPoint(aLeft.getStart().equal(aRight.getStart(), fTools::getSmallValue()));
                    const bool bSameEndPoint(aLeftEnd.equal(aRightEnd, fTools::getSmallValue()));

                    
                    if(bSameStartPoint && bSameEndPoint)
                    {
                        
                        if(!bEndOnSameLine)
                        {
                            if(bLeftIsLonger)
                            {
                                B2DPoint* pNewPoint = maNewPoints.allocatePoint(aLeftEnd);

                                if(!splitEdgeAtGivenPoint(aLeft, *pNewPoint, aCurrent))
                                {
                                    maNewPoints.freeIfLast(pNewPoint);
                                }
                            }
                            else
                            {
                                B2DPoint* pNewPoint = maNewPoints.allocatePoint(aRightEnd);

                                if(!splitEdgeAtGivenPoint(aRight, *pNewPoint, aCurrent))
                                {
                                    maNewPoints.freeIfLast(pNewPoint);
                                }
                            }
                        }

                        
                        maTrDeEdgeEntries.pop_front();
                        maTrDeEdgeEntries.pop_front();

                        continue;
                    }

                    
                    
                    bool bRangesSet(false);

                    if(!(bSameStartPoint || bSameEndPoint))
                    {
                        
                        aLeftRange = B1DRange(aLeft.getStart().getX(), aLeftEnd.getX());
                        aRightRange = B1DRange(aRight.getStart().getX(), aRightEnd.getX());
                        bRangesSet = true;

                        
                        if(aLeftRange.overlaps(aRightRange))
                        {
                            
                            
                            if(testAndCorrectEdgeIntersection(aLeft, aRight, aCurrent))
                            {
                                continue;
                            }
                        }
                    }

                    
                    
                    if(aCurrent != maTrDeEdgeEntries.end()
                        && fTools::less(aCurrent->getStart().getY(), aLeftEnd.getY()))
                    {
                        
                        if(!bRangesSet)
                        {
                            aLeftRange = B1DRange(aLeft.getStart().getX(), aLeftEnd.getX());
                            aRightRange = B1DRange(aRight.getStart().getX(), aRightEnd.getX());
                        }

                        
                        B1DRange aAllRange(aLeftRange);
                        aAllRange.expand(aRightRange);

                        
                        
                        TrDeEdgeEntries::iterator aLoop(aCurrent);
                        bool bDone(false);

                        do
                        {
                            
                            TrDeEdgeEntries::reference aCompare(*aLoop++);

                            
                            
                            
                            if(aCompare.getStart().equal(aRight.getStart(), fTools::getSmallValue()))
                            {
                                continue;
                            }

                            
                            const B1DRange aCompareRange(aCompare.getStart().getX(), aCompare.getEnd().getX());

                            
                            if(aAllRange.overlaps(aCompareRange))
                            {
                                
                                if(fTools::more(aCompare.getStart().getY(), aLeft.getStart().getY()))
                                {
                                    
                                    const B2DPoint aSplitLeft(aLeft.getCutPointForGivenY(aCompare.getStart().getY()));
                                    const B2DPoint aSplitRight(aRight.getCutPointForGivenY(aCompare.getStart().getY()));

                                    
                                    
                                    if(aCompare.getStart().getX() >= aSplitLeft.getX() &&
                                        aCompare.getStart().getX() <= aSplitRight.getX())
                                    {
                                        
                                        B2DPoint* pNewLeft = maNewPoints.allocatePoint(aSplitLeft);

                                        if(splitEdgeAtGivenPoint(aLeft, *pNewLeft, aCurrent))
                                        {
                                            bDone = true;
                                        }
                                        else
                                        {
                                            maNewPoints.freeIfLast(pNewLeft);
                                        }

                                        B2DPoint* pNewRight = maNewPoints.allocatePoint(aSplitRight);

                                        if(splitEdgeAtGivenPoint(aRight, *pNewRight, aCurrent))
                                        {
                                            bDone = true;
                                        }
                                        else
                                        {
                                            maNewPoints.freeIfLast(pNewRight);
                                        }
                                    }
                                }

                                if(!bDone && aLeftRange.overlaps(aCompareRange))
                                {
                                    
                                    bDone = testAndCorrectEdgeIntersection(aLeft, aCompare, aCurrent);
                                }

                                if(!bDone && aRightRange.overlaps(aCompareRange))
                                {
                                    
                                    bDone = testAndCorrectEdgeIntersection(aRight, aCompare, aCurrent);
                                }
                            }
                        }
                        while(!bDone
                            && aLoop != maTrDeEdgeEntries.end()
                            && fTools::less(aLoop->getStart().getY(), aLeftEnd.getY()));

                        if(bDone)
                        {
                            
                            continue;
                        }
                    }

                    
                    
                    
                    if(!bEndOnSameLine)
                    {
                        if(bLeftIsLonger)
                        {
                            B2DPoint* pNewPoint = maNewPoints.allocatePoint(aLeftEnd);

                            if(!splitEdgeAtGivenPoint(aLeft, *pNewPoint, aCurrent))
                            {
                                maNewPoints.freeIfLast(pNewPoint);
                            }
                        }
                        else
                        {
                            B2DPoint* pNewPoint = maNewPoints.allocatePoint(aRightEnd);

                            if(!splitEdgeAtGivenPoint(aRight, *pNewPoint, aCurrent))
                            {
                                maNewPoints.freeIfLast(pNewPoint);
                            }
                        }
                    }

                    
                    
                    
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
    } 
} 



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
        
        if(mfTopXLeft > mfTopXRight)
        {
            std::swap(mfTopXLeft, mfTopXRight);
        }

        
        if(mfBottomXLeft > mfBottomXRight)
        {
            std::swap(mfBottomXLeft, mfBottomXRight);
        }

        
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
} 



namespace basegfx
{
    namespace tools
    {
        
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
                
                return;
            }

            if(rPointA.equal(rPointB, fTools::getSmallValue()))
            {
                
                return;
            }

            const double fHalfLineWidth(0.5 * fLineWidth);

            if(fTools::equal(rPointA.getX(), rPointB.getX(), fTools::getSmallValue()))
            {
                
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
                
                
                const B2DVector aDelta(rPointB - rPointA);
                B2DVector aPerpendicular(-aDelta.getY(), aDelta.getX());
                aPerpendicular.setLength(fHalfLineWidth);

                
                const B2DPoint aStartLow(rPointA + aPerpendicular);
                const B2DPoint aStartHigh(rPointA - aPerpendicular);
                const B2DPoint aEndHigh(rPointB - aPerpendicular);
                const B2DPoint aEndLow(rPointB + aPerpendicular);

                
                basegfx::trapezoidhelper::TrDeEdgeEntries aTrDeEdgeEntries;

                aTrDeEdgeEntries.push_back(basegfx::trapezoidhelper::TrDeEdgeEntry(&aStartLow, &aStartHigh, 0));
                aTrDeEdgeEntries.push_back(basegfx::trapezoidhelper::TrDeEdgeEntry(&aStartHigh, &aEndHigh, 0));
                aTrDeEdgeEntries.push_back(basegfx::trapezoidhelper::TrDeEdgeEntry(&aEndHigh, &aEndLow, 0));
                aTrDeEdgeEntries.push_back(basegfx::trapezoidhelper::TrDeEdgeEntry(&aEndLow, &aStartLow, 0));
                aTrDeEdgeEntries.sort();

                
                
                
                basegfx::trapezoidhelper::TrDeEdgeEntries::iterator aCurrent(aTrDeEdgeEntries.begin());
                basegfx::trapezoidhelper::TrDeEdgeEntries::reference aLeft(*aCurrent++);
                basegfx::trapezoidhelper::TrDeEdgeEntries::reference aRight(*aCurrent++);
                const bool bEndOnSameLine(fTools::equal(aLeft.getEnd().getY(), aRight.getEnd().getY(), fTools::getSmallValue()));

                if(bEndOnSameLine)
                {
                    
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

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
