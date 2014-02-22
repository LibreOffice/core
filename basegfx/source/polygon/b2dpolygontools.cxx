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

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <rtl/instance.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dbeziertools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <numeric>
#include <limits>


#define ANGLE_BOUND_START_VALUE     (2.25)
#define ANGLE_BOUND_MINIMUM_VALUE   (0.1)
#define COUNT_SUBDIVIDE_DEFAULT     (4L)
#ifdef DBG_UTIL
static double fAngleBoundStartValue = ANGLE_BOUND_START_VALUE;
#endif
#define STEPSPERQUARTER     (3)



namespace basegfx
{
    namespace tools
    {
        void openWithGeometryChange(B2DPolygon& rCandidate)
        {
            if(rCandidate.isClosed())
            {
                if(rCandidate.count())
                {
                    rCandidate.append(rCandidate.getB2DPoint(0));

                    if(rCandidate.areControlPointsUsed() && rCandidate.isPrevControlPointUsed(0))
                    {
                        rCandidate.setPrevControlPoint(rCandidate.count() - 1, rCandidate.getPrevControlPoint(0));
                        rCandidate.resetPrevControlPoint(0);
                    }
                }

                rCandidate.setClosed(false);
            }
        }

        void closeWithGeometryChange(B2DPolygon& rCandidate)
        {
            if(!rCandidate.isClosed())
            {
                while(rCandidate.count() > 1 && rCandidate.getB2DPoint(0) == rCandidate.getB2DPoint(rCandidate.count() - 1))
                {
                    if(rCandidate.areControlPointsUsed() && rCandidate.isPrevControlPointUsed(rCandidate.count() - 1))
                    {
                        rCandidate.setPrevControlPoint(0, rCandidate.getPrevControlPoint(rCandidate.count() - 1));
                    }

                    rCandidate.remove(rCandidate.count() - 1);
                }

                rCandidate.setClosed(true);
            }
        }

        void checkClosed(B2DPolygon& rCandidate)
        {
            
            

            if(rCandidate.count() > 1 && rCandidate.getB2DPoint(0) == rCandidate.getB2DPoint(rCandidate.count() - 1))
            {
                closeWithGeometryChange(rCandidate);
            }
        }

        
        
        sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(nIndex)
            {
                return nIndex - 1L;
            }
            else if(rCandidate.count())
            {
                return rCandidate.count() - 1L;
            }
            else
            {
                return nIndex;
            }
        }

        sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(nIndex + 1L < rCandidate.count())
            {
                return nIndex + 1L;
            }
            else if(nIndex + 1L == rCandidate.count())
            {
                return 0L;
            }
            else
            {
                return nIndex;
            }
        }

        B2VectorOrientation getOrientation(const B2DPolygon& rCandidate)
        {
            B2VectorOrientation eRetval(ORIENTATION_NEUTRAL);

            if(rCandidate.count() > 2L || rCandidate.areControlPointsUsed())
            {
                const double fSignedArea(getSignedArea(rCandidate));

                if(fTools::equalZero(fSignedArea))
                {
                    
                }
                if(fSignedArea > 0.0)
                {
                    eRetval = ORIENTATION_POSITIVE;
                }
                else if(fSignedArea < 0.0)
                {
                    eRetval = ORIENTATION_NEGATIVE;
                }
            }

            return eRetval;
        }

        B2VectorContinuity getContinuityInPoint(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            return rCandidate.getContinuityInPoint(nIndex);
        }

        B2DPolygon adaptiveSubdivideByDistance(const B2DPolygon& rCandidate, double fDistanceBound)
        {
            if(rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPointCount(rCandidate.count());
                B2DPolygon aRetval;

                if(nPointCount)
                {
                    
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                    B2DCubicBezier aBezier;
                    aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                    
                    aRetval.reserve(nPointCount*4);

                    
                    aRetval.append(aBezier.getStartPoint());

                    for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                    {
                        
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        aBezier.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                        aBezier.setControlPointA(rCandidate.getNextControlPoint(a));
                        aBezier.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                        aBezier.testAndSolveTrivialBezier();

                        if(aBezier.isBezier())
                        {
                            
                            double fBound(0.0);

                            if(0.0 == fDistanceBound)
                            {
                                
                                const double fRoughLength((aBezier.getEdgeLength() + aBezier.getControlPolygonLength()) / 2.0);

                                
                                fBound = fRoughLength * 0.01;
                            }
                            else
                            {
                                
                                fBound = fDistanceBound;
                            }

                            
                            
                            if(fBound < 0.01)
                            {
                                fBound = 0.01;
                            }

                            
                            aBezier.adaptiveSubdivideByDistance(aRetval, fBound);
                        }
                        else
                        {
                            
                            aRetval.append(aBezier.getEndPoint());
                        }

                        
                        aBezier.setStartPoint(aBezier.getEndPoint());
                    }

                    if(rCandidate.isClosed())
                    {
                        
                        closeWithGeometryChange(aRetval);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolygon adaptiveSubdivideByAngle(const B2DPolygon& rCandidate, double fAngleBound)
        {
            if(rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPointCount(rCandidate.count());
                B2DPolygon aRetval;

                if(nPointCount)
                {
                    
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                    B2DCubicBezier aBezier;
                    aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                    
                    aRetval.reserve(nPointCount*4);

                    
                    aRetval.append(aBezier.getStartPoint());

                    
                    if(0.0 == fAngleBound)
                    {
#ifdef DBG_UTIL
                        fAngleBound = fAngleBoundStartValue;
#else
                        fAngleBound = ANGLE_BOUND_START_VALUE;
#endif
                    }
                    else if(fTools::less(fAngleBound, ANGLE_BOUND_MINIMUM_VALUE))
                    {
                        fAngleBound = 0.1;
                    }

                    for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                    {
                        
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        aBezier.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                        aBezier.setControlPointA(rCandidate.getNextControlPoint(a));
                        aBezier.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                        aBezier.testAndSolveTrivialBezier();

                        if(aBezier.isBezier())
                        {
                            
                            aBezier.adaptiveSubdivideByAngle(aRetval, fAngleBound, true);
                        }
                        else
                        {
                            
                            aRetval.append(aBezier.getEndPoint());
                        }

                        
                        aBezier.setStartPoint(aBezier.getEndPoint());
                    }

                    if(rCandidate.isClosed())
                    {
                        
                        closeWithGeometryChange(aRetval);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolygon adaptiveSubdivideByCount(const B2DPolygon& rCandidate, sal_uInt32 nCount)
        {
            if(rCandidate.areControlPointsUsed())
            {
                const sal_uInt32 nPointCount(rCandidate.count());
                B2DPolygon aRetval;

                if(nPointCount)
                {
                    
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                    B2DCubicBezier aBezier;
                    aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                    
                    aRetval.reserve(nPointCount*4);

                    
                    aRetval.append(aBezier.getStartPoint());

                    
                    if(0L == nCount)
                    {
                        nCount = COUNT_SUBDIVIDE_DEFAULT;
                    }

                    for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                    {
                        
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        aBezier.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                        aBezier.setControlPointA(rCandidate.getNextControlPoint(a));
                        aBezier.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                        aBezier.testAndSolveTrivialBezier();

                        if(aBezier.isBezier())
                        {
                            
                            aBezier.adaptiveSubdivideByCount(aRetval, nCount);
                        }
                        else
                        {
                            
                            aRetval.append(aBezier.getEndPoint());
                        }

                        
                        aBezier.setStartPoint(aBezier.getEndPoint());
                    }

                    if(rCandidate.isClosed())
                    {
                        
                        closeWithGeometryChange(aRetval);
                    }
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        bool isInside(const B2DPolygon& rCandidate, const B2DPoint& rPoint, bool bWithBorder)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? rCandidate.getDefaultAdaptiveSubdivision() : rCandidate);

            if(bWithBorder && isPointOnPolygon(aCandidate, rPoint, true))
            {
                return true;
            }
            else
            {
                bool bRetval(false);
                const sal_uInt32 nPointCount(aCandidate.count());

                if(nPointCount)
                {
                    B2DPoint aCurrentPoint(aCandidate.getB2DPoint(nPointCount - 1L));

                    for(sal_uInt32 a(0L); a < nPointCount; a++)
                    {
                        const B2DPoint aPreviousPoint(aCurrentPoint);
                        aCurrentPoint = aCandidate.getB2DPoint(a);

                        
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

                return bRetval;
            }
        }

        bool isInside(const B2DPolygon& rCandidate, const B2DPolygon& rPolygon, bool bWithBorder)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? rCandidate.getDefaultAdaptiveSubdivision() : rCandidate);
            const B2DPolygon aPolygon(rPolygon.areControlPointsUsed() ? rPolygon.getDefaultAdaptiveSubdivision() : rPolygon);
            const sal_uInt32 nPointCount(aPolygon.count());

            for(sal_uInt32 a(0L); a < nPointCount; a++)
            {
                const B2DPoint aTestPoint(aPolygon.getB2DPoint(a));

                if(!isInside(aCandidate, aTestPoint, bWithBorder))
                {
                    return false;
                }
            }

            return true;
        }

        B2DRange getRange(const B2DPolygon& rCandidate)
        {
            
            return rCandidate.getB2DRange();
        }

        double getSignedArea(const B2DPolygon& rCandidate)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? rCandidate.getDefaultAdaptiveSubdivision() : rCandidate);
            double fRetval(0.0);
            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount > 2)
            {
                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aPreviousPoint(aCandidate.getB2DPoint((!a) ? nPointCount - 1L : a - 1L));
                    const B2DPoint aCurrentPoint(aCandidate.getB2DPoint(a));

                    fRetval += aPreviousPoint.getX() * aCurrentPoint.getY();
                    fRetval -= aPreviousPoint.getY() * aCurrentPoint.getX();
                }

                
                
                
                if(fTools::equalZero(fRetval) || fTools::equalZero(fRetval * fRetval))
                {
                    fRetval = 0.0;
                }
            }

            return fRetval;
        }

        double getArea(const B2DPolygon& rCandidate)
        {
            double fRetval(0.0);

            if(rCandidate.count() > 2 || rCandidate.areControlPointsUsed())
            {
                fRetval = getSignedArea(rCandidate);
                const double fZero(0.0);

                if(fTools::less(fRetval, fZero))
                {
                    fRetval = -fRetval;
                }
            }

            return fRetval;
        }

        double getEdgeLength(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            const sal_uInt32 nPointCount(rCandidate.count());
            OSL_ENSURE(nIndex < nPointCount, "getEdgeLength: Access to polygon out of range (!)");
            double fRetval(0.0);

            if(nPointCount)
            {
                const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);

                if(rCandidate.areControlPointsUsed())
                {
                    B2DCubicBezier aEdge;

                    aEdge.setStartPoint(rCandidate.getB2DPoint(nIndex));
                    aEdge.setControlPointA(rCandidate.getNextControlPoint(nIndex));
                    aEdge.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                    aEdge.setEndPoint(rCandidate.getB2DPoint(nNextIndex));

                    fRetval = aEdge.getLength();
                }
                else
                {
                    const B2DPoint aCurrent(rCandidate.getB2DPoint(nIndex));
                    const B2DPoint aNext(rCandidate.getB2DPoint(nNextIndex));

                    fRetval = B2DVector(aNext - aCurrent).getLength();
                }
            }

            return fRetval;
        }

        double getLength(const B2DPolygon& rCandidate)
        {
            double fRetval(0.0);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);

                if(rCandidate.areControlPointsUsed())
                {
                    B2DCubicBezier aEdge;
                    aEdge.setStartPoint(rCandidate.getB2DPoint(0));

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        aEdge.setControlPointA(rCandidate.getNextControlPoint(a));
                        aEdge.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                        aEdge.setEndPoint(rCandidate.getB2DPoint(nNextIndex));

                        fRetval += aEdge.getLength();
                        aEdge.setStartPoint(aEdge.getEndPoint());
                    }
                }
                else
                {
                    B2DPoint aCurrent(rCandidate.getB2DPoint(0));

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        const B2DPoint aNext(rCandidate.getB2DPoint(nNextIndex));

                        fRetval += B2DVector(aNext - aCurrent).getLength();
                        aCurrent = aNext;
                    }
                }
            }

            return fRetval;
        }

        B2DPoint getPositionAbsolute(const B2DPolygon& rCandidate, double fDistance, double fLength)
        {
            B2DPoint aRetval;
            const sal_uInt32 nPointCount(rCandidate.count());

            if( 1L == nPointCount )
            {
                
                aRetval = rCandidate.getB2DPoint(0);
            }
            else if(nPointCount > 1L)
            {
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                sal_uInt32 nIndex(0L);
                bool bIndexDone(false);

                
                if(fTools::equalZero(fLength))
                {
                    fLength = getLength(rCandidate);
                }

                if(fTools::less(fDistance, 0.0))
                {
                    
                    if(rCandidate.isClosed())
                    {
                        
                        sal_uInt32 nCount(sal_uInt32(-fDistance / fLength));
                        fDistance += double(nCount + 1L) * fLength;
                    }
                    else
                    {
                        
                        fDistance = 0.0;
                        bIndexDone = true;
                    }
                }
                else if(fTools::moreOrEqual(fDistance, fLength))
                {
                    
                    if(rCandidate.isClosed())
                    {
                        
                        sal_uInt32 nCount(sal_uInt32(fDistance / fLength));
                        fDistance -= (double)(nCount) * fLength;
                    }
                    else
                    {
                        
                        fDistance = 0.0;
                        nIndex = nEdgeCount;
                        bIndexDone = true;
                    }
                }

                
                double fEdgeLength(getEdgeLength(rCandidate, nIndex));

                while(!bIndexDone)
                {
                    
                    
                    
                    
                    
                    
                    
                    if(nIndex < nEdgeCount && fTools::moreOrEqual(fDistance, fEdgeLength))
                    {
                        
                        fDistance -= fEdgeLength;
                        fEdgeLength = getEdgeLength(rCandidate, ++nIndex);
                    }
                    else
                    {
                        
                        bIndexDone = true;
                    }
                }

                
                aRetval = rCandidate.getB2DPoint(nIndex);

                
                
                if(!fTools::equalZero(fDistance))
                {
                    if(fTools::moreOrEqual(fDistance, fEdgeLength))
                    {
                        
                        const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                        aRetval = rCandidate.getB2DPoint(nNextIndex);
                    }
                    else if(fTools::equalZero(fDistance))
                    {
                        
                        aRetval = aRetval;
                    }
                    else
                    {
                        
                        const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                        const B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                        bool bDone(false);

                        
                        if(rCandidate.areControlPointsUsed())
                        {
                            
                            const B2DCubicBezier aBezierSegment(
                                aRetval, rCandidate.getNextControlPoint(nIndex),
                                rCandidate.getPrevControlPoint(nNextIndex), aNextPoint);

                            if(aBezierSegment.isBezier())
                            {
                                
                                
                                const B2DCubicBezierHelper aBezierSegmentHelper(aBezierSegment);
                                const double fBezierDistance(aBezierSegmentHelper.distanceToRelative(fDistance));

                                aRetval = aBezierSegment.interpolatePoint(fBezierDistance);
                                bDone = true;
                            }
                        }

                        if(!bDone)
                        {
                            const double fRelativeInEdge(fDistance / fEdgeLength);
                            aRetval = interpolate(aRetval, aNextPoint, fRelativeInEdge);
                        }
                    }
                }
            }

            return aRetval;
        }

        B2DPoint getPositionRelative(const B2DPolygon& rCandidate, double fDistance, double fLength)
        {
            
            if(fTools::equalZero(fLength))
            {
                fLength = getLength(rCandidate);
            }

            
            
            return getPositionAbsolute(rCandidate, fDistance * fLength, fLength);
        }

        B2DPolygon getSnippetAbsolute(const B2DPolygon& rCandidate, double fFrom, double fTo, double fLength)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                
                if(fTools::equalZero(fLength))
                {
                    fLength = getLength(rCandidate);
                }

                
                if(fTools::less(fFrom, 0.0))
                {
                    fFrom = 0.0;
                }

                
                if(fTools::more(fTo, fLength))
                {
                    fTo = fLength;
                }

                
                if(fTools::more(fFrom, fTo))
                {
                    fFrom = fTo = (fFrom + fTo) / 2.0;
                }

                if(fTools::equalZero(fFrom) && fTools::equal(fTo, fLength))
                {
                    
                    return rCandidate;
                }
                else
                {
                    B2DPolygon aRetval;
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                    double fPositionOfStart(0.0);
                    bool bStartDone(false);
                    bool bEndDone(false);

                    for(sal_uInt32 a(0L); !(bStartDone && bEndDone) && a < nEdgeCount; a++)
                    {
                        const double fEdgeLength(getEdgeLength(rCandidate, a));

                        if(!bStartDone)
                        {
                            if(fTools::equalZero(fFrom))
                            {
                                aRetval.append(rCandidate.getB2DPoint(a));

                                if(rCandidate.areControlPointsUsed())
                                {
                                    aRetval.setNextControlPoint(aRetval.count() - 1, rCandidate.getNextControlPoint(a));
                                }

                                bStartDone = true;
                            }
                            else if(fTools::moreOrEqual(fFrom, fPositionOfStart) && fTools::less(fFrom, fPositionOfStart + fEdgeLength))
                            {
                                
                                if(fTools::equalZero(fEdgeLength))
                                {
                                    aRetval.append(rCandidate.getB2DPoint(a));

                                    if(rCandidate.areControlPointsUsed())
                                    {
                                        aRetval.setNextControlPoint(aRetval.count() - 1, rCandidate.getNextControlPoint(a));
                                    }
                                }
                                else
                                {
                                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                                    const B2DPoint aStart(rCandidate.getB2DPoint(a));
                                    const B2DPoint aEnd(rCandidate.getB2DPoint(nNextIndex));
                                    bool bDone(false);

                                    if(rCandidate.areControlPointsUsed())
                                    {
                                        const B2DCubicBezier aBezierSegment(
                                            aStart, rCandidate.getNextControlPoint(a),
                                            rCandidate.getPrevControlPoint(nNextIndex), aEnd);

                                        if(aBezierSegment.isBezier())
                                        {
                                            
                                            
                                            const B2DCubicBezierHelper aBezierSegmentHelper(aBezierSegment);
                                            const double fBezierDistance(aBezierSegmentHelper.distanceToRelative(fFrom - fPositionOfStart));
                                            B2DCubicBezier aRight;

                                            aBezierSegment.split(fBezierDistance, 0, &aRight);
                                            aRetval.append(aRight.getStartPoint());
                                            aRetval.setNextControlPoint(aRetval.count() - 1, aRight.getControlPointA());
                                            bDone = true;
                                        }
                                    }

                                    if(!bDone)
                                    {
                                        const double fRelValue((fFrom - fPositionOfStart) / fEdgeLength);
                                        aRetval.append(interpolate(aStart, aEnd, fRelValue));
                                    }
                                }

                                bStartDone = true;

                                
                                if(fFrom == fTo)
                                {
                                    bEndDone = true;
                                }
                            }
                        }

                        if(!bEndDone && fTools::moreOrEqual(fTo, fPositionOfStart) && fTools::less(fTo, fPositionOfStart + fEdgeLength))
                        {
                            
                            if(fTools::equalZero(fEdgeLength))
                            {
                                const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                                aRetval.append(rCandidate.getB2DPoint(nNextIndex));

                                if(rCandidate.areControlPointsUsed())
                                {
                                    aRetval.setPrevControlPoint(aRetval.count() - 1, rCandidate.getPrevControlPoint(nNextIndex));
                                }
                            }
                            else
                            {
                                const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                                const B2DPoint aStart(rCandidate.getB2DPoint(a));
                                const B2DPoint aEnd(rCandidate.getB2DPoint(nNextIndex));
                                bool bDone(false);

                                if(rCandidate.areControlPointsUsed())
                                {
                                    const B2DCubicBezier aBezierSegment(
                                        aStart, rCandidate.getNextControlPoint(a),
                                        rCandidate.getPrevControlPoint(nNextIndex), aEnd);

                                    if(aBezierSegment.isBezier())
                                    {
                                        
                                        
                                        const B2DCubicBezierHelper aBezierSegmentHelper(aBezierSegment);
                                        const double fBezierDistance(aBezierSegmentHelper.distanceToRelative(fTo - fPositionOfStart));
                                        B2DCubicBezier aLeft;

                                        aBezierSegment.split(fBezierDistance, &aLeft, 0);
                                        aRetval.append(aLeft.getEndPoint());
                                        aRetval.setPrevControlPoint(aRetval.count() - 1, aLeft.getControlPointB());
                                        bDone = true;
                                    }
                                }

                                if(!bDone)
                                {
                                    const double fRelValue((fTo - fPositionOfStart) / fEdgeLength);
                                    aRetval.append(interpolate(aStart, aEnd, fRelValue));
                                }
                            }

                            bEndDone = true;
                        }

                        if(!bEndDone)
                        {
                            if(bStartDone)
                            {
                                
                                const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                                aRetval.append(rCandidate.getB2DPoint(nNextIndex));

                                if(rCandidate.areControlPointsUsed())
                                {
                                    aRetval.setPrevControlPoint(aRetval.count() - 1, rCandidate.getPrevControlPoint(nNextIndex));
                                    aRetval.setNextControlPoint(aRetval.count() - 1, rCandidate.getNextControlPoint(nNextIndex));
                                }
                            }

                            
                            fPositionOfStart += fEdgeLength;
                        }
                    }
                    return aRetval;
                }
            }
            else
            {
                return rCandidate;
            }
        }

        CutFlagValue findCut(
            const B2DPoint& rEdge1Start, const B2DVector& rEdge1Delta,
            const B2DPoint& rEdge2Start, const B2DVector& rEdge2Delta,
            CutFlagValue aCutFlags,
            double* pCut1, double* pCut2)
        {
            CutFlagValue aRetval(CUTFLAG_NONE);
            double fCut1(0.0);
            double fCut2(0.0);
            bool bFinished(!((bool)(aCutFlags & CUTFLAG_ALL)));

            
            if(!bFinished
                && (aCutFlags & (CUTFLAG_START1|CUTFLAG_END1))
                && (aCutFlags & (CUTFLAG_START2|CUTFLAG_END2)))
            {
                
                if(!bFinished && (aCutFlags & (CUTFLAG_START1|CUTFLAG_START2)) == (CUTFLAG_START1|CUTFLAG_START2))
                {
                    if(rEdge1Start.equal(rEdge2Start))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_START1|CUTFLAG_START2);
                    }
                }

                
                if(!bFinished && (aCutFlags & (CUTFLAG_END1|CUTFLAG_END2)) == (CUTFLAG_END1|CUTFLAG_END2))
                {
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(aEnd1.equal(aEnd2))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_END1|CUTFLAG_END2);
                        fCut1 = fCut2 = 1.0;
                    }
                }

                
                if(!bFinished && (aCutFlags & (CUTFLAG_START1|CUTFLAG_END2)) == (CUTFLAG_START1|CUTFLAG_END2))
                {
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(rEdge1Start.equal(aEnd2))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_START1|CUTFLAG_END2);
                        fCut1 = 0.0;
                        fCut2 = 1.0;
                    }
                }

                
                if(!bFinished&& (aCutFlags & (CUTFLAG_START2|CUTFLAG_END1)) == (CUTFLAG_START2|CUTFLAG_END1))
                {
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);

                    if(rEdge2Start.equal(aEnd1))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_START2|CUTFLAG_END1);
                        fCut1 = 1.0;
                        fCut2 = 0.0;
                    }
                }
            }

            if(!bFinished && (aCutFlags & CUTFLAG_LINE))
            {
                if(!bFinished && (aCutFlags & CUTFLAG_START1))
                {
                    
                    if(isPointOnEdge(rEdge1Start, rEdge2Start, rEdge2Delta, &fCut2))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_LINE|CUTFLAG_START1);
                    }
                }

                if(!bFinished && (aCutFlags & CUTFLAG_START2))
                {
                    
                    if(isPointOnEdge(rEdge2Start, rEdge1Start, rEdge1Delta, &fCut1))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_LINE|CUTFLAG_START2);
                    }
                }

                if(!bFinished && (aCutFlags & CUTFLAG_END1))
                {
                    
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);

                    if(isPointOnEdge(aEnd1, rEdge2Start, rEdge2Delta, &fCut2))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_LINE|CUTFLAG_END1);
                    }
                }

                if(!bFinished && (aCutFlags & CUTFLAG_END2))
                {
                    
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(isPointOnEdge(aEnd2, rEdge1Start, rEdge1Delta, &fCut1))
                    {
                        bFinished = true;
                        aRetval = (CUTFLAG_LINE|CUTFLAG_END2);
                    }
                }

                if(!bFinished)
                {
                    
                    fCut1 = (rEdge1Delta.getX() * rEdge2Delta.getY()) - (rEdge1Delta.getY() * rEdge2Delta.getX());

                    if(!fTools::equalZero(fCut1))
                    {
                        fCut1 = (rEdge2Delta.getY() * (rEdge2Start.getX() - rEdge1Start.getX())
                            + rEdge2Delta.getX() * (rEdge1Start.getY() - rEdge2Start.getY())) / fCut1;

                        const double fZero(0.0);
                        const double fOne(1.0);

                        
                        if(fTools::more(fCut1, fZero) && fTools::less(fCut1, fOne)
                            && (!fTools::equalZero(rEdge2Delta.getX()) || !fTools::equalZero(rEdge2Delta.getY())))
                        {
                            
                            if(fabs(rEdge2Delta.getX()) > fabs(rEdge2Delta.getY()))
                            {
                                fCut2 = (rEdge1Start.getX() + fCut1
                                    * rEdge1Delta.getX() - rEdge2Start.getX()) / rEdge2Delta.getX();
                            }
                            else
                            {
                                fCut2 = (rEdge1Start.getY() + fCut1
                                    * rEdge1Delta.getY() - rEdge2Start.getY()) / rEdge2Delta.getY();
                            }

                            
                            if(fTools::more(fCut2, fZero) && fTools::less(fCut2, fOne))
                            {
                                bFinished = true;
                                aRetval = CUTFLAG_LINE;
                            }
                        }
                    }
                }
            }

            
            if(pCut1)
            {
                *pCut1 = fCut1;
            }

            if(pCut2)
            {
                *pCut2 = fCut2;
            }

            return aRetval;
        }

        bool isPointOnEdge(
            const B2DPoint& rPoint,
            const B2DPoint& rEdgeStart,
            const B2DVector& rEdgeDelta,
            double* pCut)
        {
            bool bDeltaXIsZero(fTools::equalZero(rEdgeDelta.getX()));
            bool bDeltaYIsZero(fTools::equalZero(rEdgeDelta.getY()));
            const double fZero(0.0);
            const double fOne(1.0);

            if(bDeltaXIsZero && bDeltaYIsZero)
            {
                
                return false;
            }
            else if(bDeltaXIsZero)
            {
                
                if(fTools::equal(rPoint.getX(), rEdgeStart.getX()))
                {
                    double fValue = (rPoint.getY() - rEdgeStart.getY()) / rEdgeDelta.getY();

                    if(fTools::more(fValue, fZero) && fTools::less(fValue, fOne))
                    {
                        if(pCut)
                        {
                            *pCut = fValue;
                        }

                        return true;
                    }
                }
            }
            else if(bDeltaYIsZero)
            {
                
                if(fTools::equal(rPoint.getY(), rEdgeStart.getY()))
                {
                    double fValue = (rPoint.getX() - rEdgeStart.getX()) / rEdgeDelta.getX();

                    if(fTools::more(fValue, fZero) && fTools::less(fValue, fOne))
                    {
                        if(pCut)
                        {
                            *pCut = fValue;
                        }

                        return true;
                    }
                }
            }
            else
            {
                
                double fTOne = (rPoint.getX() - rEdgeStart.getX()) / rEdgeDelta.getX();
                double fTTwo = (rPoint.getY() - rEdgeStart.getY()) / rEdgeDelta.getY();

                if(fTools::equal(fTOne, fTTwo))
                {
                    
                    
                    double fValue = (fTOne + fTTwo) / 2.0;

                    if(fTools::more(fValue, fZero) && fTools::less(fValue, fOne))
                    {
                        
                        if(pCut)
                        {
                            *pCut = fValue;
                        }

                        return true;
                    }
                }
            }

            return false;
        }

        void applyLineDashing(const B2DPolygon& rCandidate, const ::std::vector<double>& rDotDashArray, B2DPolyPolygon* pLineTarget, B2DPolyPolygon* pGapTarget, double fDotDashLength)
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

                
                B2DCubicBezier aCurrentEdge;
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                aCurrentEdge.setStartPoint(rCandidate.getB2DPoint(0));

                
                sal_uInt32 nDotDashIndex(0);
                bool bIsLine(true);
                double fDotDashMovingLength(rDotDashArray[0]);
                B2DPolygon aSnippet;

                
                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    
                    double fLastDotDashMovingLength(0.0);
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    aCurrentEdge.setControlPointA(rCandidate.getNextControlPoint(a));
                    aCurrentEdge.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                    aCurrentEdge.setEndPoint(rCandidate.getB2DPoint(nNextIndex));

                    
                    aCurrentEdge.testAndSolveTrivialBezier();

                    if(aCurrentEdge.isBezier())
                    {
                        
                        const B2DCubicBezierHelper aCubicBezierHelper(aCurrentEdge);
                        const double fEdgeLength(aCubicBezierHelper.getLength());

                        if(!fTools::equalZero(fEdgeLength))
                        {
                            while(fTools::less(fDotDashMovingLength, fEdgeLength))
                            {
                                
                                const bool bHandleLine(bIsLine && pLineTarget);
                                const bool bHandleGap(!bIsLine && pGapTarget);

                                if(bHandleLine || bHandleGap)
                                {
                                    const double fBezierSplitStart(aCubicBezierHelper.distanceToRelative(fLastDotDashMovingLength));
                                    const double fBezierSplitEnd(aCubicBezierHelper.distanceToRelative(fDotDashMovingLength));
                                    B2DCubicBezier aBezierSnippet(aCurrentEdge.snippet(fBezierSplitStart, fBezierSplitEnd));

                                    if(!aSnippet.count())
                                    {
                                        aSnippet.append(aBezierSnippet.getStartPoint());
                                    }

                                    aSnippet.appendBezierSegment(aBezierSnippet.getControlPointA(), aBezierSnippet.getControlPointB(), aBezierSnippet.getEndPoint());

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
                                B2DCubicBezier aRight;
                                const double fBezierSplit(aCubicBezierHelper.distanceToRelative(fLastDotDashMovingLength));

                                aCurrentEdge.split(fBezierSplit, 0, &aRight);

                                if(!aSnippet.count())
                                {
                                    aSnippet.append(aRight.getStartPoint());
                                }

                                aSnippet.appendBezierSegment(aRight.getControlPointA(), aRight.getControlPointB(), aRight.getEndPoint());
                            }

                            
                            fDotDashMovingLength -= fEdgeLength;
                        }
                    }
                    else
                    {
                        
                        const double fEdgeLength(aCurrentEdge.getEdgeLength());

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
                                        aSnippet.append(interpolate(aCurrentEdge.getStartPoint(), aCurrentEdge.getEndPoint(), fLastDotDashMovingLength / fEdgeLength));
                                    }

                                    aSnippet.append(interpolate(aCurrentEdge.getStartPoint(), aCurrentEdge.getEndPoint(), fDotDashMovingLength / fEdgeLength));

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
                                    aSnippet.append(interpolate(aCurrentEdge.getStartPoint(), aCurrentEdge.getEndPoint(), fLastDotDashMovingLength / fEdgeLength));
                                }

                                aSnippet.append(aCurrentEdge.getEndPoint());
                            }

                            
                            fDotDashMovingLength -= fEdgeLength;
                        }
                    }

                    
                    aCurrentEdge.setStartPoint(aCurrentEdge.getEndPoint());
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
                        
                        
                        const B2DPolygon aFirst(pLineTarget->getB2DPolygon(0));
                        B2DPolygon aLast(pLineTarget->getB2DPolygon(nCount - 1));

                        if(aFirst.getB2DPoint(0).equal(aLast.getB2DPoint(aLast.count() - 1)))
                        {
                            
                            aLast.append(aFirst);
                            aLast.removeDoublePoints();
                            pLineTarget->setB2DPolygon(0, aLast);
                            pLineTarget->remove(nCount - 1);
                        }
                    }
                }

                if(pGapTarget)
                {
                    const sal_uInt32 nCount(pGapTarget->count());

                    if(nCount > 1)
                    {
                        
                        
                        const B2DPolygon aFirst(pGapTarget->getB2DPolygon(0));
                        B2DPolygon aLast(pGapTarget->getB2DPolygon(nCount - 1));

                        if(aFirst.getB2DPoint(0).equal(aLast.getB2DPoint(aLast.count() - 1)))
                        {
                            
                            aLast.append(aFirst);
                            aLast.removeDoublePoints();
                            pGapTarget->setB2DPolygon(0, aLast);
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

        
        
        
        
        bool isInEpsilonRange(const B2DPoint& rEdgeStart, const B2DPoint& rEdgeEnd, const B2DPoint& rTestPosition, double fDistance)
        {
            
            const B2DVector aEdge(rEdgeEnd - rEdgeStart);
            bool bDoDistanceTestStart(false);
            bool bDoDistanceTestEnd(false);

            if(aEdge.equalZero())
            {
                
                bDoDistanceTestStart = true;
            }
            else
            {
                
                const B2DVector aPerpend(getPerpendicular(aEdge));
                double fCut(
                    (aPerpend.getY() * (rTestPosition.getX() - rEdgeStart.getX())
                    + aPerpend.getX() * (rEdgeStart.getY() - rTestPosition.getY())) /
                    (aEdge.getX() * aEdge.getX() + aEdge.getY() * aEdge.getY()));
                const double fZero(0.0);
                const double fOne(1.0);

                if(fTools::less(fCut, fZero))
                {
                    
                    bDoDistanceTestStart = true;
                }
                else if(fTools::more(fCut, fOne))
                {
                    
                    bDoDistanceTestEnd = true;
                }
                else
                {
                    
                    const B2DPoint aCutPoint(interpolate(rEdgeStart, rEdgeEnd, fCut));
                    const B2DVector aDelta(rTestPosition - aCutPoint);
                    const double fDistanceSquare(aDelta.scalar(aDelta));

                    if(fDistanceSquare <= fDistance * fDistance)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            if(bDoDistanceTestStart)
            {
                const B2DVector aDelta(rTestPosition - rEdgeStart);
                const double fDistanceSquare(aDelta.scalar(aDelta));

                if(fDistanceSquare <= fDistance * fDistance)
                {
                    return true;
                }
            }
            else if(bDoDistanceTestEnd)
            {
                const B2DVector aDelta(rTestPosition - rEdgeEnd);
                const double fDistanceSquare(aDelta.scalar(aDelta));

                if(fDistanceSquare <= fDistance * fDistance)
                {
                    return true;
                }
            }

            return false;
        }

        
        
        
        bool isInEpsilonRange(const B2DPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance)
        {
            
            const B2DPolygon aCandidate(rCandidate.getDefaultAdaptiveSubdivision());
            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                B2DPoint aCurrent(aCandidate.getB2DPoint(0));

                if(nEdgeCount)
                {
                    
                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        const B2DPoint aNext(aCandidate.getB2DPoint(nNextIndex));

                        if(isInEpsilonRange(aCurrent, aNext, rTestPosition, fDistance))
                        {
                            return true;
                        }

                        
                        aCurrent = aNext;
                    }
                }
                else
                {
                    
                    
                    if(isInEpsilonRange(aCurrent, aCurrent, rTestPosition, fDistance))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect, double fRadiusX, double fRadiusY )
        {
            const double fZero(0.0);
            const double fOne(1.0);

            
            if(fTools::less(fRadiusX, fZero))
            {
                fRadiusX = fZero;
            }
            else if(fTools::more(fRadiusX, fOne))
            {
                fRadiusX = fOne;
            }

            if(fTools::less(fRadiusY, fZero))
            {
                fRadiusY = fZero;
            }
            else if(fTools::more(fRadiusY, fOne))
            {
                fRadiusY = fOne;
            }

            if(fZero == fRadiusX || fZero == fRadiusY)
            {
                B2DPolygon aRetval;

                
                
                
                
                
                const B2DPoint aBottomCenter(rRect.getCenter().getX(), rRect.getMaxY());
                aRetval.append(aBottomCenter);

                aRetval.append( B2DPoint( rRect.getMinX(), rRect.getMaxY() ) );
                aRetval.append( B2DPoint( rRect.getMinX(), rRect.getMinY() ) );
                aRetval.append( B2DPoint( rRect.getMaxX(), rRect.getMinY() ) );
                aRetval.append( B2DPoint( rRect.getMaxX(), rRect.getMaxY() ) );

                
                aRetval.setClosed( true );

                return aRetval;
            }
            else if(fOne == fRadiusX && fOne == fRadiusY)
            {
                
                const B2DPoint aCenter(rRect.getCenter());
                const double fRectRadiusX(rRect.getWidth() / 2.0);
                const double fRectRadiusY(rRect.getHeight() / 2.0);

                return createPolygonFromEllipse( aCenter, fRectRadiusX, fRectRadiusY );
            }
            else
            {
                B2DPolygon aRetval;
                const double fBowX((rRect.getWidth() / 2.0) * fRadiusX);
                const double fBowY((rRect.getHeight() / 2.0) * fRadiusY);
                const double fKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);

                
                if(fOne != fRadiusX)
                {
                    const B2DPoint aBottomCenter(rRect.getCenter().getX(), rRect.getMaxY());
                    aRetval.append(aBottomCenter);
                }

                
                {
                    const B2DPoint aBottomRight(rRect.getMaxX(), rRect.getMaxY());
                    const B2DPoint aStart(aBottomRight + B2DPoint(-fBowX, 0.0));
                    const B2DPoint aStop(aBottomRight + B2DPoint(0.0, -fBowY));
                    aRetval.append(aStart);
                    aRetval.appendBezierSegment(interpolate(aStart, aBottomRight, fKappa), interpolate(aStop, aBottomRight, fKappa), aStop);
                }

                
                {
                    const B2DPoint aTopRight(rRect.getMaxX(), rRect.getMinY());
                    const B2DPoint aStart(aTopRight + B2DPoint(0.0, fBowY));
                    const B2DPoint aStop(aTopRight + B2DPoint(-fBowX, 0.0));
                    aRetval.append(aStart);
                    aRetval.appendBezierSegment(interpolate(aStart, aTopRight, fKappa), interpolate(aStop, aTopRight, fKappa), aStop);
                }

                
                {
                    const B2DPoint aTopLeft(rRect.getMinX(), rRect.getMinY());
                    const B2DPoint aStart(aTopLeft + B2DPoint(fBowX, 0.0));
                    const B2DPoint aStop(aTopLeft + B2DPoint(0.0, fBowY));
                    aRetval.append(aStart);
                    aRetval.appendBezierSegment(interpolate(aStart, aTopLeft, fKappa), interpolate(aStop, aTopLeft, fKappa), aStop);
                }

                
                {
                    const B2DPoint aBottomLeft(rRect.getMinX(), rRect.getMaxY());
                    const B2DPoint aStart(aBottomLeft + B2DPoint(0.0, -fBowY));
                    const B2DPoint aStop(aBottomLeft + B2DPoint(fBowX, 0.0));
                    aRetval.append(aStart);
                    aRetval.appendBezierSegment(interpolate(aStart, aBottomLeft, fKappa), interpolate(aStop, aBottomLeft, fKappa), aStop);
                }

                
                aRetval.setClosed( true );

                
                if(fOne == fRadiusX || fOne == fRadiusY)
                {
                    aRetval.removeDoublePoints();
                }

                return aRetval;
            }
        }

        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect )
        {
            B2DPolygon aRetval;

            aRetval.append( B2DPoint( rRect.getMinX(), rRect.getMinY() ) );
            aRetval.append( B2DPoint( rRect.getMaxX(), rRect.getMinY() ) );
            aRetval.append( B2DPoint( rRect.getMaxX(), rRect.getMaxY() ) );
            aRetval.append( B2DPoint( rRect.getMinX(), rRect.getMaxY() ) );

            
            aRetval.setClosed( true );

            return aRetval;
        }

        namespace
        {
            struct theUnitPolygon :
                public rtl::StaticWithInit<B2DPolygon, theUnitPolygon>
            {
                B2DPolygon operator () ()
                {
                    B2DPolygon aRetval;

                    aRetval.append( B2DPoint( 0.0, 0.0 ) );
                    aRetval.append( B2DPoint( 1.0, 0.0 ) );
                    aRetval.append( B2DPoint( 1.0, 1.0 ) );
                    aRetval.append( B2DPoint( 0.0, 1.0 ) );

                    
                    aRetval.setClosed( true );

                    return aRetval;
                }
            };
        }

        B2DPolygon createUnitPolygon()
        {
            return theUnitPolygon::get();
        }

        B2DPolygon createPolygonFromCircle( const B2DPoint& rCenter, double fRadius )
        {
            return createPolygonFromEllipse( rCenter, fRadius, fRadius );
        }

        B2DPolygon impCreateUnitCircle(sal_uInt32 nStartQuadrant)
        {
            B2DPolygon aUnitCircle;
            const double fKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);
            const double fScaledKappa(fKappa * (1.0 / STEPSPERQUARTER));
            const B2DHomMatrix aRotateMatrix(createRotateB2DHomMatrix(F_PI2 / STEPSPERQUARTER));

            B2DPoint aPoint(1.0, 0.0);
            B2DPoint aForward(1.0, fScaledKappa);
            B2DPoint aBackward(1.0, -fScaledKappa);

            if(0 != nStartQuadrant)
            {
                const B2DHomMatrix aQuadrantMatrix(createRotateB2DHomMatrix(F_PI2 * (nStartQuadrant % 4)));
                aPoint *= aQuadrantMatrix;
                aBackward *= aQuadrantMatrix;
                aForward *= aQuadrantMatrix;
            }

            aUnitCircle.append(aPoint);

            for(sal_uInt32 a(0); a < STEPSPERQUARTER * 4; a++)
            {
                aPoint *= aRotateMatrix;
                aBackward *= aRotateMatrix;
                aUnitCircle.appendBezierSegment(aForward, aBackward, aPoint);
                aForward *= aRotateMatrix;
            }

            aUnitCircle.setClosed(true);
            aUnitCircle.removeDoublePoints();

            return aUnitCircle;
        }

        namespace
        {
            struct theUnitHalfCircle :
                public rtl::StaticWithInit<B2DPolygon, theUnitHalfCircle>
            {
                B2DPolygon operator()()
                {
                    B2DPolygon aUnitHalfCircle;
                    const double fKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);
                    const double fScaledKappa(fKappa * (1.0 / STEPSPERQUARTER));
                    const B2DHomMatrix aRotateMatrix(createRotateB2DHomMatrix(F_PI2 / STEPSPERQUARTER));
                    B2DPoint aPoint(1.0, 0.0);
                    B2DPoint aForward(1.0, fScaledKappa);
                    B2DPoint aBackward(1.0, -fScaledKappa);

                    aUnitHalfCircle.append(aPoint);

                    for(sal_uInt32 a(0); a < STEPSPERQUARTER * 2; a++)
                    {
                        aPoint *= aRotateMatrix;
                        aBackward *= aRotateMatrix;
                        aUnitHalfCircle.appendBezierSegment(aForward, aBackward, aPoint);
                        aForward *= aRotateMatrix;
                    }
                    return aUnitHalfCircle;
                }
            };
        }

        B2DPolygon createHalfUnitCircle()
        {
            return theUnitHalfCircle::get();
        }

        namespace
        {
            struct theUnitCircleStartQuadrantOne :
                public rtl::StaticWithInit<B2DPolygon, theUnitCircleStartQuadrantOne>
            {
                B2DPolygon operator()() { return impCreateUnitCircle(1); }
            };

            struct theUnitCircleStartQuadrantTwo :
                public rtl::StaticWithInit<B2DPolygon, theUnitCircleStartQuadrantTwo>
            {
                B2DPolygon operator()() { return impCreateUnitCircle(2); }
            };

            struct theUnitCircleStartQuadrantThree :
                public rtl::StaticWithInit<B2DPolygon, theUnitCircleStartQuadrantThree>
            {
                B2DPolygon operator()() { return impCreateUnitCircle(3); }
            };

            struct theUnitCircleStartQuadrantZero :
                public rtl::StaticWithInit<B2DPolygon, theUnitCircleStartQuadrantZero>
            {
                B2DPolygon operator()() { return impCreateUnitCircle(0); }
            };
        }

        B2DPolygon createPolygonFromUnitCircle(sal_uInt32 nStartQuadrant)
        {
            switch(nStartQuadrant % 4)
            {
                case 1 :
                    return theUnitCircleStartQuadrantOne::get();

                case 2 :
                    return theUnitCircleStartQuadrantTwo::get();

                case 3 :
                    return theUnitCircleStartQuadrantThree::get();

                default : 
                    return theUnitCircleStartQuadrantZero::get();
            }
        }

        B2DPolygon createPolygonFromEllipse( const B2DPoint& rCenter, double fRadiusX, double fRadiusY )
        {
            B2DPolygon aRetval(createPolygonFromUnitCircle());
            const B2DHomMatrix aMatrix(createScaleTranslateB2DHomMatrix(fRadiusX, fRadiusY, rCenter.getX(), rCenter.getY()));

            aRetval.transform(aMatrix);

            return aRetval;
        }

        B2DPolygon createPolygonFromUnitEllipseSegment( double fStart, double fEnd )
        {
            B2DPolygon aRetval;

            
            
            if(fTools::less(fStart, 0.0))
            {
                fStart = 0.0;
            }

            if(fTools::moreOrEqual(fStart, F_2PI))
            {
                fStart = 0.0;
            }

            if(fTools::less(fEnd, 0.0))
            {
                fEnd = 0.0;
            }

            if(fTools::moreOrEqual(fEnd, F_2PI))
            {
                fEnd = 0.0;
            }

            if(fTools::equal(fStart, fEnd))
            {
                
                aRetval.append(B2DPoint(cos(fStart), sin(fStart)));
            }
            else
            {
                const sal_uInt32 nSegments(STEPSPERQUARTER * 4);
                const double fAnglePerSegment(F_PI2 / STEPSPERQUARTER);
                const sal_uInt32 nStartSegment(sal_uInt32(fStart / fAnglePerSegment) % nSegments);
                const sal_uInt32 nEndSegment(sal_uInt32(fEnd / fAnglePerSegment) % nSegments);
                const double fKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);
                const double fScaledKappa(fKappa * (1.0 / STEPSPERQUARTER));

                B2DPoint aSegStart(cos(fStart), sin(fStart));
                aRetval.append(aSegStart);

                if(nStartSegment == nEndSegment && fTools::more(fEnd, fStart))
                {
                    
                    const B2DPoint aSegEnd(cos(fEnd), sin(fEnd));
                    const double fFactor(fScaledKappa * ((fEnd - fStart) / fAnglePerSegment));

                    aRetval.appendBezierSegment(
                        aSegStart + (B2DPoint(-aSegStart.getY(), aSegStart.getX()) * fFactor),
                        aSegEnd - (B2DPoint(-aSegEnd.getY(), aSegEnd.getX()) * fFactor),
                        aSegEnd);
                }
                else
                {
                    double fSegEndRad((nStartSegment + 1) * fAnglePerSegment);
                    double fFactor(fScaledKappa * ((fSegEndRad - fStart) / fAnglePerSegment));
                    B2DPoint aSegEnd(cos(fSegEndRad), sin(fSegEndRad));

                    aRetval.appendBezierSegment(
                        aSegStart + (B2DPoint(-aSegStart.getY(), aSegStart.getX()) * fFactor),
                        aSegEnd - (B2DPoint(-aSegEnd.getY(), aSegEnd.getX()) * fFactor),
                        aSegEnd);

                    sal_uInt32 nSegment((nStartSegment + 1) % nSegments);
                    aSegStart = aSegEnd;

                    while(nSegment != nEndSegment)
                    {
                        
                        fSegEndRad = (nSegment + 1) * fAnglePerSegment;
                        aSegEnd = B2DPoint(cos(fSegEndRad), sin(fSegEndRad));

                        aRetval.appendBezierSegment(
                            aSegStart + (B2DPoint(-aSegStart.getY(), aSegStart.getX()) * fScaledKappa),
                            aSegEnd - (B2DPoint(-aSegEnd.getY(), aSegEnd.getX()) * fScaledKappa),
                            aSegEnd);

                        nSegment = (nSegment + 1) % nSegments;
                        aSegStart = aSegEnd;
                    }

                    
                    const double fSegStartRad(nSegment * fAnglePerSegment);
                    fFactor = fScaledKappa * ((fEnd - fSegStartRad) / fAnglePerSegment);
                    aSegEnd = B2DPoint(cos(fEnd), sin(fEnd));

                    aRetval.appendBezierSegment(
                        aSegStart + (B2DPoint(-aSegStart.getY(), aSegStart.getX()) * fFactor),
                        aSegEnd - (B2DPoint(-aSegEnd.getY(), aSegEnd.getX()) * fFactor),
                        aSegEnd);
                }
            }

            
            aRetval.removeDoublePoints();

            return aRetval;
        }

        B2DPolygon createPolygonFromEllipseSegment( const B2DPoint& rCenter, double fRadiusX, double fRadiusY, double fStart, double fEnd )
        {
            B2DPolygon aRetval(createPolygonFromUnitEllipseSegment(fStart, fEnd));
            const B2DHomMatrix aMatrix(createScaleTranslateB2DHomMatrix(fRadiusX, fRadiusY, rCenter.getX(), rCenter.getY()));

            aRetval.transform(aMatrix);

            return aRetval;
        }

        bool hasNeutralPoints(const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "hasNeutralPoints: ATM works not for curves (!)");
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 2L)
            {
                B2DPoint aPrevPoint(rCandidate.getB2DPoint(nPointCount - 1L));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint((a + 1) % nPointCount));
                    const B2DVector aPrevVec(aPrevPoint - aCurrPoint);
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aOrientation(getOrientation(aNextVec, aPrevVec));

                    if(ORIENTATION_NEUTRAL == aOrientation)
                    {
                        
                        return true;
                    }
                    else
                    {
                        
                        aPrevPoint = aCurrPoint;
                        aCurrPoint = aNextPoint;
                    }
                }
            }

            return false;
        }

        B2DPolygon removeNeutralPoints(const B2DPolygon& rCandidate)
        {
            if(hasNeutralPoints(rCandidate))
            {
                const sal_uInt32 nPointCount(rCandidate.count());
                B2DPolygon aRetval;
                B2DPoint aPrevPoint(rCandidate.getB2DPoint(nPointCount - 1L));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint((a + 1) % nPointCount));
                    const B2DVector aPrevVec(aPrevPoint - aCurrPoint);
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aOrientation(getOrientation(aNextVec, aPrevVec));

                    if(ORIENTATION_NEUTRAL == aOrientation)
                    {
                        
                        aCurrPoint = aNextPoint;
                    }
                    else
                    {
                        
                        aRetval.append(aCurrPoint);

                        
                        aPrevPoint = aCurrPoint;
                        aCurrPoint = aNextPoint;
                    }
                }

                while(aRetval.count() && ORIENTATION_NEUTRAL == getOrientationForIndex(aRetval, 0L))
                {
                    aRetval.remove(0L);
                }

                
                aRetval.setClosed(rCandidate.isClosed());

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        bool isConvex(const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(!rCandidate.areControlPointsUsed(), "isConvex: ATM works not for curves (!)");
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 2L)
            {
                const B2DPoint aPrevPoint(rCandidate.getB2DPoint(nPointCount - 1L));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0L));
                B2DVector aCurrVec(aPrevPoint - aCurrPoint);
                B2VectorOrientation aOrientation(ORIENTATION_NEUTRAL);

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint((a + 1) % nPointCount));
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aCurrentOrientation(getOrientation(aNextVec, aCurrVec));

                    if(ORIENTATION_NEUTRAL == aOrientation)
                    {
                        
                        aOrientation = aCurrentOrientation;
                    }
                    else
                    {
                        if(ORIENTATION_NEUTRAL != aCurrentOrientation && aCurrentOrientation != aOrientation)
                        {
                            
                            return false;
                        }
                    }

                    
                    aCurrPoint = aNextPoint;
                    aCurrVec = -aNextVec;
                }
            }

            return true;
        }

        B2VectorOrientation getOrientationForIndex(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getOrientationForIndex: index out of range (!)");
            const B2DPoint aPrev(rCandidate.getB2DPoint(getIndexOfPredecessor(nIndex, rCandidate)));
            const B2DPoint aCurr(rCandidate.getB2DPoint(nIndex));
            const B2DPoint aNext(rCandidate.getB2DPoint(getIndexOfSuccessor(nIndex, rCandidate)));
            const B2DVector aBack(aPrev - aCurr);
            const B2DVector aForw(aNext - aCurr);

            return getOrientation(aForw, aBack);
        }

        bool isPointOnLine(const B2DPoint& rStart, const B2DPoint& rEnd, const B2DPoint& rCandidate, bool bWithPoints)
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
                const B2DVector aEdgeVector(rEnd - rStart);
                const B2DVector aTestVector(rCandidate - rStart);

                if(areParallel(aEdgeVector, aTestVector))
                {
                    const double fZero(0.0);
                    const double fOne(1.0);
                    const double fParamTestOnCurr(fabs(aEdgeVector.getX()) > fabs(aEdgeVector.getY())
                        ? aTestVector.getX() / aEdgeVector.getX()
                        : aTestVector.getY() / aEdgeVector.getY());

                    if(fTools::more(fParamTestOnCurr, fZero) && fTools::less(fParamTestOnCurr, fOne))
                    {
                        return true;
                    }
                }

                return false;
            }
        }

        bool isPointOnPolygon(const B2DPolygon& rCandidate, const B2DPoint& rPoint, bool bWithPoints)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? rCandidate.getDefaultAdaptiveSubdivision() : rCandidate);
            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount > 1L)
            {
                const sal_uInt32 nLoopCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                B2DPoint aCurrentPoint(aCandidate.getB2DPoint(0L));

                for(sal_uInt32 a(0L); a < nLoopCount; a++)
                {
                    const B2DPoint aNextPoint(aCandidate.getB2DPoint((a + 1L) % nPointCount));

                    if(isPointOnLine(aCurrentPoint, aNextPoint, rPoint, bWithPoints))
                    {
                        return true;
                    }

                    aCurrentPoint = aNextPoint;
                }
            }
            else if(nPointCount && bWithPoints)
            {
                return rPoint.equal(aCandidate.getB2DPoint(0L));
            }

            return false;
        }

        bool isPointInTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC, const B2DPoint& rCandidate, bool bWithBorder)
        {
            if(arePointsOnSameSideOfLine(rA, rB, rC, rCandidate, bWithBorder))
            {
                if(arePointsOnSameSideOfLine(rB, rC, rA, rCandidate, bWithBorder))
                {
                    if(arePointsOnSameSideOfLine(rC, rA, rB, rCandidate, bWithBorder))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        bool arePointsOnSameSideOfLine(const B2DPoint& rStart, const B2DPoint& rEnd, const B2DPoint& rCandidateA, const B2DPoint& rCandidateB, bool bWithLine)
        {
            const B2DVector aLineVector(rEnd - rStart);
            const B2DVector aVectorToA(rEnd - rCandidateA);
            const double fCrossA(aLineVector.cross(aVectorToA));

            if(fTools::equalZero(fCrossA))
            {
                
                return bWithLine;
            }

            const B2DVector aVectorToB(rEnd - rCandidateB);
            const double fCrossB(aLineVector.cross(aVectorToB));

            if(fTools::equalZero(fCrossB))
            {
                
                return bWithLine;
            }

            
            return ((fCrossA > 0.0) == (fCrossB > 0.0));
        }

        void addTriangleFan(const B2DPolygon& rCandidate, B2DPolygon& rTarget)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount > 2L)
            {
                const B2DPoint aStart(rCandidate.getB2DPoint(0L));
                B2DPoint aLast(rCandidate.getB2DPoint(1L));

                for(sal_uInt32 a(2L); a < nCount; a++)
                {
                    const B2DPoint aCurrent(rCandidate.getB2DPoint(a));
                    rTarget.append(aStart);
                    rTarget.append(aLast);
                    rTarget.append(aCurrent);

                    
                    aLast = aCurrent;
                }
            }
        }

        namespace
        {
            
            inline int lcl_sgn( const double n )
            {
                return n == 0.0 ? 0 : 1 - 2*int(rtl::math::isSignBitSet(n));
            }
        }

        bool isRectangle( const B2DPolygon& rPoly )
        {
            
            
            if( !rPoly.isClosed() ||
                rPoly.count() < 4 ||
                rPoly.areControlPointsUsed() )
            {
                return false;
            }

            
            int nNumTurns(0);

            int  nVerticalEdgeType=0;
            int  nHorizontalEdgeType=0;
            bool bNullVertex(true);
            bool bCWPolygon(false);  
                                     
            bool bOrientationSet(false); 
                                         
                                         

            
            
            const sal_Int32 nCount( rPoly.count() );
            for( sal_Int32 i=0; i<nCount; ++i )
            {
                const B2DPoint& rPoint0( rPoly.getB2DPoint(i % nCount) );
                const B2DPoint& rPoint1( rPoly.getB2DPoint((i+1) % nCount) );

                
                
                int nCurrVerticalEdgeType( lcl_sgn( rPoint1.getY() - rPoint0.getY() ) );

                
                
                int nCurrHorizontalEdgeType( lcl_sgn(rPoint1.getX() - rPoint0.getX()) );

                if( nCurrVerticalEdgeType && nCurrHorizontalEdgeType )
                    return false; 

                const bool bCurrNullVertex( !nCurrVerticalEdgeType && !nCurrHorizontalEdgeType );

                
                
                if( bCurrNullVertex )
                    continue;

                
                
                
                
                
                if( !bNullVertex )
                {
                    
                    const int nCrossProduct( nHorizontalEdgeType*nCurrVerticalEdgeType -
                                             nVerticalEdgeType*nCurrHorizontalEdgeType );

                    if( !nCrossProduct )
                        continue; 
                                  

                    
                    
                    if( !bOrientationSet )
                    {
                        bCWPolygon = nCrossProduct == 1;
                        bOrientationSet = true;
                    }
                    else
                    {
                        
                        
                        
                        
                        if( (nCrossProduct == 1) != bCWPolygon )
                            return false;
                    }

                    ++nNumTurns;

                    
                    
                    if( nNumTurns > 4 )
                        return false;
                }

                
                nVerticalEdgeType   = nCurrVerticalEdgeType;
                nHorizontalEdgeType = nCurrHorizontalEdgeType;
                bNullVertex         = false; 
                                             
                                             
            }

            return true;
        }

        B3DPolygon createB3DPolygonFromB2DPolygon(const B2DPolygon& rCandidate, double fZCoordinate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                
                const B2DPolygon aCandidate(adaptiveSubdivideByAngle(rCandidate));
                return createB3DPolygonFromB2DPolygon(aCandidate, fZCoordinate);
            }
            else
            {
                B3DPolygon aRetval;

                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    B2DPoint aPoint(rCandidate.getB2DPoint(a));
                    aRetval.append(B3DPoint(aPoint.getX(), aPoint.getY(), fZCoordinate));
                }

                
                aRetval.setClosed(rCandidate.isClosed());

                return aRetval;
            }
        }

        B2DPolygon createB2DPolygonFromB3DPolygon(const B3DPolygon& rCandidate, const B3DHomMatrix& rMat)
        {
            B2DPolygon aRetval;
            const sal_uInt32 nCount(rCandidate.count());
            const bool bIsIdentity(rMat.isIdentity());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                B3DPoint aCandidate(rCandidate.getB3DPoint(a));

                if(!bIsIdentity)
                {
                    aCandidate *= rMat;
                }

                aRetval.append(B2DPoint(aCandidate.getX(), aCandidate.getY()));
            }

            
            aRetval.setClosed(rCandidate.isClosed());

            return aRetval;
        }

        double getSmallestDistancePointToEdge(const B2DPoint& rPointA, const B2DPoint& rPointB, const B2DPoint& rTestPoint, double& rCut)
        {
            if(rPointA.equal(rPointB))
            {
                rCut = 0.0;
                const B2DVector aVector(rTestPoint - rPointA);
                return aVector.getLength();
            }
            else
            {
                
                const B2DVector aVector1(rPointB - rPointA);
                const B2DVector aVector2(rTestPoint - rPointA);
                const double fDividend((aVector2.getX() * aVector1.getX()) + (aVector2.getY() * aVector1.getY()));
                const double fDivisor((aVector1.getX() * aVector1.getX()) + (aVector1.getY() * aVector1.getY()));
                const double fCut(fDividend / fDivisor);

                if(fCut < 0.0)
                {
                    
                    rCut = 0.0;
                    return aVector2.getLength();
                }
                else if(fCut > 1.0)
                {
                    
                    rCut = 1.0;
                    const B2DVector aVector(rTestPoint - rPointB);
                    return aVector.getLength();
                }
                else
                {
                    
                    const B2DPoint aCutPoint(rPointA + fCut * aVector1);
                    const B2DVector aVector(rTestPoint - aCutPoint);
                    rCut = fCut;
                    return aVector.getLength();
                }
            }
        }

        double getSmallestDistancePointToPolygon(const B2DPolygon& rCandidate, const B2DPoint& rTestPoint, sal_uInt32& rEdgeIndex, double& rCut)
        {
            double fRetval(DBL_MAX);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1L)
            {
                const double fZero(0.0);
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                B2DCubicBezier aBezier;
                aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                {
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    aBezier.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                    double fEdgeDist;
                    double fNewCut(0.0);
                    bool bEdgeIsCurve(false);

                    if(rCandidate.areControlPointsUsed())
                    {
                        aBezier.setControlPointA(rCandidate.getNextControlPoint(a));
                        aBezier.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                        aBezier.testAndSolveTrivialBezier();
                        bEdgeIsCurve = aBezier.isBezier();
                    }

                    if(bEdgeIsCurve)
                    {
                        fEdgeDist = aBezier.getSmallestDistancePointToBezierSegment(rTestPoint, fNewCut);
                    }
                    else
                    {
                        fEdgeDist = getSmallestDistancePointToEdge(aBezier.getStartPoint(), aBezier.getEndPoint(), rTestPoint, fNewCut);
                    }

                    if(DBL_MAX == fRetval || fEdgeDist < fRetval)
                    {
                        fRetval = fEdgeDist;
                        rEdgeIndex = a;
                        rCut = fNewCut;

                        if(fTools::equal(fRetval, fZero))
                        {
                            
                            fRetval = 0.0;
                            break;
                        }
                    }

                    
                    aBezier.setStartPoint(aBezier.getEndPoint());
                }

                if(1.0 == rCut)
                {
                    
                    if(rCandidate.isClosed())
                    {
                        rEdgeIndex = getIndexOfSuccessor(rEdgeIndex, rCandidate);
                        rCut = 0.0;
                    }
                    else
                    {
                        if(rEdgeIndex != nEdgeCount - 1L)
                        {
                            rEdgeIndex++;
                            rCut = 0.0;
                        }
                    }
                }
            }

            return fRetval;
        }

        B2DPoint distort(const B2DPoint& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight)
        {
            if(fTools::equalZero(rOriginal.getWidth()) || fTools::equalZero(rOriginal.getHeight()))
            {
                return rCandidate;
            }
            else
            {
                const double fRelativeX((rCandidate.getX() - rOriginal.getMinX()) / rOriginal.getWidth());
                const double fRelativeY((rCandidate.getY() - rOriginal.getMinY()) / rOriginal.getHeight());
                const double fOneMinusRelativeX(1.0 - fRelativeX);
                const double fOneMinusRelativeY(1.0 - fRelativeY);
                const double fNewX((fOneMinusRelativeY) * ((fOneMinusRelativeX) * rTopLeft.getX() + fRelativeX * rTopRight.getX()) +
                    fRelativeY * ((fOneMinusRelativeX) * rBottomLeft.getX() + fRelativeX * rBottomRight.getX()));
                const double fNewY((fOneMinusRelativeX) * ((fOneMinusRelativeY) * rTopLeft.getY() + fRelativeY * rBottomLeft.getY()) +
                    fRelativeX * ((fOneMinusRelativeY) * rTopRight.getY() + fRelativeY * rBottomRight.getY()));

                return B2DPoint(fNewX, fNewY);
            }
        }

        B2DPolygon distort(const B2DPolygon& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount && 0.0 != rOriginal.getWidth() && 0.0 != rOriginal.getHeight())
            {
                B2DPolygon aRetval;

                for(sal_uInt32 a(0L); a < nPointCount; a++)
                {
                    aRetval.append(distort(rCandidate.getB2DPoint(a), rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));

                    if(rCandidate.areControlPointsUsed())
                    {
                        if(!rCandidate.getPrevControlPoint(a).equalZero())
                        {
                            aRetval.setPrevControlPoint(a, distort(rCandidate.getPrevControlPoint(a), rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));
                        }

                        if(!rCandidate.getNextControlPoint(a).equalZero())
                        {
                            aRetval.setNextControlPoint(a, distort(rCandidate.getNextControlPoint(a), rOriginal, rTopLeft, rTopRight, rBottomLeft, rBottomRight));
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

        B2DPolygon expandToCurve(const B2DPolygon& rCandidate)
        {
            B2DPolygon aRetval(rCandidate);

            for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
            {
                expandToCurveInPoint(aRetval, a);
            }

            return aRetval;
        }

        bool expandToCurveInPoint(B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "expandToCurveInPoint: Access to polygon out of range (!)");
            bool bRetval(false);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                
                if(!rCandidate.isPrevControlPointUsed(nIndex))
                {
                    if(!rCandidate.isClosed() && 0 == nIndex)
                    {
                        
                    }
                    else
                    {
                        const sal_uInt32 nPrevIndex((nIndex + (nPointCount - 1)) % nPointCount);
                        rCandidate.setPrevControlPoint(nIndex, interpolate(rCandidate.getB2DPoint(nIndex), rCandidate.getB2DPoint(nPrevIndex), 1.0 / 3.0));
                        bRetval = true;
                    }
                }

                
                if(!rCandidate.isNextControlPointUsed(nIndex))
                {
                    if(!rCandidate.isClosed() && nIndex + 1 == nPointCount)
                    {
                        
                    }
                    else
                    {
                        const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                        rCandidate.setNextControlPoint(nIndex, interpolate(rCandidate.getB2DPoint(nIndex), rCandidate.getB2DPoint(nNextIndex), 1.0 / 3.0));
                        bRetval = true;
                    }
                }
            }

            return bRetval;
        }

        bool setContinuityInPoint(B2DPolygon& rCandidate, sal_uInt32 nIndex, B2VectorContinuity eContinuity)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "setContinuityInPoint: Access to polygon out of range (!)");
            bool bRetval(false);
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                const B2DPoint aCurrentPoint(rCandidate.getB2DPoint(nIndex));

                switch(eContinuity)
                {
                    case CONTINUITY_NONE :
                    {
                        if(rCandidate.isPrevControlPointUsed(nIndex))
                        {
                            if(!rCandidate.isClosed() && 0 == nIndex)
                            {
                                
                                rCandidate.resetPrevControlPoint(nIndex);
                            }
                            else
                            {
                                const sal_uInt32 nPrevIndex((nIndex + (nPointCount - 1)) % nPointCount);
                                rCandidate.setPrevControlPoint(nIndex, interpolate(aCurrentPoint, rCandidate.getB2DPoint(nPrevIndex), 1.0 / 3.0));
                            }

                            bRetval = true;
                        }

                        if(rCandidate.isNextControlPointUsed(nIndex))
                        {
                            if(!rCandidate.isClosed() && nIndex == nPointCount + 1)
                            {
                                
                                rCandidate.resetNextControlPoint(nIndex);
                            }
                            else
                            {
                                const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                                rCandidate.setNextControlPoint(nIndex, interpolate(aCurrentPoint, rCandidate.getB2DPoint(nNextIndex), 1.0 / 3.0));
                            }

                            bRetval = true;
                        }

                        break;
                    }
                    case CONTINUITY_C1 :
                    {
                        if(rCandidate.isPrevControlPointUsed(nIndex) && rCandidate.isNextControlPointUsed(nIndex))
                        {
                            
                            B2DVector aVectorPrev(rCandidate.getPrevControlPoint(nIndex) - aCurrentPoint);
                            B2DVector aVectorNext(rCandidate.getNextControlPoint(nIndex) - aCurrentPoint);
                            const double fLenPrev(aVectorPrev.getLength());
                            const double fLenNext(aVectorNext.getLength());
                            aVectorPrev.normalize();
                            aVectorNext.normalize();
                            const B2VectorOrientation aOrientation(getOrientation(aVectorPrev, aVectorNext));

                            if(ORIENTATION_NEUTRAL == aOrientation && aVectorPrev.scalar(aVectorNext) < 0.0)
                            {
                                
                                if(fTools::equal(fLenPrev, fLenNext))
                                {
                                    
                                    const sal_uInt32 nPrevIndex((nIndex + (nPointCount - 1)) % nPointCount);
                                    const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                                    const double fLenPrevEdge(B2DVector(rCandidate.getB2DPoint(nPrevIndex) - aCurrentPoint).getLength() * (1.0 / 3.0));
                                    const double fLenNextEdge(B2DVector(rCandidate.getB2DPoint(nNextIndex) - aCurrentPoint).getLength() * (1.0 / 3.0));

                                    rCandidate.setControlPoints(nIndex,
                                        aCurrentPoint + (aVectorPrev * fLenPrevEdge),
                                        aCurrentPoint + (aVectorNext * fLenNextEdge));
                                    bRetval = true;
                                }
                            }
                            else
                            {
                                
                                const B2DVector aNormalizedPerpendicular(getNormalizedPerpendicular(aVectorPrev + aVectorNext));

                                if(ORIENTATION_POSITIVE == aOrientation)
                                {
                                    rCandidate.setControlPoints(nIndex,
                                        aCurrentPoint - (aNormalizedPerpendicular * fLenPrev),
                                        aCurrentPoint + (aNormalizedPerpendicular * fLenNext));
                                }
                                else
                                {
                                    rCandidate.setControlPoints(nIndex,
                                        aCurrentPoint + (aNormalizedPerpendicular * fLenPrev),
                                        aCurrentPoint - (aNormalizedPerpendicular * fLenNext));
                                }

                                bRetval = true;
                            }
                        }
                        break;
                    }
                    case CONTINUITY_C2 :
                    {
                        if(rCandidate.isPrevControlPointUsed(nIndex) && rCandidate.isNextControlPointUsed(nIndex))
                        {
                            
                            B2DVector aVectorPrev(rCandidate.getPrevControlPoint(nIndex) - aCurrentPoint);
                            B2DVector aVectorNext(rCandidate.getNextControlPoint(nIndex) - aCurrentPoint);
                            const double fCommonLength((aVectorPrev.getLength() + aVectorNext.getLength()) / 2.0);
                            aVectorPrev.normalize();
                            aVectorNext.normalize();
                            const B2VectorOrientation aOrientation(getOrientation(aVectorPrev, aVectorNext));

                            if(ORIENTATION_NEUTRAL == aOrientation && aVectorPrev.scalar(aVectorNext) < 0.0)
                            {
                                
                                const B2DVector aScaledDirection(aVectorPrev * fCommonLength);

                                rCandidate.setControlPoints(nIndex,
                                    aCurrentPoint + aScaledDirection,
                                    aCurrentPoint - aScaledDirection);
                            }
                            else
                            {
                                
                                const B2DVector aNormalizedPerpendicular(getNormalizedPerpendicular(aVectorPrev + aVectorNext));
                                const B2DVector aPerpendicular(aNormalizedPerpendicular * fCommonLength);

                                if(ORIENTATION_POSITIVE == aOrientation)
                                {
                                    rCandidate.setControlPoints(nIndex,
                                        aCurrentPoint - aPerpendicular,
                                        aCurrentPoint + aPerpendicular);
                                }
                                else
                                {
                                    rCandidate.setControlPoints(nIndex,
                                        aCurrentPoint + aPerpendicular,
                                        aCurrentPoint - aPerpendicular);
                                }
                            }

                            bRetval = true;
                        }
                        break;
                    }
                }
            }

            return bRetval;
        }

        B2DPolygon growInNormalDirection(const B2DPolygon& rCandidate, double fValue)
        {
            if(0.0 != fValue)
            {
                if(rCandidate.areControlPointsUsed())
                {
                    
                    const B2DPolygon aCandidate(adaptiveSubdivideByAngle(rCandidate));
                    return growInNormalDirection(aCandidate, fValue);
                }
                else
                {
                    B2DPolygon aRetval;
                    const sal_uInt32 nPointCount(rCandidate.count());

                    if(nPointCount)
                    {
                        B2DPoint aPrev(rCandidate.getB2DPoint(nPointCount - 1L));
                        B2DPoint aCurrent(rCandidate.getB2DPoint(0L));

                        for(sal_uInt32 a(0L); a < nPointCount; a++)
                        {
                            const B2DPoint aNext(rCandidate.getB2DPoint(a + 1L == nPointCount ? 0L : a + 1L));
                            const B2DVector aBack(aPrev - aCurrent);
                            const B2DVector aForw(aNext - aCurrent);
                            const B2DVector aPerpBack(getNormalizedPerpendicular(aBack));
                            const B2DVector aPerpForw(getNormalizedPerpendicular(aForw));
                            B2DVector aDirection(aPerpBack - aPerpForw);
                            aDirection.normalize();
                            aDirection *= fValue;
                            aRetval.append(aCurrent + aDirection);

                            
                            aPrev = aCurrent;
                            aCurrent = aNext;
                        }
                    }

                    
                    aRetval.setClosed(rCandidate.isClosed());

                    return aRetval;
                }
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolygon reSegmentPolygon(const B2DPolygon& rCandidate, sal_uInt32 nSegments)
        {
            B2DPolygon aRetval;
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount && nSegments)
            {
                
                const sal_uInt32 nSegmentCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1L);

                if(nSegmentCount == nSegments)
                {
                    aRetval = rCandidate;
                }
                else
                {
                    const double fLength(getLength(rCandidate));
                    const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nSegments : nSegments + 1L);

                    for(sal_uInt32 a(0L); a < nLoopCount; a++)
                    {
                        const double fRelativePos((double)a / (double)nSegments); 
                        const B2DPoint aNewPoint(getPositionRelative(rCandidate, fRelativePos, fLength));
                        aRetval.append(aNewPoint);
                    }

                    
                    aRetval.setClosed(rCandidate.isClosed());
                }
            }

            return aRetval;
        }

        B2DPolygon interpolate(const B2DPolygon& rOld1, const B2DPolygon& rOld2, double t)
        {
            OSL_ENSURE(rOld1.count() == rOld2.count(), "B2DPolygon interpolate: Different geometry (!)");

            if(fTools::lessOrEqual(t, 0.0) || rOld1 == rOld2)
            {
                return rOld1;
            }
            else if(fTools::moreOrEqual(t, 1.0))
            {
                return rOld2;
            }
            else
            {
                B2DPolygon aRetval;
                const bool bInterpolateVectors(rOld1.areControlPointsUsed() || rOld2.areControlPointsUsed());
                aRetval.setClosed(rOld1.isClosed() && rOld2.isClosed());

                for(sal_uInt32 a(0L); a < rOld1.count(); a++)
                {
                    aRetval.append(interpolate(rOld1.getB2DPoint(a), rOld2.getB2DPoint(a), t));

                    if(bInterpolateVectors)
                    {
                        aRetval.setPrevControlPoint(a, interpolate(rOld1.getPrevControlPoint(a), rOld2.getPrevControlPoint(a), t));
                        aRetval.setNextControlPoint(a, interpolate(rOld1.getNextControlPoint(a), rOld2.getNextControlPoint(a), t));
                    }
                }

                return aRetval;
            }
        }

        
        B2DPolygon simplifyCurveSegments(const B2DPolygon& rCandidate)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount && rCandidate.areControlPointsUsed())
            {
                
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                B2DPolygon aRetval;
                B2DCubicBezier aBezier;
                aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                
                aRetval.reserve( nEdgeCount+1);

                
                aRetval.append(aBezier.getStartPoint());

                for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                {
                    
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    aBezier.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                    aBezier.setControlPointA(rCandidate.getNextControlPoint(a));
                    aBezier.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                    aBezier.testAndSolveTrivialBezier();

                    
                    if(aBezier.isBezier())
                    {
                        
                        aRetval.appendBezierSegment(aBezier.getControlPointA(), aBezier.getControlPointB(), aBezier.getEndPoint());
                    }
                    else
                    {
                        
                        aRetval.append(aBezier.getEndPoint());
                    }

                    
                    aBezier.setStartPoint(aBezier.getEndPoint());
                }

                if(rCandidate.isClosed())
                {
                    
                    closeWithGeometryChange(aRetval);
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        
        
        
        B2DPolygon makeStartPoint(const B2DPolygon& rCandidate, sal_uInt32 nIndexOfNewStatPoint)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 2 && nIndexOfNewStatPoint != 0 && nIndexOfNewStatPoint < nPointCount)
            {
                OSL_ENSURE(rCandidate.isClosed(), "makeStartPoint: only valid for closed polygons (!)");
                B2DPolygon aRetval;

                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    const sal_uInt32 nSourceIndex((a + nIndexOfNewStatPoint) % nPointCount);
                    aRetval.append(rCandidate.getB2DPoint(nSourceIndex));

                    if(rCandidate.areControlPointsUsed())
                    {
                        aRetval.setPrevControlPoint(a, rCandidate.getPrevControlPoint(nSourceIndex));
                        aRetval.setNextControlPoint(a, rCandidate.getNextControlPoint(nSourceIndex));
                    }
                }

                return aRetval;
            }

            return rCandidate;
        }

        B2DPolygon createEdgesOfGivenLength(const B2DPolygon& rCandidate, double fLength, double fStart, double fEnd)
        {
            B2DPolygon aRetval;

            if(fLength < 0.0)
            {
                fLength = 0.0;
            }

            if(!fTools::equalZero(fLength))
            {
                if(fStart < 0.0)
                {
                    fStart = 0.0;
                }

                if(fEnd < 0.0)
                {
                    fEnd = 0.0;
                }

                if(fEnd < fStart)
                {
                    fEnd = fStart;
                }

                
                const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? rCandidate.getDefaultAdaptiveSubdivision() : rCandidate);
                const sal_uInt32 nPointCount(aCandidate.count());

                if(nPointCount > 1)
                {
                    const bool bEndActive(!fTools::equalZero(fEnd));
                    const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1);
                    B2DPoint aCurrent(aCandidate.getB2DPoint(0));
                    double fPositionInEdge(fStart);
                    double fAbsolutePosition(fStart);

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        const B2DPoint aNext(aCandidate.getB2DPoint(nNextIndex));
                        const B2DVector aEdge(aNext - aCurrent);
                        double fEdgeLength(aEdge.getLength());

                        if(!fTools::equalZero(fEdgeLength))
                        {
                            while(fTools::less(fPositionInEdge, fEdgeLength))
                            {
                                
                                const double fScalar(fPositionInEdge / fEdgeLength);
                                aRetval.append(aCurrent + (aEdge * fScalar));
                                fPositionInEdge += fLength;

                                if(bEndActive)
                                {
                                    fAbsolutePosition += fLength;

                                    if(fTools::more(fAbsolutePosition, fEnd))
                                    {
                                        break;
                                    }
                                }
                            }

                            
                            fPositionInEdge -= fEdgeLength;
                        }

                        if(bEndActive && fTools::more(fAbsolutePosition, fEnd))
                        {
                            break;
                        }

                        
                        aCurrent = aNext;
                    }

                    
                    aRetval.setClosed(aCandidate.isClosed());
                }
                else
                {
                    
                    aRetval = aCandidate;
                }
            }

            return aRetval;
        }

        B2DPolygon createWaveline(const B2DPolygon& rCandidate, double fWaveWidth, double fWaveHeight)
        {
            B2DPolygon aRetval;

            if(fWaveWidth < 0.0)
            {
                fWaveWidth = 0.0;
            }

            if(fWaveHeight < 0.0)
            {
                fWaveHeight = 0.0;
            }

            const bool bHasWidth(!fTools::equalZero(fWaveWidth));

            if(bHasWidth)
            {
                const bool bHasHeight(!fTools::equalZero(fWaveHeight));
                if(bHasHeight)
                {
                    
                    
                    
                    
                    
                    
                    
                    const B2DPolygon aEqualLenghEdges(createEdgesOfGivenLength(rCandidate, fWaveWidth));
                    const sal_uInt32 nPointCount(aEqualLenghEdges.count());

                    if(nPointCount > 1)
                    {
                        
                        B2DPoint aCurrent(aEqualLenghEdges.getB2DPoint(0));
                        aRetval.append(aCurrent);

                        for(sal_uInt32 a(0); a < nPointCount - 1; a++)
                        {
                            const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                            const B2DPoint aNext(aEqualLenghEdges.getB2DPoint(nNextIndex));
                            const B2DVector aEdge(aNext - aCurrent);
                            const B2DVector aPerpendicular(getNormalizedPerpendicular(aEdge));
                            const B2DVector aControlOffset((aEdge * 0.467308) - (aPerpendicular * fWaveHeight));

                            
                            aRetval.appendBezierSegment(
                                aCurrent + aControlOffset,
                                aNext - aControlOffset,
                                aNext);

                            
                            aCurrent = aNext;
                        }
                    }
                }
                else
                {
                    
                    aRetval = rCandidate;
                }
            }
            else
            {
                
            }

            return aRetval;
        }

        
        

        bool equal(const B2DPolygon& rCandidateA, const B2DPolygon& rCandidateB, const double& rfSmallValue)
        {
            const sal_uInt32 nPointCount(rCandidateA.count());

            if(nPointCount != rCandidateB.count())
                return false;

            const bool bClosed(rCandidateA.isClosed());

            if(bClosed != rCandidateB.isClosed())
                return false;

            const bool bAreControlPointsUsed(rCandidateA.areControlPointsUsed());

            if(bAreControlPointsUsed != rCandidateB.areControlPointsUsed())
                return false;

            for(sal_uInt32 a(0); a < nPointCount; a++)
            {
                const B2DPoint aPoint(rCandidateA.getB2DPoint(a));

                if(!aPoint.equal(rCandidateB.getB2DPoint(a), rfSmallValue))
                    return false;

                if(bAreControlPointsUsed)
                {
                    const basegfx::B2DPoint aPrev(rCandidateA.getPrevControlPoint(a));

                    if(!aPrev.equal(rCandidateB.getPrevControlPoint(a), rfSmallValue))
                        return false;

                    const basegfx::B2DPoint aNext(rCandidateA.getNextControlPoint(a));

                    if(!aNext.equal(rCandidateB.getNextControlPoint(a), rfSmallValue))
                        return false;
                }
            }

            return true;
        }

        
        B2DPolygon snapPointsOfHorizontalOrVerticalEdges(const B2DPolygon& rCandidate)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1)
            {
                
                
                B2DPolygon aRetval(rCandidate);

                
                B2ITuple aPrevTuple(basegfx::fround(rCandidate.getB2DPoint(nPointCount - 1)));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0));
                B2ITuple aCurrTuple(basegfx::fround(aCurrPoint));

                
                
                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    
                    const bool bLastRun(a + 1 == nPointCount);
                    const sal_uInt32 nNextIndex(bLastRun ? 0 : a + 1);
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                    const B2ITuple aNextTuple(basegfx::fround(aNextPoint));

                    
                    const bool bPrevVertical(aPrevTuple.getX() == aCurrTuple.getX());
                    const bool bNextVertical(aNextTuple.getX() == aCurrTuple.getX());
                    const bool bPrevHorizontal(aPrevTuple.getY() == aCurrTuple.getY());
                    const bool bNextHorizontal(aNextTuple.getY() == aCurrTuple.getY());
                    const bool bSnapX(bPrevVertical || bNextVertical);
                    const bool bSnapY(bPrevHorizontal || bNextHorizontal);

                    if(bSnapX || bSnapY)
                    {
                        const B2DPoint aSnappedPoint(
                            bSnapX ? aCurrTuple.getX() : aCurrPoint.getX(),
                            bSnapY ? aCurrTuple.getY() : aCurrPoint.getY());

                        aRetval.setB2DPoint(a, aSnappedPoint);
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

        bool containsOnlyHorizontalAndVerticalEdges(const B2DPolygon& rCandidate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                return false;
            }

            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount < 2)
            {
                return true;
            }

            const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount + 1 : nPointCount);
            basegfx::B2DPoint aLast(rCandidate.getB2DPoint(0));

            for(sal_uInt32 a(1); a < nEdgeCount; a++)
            {
                const sal_uInt32 nNextIndex(a % nPointCount);
                const basegfx::B2DPoint aCurrent(rCandidate.getB2DPoint(nNextIndex));

                if(!basegfx::fTools::equal(aLast.getX(), aCurrent.getX()) && !basegfx::fTools::equal(aLast.getY(), aCurrent.getY()))
                {
                    return false;
                }

                aLast = aCurrent;
            }

            return true;
        }

        B2DVector getTangentEnteringPoint(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            B2DVector aRetval(0.0, 0.0);
            const sal_uInt32 nCount(rCandidate.count());

            if(nIndex >= nCount)
            {
                
                return aRetval;
            }

            
            const bool bClosed(rCandidate.isClosed());
            sal_uInt32 nPrev(bClosed ? (nIndex + nCount - 1) % nCount : nIndex ? nIndex - 1 : nIndex);

            if(nPrev == nIndex)
            {
                
                return aRetval;
            }

            B2DCubicBezier aSegment;

            
            
            while(nPrev != nIndex)
            {
                
                rCandidate.getBezierSegment(nPrev, aSegment);
                aRetval = aSegment.getTangent(1.0);

                if(!aRetval.equalZero())
                {
                    
                    return aRetval;
                }

                
                nPrev = bClosed ? (nPrev + nCount - 1) % nCount : nPrev ? nPrev - 1 : nIndex;
            }

            return aRetval;
        }

        B2DVector getTangentLeavingPoint(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            B2DVector aRetval(0.0, 0.0);
            const sal_uInt32 nCount(rCandidate.count());

            if(nIndex >= nCount)
            {
                
                return aRetval;
            }

            
            const bool bClosed(rCandidate.isClosed());
            sal_uInt32 nCurrent(nIndex);
            B2DCubicBezier aSegment;

            
            
            do
            {
                
                rCandidate.getBezierSegment(nCurrent, aSegment);
                aRetval = aSegment.getTangent(0.0);

                if(!aRetval.equalZero())
                {
                    
                    return aRetval;
                }

                
                nCurrent = bClosed ? (nCurrent + 1) % nCount : nCurrent + 1 < nCount ? nCurrent + 1 : nIndex;
            }
            while(nCurrent != nIndex);

            return aRetval;
        }

        
        

        B2DPolygon UnoPointSequenceToB2DPolygon(
            const com::sun::star::drawing::PointSequence& rPointSequenceSource,
            bool bCheckClosed)
        {
            B2DPolygon aRetval;
            const sal_uInt32 nLength(rPointSequenceSource.getLength());

            if(nLength)
            {
                aRetval.reserve(nLength);
                const com::sun::star::awt::Point* pArray = rPointSequenceSource.getConstArray();
                const com::sun::star::awt::Point* pArrayEnd = pArray + rPointSequenceSource.getLength();

                for(;pArray != pArrayEnd; pArray++)
                {
                    aRetval.append(B2DPoint(pArray->X, pArray->Y));
                }

                if(bCheckClosed)
                {
                    
                    tools::checkClosed(aRetval);
                }
            }

            return aRetval;
        }

        void B2DPolygonToUnoPointSequence(
            const B2DPolygon& rPolygon,
            com::sun::star::drawing::PointSequence& rPointSequenceRetval)
        {
            B2DPolygon aPolygon(rPolygon);

            if(aPolygon.areControlPointsUsed())
            {
                OSL_ENSURE(false, "B2DPolygonToUnoPointSequence: Source contains bezier segments, wrong UNO API data type may be used (!)");
                aPolygon = aPolygon.getDefaultAdaptiveSubdivision();
            }

            const sal_uInt32 nPointCount(aPolygon.count());

            if(nPointCount)
            {
                
                
                
                const bool bIsClosed(aPolygon.isClosed());

                rPointSequenceRetval.realloc(bIsClosed ? nPointCount + 1 : nPointCount);
                com::sun::star::awt::Point* pSequence = rPointSequenceRetval.getArray();

                for(sal_uInt32 b(0); b < nPointCount; b++)
                {
                    const B2DPoint aPoint(aPolygon.getB2DPoint(b));
                    const com::sun::star::awt::Point aAPIPoint(fround(aPoint.getX()), fround(aPoint.getY()));

                    *pSequence = aAPIPoint;
                    pSequence++;
                }

                
                if(bIsClosed)
                {
                    *pSequence = *rPointSequenceRetval.getArray();
                }
            }
            else
            {
                rPointSequenceRetval.realloc(0);
            }
        }

        
        
        

        B2DPolygon UnoPolygonBezierCoordsToB2DPolygon(
            const com::sun::star::drawing::PointSequence& rPointSequenceSource,
            const com::sun::star::drawing::FlagSequence& rFlagSequenceSource,
            bool bCheckClosed)
        {
            const sal_uInt32 nCount((sal_uInt32)rPointSequenceSource.getLength());
            OSL_ENSURE(nCount == (sal_uInt32)rFlagSequenceSource.getLength(),
                "UnoPolygonBezierCoordsToB2DPolygon: Unequal count of Points and Flags (!)");

            
            B2DPolygon aRetval;
            const com::sun::star::awt::Point* pPointSequence = rPointSequenceSource.getConstArray();
            const com::sun::star::drawing::PolygonFlags* pFlagSequence = rFlagSequenceSource.getConstArray();

            
            B2DPoint aNewCoordinatePair(pPointSequence->X, pPointSequence->Y); pPointSequence++;
            com::sun::star::drawing::PolygonFlags ePolygonFlag(*pFlagSequence); pFlagSequence++;
            B2DPoint aControlA;
            B2DPoint aControlB;

            
            OSL_ENSURE(com::sun::star::drawing::PolygonFlags_CONTROL != ePolygonFlag,
                "UnoPolygonBezierCoordsToB2DPolygon: Start point is a control point, illegal input polygon (!)");

            
            aRetval.append(aNewCoordinatePair);

            for(sal_uInt32 b(1); b < nCount;)
            {
                
                bool bControlA(false);
                bool bControlB(false);

                
                aNewCoordinatePair = B2DPoint(pPointSequence->X, pPointSequence->Y);
                ePolygonFlag = *pFlagSequence;
                pPointSequence++; pFlagSequence++; b++;

                if(b < nCount && com::sun::star::drawing::PolygonFlags_CONTROL == ePolygonFlag)
                {
                    aControlA = aNewCoordinatePair;
                    bControlA = true;

                    
                    aNewCoordinatePair = B2DPoint(pPointSequence->X, pPointSequence->Y);
                    ePolygonFlag = *pFlagSequence;
                    pPointSequence++; pFlagSequence++; b++;
                }

                if(b < nCount && com::sun::star::drawing::PolygonFlags_CONTROL == ePolygonFlag)
                {
                    aControlB = aNewCoordinatePair;
                    bControlB = true;

                    
                    aNewCoordinatePair = B2DPoint(pPointSequence->X, pPointSequence->Y);
                    ePolygonFlag = *pFlagSequence;
                    pPointSequence++; pFlagSequence++; b++;
                }

                
                
                OSL_ENSURE(com::sun::star::drawing::PolygonFlags_CONTROL != ePolygonFlag && bControlA == bControlB,
                    "UnoPolygonBezierCoordsToB2DPolygon: Illegal source polygon (!)");

                
                
                
                
                
                
                
                
                if(bControlA
                    && aControlA.equal(aControlB)
                    && aControlA.equal(aRetval.getB2DPoint(aRetval.count() - 1)))
                {
                    bControlA = bControlB = false;
                }

                if(bControlA)
                {
                    
                    aRetval.appendBezierSegment(aControlA, aControlB, aNewCoordinatePair);
                }
                else
                {
                    
                    aRetval.append(aNewCoordinatePair);
                }
            }

            
            
            if(bCheckClosed)
            {
                checkClosed(aRetval);
            }

            return aRetval;
        }

        void B2DPolygonToUnoPolygonBezierCoords(
            const B2DPolygon& rPolygon,
            com::sun::star::drawing::PointSequence& rPointSequenceRetval,
            com::sun::star::drawing::FlagSequence& rFlagSequenceRetval)
        {
            const sal_uInt32 nPointCount(rPolygon.count());

            if(nPointCount)
            {
                const bool bCurve(rPolygon.areControlPointsUsed());
                const bool bClosed(rPolygon.isClosed());

                if(bCurve)
                {
                    
                    const sal_uInt32 nLoopCount(bClosed ? nPointCount : nPointCount - 1);

                    if(nLoopCount)
                    {
                        
                        
                        std::vector< com::sun::star::awt::Point > aCollectPoints;
                        std::vector< com::sun::star::drawing::PolygonFlags > aCollectFlags;

                        
                        const sal_uInt32 nMaxTargetCount((nLoopCount * 3) + 1);
                        aCollectPoints.reserve(nMaxTargetCount);
                        aCollectFlags.reserve(nMaxTargetCount);

                        
                        B2DCubicBezier aBezierSegment;
                        aBezierSegment.setStartPoint(rPolygon.getB2DPoint(0));

                        for(sal_uInt32 a(0); a < nLoopCount; a++)
                        {
                            
                            const sal_uInt32 nStartPointIndex(aCollectPoints.size());
                            aCollectPoints.push_back(
                                com::sun::star::awt::Point(
                                    fround(aBezierSegment.getStartPoint().getX()),
                                    fround(aBezierSegment.getStartPoint().getY())));
                            aCollectFlags.push_back(com::sun::star::drawing::PolygonFlags_NORMAL);

                            
                            const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                            aBezierSegment.setEndPoint(rPolygon.getB2DPoint(nNextIndex));
                            aBezierSegment.setControlPointA(rPolygon.getNextControlPoint(a));
                            aBezierSegment.setControlPointB(rPolygon.getPrevControlPoint(nNextIndex));

                            if(aBezierSegment.isBezier())
                            {
                                
                                aCollectPoints.push_back(
                                    com::sun::star::awt::Point(
                                        fround(aBezierSegment.getControlPointA().getX()),
                                        fround(aBezierSegment.getControlPointA().getY())));
                                aCollectFlags.push_back(com::sun::star::drawing::PolygonFlags_CONTROL);

                                aCollectPoints.push_back(
                                    com::sun::star::awt::Point(
                                        fround(aBezierSegment.getControlPointB().getX()),
                                        fround(aBezierSegment.getControlPointB().getY())));
                                aCollectFlags.push_back(com::sun::star::drawing::PolygonFlags_CONTROL);
                            }

                            
                            if(aBezierSegment.getControlPointA() != aBezierSegment.getStartPoint() && (bClosed || a))
                            {
                                const B2VectorContinuity eCont(rPolygon.getContinuityInPoint(a));

                                if(CONTINUITY_C1 == eCont)
                                {
                                    aCollectFlags[nStartPointIndex] = com::sun::star::drawing::PolygonFlags_SMOOTH;
                                }
                                else if(CONTINUITY_C2 == eCont)
                                {
                                    aCollectFlags[nStartPointIndex] = com::sun::star::drawing::PolygonFlags_SYMMETRIC;
                                }
                            }

                            
                            aBezierSegment.setStartPoint(aBezierSegment.getEndPoint());
                        }

                        if(bClosed)
                        {
                            
                            aCollectPoints.push_back(aCollectPoints[0]);
                            aCollectFlags.push_back(com::sun::star::drawing::PolygonFlags_NORMAL);
                        }
                        else
                        {
                            
                            const B2DPoint aClosingPoint(rPolygon.getB2DPoint(nPointCount - 1L));
                            aCollectPoints.push_back(
                                com::sun::star::awt::Point(
                                    fround(aClosingPoint.getX()),
                                    fround(aClosingPoint.getY())));
                            aCollectFlags.push_back(com::sun::star::drawing::PolygonFlags_NORMAL);
                        }

                        
                        const sal_uInt32 nTargetCount(aCollectPoints.size());
                        OSL_ENSURE(nTargetCount == aCollectFlags.size(), "Unequal Point and Flag count (!)");

                        rPointSequenceRetval.realloc((sal_Int32)nTargetCount);
                        rFlagSequenceRetval.realloc((sal_Int32)nTargetCount);
                        com::sun::star::awt::Point* pPointSequence = rPointSequenceRetval.getArray();
                        com::sun::star::drawing::PolygonFlags* pFlagSequence = rFlagSequenceRetval.getArray();

                        for(sal_uInt32 a(0); a < nTargetCount; a++)
                        {
                            *pPointSequence = aCollectPoints[a];
                            *pFlagSequence = aCollectFlags[a];
                            pPointSequence++;
                            pFlagSequence++;
                        }
                    }
                }
                else
                {
                    
                    const sal_uInt32 nTargetCount(nPointCount + (bClosed ? 1 : 0));

                    rPointSequenceRetval.realloc((sal_Int32)nTargetCount);
                    rFlagSequenceRetval.realloc((sal_Int32)nTargetCount);

                    com::sun::star::awt::Point* pPointSequence = rPointSequenceRetval.getArray();
                    com::sun::star::drawing::PolygonFlags* pFlagSequence = rFlagSequenceRetval.getArray();

                    for(sal_uInt32 a(0); a < nPointCount; a++)
                    {
                        const B2DPoint aB2DPoint(rPolygon.getB2DPoint(a));
                        const com::sun::star::awt::Point aAPIPoint(
                            fround(aB2DPoint.getX()),
                            fround(aB2DPoint.getY()));

                        *pPointSequence = aAPIPoint;
                        *pFlagSequence = com::sun::star::drawing::PolygonFlags_NORMAL;
                        pPointSequence++;
                        pFlagSequence++;
                    }

                    if(bClosed)
                    {
                        
                        *pPointSequence = *rPointSequenceRetval.getConstArray();
                        *pFlagSequence = com::sun::star::drawing::PolygonFlags_NORMAL;
                    }
                }
            }
            else
            {
                rPointSequenceRetval.realloc(0);
                rFlagSequenceRetval.realloc(0);
            }
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
