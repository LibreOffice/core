/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cmath>
#include <limits>

#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/range/b2drange.hxx>

namespace basegfx
{
class b2dpolypolygoncutter : public CppUnit::TestFixture
{
public:
    void testMergeToSinglePolyPolygon()
    {
        { // Adjacent polygons merged to one, closed manually.
            B2DPolygon poly1{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } };
            B2DPolygon poly2{ { 0, 1 }, { 1, 1 }, { 1, 2 }, { 0, 2 }, { 0, 1 } };
            B2DPolyPolygon expected(
                B2DPolygon{ { 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 2 }, { 0, 1 }, { 0, 0 } });
            expected.setClosed(true);
            B2DPolyPolygon result
                = utils::mergeToSinglePolyPolygon({ B2DPolyPolygon(poly1), B2DPolyPolygon(poly2) });
            CPPUNIT_ASSERT_EQUAL(expected, result);
        }

        { // Adjacent polygons merged to one, closed using setClosed().
            B2DPolygon poly1{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
            B2DPolygon poly2{ { 0, 1 }, { 1, 1 }, { 1, 2 }, { 0, 2 } };
            poly1.setClosed(true);
            poly2.setClosed(true);
            B2DPolyPolygon expected(
                B2DPolygon{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 2 }, { 0, 1 } });
            expected.setClosed(true);
            B2DPolyPolygon result
                = utils::mergeToSinglePolyPolygon({ B2DPolyPolygon(poly1), B2DPolyPolygon(poly2) });
            CPPUNIT_ASSERT_EQUAL(expected, result);
        }

        { // Non-adjacent polygons, no merge.
            B2DPolygon poly1{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
            B2DPolygon poly2{ { 0, 2 }, { 1, 3 }, { 1, 3 }, { 0, 3 } };
            poly1.setClosed(true);
            poly2.setClosed(true);
            B2DPolyPolygon expected;
            expected.append(poly1);
            expected.append(poly2);
            B2DPolyPolygon result
                = utils::mergeToSinglePolyPolygon({ B2DPolyPolygon(poly1), B2DPolyPolygon(poly2) });
            CPPUNIT_ASSERT_EQUAL(expected, result);
        }

        { // Horizontal and vertical rectangle that together form a cross.
            B2DPolygon poly1{ { 1, 0 }, { 2, 0 }, { 2, 3 }, { 1, 3 } };
            B2DPolygon poly2{ { 0, 1 }, { 3, 1 }, { 3, 2 }, { 0, 2 } };
            poly1.setClosed(true);
            poly2.setClosed(true);
            B2DPolyPolygon expected(B2DPolygon{ { 1, 0 },
                                                { 2, 0 },
                                                { 2, 1 },
                                                { 3, 1 },
                                                { 3, 2 },
                                                { 2, 2 },
                                                { 2, 3 },
                                                { 1, 3 },
                                                { 1, 2 },
                                                { 0, 2 },
                                                { 0, 1 },
                                                { 1, 1 } });
            expected.setClosed(true);
            B2DPolyPolygon result
                = utils::mergeToSinglePolyPolygon({ B2DPolyPolygon(poly1), B2DPolyPolygon(poly2) });
            CPPUNIT_ASSERT_EQUAL(expected, result);
        }
    }

