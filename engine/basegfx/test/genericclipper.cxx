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

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

using namespace ::basegfx;

namespace basegfx2d
{

class genericclipper : public CppUnit::TestFixture
{
private:
    B2DPolygon aSelfIntersecting;
    B2DPolygon aShiftedRectangle;

public:
    // initialise your test code values here.
    void setUp() override
    {
        aSelfIntersecting.append(B2DPoint(0,  0));
        aSelfIntersecting.append(B2DPoint(0,  100));
        aSelfIntersecting.append(B2DPoint(75, 100));
        aSelfIntersecting.append(B2DPoint(75, 50));
        aSelfIntersecting.append(B2DPoint(25, 50));
        aSelfIntersecting.append(B2DPoint(25, 150));
        aSelfIntersecting.append(B2DPoint(100,150));
        aSelfIntersecting.append(B2DPoint(100,0));
        aSelfIntersecting.setClosed(true);

        aShiftedRectangle = utils::createPolygonFromRect(
            B2DRange(0,90,20,150));
    }


    void validate(const char* pName,
                  const char* pValidSvgD,
                  B2DPolyPolygon (*pFunc)(const B2DPolyPolygon&, const B2DPolyPolygon&)) const
    {
        const B2DPolyPolygon aSelfIntersect(
            utils::prepareForPolygonOperation(aSelfIntersecting));
        const B2DPolyPolygon aRect(
            utils::prepareForPolygonOperation(aShiftedRectangle));
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "%s input LHS - svg:d=\"%s\"\n",
                pName, OUStringToOString(
                    basegfx::utils::exportToSvgD(
                        aSelfIntersect, true, true, false),
                    RTL_TEXTENCODING_UTF8).getStr() );
        fprintf(stderr, "%s input RHS - svg:d=\"%s\"\n",
                pName, OUStringToOString(
                    basegfx::utils::exportToSvgD(
                        aRect, true, true, false),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        const B2DPolyPolygon aRes=
            pFunc(aSelfIntersect, aRect);

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "%s - svg:d=\"%s\"\n",
                pName, OUStringToOString(
                    basegfx::utils::exportToSvgD(aRes, true, true, false),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        OUString aValid=OUString::createFromAscii(pValidSvgD);

        CPPUNIT_ASSERT_EQUAL_MESSAGE(pName,
                               aValid, basegfx::utils::exportToSvgD(aRes, true, true, false));
    }

    void validateOr()
    {
        const char* const pValid="m0 0h100v150h-75v-50h-5v50h-20v-50zm25 100h50v-50h-50z";
        validate("validateOr", pValid, &utils::solvePolygonOperationOr);
    }

    void validateXor()
    {
        const char* const pValid="m0 0h100v150h-75v-50h-5v-10h-20zm25 100h50v-50h-50zm-25 0h20v50h-20z";
        validate("validateXor", pValid, &utils::solvePolygonOperationXor);
    }

    void validateAnd()
    {
        const char* const pValid="m0 100v-10h20v10z";
        validate("validateAnd", pValid, &utils::solvePolygonOperationAnd);
    }

    void validateDiff()
    {
        const char* const pValid="m0 0h100v150h-75v-50h-5v-10h-20zm25 100h50v-50h-50z";
        validate("validateDiff", pValid, &utils::solvePolygonOperationDiff);
    }

    void validateCrossover(const char* pName,
                           const char* pInputSvgD,
                           const char* pValidSvgD) const
    {
        OUString aInput=OUString::createFromAscii(pInputSvgD);
        OUString aValid=OUString::createFromAscii(pValidSvgD);
        B2DPolyPolygon aInputPoly, aValidPoly;

        CPPUNIT_ASSERT(utils::importFromSvgD(aInputPoly, aInput, false, nullptr));
        CPPUNIT_ASSERT(utils::importFromSvgD(aValidPoly, aValid, false, nullptr));

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            pName,
            aValid,
            basegfx::utils::exportToSvgD(
                utils::solveCrossovers(aInputPoly), true, true, false));
    }

    void checkCrossoverSolver()
    {
        // partially intersecting polygons, with a common subsection
        validateCrossover(
            "partially intersecting",
            "m0 0 v 5  h 3 h 1 h 1 h 1 v -2 v -3 z"
              "m3 7 v -2 h 1 h 1 h 1 v -2 h 1 v 3 z",
            "m0 0v5h3 1 1 1v-2-3zm3 7v-2h1 1 1v-2h1v3z");

        // first polygon is identical to subset of second polygon
        validateCrossover(
            "full subset",
            "m0 0 v 5  h 3 h 1 h 1 v -5 z"
              "m3 10 v -5 h 1 h 1 v -5 h -5 v 5 h 3 z",
            "m0 0v5h3 1 1v-5zm3 10v-5zm1-5h1v-5h-5v5h3z");

        // first polygon is identical to subset of second polygon, but
        // oriented in the opposite direction
        validateCrossover(
            "full subset, opposite direction",
            "m0 0 v 5 h 3 h 1 h 1 v -5 z"
              "m3 10 v -5 h -1 h -1 h -1 v -5 h 5 v 5 h 2 z",
            "m0 0v5h1 1 1-1-1-1v-5h5v5-5zm4 5h1 2l-4 5v-5z");

        // first polygon is identical to subset of second polygon, and
        // has a curve segment (triggers different code path)
        validateCrossover(
            "full subset, plus curves",
            "m0 0 v 5  h 3 h 1 h 1 c 2 0 2 0 0 -5 z"
              "m3 10 v -5 h 1 h 1 c 2 0 2 0 0 -5 h -5 v 5 h 3 z",
            "m0 0v5h3 1 1c2 0 2 0 0-5zm3 10v-5zm1-5h1c2 0 2 0 0-5h-5v5h3z");
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    static void checkLine(const char* pName, const B2DPolygon& rLine,
                          std::initializer_list<B2DPoint> aExpected)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(pName, sal_uInt32(aExpected.size()), rLine.count());
        sal_uInt32 nIndex(0);
        for (const B2DPoint& rPoint : aExpected)
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(pName, rPoint.getX(),
                                                 rLine.getB2DPoint(nIndex).getX(), 1e-9);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(pName, rPoint.getY(),
                                                 rLine.getB2DPoint(nIndex).getY(), 1e-9);
            nIndex++;
        }
    }

