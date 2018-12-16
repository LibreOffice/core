/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <o3tl/string_view.hxx>

namespace {

class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCharLiteral);
    CPPUNIT_TEST(testChar16Literal);
    CPPUNIT_TEST(testChar32Literal);
    CPPUNIT_TEST(testWcharLiteral);
    CPPUNIT_TEST(testOperations);
    CPPUNIT_TEST(testOutput);
    CPPUNIT_TEST_SUITE_END();

    void testCharLiteral() {
        char * const s1 = const_cast<char *>("foo");
        o3tl::string_view v1(s1);
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(3), v1.size());
        char const * const s2 = "foo";
        o3tl::string_view v2(s2);
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(3), v2.size());
        char s3[] = "foo";
        o3tl::string_view v3(s3);
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(3), v3.size());
        o3tl::string_view v4("foo");
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(3), v4.size());
    }

    void testChar16Literal() {
        char16_t * const s1 = const_cast<char16_t *>(u"foo");
        o3tl::u16string_view v1(s1);
        CPPUNIT_ASSERT_EQUAL(o3tl::u16string_view::size_type(3), v1.size());
        char16_t const * const s2 = u"foo";
        o3tl::u16string_view v2(s2);
        CPPUNIT_ASSERT_EQUAL(o3tl::u16string_view::size_type(3), v2.size());
        char16_t s3[] = u"foo";
        o3tl::u16string_view v3(s3);
        CPPUNIT_ASSERT_EQUAL(o3tl::u16string_view::size_type(3), v3.size());
        o3tl::u16string_view v4(u"foo");
        CPPUNIT_ASSERT_EQUAL(o3tl::u16string_view::size_type(3), v4.size());
    }

    void testChar32Literal() {
        char32_t * const s1 = const_cast<char32_t *>(U"foo");
        o3tl::u32string_view v1(s1);
        CPPUNIT_ASSERT_EQUAL(o3tl::u32string_view::size_type(3), v1.size());
        char32_t const * const s2 = U"foo";
        o3tl::u32string_view v2(s2);
        CPPUNIT_ASSERT_EQUAL(o3tl::u32string_view::size_type(3), v2.size());
        char32_t s3[] = U"foo";
        o3tl::u32string_view v3(s3);
        CPPUNIT_ASSERT_EQUAL(o3tl::u32string_view::size_type(3), v3.size());
        o3tl::u32string_view v4(U"foo");
        CPPUNIT_ASSERT_EQUAL(o3tl::u32string_view::size_type(3), v4.size());
    }

    void testWcharLiteral() {
        wchar_t * const s1 = const_cast<wchar_t *>(L"foo");
        o3tl::wstring_view v1(s1);
        CPPUNIT_ASSERT_EQUAL(o3tl::wstring_view::size_type(3), v1.size());
        wchar_t const * const s2 = L"foo";
        o3tl::wstring_view v2(s2);
        CPPUNIT_ASSERT_EQUAL(o3tl::wstring_view::size_type(3), v2.size());
        wchar_t s3[] = L"foo";
        o3tl::wstring_view v3(s3);
        CPPUNIT_ASSERT_EQUAL(o3tl::wstring_view::size_type(3), v3.size());
        o3tl::wstring_view v4(L"foo");
        CPPUNIT_ASSERT_EQUAL(o3tl::wstring_view::size_type(3), v4.size());
    }

    void testOperations() {
        o3tl::string_view const v("fox");
        auto npos = o3tl::string_view::npos;
            // o3tl::basic_string_view::npos will be (implicitly) inline with
            // C++17, but for now can't be passed as 'const T& expected'
            // argument into CppUnit::assertEquals, so take this detour
        CPPUNIT_ASSERT_EQUAL('f', *v.begin());
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::difference_type(3), v.end() - v.begin());
        CPPUNIT_ASSERT_EQUAL('f', *v.cbegin());
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::difference_type(3), v.cend() - v.cbegin());
        CPPUNIT_ASSERT_EQUAL('x', *v.rbegin());
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::difference_type(3), v.rend() - v.rbegin());
        CPPUNIT_ASSERT_EQUAL('x', *v.crbegin());
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::difference_type(3), v.crend() - v.crbegin());
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(3), v.size());
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(3), v.length());
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::npos - 1, v.max_size());
        CPPUNIT_ASSERT(!v.empty());
        CPPUNIT_ASSERT_EQUAL('o', v[1]);
        try {
            v.at(o3tl::string_view::npos);
            CPPUNIT_FAIL("missing exception");
        } catch (std::out_of_range &) {}
        CPPUNIT_ASSERT_EQUAL('f', v.at(0));
        CPPUNIT_ASSERT_EQUAL('x', v.at(2));
        try {
            v.at(3);
            CPPUNIT_FAIL("missing exception");
        } catch (std::out_of_range &) {}
        CPPUNIT_ASSERT_EQUAL('f', v.front());
        CPPUNIT_ASSERT_EQUAL('x', v.back());
        CPPUNIT_ASSERT_EQUAL('f', *v.data());
        {
            o3tl::string_view v1("fox");
            v1.remove_prefix(2);
            CPPUNIT_ASSERT_EQUAL('x', v1.front());
            CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(1), v1.size());
        }
        {
            o3tl::string_view v1("fox");
            v1.remove_suffix(2);
            CPPUNIT_ASSERT_EQUAL('f', v1.front());
            CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(1), v1.size());
        }
        {
            o3tl::string_view v1("fox");
            o3tl::string_view v2("giraffe");
            v1.swap(v2);
            CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(7), v1.size());
            CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(3), v2.size());
        }
        {
            char a[2];
            auto n = v.copy(a, 10, 1);
            CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(2), n);
            CPPUNIT_ASSERT_EQUAL('o', a[0]);
            CPPUNIT_ASSERT_EQUAL('x', a[1]);
        }
        {
            auto v1 = v.substr(1);
            CPPUNIT_ASSERT_EQUAL('o', v1.front());
            CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(2), v1.size());
        }
        CPPUNIT_ASSERT(v.compare(o3tl::string_view("foo")) > 0);
        CPPUNIT_ASSERT(v.compare(0, 2, o3tl::string_view("foo")) < 0);
        CPPUNIT_ASSERT_EQUAL(
            0, v.compare(0, 2, o3tl::string_view("foo"), 0, 2));
        CPPUNIT_ASSERT_EQUAL(0, v.compare("fox"));
        CPPUNIT_ASSERT(v.compare(1, 2, "abc") > 0);
        CPPUNIT_ASSERT_EQUAL(0, v.compare(1, 2, "oxx", 2));
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(1), v.find("ox"));
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(1), v.find('o'));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find("oxx", 0, 2));
        CPPUNIT_ASSERT_EQUAL(npos, v.find("oxx"));
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(1), v.rfind("ox"));
        CPPUNIT_ASSERT_EQUAL(o3tl::string_view::size_type(1), v.rfind('o'));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1),
            v.rfind("oxx", o3tl::string_view::npos, 2));
        CPPUNIT_ASSERT_EQUAL(npos, v.rfind("oxx"));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_first_of("nop"));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_first_of('o'));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_first_of("nofx", 0, 2));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(0), v.find_first_of("nofx"));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_last_of("nop"));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_last_of('o'));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1),
            v.find_last_of("nofx", o3tl::string_view::npos, 2));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(2), v.find_last_of("nofx"));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_first_not_of("fx"));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_first_not_of('f'));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_first_not_of("fxo", 0, 2));
        CPPUNIT_ASSERT_EQUAL(npos, v.find_first_not_of("fxo"));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_last_not_of("fx"));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1), v.find_last_not_of('x'));
        CPPUNIT_ASSERT_EQUAL(
            o3tl::string_view::size_type(1),
            v.find_last_not_of("fxo", o3tl::string_view::npos, 2));
        CPPUNIT_ASSERT_EQUAL(npos, v.find_last_not_of("fxo"));
    }

    void testOutput() {
        std::ostringstream s;
        s << o3tl::string_view("foo");
        CPPUNIT_ASSERT_EQUAL(std::string("foo"), s.str());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
