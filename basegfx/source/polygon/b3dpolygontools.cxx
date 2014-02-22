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

#include <osl/diagnose.h>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/tuple/b3ituple.hxx>
#include <numeric>



namespace basegfx
{
    namespace tools
    {
        
        void checkClosed(B3DPolygon& rCandidate)
        {
            while(rCandidate.count() > 1L
                && rCandidate.getB3DPoint(0L).equal(rCandidate.getB3DPoint(rCandidate.count() - 1L)))
            {
                rCandidate.setClosed(true);
                rCandidate.remove(rCandidate.count() - 1L);
            }
        }

        sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const B3DPolygon& rCandidate)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(nIndex + 1L < rCandidate.count())
            {
                return nIndex + 1L;
            }
            else
            {
                return 0L;
            }
        }

        B3DRange getRange(const B3DPolygon& rCandidate)
        {
            B3DRange aRetval;
            const sal_uInt32 nPointCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPointCount; a++)
            {
                const B3DPoint aTestPoint(rCandidate.getB3DPoint(a));
                aRetval.expand(aTestPoint);
            }

            return aRetval;
        }

        B3DVector getNormal(const B3DPolygon& rCandidate)
        {
            return rCandidate.getNormal();
        }

        double getLength(const B3DPolygon& rCandidate)
        {
            double fRetval(0.0);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1L)
            {
                const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);

                for(sal_uInt32 a(0L); a < nLoopCount; a++)
                {
                    const sal_uInt32 nNextIndex(getIndexOfSuccessor(a, rCandidate));
                    const B3DPoint aCurrentPoint(rCandidate.getB3DPoint(a));
                    const B3DPoint aNextPoint(rCandidate.getB3DPoint(nNextIndex));
                    const B3DVector aVector(aNextPoint - aCurrentPoint);
                    fRetval += aVector.getLength();
                }
            }

            return fRetval;
        }

        void applyLineDashing(const B3DPolygon& rCandidate, const ::std::vector<double>& rDotDashArray, B3DPolyPolygon* pLineTarget, B3DPolyPolygon* pGapTarget, double fDotDashLength)
        {
            const sal_uInt32 nPointCount(rCandidate.count());
            const sal_uInt32 nDotDashCount(rDotDashArray.size());

            if(fTools::lessOrEqual(fDotDashLength, 0.0))
            {
                fDotDashLength = ::std::accumulate(rDotDashArray.begin(), rDotDashArray.end(), 0.0);
            }

            if(fTools::more(fDotDashLength, 0.0) && (pLineTarget || pGapTarget) && nPointCount)
            {
                
                if(pLineTarget)
                {
                    pLineTarget->clear();
                }

                if(pGapTarget)
                {
                    pGapTarget->clear();
                }

                
                B3DPoint aCurrentPoint(rCandidate.getB3DPoint(0));
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);

                
                sal_uInt32 nDotDashIndex(0);
                bool bIsLine(true);
                double fDotDashMovingLength(rDotDashArray[0]);
                B3DPolygon aSnippet;

                
                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    
                    double fLastDotDashMovingLength(0.0);
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    const B3DPoint aNextPoint(rCandidate.getB3DPoint(nNextIndex));
                    const double fEdgeLength(B3DVector(aNextPoint - aCurrentPoint).getLength());

                    if(!fTools::equalZero(fEdgeLength))
                    {
                        while(fTools::less(fDotDashMovingLength, fEdgeLength))
                        {
                            
                            const bool bHandleLine(bIsLine && pLineTarget);
                            const bool bHandleGap(!bIsLine && pGapTarget);

                            if(bHandleLine || bHandleGap)
                            {
                                if(!aSnippet.count())
                                {
                                    aSnippet.append(interpolate(aCurrentPoint, aNextPoint, fLastDotDashMovingLength / fEdgeLength));
                                }

                                aSnippet.append(interpolate(aCurrentPoint, aNextPoint, fDotDashMovingLength / fEdgeLength));

                                if(bHandleLine)
                                {
                                    pLineTarget->append(aSnippet);
                                }
                                else
                                {
                                    pGapTarget->append(aSnippet);
                                }

                                aSnippet.clear();
                            }

                            
                            fLastDotDashMovingLength = fDotDashMovingLength;
                            fDotDashMovingLength += rDotDashArray[(++nDotDashIndex) % nDotDashCount];
                            bIsLine = !bIsLine;
                        }

                        
                        const bool bHandleLine(bIsLine && pLineTarget);
                        const bool bHandleGap(!bIsLine && pGapTarget);

                        if(bHandleLine || bHandleGap)
                        {
                            if(!aSnippet.count())
                            {
                                aSnippet.append(interpolate(aCurrentPoint, aNextPoint, fLastDotDashMovingLength / fEdgeLength));
                            }

                            aSnippet.append(aNextPoint);
                        }

                        
                        fDotDashMovingLength -= fEdgeLength;
                    }

                    
                    aCurrentPoint = aNextPoint;
                }

                
                if(aSnippet.count())
                {
                    if(bIsLine && pLineTarget)
                    {
                        pLineTarget->append(aSnippet);
                    }
                    else if(!bIsLine && pGapTarget)
                    {
                        pGapTarget->append(aSnippet);
                    }
                }

                
                if(pLineTarget)
                {
                    const sal_uInt32 nCount(pLineTarget->count());

                    if(nCount > 1)
                    {
                        
                        
                        const B3DPolygon aFirst(pLineTarget->getB3DPolygon(0));
                        B3DPolygon aLast(pLineTarget->getB3DPolygon(nCount - 1));

                        if(aFirst.getB3DPoint(0).equal(aLast.getB3DPoint(aLast.count() - 1)))
                        {
                            
                            aLast.append(aFirst);
                            aLast.removeDoublePoints();
                            pLineTarget->setB3DPolygon(0, aLast);
                            pLineTarget->remove(nCount - 1);
                        }
                    }
                }

                if(pGapTarget)
                {
                    const sal_uInt32 nCount(pGapTarget->count());

                    if(nCount > 1)
                    {
                        
                        
                        const B3DPolygon aFirst(pGapTarget->getB3DPolygon(0));
                        B3DPolygon aLast(pGapTarget->getB3DPolygon(nCount - 1));

                        if(aFirst.getB3DPoint(0).equal(aLast.getB3DPoint(aLast.count() - 1)))
                        {
                            
                            aLast.append(aFirst);
                            aLast.removeDoublePoints();
                            pGapTarget->setB3DPolygon(0, aLast);
                            pGapTarget->remove(nCount - 1);
                        }
                    }
                }
            }
            else
            {
                
                if(pLineTarget)
                {
                    pLineTarget->append(rCandidate);
                }

                if(pGapTarget)
                {
                    pGapTarget->append(rCandidate);
                }
            }
        }

        B3DPolygon applyDefaultNormalsSphere( const B3DPolygon& rCandidate, const B3DPoint& rCenter)
        {
            B3DPolygon aRetval(rCandidate);

            for(sal_uInt32 a(0L); a < aRetval.count(); a++)
            {
                B3DVector aVector(aRetval.getB3DPoint(a) - rCenter);
                aVector.normalize();
                aRetval.setNormal(a, aVector);
            }

            return aRetval;
        }

        B3DPolygon invertNormals( const B3DPolygon& rCandidate)
        {
            B3DPolygon aRetval(rCandidate);

            if(aRetval.areNormalsUsed())
            {
                for(sal_uInt32 a(0L); a < aRetval.count(); a++)
                {
                    aRetval.setNormal(a, -aRetval.getNormal(a));
                }
            }

            return aRetval;
        }

        B3DPolygon applyDefaultTextureCoordinatesParallel( const B3DPolygon& rCandidate, const B3DRange& rRange, bool bChangeX, bool bChangeY)
        {
            B3DPolygon aRetval(rCandidate);

            if(bChangeX || bChangeY)
            {
                
                
                const double fWidth(rRange.getWidth());
                const double fHeight(rRange.getHeight());
                const bool bWidthSet(!fTools::equalZero(fWidth));
                const bool bHeightSet(!fTools::equalZero(fHeight));
                const double fOne(1.0);

                for(sal_uInt32 a(0L); a < aRetval.count(); a++)
                {
                    const B3DPoint aPoint(aRetval.getB3DPoint(a));
                    B2DPoint aTextureCoordinate(aRetval.getTextureCoordinate(a));

                    if(bChangeX)
                    {
                        if(bWidthSet)
                        {
                            aTextureCoordinate.setX((aPoint.getX() - rRange.getMinX()) / fWidth);
                        }
                        else
                        {
                            aTextureCoordinate.setX(0.0);
                        }
                    }

                    if(bChangeY)
                    {
                        if(bHeightSet)
                        {
                            aTextureCoordinate.setY(fOne - ((aPoint.getY() - rRange.getMinY()) / fHeight));
                        }
                        else
                        {
                            aTextureCoordinate.setY(fOne);
                        }
                    }

                    aRetval.setTextureCoordinate(a, aTextureCoordinate);
                }
            }

            return aRetval;
        }

        B3DPolygon applyDefaultTextureCoordinatesSphere( const B3DPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX, bool bChangeY)
        {
            B3DPolygon aRetval(rCandidate);

            if(bChangeX || bChangeY)
            {
                
                
                const double fOne(1.0);
                const sal_uInt32 nPointCount(aRetval.count());
                bool bPolarPoints(false);
                sal_uInt32 a;

                
                
                const B3DRange aPlaneRange(getRange(rCandidate));
                const B3DPoint aPlaneCenter(aPlaneRange.getCenter() - rCenter);
                const double fXCenter(fOne - ((atan2(aPlaneCenter.getZ(), aPlaneCenter.getX()) + F_PI) / F_2PI));

                for(a = 0L; a < nPointCount; a++)
                {
                    const B3DVector aVector(aRetval.getB3DPoint(a) - rCenter);
                    const double fY(fOne - ((atan2(aVector.getY(), aVector.getXZLength()) + F_PI2) / F_PI));
                    B2DPoint aTexCoor(aRetval.getTextureCoordinate(a));

                    if(fTools::equalZero(fY))
                    {
                        
                        if(bChangeY)
                        {
                            aTexCoor.setY(0.0);

                            if(bChangeX)
                            {
                                bPolarPoints = true;
                            }
                        }
                    }
                    else if(fTools::equal(fY, fOne))
                    {
                        
                        
                        if(bChangeY)
                        {
                            aTexCoor.setY(fOne);

                            if(bChangeX)
                            {
                                bPolarPoints = true;
                            }
                        }
                    }
                    else
                    {
                        double fX(fOne - ((atan2(aVector.getZ(), aVector.getX()) + F_PI) / F_2PI));

                        
                        if(fX > fXCenter + 0.5)
                        {
                            fX -= fOne;
                        }
                        else if(fX < fXCenter - 0.5)
                        {
                            fX += fOne;
                        }

                        if(bChangeX)
                        {
                            aTexCoor.setX(fX);
                        }

                        if(bChangeY)
                        {
                            aTexCoor.setY(fY);
                        }
                    }

                    aRetval.setTextureCoordinate(a, aTexCoor);
                }

                if(bPolarPoints)
                {
                    
                    
                    for(a = 0L; a < nPointCount; a++)
                    {
                        B2DPoint aTexCoor(aRetval.getTextureCoordinate(a));

                        if(fTools::equalZero(aTexCoor.getY()) || fTools::equal(aTexCoor.getY(), fOne))
                        {
                            
                            const B2DPoint aPrevTexCoor(aRetval.getTextureCoordinate(a ? a - 1L : nPointCount - 1L));
                            const B2DPoint aNextTexCoor(aRetval.getTextureCoordinate((a + 1L) % nPointCount));
                            const bool bPrevPole(fTools::equalZero(aPrevTexCoor.getY()) || fTools::equal(aPrevTexCoor.getY(), fOne));
                            const bool bNextPole(fTools::equalZero(aNextTexCoor.getY()) || fTools::equal(aNextTexCoor.getY(), fOne));

                            if(!bPrevPole && !bNextPole)
                            {
                                
                                aTexCoor.setX((aPrevTexCoor.getX() + aNextTexCoor.getX()) / 2.0);
                            }
                            else if(!bNextPole)
                            {
                                
                                aTexCoor.setX(aNextTexCoor.getX());
                            }
                            else
                            {
                                
                                aTexCoor.setX(aPrevTexCoor.getX());
                            }

                            aRetval.setTextureCoordinate(a, aTexCoor);
                        }
                    }
                }
            }

            return aRetval;
        }

        bool isInside(const B3DPolygon& rCandidate, const B3DPoint& rPoint, bool bWithBorder)
        {
            if(bWithBorder && isPointOnPolygon(rCandidate, rPoint, true))
            {
                return true;
            }
            else
            {
                bool bRetval(false);
                const B3DVector aPlaneNormal(rCandidate.getNormal());

                if(!aPlaneNormal.equalZero())
                {
                    const sal_uInt32 nPointCount(rCandidate.count());

                    if(nPointCount)
                    {
                        B3DPoint aCurrentPoint(rCandidate.getB3DPoint(nPointCount - 1));
                        const double fAbsX(fabs(aPlaneNormal.getX()));
                        const double fAbsY(fabs(aPlaneNormal.getY()));
                        const double fAbsZ(fabs(aPlaneNormal.getZ()));

                        if(fAbsX > fAbsY && fAbsX > fAbsZ)
                        {
                            
                            
                            for(sal_uInt32 a(0); a < nPointCount; a++)
                            {
                                const B3DPoint aPreviousPoint(aCurrentPoint);
                                aCurrentPoint = rCandidate.getB3DPoint(a);

                                
                                const bool bCompZA(fTools::more(aPreviousPoint.getZ(), rPoint.getZ()));
                                const bool bCompZB(fTools::more(aCurrentPoint.getZ(), rPoint.getZ()));

                                if(bCompZA != bCompZB)
                                {
                                    
                                    const bool bCompYA(fTools::more(aPreviousPoint.getY(), rPoint.getY()));
                                    const bool bCompYB(fTools::more(aCurrentPoint.getY(), rPoint.getY()));

                                    if(bCompYA == bCompYB)
                                    {
                                        if(bCompYA)
                                        {
                                            bRetval = !bRetval;
                                        }
                                    }
                                    else
                                    {
                                        const double fCompare(
                                            aCurrentPoint.getY() - (aCurrentPoint.getZ() - rPoint.getZ()) *
                                            (aPreviousPoint.getY() - aCurrentPoint.getY()) /
                                            (aPreviousPoint.getZ() - aCurrentPoint.getZ()));

                                        if(fTools::more(fCompare, rPoint.getY()))
                                        {
                                            bRetval = !bRetval;
                                        }
                                    }
                                }
                            }
                        }
                        else if(fAbsY > fAbsX && fAbsY > fAbsZ)
                        {
                            
                            
                            for(sal_uInt32 a(0); a < nPointCount; a++)
                            {
                                const B3DPoint aPreviousPoint(aCurrentPoint);
                                aCurrentPoint = rCandidate.getB3DPoint(a);

                                
                                const bool bCompZA(fTools::more(aPreviousPoint.getZ(), rPoint.getZ()));
                                const bool bCompZB(fTools::more(aCurrentPoint.getZ(), rPoint.getZ()));

                                if(bCompZA != bCompZB)
                                {
                                    
                                    const bool bCompXA(fTools::more(aPreviousPoint.getX(), rPoint.getX()));
                                    const bool bCompXB(fTools::more(aCurrentPoint.getX(), rPoint.getX()));

                                    if(bCompXA == bCompXB)
                                    {
                                        if(bCompXA)
                                        {
                                            bRetval = !bRetval;
                                        }
                                    }
                                    else
                                    {
                                        const double fCompare(
                                            aCurrentPoint.getX() - (aCurrentPoint.getZ() - rPoint.getZ()) *
                                            (aPreviousPoint.getX() - aCurrentPoint.getX()) /
                                            (aPreviousPoint.getZ() - aCurrentPoint.getZ()));

                                        if(fTools::more(fCompare, rPoint.getX()))
                                        {
                                            bRetval = !bRetval;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            
                            
                            for(sal_uInt32 a(0); a < nPointCount; a++)
                            {
                                const B3DPoint aPreviousPoint(aCurrentPoint);
                                aCurrentPoint = rCandidate.getB3DPoint(a);

                                
                                const bool bCompYA(fTools::more(aPreviousPoint.getY(), rPoint.getY()));
                                const bool bCompYB(fTools::more(aCurrentPoint.getY(), rPoint.getY()));

                                if(bCompYA != bCompYB)
                                {
                                    
                                    const bool bCompXA(fTools::more(aPreviousPoint.getX(), rPoint.getX()));
                                    const bool bCompXB(fTools::more(aCurrentPoint.getX(), rPoint.getX()));

                                    if(bCompXA == bCompXB)
                                    {
                                        if(bCompXA)
                                        {
                                            bRetval = !bRetval;
                                        }
                                    }
                                    else
                                    {
                                        const double fCompare(
                                            aCurrentPoint.getX() - (aCurrentPoint.getY() - rPoint.getY()) *
                                            (aPreviousPoint.getX() - aCurrentPoint.getX()) /
                                            (aPreviousPoint.getY() - aCurrentPoint.getY()));

                                        if(fTools::more(fCompare, rPoint.getX()))
                                        {
                                            bRetval = !bRetval;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                return bRetval;
            }
        }

        bool isPointOnLine(const B3DPoint& rStart, const B3DPoint& rEnd, const B3DPoint& rCandidate, bool bWithPoints)
        {
            if(rCandidate.equal(rStart) || rCandidate.equal(rEnd))
            {
                
                return bWithPoints;
            }
            else if(rStart.equal(rEnd))
            {
                
                return false;
            }
            else
            {
                const B3DVector aEdgeVector(rEnd - rStart);
                const B3DVector aTestVector(rCandidate - rStart);

                if(areParallel(aEdgeVector, aTestVector))
                {
                    const double fZero(0.0);
                    const double fOne(1.0);
                    double fParamTestOnCurr(0.0);

                    if(aEdgeVector.getX() > aEdgeVector.getY())
                    {
                        if(aEdgeVector.getX() > aEdgeVector.getZ())
                        {
                            
                            fParamTestOnCurr = aTestVector.getX() / aEdgeVector.getX();
                        }
                        else
                        {
                            
                            fParamTestOnCurr = aTestVector.getZ() / aEdgeVector.getZ();
                        }
                    }
                    else
                    {
                        if(aEdgeVector.getY() > aEdgeVector.getZ())
                        {
                            
                            fParamTestOnCurr = aTestVector.getY() / aEdgeVector.getY();
                        }
                        else
                        {
                            
                            fParamTestOnCurr = aTestVector.getZ() / aEdgeVector.getZ();
                        }
                    }

                    if(fTools::more(fParamTestOnCurr, fZero) && fTools::less(fParamTestOnCurr, fOne))
                    {
                        return true;
                    }
                }

                return false;
            }
        }

        bool isPointOnPolygon(const B3DPolygon& rCandidate, const B3DPoint& rPoint, bool bWithPoints)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1L)
            {
                const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                B3DPoint aCurrentPoint(rCandidate.getB3DPoint(0));

                for(sal_uInt32 a(0); a < nLoopCount; a++)
                {
                    const B3DPoint aNextPoint(rCandidate.getB3DPoint((a + 1) % nPointCount));

                    if(isPointOnLine(aCurrentPoint, aNextPoint, rPoint, bWithPoints))
                    {
                        return true;
                    }

                    aCurrentPoint = aNextPoint;
                }
            }
            else if(nPointCount && bWithPoints)
            {
                return rPoint.equal(rCandidate.getB3DPoint(0));
            }

            return false;
        }

        bool getCutBetweenLineAndPlane(const B3DVector& rPlaneNormal, const B3DPoint& rPlanePoint, const B3DPoint& rEdgeStart, const B3DPoint& rEdgeEnd, double& fCut)
        {
            if(!rPlaneNormal.equalZero() && !rEdgeStart.equal(rEdgeEnd))
            {
                const B3DVector aTestEdge(rEdgeEnd - rEdgeStart);
                const double fScalarEdge(rPlaneNormal.scalar(aTestEdge));

                if(!fTools::equalZero(fScalarEdge))
                {
                    const B3DVector aCompareEdge(rPlanePoint - rEdgeStart);
                    const double fScalarCompare(rPlaneNormal.scalar(aCompareEdge));

                    fCut = fScalarCompare / fScalarEdge;
                    return true;
                }
            }

            return false;
        }

        
        

        bool equal(const B3DPolygon& rCandidateA, const B3DPolygon& rCandidateB, const double& rfSmallValue)
        {
            const sal_uInt32 nPointCount(rCandidateA.count());

            if(nPointCount != rCandidateB.count())
                return false;

            const bool bClosed(rCandidateA.isClosed());

            if(bClosed != rCandidateB.isClosed())
                return false;

            for(sal_uInt32 a(0); a < nPointCount; a++)
            {
                const B3DPoint aPoint(rCandidateA.getB3DPoint(a));

                if(!aPoint.equal(rCandidateB.getB3DPoint(a), rfSmallValue))
                    return false;
            }

            return true;
        }

        
        B3DPolygon snapPointsOfHorizontalOrVerticalEdges(const B3DPolygon& rCandidate)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1)
            {
                
                
                B3DPolygon aRetval(rCandidate);

                
                B3ITuple aPrevTuple(basegfx::fround(rCandidate.getB3DPoint(nPointCount - 1)));
                B3DPoint aCurrPoint(rCandidate.getB3DPoint(0));
                B3ITuple aCurrTuple(basegfx::fround(aCurrPoint));

                
                
                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    
                    const bool bLastRun(a + 1 == nPointCount);
                    const sal_uInt32 nNextIndex(bLastRun ? 0 : a + 1);
                    const B3DPoint aNextPoint(rCandidate.getB3DPoint(nNextIndex));
                    const B3ITuple aNextTuple(basegfx::fround(aNextPoint));

                    
                    const bool bPrevVertical(aPrevTuple.getX() == aCurrTuple.getX());
                    const bool bNextVertical(aNextTuple.getX() == aCurrTuple.getX());
                    const bool bPrevHorizontal(aPrevTuple.getY() == aCurrTuple.getY());
                    const bool bNextHorizontal(aNextTuple.getY() == aCurrTuple.getY());
                    const bool bSnapX(bPrevVertical || bNextVertical);
                    const bool bSnapY(bPrevHorizontal || bNextHorizontal);

                    if(bSnapX || bSnapY)
                    {
                        const B3DPoint aSnappedPoint(
                            bSnapX ? aCurrTuple.getX() : aCurrPoint.getX(),
                            bSnapY ? aCurrTuple.getY() : aCurrPoint.getY(),
                            aCurrPoint.getZ());

                        aRetval.setB3DPoint(a, aSnappedPoint);
                    }

                    
                    if(!bLastRun)
                    {
                        aPrevTuple = aCurrTuple;
                        aCurrPoint = aNextPoint;
                        aCurrTuple = aNextTuple;
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
