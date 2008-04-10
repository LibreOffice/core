/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dcubicbezier.cxx,v $
 * $Revision: 1.15 $
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
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <limits>

// #i37443#
#define FACTOR_FOR_UNSHARPEN    (1.6)
#ifdef DBG_UTIL
static double fMultFactUnsharpen = FACTOR_FOR_UNSHARPEN;
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        void ImpSubDivAngle(
            const B2DPoint& rfPA,           // start point
            const B2DPoint& rfEA,           // edge on A
            const B2DPoint& rfEB,           // edge on B
            const B2DPoint& rfPB,           // end point
            B2DPolygon& rTarget,            // target polygon
            double fAngleBound,             // angle bound in [0.0 .. 2PI]
            bool bAllowUnsharpen,           // #i37443# allow the criteria to get unsharp in recursions
            sal_uInt16 nMaxRecursionDepth)  // endless loop protection
        {
            if(nMaxRecursionDepth)
            {
                // do angle test
                B2DVector aLeft(rfEA - rfPA);
                B2DVector aRight(rfEB - rfPB);

                // #i72104#
                if(aLeft.equalZero())
                {
                    aLeft = rfEB - rfPA;
                }

                if(aRight.equalZero())
                {
                    aRight = rfEA - rfPB;
                }

                const double fCurrentAngle(aLeft.angle(aRight));

                if(fabs(fCurrentAngle) > (F_PI - fAngleBound))
                {
                    // end recursion
                    nMaxRecursionDepth = 0;
                }
                else
                {
                    if(bAllowUnsharpen)
                    {
                        // #i37443# unsharpen criteria
#ifdef DBG_UTIL
                        fAngleBound *= fMultFactUnsharpen;
#else
                        fAngleBound *= FACTOR_FOR_UNSHARPEN;
#endif
                    }
                }
            }

            if(nMaxRecursionDepth)
            {
                // divide at 0.5
                const B2DPoint aS1L(average(rfPA, rfEA));
                const B2DPoint aS1C(average(rfEA, rfEB));
                const B2DPoint aS1R(average(rfEB, rfPB));
                const B2DPoint aS2L(average(aS1L, aS1C));
                const B2DPoint aS2R(average(aS1C, aS1R));
                const B2DPoint aS3C(average(aS2L, aS2R));

                // left recursion
                ImpSubDivAngle(rfPA, aS1L, aS2L, aS3C, rTarget, fAngleBound, bAllowUnsharpen, nMaxRecursionDepth - 1);

                // right recursion
                ImpSubDivAngle(aS3C, aS2R, aS1R, rfPB, rTarget, fAngleBound, bAllowUnsharpen, nMaxRecursionDepth - 1);
            }
            else
            {
                rTarget.append(rfPB);
            }
        }

        void ImpSubDivAngleStart(
            const B2DPoint& rfPA,           // start point
            const B2DPoint& rfEA,           // edge on A
            const B2DPoint& rfEB,           // edge on B
            const B2DPoint& rfPB,           // end point
            B2DPolygon& rTarget,            // target polygon
            const double& rfAngleBound,     // angle bound in [0.0 .. 2PI]
            bool bAllowUnsharpen)           // #i37443# allow the criteria to get unsharp in recursions
        {
            sal_uInt16 nMaxRecursionDepth(8);
            const B2DVector aLeft(rfEA - rfPA);
            const B2DVector aRight(rfEB - rfPB);
            bool bLeftEqualZero(aLeft.equalZero());
            bool bRightEqualZero(aRight.equalZero());
            bool bAllParallel(false);

            if(bLeftEqualZero && bRightEqualZero)
            {
                nMaxRecursionDepth = 0;
            }
            else
            {
                const B2DVector aBase(rfPB - rfPA);
                const bool bBaseEqualZero(aBase.equalZero()); // #i72104#

                if(!bBaseEqualZero)
                {
                    const bool bLeftParallel(bLeftEqualZero ? true : areParallel(aLeft, aBase));
                    const bool bRightParallel(bRightEqualZero ? true : areParallel(aRight, aBase));

                    if(bLeftParallel && bRightParallel)
                    {
                        bAllParallel = true;

                        if(!bLeftEqualZero)
                        {
                            double fFactor;

                            if(fabs(aBase.getX()) > fabs(aBase.getY()))
                            {
                                fFactor = aLeft.getX() / aBase.getX();
                            }
                            else
                            {
                                fFactor = aLeft.getY() / aBase.getY();
                            }

                            if(fFactor >= 0.0 && fFactor <= 1.0)
                            {
                                bLeftEqualZero = true;
                            }
                        }

                        if(!bRightEqualZero)
                        {
                            double fFactor;

                            if(fabs(aBase.getX()) > fabs(aBase.getY()))
                            {
                                fFactor = aRight.getX() / -aBase.getX();
                            }
                            else
                            {
                                fFactor = aRight.getY() / -aBase.getY();
                            }

                            if(fFactor >= 0.0 && fFactor <= 1.0)
                            {
                                bRightEqualZero = true;
                            }
                        }

                        if(bLeftEqualZero && bRightEqualZero)
                        {
                            nMaxRecursionDepth = 0;
                        }
                    }
                }
            }

            if(nMaxRecursionDepth)
            {
                // divide at 0.5 ad test both edges for angle criteria
                const B2DPoint aS1L(average(rfPA, rfEA));
                const B2DPoint aS1C(average(rfEA, rfEB));
                const B2DPoint aS1R(average(rfEB, rfPB));
                const B2DPoint aS2L(average(aS1L, aS1C));
                const B2DPoint aS2R(average(aS1C, aS1R));
                const B2DPoint aS3C(average(aS2L, aS2R));

                // test left
                bool bAngleIsSmallerLeft(bAllParallel && bLeftEqualZero);
                if(!bAngleIsSmallerLeft)
                {
                    const B2DVector aLeftLeft(bLeftEqualZero ? aS2L - aS1L : aS1L - rfPA); // #i72104#
                    const B2DVector aRightLeft(aS2L - aS3C);
                    const double fCurrentAngleLeft(aLeftLeft.angle(aRightLeft));
                    bAngleIsSmallerLeft = (fabs(fCurrentAngleLeft) > (F_PI - rfAngleBound));
                }

                // test right
                bool bAngleIsSmallerRight(bAllParallel && bRightEqualZero);
                if(!bAngleIsSmallerRight)
                {
                    const B2DVector aLeftRight(aS2R - aS3C);
                    const B2DVector aRightRight(bRightEqualZero ? aS2R - aS1R : aS1R - rfPB); // #i72104#
                    const double fCurrentAngleRight(aLeftRight.angle(aRightRight));
                    bAngleIsSmallerRight = (fabs(fCurrentAngleRight) > (F_PI - rfAngleBound));
                }

                if(bAngleIsSmallerLeft && bAngleIsSmallerRight)
                {
                    // no recursion necessary at all
                    nMaxRecursionDepth = 0;
                }
                else
                {
                    // left
                    if(bAngleIsSmallerLeft)
                    {
                        rTarget.append(aS3C);
                    }
                    else
                    {
                        ImpSubDivAngle(rfPA, aS1L, aS2L, aS3C, rTarget, rfAngleBound, bAllowUnsharpen, nMaxRecursionDepth);
                    }

                    // right
                    if(bAngleIsSmallerRight)
                    {
                        rTarget.append(rfPB);
                    }
                    else
                    {
                        ImpSubDivAngle(aS3C, aS2R, aS1R, rfPB, rTarget, rfAngleBound, bAllowUnsharpen, nMaxRecursionDepth);
                    }
                }
            }

            if(!nMaxRecursionDepth)
            {
                rTarget.append(rfPB);
            }
        }

        void ImpSubDivDistance(
            const B2DPoint& rfPA,           // start point
            const B2DPoint& rfEA,           // edge on A
            const B2DPoint& rfEB,           // edge on B
            const B2DPoint& rfPB,           // end point
            B2DPolygon& rTarget,            // target polygon
            double fDistanceBound2,         // quadratic distance criteria
            double fLastDistanceError2,     // the last quadratic distance error
            sal_uInt16 nMaxRecursionDepth)  // endless loop protection
        {
            if(nMaxRecursionDepth)
            {
                // decide if another recursion is needed. If not, set
                // nMaxRecursionDepth to zero

                // Perform bezier flatness test (lecture notes from R. Schaback,
                // Mathematics of Computer-Aided Design, Uni Goettingen, 2000)
                //
                // ||P(t) - L(t)|| <= max     ||b_j - b_0 - j/n(b_n - b_0)||
                //                    0<=j<=n
                //
                // What is calculated here is an upper bound to the distance from
                // a line through b_0 and b_3 (rfPA and P4 in our notation) and the
                // curve. We can drop 0 and n from the running indices, since the
                // argument of max becomes zero for those cases.
                const double fJ1x(rfEA.getX() - rfPA.getX() - 1.0/3.0*(rfPB.getX() - rfPA.getX()));
                const double fJ1y(rfEA.getY() - rfPA.getY() - 1.0/3.0*(rfPB.getY() - rfPA.getY()));
                const double fJ2x(rfEB.getX() - rfPA.getX() - 2.0/3.0*(rfPB.getX() - rfPA.getX()));
                const double fJ2y(rfEB.getY() - rfPA.getY() - 2.0/3.0*(rfPB.getY() - rfPA.getY()));
                const double fDistanceError2(::std::max(fJ1x*fJ1x + fJ1y*fJ1y, fJ2x*fJ2x + fJ2y*fJ2y));

                // stop if error measure does not improve anymore. This is a
                // safety guard against floating point inaccuracies.
                // stop if distance from line is guaranteed to be bounded by d
                const bool bFurtherDivision(fLastDistanceError2 > fDistanceError2 && fDistanceError2 >= fDistanceBound2);

                if(bFurtherDivision)
                {
                    // remember last error value
                    fLastDistanceError2 = fDistanceError2;
                }
                else
                {
                    // stop recustion
                    nMaxRecursionDepth = 0;
                }
            }

            if(nMaxRecursionDepth)
            {
                // divide at 0.5
                const B2DPoint aS1L(average(rfPA, rfEA));
                const B2DPoint aS1C(average(rfEA, rfEB));
                const B2DPoint aS1R(average(rfEB, rfPB));
                const B2DPoint aS2L(average(aS1L, aS1C));
                const B2DPoint aS2R(average(aS1C, aS1R));
                const B2DPoint aS3C(average(aS2L, aS2R));

                // left recursion
                ImpSubDivDistance(rfPA, aS1L, aS2L, aS3C, rTarget, fDistanceBound2, fLastDistanceError2, nMaxRecursionDepth - 1);

                // right recursion
                ImpSubDivDistance(aS3C, aS2R, aS1R, rfPB, rTarget, fDistanceBound2, fLastDistanceError2, nMaxRecursionDepth - 1);
            }
            else
            {
                rTarget.append(rfPB);
            }
        }
    } // end of anonymous namespace
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    B2DCubicBezier::B2DCubicBezier(const B2DCubicBezier& rBezier)
    :   maStartPoint(rBezier.maStartPoint),
        maEndPoint(rBezier.maEndPoint),
        maControlPointA(rBezier.maControlPointA),
        maControlPointB(rBezier.maControlPointB)
    {
    }

    B2DCubicBezier::B2DCubicBezier()
    {
    }

    B2DCubicBezier::B2DCubicBezier(const B2DPoint& rStart, const B2DPoint& rEnd)
    :   maStartPoint(rStart),
        maEndPoint(rEnd),
        maControlPointA(rStart),
        maControlPointB(rEnd)
    {
    }

    B2DCubicBezier::B2DCubicBezier(const B2DPoint& rStart, const B2DPoint& rControlPointA, const B2DPoint& rControlPointB, const B2DPoint& rEnd)
    :   maStartPoint(rStart),
        maEndPoint(rEnd),
        maControlPointA(rControlPointA),
        maControlPointB(rControlPointB)
    {
    }

    B2DCubicBezier::~B2DCubicBezier()
    {
    }

    // assignment operator
    B2DCubicBezier& B2DCubicBezier::operator=(const B2DCubicBezier& rBezier)
    {
        maStartPoint = rBezier.maStartPoint;
        maEndPoint = rBezier.maEndPoint;
        maControlPointA = rBezier.maControlPointA;
        maControlPointB = rBezier.maControlPointB;

        return *this;
    }

    // compare operators
    bool B2DCubicBezier::operator==(const B2DCubicBezier& rBezier) const
    {
        return (
            maStartPoint == rBezier.maStartPoint
            && maEndPoint == rBezier.maEndPoint
            && maControlPointA == rBezier.maControlPointA
            && maControlPointB == rBezier.maControlPointB
        );
    }

    bool B2DCubicBezier::operator!=(const B2DCubicBezier& rBezier) const
    {
        return (
            maStartPoint != rBezier.maStartPoint
            || maEndPoint != rBezier.maEndPoint
            || maControlPointA != rBezier.maControlPointA
            || maControlPointB != rBezier.maControlPointB
        );
    }

    // test if vectors are used
    bool B2DCubicBezier::isBezier() const
    {
        if(maControlPointA != maStartPoint || maControlPointB != maEndPoint)
        {
            return true;
        }

        return false;
    }

    void B2DCubicBezier::testAndSolveTrivialBezier()
    {
        if(maControlPointA != maStartPoint || maControlPointB != maEndPoint)
        {
            const B2DVector aEdge(maEndPoint - maStartPoint);

            // controls parallel to edge can be trivial. No edge -> not parallel -> control can not be trivial
            if(!aEdge.equalZero())
            {
                const B2DVector aVecA(maControlPointA - maStartPoint);
                const B2DVector aVecB(maControlPointB - maEndPoint);
                const bool bAIsZero(aVecA.equalZero());
                const bool bBIsZero(aVecB.equalZero());
                bool bACanBeZero(false);
                bool bBCanBeZero(false);

                if(!bAIsZero)
                {
                    // parallel to edge?
                    if(areParallel(aVecA, aEdge))
                    {
                        // get scale to edge
                        const double fScale(fabs(aEdge.getX()) > fabs(aEdge.getY()) ? aVecA.getX() / aEdge.getX() : aVecA.getY() / aEdge.getY());

                        // end point of vector in edge range?
                        if(fTools::more(fScale, 0.0) && fTools::lessOrEqual(fScale, 1.0))
                        {
                            bACanBeZero = true;
                        }
                    }
                }

                if(!bBIsZero)
                {
                    // parallel to edge?
                    if(areParallel(aVecB, aEdge))
                    {
                        // get scale to edge
                        const double fScale(fabs(aEdge.getX()) > fabs(aEdge.getY()) ? aVecB.getX() / aEdge.getX() : aVecB.getY() / aEdge.getY());

                        // end point of vector in edge range? Caution: controlB is directed AGAINST edge
                        if(fTools::less(fScale, 0.0) && fTools::moreOrEqual(fScale, -1.0))
                        {
                            bBCanBeZero = true;
                        }
                    }
                }

                // if both are/can be reduced, do it.
                // Not possible if only one is/can be reduced (!)
                if((bAIsZero || bACanBeZero) && (bBIsZero || bBCanBeZero))
                {
                    if(!bAIsZero)
                    {
                        maControlPointA = maStartPoint;
                    }

                    if(!bBIsZero)
                    {
                        maControlPointB = maEndPoint;
                    }
                }
            }
        }
    }

    double B2DCubicBezier::getEdgeLength() const
    {
        const B2DVector aEdge(maEndPoint - maStartPoint);
        return aEdge.getLength();
    }

    double B2DCubicBezier::getControlPolygonLength() const
    {
        const B2DVector aVectorA(maControlPointA - maStartPoint);
        const B2DVector aVectorB(maEndPoint - maControlPointB);

        if(!aVectorA.equalZero() || !aVectorB.equalZero())
        {
            const B2DVector aTop(maControlPointB - maControlPointA);
            return (aVectorA.getLength() + aVectorB.getLength() + aTop.getLength());
        }
        else
        {
            return getEdgeLength();
        }
    }

    void B2DCubicBezier::adaptiveSubdivideByAngle(B2DPolygon& rTarget, double fAngleBound, bool bAllowUnsharpen) const
    {
        if(isBezier())
        {
            // use support method #i37443# and allow unsharpen the criteria
            ImpSubDivAngleStart(maStartPoint, maControlPointA, maControlPointB, maEndPoint, rTarget, fAngleBound * F_PI180, bAllowUnsharpen);
        }
        else
        {
            rTarget.append(getEndPoint());
        }
    }

    // #i37443# adaptive subdivide by nCount subdivisions
    void B2DCubicBezier::adaptiveSubdivideByCount(B2DPolygon& rTarget, sal_uInt32 nCount) const
    {
        for(sal_uInt32 a(0L); a < nCount; a++)
        {
            const double fPos(double(a + 1L) / double(nCount + 1L));
            rTarget.append(interpolatePoint(fPos));
        }

        rTarget.append(getEndPoint());
    }

    // adaptive subdivide by distance
    void B2DCubicBezier::adaptiveSubdivideByDistance(B2DPolygon& rTarget, double fDistanceBound) const
    {
        if(isBezier())
        {
            ImpSubDivDistance(maStartPoint, maControlPointA, maControlPointB, maEndPoint, rTarget,
                fDistanceBound * fDistanceBound, ::std::numeric_limits<double>::max(), 30);
        }
        else
        {
            rTarget.append(getEndPoint());
        }
    }

    B2DPoint B2DCubicBezier::interpolatePoint(double t) const
    {
        OSL_ENSURE(t >= 0.0 && t <= 1.0, "B2DCubicBezier::interpolatePoint: Access out of range (!)");

        if(isBezier())
        {
            const B2DPoint aS1L(interpolate(maStartPoint, maControlPointA, t));
            const B2DPoint aS1C(interpolate(maControlPointA, maControlPointB, t));
            const B2DPoint aS1R(interpolate(maControlPointB, maEndPoint, t));
            const B2DPoint aS2L(interpolate(aS1L, aS1C, t));
            const B2DPoint aS2R(interpolate(aS1C, aS1R, t));

            return interpolate(aS2L, aS2R, t);
        }
        else
        {
            return interpolate(maStartPoint, maEndPoint, t);
        }
    }

    double B2DCubicBezier::getSmallestDistancePointToBezierSegment(const B2DPoint& rTestPoint, double& rCut) const
    {
        const sal_uInt32 nInitialDivisions(3L);
        B2DPolygon aInitialPolygon;

        // as start make a fix division, creates nInitialDivisions + 2L points
        aInitialPolygon.append(getStartPoint());
        adaptiveSubdivideByCount(aInitialPolygon, nInitialDivisions);

        // now look for the closest point
        const sal_uInt32 nPointCount(aInitialPolygon.count());
        B2DVector aVector(rTestPoint - aInitialPolygon.getB2DPoint(0L));
        double fQuadDist(aVector.getX() * aVector.getX() + aVector.getY() * aVector.getY());
        double fNewQuadDist;
        sal_uInt32 nSmallestIndex(0L);

        for(sal_uInt32 a(1L); a < nPointCount; a++)
        {
            aVector = B2DVector(rTestPoint - aInitialPolygon.getB2DPoint(a));
            fNewQuadDist = aVector.getX() * aVector.getX() + aVector.getY() * aVector.getY();

            if(fNewQuadDist < fQuadDist)
            {
                fQuadDist = fNewQuadDist;
                nSmallestIndex = a;
            }
        }

        // look right and left for even smaller distances
        double fStepValue(1.0 / (double)((nPointCount - 1L) * 2L)); // half the edge step width
        double fPosition((double)nSmallestIndex / (double)(nPointCount - 1L));
        bool bDone(false);

        while(!bDone)
        {
            if(!bDone)
            {
                // test left
                double fPosLeft(fPosition - fStepValue);

                if(fPosLeft < 0.0)
                {
                    fPosLeft = 0.0;
                    aVector = B2DVector(rTestPoint - maStartPoint);
                }
                else
                {
                    aVector = B2DVector(rTestPoint - interpolatePoint(fPosLeft));
                }

                fNewQuadDist = aVector.getX() * aVector.getX() + aVector.getY() * aVector.getY();

                if(fTools::less(fNewQuadDist, fQuadDist))
                {
                    fQuadDist = fNewQuadDist;
                    fPosition = fPosLeft;
                }
                else
                {
                    // test right
                    double fPosRight(fPosition + fStepValue);

                    if(fPosRight > 1.0)
                    {
                        fPosRight = 1.0;
                        aVector = B2DVector(rTestPoint - maEndPoint);
                    }
                    else
                    {
                        aVector = B2DVector(rTestPoint - interpolatePoint(fPosRight));
                    }

                    fNewQuadDist = aVector.getX() * aVector.getX() + aVector.getY() * aVector.getY();

                    if(fTools::less(fNewQuadDist, fQuadDist))
                    {
                        fQuadDist = fNewQuadDist;
                        fPosition = fPosRight;
                    }
                    else
                    {
                        // not less left or right, done
                        bDone = true;
                    }
                }
            }

            if(0.0 == fPosition || 1.0 == fPosition)
            {
                // if we are completely left or right, we are done
                bDone = true;
            }

            if(!bDone)
            {
                // prepare next step value
                fStepValue /= 2.0;
            }
        }

        rCut = fPosition;
        return sqrt(fQuadDist);
    }

    void B2DCubicBezier::split(double t, B2DCubicBezier& rBezierA, B2DCubicBezier& rBezierB) const
    {
        OSL_ENSURE(t >= 0.0 && t <= 1.0, "B2DCubicBezier::split: Access out of range (!)");

        if(isBezier())
        {
            const B2DPoint aS1L(interpolate(maStartPoint, maControlPointA, t));
            const B2DPoint aS1C(interpolate(maControlPointA, maControlPointB, t));
            const B2DPoint aS1R(interpolate(maControlPointB, maEndPoint, t));
            const B2DPoint aS2L(interpolate(aS1L, aS1C, t));
            const B2DPoint aS2R(interpolate(aS1C, aS1R, t));
            const B2DPoint aS3C(interpolate(aS2L, aS2R, t));

            rBezierA.setStartPoint(maStartPoint);
            rBezierA.setEndPoint(aS3C);
            rBezierA.setControlPointA(aS1L);
            rBezierA.setControlPointB(aS2L);

            rBezierB.setStartPoint(aS3C);
            rBezierB.setEndPoint(maEndPoint);
            rBezierB.setControlPointA(aS2R);
            rBezierB.setControlPointB(aS1R);
        }
        else
        {
            const B2DPoint aSplit(interpolate(maStartPoint, maEndPoint, t));

            rBezierA.setStartPoint(maStartPoint);
            rBezierA.setEndPoint(aSplit);
            rBezierA.setControlPointA(maStartPoint);
            rBezierA.setControlPointB(aSplit);

            rBezierB.setStartPoint(aSplit);
            rBezierB.setEndPoint(maEndPoint);
            rBezierB.setControlPointA(aSplit);
            rBezierB.setControlPointB(maEndPoint);
        }
    }

    B2DRange B2DCubicBezier::getRange() const
    {
        B2DRange aRetval(maStartPoint, maEndPoint);

        aRetval.expand(maControlPointA);
        aRetval.expand(maControlPointB);

        return aRetval;
    }
} // end of namespace basegfx

// eof
