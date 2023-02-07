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
#include <cppunit/plugin/TestPlugIn.h>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>

class B2DRangeTest : public CppUnit::TestFixture
{
    void testCreation()
    {
        basegfx::B2DRange aRange(1.2, 2.3, 3.5, 4.8);

        CPPUNIT_ASSERT_EQUAL(basegfx::B2DRange(1.2, 2.3, 3.5, 4.8), aRange);

        CPPUNIT_ASSERT_EQUAL(1.2, aRange.getMinX());
        CPPUNIT_ASSERT_EQUAL(3.5, aRange.getMaxX());
        CPPUNIT_ASSERT_EQUAL(2.3, aRange.getMinY());
        CPPUNIT_ASSERT_EQUAL(4.8, aRange.getMaxY());

        CPPUNIT_ASSERT_EQUAL(2.3, aRange.getWidth());
        CPPUNIT_ASSERT_EQUAL(2.5, aRange.getHeight());

        // wrong order of the interval
        basegfx::B2DRange aRange2(1.0, 1.0, 0.0, 0.0);

        CPPUNIT_ASSERT_EQUAL(basegfx::B2DRange(1.0, 1.0, 0.0, 0.0), aRange2);
        CPPUNIT_ASSERT_EQUAL(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0), aRange2);

        CPPUNIT_ASSERT_EQUAL(0.0, aRange2.getMinX());
        CPPUNIT_ASSERT_EQUAL(0.0, aRange2.getMinY());
        CPPUNIT_ASSERT_EQUAL(1.0, aRange2.getMaxX());
        CPPUNIT_ASSERT_EQUAL(1.0, aRange2.getMaxY());

        CPPUNIT_ASSERT_EQUAL(1.0, aRange2.getWidth());
        CPPUNIT_ASSERT_EQUAL(1.0, aRange2.getHeight());
    }

    void testRound()
    {
        basegfx::B2DRange aRange(1.2, 2.3, 3.5, 4.8);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple range rounding from double to integer",
                                     basegfx::B2IRange(1, 2, 4, 5), fround(aRange));
    }

    void testCenter()
    {
        basegfx::B2DRange aRange(1.0, 2.0, 2.0, 3.0);
        CPPUNIT_ASSERT_EQUAL(1.5, aRange.getCenterX());
        CPPUNIT_ASSERT_EQUAL(2.5, aRange.getCenterY());
    }

    void testIntersect()
    {
        basegfx::B2DRange aRange(1.0, 1.0, 4.0, 4.0);
        aRange.intersect(basegfx::B2DRange(0.0, 0.0, 2.0, 2.0));

        CPPUNIT_ASSERT_EQUAL(basegfx::B2DRange(1.0, 1.0, 2.0, 2.0), aRange);
    }

    CPPUNIT_TEST_SUITE(B2DRangeTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testRound);
    CPPUNIT_TEST(testCenter);
    CPPUNIT_TEST(testIntersect);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(B2DRangeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
