/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// activate the extra needed ctor
#define RTL_STRING_UNITTEST

#include <sal/config.h>

#include <string>
#include <string_view>
#include <utility>

#include <o3tl/cppunittraitshelper.hxx>
#include <sal/types.h>
#include <config_global.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/string.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

extern bool rtl_string_unittest_const_literal;
bool rtl_string_unittest_invalid_conversion;

namespace test::oustring {

class StringLiterals: public CppUnit::TestFixture
{
private:
    void checkCtors();
    void checkUsage();
    void checkBuffer();
    void checkOUStringLiteral();
    void checkOUStringChar();
    void checkUtf16();
    void checkEmbeddedNul();

    void testcall( const char str[] );

    // Check that OUStringLiteral ctor is actually constexpr:
    static constexpr rtlunittest::OUStringLiteral dummy{u"dummy"};

CPPUNIT_TEST_SUITE(StringLiterals);
CPPUNIT_TEST(checkCtors);
CPPUNIT_TEST(checkUsage);
CPPUNIT_TEST(checkBuffer);
CPPUNIT_TEST(checkOUStringLiteral);
CPPUNIT_TEST(checkOUStringChar);
CPPUNIT_TEST(checkUtf16);
CPPUNIT_TEST(checkEmbeddedNul);
CPPUNIT_TEST_SUITE_END();
};

// reset the flag, evaluate the expression and return
// whether the string literal ctor was used (i.e. whether the conversion was valid)
template<typename T> static bool VALID_CONVERSION( T && expression )
{
    rtl_string_unittest_invalid_conversion = false;
    // OK to std::forward expression twice; what is relevant in both ctor calls
    // is not the content of the passed argument (which is ignored anyway by the
    // special RTL_STRING_UNITTEST ctors) but only its type:
    ( void ) rtl::OUString( std::forward<T>(expression) );
    ( void ) rtl::OUStringBuffer( std::forward<T>(expression) );
    return !rtl_string_unittest_invalid_conversion;
}
template<typename T> static bool VALID_CONVERSION_CALL( T f )
{
    rtl_string_unittest_invalid_conversion = false;
    ( void ) rtl::OUString( f() );
    ( void ) rtl::OUStringBuffer( f() );
    return !rtl_string_unittest_invalid_conversion;
}

void test::oustring::StringLiterals::checkCtors()
{
    CPPUNIT_ASSERT( VALID_CONVERSION( "test" ));
    const char good1[] = "test";
    CPPUNIT_ASSERT( VALID_CONVERSION( good1 ));

    CPPUNIT_ASSERT( !VALID_CONVERSION( static_cast<const char*>("test") ));
    const char* bad1 = good1;
    CPPUNIT_ASSERT( !VALID_CONVERSION( bad1 ));
    char bad2[] = "test";
    CPPUNIT_ASSERT( !VALID_CONVERSION( bad2 ));
    char* bad3 = bad2;
    CPPUNIT_ASSERT( !VALID_CONVERSION( bad3 ));
    const char* bad4[] = { "test1" };
    CPPUNIT_ASSERT( !VALID_CONVERSION( bad4[ 0 ] ));
    testcall( good1 );

// This one is technically broken, since the first element is 6 characters test\0\0,
// but there does not appear a way to detect this by compile time (runtime will assert()).
// RTL_CONSTASCII_USTRINGPARAM() has the same flaw.
    const char bad5[][ 6 ] = { "test", "test2" };
//    CPPUNIT_ASSERT( VALID_CONVERSION( bad5[ 0 ] ));
    CPPUNIT_ASSERT( VALID_CONVERSION( bad5[ 1 ] ));

// Check that contents are correct and equal to the case when RTL_CONSTASCII_USTRINGPARAM is used.
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "" ), rtl::OUString( "" ));
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "ab" ), rtl::OUString( "ab" ));
#if 0
// Also check that embedded \0 is included.
// In fact, allowing this is probably just trouble, so this now asserts.
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "\0" ), rtl::OUString( "\0" ));
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "a\0b" ), rtl::OUString( "a\0b" ));
#endif
}

void test::oustring::StringLiterals::testcall( const char str[] )
{
    CPPUNIT_ASSERT(
        !VALID_CONVERSION_CALL([&str]() { return rtl::OUString(str); }));
}

