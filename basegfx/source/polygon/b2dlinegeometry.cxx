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

#include <cstdio>
#include <osl/diagnose.h>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

namespace basegfx
{
    namespace tools
    {
        B2DPolyPolygon createAreaGeometryForLineStartEnd(
            const B2DPolygon& rCandidate,
            const B2DPolyPolygon& rArrow,
            bool bStart,
            double fWidth,
            double fCandidateLength,
            double fDockingPosition, // 0->top, 1->bottom
            double* pConsumedLength,
            double fShift)
        {
            B2DPolyPolygon aRetval;
            OSL_ENSURE(rCandidate.count() > 1L, "createAreaGeometryForLineStartEnd: Line polygon has too less points (!)");
            OSL_ENSURE(rArrow.count() > 0L, "createAreaGeometryForLineStartEnd: Empty arrow tools::PolyPolygon (!)");
            OSL_ENSURE(fWidth > 0.0, "createAreaGeometryForLineStartEnd: Width too small (!)");
            OSL_ENSURE(fDockingPosition >= 0.0 && fDockingPosition <= 1.0,
                "createAreaGeometryForLineStartEnd: fDockingPosition out of range [0.0 .. 1.0] (!)");

            if(fWidth < 0.0)
            {
                fWidth = -fWidth;
            }

            if(rCandidate.count() > 1 && rArrow.count() && !fTools::equalZero(fWidth))
            {
                if(fDockingPosition < 0.0)
                {
                    fDockingPosition = 0.0;
                }
                else if(fDockingPosition > 1.0)
                {
                    fDockingPosition = 1.0;
                }

                // init return value from arrow
                aRetval.append(rArrow);

                // get size of the arrow
                const B2DRange aArrowSize(getRange(rArrow));

                // build ArrowTransform; center in X, align with axis in Y
                B2DHomMatrix aArrowTransform(basegfx::tools::createTranslateB2DHomMatrix(
                    -aArrowSize.getCenter().getX(), -aArrowSize.getMinimum().getY()));

                // scale to target size
                const double fArrowScale(fWidth / (aArrowSize.getWidth()));
                aArrowTransform.scale(fArrowScale, fArrowScale);

                // get arrow size in Y
                B2DPoint aUpperCenter(aArrowSize.getCenter().getX(), aArrowSize.getMaximum().getY());
                aUpperCenter *= aArrowTransform;
                const double fArrowYLength(B2DVector(aUpperCenter).getLength());

                // move arrow to have docking position centered
                aArrowTransform.translate(0.0, -fArrowYLength * fDockingPosition + fShift);

                // prepare polygon length
                if(fTools::equalZero(fCandidateLength))
                {
                    fCandidateLength = getLength(rCandidate);
                }

                // get the polygon vector we want to plant this arrow on
                const double fConsumedLength(fArrowYLength * (1.0 - fDockingPosition) - fShift);
                const B2DVector aHead(rCandidate.getB2DPoint((bStart) ? 0L : rCandidate.count() - 1L));
                const B2DVector aTail(getPositionAbsolute(rCandidate,
                    (bStart) ? fConsumedLength : fCandidateLength - fConsumedLength, fCandidateLength));

                // from that vector, take the needed rotation and add rotate for arrow to transformation
                const B2DVector aTargetDirection(aHead - aTail);
                const double fRotation(atan2(aTargetDirection.getY(), aTargetDirection.getX()) + (90.0 * F_PI180));

                // rotate around docking position
                aArrowTransform.rotate(fRotation);

                // move arrow docking position to polygon head
                aArrowTransform.translate(aHead.getX(), aHead.getY());

                // transform retval and close
                aRetval.transform(aArrowTransform);
                aRetval.setClosed(true);

                // if pConsumedLength is asked for, fill it
                if(pConsumedLength)
                {
                    *pConsumedLength = fConsumedLength;
                }
            }

            return aRetval;
        }
    } // end of namespace tools
} // end of namespace basegfx

