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

#include <utility>

#include <sal/types.h>
#include <config_global.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/string.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"

extern bool rtl_string_unittest_const_literal;
bool rtl_string_unittest_invalid_conversion;

namespace test { namespace oustring {

class StringLiterals: public CppUnit::TestFixture
{
private:
    void checkCtors();
    void checkUsage();
    void checkExtraIntArgument();
    void checkNonconstChar();
    void checkBuffer();
    void checkOUStringLiteral();
    void checkOUStringLiteral1();
    void checkUtf16();

    void testcall( const char str[] );

CPPUNIT_TEST_SUITE(StringLiterals);
CPPUNIT_TEST(checkCtors);
CPPUNIT_TEST(checkUsage);
CPPUNIT_TEST(checkExtraIntArgument);
CPPUNIT_TEST(checkNonconstChar);
CPPUNIT_TEST(checkBuffer);
CPPUNIT_TEST(checkOUStringLiteral);
CPPUNIT_TEST(checkOUStringLiteral1);
CPPUNIT_TEST(checkUtf16);
CPPUNIT_TEST_SUITE_END();
};

// reset the flag, evaluate the expression and return
// whether the string literal ctor was used (i.e. whether the conversion was valid)
template<typename T> bool VALID_CONVERSION( T && expression )
{
    rtl_string_unittest_invalid_conversion = false;
    // OK to std::forward expression twice; what is relevant in both ctor calls
    // is not the content of the passed argument (which is ignored anyway by the
    // special RTL_STRING_UNITTEST ctors) but only its type:
    ( void ) rtl::OUString( std::forward<T>(expression) );
    ( void ) rtl::OUStringBuffer( std::forward<T>(expression) );
    return !rtl_string_unittest_invalid_conversion;
}
template<typename T> bool VALID_CONVERSION_CALL( T f )
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

void test::oustring::StringLiterals::checkExtraIntArgument()
{
    // This makes sure that using by mistake RTL_CONSTASCII_STRINGPARAM does not trigger a different
    // overload, i.e. the second argument to match() in this case is the indexFrom argument,
    // but with the macro it would contain the length of the string. Therefore
    // match( RTL_CONSTASCII_STRINGPARAM( "bar" )) would be match( "bar", 3 ), which would be
    // true when called for OUString( "foobar" ). But this should not happen because of the
    // &foo[0] trick in the RTL_CONSTASCII_STRINGPARAM macro.
    CPPUNIT_ASSERT( !rtl::OUString("foobar").match( "bar" ));
    CPPUNIT_ASSERT( !rtl::OUString("foobar").match( RTL_CONSTASCII_STRINGPARAM( "bar" )));
}

void test::oustring::StringLiterals::checkNonconstChar()
{ // check that non-const char[] data do not trigger string literal overloads
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "foobar" ), rtl::OUString( "footest" ).replaceAll( "test", "bar" ));
    char test[] = "test";
    char bar[] = "bar";
    const char consttest[] = "test";
    const char constbar[] = "bar";
    CPPUNIT_ASSERT(
        !VALID_CONVERSION_CALL(
            [&test, &bar]() {
                return rtl::OUString("footest").replaceAll(test, bar); }));
    CPPUNIT_ASSERT(
        !VALID_CONVERSION_CALL(
            [&consttest, &bar]() {
                return rtl::OUString("footest").replaceAll(consttest, bar); }));
    CPPUNIT_ASSERT(
        !VALID_CONVERSION(
            [&test, &constbar]() {
                return rtl::OUString("footest").replaceAll(test, constbar); }));
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "foobar" ), rtl::OUString( "footest" ).replaceAll( consttest, constbar ));
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
        ? rtlunittest::OUStringLiteral("a")
        : rtlunittest::OUStringLiteral("bb");
}

}

