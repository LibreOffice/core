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

#include <basegfx/curve/CubicSpline.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <vector>

namespace basegfx
{
class CubicSplineTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testRejectsSinglePoint)
{
    std::vector<B2DPoint> aPoints{ B2DPoint(0.0, 0.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Natural);
    CPPUNIT_ASSERT(!aSpline.isValid());
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testRejectsPeriodicWhenEndsDiffer)
{
    // The periodic case is only meaningful when the first and last point
    // coincide. Otherwise the curve cannot close smoothly.
    std::vector<B2DPoint> aPoints{ B2DPoint(0.0, 0.0), B2DPoint(1.0, 1.0), B2DPoint(2.0, 0.0),
                                   B2DPoint(3.0, 2.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Periodic);
    CPPUNIT_ASSERT(!aSpline.isValid());
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testNaturalEndpoints)
{
    // The curve passes through every input point in order. getPosition at
    // the parameter bounds yields the first and last input.
    std::vector<B2DPoint> aPoints{ B2DPoint(1.0, 2.0), B2DPoint(2.0, 5.0), B2DPoint(4.0, 1.0),
                                   B2DPoint(6.0, 3.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Natural);
    CPPUNIT_ASSERT(aSpline.isValid());

    B2DPoint aFirst = aSpline.getPosition(0.0);
    B2DPoint aLast = aSpline.getPosition(1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aFirst.getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aFirst.getY(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, aLast.getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aLast.getY(), 1e-10);
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testNaturalLinearInputStaysLinear)
{
    // Points on y = 3x - 1; cubic spline interpolation reproduces linears
    // exactly, so every sample stays on the line.
    std::vector<B2DPoint> aPoints{ B2DPoint(0.0, -1.0), B2DPoint(1.0, 2.0), B2DPoint(2.0, 5.0),
                                   B2DPoint(3.0, 8.0), B2DPoint(4.0, 11.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Natural);
    CPPUNIT_ASSERT(aSpline.isValid());

    for (int nI = 0; nI <= 20; ++nI)
    {
        double fT = double(nI) / 20.0;
        B2DPoint aPoint = aSpline.getPosition(fT);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0 * aPoint.getX() - 1.0, aPoint.getY(), 1e-8);
    }
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testNaturalSymmetricInputStaysSymmetric)
{
    // Input is mirror-symmetric around x = 1.5. The natural spline preserves
    // that symmetry so samples mirror around x = 1.5 as well.
    std::vector<B2DPoint> aPoints{ B2DPoint(0.0, 0.0), B2DPoint(1.0, 2.0), B2DPoint(2.0, 2.0),
                                   B2DPoint(3.0, 0.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Natural);
    CPPUNIT_ASSERT(aSpline.isValid());

    for (int nI = 0; nI <= 10; ++nI)
    {
        double fT = double(nI) / 20.0;
        B2DPoint aLeft = aSpline.getPosition(fT);
        B2DPoint aRight = aSpline.getPosition(1.0 - fT);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, 0.5 * (aLeft.getX() + aRight.getX()), 1e-8);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aLeft.getY(), aRight.getY(), 1e-8);
    }
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testNaturalKnownThreePointResult)
{
    // For points (0, 0), (1, 2), (2, 0) and natural boundary the spline
    // on the first segment is S(x) = -x^3 + 3x; at x = 0.5 that is 1.375.
    std::vector<B2DPoint> aPoints{ B2DPoint(0.0, 0.0), B2DPoint(1.0, 2.0), B2DPoint(2.0, 0.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Natural);
    CPPUNIT_ASSERT(aSpline.isValid());

    // Parameter 0.25 of total range falls at parameter k = 0.5 on segment 0
    // because segment count is 2.
    B2DPoint aMid = aSpline.getPosition(0.25);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, aMid.getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.375, aMid.getY(), 1e-10);
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testNaturalPolygonShape)
{
    // n + 1 input points become n cubic Bezier segments and n + 1 vertices.
    std::vector<B2DPoint> aPoints{ B2DPoint(0.0, 0.0), B2DPoint(1.0, 2.0), B2DPoint(2.0, 1.0),
                                   B2DPoint(3.0, 3.0), B2DPoint(4.0, 0.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Natural);
    CPPUNIT_ASSERT(aSpline.isValid());
    B2DPolygon aPolygon = aSpline.getPolygon();
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(5), aPolygon.count());
    CPPUNIT_ASSERT(aPolygon.areControlPointsUsed());
    // Vertices coincide with input points.
    for (size_t nI = 0; nI < aPoints.size(); ++nI)
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aPoints[nI].getX(), aPolygon.getB2DPoint(nI).getX(), 1e-10);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aPoints[nI].getY(), aPolygon.getB2DPoint(nI).getY(), 1e-10);
    }
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testPeriodicClosesAtSamePoint)
{
    // The periodic curve must end where it started.
    std::vector<B2DPoint> aPoints{ B2DPoint(0.0, 0.0), B2DPoint(1.0, 1.0), B2DPoint(2.0, 0.0),
                                   B2DPoint(1.0, -1.0), B2DPoint(0.0, 0.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Periodic);
    CPPUNIT_ASSERT(aSpline.isValid());

    B2DPoint aFirst = aSpline.getPosition(0.0);
    B2DPoint aLast = aSpline.getPosition(1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aFirst.getX(), aLast.getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aFirst.getY(), aLast.getY(), 1e-10);
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testPeriodicSymmetricInputStaysSymmetric)
{
    // A periodic loop that is symmetric around its midpoint stays so.
    std::vector<B2DPoint> aPoints{ B2DPoint(0.0, 0.0), B2DPoint(1.0, 2.0), B2DPoint(2.0, 0.0),
                                   B2DPoint(1.0, -2.0), B2DPoint(0.0, 0.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Periodic);
    CPPUNIT_ASSERT(aSpline.isValid());

    // Sample pairs equidistant from the midpoint parameter 0.5; the y
    // values must be opposite in sign and the x values mirror.
    for (int nI = 1; nI <= 5; ++nI)
    {
        double fOffset = double(nI) / 20.0;
        B2DPoint aBefore = aSpline.getPosition(0.5 - fOffset);
        B2DPoint aAfter = aSpline.getPosition(0.5 + fOffset);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aBefore.getX(), aAfter.getX(), 1e-8);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-aBefore.getY(), aAfter.getY(), 1e-8);
    }
}

CPPUNIT_TEST_FIXTURE(CubicSplineTest, testGetPositionMatchesPolygonStart)
{
    // getPolygon and getPosition are built from the same solved spline,
    // so the polygon's first vertex must equal getPosition(0).
    std::vector<B2DPoint> aPoints{ B2DPoint(2.0, 3.0), B2DPoint(4.0, 5.0), B2DPoint(6.0, 1.0),
                                   B2DPoint(8.0, 4.0) };
    CubicSpline aSpline(aPoints, CubicSpline::BoundaryCondition::Natural);
    CPPUNIT_ASSERT(aSpline.isValid());

    B2DPolygon aPolygon = aSpline.getPolygon();
    B2DPoint aStart = aSpline.getPosition(0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aPolygon.getB2DPoint(0).getX(), aStart.getX(), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aPolygon.getB2DPoint(0).getY(), aStart.getY(), 1e-10);
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
