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

#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <osl/diagnose.h>

#include <limits>

// #i37443#
#define FACTOR_FOR_UNSHARPEN    (1.6)
#ifdef DBG_UTIL
static const double fMultFactUnsharpen = FACTOR_FOR_UNSHARPEN;
#endif

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
            const double& rfAngleBound)     // angle bound in [0.0 .. 2PI]
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
                    const bool bLeftParallel(bLeftEqualZero || areParallel(aLeft, aBase));
                    const bool bRightParallel(bRightEqualZero || areParallel(aRight, aBase));

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
                        ImpSubDivAngle(rfPA, aS1L, aS2L, aS3C, rTarget, rfAngleBound, true/*bAllowUnsharpen*/, nMaxRecursionDepth);
                    }

                    // right
                    if(bAngleIsSmallerRight)
                    {
                        rTarget.append(rfPB);
                    }
                    else
                    {
                        ImpSubDivAngle(aS3C, aS2R, aS1R, rfPB, rTarget, rfAngleBound, true/*bAllowUnsharpen*/, nMaxRecursionDepth);
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

                // ||P(t) - L(t)|| <= max     ||b_j - b_0 - j/n(b_n - b_0)||
                //                    0<=j<=n

                // What is calculated here is an upper bound to the distance from
                // a line through b_0 and b_3 (rfPA and P4 in our notation) and the
                // curve. We can drop 0 and n from the running indices, since the
                // argument of max becomes zero for those cases.
                const double fJ1x(rfEA.getX() - rfPA.getX() - 1.0/3.0*(rfPB.getX() - rfPA.getX()));
                const double fJ1y(rfEA.getY() - rfPA.getY() - 1.0/3.0*(rfPB.getY() - rfPA.getY()));
                const double fJ2x(rfEB.getX() - rfPA.getX() - 2.0/3.0*(rfPB.getX() - rfPA.getX()));
                const double fJ2y(rfEB.getY() - rfPA.getY() - 2.0/3.0*(rfPB.getY() - rfPA.getY()));
                const double fDistanceError2(std::max(fJ1x*fJ1x + fJ1y*fJ1y, fJ2x*fJ2x + fJ2y*fJ2y));

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
                    // stop recursion
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

namespace basegfx
{
    B2DCubicBezier::B2DCubicBezier(const B2DCubicBezier&) = default;

    B2DCubicBezier::B2DCubicBezier() = default;

    B2DCubicBezier::B2DCubicBezier(const B2DPoint& rStart, const B2DPoint& rControlPointA, const B2DPoint& rControlPointB, const B2DPoint& rEnd)
    :   maStartPoint(rStart),
        maEndPoint(rEnd),
        maControlPointA(rControlPointA),
        maControlPointB(rControlPointB)
    {
    }

    B2DCubicBezier::~B2DCubicBezier() = default;

    // assignment operator
    B2DCubicBezier& B2DCubicBezier::operator=(const B2DCubicBezier&) = default;

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

    bool B2DCubicBezier::equal(const B2DCubicBezier& rBezier) const
    {
        return (
            maStartPoint.equal(rBezier.maStartPoint)
            && maEndPoint.equal(rBezier.maEndPoint)
            && maControlPointA.equal(rBezier.maControlPointA)
            && maControlPointB.equal(rBezier.maControlPointB)
        );
    }

    // test if vectors are used
    bool B2DCubicBezier::isBezier() const
    {
        return maControlPointA != maStartPoint || maControlPointB != maEndPoint;
    }

    void B2DCubicBezier::testAndSolveTrivialBezier()
    {
        if(maControlPointA != maStartPoint || maControlPointB != maEndPoint)
        {
            const B2DVector aEdge(maEndPoint - maStartPoint);

            // controls parallel to edge can be trivial. No edge -> not parallel -> control can
            // still not be trivial (e.g. ballon loop)
            if(!aEdge.equalZero())
            {
                // get control vectors
                const B2DVector aVecA(maControlPointA - maStartPoint);
                const B2DVector aVecB(maControlPointB - maEndPoint);

                // check if trivial per se
                bool bAIsTrivial(aVecA.equalZero());
                bool bBIsTrivial(aVecB.equalZero());

                // #i102241# prepare inverse edge length to normalize cross values;
                // else the small compare value used in fTools::equalZero
                // will be length dependent and this detection will work as less
                // precise as longer the edge is. In principle, the length of the control
                // vector would need to be used too, but to be trivial it is assumed to
                // be of roughly equal length to the edge, so edge length can be used
                // for both. Only needed when one of both is not trivial per se.
                const double fInverseEdgeLength(bAIsTrivial && bBIsTrivial
                    ? 1.0
                    : 1.0 / aEdge.getLength());

                // if A is not zero, check if it could be
                if(!bAIsTrivial)
                {
                    // #i102241# parallel to edge? Check aVecA, aEdge. Use cross() which does what
                    // we need here with the precision we need
                    const double fCross(aVecA.cross(aEdge) * fInverseEdgeLength);

                    if(fTools::equalZero(fCross))
                    {
                        // get scale to edge. Use bigger distance for numeric quality
                        const double fScale(fabs(aEdge.getX()) > fabs(aEdge.getY())
                            ? aVecA.getX() / aEdge.getX()
                            : aVecA.getY() / aEdge.getY());

                        // relative end point of vector in edge range?
                        if (fTools::betweenOrEqualEither(fScale, 0.0, 1.0))
                        {
                            bAIsTrivial = true;
                        }
                    }
                }

                // if B is not zero, check if it could be, but only if A is already trivial;
                // else solve to trivial will not be possible for whole edge
                if(bAIsTrivial && !bBIsTrivial)
                {
                    // parallel to edge? Check aVecB, aEdge
                    const double fCross(aVecB.cross(aEdge) * fInverseEdgeLength);

                    if(fTools::equalZero(fCross))
                    {
                        // get scale to edge. Use bigger distance for numeric quality
                        const double fScale(fabs(aEdge.getX()) > fabs(aEdge.getY())
                            ? aVecB.getX() / aEdge.getX()
                            : aVecB.getY() / aEdge.getY());

                        // end point of vector in edge range? Caution: controlB is directed AGAINST edge
                        if (fTools::betweenOrEqualEither(fScale, -1.0, 0.0))
                        {
                            bBIsTrivial = true;
                        }
                    }
                }

                // if both are/can be reduced, do it.
                // Not possible if only one is/can be reduced (!)
                if(bAIsTrivial && bBIsTrivial)
                {
                    maControlPointA = maStartPoint;
                    maControlPointB = maEndPoint;
                }
            }
        }
    }

    namespace {
        double impGetLength(const B2DCubicBezier& rEdge, double fDeviation, sal_uInt32 nRecursionWatch)
        {
            const double fEdgeLength(rEdge.getEdgeLength());
            const double fControlPolygonLength(rEdge.getControlPolygonLength());
            const double fCurrentDeviation(fTools::equalZero(fControlPolygonLength) ? 0.0 : 1.0 - (fEdgeLength / fControlPolygonLength));

            if(!nRecursionWatch || fTools:: lessOrEqual(fCurrentDeviation, fDeviation))
            {
                return (fEdgeLength + fControlPolygonLength) * 0.5;
            }
            else
            {
                B2DCubicBezier aLeft, aRight;
                const double fNewDeviation(fDeviation * 0.5);
                const sal_uInt32 nNewRecursionWatch(nRecursionWatch - 1);

                rEdge.split(0.5, &aLeft, &aRight);

                return impGetLength(aLeft, fNewDeviation, nNewRecursionWatch)
                    + impGetLength(aRight, fNewDeviation, nNewRecursionWatch);
            }
        }
    }

    double B2DCubicBezier::getLength(double fDeviation) const
    {
        if(isBezier())
        {
            if(fDeviation < 0.00000001)
            {
                fDeviation = 0.00000001;
            }

            return impGetLength(*this, fDeviation, 6);
        }
        else
        {
            return B2DVector(getEndPoint() - getStartPoint()).getLength();
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

    void B2DCubicBezier::adaptiveSubdivideByAngle(B2DPolygon& rTarget, double fAngleBound) const
    {
        if(isBezier())
        {
            // use support method #i37443# and allow unsharpen the criteria
            ImpSubDivAngleStart(maStartPoint, maControlPointA, maControlPointB, maEndPoint, rTarget,
                                deg2rad(fAngleBound));
        }
        else
        {
            rTarget.append(getEndPoint());
        }
    }

    B2DVector B2DCubicBezier::getTangent(double t) const
    {
        if(fTools::lessOrEqual(t, 0.0))
        {
            // tangent in start point
            B2DVector aTangent(getControlPointA() - getStartPoint());

            if(!aTangent.equalZero())
            {
                return aTangent;
            }

            // start point and control vector are the same, fallback
            // to implicit start vector to control point B
            aTangent = (getControlPointB() - getStartPoint()) * 0.3;

            if(!aTangent.equalZero())
            {
                return aTangent;
            }

            // not a bezier at all, return edge vector
            return (getEndPoint() - getStartPoint()) * 0.3;
        }
        else if(fTools::moreOrEqual(t, 1.0))
        {
            // tangent in end point
            B2DVector aTangent(getEndPoint() - getControlPointB());

            if(!aTangent.equalZero())
            {
                return aTangent;
            }

            // end point and control vector are the same, fallback
            // to implicit start vector from control point A
            aTangent = (getEndPoint() - getControlPointA()) * 0.3;

            if(!aTangent.equalZero())
            {
                return aTangent;
            }

            // not a bezier at all, return edge vector
            return (getEndPoint() - getStartPoint()) * 0.3;
        }
        else
        {
            // t is in ]0.0 .. 1.0[. Split and extract
            B2DCubicBezier aRight;
            split(t, nullptr, &aRight);

            return aRight.getControlPointA() - aRight.getStartPoint();
        }
    }

    // #i37443# adaptive subdivide by nCount subdivisions
    void B2DCubicBezier::adaptiveSubdivideByCount(B2DPolygon& rTarget, sal_uInt32 nCount) const
    {
        const double fLenFact(1.0 / static_cast< double >(nCount + 1));

        for(sal_uInt32 a(1); a <= nCount; a++)
        {
            const double fPos(static_cast< double >(a) * fLenFact);
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
                fDistanceBound * fDistanceBound, std::numeric_limits<double>::max(), 30);
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
        const sal_uInt32 nInitialDivisions(3);
        B2DPolygon aInitialPolygon;

        // as start make a fix division, creates nInitialDivisions + 2 points
        aInitialPolygon.append(getStartPoint());
        adaptiveSubdivideByCount(aInitialPolygon, nInitialDivisions);

        // now look for the closest point
        const sal_uInt32 nPointCount(aInitialPolygon.count());
        B2DVector aVector(rTestPoint - aInitialPolygon.getB2DPoint(0));
        double fQuadDist(aVector.getX() * aVector.getX() + aVector.getY() * aVector.getY());
        double fNewQuadDist;
        sal_uInt32 nSmallestIndex(0);

        for(sal_uInt32 a(1); a < nPointCount; a++)
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
        double fStepValue(1.0 / static_cast<double>((nPointCount - 1) * 2)); // half the edge step width
        double fPosition(static_cast<double>(nSmallestIndex) / static_cast<double>(nPointCount - 1));

        while(true)
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
                    break;
                }
            }

            if(fPosition == 0.0 || fPosition == 1.0)
            {
                // if we are completely left or right, we are done
                break;
            }

            // prepare next step value
            fStepValue /= 2.0;
        }

        rCut = fPosition;
        return sqrt(fQuadDist);
    }

    void B2DCubicBezier::split(double t, B2DCubicBezier* pBezierA, B2DCubicBezier* pBezierB) const
    {
        OSL_ENSURE(t >= 0.0 && t <= 1.0, "B2DCubicBezier::split: Access out of range (!)");

        if(!pBezierA && !pBezierB)
        {
            return;
        }

        if(isBezier())
        {
            const B2DPoint aS1L(interpolate(maStartPoint, maControlPointA, t));
            const B2DPoint aS1C(interpolate(maControlPointA, maControlPointB, t));
            const B2DPoint aS1R(interpolate(maControlPointB, maEndPoint, t));
            const B2DPoint aS2L(interpolate(aS1L, aS1C, t));
            const B2DPoint aS2R(interpolate(aS1C, aS1R, t));
            const B2DPoint aS3C(interpolate(aS2L, aS2R, t));

            if(pBezierA)
            {
                pBezierA->setStartPoint(maStartPoint);
                pBezierA->setEndPoint(aS3C);
                pBezierA->setControlPointA(aS1L);
                pBezierA->setControlPointB(aS2L);
            }

            if(pBezierB)
            {
                pBezierB->setStartPoint(aS3C);
                pBezierB->setEndPoint(maEndPoint);
                pBezierB->setControlPointA(aS2R);
                pBezierB->setControlPointB(aS1R);
            }
        }
        else
        {
            const B2DPoint aSplit(interpolate(maStartPoint, maEndPoint, t));

            if(pBezierA)
            {
                pBezierA->setStartPoint(maStartPoint);
                pBezierA->setEndPoint(aSplit);
                pBezierA->setControlPointA(maStartPoint);
                pBezierA->setControlPointB(aSplit);
            }

            if(pBezierB)
            {
                pBezierB->setStartPoint(aSplit);
                pBezierB->setEndPoint(maEndPoint);
                pBezierB->setControlPointA(aSplit);
                pBezierB->setControlPointB(maEndPoint);
            }
        }
    }

    B2DCubicBezier B2DCubicBezier::snippet(double fStart, double fEnd) const
    {
        B2DCubicBezier aRetval;

        if(fTools::more(fStart, 1.0))
        {
            fStart = 1.0;
        }
        else if(fTools::less(fStart, 0.0))
        {
            fStart = 0.0;
        }

        if(fTools::more(fEnd, 1.0))
        {
            fEnd = 1.0;
        }
        else if(fTools::less(fEnd, 0.0))
        {
            fEnd = 0.0;
        }

        if(fEnd <= fStart)
        {
            // empty or NULL, create single point at center
            const double fSplit((fEnd + fStart) * 0.5);
            const B2DPoint aPoint(interpolate(getStartPoint(), getEndPoint(), fSplit));
            aRetval.setStartPoint(aPoint);
            aRetval.setEndPoint(aPoint);
            aRetval.setControlPointA(aPoint);
            aRetval.setControlPointB(aPoint);
        }
        else
        {
            if(isBezier())
            {
                // copy bezier; cut off right, then cut off left. Do not forget to
                // adapt cut value when both cuts happen
                const bool bEndIsOne(fTools::equal(fEnd, 1.0));
                const bool bStartIsZero(fTools::equalZero(fStart));
                aRetval = *this;

                if(!bEndIsOne)
                {
                    aRetval.split(fEnd, &aRetval, nullptr);

                    if(!bStartIsZero)
                    {
                        fStart /= fEnd;
                    }
                }

                if(!bStartIsZero)
                {
                    aRetval.split(fStart, nullptr, &aRetval);
                }
            }
            else
            {
                // no bezier, create simple edge
                const B2DPoint aPointA(interpolate(getStartPoint(), getEndPoint(), fStart));
                const B2DPoint aPointB(interpolate(getStartPoint(), getEndPoint(), fEnd));
                aRetval.setStartPoint(aPointA);
                aRetval.setEndPoint(aPointB);
                aRetval.setControlPointA(aPointA);
                aRetval.setControlPointB(aPointB);
            }
        }

        return aRetval;
    }

    B2DRange B2DCubicBezier::getRange() const
    {
        B2DRange aRetval(maStartPoint, maEndPoint);

        aRetval.expand(maControlPointA);
        aRetval.expand(maControlPointB);

        return aRetval;
    }

    bool B2DCubicBezier::getMinimumExtremumPosition(double& rfResult) const
    {
        std::vector< double > aAllResults;

        aAllResults.reserve(4);
        getAllExtremumPositions(aAllResults);

        const sal_uInt32 nCount(aAllResults.size());

        if(!nCount)
        {
            return false;
        }
        else if(nCount == 1)
        {
            rfResult = aAllResults[0];
            return true;
        }
        else
        {
            rfResult = *(std::min_element(aAllResults.begin(), aAllResults.end()));
            return true;
        }
    }

    namespace
    {
        void impCheckExtremumResult(double fCandidate, std::vector< double >& rResult)
        {
            // check for range ]0.0 .. 1.0[ with excluding 1.0 and 0.0 clearly
            // by using the equalZero test, NOT ::more or ::less which will use the
            // ApproxEqual() which is too exact here
            if(fCandidate > 0.0 && !fTools::equalZero(fCandidate))
            {
                if(fCandidate < 1.0 && !fTools::equalZero(fCandidate - 1.0))
                {
                    rResult.push_back(fCandidate);
                }
            }
        }
    }

    void B2DCubicBezier::getAllExtremumPositions(std::vector< double >& rResults) const
    {
        rResults.clear();

        // calculate the x-extrema parameters by zeroing first x-derivative
        // of the cubic bezier's parametric formula, which results in a
        // quadratic equation: dBezier/dt = t*t*fAX - 2*t*fBX + fCX
        const B2DPoint aControlDiff( maControlPointA - maControlPointB );
        double fCX = maControlPointA.getX() - maStartPoint.getX();
        const double fBX = fCX + aControlDiff.getX();
        const double fAX = 3 * aControlDiff.getX() + (maEndPoint.getX() - maStartPoint.getX());

        if(fTools::equalZero(fCX))
        {
            // detect fCX equal zero and truncate to real zero value in that case
            fCX = 0.0;
        }

        if( !fTools::equalZero(fAX) )
        {
            // derivative is polynomial of order 2 => use binomial formula
            const double fD = fBX*fBX - fAX*fCX;
            if( fD >= 0.0 )
            {
                const double fS = sqrt(fD);
                // calculate both roots (avoiding a numerically unstable subtraction)
                const double fQ = fBX + ((fBX >= 0) ? +fS : -fS);
                impCheckExtremumResult(fQ / fAX, rResults);
                if( !fTools::equalZero(fS) ) // ignore root multiplicity
                    impCheckExtremumResult(fCX / fQ, rResults);
            }
        }
        else if( !fTools::equalZero(fBX) )
        {
            // derivative is polynomial of order 1 => one extrema
            impCheckExtremumResult(fCX / (2 * fBX), rResults);
        }

        // calculate the y-extrema parameters by zeroing first y-derivative
        double fCY = maControlPointA.getY() - maStartPoint.getY();
        const double fBY = fCY + aControlDiff.getY();
        const double fAY = 3 * aControlDiff.getY() + (maEndPoint.getY() - maStartPoint.getY());

        if(fTools::equalZero(fCY))
        {
            // detect fCY equal zero and truncate to real zero value in that case
            fCY = 0.0;
        }

        if( !fTools::equalZero(fAY) )
        {
            // derivative is polynomial of order 2 => use binomial formula
            const double fD = fBY*fBY - fAY*fCY;
            if( fD >= 0.0 )
            {
                const double fS = sqrt(fD);
                // calculate both roots (avoiding a numerically unstable subtraction)
                const double fQ = fBY + ((fBY >= 0) ? +fS : -fS);
                impCheckExtremumResult(fQ / fAY, rResults);
                if( !fTools::equalZero(fS) ) // ignore root multiplicity
                    impCheckExtremumResult(fCY / fQ, rResults);
            }
        }
        else if( !fTools::equalZero(fBY) )
        {
            // derivative is polynomial of order 1 => one extrema
            impCheckExtremumResult(fCY / (2 * fBY), rResults);
        }
    }

    void B2DCubicBezier::transform(const basegfx::B2DHomMatrix& rMatrix)
    {
        if(!rMatrix.isIdentity())
        {
            if(maControlPointA == maStartPoint)
            {
                maControlPointA = maStartPoint = rMatrix * maStartPoint;
            }
            else
            {
                maStartPoint *= rMatrix;
                maControlPointA *= rMatrix;
            }

            if(maControlPointB == maEndPoint)
            {
                maControlPointB = maEndPoint = rMatrix * maEndPoint;
            }
            else
            {
                maEndPoint *= rMatrix;
                maControlPointB *= rMatrix;
            }
        }
    }

    void B2DCubicBezier::fround()
    {
        if(maControlPointA == maStartPoint)
        {
            maControlPointA = maStartPoint = basegfx::B2DPoint(
                basegfx::fround(maStartPoint.getX()),
                basegfx::fround(maStartPoint.getY()));
        }
        else
        {
            maStartPoint = basegfx::B2DPoint(
                basegfx::fround(maStartPoint.getX()),
                basegfx::fround(maStartPoint.getY()));
            maControlPointA = basegfx::B2DPoint(
                basegfx::fround(maControlPointA.getX()),
                basegfx::fround(maControlPointA.getY()));
        }

        if(maControlPointB == maEndPoint)
        {
            maControlPointB = maEndPoint = basegfx::B2DPoint(
                basegfx::fround(maEndPoint.getX()),
                basegfx::fround(maEndPoint.getY()));
        }
        else
        {
            maEndPoint = basegfx::B2DPoint(
                basegfx::fround(maEndPoint.getX()),
                basegfx::fround(maEndPoint.getY()));
            maControlPointB = basegfx::B2DPoint(
                basegfx::fround(maControlPointB.getX()),
                basegfx::fround(maControlPointB.getY()));
        }
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