void test::oustring::StringLiterals::checkUsage()
{
// simply check that all string literal based calls work as expected
// also check that they really use string literal overload and do not convert to OUString
    rtl::OUString foo( "foo" );
    rtl::OUString FoO( "FoO" );
    rtl::OUString foobarfoo( "foobarfoo" );
    rtl::OUString foobar( "foobar" );
    rtl::OUString FooBaRfoo( "FooBaRfoo" );
    rtl::OUString FooBaR( "FooBaR" );
    rtl::OUString bar( "bar" );
    rtl::OUString test( "test" );

    rtl_string_unittest_const_literal = false; // start checking for OUString conversions
    CPPUNIT_ASSERT_EQUAL( foo, rtl::OUString() = "foo" );
    CPPUNIT_ASSERT( FoO.equalsIgnoreAsciiCase( "fOo" ));
    CPPUNIT_ASSERT( foobarfoo.match( "bar", 3 ));
    CPPUNIT_ASSERT( foobar.match( "foo" ));
    CPPUNIT_ASSERT( FooBaRfoo.matchIgnoreAsciiCase( "bAr", 3 ));
    CPPUNIT_ASSERT( FooBaR.matchIgnoreAsciiCase( "fOo" ));
    CPPUNIT_ASSERT( foobar.startsWith( "foo" ));
    CPPUNIT_ASSERT( FooBaR.startsWithIgnoreAsciiCase( "foo" ));
    CPPUNIT_ASSERT( foobar.endsWith( "bar" ));
    CPPUNIT_ASSERT( FooBaR.endsWithIgnoreAsciiCase( "bar" ));
    CPPUNIT_ASSERT( bool(foo == "foo") );
    CPPUNIT_ASSERT( bool("foo" == foo) );
    CPPUNIT_ASSERT( foo != "bar" );
    CPPUNIT_ASSERT( "foo" != bar );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(6), foobarfoo.indexOf( "foo", 1 ) );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(6), foobarfoo.lastIndexOf( "foo" ) );
    CPPUNIT_ASSERT( bool(foobarfoo.replaceFirst( "foo", test ) == "testbarfoo") );
    CPPUNIT_ASSERT( bool(foobarfoo.replaceFirst( "foo", "test" ) == "testbarfoo") );
    CPPUNIT_ASSERT( bool(foobarfoo.replaceAll( "foo", test ) == "testbartest") );
    CPPUNIT_ASSERT( bool(foobarfoo.replaceAll( "foo", "test" ) == "testbartest") );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(0), foo.reverseCompareTo( "foo" ) );
    // if this is not true, some of the calls above converted to OUString
    CPPUNIT_ASSERT( !rtl_string_unittest_const_literal );
}

void test::oustring::StringLiterals::checkBuffer()
{
    rtl::OUStringBuffer buf;
    buf.append( "foo" );
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "foo" ), buf.toString());
    buf.append( "bar" );
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "foobar" ), buf.toString());
    buf.insert( 3, "baz" );
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "foobazbar" ), buf.toString());
    char d[] = "d";
    CPPUNIT_ASSERT( !VALID_CONVERSION( buf.append( rtl::OUString( d ))));
    CPPUNIT_ASSERT( !VALID_CONVERSION( buf.append( rtl::OUStringBuffer( d ))));
}

namespace {

rtl::OUString conditional(bool flag) {
    return flag
        ? rtlunittest::OUStringLiteral(u"a")
        : rtlunittest::OUStringLiteral(u"bb");
}

}