namespace basegfx
{
    // anonymus namespace for local helpers
    namespace
    {
        bool impIsSimpleEdge(const B2DCubicBezier& rCandidate, double fMaxCosQuad, double fMaxPartOfEdgeQuad)
        {
            // isBezier() is true, already tested by caller
            const B2DVector aEdge(rCandidate.getEndPoint() - rCandidate.getStartPoint());

            if(aEdge.equalZero())
            {
                // start and end point the same, but control vectors used -> balloon curve loop
                // is not a simple edge
                return false;
            }

            // get tangentA and scalar with edge
            const B2DVector aTangentA(rCandidate.getTangent(0.0));
            const double fScalarAE(aEdge.scalar(aTangentA));

            if(fTools::lessOrEqual(fScalarAE, 0.0))
            {
                // angle between TangentA and Edge is bigger or equal 90 degrees
                return false;
            }

            // get self-scalars for E and A
            const double fScalarE(aEdge.scalar(aEdge));
            const double fScalarA(aTangentA.scalar(aTangentA));
            const double fLengthCompareE(fScalarE * fMaxPartOfEdgeQuad);

            if(fTools::moreOrEqual(fScalarA, fLengthCompareE))
            {
                // length of TangentA is more than fMaxPartOfEdge of length of edge
                return false;
            }

            if(fTools::lessOrEqual(fScalarAE * fScalarAE, fScalarA * fScalarE * fMaxCosQuad))
            {
                // angle between TangentA and Edge is bigger or equal angle defined by fMaxCos
                return false;
            }

            // get tangentB and scalar with edge
            const B2DVector aTangentB(rCandidate.getTangent(1.0));
            const double fScalarBE(aEdge.scalar(aTangentB));

            if(fTools::lessOrEqual(fScalarBE, 0.0))
            {
                // angle between TangentB and Edge is bigger or equal 90 degrees
                return false;
            }

            // get self-scalar for B
            const double fScalarB(aTangentB.scalar(aTangentB));

            if(fTools::moreOrEqual(fScalarB, fLengthCompareE))
            {
                // length of TangentB is more than fMaxPartOfEdge of length of edge
                return false;
            }

            if(fTools::lessOrEqual(fScalarBE * fScalarBE, fScalarB * fScalarE * fMaxCosQuad))
            {
                // angle between TangentB and Edge is bigger or equal defined by fMaxCos
                return false;
            }

            return true;
        }

        void impSubdivideToSimple(const B2DCubicBezier& rCandidate, B2DPolygon& rTarget, double fMaxCosQuad, double fMaxPartOfEdgeQuad, sal_uInt32 nMaxRecursionDepth)
        {
            if(!nMaxRecursionDepth || impIsSimpleEdge(rCandidate, fMaxCosQuad, fMaxPartOfEdgeQuad))
            {
                rTarget.appendBezierSegment(rCandidate.getControlPointA(), rCandidate.getControlPointB(), rCandidate.getEndPoint());
            }
            else
            {
                B2DCubicBezier aLeft, aRight;
                rCandidate.split(0.5, &aLeft, &aRight);

                impSubdivideToSimple(aLeft, rTarget, fMaxCosQuad, fMaxPartOfEdgeQuad, nMaxRecursionDepth - 1);
                impSubdivideToSimple(aRight, rTarget, fMaxCosQuad, fMaxPartOfEdgeQuad, nMaxRecursionDepth - 1);
            }
        }

