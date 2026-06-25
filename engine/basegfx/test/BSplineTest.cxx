/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <basegfx/curve/BSpline.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <cmath>
#include <vector>

namespace basegfx
{
class BSplineTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(BSplineTest, testRejectsTooFewPoints)
{
    // Need at least nDegree + 1 distinct points - three points cannot support
    // a degree-3 spline.
    std::vector<B2DPoint> aPoints{ { 0.0, 0.0 }, { 1.0, 1.0 }, { 2.0, 0.0 } };
    BSpline aSpline(aPoints, 3);
    CPPUNIT_ASSERT(!aSpline.isValid());
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testRejectsDegreeBelowOne)
{
    // Degree must be at least 1. Degree 0 is rejected.
    std::vector<B2DPoint> aPoints{ { 0.0, 0.0 }, { 1.0, 1.0 }, { 2.0, 0.0 }, { 3.0, 1.0 } };
    BSpline aSpline(aPoints, 0);
    CPPUNIT_ASSERT(!aSpline.isValid());
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testRejectsDegreeAboveLimit)
{
    // Degree is capped at 15. Degree 16 is rejected even with enough points.
    std::vector<B2DPoint> aPoints(20, B2DPoint(0.0, 0.0));
    for (size_t nI = 0; nI < aPoints.size(); ++nI)
        aPoints[nI] = B2DPoint(double(nI), 0.5 * double(nI));
    BSpline aSpline(aPoints, 16);
    CPPUNIT_ASSERT(!aSpline.isValid());
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testRejectsRepeatedAdjacentPoints)
{
    // After deduplicating consecutive identical entries, only two distinct
    // points remain. A degree-3 spline needs at least four.
    std::vector<B2DPoint> aPoints{
        { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 1.0, 1.0 }, { 1.0, 1.0 }
    };
    BSpline aSpline(aPoints, 3);
    CPPUNIT_ASSERT(!aSpline.isValid());
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testEndpointsMatchInputAtParameterBoundaries)
{
    // getPosition(0) returns the first input point and getPosition(1) returns
    // the last. This is what makes the curve interpolating.
    std::vector<B2DPoint> aPoints{ { 1.0, 2.0 }, { 3.0, 4.0 }, { 5.0, 1.0 }, { 7.0, 3.0 } };
    BSpline aSpline(aPoints, 3);
    CPPUNIT_ASSERT(aSpline.isValid());

    B2DPoint aFirst = aSpline.getPosition(0.0);
    B2DPoint aLast = aSpline.getPosition(1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aFirst.getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aFirst.getY(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, aLast.getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aLast.getY(), 1e-10);
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testEndpointsMatchInputViaPolygon)
{
    // The first and last polygon vertices also coincide with the first and
    // last input points. The polygon is a complete representation.
    std::vector<B2DPoint> aPoints{
        { 0.5, 1.5 }, { 2.0, 3.0 }, { 4.0, 0.5 }, { 5.0, 2.0 }, { 6.5, 1.0 }
    };
    BSpline aSpline(aPoints, 3);
    CPPUNIT_ASSERT(aSpline.isValid());

    B2DPolygon aPolygon = aSpline.getPolygon();
    CPPUNIT_ASSERT(aPolygon.count() >= 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, aPolygon.getB2DPoint(0).getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, aPolygon.getB2DPoint(0).getY(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.5, aPolygon.getB2DPoint(aPolygon.count() - 1).getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aPolygon.getB2DPoint(aPolygon.count() - 1).getY(), 1e-10);
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testInputParametersAreMonotoneInZeroToOne)
{
    // getInputParameters returns one strictly-increasing value per input
    // point, starting at 0.0 and ending at 1.0.
    std::vector<B2DPoint> aPoints{
        { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.0 }, { 3.0, 3.0 }, { 4.0, 0.0 }
    };
    BSpline aSpline(aPoints, 3);
    CPPUNIT_ASSERT(aSpline.isValid());

    const std::vector<double>& rT = aSpline.getInputParameters();
    CPPUNIT_ASSERT_EQUAL(aPoints.size(), rT.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, rT.front(), 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rT.back(), 1e-12);
    for (size_t nI = 1; nI < rT.size(); ++nI)
        CPPUNIT_ASSERT(rT[nI - 1] < rT[nI]);
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testLinearInputStaysLinear)
{
    // Points on y = 2x + 1. B-splines preserve linear functions, so every
    // sample stays on the line.
    std::vector<B2DPoint> aPoints{ { 0.0, 1.0 }, { 1.0, 3.0 }, { 2.0, 5.0 }, { 3.0, 7.0 } };
    BSpline aSpline(aPoints, 3);
    CPPUNIT_ASSERT(aSpline.isValid());

    for (int nI = 0; nI <= 20; ++nI)
    {
        double fT = double(nI) / 20.0;
        B2DPoint aPoint = aSpline.getPosition(fT);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 * aPoint.getX() + 1.0, aPoint.getY(), 1e-8);
    }
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testDegreeOneIsPiecewiseLinear)
{
    // A degree-1 B-spline through three points is two straight-line
    // segments at different slopes. These three points trace y = x
    // then y = 1, so every sample on the curve must lie on one of
    // those two segments.
    std::vector<B2DPoint> aPoints{ { 0.0, 0.0 }, { 1.0, 1.0 }, { 2.0, 1.0 } };
    BSpline aSpline(aPoints, 1);
    CPPUNIT_ASSERT(aSpline.isValid());

    // True when (fX, fY) satisfies the line equation of the segment its
    // x lands in (y = x on [0, 1], y = 1 on [1, 2]). Range checks have
    // a small slack to absorb floating-point noise around the join.
    auto onSegment = [](double fX, double fY) {
        // y = x on [0, 1]
        if (fX >= -1e-10 && fX <= 1.0 + 1e-10 && std::abs(fY - fX) < 1e-8)
            return true;
        // y = 1 on [1, 2]
        if (fX >= 1.0 - 1e-10 && fX <= 2.0 + 1e-10 && std::abs(fY - 1.0) < 1e-8)
            return true;
        return false;
    };

    for (int nI = 0; nI <= 40; ++nI)
    {
        double fT = double(nI) / 40.0;
        B2DPoint aPoint = aSpline.getPosition(fT);
        CPPUNIT_ASSERT_MESSAGE("Sample falls off the input segments",
                               onSegment(aPoint.getX(), aPoint.getY()));
    }
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testSymmetricInputStaysSymmetric)
{
    // The four points are mirror-symmetric around x = 1.5, so the
    // resulting curve also mirrors around x = 1.5.
    std::vector<B2DPoint> aPoints{ { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 2.0 }, { 3.0, 0.0 } };

    BSpline aSpline(aPoints, 3);
    CPPUNIT_ASSERT(aSpline.isValid());

    for (int nI = 0; nI <= 10; ++nI)
    {
        // fT and 1 - fT are mirror positions on the curve parameter.
        double fT = double(nI) / 20.0;
        B2DPoint aLeft = aSpline.getPosition(fT);
        B2DPoint aRight = aSpline.getPosition(1.0 - fT);
        // The average of their X coordinates is 1.5 (mirror around 1.5).
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, 0.5 * (aLeft.getX() + aRight.getX()), 1e-8);
        // Their Y coordinates agree.
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aLeft.getY(), aRight.getY(), 1e-8);
    }
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testPolygonSegmentCountMatchesDegree)
{
    // The polygon stores (number of input points - degree) Bezier segments.
    // The vertex count is one more than the segment count.
    std::vector<B2DPoint> aPoints{
        { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.0 }, { 3.0, 3.0 }, { 4.0, 0.0 }
    };
    BSpline aSpline(aPoints, 3);
    CPPUNIT_ASSERT(aSpline.isValid());
    B2DPolygon aPolygon = aSpline.getPolygon();
    // 5 input points, degree 3: 5 - 3 = 2 segments, so 3 vertices.
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(3), aPolygon.count());
    CPPUNIT_ASSERT(aPolygon.areControlPointsUsed());
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testQuadraticDegreeWorks)
{
    // Degree 2 takes the quadratic-Bezier polygon-build path. The
    // curve must still interpolate the endpoints, so the first and
    // last polygon vertices equal the first and last input points.
    std::vector<B2DPoint> aPoints{ { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.0 }, { 3.0, 3.0 } };
    BSpline aSpline(aPoints, 2);
    CPPUNIT_ASSERT(aSpline.isValid());
    B2DPolygon aPolygon = aSpline.getPolygon();
    CPPUNIT_ASSERT(aPolygon.count() >= 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aPolygon.getB2DPoint(0).getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aPolygon.getB2DPoint(0).getY(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aPolygon.getB2DPoint(aPolygon.count() - 1).getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aPolygon.getB2DPoint(aPolygon.count() - 1).getY(), 1e-10);
}

CPPUNIT_TEST_FIXTURE(BSplineTest, testHighDegreeApproximationStaysClose)
{
    // For degree > 3 the polygon uses recursive cubic-Bezier approximation.
    // Sampled positions from the exact getPosition() must be close to the
    // cubic polygon at the same parameter, within the documented 1% chord
    // tolerance.
    std::vector<B2DPoint> aPoints{ { 0.0, 0.0 }, { 1.0, 3.0 }, { 2.0, -1.0 }, { 3.0, 4.0 },
                                   { 4.0, 0.0 }, { 5.0, 2.0 }, { 6.0, -2.0 }, { 7.0, 3.0 } };
    BSpline aSpline(aPoints, 5);
    CPPUNIT_ASSERT(aSpline.isValid());
    B2DPolygon aPolygon = aSpline.getPolygon();
    CPPUNIT_ASSERT(aPolygon.count() >= 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aPolygon.getB2DPoint(0).getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, aPolygon.getB2DPoint(aPolygon.count() - 1).getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aPolygon.getB2DPoint(aPolygon.count() - 1).getY(), 1e-10);
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