    void testNonzeroConformOverlappingSquares()
    {
        // two squares in the same direction overlap, nonzero fills the overlap once
        B2DPolyPolygon aInput(utils::createPolygonFromRect(B2DRange(0, 0, 2, 2)));
        aInput.append(utils::createPolygonFromRect(B2DRange(1, 1, 3, 3)));

        const B2DPolyPolygon aResult(utils::createNonzeroConform(aInput));

        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aResult.count());
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(8), aResult.getB2DPolygon(0).count());
        CPPUNIT_ASSERT(utils::isInside(aResult, B2DPoint(1.5, 1.5)));
        CPPUNIT_ASSERT(utils::isInside(aResult, B2DPoint(2.5, 2.5)));
        CPPUNIT_ASSERT(!utils::isInside(aResult, B2DPoint(2.5, 0.5)));
    }

    void testNonzeroConformStar()
    {
        // a pentagram winds twice around its center, so nonzero fills the center
        B2DPolygon aStar;
        for (int a = 0; a < 5; a++)
        {
            const double fAngle(a * 4.0 * M_PI / 5.0);
            aStar.append(B2DPoint(std::sin(fAngle), -std::cos(fAngle)));
        }
        aStar.setClosed(true);
        CPPUNIT_ASSERT(!utils::isInside(aStar, B2DPoint(0, 0)));

        const B2DPolyPolygon aResult(utils::createNonzeroConform(B2DPolyPolygon(aStar)));

        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aResult.count());
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(10), aResult.getB2DPolygon(0).count());
        CPPUNIT_ASSERT(utils::isInside(aResult, B2DPoint(0, 0)));
    }

    void testNonzeroConformKeepsHole()
    {
        // the outer square twice and the inner one twice the other way: the winding is two
        // around the hole and zero in it
        B2DPolygon aInner(utils::createPolygonFromRect(B2DRange(1, 1, 2, 2)));
        aInner.flip();
        B2DPolyPolygon aInput(utils::createPolygonFromRect(B2DRange(0, 0, 3, 3)));
        aInput.append(utils::createPolygonFromRect(B2DRange(0, 0, 3, 3)));
        aInput.append(aInner);
        aInput.append(aInner);

        const B2DPolyPolygon aResult(utils::createNonzeroConform(aInput));

        CPPUNIT_ASSERT_EQUAL(sal_uInt32(2), aResult.count());
        CPPUNIT_ASSERT(utils::isInside(aResult, B2DPoint(0.5, 0.5)));
        CPPUNIT_ASSERT(!utils::isInside(aResult, B2DPoint(1.5, 1.5)));
    }

    void testNonzeroConformNotFinite()
    {
        // a point that is not finite leaves the polygon as it was, crossings and all
        for (const double fBad :
             { std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity() })
        {
            B2DPolygon aPolygon{ { 0, 0 }, { 2, 0 }, { 0, 2 }, { 2, fBad } };
            aPolygon.setClosed(true);

            const B2DPolyPolygon aResult(utils::createNonzeroConform(B2DPolyPolygon(aPolygon)));

            CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aResult.count());
            const B2DPolygon& rResult(aResult.getB2DPolygon(0));
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), rResult.count());
            CPPUNIT_ASSERT_EQUAL(B2DPoint(0, 2), rResult.getB2DPoint(2));
            CPPUNIT_ASSERT(!std::isfinite(rResult.getB2DPoint(3).getY()));
        }
    }

    void testOrKeepsCurves()
    {
        const B2DPolygon aLeft(utils::createPolygonFromCircle(B2DPoint(0, 0), 1.0));
        const B2DPolygon aRight(utils::createPolygonFromCircle(B2DPoint(1, 0), 1.0));

        const B2DPolyPolygon aResult(
            utils::solvePolygonOperationOr(B2DPolyPolygon(aLeft), B2DPolyPolygon(aRight)));

        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aResult.count());
        const B2DPolygon& rOutline(aResult.getB2DPolygon(0));
        CPPUNIT_ASSERT(rOutline.areControlPointsUsed());
        // the parts of the circles that are not cut come back as they were
        sal_uInt32 nWhole(0);
        for (sal_uInt32 a(0); a < rOutline.count(); a++)
        {
            B2DCubicBezier aSegment;
            rOutline.getBezierSegment(a, aSegment);
            for (const B2DPolygon& rCircle : { aLeft, aRight })
                for (sal_uInt32 b(0); b < rCircle.count(); b++)
                {
                    B2DCubicBezier aInput;
                    rCircle.getBezierSegment(b, aInput);
                    if (aInput.equal(aSegment))
                        nWhole++;
                }
        }
        CPPUNIT_ASSERT_GREATER(sal_uInt32(0), nWhole);
        // where the circles cross the outline turns inward
        CPPUNIT_ASSERT(utils::isInside(aResult, B2DPoint(0.5, 0.8)));
        CPPUNIT_ASSERT(!utils::isInside(aResult, B2DPoint(0.5, 0.9)));
    }

    CPPUNIT_TEST_SUITE(b2dpolypolygoncutter);
    CPPUNIT_TEST(testMergeToSinglePolyPolygon);
    CPPUNIT_TEST(testNonzeroConformOverlappingSquares);
    CPPUNIT_TEST(testNonzeroConformStar);
    CPPUNIT_TEST(testNonzeroConformKeepsHole);
    CPPUNIT_TEST(testNonzeroConformNotFinite);
    CPPUNIT_TEST(testOrKeepsCurves);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolypolygoncutter);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
