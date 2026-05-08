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

#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

namespace basegfx
{
class b2dpolygontools : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void testIsRectangle()
    {
        B2DPolygon aRect1(utils::createPolygonFromRect(B2DRange(0, 0, 1, 1)));

        B2DPolygon aRect2{ { 0, 0 }, { 1, 0 }, { 1, 0.5 }, { 1, 1 }, { 0, 1 } };
        aRect2.setClosed(true);

        B2DPolygon aNonRect1{ { 0, 0 }, { 1, 0 }, { 0.5, 1 }, { 0.5, 0 } };
        aNonRect1.setClosed(true);

        B2DPolygon aNonRect2{ { 0, 0 }, { 1, 1 }, { 1, 0 }, { 0, 1 } };
        aNonRect2.setClosed(true);

        B2DPolygon aNonRect3{ { 0, 0 }, { 1, 0 }, { 1, 1 } };
        aNonRect3.setClosed(true);

        B2DPolygon aNonRect4{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

        B2DPolygon aNonRect5{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
        aNonRect5.setControlPoints(1, B2DPoint(1, 0), B2DPoint(-11, 0));
        aNonRect5.setClosed(true);

        CPPUNIT_ASSERT_MESSAGE("checking rectangle-ness of rectangle 1",
                               utils::isRectangle(aRect1));
        CPPUNIT_ASSERT_MESSAGE("checking rectangle-ness of rectangle 2",
                               utils::isRectangle(aRect2));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 1",
                               !utils::isRectangle(aNonRect1));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 2",
                               !utils::isRectangle(aNonRect2));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 3",
                               !utils::isRectangle(aNonRect3));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 4",
                               !utils::isRectangle(aNonRect4));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 5",
                               !utils::isRectangle(aNonRect5));
    }

    // tdf#143031 EMF+ notched arrow caps left a visible gap between the line
    // shaft and the cap because the line was shortened by the cap polygon's
    // full Y extent (out to the back wings) even though the centerline exits
    // the polygon at the notch. createAreaGeometryForLineStartEnd should now
    // detect that and shorten the line only to the notch depth.
    void testCreateAreaGeometryForLineStartEndNotchedArrow()
    {
        // Notched arrow: tip at (0,0), back wings at (+/-1.5, 3), notch at (0, 2).
        B2DPolygon aArrow{ { 0, 2 }, { -1.5, 3 }, { 0, 0 }, { 1.5, 3 } };
        aArrow.setClosed(true);
        B2DPolyPolygon aArrowPP(aArrow);

        // A horizontal line of length 1000.
        B2DPolygon aLine{ { 0, 0 }, { 1000, 0 } };

        const double fWidth = 300.0;
        double fConsumed = 0.0;
        utils::createAreaGeometryForLineStartEnd(aLine, aArrowPP, false /* end */, fWidth,
                                                 1000.0 /* fCandidateLength */,
                                                 0.0 /* fDockingPosition */, &fConsumed);

        // fArrowYLength == fWidth == 300; notch is at Y=2 of 3, so consumed
        // length should be 200, not 300.
        CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, fConsumed, 0.001);
    }

    // Backward-compatibility check: a simple triangle (no concave back) should
    // still consume the full polygon Y length, as before.
    void testCreateAreaGeometryForLineStartEndSimpleTriangle()
    {
        B2DPolygon aArrow{ { 0, 0 }, { -1.5, 3 }, { 1.5, 3 } };
        aArrow.setClosed(true);
        B2DPolyPolygon aArrowPP(aArrow);

        B2DPolygon aLine{ { 0, 0 }, { 1000, 0 } };

        const double fWidth = 300.0;
        double fConsumed = 0.0;
        utils::createAreaGeometryForLineStartEnd(aLine, aArrowPP, false, fWidth, 1000.0, 0.0,
                                                 &fConsumed);

        CPPUNIT_ASSERT_DOUBLES_EQUAL(300.0, fConsumed, 0.001);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolygontools);
    CPPUNIT_TEST(testIsRectangle);
    CPPUNIT_TEST(testCreateAreaGeometryForLineStartEndNotchedArrow);
    CPPUNIT_TEST(testCreateAreaGeometryForLineStartEndSimpleTriangle);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolygontools

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolygontools);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
