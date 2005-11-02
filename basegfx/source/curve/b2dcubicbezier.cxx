/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dcubicbezier.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:56:14 $
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

#ifndef _BGFX_CURVE_B2DCUBICBEZIER_HXX
#include <basegfx/curve/b2dcubicbezier.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

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
        void ImpSubDiv(
            const B2DPoint& rfPA,           // start point
            const B2DPoint& rfEA,           // edge on A
            const B2DPoint& rfEB,           // edge on B
            const B2DPoint& rfPB,           // end point
            B2DPolygon& rTarget,            // target polygon
            double fAngleBound,             // angle bound in [0.0 .. 2PI]
            bool bAddLastPoint,             // should last point be added?
            bool bAllowUnsharpen,           // #i37443# allow the criteria to get unsharp in recursions
            sal_uInt16 nMaxRecursionDepth)  // endless loop protection
        {
            if(nMaxRecursionDepth)
            {
                // do angle test
                const B2DVector aLeft(rfEA - rfPA);
                const B2DVector aRight(rfEB - rfPB);
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
                ImpSubDiv(rfPA, aS1L, aS2L, aS3C, rTarget, fAngleBound,
                    bAddLastPoint, bAllowUnsharpen, nMaxRecursionDepth - 1);

                // right recursion
                ImpSubDiv(aS3C, aS2R, aS1R, rfPB, rTarget, fAngleBound,
                    bAddLastPoint, bAllowUnsharpen, nMaxRecursionDepth - 1);
            }
            else
            {
                // add points
                rTarget.append(rfPA);

                if(bAddLastPoint)
                {
                    rTarget.append(rfPB);
                }
            }
        }

        void ImpSubDivStart(
            const B2DPoint& rfPA,           // start point
            const B2DPoint& rfEA,           // edge on A
            const B2DPoint& rfEB,           // edge on B
            const B2DPoint& rfPB,           // end point
            B2DPolygon& rTarget,            // target polygon
            const double& rfAngleBound,     // angle bound in [0.0 .. 2PI]
            bool bAddLastPoint,             // should last point be added?
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
                const bool bBaseEqualZero(aLeft.equalZero());

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
                    const B2DVector aLeftLeft(aS1L - rfPA);
                    const B2DVector aRightLeft(aS2L - aS3C);
                    const double fCurrentAngleLeft(aLeftLeft.angle(aRightLeft));
                    bAngleIsSmallerLeft = (fabs(fCurrentAngleLeft) > (F_PI - rfAngleBound));
                }

                // test right
                bool bAngleIsSmallerRight(bAllParallel && bRightEqualZero);
                if(!bAngleIsSmallerRight)
                {
                    const B2DVector aLeftRight(aS2R - aS3C);
                    const B2DVector aRightRight(aS1R - rfPB);
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
                        rTarget.append(rfPA);
                        if(bAddLastPoint)
                        {
                            rTarget.append(aS3C);
                        }
                    }
                    else
                    {
                        ImpSubDiv(rfPA, aS1L, aS2L, aS3C, rTarget, rfAngleBound,
                            bAddLastPoint, bAllowUnsharpen, nMaxRecursionDepth);
                    }

                    // right
                    if(bAngleIsSmallerRight)
                    {
                        rTarget.append(aS3C);
                        if(bAddLastPoint)
                        {
                            rTarget.append(rfPB);
                        }
                    }
                    else
                    {
                        ImpSubDiv(aS3C, aS2R, aS1R, rfPB, rTarget, rfAngleBound,
                            bAddLastPoint, bAllowUnsharpen, nMaxRecursionDepth);
                    }
                }
            }

            if(!nMaxRecursionDepth)
            {
                rTarget.append(rfPA);
                if(bAddLastPoint)
                {
                    rTarget.append(rfPB);
                }
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

    B2DCubicBezier::B2DCubicBezier(const B2DPoint& rStart, const B2DVector& rControlVectorA, const B2DVector& rControlVectorB, const B2DPoint& rEnd)
    :   maStartPoint(rStart),
        maEndPoint(rEnd),
        maControlPointA(rStart + rControlVectorA),
        maControlPointB(rStart + rControlVectorB)
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
        // TODO
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
        const B2DVector aTop(maControlPointB - maControlPointA);
        return (aVectorA.getLength() + aVectorB.getLength() + aTop.getLength());
    }

    void B2DCubicBezier::adaptiveSubdivideByAngle(B2DPolygon& rTarget, double fAngleBound,
        bool bAddLastPoint, bool bAllowUnsharpen) const
    {
        // use support method #i37443# and allow unsharpen the criteria
        ImpSubDivStart(maStartPoint, maControlPointA, maControlPointB, maEndPoint,
            rTarget, fAngleBound * F_PI180, bAddLastPoint, bAllowUnsharpen);
    }

    // #i37443# adaptive subdivide by nCount subdivisions
    void B2DCubicBezier::adaptiveSubdivideByCount(B2DPolygon& rTarget, sal_uInt32 nCount, bool bAddLastPoint) const
    {
        rTarget.append(maStartPoint);

        for(sal_uInt32 a(0L); a < nCount; a++)
        {
            const double fPos(double(a + 1L) / double(nCount + 1L));
            rTarget.append(interpolatePoint(fPos));
        }

        if(bAddLastPoint)
        {
            rTarget.append(maEndPoint);
        }
    }

    B2DPoint B2DCubicBezier::interpolatePoint(double t) const
    {
        OSL_ENSURE(t >= 0.0 && t <= 1.0, "B2DCubicBezier::interpolatePoint: Access out of range (!)");
        const B2DPoint aS1L(interpolate(maStartPoint, maControlPointA, t));
        const B2DPoint aS1C(interpolate(maControlPointA, maControlPointB, t));
        const B2DPoint aS1R(interpolate(maControlPointB, maEndPoint, t));
        const B2DPoint aS2L(interpolate(aS1L, aS1C, t));
        const B2DPoint aS2R(interpolate(aS1C, aS1R, t));
        return interpolate(aS2L, aS2R, t);
    }

    double B2DCubicBezier::getSmallestDistancePointToBezierSegment(const B2DPoint& rTestPoint, double& rCut) const
    {
        const sal_uInt32 nInitialDivisions(3L);
        B2DPolygon aInitialPolygon;

        // as start make a fix division, creates nInitialDivisions + 2L points
        adaptiveSubdivideByCount(aInitialPolygon, nInitialDivisions, true);

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

    B2DRange B2DCubicBezier::getRange() const
    {
        B2DRange aRetval(maStartPoint, maEndPoint);

        aRetval.expand(maControlPointA);
        aRetval.expand(maControlPointB);

        return aRetval;
    }
} // end of namespace basegfx

// eof
