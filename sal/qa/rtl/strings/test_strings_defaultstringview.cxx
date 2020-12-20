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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
    void string() { CPPUNIT_ASSERT_EQUAL(OString(), OString(std::string_view())); }

    void stringbuffer()
    {
        // No functions related to OStringBuffer that take a std::string_view or std::u16string_view
        // argument.
    }

    void ustring()
    {
        CPPUNIT_ASSERT_EQUAL(OUString(), OUString(std::u16string_view()));
        OUString s1("foo");
        s1 = std::u16string_view();
        CPPUNIT_ASSERT_EQUAL(OUString(), s1);
        OUString s2("foo");
        s2 += std::u16string_view();
        CPPUNIT_ASSERT_EQUAL(OUString("foo"), s2);
        CPPUNIT_ASSERT_GREATER(sal_Int32(0),
                               OUString("foo").reverseCompareTo(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(false, OUString("foo").equalsIgnoreAsciiCase(std::u16string_view()));
        CPPUNIT_ASSERT_GREATER(sal_Int32(0),
                               OUString("foo").compareToIgnoreAsciiCase(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(true, OUString("foo").match(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(true, OUString("foo").matchIgnoreAsciiCase(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(true, OUString("foo").startsWith(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(true,
                             OUString("foo").startsWithIgnoreAsciiCase(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(true, OUString("foo").endsWith(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(true, OUString("foo").endsWithIgnoreAsciiCase(std::u16string_view()));
        OUString const foo("foo"); // avoid loplugin:stringconstant, loplugin:stringview
        CPPUNIT_ASSERT_EQUAL(false, foo == std::u16string_view());
        CPPUNIT_ASSERT_EQUAL(true, foo != std::u16string_view());
        CPPUNIT_ASSERT_EQUAL(false, foo < std::u16string_view());
        CPPUNIT_ASSERT_EQUAL(false, foo <= std::u16string_view());
        CPPUNIT_ASSERT_EQUAL(true, foo > std::u16string_view());
        CPPUNIT_ASSERT_EQUAL(true, foo >= std::u16string_view());
        CPPUNIT_ASSERT_EQUAL(false, std::u16string_view() == foo);
        CPPUNIT_ASSERT_EQUAL(true, std::u16string_view() != foo);
        CPPUNIT_ASSERT_EQUAL(true, std::u16string_view() < foo);
        CPPUNIT_ASSERT_EQUAL(true, std::u16string_view() <= foo);
        CPPUNIT_ASSERT_EQUAL(false, std::u16string_view() > foo);
        CPPUNIT_ASSERT_EQUAL(false, std::u16string_view() >= foo);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), OUString("foo").indexOf(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), OUString("foo").lastIndexOf(std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), OUString("foo").lastIndexOf(std::u16string_view(), 3));
        CPPUNIT_ASSERT_EQUAL(
            OUString("foobarfoo"),
            OUString("foobarfoo").replaceFirst(std::u16string_view(), std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(
            OUString("barfoo"),
            OUString("foobarfoo").replaceFirst(std::u16string_view(u"foo"), std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(
            OUString("foobarfoo"),
            OUString("foobarfoo").replaceFirst(std::u16string_view(), std::u16string_view(u"baz")));
        CPPUNIT_ASSERT_EQUAL(OUString("barfoo"),
                             OUString("foobarfoo").replaceFirst("foo", std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(OUString("foobarfoo"),
                             OUString("foobarfoo").replaceFirst(std::u16string_view(), "baz"));
        CPPUNIT_ASSERT_EQUAL(
            OUString("foobarfoo"),
            OUString("foobarfoo").replaceAll(std::u16string_view(), std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(
            OUString("bar"),
            OUString("foobarfoo").replaceAll(std::u16string_view(u"foo"), std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(
            OUString("foobarfoo"),
            OUString("foobarfoo").replaceAll(std::u16string_view(), std::u16string_view(u"baz")));
        CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                             OUString("foobarfoo").replaceAll("foo", std::u16string_view()));
        CPPUNIT_ASSERT_EQUAL(OUString("foobarfoo"),
                             OUString("foobarfoo").replaceAll(std::u16string_view(), "baz"));
        CPPUNIT_ASSERT_EQUAL(OUString(), OUString::createFromAscii(std::string_view()));
    }

    void ustringbuffer()
    {
        OUStringBuffer b("foo");
        b.append(std::u16string_view());
        CPPUNIT_ASSERT_EQUAL(OUString("foo"), b.toString());
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(string);
    CPPUNIT_TEST(stringbuffer);
    CPPUNIT_TEST(ustring);
    CPPUNIT_TEST(ustringbuffer);
    CPPUNIT_TEST_SUITE_END();
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
