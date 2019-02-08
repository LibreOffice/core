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
#include <basegfx/point/b2dpoint.hxx>

namespace basegfx
{
class b2dpolygon : public CppUnit::TestFixture
{
public:
    void testBasics()
    {
        B2DPolygon aPoly;

        aPoly.appendBezierSegment(B2DPoint(1, 1), B2DPoint(2, 2), B2DPoint(3, 3));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("#1 first polygon point wrong", B2DPoint(3, 3),
                                     aPoly.getB2DPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#1 first control point wrong", B2DPoint(2, 2),
                                     aPoly.getPrevControlPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#1 second control point wrong", B2DPoint(3, 3),
                                     aPoly.getNextControlPoint(0));
        CPPUNIT_ASSERT_MESSAGE("next control point not used", !aPoly.isNextControlPointUsed(0));

        aPoly.setNextControlPoint(0, B2DPoint(4, 4));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#1.1 second control point wrong", B2DPoint(4, 4),
                                     aPoly.getNextControlPoint(0));
        CPPUNIT_ASSERT_MESSAGE("next control point used", aPoly.isNextControlPointUsed(0));
        CPPUNIT_ASSERT_MESSAGE("areControlPointsUsed() wrong", aPoly.areControlPointsUsed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getContinuityInPoint() wrong", B2VectorContinuity::C2,
                                     aPoly.getContinuityInPoint(0));

        aPoly.resetControlPoints();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("resetControlPoints() did not clear", B2DPoint(3, 3),
                                     aPoly.getB2DPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("resetControlPoints() did not clear", B2DPoint(3, 3),
                                     aPoly.getPrevControlPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("resetControlPoints() did not clear", B2DPoint(3, 3),
                                     aPoly.getNextControlPoint(0));
        CPPUNIT_ASSERT_MESSAGE("areControlPointsUsed() wrong #2", !aPoly.areControlPointsUsed());

        aPoly.clear();
        aPoly.append(B2DPoint(0, 0));
        aPoly.appendBezierSegment(B2DPoint(1, 1), B2DPoint(2, 2), B2DPoint(3, 3));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 first polygon point wrong", B2DPoint(0, 0),
                                     aPoly.getB2DPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 first control point wrong", B2DPoint(0, 0),
                                     aPoly.getPrevControlPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 second control point wrong", B2DPoint(1, 1),
                                     aPoly.getNextControlPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 third control point wrong", B2DPoint(2, 2),
                                     aPoly.getPrevControlPoint(1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 fourth control point wrong", B2DPoint(3, 3),
                                     aPoly.getNextControlPoint(1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 second polygon point wrong", B2DPoint(3, 3),
                                     aPoly.getB2DPoint(1));
    }
    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolygon);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolygon

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolygon);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
