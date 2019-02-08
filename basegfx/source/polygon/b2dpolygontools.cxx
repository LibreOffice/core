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
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dbeziertools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <numeric>

// #i37443#
#define ANGLE_BOUND_START_VALUE     (2.25)
#define ANGLE_BOUND_MINIMUM_VALUE   (0.1)
#define STEPSPERQUARTER     (3)

namespace basegfx
{
    namespace utils
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
            // #i80172# Removed unnecessary assertion
            // OSL_ENSURE(!rCandidate.isClosed(), "checkClosed: already closed (!)");

            if(rCandidate.count() > 1 && rCandidate.getB2DPoint(0) == rCandidate.getB2DPoint(rCandidate.count() - 1))
            {
                closeWithGeometryChange(rCandidate);
            }
        }

        // Get successor and predecessor indices. Returning the same index means there
        // is none. Same for successor.
        sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(nIndex)
            {
                return nIndex - 1;
            }
            else if(rCandidate.count())
            {
                return rCandidate.count() - 1;
            }
            else
            {
                return nIndex;
            }
        }

        sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const B2DPolygon& rCandidate)
        {
            OSL_ENSURE(nIndex < rCandidate.count(), "getIndexOfPredecessor: Access to polygon out of range (!)");

            if(nIndex + 1 < rCandidate.count())
            {
                return nIndex + 1;
            }
            else if(nIndex + 1 == rCandidate.count())
            {
                return 0;
            }
            else
            {
                return nIndex;
            }
        }

        B2VectorOrientation getOrientation(const B2DPolygon& rCandidate)
        {
            B2VectorOrientation eRetval(B2VectorOrientation::Neutral);

            if(rCandidate.count() > 2 || rCandidate.areControlPointsUsed())
            {
                const double fSignedArea(getSignedArea(rCandidate));

                if(fTools::equalZero(fSignedArea))
                {
                    // B2VectorOrientation::Neutral, already set
                }
                if(fSignedArea > 0.0)
                {
                    eRetval = B2VectorOrientation::Positive;
                }
                else if(fSignedArea < 0.0)
                {
                    eRetval = B2VectorOrientation::Negative;
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
                    // prepare edge-oriented loop
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                    B2DCubicBezier aBezier;
                    aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                    // perf: try to avoid too many realloctions by guessing the result's pointcount
                    aRetval.reserve(nPointCount*4);

                    // add start point (always)
                    aRetval.append(aBezier.getStartPoint());

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        // get next and control points
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        aBezier.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                        aBezier.setControlPointA(rCandidate.getNextControlPoint(a));
                        aBezier.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                        aBezier.testAndSolveTrivialBezier();

                        if(aBezier.isBezier())
                        {
                            // add curved edge and generate DistanceBound
                            double fBound(0.0);

                            if(fDistanceBound == 0.0)
                            {
                                // If not set, use B2DCubicBezier functionality to guess a rough value
                                const double fRoughLength((aBezier.getEdgeLength() + aBezier.getControlPolygonLength()) / 2.0);

                                // take 1/100th of the rough curve length
                                fBound = fRoughLength * 0.01;
                            }
                            else
                            {
                                // use given bound value
                                fBound = fDistanceBound;
                            }

                            // make sure bound value is not too small. The base units are 1/100th mm, thus
                            // just make sure it's not smaller then 1/100th of that
                            if(fBound < 0.01)
                            {
                                fBound = 0.01;
                            }

                            // call adaptive subdivide which adds edges to aRetval accordingly
                            aBezier.adaptiveSubdivideByDistance(aRetval, fBound);
                        }
                        else
                        {
                            // add non-curved edge
                            aRetval.append(aBezier.getEndPoint());
                        }

                        // prepare next step
                        aBezier.setStartPoint(aBezier.getEndPoint());
                    }

                    if(rCandidate.isClosed())
                    {
                        // set closed flag and correct last point (which is added double now).
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
                    // prepare edge-oriented loop
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                    B2DCubicBezier aBezier;
                    aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                    // perf: try to avoid too many realloctions by guessing the result's pointcount
                    aRetval.reserve(nPointCount*4);

                    // add start point (always)
                    aRetval.append(aBezier.getStartPoint());

                    // #i37443# prepare convenient AngleBound if none was given
                    if(fAngleBound == 0.0)
                    {
                        fAngleBound = ANGLE_BOUND_START_VALUE;
                    }
                    else if(fTools::less(fAngleBound, ANGLE_BOUND_MINIMUM_VALUE))
                    {
                        fAngleBound = 0.1;
                    }

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        // get next and control points
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        aBezier.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                        aBezier.setControlPointA(rCandidate.getNextControlPoint(a));
                        aBezier.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                        aBezier.testAndSolveTrivialBezier();

                        if(aBezier.isBezier())
                        {
                            // call adaptive subdivide
                            aBezier.adaptiveSubdivideByAngle(aRetval, fAngleBound);
                        }
                        else
                        {
                            // add non-curved edge
                            aRetval.append(aBezier.getEndPoint());
                        }

                        // prepare next step
                        aBezier.setStartPoint(aBezier.getEndPoint());
                    }

                    if(rCandidate.isClosed())
                    {
                        // set closed flag and correct last point (which is added double now).
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

            if(bWithBorder && isPointOnPolygon(aCandidate, rPoint))
            {
                return true;
            }
            else
            {
                bool bRetval(false);
                const sal_uInt32 nPointCount(aCandidate.count());

                if(nPointCount)
                {
                    B2DPoint aCurrentPoint(aCandidate.getB2DPoint(nPointCount - 1));

                    for(sal_uInt32 a(0); a < nPointCount; a++)
                    {
                        const B2DPoint aPreviousPoint(aCurrentPoint);
                        aCurrentPoint = aCandidate.getB2DPoint(a);

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

                return bRetval;
            }
        }

        bool isInside(const B2DPolygon& rCandidate, const B2DPolygon& rPolygon, bool bWithBorder)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? rCandidate.getDefaultAdaptiveSubdivision() : rCandidate);
            const B2DPolygon aPolygon(rPolygon.areControlPointsUsed() ? rPolygon.getDefaultAdaptiveSubdivision() : rPolygon);
            const sal_uInt32 nPointCount(aPolygon.count());

            for(sal_uInt32 a(0); a < nPointCount; a++)
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
            // changed to use internally buffered version at B2DPolygon
            return rCandidate.getB2DRange();
        }

        double getSignedArea(const B2DPolygon& rCandidate)
        {
            const B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? rCandidate.getDefaultAdaptiveSubdivision() : rCandidate);
            double fRetval(0.0);
            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount > 2)
            {
                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    const B2DPoint aPreviousPoint(aCandidate.getB2DPoint((!a) ? nPointCount - 1 : a - 1));
                    const B2DPoint aCurrentPoint(aCandidate.getB2DPoint(a));

                    fRetval += aPreviousPoint.getX() * aCurrentPoint.getY();
                    fRetval -= aPreviousPoint.getY() * aCurrentPoint.getX();
                }

                // correct to zero if small enough. Also test the quadratic
                // of the result since the precision is near quadratic due to
                // the algorithm
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
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);

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

            if( nPointCount == 1 )
            {
                // only one point (i.e. no edge) - simply take that point
                aRetval = rCandidate.getB2DPoint(0);
            }
            else if(nPointCount > 1)
            {
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                sal_uInt32 nIndex(0);
                bool bIndexDone(false);

                // get length if not given
                if(fTools::equalZero(fLength))
                {
                    fLength = getLength(rCandidate);
                }

                if(fTools::less(fDistance, 0.0))
                {
                    // handle fDistance < 0.0
                    if(rCandidate.isClosed())
                    {
                        // if fDistance < 0.0 increment with multiple of fLength
                        sal_uInt32 nCount(sal_uInt32(-fDistance / fLength));
                        fDistance += double(nCount + 1) * fLength;
                    }
                    else
                    {
                        // crop to polygon start
                        fDistance = 0.0;
                        bIndexDone = true;
                    }
                }
                else if(fTools::moreOrEqual(fDistance, fLength))
                {
                    // handle fDistance >= fLength
                    if(rCandidate.isClosed())
                    {
                        // if fDistance >= fLength decrement with multiple of fLength
                        sal_uInt32 nCount(sal_uInt32(fDistance / fLength));
                        fDistance -= static_cast<double>(nCount) * fLength;
                    }
                    else
                    {
                        // crop to polygon end
                        fDistance = 0.0;
                        nIndex = nEdgeCount;
                        bIndexDone = true;
                    }
                }

                // look for correct index. fDistance is now [0.0 .. fLength[
                double fEdgeLength(getEdgeLength(rCandidate, nIndex));

                while(!bIndexDone)
                {
                    // edge found must be on the half-open range
                    // [0,fEdgeLength).
                    // Note that in theory, we cannot move beyond
                    // the last polygon point, since fDistance>=fLength
                    // is checked above. Unfortunately, with floating-
                    // point calculations, this case might happen.
                    // Handled by nIndex check below
                    if (nIndex+1 < nEdgeCount && fTools::moreOrEqual(fDistance, fEdgeLength))
                    {
                        // go to next edge
                        fDistance -= fEdgeLength;
                        fEdgeLength = getEdgeLength(rCandidate, ++nIndex);
                    }
                    else
                    {
                        // it's on this edge, stop
                        bIndexDone = true;
                    }
                }

                // get the point using nIndex
                aRetval = rCandidate.getB2DPoint(nIndex);

                // if fDistance != 0.0, move that length on the edge. The edge
                // length is in fEdgeLength.
                if(!fTools::equalZero(fDistance))
                {
                    if(fTools::moreOrEqual(fDistance, fEdgeLength))
                    {
                        // end point of chosen edge
                        const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                        aRetval = rCandidate.getB2DPoint(nNextIndex);
                    }
                    else if(fTools::equalZero(fDistance))
                    {
                        // start point of chosen edge
                    }
                    else
                    {
                        // inside edge
                        const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                        const B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                        bool bDone(false);

                        // add calculated average value to the return value
                        if(rCandidate.areControlPointsUsed())
                        {
                            // get as bezier segment
                            const B2DCubicBezier aBezierSegment(
                                aRetval, rCandidate.getNextControlPoint(nIndex),
                                rCandidate.getPrevControlPoint(nNextIndex), aNextPoint);

                            if(aBezierSegment.isBezier())
                            {
                                // use B2DCubicBezierHelper to bridge the non-linear gap between
                                // length and bezier distances
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
            // get length if not given
            if(fTools::equalZero(fLength))
            {
                fLength = getLength(rCandidate);
            }

            // multiply fDistance with real length to get absolute position and
            // use getPositionAbsolute
            return getPositionAbsolute(rCandidate, fDistance * fLength, fLength);
        }

        B2DPolygon getSnippetAbsolute(const B2DPolygon& rCandidate, double fFrom, double fTo, double fLength)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                // get length if not given
                if(fTools::equalZero(fLength))
                {
                    fLength = getLength(rCandidate);
                }

                // test and correct fFrom
                if(fTools::less(fFrom, 0.0))
                {
                    fFrom = 0.0;
                }

                // test and correct fTo
                if(fTools::more(fTo, fLength))
                {
                    fTo = fLength;
                }

                // test and correct relationship of fFrom, fTo
                if(fTools::more(fFrom, fTo))
                {
                    fFrom = fTo = (fFrom + fTo) / 2.0;
                }

                if(fTools::equalZero(fFrom) && fTools::equal(fTo, fLength))
                {
                    // no change, result is the whole polygon
                    return rCandidate;
                }
                else
                {
                    B2DPolygon aRetval;
                    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                    double fPositionOfStart(0.0);
                    bool bStartDone(false);
                    bool bEndDone(false);

                    for(sal_uInt32 a(0); !(bStartDone && bEndDone) && a < nEdgeCount; a++)
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
                                // calculate and add start point
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
                                            // use B2DCubicBezierHelper to bridge the non-linear gap between
                                            // length and bezier distances
                                            const B2DCubicBezierHelper aBezierSegmentHelper(aBezierSegment);
                                            const double fBezierDistance(aBezierSegmentHelper.distanceToRelative(fFrom - fPositionOfStart));
                                            B2DCubicBezier aRight;

                                            aBezierSegment.split(fBezierDistance, nullptr, &aRight);
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

                                // if same point, end is done, too.
                                if(rtl::math::approxEqual(fFrom, fTo))
                                {
                                    bEndDone = true;
                                }
                            }
                        }

                        if(!bEndDone && fTools::moreOrEqual(fTo, fPositionOfStart) && fTools::less(fTo, fPositionOfStart + fEdgeLength))
                        {
                            // calculate and add end point
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
                                        // use B2DCubicBezierHelper to bridge the non-linear gap between
                                        // length and bezier distances
                                        const B2DCubicBezierHelper aBezierSegmentHelper(aBezierSegment);
                                        const double fBezierDistance(aBezierSegmentHelper.distanceToRelative(fTo - fPositionOfStart));
                                        B2DCubicBezier aLeft;

                                        aBezierSegment.split(fBezierDistance, &aLeft, nullptr);
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
                                // add segments end point
                                const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                                aRetval.append(rCandidate.getB2DPoint(nNextIndex));

                                if(rCandidate.areControlPointsUsed())
                                {
                                    aRetval.setPrevControlPoint(aRetval.count() - 1, rCandidate.getPrevControlPoint(nNextIndex));
                                    aRetval.setNextControlPoint(aRetval.count() - 1, rCandidate.getNextControlPoint(nNextIndex));
                                }
                            }

                            // increment fPositionOfStart
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
            CutFlagValue aRetval(CutFlagValue::NONE);
            double fCut1(0.0);
            double fCut2(0.0);
            bool bFinished(!static_cast<bool>(aCutFlags & CutFlagValue::ALL));

            // test for same points?
            if(!bFinished
                && (aCutFlags & (CutFlagValue::START1|CutFlagValue::END1))
                && (aCutFlags & (CutFlagValue::START2|CutFlagValue::END2)))
            {
                // same startpoint?
                if((aCutFlags & (CutFlagValue::START1|CutFlagValue::START2)) == (CutFlagValue::START1|CutFlagValue::START2))
                {
                    if(rEdge1Start.equal(rEdge2Start))
                    {
                        bFinished = true;
                        aRetval = (CutFlagValue::START1|CutFlagValue::START2);
                    }
                }

                // same endpoint?
                if(!bFinished && (aCutFlags & (CutFlagValue::END1|CutFlagValue::END2)) == (CutFlagValue::END1|CutFlagValue::END2))
                {
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(aEnd1.equal(aEnd2))
                    {
                        bFinished = true;
                        aRetval = (CutFlagValue::END1|CutFlagValue::END2);
                        fCut1 = fCut2 = 1.0;
                    }
                }

                // startpoint1 == endpoint2?
                if(!bFinished && (aCutFlags & (CutFlagValue::START1|CutFlagValue::END2)) == (CutFlagValue::START1|CutFlagValue::END2))
                {
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(rEdge1Start.equal(aEnd2))
                    {
                        bFinished = true;
                        aRetval = (CutFlagValue::START1|CutFlagValue::END2);
                        fCut1 = 0.0;
                        fCut2 = 1.0;
                    }
                }

                // startpoint2 == endpoint1?
                if(!bFinished&& (aCutFlags & (CutFlagValue::START2|CutFlagValue::END1)) == (CutFlagValue::START2|CutFlagValue::END1))
                {
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);

                    if(rEdge2Start.equal(aEnd1))
                    {
                        bFinished = true;
                        aRetval = (CutFlagValue::START2|CutFlagValue::END1);
                        fCut1 = 1.0;
                        fCut2 = 0.0;
                    }
                }
            }

            if(!bFinished && (aCutFlags & CutFlagValue::LINE))
            {
                if(aCutFlags & CutFlagValue::START1)
                {
                    // start1 on line 2 ?
                    if(isPointOnEdge(rEdge1Start, rEdge2Start, rEdge2Delta, &fCut2))
                    {
                        bFinished = true;
                        aRetval = (CutFlagValue::LINE|CutFlagValue::START1);
                    }
                }

                if(!bFinished && (aCutFlags & CutFlagValue::START2))
                {
                    // start2 on line 1 ?
                    if(isPointOnEdge(rEdge2Start, rEdge1Start, rEdge1Delta, &fCut1))
                    {
                        bFinished = true;
                        aRetval = (CutFlagValue::LINE|CutFlagValue::START2);
                    }
                }

                if(!bFinished && (aCutFlags & CutFlagValue::END1))
                {
                    // end1 on line 2 ?
                    const B2DPoint aEnd1(rEdge1Start + rEdge1Delta);

                    if(isPointOnEdge(aEnd1, rEdge2Start, rEdge2Delta, &fCut2))
                    {
                        bFinished = true;
                        aRetval = (CutFlagValue::LINE|CutFlagValue::END1);
                    }
                }

                if(!bFinished && (aCutFlags & CutFlagValue::END2))
                {
                    // end2 on line 1 ?
                    const B2DPoint aEnd2(rEdge2Start + rEdge2Delta);

                    if(isPointOnEdge(aEnd2, rEdge1Start, rEdge1Delta, &fCut1))
                    {
                        bFinished = true;
                        aRetval = (CutFlagValue::LINE|CutFlagValue::END2);
                    }
                }

                if(!bFinished)
                {
                    // cut in line1, line2 ?
                    fCut1 = (rEdge1Delta.getX() * rEdge2Delta.getY()) - (rEdge1Delta.getY() * rEdge2Delta.getX());

                    if(!fTools::equalZero(fCut1))
                    {
                        fCut1 = (rEdge2Delta.getY() * (rEdge2Start.getX() - rEdge1Start.getX())
                            + rEdge2Delta.getX() * (rEdge1Start.getY() - rEdge2Start.getY())) / fCut1;

                        const double fZero(0.0);
                        const double fOne(1.0);

                        // inside parameter range edge1 AND fCut2 is calculable
                        if(fTools::more(fCut1, fZero) && fTools::less(fCut1, fOne)
                            && (!fTools::equalZero(rEdge2Delta.getX()) || !fTools::equalZero(rEdge2Delta.getY())))
                        {
                            // take the more precise calculation of the two possible
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

                            // inside parameter range edge2, too
                            if(fTools::more(fCut2, fZero) && fTools::less(fCut2, fOne))
                            {
                                aRetval = CutFlagValue::LINE;
                            }
                        }
                    }
                }
            }

            // copy values if wanted
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
                // no line, just a point
                return false;
            }
            else if(bDeltaXIsZero)
            {
                // vertical line
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
                // horizontal line
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
                // any angle line
                double fTOne = (rPoint.getX() - rEdgeStart.getX()) / rEdgeDelta.getX();
                double fTTwo = (rPoint.getY() - rEdgeStart.getY()) / rEdgeDelta.getY();

                if(fTools::equal(fTOne, fTTwo))
                {
                    // same parameter representation, point is on line. Take
                    // middle value for better results
                    double fValue = (fTOne + fTTwo) / 2.0;

                    if(fTools::more(fValue, fZero) && fTools::less(fValue, fOne))
                    {
                        // point is inside line bounds, too
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

        void applyLineDashing(const B2DPolygon& rCandidate, const std::vector<double>& rDotDashArray, B2DPolyPolygon* pLineTarget, B2DPolyPolygon* pGapTarget, double fDotDashLength)
        {
            const sal_uInt32 nPointCount(rCandidate.count());
            const sal_uInt32 nDotDashCount(rDotDashArray.size());

            if(fTools::lessOrEqual(fDotDashLength, 0.0))
            {
                fDotDashLength = std::accumulate(rDotDashArray.begin(), rDotDashArray.end(), 0.0);
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
                B2DCubicBezier aCurrentEdge;
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                aCurrentEdge.setStartPoint(rCandidate.getB2DPoint(0));

                // prepare DotDashArray iteration and the line/gap switching bool
                sal_uInt32 nDotDashIndex(0);
                bool bIsLine(true);
                double fDotDashMovingLength(rDotDashArray[0]);
                B2DPolygon aSnippet;

                // iterate over all edges
                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // update current edge (fill in C1, C2 and end point)
                    double fLastDotDashMovingLength(0.0);
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    aCurrentEdge.setControlPointA(rCandidate.getNextControlPoint(a));
                    aCurrentEdge.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                    aCurrentEdge.setEndPoint(rCandidate.getB2DPoint(nNextIndex));

                    // check if we have a trivial bezier segment -> possible fallback to edge
                    aCurrentEdge.testAndSolveTrivialBezier();

                    if(aCurrentEdge.isBezier())
                    {
                        // bezier segment
                        const B2DCubicBezierHelper aCubicBezierHelper(aCurrentEdge);
                        const double fEdgeLength(aCubicBezierHelper.getLength());

                        if(!fTools::equalZero(fEdgeLength))
                        {
                            while(fTools::less(fDotDashMovingLength, fEdgeLength))
                            {
                                // new split is inside edge, create and append snippet [fLastDotDashMovingLength, fDotDashMovingLength]
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

                                // prepare next DotDashArray step and flip line/gap flag
                                fLastDotDashMovingLength = fDotDashMovingLength;
                                fDotDashMovingLength += rDotDashArray[(++nDotDashIndex) % nDotDashCount];
                                bIsLine = !bIsLine;
                            }

                            // append closing snippet [fLastDotDashMovingLength, fEdgeLength]
                            const bool bHandleLine(bIsLine && pLineTarget);
                            const bool bHandleGap(!bIsLine && pGapTarget);

                            if(bHandleLine || bHandleGap)
                            {
                                B2DCubicBezier aRight;
                                const double fBezierSplit(aCubicBezierHelper.distanceToRelative(fLastDotDashMovingLength));

                                aCurrentEdge.split(fBezierSplit, nullptr, &aRight);

                                if(!aSnippet.count())
                                {
                                    aSnippet.append(aRight.getStartPoint());
                                }

                                aSnippet.appendBezierSegment(aRight.getControlPointA(), aRight.getControlPointB(), aRight.getEndPoint());
                            }

                            // prepare move to next edge
                            fDotDashMovingLength -= fEdgeLength;
                        }
                    }
                    else
                    {
                        // simple edge
                        const double fEdgeLength(aCurrentEdge.getEdgeLength());

                        if(!fTools::equalZero(fEdgeLength))
                        {
                            while(fTools::less(fDotDashMovingLength, fEdgeLength))
                            {
                                // new split is inside edge, create and append snippet [fLastDotDashMovingLength, fDotDashMovingLength]
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
                                    aSnippet.append(interpolate(aCurrentEdge.getStartPoint(), aCurrentEdge.getEndPoint(), fLastDotDashMovingLength / fEdgeLength));
                                }

                                aSnippet.append(aCurrentEdge.getEndPoint());
                            }

                            // prepare move to next edge
                            fDotDashMovingLength -= fEdgeLength;
                        }
                    }

                    // prepare next edge step (end point gets new start point)
                    aCurrentEdge.setStartPoint(aCurrentEdge.getEndPoint());
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
                        // thus direct point access below is allowed
                        const B2DPolygon aFirst(pLineTarget->getB2DPolygon(0));
                        B2DPolygon aLast(pLineTarget->getB2DPolygon(nCount - 1));

                        if(aFirst.getB2DPoint(0).equal(aLast.getB2DPoint(aLast.count() - 1)))
                        {
                            // start of first and end of last are the same -> merge them
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
                        // these polygons were created above, there exists none with less than two points,
                        // thus direct point access below is allowed
                        const B2DPolygon aFirst(pGapTarget->getB2DPolygon(0));
                        B2DPolygon aLast(pGapTarget->getB2DPolygon(nCount - 1));

                        if(aFirst.getB2DPoint(0).equal(aLast.getB2DPoint(aLast.count() - 1)))
                        {
                            // start of first and end of last are the same -> merge them
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

        // test if point is inside epsilon-range around an edge defined
        // by the two given points. Can be used for HitTesting. The epsilon-range
        // is defined to be the rectangle centered to the given edge, using height
        // 2 x fDistance, and the circle around both points with radius fDistance.
        bool isInEpsilonRange(const B2DPoint& rEdgeStart, const B2DPoint& rEdgeEnd, const B2DPoint& rTestPosition, double fDistance)
        {
            // build edge vector
            const B2DVector aEdge(rEdgeEnd - rEdgeStart);
            bool bDoDistanceTestStart(false);
            bool bDoDistanceTestEnd(false);

            if(aEdge.equalZero())
            {
                // no edge, just a point. Do one of the distance tests.
                bDoDistanceTestStart = true;
            }
            else
            {
                // edge has a length. Create perpendicular vector.
                const B2DVector aPerpend(getPerpendicular(aEdge));
                double fCut(
                    (aPerpend.getY() * (rTestPosition.getX() - rEdgeStart.getX())
                    + aPerpend.getX() * (rEdgeStart.getY() - rTestPosition.getY())) /
                    (aEdge.getX() * aEdge.getX() + aEdge.getY() * aEdge.getY()));
                const double fZero(0.0);
                const double fOne(1.0);

                if(fTools::less(fCut, fZero))
                {
                    // left of rEdgeStart
                    bDoDistanceTestStart = true;
                }
                else if(fTools::more(fCut, fOne))
                {
                    // right of rEdgeEnd
                    bDoDistanceTestEnd = true;
                }
                else
                {
                    // inside line [0.0 .. 1.0]
                    const B2DPoint aCutPoint(interpolate(rEdgeStart, rEdgeEnd, fCut));
                    const B2DVector aDelta(rTestPosition - aCutPoint);
                    const double fDistanceSquare(aDelta.scalar(aDelta));

                    return fDistanceSquare <= fDistance * fDistance;
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

        // test if point is inside epsilon-range around the given Polygon. Can be used
        // for HitTesting. The epsilon-range is defined to be the tube around the polygon
        // with distance fDistance and rounded edges (start and end point).
        bool isInEpsilonRange(const B2DPolygon& rCandidate, const B2DPoint& rTestPosition, double fDistance)
        {
            // force to non-bezier polygon
            const B2DPolygon& aCandidate(rCandidate.getDefaultAdaptiveSubdivision());
            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1);
                B2DPoint aCurrent(aCandidate.getB2DPoint(0));

                if(nEdgeCount)
                {
                    // edges
                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        const B2DPoint aNext(aCandidate.getB2DPoint(nNextIndex));

                        if(isInEpsilonRange(aCurrent, aNext, rTestPosition, fDistance))
                        {
                            return true;
                        }

                        // prepare next step
                        aCurrent = aNext;
                    }
                }
                else
                {
                    // no edges, but points -> not closed. Check single point. Just
                    // use isInEpsilonRange with twice the same point, it handles those well
                    if(isInEpsilonRange(aCurrent, aCurrent, rTestPosition, fDistance))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        // Calculates distance of curve point to its control point for a Bézier curve, that
        // approximates a unit circle arc. fAngle is the center angle of the circle arc. The
        // constrain 0<=fAngle<=pi/2 must not be violated to give a useful accuracy. For details
        // and alternatives read document "ApproxCircleInfo.odt", attachment of bug tdf#121425.
        static double impDistanceBezierPointToControl(double fAngle)
        {
            SAL_WARN_IF(fAngle < 0 || fAngle > F_PI2,"basegfx","angle not suitable for approximate circle");
            if (0 <= fAngle && fAngle <= F_PI2)
            {
                return 4.0/3.0 * ( tan(fAngle/4.0));
            }
            else
                return 0;
        }

        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect, double fRadiusX, double fRadiusY )
        {
            const double fZero(0.0);
            const double fOne(1.0);

            fRadiusX = std::clamp(fRadiusX, 0.0, 1.0);
            fRadiusY = std::clamp(fRadiusY, 0.0, 1.0);

            if(rtl::math::approxEqual(fZero, fRadiusX) || rtl::math::approxEqual(fZero, fRadiusY))
            {
                // at least in one direction no radius, use rectangle.
                // Do not use createPolygonFromRect() here since original
                // creator (historical reasons) still creates a start point at the
                // bottom center, so do the same here to get the same line patterns.
                // Due to this the order of points is different, too.
                const B2DPoint aBottomCenter(rRect.getCenter().getX(), rRect.getMaxY());
                B2DPolygon aPolygon {
                    aBottomCenter,
                    { rRect.getMinX(), rRect.getMaxY() },
                    { rRect.getMinX(), rRect.getMinY() },
                    { rRect.getMaxX(), rRect.getMinY() },
                    { rRect.getMaxX(), rRect.getMaxY() }
                };

                // close
                aPolygon.setClosed( true );

                return aPolygon;
            }
            else if(rtl::math::approxEqual(fOne, fRadiusX) && rtl::math::approxEqual(fOne, fRadiusY))
            {
                // in both directions full radius, use ellipse
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
                const double fKappa(impDistanceBezierPointToControl(F_PI2));

                // create start point at bottom center
                if(!rtl::math::approxEqual(fOne, fRadiusX))
                {
                    const B2DPoint aBottomCenter(rRect.getCenter().getX(), rRect.getMaxY());
                    aRetval.append(aBottomCenter);
                }

                // create first bow
                {
                    const B2DPoint aBottomRight(rRect.getMaxX(), rRect.getMaxY());
                    const B2DPoint aStart(aBottomRight + B2DPoint(-fBowX, 0.0));
                    const B2DPoint aStop(aBottomRight + B2DPoint(0.0, -fBowY));
                    aRetval.append(aStart);
                    aRetval.appendBezierSegment(interpolate(aStart, aBottomRight, fKappa), interpolate(aStop, aBottomRight, fKappa), aStop);
                }

                // create second bow
                {
                    const B2DPoint aTopRight(rRect.getMaxX(), rRect.getMinY());
                    const B2DPoint aStart(aTopRight + B2DPoint(0.0, fBowY));
                    const B2DPoint aStop(aTopRight + B2DPoint(-fBowX, 0.0));
                    aRetval.append(aStart);
                    aRetval.appendBezierSegment(interpolate(aStart, aTopRight, fKappa), interpolate(aStop, aTopRight, fKappa), aStop);
                }

                // create third bow
                {
                    const B2DPoint aTopLeft(rRect.getMinX(), rRect.getMinY());
                    const B2DPoint aStart(aTopLeft + B2DPoint(fBowX, 0.0));
                    const B2DPoint aStop(aTopLeft + B2DPoint(0.0, fBowY));
                    aRetval.append(aStart);
                    aRetval.appendBezierSegment(interpolate(aStart, aTopLeft, fKappa), interpolate(aStop, aTopLeft, fKappa), aStop);
                }

                // create forth bow
                {
                    const B2DPoint aBottomLeft(rRect.getMinX(), rRect.getMaxY());
                    const B2DPoint aStart(aBottomLeft + B2DPoint(0.0, -fBowY));
                    const B2DPoint aStop(aBottomLeft + B2DPoint(fBowX, 0.0));
                    aRetval.append(aStart);
                    aRetval.appendBezierSegment(interpolate(aStart, aBottomLeft, fKappa), interpolate(aStop, aBottomLeft, fKappa), aStop);
                }

                // close
                aRetval.setClosed( true );

                // remove double created points if there are extreme radii involved
                if(rtl::math::approxEqual(fOne, fRadiusX) || rtl::math::approxEqual(fOne, fRadiusY))
                {
                    aRetval.removeDoublePoints();
                }

                return aRetval;
            }
        }

        B2DPolygon createPolygonFromRect( const B2DRectangle& rRect )
        {
            B2DPolygon aPolygon {
                { rRect.getMinX(), rRect.getMinY() },
                { rRect.getMaxX(), rRect.getMinY() },
                { rRect.getMaxX(), rRect.getMaxY() },
                { rRect.getMinX(), rRect.getMaxY() }
            };

            // close
            aPolygon.setClosed( true );

            return aPolygon;
        }

        B2DPolygon const & createUnitPolygon()
        {
            static auto const singleton = [] {
                    B2DPolygon aPolygon {
                        { 0.0, 0.0 },
                        { 1.0, 0.0 },
                        { 1.0, 1.0 },
                        { 0.0, 1.0 }
                    };

                    // close
                    aPolygon.setClosed( true );

                    return aPolygon;
            }();
            return singleton;
        }

        B2DPolygon createPolygonFromCircle( const B2DPoint& rCenter, double fRadius )
        {
            return createPolygonFromEllipse( rCenter, fRadius, fRadius );
        }

        static B2DPolygon impCreateUnitCircle(sal_uInt32 nStartQuadrant)
        {
            B2DPolygon aUnitCircle;
            const double fSegmentKappa = impDistanceBezierPointToControl(F_PI2 / STEPSPERQUARTER);
            const B2DHomMatrix aRotateMatrix(createRotateB2DHomMatrix(F_PI2 / STEPSPERQUARTER));

            B2DPoint aPoint(1.0, 0.0);
            B2DPoint aForward(1.0, fSegmentKappa);
            B2DPoint aBackward(1.0, -fSegmentKappa);

            if(nStartQuadrant != 0)
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

        B2DPolygon const & createHalfUnitCircle()
        {
            static auto const singleton = [] {
                    B2DPolygon aUnitHalfCircle;
                    const double fSegmentKappa(impDistanceBezierPointToControl(F_PI2 / STEPSPERQUARTER));
                    const B2DHomMatrix aRotateMatrix(createRotateB2DHomMatrix(F_PI2 / STEPSPERQUARTER));
                    B2DPoint aPoint(1.0, 0.0);
                    B2DPoint aForward(1.0, fSegmentKappa);
                    B2DPoint aBackward(1.0, -fSegmentKappa);

                    aUnitHalfCircle.append(aPoint);

                    for(sal_uInt32 a(0); a < STEPSPERQUARTER * 2; a++)
                    {
                        aPoint *= aRotateMatrix;
                        aBackward *= aRotateMatrix;
                        aUnitHalfCircle.appendBezierSegment(aForward, aBackward, aPoint);
                        aForward *= aRotateMatrix;
                    }
                    return aUnitHalfCircle;
                }();
            return singleton;
        }

        B2DPolygon const & createPolygonFromUnitCircle(sal_uInt32 nStartQuadrant)
        {
            switch(nStartQuadrant % 4)
            {
                case 1 :
                    {
                        static auto const singleton = impCreateUnitCircle(1);
                        return singleton;
                    }

                case 2 :
                    {
                        static auto const singleton = impCreateUnitCircle(2);
                        return singleton;
                    }

                case 3 :
                    {
                        static auto const singleton = impCreateUnitCircle(3);
                        return singleton;
                    }

                default : // case 0 :
                    {
                        static auto const singleton = impCreateUnitCircle(0);
                        return singleton;
                    }
            }
        }

        B2DPolygon createPolygonFromEllipse( const B2DPoint& rCenter, double fRadiusX, double fRadiusY, sal_uInt32 nStartQuadrant)
        {
            B2DPolygon aRetval(createPolygonFromUnitCircle(nStartQuadrant));
            const B2DHomMatrix aMatrix(createScaleTranslateB2DHomMatrix(fRadiusX, fRadiusY, rCenter.getX(), rCenter.getY()));

            aRetval.transform(aMatrix);

            return aRetval;
        }

        B2DPolygon createPolygonFromUnitEllipseSegment( double fStart, double fEnd )
        {
            B2DPolygon aRetval;

            // truncate fStart, fEnd to a range of [0.0 .. F_2PI[ where F_2PI
            // falls back to 0.0 to ensure a unique definition
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
                // same start and end angle, add single point
                aRetval.append(B2DPoint(cos(fStart), sin(fStart)));
            }
            else
            {
                const sal_uInt32 nSegments(STEPSPERQUARTER * 4);
                const double fAnglePerSegment(F_PI2 / STEPSPERQUARTER);
                const sal_uInt32 nStartSegment(sal_uInt32(fStart / fAnglePerSegment) % nSegments);
                const sal_uInt32 nEndSegment(sal_uInt32(fEnd / fAnglePerSegment) % nSegments);
                const double fSegmentKappa(impDistanceBezierPointToControl(fAnglePerSegment));

                B2DPoint aSegStart(cos(fStart), sin(fStart));
                aRetval.append(aSegStart);

                if(nStartSegment == nEndSegment && fTools::more(fEnd, fStart))
                {
                    // start and end in one sector and in the right order, create in one segment
                    const B2DPoint aSegEnd(cos(fEnd), sin(fEnd));
                    const double fFactor(impDistanceBezierPointToControl(fEnd - fStart));

                    aRetval.appendBezierSegment(
                        aSegStart + (B2DPoint(-aSegStart.getY(), aSegStart.getX()) * fFactor),
                        aSegEnd - (B2DPoint(-aSegEnd.getY(), aSegEnd.getX()) * fFactor),
                        aSegEnd);
                }
                else
                {
                    double fSegEndRad((nStartSegment + 1) * fAnglePerSegment);
                    double fFactor(impDistanceBezierPointToControl(fSegEndRad - fStart));
                    B2DPoint aSegEnd(cos(fSegEndRad), sin(fSegEndRad));

                    aRetval.appendBezierSegment(
                        aSegStart + (B2DPoint(-aSegStart.getY(), aSegStart.getX()) * fFactor),
                        aSegEnd - (B2DPoint(-aSegEnd.getY(), aSegEnd.getX()) * fFactor),
                        aSegEnd);

                    sal_uInt32 nSegment((nStartSegment + 1) % nSegments);
                    aSegStart = aSegEnd;

                    while(nSegment != nEndSegment)
                    {
                        // No end in this sector, add full sector.
                        fSegEndRad = (nSegment + 1) * fAnglePerSegment;
                        aSegEnd = B2DPoint(cos(fSegEndRad), sin(fSegEndRad));

                        aRetval.appendBezierSegment(
                            aSegStart + (B2DPoint(-aSegStart.getY(), aSegStart.getX()) * fSegmentKappa),
                            aSegEnd - (B2DPoint(-aSegEnd.getY(), aSegEnd.getX()) * fSegmentKappa),
                            aSegEnd);

                        nSegment = (nSegment + 1) % nSegments;
                        aSegStart = aSegEnd;
                    }

                    // End in this sector
                    const double fSegStartRad(nSegment * fAnglePerSegment);
                    fFactor= impDistanceBezierPointToControl(fEnd - fSegStartRad);
                    aSegEnd = B2DPoint(cos(fEnd), sin(fEnd));

                    aRetval.appendBezierSegment(
                        aSegStart + (B2DPoint(-aSegStart.getY(), aSegStart.getX()) * fFactor),
                        aSegEnd - (B2DPoint(-aSegEnd.getY(), aSegEnd.getX()) * fFactor),
                        aSegEnd);
                }
            }

            // remove double points between segments created by segmented creation
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

            if(nPointCount > 2)
            {
                B2DPoint aPrevPoint(rCandidate.getB2DPoint(nPointCount - 1));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0));

                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint((a + 1) % nPointCount));
                    const B2DVector aPrevVec(aPrevPoint - aCurrPoint);
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aOrientation(getOrientation(aNextVec, aPrevVec));

                    if(aOrientation == B2VectorOrientation::Neutral)
                    {
                        // current has neutral orientation
                        return true;
                    }
                    else
                    {
                        // prepare next
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
                B2DPoint aPrevPoint(rCandidate.getB2DPoint(nPointCount - 1));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0));

                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint((a + 1) % nPointCount));
                    const B2DVector aPrevVec(aPrevPoint - aCurrPoint);
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aOrientation(getOrientation(aNextVec, aPrevVec));

                    if(aOrientation == B2VectorOrientation::Neutral)
                    {
                        // current has neutral orientation, leave it out and prepare next
                        aCurrPoint = aNextPoint;
                    }
                    else
                    {
                        // add current point
                        aRetval.append(aCurrPoint);

                        // prepare next
                        aPrevPoint = aCurrPoint;
                        aCurrPoint = aNextPoint;
                    }
                }

                while(aRetval.count() && getOrientationForIndex(aRetval, 0) == B2VectorOrientation::Neutral)
                {
                    aRetval.remove(0);
                }

                // copy closed state
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

            if(nPointCount > 2)
            {
                const B2DPoint aPrevPoint(rCandidate.getB2DPoint(nPointCount - 1));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0));
                B2DVector aCurrVec(aPrevPoint - aCurrPoint);
                B2VectorOrientation aOrientation(B2VectorOrientation::Neutral);

                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint((a + 1) % nPointCount));
                    const B2DVector aNextVec(aNextPoint - aCurrPoint);
                    const B2VectorOrientation aCurrentOrientation(getOrientation(aNextVec, aCurrVec));

                    if(aOrientation == B2VectorOrientation::Neutral)
                    {
                        // set start value, maybe neutral again
                        aOrientation = aCurrentOrientation;
                    }
                    else
                    {
                        if(aCurrentOrientation != B2VectorOrientation::Neutral && aCurrentOrientation != aOrientation)
                        {
                            // different orientations found, that's it
                            return false;
                        }
                    }

                    // prepare next
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

            if(nPointCount > 1)
            {
                const sal_uInt32 nLoopCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1);
                B2DPoint aCurrentPoint(aCandidate.getB2DPoint(0));

                for(sal_uInt32 a(0); a < nLoopCount; a++)
                {
                    const B2DPoint aNextPoint(aCandidate.getB2DPoint((a + 1) % nPointCount));

                    if(isPointOnLine(aCurrentPoint, aNextPoint, rPoint, bWithPoints))
                    {
                        return true;
                    }

                    aCurrentPoint = aNextPoint;
                }
            }
            else if(nPointCount && bWithPoints)
            {
                return rPoint.equal(aCandidate.getB2DPoint(0));
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

            // tdf#88352 increase numerical correctness and use rtl::math::approxEqual
            // instead of fTools::equalZero which compares with a fixed small value
            if(fCrossA == 0.0)
            {
                // one point on the line
                return bWithLine;
            }

            const B2DVector aVectorToB(rEnd - rCandidateB);
            const double fCrossB(aLineVector.cross(aVectorToB));

            // increase numerical correctness
            if(fCrossB == 0.0)
            {
                // one point on the line
                return bWithLine;
            }

            // return true if they both have the same sign
            return ((fCrossA > 0.0) == (fCrossB > 0.0));
        }

        void addTriangleFan(
            const B2DPolygon& rCandidate,
            triangulator::B2DTriangleVector& rTarget)
        {
            const sal_uInt32 nCount(rCandidate.count());

            if(nCount > 2)
            {
                const B2DPoint aStart(rCandidate.getB2DPoint(0));
                B2DPoint aLast(rCandidate.getB2DPoint(1));

                for(sal_uInt32 a(2); a < nCount; a++)
                {
                    const B2DPoint aCurrent(rCandidate.getB2DPoint(a));
                    rTarget.emplace_back(
                        aStart,
                        aLast,
                        aCurrent);

                    // prepare next
                    aLast = aCurrent;
                }
            }
        }

        namespace
        {
            /// return 0 for input of 0, -1 for negative and 1 for positive input
            int lcl_sgn( const double n )
            {
                return n == 0.0 ? 0 : 1 - 2*int(rtl::math::isSignBitSet(n));
            }
        }

        bool isRectangle( const B2DPolygon& rPoly )
        {
            // polygon must be closed to resemble a rect, and contain
            // at least four points.
            if( !rPoly.isClosed() ||
                rPoly.count() < 4 ||
                rPoly.areControlPointsUsed() )
            {
                return false;
            }

            // number of 90 degree turns the polygon has taken
            int nNumTurns(0);

            int  nVerticalEdgeType=0;
            int  nHorizontalEdgeType=0;
            bool bNullVertex(true);
            bool bCWPolygon(false);  // when true, polygon is CW
                                     // oriented, when false, CCW
            bool bOrientationSet(false); // when false, polygon
                                         // orientation has not yet
                                         // been determined.

            // scan all _edges_ (which involves coming back to point 0
            // for the last edge - thus the modulo operation below)
            const sal_Int32 nCount( rPoly.count() );
            for( sal_Int32 i=0; i<nCount; ++i )
            {
                const B2DPoint& rPoint0( rPoly.getB2DPoint(i % nCount) );
                const B2DPoint& rPoint1( rPoly.getB2DPoint((i+1) % nCount) );

                // is 0 for zero direction vector, 1 for south edge and -1
                // for north edge (standard screen coordinate system)
                int nCurrVerticalEdgeType( lcl_sgn( rPoint1.getY() - rPoint0.getY() ) );

                // is 0 for zero direction vector, 1 for east edge and -1
                // for west edge (standard screen coordinate system)
                int nCurrHorizontalEdgeType( lcl_sgn(rPoint1.getX() - rPoint0.getX()) );

                if( nCurrVerticalEdgeType && nCurrHorizontalEdgeType )
                    return false; // oblique edge - for sure no rect

                const bool bCurrNullVertex( !nCurrVerticalEdgeType && !nCurrHorizontalEdgeType );

                // current vertex is equal to previous - just skip,
                // until we have a real edge
                if( bCurrNullVertex )
                    continue;

                // if previous edge has two identical points, because
                // no previous edge direction was available, simply
                // take this first non-null edge as the start
                // direction. That's what will happen here, if
                // bNullVertex is false
                if( !bNullVertex )
                {
                    // 2D cross product - is 1 for CW and -1 for CCW turns
                    const int nCrossProduct( nHorizontalEdgeType*nCurrVerticalEdgeType -
                                             nVerticalEdgeType*nCurrHorizontalEdgeType );

                    if( !nCrossProduct )
                        continue; // no change in orientation -
                                  // collinear edges - just go on

                    // if polygon orientation is not set, we'll
                    // determine it now
                    if( !bOrientationSet )
                    {
                        bCWPolygon = nCrossProduct == 1;
                        bOrientationSet = true;
                    }
                    else
                    {
                        // if current turn orientation is not equal
                        // initial orientation, this is not a
                        // rectangle (as rectangles have consistent
                        // orientation).
                        if( (nCrossProduct == 1) != bCWPolygon )
                            return false;
                    }

                    ++nNumTurns;

                    // More than four 90 degree turns are an
                    // indication that this must not be a rectangle.
                    if( nNumTurns > 4 )
                        return false;
                }

                // store current state for the next turn
                nVerticalEdgeType   = nCurrVerticalEdgeType;
                nHorizontalEdgeType = nCurrHorizontalEdgeType;
                bNullVertex         = false; // won't reach this line,
                                             // if bCurrNullVertex is
                                             // true - see above
            }

            return true;
        }

        B3DPolygon createB3DPolygonFromB2DPolygon(const B2DPolygon& rCandidate, double fZCoordinate)
        {
            if(rCandidate.areControlPointsUsed())
            {
                // call myself recursively with subdivided input
                const B2DPolygon aCandidate(adaptiveSubdivideByAngle(rCandidate));
                return createB3DPolygonFromB2DPolygon(aCandidate, fZCoordinate);
            }
            else
            {
                B3DPolygon aRetval;

                for(sal_uInt32 a(0); a < rCandidate.count(); a++)
                {
                    B2DPoint aPoint(rCandidate.getB2DPoint(a));
                    aRetval.append(B3DPoint(aPoint.getX(), aPoint.getY(), fZCoordinate));
                }

                // copy closed state
                aRetval.setClosed(rCandidate.isClosed());

                return aRetval;
            }
        }

        B2DPolygon createB2DPolygonFromB3DPolygon(const B3DPolygon& rCandidate, const B3DHomMatrix& rMat)
        {
            B2DPolygon aRetval;
            const sal_uInt32 nCount(rCandidate.count());
            const bool bIsIdentity(rMat.isIdentity());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                B3DPoint aCandidate(rCandidate.getB3DPoint(a));

                if(!bIsIdentity)
                {
                    aCandidate *= rMat;
                }

                aRetval.append(B2DPoint(aCandidate.getX(), aCandidate.getY()));
            }

            // copy closed state
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
                // get the relative cut value on line vector (Vector1) for cut with perpendicular through TestPoint
                const B2DVector aVector1(rPointB - rPointA);
                const B2DVector aVector2(rTestPoint - rPointA);
                const double fDividend((aVector2.getX() * aVector1.getX()) + (aVector2.getY() * aVector1.getY()));
                const double fDivisor((aVector1.getX() * aVector1.getX()) + (aVector1.getY() * aVector1.getY()));
                const double fCut(fDividend / fDivisor);

                if(fCut < 0.0)
                {
                    // not in line range, get distance to PointA
                    rCut = 0.0;
                    return aVector2.getLength();
                }
                else if(fCut > 1.0)
                {
                    // not in line range, get distance to PointB
                    rCut = 1.0;
                    const B2DVector aVector(rTestPoint - rPointB);
                    return aVector.getLength();
                }
                else
                {
                    // in line range
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

            if(nPointCount > 1)
            {
                const double fZero(0.0);
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                B2DCubicBezier aBezier;
                aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
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

                    if(fRetval == DBL_MAX || fEdgeDist < fRetval)
                    {
                        fRetval = fEdgeDist;
                        rEdgeIndex = a;
                        rCut = fNewCut;

                        if(fTools::equal(fRetval, fZero))
                        {
                            // already found zero distance, cannot get better. Ensure numerical zero value and end loop.
                            fRetval = 0.0;
                            break;
                        }
                    }

                    // prepare next step
                    aBezier.setStartPoint(aBezier.getEndPoint());
                }

                if(rtl::math::approxEqual(1.0, rCut))
                {
                    // correct rEdgeIndex when not last point
                    if(rCandidate.isClosed())
                    {
                        rEdgeIndex = getIndexOfSuccessor(rEdgeIndex, rCandidate);
                        rCut = 0.0;
                    }
                    else
                    {
                        if(rEdgeIndex != nEdgeCount - 1)
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
                const double fNewX(fOneMinusRelativeY * (fOneMinusRelativeX * rTopLeft.getX() + fRelativeX * rTopRight.getX()) +
                    fRelativeY * (fOneMinusRelativeX * rBottomLeft.getX() + fRelativeX * rBottomRight.getX()));
                const double fNewY(fOneMinusRelativeX * (fOneMinusRelativeY * rTopLeft.getY() + fRelativeY * rBottomLeft.getY()) +
                    fRelativeX * (fOneMinusRelativeY * rTopRight.getY() + fRelativeY * rBottomRight.getY()));

                return B2DPoint(fNewX, fNewY);
            }
        }

        B2DPolygon distort(const B2DPolygon& rCandidate, const B2DRange& rOriginal, const B2DPoint& rTopLeft, const B2DPoint& rTopRight, const B2DPoint& rBottomLeft, const B2DPoint& rBottomRight)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount && rOriginal.getWidth() != 0.0 && rOriginal.getHeight() != 0.0)
            {
                B2DPolygon aRetval;

                for(sal_uInt32 a(0); a < nPointCount; a++)
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

            for(sal_uInt32 a(0); a < rCandidate.count(); a++)
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
                // predecessor
                if(!rCandidate.isPrevControlPointUsed(nIndex))
                {
                    if(!rCandidate.isClosed() && nIndex == 0)
                    {
                        // do not create previous vector for start point of open polygon
                    }
                    else
                    {
                        const sal_uInt32 nPrevIndex((nIndex + (nPointCount - 1)) % nPointCount);
                        rCandidate.setPrevControlPoint(nIndex, interpolate(rCandidate.getB2DPoint(nIndex), rCandidate.getB2DPoint(nPrevIndex), 1.0 / 3.0));
                        bRetval = true;
                    }
                }

                // successor
                if(!rCandidate.isNextControlPointUsed(nIndex))
                {
                    if(!rCandidate.isClosed() && nIndex + 1 == nPointCount)
                    {
                        // do not create next vector for end point of open polygon
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
                    case B2VectorContinuity::NONE :
                    {
                        if(rCandidate.isPrevControlPointUsed(nIndex))
                        {
                            if(!rCandidate.isClosed() && nIndex == 0)
                            {
                                // remove existing previous vector for start point of open polygon
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
                                // remove next vector for end point of open polygon
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
                    case B2VectorContinuity::C1 :
                    {
                        if(rCandidate.isPrevControlPointUsed(nIndex) && rCandidate.isNextControlPointUsed(nIndex))
                        {
                            // lengths both exist since both are used
                            B2DVector aVectorPrev(rCandidate.getPrevControlPoint(nIndex) - aCurrentPoint);
                            B2DVector aVectorNext(rCandidate.getNextControlPoint(nIndex) - aCurrentPoint);
                            const double fLenPrev(aVectorPrev.getLength());
                            const double fLenNext(aVectorNext.getLength());
                            aVectorPrev.normalize();
                            aVectorNext.normalize();
                            const B2VectorOrientation aOrientation(getOrientation(aVectorPrev, aVectorNext));

                            if(aOrientation == B2VectorOrientation::Neutral && aVectorPrev.scalar(aVectorNext) < 0.0)
                            {
                                // parallel and opposite direction; check length
                                if(fTools::equal(fLenPrev, fLenNext))
                                {
                                    // this would be even C2, but we want C1. Use the lengths of the corresponding edges.
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
                                // not parallel or same direction, set vectors and length
                                const B2DVector aNormalizedPerpendicular(getNormalizedPerpendicular(aVectorPrev + aVectorNext));

                                if(aOrientation == B2VectorOrientation::Positive)
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
                    case B2VectorContinuity::C2 :
                    {
                        if(rCandidate.isPrevControlPointUsed(nIndex) && rCandidate.isNextControlPointUsed(nIndex))
                        {
                            // lengths both exist since both are used
                            B2DVector aVectorPrev(rCandidate.getPrevControlPoint(nIndex) - aCurrentPoint);
                            B2DVector aVectorNext(rCandidate.getNextControlPoint(nIndex) - aCurrentPoint);
                            const double fCommonLength((aVectorPrev.getLength() + aVectorNext.getLength()) / 2.0);
                            aVectorPrev.normalize();
                            aVectorNext.normalize();
                            const B2VectorOrientation aOrientation(getOrientation(aVectorPrev, aVectorNext));

                            if(aOrientation == B2VectorOrientation::Neutral && aVectorPrev.scalar(aVectorNext) < 0.0)
                            {
                                // parallel and opposite direction; set length. Use one direction for better numerical correctness
                                const B2DVector aScaledDirection(aVectorPrev * fCommonLength);

                                rCandidate.setControlPoints(nIndex,
                                    aCurrentPoint + aScaledDirection,
                                    aCurrentPoint - aScaledDirection);
                            }
                            else
                            {
                                // not parallel or same direction, set vectors and length
                                const B2DVector aNormalizedPerpendicular(getNormalizedPerpendicular(aVectorPrev + aVectorNext));
                                const B2DVector aPerpendicular(aNormalizedPerpendicular * fCommonLength);

                                if(aOrientation == B2VectorOrientation::Positive)
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
            if(fValue != 0.0)
            {
                if(rCandidate.areControlPointsUsed())
                {
                    // call myself recursively with subdivided input
                    const B2DPolygon aCandidate(adaptiveSubdivideByAngle(rCandidate));
                    return growInNormalDirection(aCandidate, fValue);
                }
                else
                {
                    B2DPolygon aRetval;
                    const sal_uInt32 nPointCount(rCandidate.count());

                    if(nPointCount)
                    {
                        B2DPoint aPrev(rCandidate.getB2DPoint(nPointCount - 1));
                        B2DPoint aCurrent(rCandidate.getB2DPoint(0));

                        for(sal_uInt32 a(0); a < nPointCount; a++)
                        {
                            const B2DPoint aNext(rCandidate.getB2DPoint(a + 1 == nPointCount ? 0 : a + 1));
                            const B2DVector aBack(aPrev - aCurrent);
                            const B2DVector aForw(aNext - aCurrent);
                            const B2DVector aPerpBack(getNormalizedPerpendicular(aBack));
                            const B2DVector aPerpForw(getNormalizedPerpendicular(aForw));
                            B2DVector aDirection(aPerpBack - aPerpForw);
                            aDirection.normalize();
                            aDirection *= fValue;
                            aRetval.append(aCurrent + aDirection);

                            // prepare next step
                            aPrev = aCurrent;
                            aCurrent = aNext;
                        }
                    }

                    // copy closed state
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
                // get current segment count
                const sal_uInt32 nSegmentCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);

                if(nSegmentCount == nSegments)
                {
                    aRetval = rCandidate;
                }
                else
                {
                    const double fLength(getLength(rCandidate));
                    const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nSegments : nSegments + 1);

                    for(sal_uInt32 a(0); a < nLoopCount; a++)
                    {
                        const double fRelativePos(static_cast<double>(a) / static_cast<double>(nSegments)); // 0.0 .. 1.0
                        const B2DPoint aNewPoint(getPositionRelative(rCandidate, fRelativePos, fLength));
                        aRetval.append(aNewPoint);
                    }

                    // copy closed flag
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

                for(sal_uInt32 a(0); a < rOld1.count(); a++)
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

        // #i76891#
        B2DPolygon simplifyCurveSegments(const B2DPolygon& rCandidate)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount && rCandidate.areControlPointsUsed())
            {
                // prepare loop
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                B2DPolygon aRetval;
                B2DCubicBezier aBezier;
                aBezier.setStartPoint(rCandidate.getB2DPoint(0));

                // try to avoid costly reallocations
                aRetval.reserve( nEdgeCount+1);

                // add start point
                aRetval.append(aBezier.getStartPoint());

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // get values for edge
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    aBezier.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                    aBezier.setControlPointA(rCandidate.getNextControlPoint(a));
                    aBezier.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                    aBezier.testAndSolveTrivialBezier();

                    // still bezier?
                    if(aBezier.isBezier())
                    {
                        // add edge with control vectors
                        aRetval.appendBezierSegment(aBezier.getControlPointA(), aBezier.getControlPointB(), aBezier.getEndPoint());
                    }
                    else
                    {
                        // add edge
                        aRetval.append(aBezier.getEndPoint());
                    }

                    // next point
                    aBezier.setStartPoint(aBezier.getEndPoint());
                }

                if(rCandidate.isClosed())
                {
                    // set closed flag, rescue control point and correct last double point
                    closeWithGeometryChange(aRetval);
                }

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        // makes the given indexed point the new polygon start point. To do that, the points in the
        // polygon will be rotated. This is only valid for closed polygons, for non-closed ones
        // an assertion will be triggered
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

                // iterate and consume pieces with fLength. First subdivide to reduce input to line segments
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
                                // move position on edge forward as long as on edge
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

                            // subtract length of current edge
                            fPositionInEdge -= fEdgeLength;
                        }

                        if(bEndActive && fTools::more(fAbsolutePosition, fEnd))
                        {
                            break;
                        }

                        // prepare next step
                        aCurrent = aNext;
                    }

                    // keep closed state
                    aRetval.setClosed(aCandidate.isClosed());
                }
                else
                {
                    // source polygon has only one point, return unchanged
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
                    // width and height, create waveline. First subdivide to reduce input to line segments
                    // of WaveWidth. Last segment may be missing. If this turns out to be a problem, it
                    // may be added here again using the original last point from rCandidate. It may
                    // also be the case that rCandidate was closed. To simplify things it is handled here
                    // as if it was opened.
                    // Result from createEdgesOfGivenLength contains no curved segments, handle as straight
                    // edges.
                    const B2DPolygon aEqualLenghEdges(createEdgesOfGivenLength(rCandidate, fWaveWidth));
                    const sal_uInt32 nPointCount(aEqualLenghEdges.count());

                    if(nPointCount > 1)
                    {
                        // iterate over straight edges, add start point
                        B2DPoint aCurrent(aEqualLenghEdges.getB2DPoint(0));
                        aRetval.append(aCurrent);

                        for(sal_uInt32 a(0); a < nPointCount - 1; a++)
                        {
                            const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                            const B2DPoint aNext(aEqualLenghEdges.getB2DPoint(nNextIndex));
                            const B2DVector aEdge(aNext - aCurrent);
                            const B2DVector aPerpendicular(getNormalizedPerpendicular(aEdge));
                            const B2DVector aControlOffset((aEdge * 0.467308) - (aPerpendicular * fWaveHeight));

                            // add curve segment
                            aRetval.appendBezierSegment(
                                aCurrent + aControlOffset,
                                aNext - aControlOffset,
                                aNext);

                            // prepare next step
                            aCurrent = aNext;
                        }
                    }
                }
                else
                {
                    // width but no height -> return original polygon
                    aRetval = rCandidate;
                }
            }
            else
            {
                // no width -> no waveline, stay empty and return
            }

            return aRetval;
        }

        // snap points of horizontal or vertical edges to discrete values
        B2DPolygon snapPointsOfHorizontalOrVerticalEdges(const B2DPolygon& rCandidate)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount > 1)
            {
                // Start by copying the source polygon to get a writeable copy. The closed state is
                // copied by aRetval's initialisation, too, so no need to copy it in this method
                B2DPolygon aRetval(rCandidate);

                // prepare geometry data. Get rounded from original
                B2ITuple aPrevTuple(basegfx::fround(rCandidate.getB2DPoint(nPointCount - 1)));
                B2DPoint aCurrPoint(rCandidate.getB2DPoint(0));
                B2ITuple aCurrTuple(basegfx::fround(aCurrPoint));

                // loop over all points. This will also snap the implicit closing edge
                // even when not closed, but that's no problem here
                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    // get next point. Get rounded from original
                    const bool bLastRun(a + 1 == nPointCount);
                    const sal_uInt32 nNextIndex(bLastRun ? 0 : a + 1);
                    const B2DPoint aNextPoint(rCandidate.getB2DPoint(nNextIndex));
                    const B2ITuple aNextTuple(basegfx::fround(aNextPoint));

                    // get the states
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

        B2DVector getTangentEnteringPoint(const B2DPolygon& rCandidate, sal_uInt32 nIndex)
        {
            B2DVector aRetval(0.0, 0.0);
            const sal_uInt32 nCount(rCandidate.count());

            if(nIndex >= nCount)
            {
                // out of range
                return aRetval;
            }

            // start immediately at prev point compared to nIndex
            const bool bClosed(rCandidate.isClosed());
            sal_uInt32 nPrev(bClosed ? (nIndex + nCount - 1) % nCount : nIndex ? nIndex - 1 : nIndex);

            if(nPrev == nIndex)
            {
                // no previous, done
                return aRetval;
            }

            B2DCubicBezier aSegment;

            // go backward in the polygon; if closed, maximal back to start index (nIndex); if not closed,
            // until zero. Use nIndex as stop criteria
            while(nPrev != nIndex)
            {
                // get BezierSegment and tangent at the *end* of segment
                rCandidate.getBezierSegment(nPrev, aSegment);
                aRetval = aSegment.getTangent(1.0);

                if(!aRetval.equalZero())
                {
                    // if we have a tangent, return it
                    return aRetval;
                }

                // prepare index before checked one
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
                // out of range
                return aRetval;
            }

            // start at nIndex
            const bool bClosed(rCandidate.isClosed());
            sal_uInt32 nCurrent(nIndex);
            B2DCubicBezier aSegment;

            // go forward; if closed, do this until once around and back at start index (nIndex); if not
            // closed, until last point (nCount - 1). Use nIndex as stop criteria
            do
            {
                // get BezierSegment and tangent at the *beginning* of segment
                rCandidate.getBezierSegment(nCurrent, aSegment);
                aRetval = aSegment.getTangent(0.0);

                if(!aRetval.equalZero())
                {
                    // if we have a tangent, return it
                    return aRetval;
                }

                // prepare next index
                nCurrent = bClosed ? (nCurrent + 1) % nCount : nCurrent + 1 < nCount ? nCurrent + 1 : nIndex;
            }
            while(nCurrent != nIndex);

            return aRetval;
        }

        // converters for css::drawing::PointSequence

        B2DPolygon UnoPointSequenceToB2DPolygon(
            const css::drawing::PointSequence& rPointSequenceSource)
        {
            B2DPolygon aRetval;
            const sal_uInt32 nLength(rPointSequenceSource.getLength());

            if(nLength)
            {
                aRetval.reserve(nLength);
                const css::awt::Point* pArray = rPointSequenceSource.getConstArray();
                const css::awt::Point* pArrayEnd = pArray + rPointSequenceSource.getLength();

                for(;pArray != pArrayEnd; pArray++)
                {
                    aRetval.append(B2DPoint(pArray->X, pArray->Y));
                }

                // check for closed state flag
                utils::checkClosed(aRetval);
            }

            return aRetval;
        }

        void B2DPolygonToUnoPointSequence(
            const B2DPolygon& rPolygon,
            css::drawing::PointSequence& rPointSequenceRetval)
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
                // Take closed state into account, the API polygon still uses the old closed definition
                // with last/first point are identical (cannot hold information about open polygons with identical
                // first and last point, though)
                const bool bIsClosed(aPolygon.isClosed());

                rPointSequenceRetval.realloc(bIsClosed ? nPointCount + 1 : nPointCount);
                css::awt::Point* pSequence = rPointSequenceRetval.getArray();

                for(sal_uInt32 b(0); b < nPointCount; b++)
                {
                    const B2DPoint aPoint(aPolygon.getB2DPoint(b));
                    const css::awt::Point aAPIPoint(fround(aPoint.getX()), fround(aPoint.getY()));

                    *pSequence = aAPIPoint;
                    pSequence++;
                }

                // copy first point if closed
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

        // converters for css::drawing::PointSequence and
        // css::drawing::FlagSequence to B2DPolygon (curved polygons)

        B2DPolygon UnoPolygonBezierCoordsToB2DPolygon(
            const css::drawing::PointSequence& rPointSequenceSource,
            const css::drawing::FlagSequence& rFlagSequenceSource)
        {
            const sal_uInt32 nCount(static_cast<sal_uInt32>(rPointSequenceSource.getLength()));
            OSL_ENSURE(nCount == static_cast<sal_uInt32>(rFlagSequenceSource.getLength()),
                "UnoPolygonBezierCoordsToB2DPolygon: Unequal count of Points and Flags (!)");

            // prepare new polygon
            B2DPolygon aRetval;

            if(0 != nCount)
            {
                const css::awt::Point* pPointSequence = rPointSequenceSource.getConstArray();
                const css::drawing::PolygonFlags* pFlagSequence = rFlagSequenceSource.getConstArray();

                // get first point and flag
                B2DPoint aNewCoordinatePair(pPointSequence->X, pPointSequence->Y); pPointSequence++;
                css::drawing::PolygonFlags ePolygonFlag(*pFlagSequence); pFlagSequence++;
                B2DPoint aControlA;
                B2DPoint aControlB;

                // first point is not allowed to be a control point
                OSL_ENSURE(ePolygonFlag != css::drawing::PolygonFlags_CONTROL,
                    "UnoPolygonBezierCoordsToB2DPolygon: Start point is a control point, illegal input polygon (!)");

                // add first point as start point
                aRetval.append(aNewCoordinatePair);

                for(sal_uInt32 b(1); b < nCount;)
                {
                    // prepare loop
                    bool bControlA(false);
                    bool bControlB(false);

                    // get next point and flag
                    aNewCoordinatePair = B2DPoint(pPointSequence->X, pPointSequence->Y);
                    ePolygonFlag = *pFlagSequence;
                    pPointSequence++; pFlagSequence++; b++;

                    if(b < nCount && ePolygonFlag == css::drawing::PolygonFlags_CONTROL)
                    {
                        aControlA = aNewCoordinatePair;
                        bControlA = true;

                        // get next point and flag
                        aNewCoordinatePair = B2DPoint(pPointSequence->X, pPointSequence->Y);
                        ePolygonFlag = *pFlagSequence;
                        pPointSequence++; pFlagSequence++; b++;
                    }

                    if(b < nCount && ePolygonFlag == css::drawing::PolygonFlags_CONTROL)
                    {
                        aControlB = aNewCoordinatePair;
                        bControlB = true;

                        // get next point and flag
                        aNewCoordinatePair = B2DPoint(pPointSequence->X, pPointSequence->Y);
                        ePolygonFlag = *pFlagSequence;
                        pPointSequence++; pFlagSequence++; b++;
                    }

                    // two or no control points are consumed, another one would be an error.
                    // It's also an error if only one control point was read
                    SAL_WARN_IF(ePolygonFlag == css::drawing::PolygonFlags_CONTROL || bControlA != bControlB,
                        "basegfx", "UnoPolygonBezierCoordsToB2DPolygon: Illegal source polygon (!)");

                    // the previous writes used the B2DPolyPoygon -> utils::PolyPolygon converter
                    // which did not create minimal PolyPolygons, but created all control points
                    // as null vectors (identical points). Because of the former P(CA)(CB)-norm of
                    // B2DPolygon and it's unused sign of being the zero-vector and CA and CB being
                    // relative to P, an empty edge was exported as P == CA == CB. Luckily, the new
                    // export format can be read without errors by the old OOo-versions, so we need only
                    // to correct here at read and do not need to export a wrong but compatible version
                    // for the future.
                    if(bControlA
                        && aControlA.equal(aControlB)
                        && aControlA.equal(aRetval.getB2DPoint(aRetval.count() - 1)))
                    {
                        bControlA = false;
                    }

                    if(bControlA)
                    {
                        // add bezier edge
                        aRetval.appendBezierSegment(aControlA, aControlB, aNewCoordinatePair);
                    }
                    else
                    {
                        // add edge
                        aRetval.append(aNewCoordinatePair);
                    }
                }

                // #i72807# API import uses old line start/end-equal definition for closed,
                // so we need to correct this to closed state here
                checkClosed(aRetval);
            }

            return aRetval;
        }

        void B2DPolygonToUnoPolygonBezierCoords(
            const B2DPolygon& rPolygon,
            css::drawing::PointSequence& rPointSequenceRetval,
            css::drawing::FlagSequence& rFlagSequenceRetval)
        {
            const sal_uInt32 nPointCount(rPolygon.count());

            if(nPointCount)
            {
                const bool bCurve(rPolygon.areControlPointsUsed());
                const bool bClosed(rPolygon.isClosed());

                if(bCurve)
                {
                    // calculate target point count
                    const sal_uInt32 nLoopCount(bClosed ? nPointCount : nPointCount - 1);

                    if(nLoopCount)
                    {
                        // prepare target data. The real needed number of target points (and flags)
                        // could only be calculated by using two loops, so use dynamic memory
                        std::vector< css::awt::Point > aCollectPoints;
                        std::vector< css::drawing::PolygonFlags > aCollectFlags;

                        // reserve maximum creatable points
                        const sal_uInt32 nMaxTargetCount((nLoopCount * 3) + 1);
                        aCollectPoints.reserve(nMaxTargetCount);
                        aCollectFlags.reserve(nMaxTargetCount);

                        // prepare current bezier segment by setting start point
                        B2DCubicBezier aBezierSegment;
                        aBezierSegment.setStartPoint(rPolygon.getB2DPoint(0));

                        for(sal_uInt32 a(0); a < nLoopCount; a++)
                        {
                            // add current point (always) and remember StartPointIndex for evtl. later corrections
                            const sal_uInt32 nStartPointIndex(aCollectPoints.size());
                            aCollectPoints.emplace_back(
                                    fround(aBezierSegment.getStartPoint().getX()),
                                    fround(aBezierSegment.getStartPoint().getY()));
                            aCollectFlags.push_back(css::drawing::PolygonFlags_NORMAL);

                            // prepare next segment
                            const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                            aBezierSegment.setEndPoint(rPolygon.getB2DPoint(nNextIndex));
                            aBezierSegment.setControlPointA(rPolygon.getNextControlPoint(a));
                            aBezierSegment.setControlPointB(rPolygon.getPrevControlPoint(nNextIndex));

                            if(aBezierSegment.isBezier())
                            {
                                // if bezier is used, add always two control points due to the old schema
                                aCollectPoints.emplace_back(
                                        fround(aBezierSegment.getControlPointA().getX()),
                                        fround(aBezierSegment.getControlPointA().getY()));
                                aCollectFlags.push_back(css::drawing::PolygonFlags_CONTROL);

                                aCollectPoints.emplace_back(
                                        fround(aBezierSegment.getControlPointB().getX()),
                                        fround(aBezierSegment.getControlPointB().getY()));
                                aCollectFlags.push_back(css::drawing::PolygonFlags_CONTROL);
                            }

                            // test continuity with previous control point to set flag value
                            if(aBezierSegment.getControlPointA() != aBezierSegment.getStartPoint() && (bClosed || a))
                            {
                                const B2VectorContinuity eCont(rPolygon.getContinuityInPoint(a));

                                if(eCont == B2VectorContinuity::C1)
                                {
                                    aCollectFlags[nStartPointIndex] = css::drawing::PolygonFlags_SMOOTH;
                                }
                                else if(eCont == B2VectorContinuity::C2)
                                {
                                    aCollectFlags[nStartPointIndex] = css::drawing::PolygonFlags_SYMMETRIC;
                                }
                            }

                            // prepare next loop
                            aBezierSegment.setStartPoint(aBezierSegment.getEndPoint());
                        }

                        if(bClosed)
                        {
                            // add first point again as closing point due to old definition
                            aCollectPoints.push_back(aCollectPoints[0]);
                            aCollectFlags.push_back(css::drawing::PolygonFlags_NORMAL);
                        }
                        else
                        {
                            // add last point as closing point
                            const B2DPoint aClosingPoint(rPolygon.getB2DPoint(nPointCount - 1));
                            aCollectPoints.emplace_back(
                                    fround(aClosingPoint.getX()),
                                    fround(aClosingPoint.getY()));
                            aCollectFlags.push_back(css::drawing::PolygonFlags_NORMAL);
                        }

                        // copy collected data to target arrays
                        const sal_uInt32 nTargetCount(aCollectPoints.size());
                        OSL_ENSURE(nTargetCount == aCollectFlags.size(), "Unequal Point and Flag count (!)");

                        rPointSequenceRetval.realloc(static_cast<sal_Int32>(nTargetCount));
                        rFlagSequenceRetval.realloc(static_cast<sal_Int32>(nTargetCount));
                        css::awt::Point* pPointSequence = rPointSequenceRetval.getArray();
                        css::drawing::PolygonFlags* pFlagSequence = rFlagSequenceRetval.getArray();

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
                    // straightforward point list creation
                    const sal_uInt32 nTargetCount(nPointCount + (bClosed ? 1 : 0));

                    rPointSequenceRetval.realloc(static_cast<sal_Int32>(nTargetCount));
                    rFlagSequenceRetval.realloc(static_cast<sal_Int32>(nTargetCount));

                    css::awt::Point* pPointSequence = rPointSequenceRetval.getArray();
                    css::drawing::PolygonFlags* pFlagSequence = rFlagSequenceRetval.getArray();

                    for(sal_uInt32 a(0); a < nPointCount; a++)
                    {
                        const B2DPoint aB2DPoint(rPolygon.getB2DPoint(a));
                        const css::awt::Point aAPIPoint(
                            fround(aB2DPoint.getX()),
                            fround(aB2DPoint.getY()));

                        *pPointSequence = aAPIPoint;
                        *pFlagSequence = css::drawing::PolygonFlags_NORMAL;
                        pPointSequence++;
                        pFlagSequence++;
                    }

                    if(bClosed)
                    {
                        // add first point as closing point
                        *pPointSequence = *rPointSequenceRetval.getConstArray();
                        *pFlagSequence = css::drawing::PolygonFlags_NORMAL;
                    }
                }
            }
            else
            {
                rPointSequenceRetval.realloc(0);
                rFlagSequenceRetval.realloc(0);
            }
        }

    } // end of namespace utils
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