void test::oustring::StringLiterals::checkOUStringLiteral()
{
    CPPUNIT_ASSERT(bool(conditional(true) == "a"));
    CPPUNIT_ASSERT(bool(conditional(false) == "bb"));

    rtl::OUString s1(rtlunittest::OUStringLiteral("abc"));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abc"), s1);
    s1 = rtlunittest::OUStringLiteral("de");
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("de"), s1);
    s1 += rtlunittest::OUStringLiteral("fde");
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("defde"), s1);
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        s1.reverseCompareTo(rtlunittest::OUStringLiteral("defde")));
    CPPUNIT_ASSERT(
        s1.equalsIgnoreAsciiCase(rtlunittest::OUStringLiteral("DEFDE")));
    CPPUNIT_ASSERT(s1.match(rtlunittest::OUStringLiteral("fde"), 2));
    CPPUNIT_ASSERT(
        s1.matchIgnoreAsciiCase(rtlunittest::OUStringLiteral("FDE"), 2));
    rtl::OUString s2;
    CPPUNIT_ASSERT(s1.startsWith(rtlunittest::OUStringLiteral("de"), &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("fde"), s2);
    CPPUNIT_ASSERT(
        s1.startsWithIgnoreAsciiCase(
            rtlunittest::OUStringLiteral("DEFD"), &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("e"), s2);
    CPPUNIT_ASSERT(s1.endsWith(rtlunittest::OUStringLiteral("de"), &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("def"), s2);
    CPPUNIT_ASSERT(
        s1.endsWithIgnoreAsciiCase(rtlunittest::OUStringLiteral("EFDE"), &s2));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("d"), s2);
    CPPUNIT_ASSERT(bool(s1 == rtlunittest::OUStringLiteral("defde")));
    CPPUNIT_ASSERT(bool(rtlunittest::OUStringLiteral("defde") == s1));
    CPPUNIT_ASSERT(s1 != rtlunittest::OUStringLiteral("abc"));
    CPPUNIT_ASSERT(rtlunittest::OUStringLiteral("abc") != s1);
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(3), s1.indexOf(rtlunittest::OUStringLiteral("de"), 1));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(3), s1.lastIndexOf(rtlunittest::OUStringLiteral("de")));
    sal_Int32 i = 0;
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst(
            rtlunittest::OUStringLiteral("de"), rtl::OUString("abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst(
            rtl::OUString("de"), rtlunittest::OUStringLiteral("abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst(
            rtlunittest::OUStringLiteral("de"),
            rtlunittest::OUStringLiteral("abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst(rtlunittest::OUStringLiteral("de"), "abc", &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfde"),
        s1.replaceFirst("de", rtlunittest::OUStringLiteral("abc"), &i));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), i);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll(
            rtlunittest::OUStringLiteral("de"), rtl::OUString("abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll(
            rtl::OUString("de"), rtlunittest::OUStringLiteral("abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll(
            rtlunittest::OUStringLiteral("de"),
            rtlunittest::OUStringLiteral("abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll(rtlunittest::OUStringLiteral("de"), "abc"));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcfabc"),
        s1.replaceAll("de", rtlunittest::OUStringLiteral("abc")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcdef"),
        rtl::OUString(
            rtl::OUString("abc") + rtlunittest::OUStringLiteral("def")));
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("abcdef"),
        rtl::OUString(
            rtlunittest::OUStringLiteral("abc") + rtl::OUString("def")));
    rtl::OUStringBuffer b(rtlunittest::OUStringLiteral("abc"));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abc"), b.toString());
    b.append(rtlunittest::OUStringLiteral("def"));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abcdef"), b.toString());
    b.insert(2, rtlunittest::OUStringLiteral("gabab"));
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("abgababcdef"), b.toString());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(3), b.indexOf(rtlunittest::OUStringLiteral("ab"), 1));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(5), b.lastIndexOf(rtlunittest::OUStringLiteral("ab")));
}

void test::oustring::StringLiterals::checkOUStringLiteral1()
{
    auto l1 = rtlunittest::OUStringLiteral1('A');
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('A'), l1.c);

    char const c2 = 'A';
    auto l2 = rtlunittest::OUStringLiteral1(c2);
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('A'), l2.c);

    char c3 = 'A'; auto l3 = rtlunittest::OUStringLiteral1(c3);
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('A'), l3.c);

    auto l4 = rtlunittest::OUStringLiteral1(sal_Unicode('A'));
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('A'), l4.c);

    sal_Unicode const c5 = 0x100;
    auto l5 = rtlunittest::OUStringLiteral1(c5);
    CPPUNIT_ASSERT_EQUAL(c5, l5.c);

    rtl::OUString s1{rtlunittest::OUStringLiteral1('A')};
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s1.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('A'), s1[0]);

    CPPUNIT_ASSERT_EQUAL(
        true, rtl::OUString("A") == rtlunittest::OUStringLiteral1('A'));
    CPPUNIT_ASSERT_EQUAL(
        false, rtl::OUString("AB") == rtlunittest::OUStringLiteral1('A'));
    CPPUNIT_ASSERT_EQUAL(
        false, rtl::OUString("A") != rtlunittest::OUStringLiteral1('A'));
    CPPUNIT_ASSERT_EQUAL(
        true, rtl::OUString("AB") != rtlunittest::OUStringLiteral1('A'));

    rtl::OUString s2("A" + rtlunittest::OUStringLiteral1('b'));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), s2.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('A'), s2[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('b'), s2[1]);
}

void test::oustring::StringLiterals::checkUtf16() {
#if HAVE_CXX11_UTF16_STRING_LITERAL && !defined SAL_W32
        // sal_Unicode is still wchar_t not char16_t even for MSVC 2015
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
#endif
}

}} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::StringLiterals);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