        B2DPolygon subdivideToSimple(const B2DPolygon& rCandidate, double fMaxCosQuad, double fMaxPartOfEdgeQuad)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(rCandidate.areControlPointsUsed() && nPointCount)
            {
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                B2DPolygon aRetval;
                B2DCubicBezier aEdge;

                // prepare edge for loop
                aEdge.setStartPoint(rCandidate.getB2DPoint(0));
                aRetval.append(aEdge.getStartPoint());

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // fill B2DCubicBezier
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    aEdge.setControlPointA(rCandidate.getNextControlPoint(a));
                    aEdge.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                    aEdge.setEndPoint(rCandidate.getB2DPoint(nNextIndex));

                    // get rid of unnecessary bezier segments
                    aEdge.testAndSolveTrivialBezier();

                    if(aEdge.isBezier())
                    {
                        // before splitting recursively with internal simple criteria, use
                        // ExtremumPosFinder to remove those
                        ::std::vector< double > aExtremumPositions;

                        aExtremumPositions.reserve(4);
                        aEdge.getAllExtremumPositions(aExtremumPositions);

                        const sal_uInt32 nCount(aExtremumPositions.size());

                        if(nCount)
                        {
                            if(nCount > 1)
                            {
                                // create order from left to right
                                ::std::sort(aExtremumPositions.begin(), aExtremumPositions.end());
                            }

                            for(sal_uInt32 b(0); b < nCount;)
                            {
                                // split aEdge at next split pos
                                B2DCubicBezier aLeft;
                                const double fSplitPos(aExtremumPositions[b++]);

                                aEdge.split(fSplitPos, &aLeft, &aEdge);
                                aLeft.testAndSolveTrivialBezier();

                                // consume left part
                                if(aLeft.isBezier())
                                {
                                    impSubdivideToSimple(aLeft, aRetval, fMaxCosQuad, fMaxPartOfEdgeQuad, 6);
                                }
                                else
                                {
                                    aRetval.append(aLeft.getEndPoint());
                                }

                                if(b < nCount)
                                {
                                    // correct the remaining split positions to fit to shortened aEdge
                                    const double fScaleFactor(1.0 / (1.0 - fSplitPos));

                                    for(sal_uInt32 c(b); c < nCount; c++)
                                    {
                                        aExtremumPositions[c] = (aExtremumPositions[c] - fSplitPos) * fScaleFactor;
                                    }
                                }
                            }

                            // test the shortened rest of aEdge
                            aEdge.testAndSolveTrivialBezier();

                            // consume right part
                            if(aEdge.isBezier())
                            {
                                impSubdivideToSimple(aEdge, aRetval, fMaxCosQuad, fMaxPartOfEdgeQuad, 6);
                            }
                            else
                            {
                                aRetval.append(aEdge.getEndPoint());
                            }
                        }
                        else
                        {
                            impSubdivideToSimple(aEdge, aRetval, fMaxCosQuad, fMaxPartOfEdgeQuad, 6);
                        }
                    }
                    else
                    {
                        // straight edge, add point
                        aRetval.append(aEdge.getEndPoint());
                    }

                    // prepare edge for next step
                    aEdge.setStartPoint(aEdge.getEndPoint());
                }

