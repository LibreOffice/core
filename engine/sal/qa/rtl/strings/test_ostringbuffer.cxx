/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <rtl/ustrbuf.hxx>
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

    void testConcatRefersTo()
    {
        OStringBuffer b("abcdef");
        const char* const pBegin = b.getStr();
        const char* const pEnd = pBegin + b.getLength();
        static constexpr OString s = "abc"_ostr;
        CPPUNIT_ASSERT((b + "x").refersTo(pBegin, pEnd));
        CPPUNIT_ASSERT(
            (OString::Concat("x") + std::string_view(b).substr(3)).refersTo(pBegin, pEnd));
        CPPUNIT_ASSERT((s + "x" + b.getStr()).refersTo(pBegin, pEnd));
        CPPUNIT_ASSERT((s + "x" + pEnd).refersTo(pBegin, pEnd));
        CPPUNIT_ASSERT((s + "x" + std::string_view(pEnd, 1)).refersTo(pBegin, pEnd));
        CPPUNIT_ASSERT(!(s + "x" + std::string_view(b).substr(6)).refersTo(pBegin, pEnd));
        CPPUNIT_ASSERT(!(s + OString::number(42)).refersTo(pBegin, pEnd));

        OUStringBuffer u(u"abc");
        CPPUNIT_ASSERT((OUString::Concat("x") + OUString::unacquired(u))
                           .refersTo(u.getStr(), u.getStr() + u.getLength()));

        // A cleared buffer and an empty string may share the static empty string
        OStringBuffer c("abc");
        const OString e(c.makeStringAndClear().copy(3));
        c.append(OString::Concat(e.getStr()) + "x");
        CPPUNIT_ASSERT_EQUAL("x"_ostr, c.toString());
        OStringBuffer g("abc");
        const OString h(g.makeStringAndClear().copy(3));
        g.append(h.getStr(), 1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), g.getLength());
        OUStringBuffer v(u"abc");
        const OUString w(v.makeStringAndClear().copy(3));
        v.append(w.getStr(), 1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), v.getLength());
    }

    void testAssignOwnText()
    {
        OStringBuffer b("abc");
        b = std::string_view(b);
        CPPUNIT_ASSERT_EQUAL("abc"_ostr, b.toString());

        OUStringBuffer u(u"abc");
        u = OUString::unacquired(u);
        CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, u.toString());

        // A cleared buffer and an empty string may share the static empty string
        OStringBuffer c("abc");
        const OString e(c.makeStringAndClear().copy(3));
        c = std::string_view(e.getStr(), 1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), c.getLength());
        OUStringBuffer d(u"abc");
        const OUString f(d.makeStringAndClear().copy(3));
        d = std::u16string_view(f.getStr(), 1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), d.getLength());
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStringView);
    CPPUNIT_TEST(testOStringChar);
    CPPUNIT_TEST(testConcatRefersTo);
    CPPUNIT_TEST(testAssignOwnText);
    CPPUNIT_TEST_SUITE_END();
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
