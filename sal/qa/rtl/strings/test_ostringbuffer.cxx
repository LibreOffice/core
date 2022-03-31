/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/strbuf.hxx>
#include <rtl/stringutils.hxx>
#include <sal/types.h>

namespace
{
class Test : public CppUnit::TestFixture
{
private:
    void testStringView()
    {
        OStringBuffer b("foobar");
        b = std::string_view("abc").substr(
            0, 2); // avoid the string_view accidentally being NUL-terminated
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), b.getLength());
        CPPUNIT_ASSERT_EQUAL('a', b.getStr()[0]);
        CPPUNIT_ASSERT_EQUAL('b', b.getStr()[1]);
        CPPUNIT_ASSERT_EQUAL('\0', b.getStr()[2]);
    }

    void testOStringChar()
    {
        OStringBuffer b("foobar");
        b = OStringChar('a');
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), b.getLength());
        CPPUNIT_ASSERT_EQUAL('a', b.getStr()[0]);
        CPPUNIT_ASSERT_EQUAL('\0', b.getStr()[1]);
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStringView);
    CPPUNIT_TEST(testOStringChar);
    CPPUNIT_TEST_SUITE_END();
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