                // copy closed flag and check for double points
                aRetval.setClosed(rCandidate.isClosed());
                aRetval.removeDoublePoints();

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolygon createAreaGeometryForEdge(
            const B2DCubicBezier& rEdge,
            double fHalfLineWidth,
            bool bStartRound,
            bool bEndRound,
            bool bStartSquare,
            bool bEndSquare)
        {
            // create polygon for edge
            // Unfortunately, while it would be geometrically correct to not add
            // the in-between points EdgeEnd and EdgeStart, it leads to rounding
            // errors when converting to integer polygon coordinates for painting
            if(rEdge.isBezier())
            {
                // prepare target and data common for upper and lower
                B2DPolygon aBezierPolygon;
                const B2DVector aPureEdgeVector(rEdge.getEndPoint() - rEdge.getStartPoint());
                const double fEdgeLength(aPureEdgeVector.getLength());
                const bool bIsEdgeLengthZero(fTools::equalZero(fEdgeLength));
                B2DVector aTangentA(rEdge.getTangent(0.0)); aTangentA.normalize();
                B2DVector aTangentB(rEdge.getTangent(1.0)); aTangentB.normalize();
                const B2DVector aNormalizedPerpendicularA(getPerpendicular(aTangentA));
                const B2DVector aNormalizedPerpendicularB(getPerpendicular(aTangentB));

                // create upper displacement vectors and check if they cut
                const B2DVector aPerpendStartA(aNormalizedPerpendicularA * -fHalfLineWidth);
                const B2DVector aPerpendEndA(aNormalizedPerpendicularB * -fHalfLineWidth);
                double fCutA(0.0);
                const CutFlagValue aCutA(tools::findCut(
                    rEdge.getStartPoint(), aPerpendStartA,
                    rEdge.getEndPoint(), aPerpendEndA,
                    CutFlagValue::ALL, &fCutA));
                const bool bCutA(CutFlagValue::NONE != aCutA);

                // create lower displacement vectors and check if they cut
                const B2DVector aPerpendStartB(aNormalizedPerpendicularA * fHalfLineWidth);
                const B2DVector aPerpendEndB(aNormalizedPerpendicularB * fHalfLineWidth);
                double fCutB(0.0);
                const CutFlagValue aCutB(tools::findCut(
                    rEdge.getEndPoint(), aPerpendEndB,
                    rEdge.getStartPoint(), aPerpendStartB,
                    CutFlagValue::ALL, &fCutB));
                const bool bCutB(CutFlagValue::NONE != aCutB);

                // check if cut happens
                const bool bCut(bCutA || bCutB);
                B2DPoint aCutPoint;

                // create left edge
                if(bStartRound || bStartSquare)
                {
                    if(bStartRound)
                    {
                        basegfx::B2DPolygon aStartPolygon(tools::createHalfUnitCircle());

                        aStartPolygon.transform(
                            tools::createScaleShearXRotateTranslateB2DHomMatrix(
                                fHalfLineWidth, fHalfLineWidth,
                                0.0,
                                atan2(aTangentA.getY(), aTangentA.getX()) + F_PI2,
                                rEdge.getStartPoint().getX(), rEdge.getStartPoint().getY()));
                        aBezierPolygon.append(aStartPolygon);
                    }
                    else // bStartSquare
                    {
                        const basegfx::B2DPoint aStart(rEdge.getStartPoint() - (aTangentA * fHalfLineWidth));

                        if(bCutB)
                        {
                            aBezierPolygon.append(rEdge.getStartPoint() + aPerpendStartB);
                        }

                        aBezierPolygon.append(aStart + aPerpendStartB);
                        aBezierPolygon.append(aStart + aPerpendStartA);

                        if(bCutA)
                        {
                            aBezierPolygon.append(rEdge.getStartPoint() + aPerpendStartA);
                        }
                    }
                }
                else
                {
                    // append original in-between point
                    aBezierPolygon.append(rEdge.getStartPoint());
                }

                // create upper edge.
                {
                    if(bCutA)
                    {
                        // calculate cut point and add
                        aCutPoint = rEdge.getStartPoint() + (aPerpendStartA * fCutA);
                        aBezierPolygon.append(aCutPoint);
                    }
                    else
                    {
                        // create scaled bezier segment
                        const B2DPoint aStart(rEdge.getStartPoint() + aPerpendStartA);
                        const B2DPoint aEnd(rEdge.getEndPoint() + aPerpendEndA);
                        const B2DVector aEdge(aEnd - aStart);
                        const double fLength(aEdge.getLength());
                        const double fScale(bIsEdgeLengthZero ? 1.0 : fLength / fEdgeLength);
                        const B2DVector fRelNext(rEdge.getControlPointA() - rEdge.getStartPoint());
                        const B2DVector fRelPrev(rEdge.getControlPointB() - rEdge.getEndPoint());

                        aBezierPolygon.append(aStart);
                        aBezierPolygon.appendBezierSegment(aStart + (fRelNext * fScale), aEnd + (fRelPrev * fScale), aEnd);
                    }
                }

                // create right edge
                if(bEndRound || bEndSquare)
                {
                    if(bEndRound)
                    {
                        basegfx::B2DPolygon aEndPolygon(tools::createHalfUnitCircle());

                        aEndPolygon.transform(
                            tools::createScaleShearXRotateTranslateB2DHomMatrix(
                                fHalfLineWidth, fHalfLineWidth,
                                0.0,
                                atan2(aTangentB.getY(), aTangentB.getX()) - F_PI2,
                                rEdge.getEndPoint().getX(), rEdge.getEndPoint().getY()));
                        aBezierPolygon.append(aEndPolygon);
                    }
                    else // bEndSquare
                    {
                        const basegfx::B2DPoint aEnd(rEdge.getEndPoint() + (aTangentB * fHalfLineWidth));

                        if(bCutA)
                        {
                            aBezierPolygon.append(rEdge.getEndPoint() + aPerpendEndA);
                        }

                        aBezierPolygon.append(aEnd + aPerpendEndA);
                        aBezierPolygon.append(aEnd + aPerpendEndB);

                        if(bCutB)
                        {
                            aBezierPolygon.append(rEdge.getEndPoint() + aPerpendEndB);
                        }
                    }
                }
                else
                {
                    // append original in-between point
                    aBezierPolygon.append(rEdge.getEndPoint());
                }

                // create lower edge.
                {
                    if(bCutB)
                    {
                        // calculate cut point and add
                        aCutPoint = rEdge.getEndPoint() + (aPerpendEndB * fCutB);
                        aBezierPolygon.append(aCutPoint);
                    }
                    else
                    {
                        // create scaled bezier segment
                        const B2DPoint aStart(rEdge.getEndPoint() + aPerpendEndB);
                        const B2DPoint aEnd(rEdge.getStartPoint() + aPerpendStartB);
                        const B2DVector aEdge(aEnd - aStart);
                        const double fLength(aEdge.getLength());
                        const double fScale(bIsEdgeLengthZero ? 1.0 : fLength / fEdgeLength);
                        const B2DVector fRelNext(rEdge.getControlPointB() - rEdge.getEndPoint());
                        const B2DVector fRelPrev(rEdge.getControlPointA() - rEdge.getStartPoint());

                        aBezierPolygon.append(aStart);
                        aBezierPolygon.appendBezierSegment(aStart + (fRelNext * fScale), aEnd + (fRelPrev * fScale), aEnd);
                    }
                }

                // close
                aBezierPolygon.setClosed(true);

                if(bStartRound || bEndRound)
                {
                    // double points possible when round caps are used at start or end
                    aBezierPolygon.removeDoublePoints();
                }

                if(bCut && ((bStartRound || bStartSquare) && (bEndRound || bEndSquare)))
                {
                    // When cut exists and both ends are extended with caps, a self-intersecting polygon
                    // is created; one cut point is known, but there is a 2nd one in the caps geometry.
                    // Solve by using tooling.
                    // Remark: This nearly never happens due to curve preparations to extreme points
                    // and maximum angle turning, but I constructed a test case and checked that it is
                    // working propery.
                    const B2DPolyPolygon aTemp(tools::solveCrossovers(aBezierPolygon));
                    const sal_uInt32 nTempCount(aTemp.count());

                    if(nTempCount)
                    {
                        if(nTempCount > 1)
                        {
                            // as expected, multiple polygons (with same orientation). Remove
                            // the one which contains aCutPoint, or better take the one without
                            for (sal_uInt32 a(0); a < nTempCount; a++)
                            {
                                aBezierPolygon = aTemp.getB2DPolygon(a);

                                const sal_uInt32 nCandCount(aBezierPolygon.count());

                                for(sal_uInt32 b(0); b < nCandCount; b++)
                                {
                                    if(aCutPoint.equal(aBezierPolygon.getB2DPoint(b)))
                                    {
                                        aBezierPolygon.clear();
                                        break;
                                    }
                                }

                                if(aBezierPolygon.count())
                                {
                                    break;
                                }
                            }

                            OSL_ENSURE(aBezierPolygon.count(), "Error in line geometry creation, could not solve self-intersection (!)");
                        }
                        else
                        {
                            // none found, use result
                            aBezierPolygon = aTemp.getB2DPolygon(0);
                        }
                    }
                    else
                    {
                        OSL_ENSURE(false, "Error in line geometry creation, could not solve self-intersection (!)");
                    }
                }

                // return
                return aBezierPolygon;
            }
            else
            {
                // Get start and  end point, create tangent and set to needed length
                B2DVector aTangent(rEdge.getEndPoint() - rEdge.getStartPoint());
                aTangent.setLength(fHalfLineWidth);

                // prepare return value
                B2DPolygon aEdgePolygon;

                // buffered angle
                double fAngle(0.0);
                bool bAngle(false);

                // buffered perpendicular
                B2DVector aPerpend;
                bool bPerpend(false);

                // create left vertical
                if(bStartRound)
                {
                    aEdgePolygon = tools::createHalfUnitCircle();
                    fAngle = atan2(aTangent.getY(), aTangent.getX());
                    bAngle = true;
                    aEdgePolygon.transform(
                        tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            fHalfLineWidth, fHalfLineWidth,
                            0.0,
                            fAngle + F_PI2,
                            rEdge.getStartPoint().getX(), rEdge.getStartPoint().getY()));
                }
                else
                {
                    aPerpend.setX(-aTangent.getY());
                    aPerpend.setY(aTangent.getX());
                    bPerpend = true;

                    if(bStartSquare)
                    {
                        const basegfx::B2DPoint aStart(rEdge.getStartPoint() - aTangent);

                        aEdgePolygon.append(aStart + aPerpend);
                        aEdgePolygon.append(aStart - aPerpend);
                    }
                    else
                    {
                        aEdgePolygon.append(rEdge.getStartPoint() + aPerpend);
                        aEdgePolygon.append(rEdge.getStartPoint()); // keep the in-between point for numerical reasons
                        aEdgePolygon.append(rEdge.getStartPoint() - aPerpend);
                    }
                }

                // create right vertical
                if(bEndRound)
                {
                    basegfx::B2DPolygon aEndPolygon(tools::createHalfUnitCircle());

                    if(!bAngle)
                    {
                        fAngle = atan2(aTangent.getY(), aTangent.getX());
                    }

                    aEndPolygon.transform(
                        tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            fHalfLineWidth, fHalfLineWidth,
                            0.0,
                            fAngle - F_PI2,
                            rEdge.getEndPoint().getX(), rEdge.getEndPoint().getY()));
                    aEdgePolygon.append(aEndPolygon);
                }
                else
                {
                    if(!bPerpend)
                    {
                        aPerpend.setX(-aTangent.getY());
                        aPerpend.setY(aTangent.getX());
                    }

                    if(bEndSquare)
                    {
                        const basegfx::B2DPoint aEnd(rEdge.getEndPoint() + aTangent);

                        aEdgePolygon.append(aEnd - aPerpend);
                        aEdgePolygon.append(aEnd + aPerpend);
                    }
                    else
                    {
                        aEdgePolygon.append(rEdge.getEndPoint() - aPerpend);
                        aEdgePolygon.append(rEdge.getEndPoint()); // keep the in-between point for numerical reasons
                        aEdgePolygon.append(rEdge.getEndPoint() + aPerpend);
                    }
                }

