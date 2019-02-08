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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

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
        const char* const pValid="m0 0h100v150h-75v-50h-5v50h-20v-50-10zm75 100v-50h-50v50z";
        validate("validateOr", pValid, &utils::solvePolygonOperationOr);
    }

    void validateXor()
    {
        const char* const pValid="m0 0h100v150h-75v-50h-5v50h-20v-50-10zm0 100h20v-10h-20zm75 0v-50h-50v50z";
        validate("validateXor", pValid, &utils::solvePolygonOperationXor);
    }

    void validateAnd()
    {
        const char* const pValid="m0 100v-10h20v10z";
        validate("validateAnd", pValid, &utils::solvePolygonOperationAnd);
    }

    void validateDiff()
    {
        const char* const pValid="m0 90v-90h100v150h-75v-50h-5v-10zm75 10v-50h-50v50z";
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

    CPPUNIT_TEST_SUITE(genericclipper);
    CPPUNIT_TEST(validateOr);
    CPPUNIT_TEST(validateXor);
    CPPUNIT_TEST(validateAnd);
    CPPUNIT_TEST(validateDiff);
    CPPUNIT_TEST(checkCrossoverSolver);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::genericclipper);
} // namespace basegfx2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