void test::oustring::StringLiterals::checkOUStringLiteral()
{
    CPPUNIT_ASSERT(bool(conditional(true) == "a"));
    CPPUNIT_ASSERT(bool(conditional(false) == "bb"));

    rtl::OUString s1(rtlunittest::OUStringLiteral(u"abc"));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abc"), s1);
    s1 = rtlunittest::OUStringLiteral(u"de");
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("de"), s1);
    s1 += rtlunittest::OUStringLiteral(u"fde");
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("defde"), s1);
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        s1.reverseCompareTo(rtlunittest::OUStringLiteral(u"defde")));
    CPPUNIT_ASSERT(
        s1.equalsIgnoreAsciiCase(rtlunittest::OUStringLiteral(u"DEFDE")));
    CPPUNIT_ASSERT(s1.match(rtlunittest::OUStringLiteral(u"fde"), 2));
    CPPUNIT_ASSERT(
        s1.matchIgnoreAsciiCase(rtlunittest::OUStringLiteral(u"FDE"), 2));
    rtl::OUString s2;
    CPPUNIT_ASSERT(s1.startsWith(rtlunittest::OUStringLiteral(u"de"), &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("fde"), s2);
    CPPUNIT_ASSERT(
        s1.startsWithIgnoreAsciiCase(
            rtlunittest::OUStringLiteral(u"DEFD"), &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("e"), s2);
    CPPUNIT_ASSERT(s1.endsWith(rtlunittest::OUStringLiteral(u"de"), &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("def"), s2);
    CPPUNIT_ASSERT(
        s1.endsWithIgnoreAsciiCase(rtlunittest::OUStringLiteral(u"EFDE"), &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("d"), s2);
    CPPUNIT_ASSERT(bool(s1 == rtlunittest::OUStringLiteral(u"defde")));
    CPPUNIT_ASSERT(bool(rtlunittest::OUStringLiteral(u"defde") == s1));
    CPPUNIT_ASSERT(s1 != rtlunittest::OUStringLiteral(u"abc"));
    CPPUNIT_ASSERT(rtlunittest::OUStringLiteral(u"abc") != s1);
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(3), s1.indexOf(rtlunittest::OUStringLiteral(u"de"), 1));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(3), s1.lastIndexOf(rtlunittest::OUStringLiteral(u"de")));
    sal_Int32 i = 0;
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst(
            rtlunittest::OUStringLiteral(u"de"), rtl::OUString("abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst(
            rtl::OUString("de"), rtlunittest::OUStringLiteral(u"abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst(
            rtlunittest::OUStringLiteral(u"de"),
            rtlunittest::OUStringLiteral(u"abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst(rtlunittest::OUStringLiteral(u"de"), "abc", &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst("de", rtlunittest::OUStringLiteral(u"abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll(
            rtlunittest::OUStringLiteral(u"de"), rtl::OUString("abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll(
            rtl::OUString("de"), rtlunittest::OUStringLiteral(u"abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll(
            rtlunittest::OUStringLiteral(u"de"),
            rtlunittest::OUStringLiteral(u"abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll(rtlunittest::OUStringLiteral(u"de"), "abc"));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll("de", rtlunittest::OUStringLiteral(u"abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcdef"),
        rtl::OUString(
            rtl::OUString("abc") + rtlunittest::OUStringLiteral(u"def")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcdef"),
        rtl::OUString(
            rtlunittest::OUStringLiteral(u"abc") + rtl::OUString("def")));
    rtl::OUStringBuffer b(rtlunittest::OUStringLiteral(u"abc"));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abc"), b.toString());
    b.append(rtlunittest::OUStringLiteral(u"def"));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abcdef"), b.toString());
    b.insert(2, rtlunittest::OUStringLiteral(u"gabab"));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abgababcdef"), b.toString());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(3), b.indexOf(rtlunittest::OUStringLiteral(u"ab"), 1));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(5), b.lastIndexOf(rtlunittest::OUStringLiteral(u"ab")));
}

void test::oustring::StringLiterals::checkOUStringChar()
{
    auto l1 = rtlunittest::OUStringChar('A');
    CPPUNIT_ASSERT_EQUAL(u'A', l1.c);

    char const c2 = 'A';
    auto l2 = rtlunittest::OUStringChar(c2);
    CPPUNIT_ASSERT_EQUAL(u'A', l2.c);

    char c3 = 'A'; auto l3 = rtlunittest::OUStringChar(c3);
    CPPUNIT_ASSERT_EQUAL(u'A', l3.c);

    auto l4 = rtlunittest::OUStringChar(u'A');
    CPPUNIT_ASSERT_EQUAL(u'A', l4.c);

    sal_Unicode const c5 = 0x100;
    auto l5 = rtlunittest::OUStringChar(c5);
    CPPUNIT_ASSERT_EQUAL(c5, l5.c);

    rtl::OUString s1{rtlunittest::OUStringChar('A')};
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s1.getLength());
    CPPUNIT_ASSERT_EQUAL(u'A', s1[0]);

    CPPUNIT_ASSERT_EQUAL(
        true, rtl::OUString("A") == rtlunittest::OUStringChar('A'));
    CPPUNIT_ASSERT_EQUAL(
        false, rtl::OUString("AB") == rtlunittest::OUStringChar('A'));
    CPPUNIT_ASSERT_EQUAL(
        false, rtl::OUString("A") != rtlunittest::OUStringChar('A'));
    CPPUNIT_ASSERT_EQUAL(
        true, rtl::OUString("AB") != rtlunittest::OUStringChar('A'));

    rtl::OUString s2("A" + rtlunittest::OUStringChar('b'));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), s2.getLength());
    CPPUNIT_ASSERT_EQUAL(u'A', s2[0]);
    CPPUNIT_ASSERT_EQUAL(u'b', s2[1]);
}

void test::oustring::StringLiterals::checkUtf16() {
    rtl::OUString s1(u"abc");
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abc"), s1);
    s1 = u"de";
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("de"), s1);
    s1 += u"fde";
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("defde"), s1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s1.reverseCompareTo(u"defde"));
    CPPUNIT_ASSERT(s1.equalsIgnoreAsciiCase(u"DEFDE"));
    CPPUNIT_ASSERT(s1.match(u"fde", 2));
    CPPUNIT_ASSERT(s1.matchIgnoreAsciiCase(u"FDE", 2));
    rtl::OUString s2;
    CPPUNIT_ASSERT(s1.startsWith(u"de", &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(u"fde"), s2);
    CPPUNIT_ASSERT(s1.startsWithIgnoreAsciiCase(u"DEFD", &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(u"e"), s2);
    CPPUNIT_ASSERT(s1.endsWith(u"de", &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(u"def"), s2);
    CPPUNIT_ASSERT(s1.endsWithIgnoreAsciiCase(u"EFDE", &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(u"d"), s2);
    CPPUNIT_ASSERT(bool(s1 == u"defde"));
    CPPUNIT_ASSERT(bool(u"defde" == s1));
    CPPUNIT_ASSERT(s1 != u"abc");
    CPPUNIT_ASSERT(u"abc" != s1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), s1.indexOf(u"de", 1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), s1.lastIndexOf(u"de"));
    sal_Int32 i = 0;
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfde"),
        s1.replaceFirst(u"de", rtl::OUString("abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfde"),
        s1.replaceFirst(rtl::OUString("de"), u"abc", &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfde"), s1.replaceFirst(u"de", u"abc", &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfde"), s1.replaceFirst(u"de", "abc", &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfde"), s1.replaceFirst("de", u"abc", &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfabc"), s1.replaceAll(u"de", rtl::OUString("abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfabc"), s1.replaceAll(rtl::OUString("de"), u"abc"));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfabc"), s1.replaceAll(u"de", u"abc"));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfabc"), s1.replaceAll(u"de", "abc"));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(u"abcfabc"), s1.replaceAll("de", u"abc"));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcdef"), rtl::OUString(rtl::OUString("abc") + u"def"));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcdef"), rtl::OUString(u"abc" + rtl::OUString("def")));
    rtl::OUStringBuffer b(u"abc");
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abc"), b.toString());
    b.append(u"def");
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abcdef"), b.toString());
    b.insert(2, u"gabab");
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abgababcdef"), b.toString());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), b.indexOf(u"ab", 1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), b.lastIndexOf(u"ab"));
}

void test::oustring::StringLiterals::checkEmbeddedNul() {
    using namespace std::literals;
    rtl::OUString const s("foobar");
    constexpr char16_t const a[] = u"foo\0hidden";
    char16_t const * const p = a;
    CPPUNIT_ASSERT(s.startsWith(a));
    CPPUNIT_ASSERT(s.startsWith(p));
    CPPUNIT_ASSERT(s.startsWith(u"foo\0hidden"));
    CPPUNIT_ASSERT(!s.startsWith(u"foo\0hidden"s));
    CPPUNIT_ASSERT(!s.startsWith(u"foo\0hidden"sv));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::StringLiterals);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