                // close and return
                aEdgePolygon.setClosed(true);

                return aEdgePolygon;
            }
        }

        B2DPolygon createAreaGeometryForJoin(
            const B2DVector& rTangentPrev,
            const B2DVector& rTangentEdge,
            const B2DVector& rPerpendPrev,
            const B2DVector& rPerpendEdge,
            const B2DPoint& rPoint,
            double fHalfLineWidth,
            B2DLineJoin eJoin,
            double fMiterMinimumAngle)
        {
            OSL_ENSURE(fHalfLineWidth > 0.0, "createAreaGeometryForJoin: LineWidth too small (!)");
            OSL_ENSURE(B2DLineJoin::NONE != eJoin, "createAreaGeometryForJoin: B2DLineJoin::NONE not allowed (!)");

            // LineJoin from tangent rPerpendPrev to tangent rPerpendEdge in rPoint
            B2DPolygon aEdgePolygon;
            const B2DPoint aStartPoint(rPoint + rPerpendPrev);
            const B2DPoint aEndPoint(rPoint + rPerpendEdge);

            // test if for Miter, the angle is too small and the fallback
            // to bevel needs to be used
            if(B2DLineJoin::Miter == eJoin)
            {
                const double fAngle(fabs(rPerpendPrev.angle(rPerpendEdge)));

                if((F_PI - fAngle) < fMiterMinimumAngle)
                {
                    // fallback to bevel
                    eJoin = B2DLineJoin::Bevel;
                }
            }

            switch(eJoin)
            {
                case B2DLineJoin::Miter :
                {
                    aEdgePolygon.append(aEndPoint);
                    aEdgePolygon.append(rPoint);
                    aEdgePolygon.append(aStartPoint);

                    // Look for the cut point between start point along rTangentPrev and
                    // end point along rTangentEdge. -rTangentEdge should be used, but since
                    // the cut value is used for interpolating along the first edge, the negation
                    // is not needed since the same fCut will be found on the first edge.
                    // If it exists, insert it to complete the mitered fill polygon.
                    double fCutPos(0.0);
                    tools::findCut(aStartPoint, rTangentPrev, aEndPoint, rTangentEdge, CutFlagValue::ALL, &fCutPos);

                    if(0.0 != fCutPos)
                    {
                        const B2DPoint aCutPoint(aStartPoint + (rTangentPrev * fCutPos));
                        aEdgePolygon.append(aCutPoint);
                    }

                    break;
                }
                case B2DLineJoin::Round :
                {
                    // use tooling to add needed EllipseSegment
                    double fAngleStart(atan2(rPerpendPrev.getY(), rPerpendPrev.getX()));
                    double fAngleEnd(atan2(rPerpendEdge.getY(), rPerpendEdge.getX()));

                    // atan2 results are [-PI .. PI], consolidate to [0.0 .. 2PI]
                    if(fAngleStart < 0.0)
                    {
                        fAngleStart += F_2PI;
                    }

                    if(fAngleEnd < 0.0)
                    {
                        fAngleEnd += F_2PI;
                    }

                    const B2DPolygon aBow(tools::createPolygonFromEllipseSegment(rPoint, fHalfLineWidth, fHalfLineWidth, fAngleStart, fAngleEnd));

                    if(aBow.count() > 1)
                    {
                        // #i101491#
                        // use the original start/end positions; the ones from bow creation may be numerically
                        // different due to their different creation. To guarantee good merging quality with edges
                        // and edge roundings (and to reduce point count)
                        aEdgePolygon = aBow;
                        aEdgePolygon.setB2DPoint(0, aStartPoint);
                        aEdgePolygon.setB2DPoint(aEdgePolygon.count() - 1, aEndPoint);
                        aEdgePolygon.append(rPoint);

                        break;
                    }
                    else
                    {
                        SAL_FALLTHROUGH; // wanted fall-through to default
                    }
                }
                default: // B2DLineJoin::Bevel
                {
                    aEdgePolygon.append(aEndPoint);
                    aEdgePolygon.append(rPoint);
                    aEdgePolygon.append(aStartPoint);

                    break;
                }
            }

            // create last polygon part for edge
            aEdgePolygon.setClosed(true);

            return aEdgePolygon;
        }
    } // end of anonymus namespace

    namespace tools
    {
        B2DPolyPolygon createAreaGeometry(
            const B2DPolygon& rCandidate,
            double fHalfLineWidth,
            B2DLineJoin eJoin,
            css::drawing::LineCap eCap,
            double fMaxAllowedAngle,
            double fMaxPartOfEdge,
            double fMiterMinimumAngle)
        {
            if(fMaxAllowedAngle > F_PI2)
            {
                fMaxAllowedAngle = F_PI2;
            }
            else if(fMaxAllowedAngle < 0.01 * F_PI2)
            {
                fMaxAllowedAngle = 0.01 * F_PI2;
            }

            if(fMaxPartOfEdge > 1.0)
            {
                fMaxPartOfEdge = 1.0;
            }
            else if(fMaxPartOfEdge < 0.01)
            {
                fMaxPartOfEdge = 0.01;
            }

            if(fMiterMinimumAngle > F_PI)
            {
                fMiterMinimumAngle = F_PI;
            }
            else if(fMiterMinimumAngle < 0.01 * F_PI)
            {
                fMiterMinimumAngle = 0.01 * F_PI;
            }

            B2DPolygon aCandidate(rCandidate);
            const double fMaxCos(cos(fMaxAllowedAngle));

            aCandidate.removeDoublePoints();
            aCandidate = subdivideToSimple(aCandidate, fMaxCos * fMaxCos, fMaxPartOfEdge * fMaxPartOfEdge);

            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount)
            {
                B2DPolyPolygon aRetval;
                const bool bIsClosed(aCandidate.isClosed());
                const sal_uInt32 nEdgeCount(bIsClosed ? nPointCount : nPointCount - 1);
                const bool bLineCap(!bIsClosed && css::drawing::LineCap_BUTT != eCap);

                if(nEdgeCount)
                {
                    B2DCubicBezier aEdge;
                    B2DCubicBezier aPrev;

                    const bool bEventuallyCreateLineJoin(B2DLineJoin::NONE != eJoin);
                    // prepare edge
                    aEdge.setStartPoint(aCandidate.getB2DPoint(0));

                    if(bIsClosed && bEventuallyCreateLineJoin)
                    {
                        // prepare previous edge
                        const sal_uInt32 nPrevIndex(nPointCount - 1);
                        aPrev.setStartPoint(aCandidate.getB2DPoint(nPrevIndex));
                        aPrev.setControlPointA(aCandidate.getNextControlPoint(nPrevIndex));
                        aPrev.setControlPointB(aCandidate.getPrevControlPoint(0));
                        aPrev.setEndPoint(aEdge.getStartPoint());
                    }

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        // fill current Edge
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        aEdge.setControlPointA(aCandidate.getNextControlPoint(a));
                        aEdge.setControlPointB(aCandidate.getPrevControlPoint(nNextIndex));
                        aEdge.setEndPoint(aCandidate.getB2DPoint(nNextIndex));

                        // check and create linejoin
                        if(bEventuallyCreateLineJoin && (bIsClosed || 0 != a))
                        {
                            B2DVector aTangentPrev(aPrev.getTangent(1.0)); aTangentPrev.normalize();
                            B2DVector aTangentEdge(aEdge.getTangent(0.0)); aTangentEdge.normalize();
                            B2VectorOrientation aOrientation(getOrientation(aTangentPrev, aTangentEdge));

                            if(B2VectorOrientation::Neutral == aOrientation)
                            {
                                   // they are parallel or empty; if they are both not zero and point
                                   // in opposite direction, a half-circle is needed
                                   if(!aTangentPrev.equalZero() && !aTangentEdge.equalZero())
                                   {
                                    const double fAngle(fabs(aTangentPrev.angle(aTangentEdge)));

                                    if(fTools::equal(fAngle, F_PI))
                                    {
                                        // for half-circle production, fallback to positive
                                        // orientation
                                        aOrientation = B2VectorOrientation::Positive;
                                    }
                                }
                            }

                            if(B2VectorOrientation::Positive == aOrientation)
                            {
                                const B2DVector aPerpendPrev(getPerpendicular(aTangentPrev) * -fHalfLineWidth);
                                const B2DVector aPerpendEdge(getPerpendicular(aTangentEdge) * -fHalfLineWidth);

                                aRetval.append(
                                    createAreaGeometryForJoin(
                                        aTangentPrev,
                                        aTangentEdge,
                                        aPerpendPrev,
                                        aPerpendEdge,
                                        aEdge.getStartPoint(),
                                        fHalfLineWidth,
                                        eJoin,
                                        fMiterMinimumAngle));
                            }
                            else if(B2VectorOrientation::Negative == aOrientation)
                            {
                                const B2DVector aPerpendPrev(getPerpendicular(aTangentPrev) * fHalfLineWidth);
                                const B2DVector aPerpendEdge(getPerpendicular(aTangentEdge) * fHalfLineWidth);

                                aRetval.append(
                                    createAreaGeometryForJoin(
                                        aTangentEdge,
                                        aTangentPrev,
                                        aPerpendEdge,
                                        aPerpendPrev,
                                        aEdge.getStartPoint(),
                                        fHalfLineWidth,
                                        eJoin,
                                        fMiterMinimumAngle));
                            }
                        }

                        // create geometry for edge
                        const bool bLast(a + 1 == nEdgeCount);

                        if(bLineCap)
                        {
                            const bool bFirst(!a);

                            aRetval.append(
                                createAreaGeometryForEdge(
                                    aEdge,
                                    fHalfLineWidth,
                                    bFirst && css::drawing::LineCap_ROUND == eCap,
                                    bLast && css::drawing::LineCap_ROUND == eCap,
                                    bFirst && css::drawing::LineCap_SQUARE == eCap,
                                    bLast && css::drawing::LineCap_SQUARE == eCap));
                        }
                        else
                        {
                            aRetval.append(
                                createAreaGeometryForEdge(
                                    aEdge,
                                    fHalfLineWidth,
                                    false,
                                    false,
                                    false,
                                    false));
                        }

                        // prepare next step
                        if(!bLast)
                        {
                            if(bEventuallyCreateLineJoin)
                            {
                                aPrev = aEdge;
                            }

                            aEdge.setStartPoint(aEdge.getEndPoint());
                        }
                    }
                }

                return aRetval;
            }
            else
            {
                return B2DPolyPolygon(rCandidate);
            }
        }
    } // end of namespace tools
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
