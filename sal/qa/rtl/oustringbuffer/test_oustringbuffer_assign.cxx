/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <o3tl/cppunittraitshelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
private:
    void test()
    {
        OUStringBuffer b1;
        OUString s1(u"123456789012345"_ustr);
        b1 = s1;
        CPPUNIT_ASSERT_EQUAL(s1, b1.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b1.getCapacity());
        OUString s2(u"abc"_ustr);
        b1 = s2;
        CPPUNIT_ASSERT_EQUAL(s2, b1.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b1.getCapacity());
        OUString s3(u"1234567890123456"_ustr);
        b1 = s3;
        CPPUNIT_ASSERT_EQUAL(s3, b1.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(32), b1.getCapacity());
        OUStringBuffer b2;
        b2 = "123456789012345";
        CPPUNIT_ASSERT_EQUAL(s1, b2.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b2.getCapacity());
        b2 = "abc";
        CPPUNIT_ASSERT_EQUAL(s2, b2.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b2.getCapacity());
        b2 = "1234567890123456";
        CPPUNIT_ASSERT_EQUAL(s3, b2.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(32), b2.getCapacity());
        OUStringBuffer b3;
        b3 = u"123456789012345";
        CPPUNIT_ASSERT_EQUAL(s1, b3.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b3.getCapacity());
        b3 = u"abc";
        CPPUNIT_ASSERT_EQUAL(s2, b3.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b3.getCapacity());
        b3 = u"1234567890123456";
        CPPUNIT_ASSERT_EQUAL(s3, b3.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(32), b3.getCapacity());
        OUStringBuffer b4;
        b4 = OUStringLiteral(u"1") + "23456789012345";
        CPPUNIT_ASSERT_EQUAL(s1, b4.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b4.getCapacity());
        b4 = OUStringLiteral(u"a") + "bc";
        CPPUNIT_ASSERT_EQUAL(s2, b4.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b4.getCapacity());
        b4 = OUStringLiteral(u"1") + "234567890123456";
        CPPUNIT_ASSERT_EQUAL(s3, b4.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(32), b4.getCapacity());
        b4 = OUStringChar('a');
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), b4.getLength());
        CPPUNIT_ASSERT_EQUAL(u'a', b4.getStr()[0]);
        CPPUNIT_ASSERT_EQUAL(u'\0', b4.getStr()[1]);
        b4 = std::u16string_view(u"abc").substr(
            0, 2); // avoid the string_view accidentally being NUL-terminated
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), b4.getLength());
        CPPUNIT_ASSERT_EQUAL(u'a', b4.getStr()[0]);
        CPPUNIT_ASSERT_EQUAL(u'b', b4.getStr()[1]);
        CPPUNIT_ASSERT_EQUAL(u'\0', b4.getStr()[2]);
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
