/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <basegfx/range/b2irange.hxx>

class B2IRangeTest : public CppUnit::TestFixture
{
    void testCreation()
    {
        basegfx::B2IRange aRange(1, 2, 3, 4);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRange.getMinX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRange.getMaxX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRange.getMinY());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRange.getMaxY());

        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRange.getWidth());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRange.getHeight());
    }

    void testCenter()
    {
        basegfx::B2IRange aRange(1, 2, 2, 3);
        CPPUNIT_ASSERT_EQUAL(1.5, aRange.getCenterX());
        CPPUNIT_ASSERT_EQUAL(2.5, aRange.getCenterY());
    }

    CPPUNIT_TEST_SUITE(B2IRangeTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testCenter);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(B2IRangeTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