    void testLineThroughSquare()
    {
        // a line through a square is cut at both sides of it
        const B2DPolyPolygon aLine(B2DPolygon{ { -1, 1 }, { 3, 1 } });
        const B2DPolyPolygon aSquare(utils::createPolygonFromRect(B2DRange(0, 0, 2, 2)));

        const B2DPolyPolygon aInside(
            utils::clipPolyPolygonOnPolyPolygon(aLine, aSquare, true, true));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aInside.count());
        CPPUNIT_ASSERT(!aInside.getB2DPolygon(0).isClosed());
        checkLine("inside", aInside.getB2DPolygon(0), { { 0, 1 }, { 2, 1 } });

        const B2DPolyPolygon aOutside(
            utils::clipPolyPolygonOnPolyPolygon(aLine, aSquare, false, true));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(2), aOutside.count());
        checkLine("outside 1", aOutside.getB2DPolygon(0), { { -1, 1 }, { 0, 1 } });
        checkLine("outside 2", aOutside.getB2DPolygon(1), { { 2, 1 }, { 3, 1 } });
    }

    void testClosedLine()
    {
        // a closed line half in the clip gives one open line per side, the outer one runs on
        // over the start of the closed one
        B2DPolygon aSquareLine{ { 0, 0 }, { 2, 0 }, { 2, 2 }, { 0, 2 } };
        aSquareLine.setClosed(true);
        const B2DPolyPolygon aClip(utils::createPolygonFromRect(B2DRange(1, -1, 3, 3)));

        const B2DPolyPolygon aInside(
            utils::clipPolyPolygonOnPolyPolygon(B2DPolyPolygon(aSquareLine), aClip, true, true));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aInside.count());
        CPPUNIT_ASSERT(!aInside.getB2DPolygon(0).isClosed());
        checkLine("inside", aInside.getB2DPolygon(0), { { 1, 0 }, { 2, 0 }, { 2, 2 }, { 1, 2 } });

        const B2DPolyPolygon aOutside(
            utils::clipPolyPolygonOnPolyPolygon(B2DPolyPolygon(aSquareLine), aClip, false, true));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aOutside.count());
        checkLine("outside", aOutside.getB2DPolygon(0), { { 1, 2 }, { 0, 2 }, { 0, 0 }, { 1, 0 } });

        // all of it inside stays as it is, closed
        const B2DPolyPolygon aWhole(utils::clipPolyPolygonOnPolyPolygon(
            B2DPolyPolygon(aSquareLine),
            B2DPolyPolygon(utils::createPolygonFromRect(B2DRange(-1, -1, 3, 3))), true, true));
        CPPUNIT_ASSERT_EQUAL(B2DPolyPolygon(aSquareLine), aWhole);
    }

    void testCurvedLine()
    {
        // a curve cut by the clip stays a curve, on the same course as before
        B2DPolygon aCurve;
        aCurve.append(B2DPoint(-1, 0));
        aCurve.appendBezierSegment(B2DPoint(0, 2), B2DPoint(2, 2), B2DPoint(3, 0));
        const B2DPolyPolygon aClip(utils::createPolygonFromRect(B2DRange(0, -1, 2, 3)));

        const B2DPolyPolygon aInside(
            utils::clipPolyPolygonOnPolyPolygon(B2DPolyPolygon(aCurve), aClip, true, true));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aInside.count());
        const B2DPolygon aPart(aInside.getB2DPolygon(0));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(2), aPart.count());
        CPPUNIT_ASSERT(aPart.areControlPointsUsed());
        B2DCubicBezier aSegment;
        aPart.getBezierSegment(0, aSegment);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aSegment.getStartPoint().getX(), 1e-9);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aSegment.getEndPoint().getX(), 1e-9);
        // the curve is symmetric, so the middle of the part is the middle of the curve
        B2DCubicBezier aWhole;
        aCurve.getBezierSegment(0, aWhole);
        const B2DPoint aExpected(aWhole.interpolatePoint(0.5));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aExpected.getX(), aSegment.interpolatePoint(0.5).getX(), 1e-9);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aExpected.getY(), aSegment.interpolatePoint(0.5).getY(), 1e-9);
    }

    void testLineOnClipBorder()
    {
        // a line on the border of the clip counts as inside
        const B2DPolyPolygon aLine(B2DPolygon{ { 0, 0 }, { 2, 0 } });
        const B2DPolyPolygon aSquare(utils::createPolygonFromRect(B2DRange(0, 0, 2, 2)));

        CPPUNIT_ASSERT_EQUAL(aLine, utils::clipPolyPolygonOnPolyPolygon(aLine, aSquare, true, true));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
                             utils::clipPolyPolygonOnPolyPolygon(aLine, aSquare, false, true)
                                 .count());
    }

    void testLineOfNoLength()
    {
        // a line from a point to itself stays as it is where its point is kept
        const B2DPolyPolygon aCircle(utils::createPolygonFromCircle(B2DPoint(0, 0), 2));
        const B2DPolyPolygon aIn(B2DPolygon{ { 1, 0 }, { 1, 0 } });
        // in the box of the circle, but outside of it
        const B2DPolyPolygon aOut(B2DPolygon{ { 1.8, 1.8 }, { 1.8, 1.8 } });

        auto clip = [&](const B2DPolyPolygon& rLine, bool bInside)
        { return utils::clipPolyPolygonOnPolyPolygon(rLine, aCircle, bInside, true); };
        CPPUNIT_ASSERT(bool(aIn == clip(aIn, true)));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), clip(aIn, false).count());
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), clip(aOut, true).count());
        CPPUNIT_ASSERT(bool(aOut == clip(aOut, false)));
    }

    CPPUNIT_TEST_SUITE(genericclipper);
    CPPUNIT_TEST(validateOr);
    CPPUNIT_TEST(validateXor);
    CPPUNIT_TEST(validateAnd);
    CPPUNIT_TEST(validateDiff);
    CPPUNIT_TEST(checkCrossoverSolver);
    CPPUNIT_TEST(testLineThroughSquare);
    CPPUNIT_TEST(testClosedLine);
    CPPUNIT_TEST(testCurvedLine);
    CPPUNIT_TEST(testLineOnClipBorder);
    CPPUNIT_TEST(testLineOfNoLength);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::genericclipper);
} // namespace basegfx2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
