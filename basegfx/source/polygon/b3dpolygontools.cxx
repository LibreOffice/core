/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // B3DPolygon tools
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
                // clear targets
                if(pLineTarget)
                {
                    pLineTarget->clear();
                }

                if(pGapTarget)
                {
                    pGapTarget->clear();
                }

                // prepare current edge's start
                B3DPoint aCurrentPoint(rCandidate.getB3DPoint(0));
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);

                // prepare DotDashArray iteration and the line/gap switching bool
                sal_uInt32 nDotDashIndex(0);
                bool bIsLine(true);
                double fDotDashMovingLength(rDotDashArray[0]);
                B3DPolygon aSnippet;

                // iterate over all edges
                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // update current edge
                    double fLastDotDashMovingLength(0.0);
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    const B3DPoint aNextPoint(rCandidate.getB3DPoint(nNextIndex));
                    const double fEdgeLength(B3DVector(aNextPoint - aCurrentPoint).getLength());

                    while(fTools::less(fDotDashMovingLength, fEdgeLength))
                    {
                        // new split is inside edge, create and append snippet [fLastDotDashMovingLength, fDotDashMovingLength]
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

                        // prepare next DotDashArray step and flip line/gap flag
                        fLastDotDashMovingLength = fDotDashMovingLength;
                        fDotDashMovingLength += rDotDashArray[(++nDotDashIndex) % nDotDashCount];
                        bIsLine = !bIsLine;
                    }

                    // append snippet [fLastDotDashMovingLength, fEdgeLength]
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

                    // prepare move to next edge
                    fDotDashMovingLength -= fEdgeLength;

                    // prepare next edge step (end point gets new start point)
                    aCurrentPoint = aNextPoint;
                }

                // append last intermediate results (if exists)
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

                // check if start and end polygon may be merged
                if(pLineTarget)
                {
                    const sal_uInt32 nCount(pLineTarget->count());

                    if(nCount > 1)
                    {
                        // these polygons were created above, there exists none with less than two points,
                        // thus dircet point access below is allowed
                        const B3DPolygon aFirst(pLineTarget->getB3DPolygon(0));
                        B3DPolygon aLast(pLineTarget->getB3DPolygon(nCount - 1));

                        if(aFirst.getB3DPoint(0).equal(aLast.getB3DPoint(aLast.count() - 1)))
                        {
                            // start of first and end of last are the same -> merge them
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
                        // these polygons were created above, there exists none with less than two points,
                        // thus dircet point access below is allowed
                        const B3DPolygon aFirst(pGapTarget->getB3DPolygon(0));
                        B3DPolygon aLast(pGapTarget->getB3DPolygon(nCount - 1));

                        if(aFirst.getB3DPoint(0).equal(aLast.getB3DPoint(aLast.count() - 1)))
                        {
                            // start of first and end of last are the same -> merge them
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
                // parameters make no sense, just add source to targets
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
                // create projection of standard texture coordinates in (X, Y) onto
                // the 3d coordinates straight
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
                // create texture coordinates using sphere projection to cartesian coordinates,
                // use object's center as base
                const double fOne(1.0);
                const sal_uInt32 nPointCount(aRetval.count());
                bool bPolarPoints(false);
                sal_uInt32 a;

                // create center cartesian coordinates to have a possibility to decide if on boundary
                // transitions which value to choose
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
                        // point is a north polar point, no useful X-coordinate can be created.
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
                        // point is a south polar point, no useful X-coordinate can be created. Set
                        // Y-coordinte, though
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

                        // correct cartesinan point coordiante dependent from center value
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
                    // correct X-texture coordinates if polar points are contained. Those
                    // coordinates cannot be correct, so use prev or next X-coordinate
                    for(a = 0L; a < nPointCount; a++)
                    {
                        B2DPoint aTexCoor(aRetval.getTextureCoordinate(a));

                        if(fTools::equalZero(aTexCoor.getY()) || fTools::equal(aTexCoor.getY(), fOne))
                        {
                            // get prev, next TexCoor and test for pole
                            const B2DPoint aPrevTexCoor(aRetval.getTextureCoordinate(a ? a - 1L : nPointCount - 1L));
                            const B2DPoint aNextTexCoor(aRetval.getTextureCoordinate((a + 1L) % nPointCount));
                            const bool bPrevPole(fTools::equalZero(aPrevTexCoor.getY()) || fTools::equal(aPrevTexCoor.getY(), fOne));
                            const bool bNextPole(fTools::equalZero(aNextTexCoor.getY()) || fTools::equal(aNextTexCoor.getY(), fOne));

                            if(!bPrevPole && !bNextPole)
                            {
                                // both no poles, mix them
                                aTexCoor.setX((aPrevTexCoor.getX() + aNextTexCoor.getX()) / 2.0);
                            }
                            else if(!bNextPole)
                            {
                                // copy next
                                aTexCoor.setX(aNextTexCoor.getX());
                            }
                            else
                            {
                                // copy prev, even if it's a pole, hopefully it is already corrected
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
                            // normal points mostly in X-Direction, use YZ-Polygon projection for check
                            // x -> y, y -> z
                            for(sal_uInt32 a(0); a < nPointCount; a++)
                            {
                                const B3DPoint aPreviousPoint(aCurrentPoint);
                                aCurrentPoint = rCandidate.getB3DPoint(a);

                                // cross-over in Z?
                                const bool bCompZA(fTools::more(aPreviousPoint.getZ(), rPoint.getZ()));
                                const bool bCompZB(fTools::more(aCurrentPoint.getZ(), rPoint.getZ()));

                                if(bCompZA != bCompZB)
                                {
                                    // cross-over in Y?
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
                            // normal points mostly in Y-Direction, use XZ-Polygon projection for check
                            // x -> x, y -> z
                            for(sal_uInt32 a(0); a < nPointCount; a++)
                            {
                                const B3DPoint aPreviousPoint(aCurrentPoint);
                                aCurrentPoint = rCandidate.getB3DPoint(a);

                                // cross-over in Z?
                                const bool bCompZA(fTools::more(aPreviousPoint.getZ(), rPoint.getZ()));
                                const bool bCompZB(fTools::more(aCurrentPoint.getZ(), rPoint.getZ()));

                                if(bCompZA != bCompZB)
                                {
                                    // cross-over in X?
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
                            // normal points mostly in Z-Direction, use XY-Polygon projection for check
                            // x -> x, y -> y
                            for(sal_uInt32 a(0); a < nPointCount; a++)
                            {
                                const B3DPoint aPreviousPoint(aCurrentPoint);
                                aCurrentPoint = rCandidate.getB3DPoint(a);

                                // cross-over in Y?
                                const bool bCompYA(fTools::more(aPreviousPoint.getY(), rPoint.getY()));
                                const bool bCompYB(fTools::more(aCurrentPoint.getY(), rPoint.getY()));

                                if(bCompYA != bCompYB)
                                {
                                    // cross-over in X?
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
                // candidate is in epsilon around start or end -> inside
                return bWithPoints;
            }
            else if(rStart.equal(rEnd))
            {
                // start and end are equal, but candidate is outside their epsilon -> outside
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
                            // X is biggest
                            fParamTestOnCurr = aTestVector.getX() / aEdgeVector.getX();
                        }
                        else
                        {
                            // Z is biggest
                            fParamTestOnCurr = aTestVector.getZ() / aEdgeVector.getZ();
                        }
                    }
                    else
                    {
                        if(aEdgeVector.getY() > aEdgeVector.getZ())
                        {
                            // Y is biggest
                            fParamTestOnCurr = aTestVector.getY() / aEdgeVector.getY();
                        }
                        else
                        {
                            // Z is biggest
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

        // snap points of horizontal or vertical edges to discrete values
        B3DPolygon snapPointsOfHorizontalOrVerticalEdges(const B3DPolygon& rCandidate)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1)
            {
                // Start by copying the source polygon to get a writeable copy. The closed state is
                // copied by aRetval's initialisation, too, so no need to copy it in this method
                B3DPolygon aRetval(rCandidate);

                // prepare geometry data. Get rounded from original
                B3ITuple aPrevTuple(basegfx::fround(rCandidate.getB3DPoint(nPointCount - 1)));
                B3DPoint aCurrPoint(rCandidate.getB3DPoint(0));
                B3ITuple aCurrTuple(basegfx::fround(aCurrPoint));

                // loop over all points. This will also snap the implicit closing edge
                // even when not closed, but that's no problem here
                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    // get next point. Get rounded from original
                    const bool bLastRun(a + 1 == nPointCount);
                    const sal_uInt32 nNextIndex(bLastRun ? 0 : a + 1);
                    const B3DPoint aNextPoint(rCandidate.getB3DPoint(nNextIndex));
                    const B3ITuple aNextTuple(basegfx::fround(aNextPoint));

                    // get the states
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

                    // prepare next point
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

    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
