/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dlinegeometry.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:45:33 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POLYGON_B2DLINEGEOMETRY_HXX
#include <basegfx/polygon/b2dlinegeometry.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // anonymus namespace for local helpers
    namespace
    {
        // create area geometry for given edge. Edge is defined with the
        // points rEdgeStart and rEdgeEnd. fHalfLineWidth defines the relative width.
        // The created polygon will be positively oriented and free of
        // self intersections.
        // bCreateInBetweenPoints defines if EdgeStart and EdgeEnd themselves will
        // be added (see comment in implementation).
        B2DPolygon createAreaGeometryForEdge(
            const B2DPoint& rEdgeStart,
            const B2DPoint& rEdgeEnd,
            double fHalfLineWidth,
            bool bCreateInBetweenPoints)
        {
            OSL_ENSURE(fHalfLineWidth > 0.0, "createAreaGeometryForEdge: LineWidth too small (!)");
            B2DPolygon aRetval;

            // get perpendicular vector for reaching the outer edges
            const B2DVector aEdgeVector(rEdgeEnd - rEdgeStart);
            B2DVector aPerpendEdgeVector(getNormalizedPerpendicular(aEdgeVector));
            aPerpendEdgeVector *= fHalfLineWidth;

            // create polygon for edge
            // Unfortunately, while it would be geometrically correct to not add
            // the in-between points rEdgeEnd and rEdgeStart, it leads to rounding
            // errors when converting to integer polygon coordiates for painting.
            aRetval.append(rEdgeStart - aPerpendEdgeVector);
            aRetval.append(rEdgeEnd - aPerpendEdgeVector);

            if(bCreateInBetweenPoints)
            {
                aRetval.append(rEdgeEnd);
            }

            aRetval.append(rEdgeEnd + aPerpendEdgeVector);
            aRetval.append(rEdgeStart + aPerpendEdgeVector);

            if(bCreateInBetweenPoints)
            {
                aRetval.append(rEdgeStart);
            }

            aRetval.setClosed(true);

#ifdef DBG_UTIL
            // check orientation (debug only)
            if(tools::getOrientation(aRetval) == ORIENTATION_NEGATIVE)
            {
                OSL_ENSURE(false, "createAreaGeometryForEdge: orientation of return value is negative (!)");
            }
#endif

            return aRetval;
        }

        // create join polygon for given angle. Angle is defined with the
        // points rLeft, rCenter and rRight. The given join type defines how
        // the join segment will be created. fHalfLineWidth defines the relative width.
        // fDegreeStepWidth is used when rounding edges.
        // fMiterMinimumAngle is used to define when miter is forced to bevel.
        // The created polygon will be positively or neuteral oriented and free of
        // self intersections.
        B2DPolygon createAreaGeometryForJoin(
            const B2DPoint& rLeft,
            const B2DPoint& rCenter,
            const B2DPoint& rRight,
            double fHalfLineWidth,
            tools::B2DLineJoin eJoin,
            double fDegreeStepWidth,
            double fMiterMinimumAngle)
        {
            OSL_ENSURE(fHalfLineWidth > 0.0, "createAreaGeometryForJoin: LineWidth too small (!)");
            OSL_ENSURE(fDegreeStepWidth > 0.0, "createAreaGeometryForJoin: DegreeStepWidth too small (!)");
            OSL_ENSURE(tools::B2DLINEJOIN_NONE != eJoin, "createAreaGeometryForJoin: B2DLINEJOIN_NONE not allowed (!)");
            B2DPolygon aRetval;

            // get perpendicular vector for left and right
            const B2DVector aLeftVector(rCenter - rLeft);
            B2DVector aPerpendLeftVector(getNormalizedPerpendicular(aLeftVector));
            const B2DVector aRightVector(rRight - rCenter);
            B2DVector aPerpendRightVector(getNormalizedPerpendicular(aRightVector));

            // get vector orientation
            B2VectorOrientation aOrientation(getOrientation(aPerpendLeftVector, aPerpendRightVector));

            if(ORIENTATION_NEUTRAL != aOrientation)
            {
                // prepare perpend vectors to be able to go from left to right.
                // also multiply with fHalfLineWidth to get geometric vectors with correct length
                if(ORIENTATION_POSITIVE == aOrientation)
                {
                    // mirror to have them above the edge vectors
                    const double fNegativeHalfLineWidth(-fHalfLineWidth);
                    aPerpendLeftVector *= fNegativeHalfLineWidth;
                    aPerpendRightVector *= fNegativeHalfLineWidth;
                }
                else
                {
                    // exchange left and right
                    const B2DVector aTemp(aPerpendLeftVector.getX() * fHalfLineWidth, aPerpendLeftVector.getY() * fHalfLineWidth);
                    aPerpendLeftVector.setX(aPerpendRightVector.getX() * fHalfLineWidth);
                    aPerpendLeftVector.setY(aPerpendRightVector.getY() * fHalfLineWidth);
                    aPerpendRightVector = aTemp;
                }

                // test if for Miter, the angle is too small
                if(tools::B2DLINEJOIN_MITER == eJoin)
                {
                    const double fAngle(fabs(aPerpendLeftVector.angle(aPerpendRightVector)));

                    if((F_PI - fAngle) < (15.0 * F_PI180))
                    {
                        // force to bevel
                        eJoin = tools::B2DLINEJOIN_BEVEL;
                    }
                }

                // create specific edge polygon
                switch(eJoin)
                {
                    case tools::B2DLINEJOIN_MIDDLE :
                    case tools::B2DLINEJOIN_BEVEL :
                    {
                        // create polygon for edge, go from left to right
                        aRetval.append(rCenter);
                        aRetval.append(rCenter + aPerpendLeftVector);
                        aRetval.append(rCenter + aPerpendRightVector);
                        aRetval.setClosed(true);

                        break;
                    }
                    case tools::B2DLINEJOIN_MITER :
                    {
                        // create first polygon part for edge, go from left to right
                        aRetval.append(rCenter);
                        aRetval.append(rCenter + aPerpendLeftVector);

                        double fCutPos(0.0);
                        const B2DPoint aLeftCutPoint(rCenter + aPerpendLeftVector);
                        const B2DPoint aRightCutPoint(rCenter + aPerpendRightVector);

                        if(ORIENTATION_POSITIVE == aOrientation)
                        {
                            tools::findCut(aLeftCutPoint, aLeftVector, aRightCutPoint, -aRightVector, CUTFLAG_ALL, &fCutPos);

                            if(0.0 != fCutPos)
                            {
                                const B2DPoint aCutPoint(
                                    interpolate(aLeftCutPoint, aLeftCutPoint + aLeftVector, fCutPos));
                                aRetval.append(aCutPoint);
                            }
                        }
                        else
                        {
                            // peroendiculars are exchanged, also use exchanged EdgeVectors
                            tools::findCut(aLeftCutPoint, -aRightVector, aRightCutPoint, aLeftVector, CUTFLAG_ALL, &fCutPos);

                            if(0.0 != fCutPos)
                            {
                                const B2DPoint aCutPoint(
                                    interpolate(aLeftCutPoint, aLeftCutPoint - aRightVector, fCutPos));
                                aRetval.append(aCutPoint);
                            }
                        }

                        // create last polygon part for edge
                        aRetval.append(rCenter + aPerpendRightVector);
                        aRetval.setClosed(true);

                        break;
                    }
                    case tools::B2DLINEJOIN_ROUND :
                    {
                        // create first polygon part for edge, go from left to right
                        aRetval.append(rCenter);
                        aRetval.append(rCenter + aPerpendLeftVector);

                        // get angle and prepare
                        double fAngle(aPerpendLeftVector.angle(aPerpendRightVector));
                        const bool bNegative(fAngle < 0.0);
                        if(bNegative)
                        {
                            fAngle = fabs(fAngle);
                        }

                        // substract first step, first position is added to
                        // the polygon yet
                        fAngle -= fDegreeStepWidth;

                        // create points as long as angle is > 0.0
                        if(fAngle > 0.0)
                        {
                            // get start angle
                            double fAngleOfLeftPerpendVector(
                                atan2(aPerpendLeftVector.getY(), aPerpendLeftVector.getX()));

                            while(fAngle > 0.0)
                            {
                                // calculate rotated vector
                                fAngleOfLeftPerpendVector += (bNegative ? -fDegreeStepWidth : fDegreeStepWidth);
                                const B2DVector aRotatedVector(
                                    rCenter.getX() + (cos(fAngleOfLeftPerpendVector) * fHalfLineWidth),
                                    rCenter.getY() + (sin(fAngleOfLeftPerpendVector) * fHalfLineWidth));

                                // add point
                                aRetval.append(aRotatedVector);

                                // substract next step
                                fAngle -= fDegreeStepWidth;
                            }
                        }

                        // create last polygon part for edge
                        aRetval.append(rCenter + aPerpendRightVector);
                        aRetval.setClosed(true);

                        break;
                    }
                }
            }

#ifdef DBG_UTIL
            // check orientation (debug only)
            if(tools::getOrientation(aRetval) == ORIENTATION_NEGATIVE)
            {
                OSL_ENSURE(false, "createAreaGeometryForJoin: orientation of return value is negative (!)");
            }
#endif

            return aRetval;
        }
    } // end of anonymus namespace

    namespace tools
    {
        B2DPolyPolygon createAreaGeometryForPolygon(const B2DPolygon& rCandidate,
            double fHalfLineWidth,
            B2DLineJoin eJoin,
            double fDegreeStepWidth,
            double fMiterMinimumAngle)
        {
            OSL_ENSURE(fHalfLineWidth > 0.0, "createAreaGeometryForPolygon: LineWidth too small (!)");
            OSL_ENSURE(fDegreeStepWidth > 0.0, "createAreaGeometryForPolygon: DegreeStepWidth too small (!)");
            B2DPolyPolygon aRetval;
            const sal_uInt32 nCount(rCandidate.count());

            if(rCandidate.isClosed())
            {
                const bool bNeedToCreateJoinPolygon(B2DLINEJOIN_NONE != eJoin);
                bool bLastNeededToCreateJoinPolygon(false);

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    // get left, right, prev and next positions for edge
                    B2DPoint aEdgeStart(rCandidate.getB2DPoint(a));
                    B2DPoint aEdgeEnd(rCandidate.getB2DPoint((a + 1L) % nCount));

                    // create geometry for edge and add to result
                    B2DPolygon aEdgePolygon(createAreaGeometryForEdge(
                        aEdgeStart, aEdgeEnd, fHalfLineWidth,
                        bNeedToCreateJoinPolygon || bLastNeededToCreateJoinPolygon));
                    aRetval.append(aEdgePolygon);

                    if(bNeedToCreateJoinPolygon)
                    {
                        // create fill polygon for linejoin and add to result
                        B2DPoint aNextEdge(rCandidate.getB2DPoint((a + 2L) % nCount));
                        B2DPolygon aJoinPolygon(createAreaGeometryForJoin(
                            aEdgeStart, aEdgeEnd, aNextEdge, fHalfLineWidth, eJoin, fDegreeStepWidth, fMiterMinimumAngle));

                        if(aRetval.count())
                        {
                            aRetval.append(aJoinPolygon);
                        }
                    }

                    bLastNeededToCreateJoinPolygon = bNeedToCreateJoinPolygon;
                }
            }
            else if(nCount > 1L)
            {
                bool bLastNeededToCreateJoinPolygon(false);

                for(sal_uInt32 a(0L); a < nCount - 1L; a++)
                {
                    // get left, right positions for edge
                    B2DPoint aEdgeStart(rCandidate.getB2DPoint(a));
                    B2DPoint aEdgeEnd(rCandidate.getB2DPoint(a + 1L));
                    const bool bNeedToCreateJoinPolygon((a + 2L < nCount) && B2DLINEJOIN_NONE != eJoin);

                    // create geometry for edge and add to result
                    B2DPolygon aEdgePolygon(
                        createAreaGeometryForEdge(aEdgeStart, aEdgeEnd, fHalfLineWidth,
                        bNeedToCreateJoinPolygon || bLastNeededToCreateJoinPolygon));
                    aRetval.append(aEdgePolygon);

                    // test if next exists
                    if(bNeedToCreateJoinPolygon)
                    {
                        // create fill polygon for linejoin and add to result
                        B2DPoint aNextEdge(rCandidate.getB2DPoint((a + 2L)));
                        B2DPolygon aJoinPolygon(createAreaGeometryForJoin(
                            aEdgeStart, aEdgeEnd, aNextEdge, fHalfLineWidth, eJoin, fDegreeStepWidth, fMiterMinimumAngle));

                        if(aRetval.count())
                        {
                            aRetval.append(aJoinPolygon);
                        }
                    }

                    bLastNeededToCreateJoinPolygon = bNeedToCreateJoinPolygon;
                }
            }

            return aRetval;
        }

        B2DPolyPolygon createAreaGeometryForLineStartEnd(
            const B2DPolygon& rCandidate,
            const B2DPolyPolygon& rArrow,
            bool bStart,
            double fWidth,
            double fDockingPosition, // 0->top, 1->bottom
            double* pConsumedLength)
        {
            OSL_ENSURE(rCandidate.count() > 1L, "createAreaGeometryForLineStartEnd: Line polygon has too less points too small (!)");
            OSL_ENSURE(rArrow.count() > 0L, "createAreaGeometryForLineStartEnd: No arrow PolyPolygon (!)");
            OSL_ENSURE(fWidth > 0.0, "createAreaGeometryForLineStartEnd: Width too small (!)");
            OSL_ENSURE(fDockingPosition >= 0.0 && fDockingPosition <= 1.0,
                "createAreaGeometryForLineStartEnd: fDockingPosition out of range [0.0 .. 1.0] (!)");

            // init return value from arrow
            B2DPolyPolygon aRetval(rArrow);

            // get size of the arrow
            const B2DRange aArrowSize(getRange(rArrow));

            // build ArrowTransform
            B2DHomMatrix aArrowTransform;

            // center in X, align with axis in Y
            aArrowTransform.translate(-aArrowSize.getCenter().getX(), -aArrowSize.getMinimum().getY());

            // scale to target size
            const double fArrowScale(fWidth / (aArrowSize.getRange().getX()));
            aArrowTransform.scale(fArrowScale, fArrowScale);

            // get arrow size in Y
            B2DPoint aUpperCenter(aArrowSize.getCenter().getX(), aArrowSize.getMaximum().getY());
            aUpperCenter *= aArrowTransform;
            const double fArrowYLength(B2DVector(aUpperCenter).getLength());

            // move arrow to have docking position centered
            aArrowTransform.translate(0.0, -fArrowYLength * fDockingPosition);

            // get the polygon vector we want to plant this arrow on
            const double fCandidateLength(getLength(rCandidate));
            const double fConsumedLength(fArrowYLength * (1.0 - fDockingPosition));
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

            return aRetval;
        }
    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
