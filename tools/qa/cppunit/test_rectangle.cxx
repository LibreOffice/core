/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <tools/gen.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
public:
    void test_rectangle();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_rectangle);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test_rectangle()
{
    {
        tools::Rectangle aRect(1, 1, 1, 1);

        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());

        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getHeight());
    }

    {
        tools::Rectangle aRect(Point(), Size(1, 1));

        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Left());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Top());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Right());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Bottom());

        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());

        aRect.setX(12);
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());
        aRect.setY(12);
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
