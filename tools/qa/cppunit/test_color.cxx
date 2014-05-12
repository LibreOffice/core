/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <tools/color.hxx>

namespace
{

class Test: public CppUnit::TestFixture
{
public:
    void test_asRGBColor();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_asRGBColor);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test_asRGBColor()
{
    Color aColor;
    aColor = COL_BLACK;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("000000"));

    aColor = COL_WHITE;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("ffffff"));

    aColor = COL_RED;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("800000"));

    aColor = COL_TRANSPARENT;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("ffffff"));

    aColor = COL_BLUE;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("000080"));

    aColor.SetRed(0x12);
    aColor.SetGreen(0x34);
    aColor.SetBlue(0x56);
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("123456"));

    aColor = COL_AUTO;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("ffffff"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
