/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Lubos Lunak <l.lunak@suse.cz> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// activate the extra needed ctor
#define RTL_STRING_UNITTEST
bool rtl_string_unittest_const_literal;
bool rtl_string_unittest_invalid_conversion;
bool rtl_string_unittest_const_literal_function;
bool rtl_string_unittest_non_const_literal_function;

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"

namespace rtlunittest {

template< typename charT, typename traits > std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, rtl::OString const & string)
{
    return stream << string.getStr();
        // best effort; potentially loses data due to embedded null characters
}

}

namespace test { namespace ostring {

class StringLiterals: public CppUnit::TestFixture
{
private:
    void checkCtors();
    void checkUsage();
    void checkNonConstUsage();
    void checkBuffer();

    void testcall( const char str[] );

    static const char bad5[];
    static char bad6[];

CPPUNIT_TEST_SUITE(StringLiterals);
CPPUNIT_TEST(checkCtors);
CPPUNIT_TEST(checkUsage);
CPPUNIT_TEST(checkNonConstUsage);
CPPUNIT_TEST(checkBuffer);
CPPUNIT_TEST_SUITE_END();
};

// reset the flag, call OString ctor with the given argument and return
// whether the string literal ctor was used
#define CONST_CTOR_USED( argument ) \
    ( \
    rtl_string_unittest_const_literal = false, \
    ( void ) rtl::OString( argument ), \
    result_tmp = rtl_string_unittest_const_literal, \
    rtl_string_unittest_const_literal = false, \
    ( void ) rtl::OStringBuffer( argument ), \
    rtl_string_unittest_const_literal && result_tmp )

void test::ostring::StringLiterals::checkCtors()
{
// string literal ctors do not work with SFINAE broken and are disabled
#ifndef HAVE_SFINAE_ANONYMOUS_BROKEN
    bool result_tmp;
    CPPUNIT_ASSERT( CONST_CTOR_USED( "test" ));
    const char good1[] = "test";
    CPPUNIT_ASSERT( CONST_CTOR_USED( good1 ));

    CPPUNIT_ASSERT( !CONST_CTOR_USED( (const char*) "test" ));
    const char* bad1 = good1;
    CPPUNIT_ASSERT( !CONST_CTOR_USED( bad1 ));
    char bad2[] = "test";
    CPPUNIT_ASSERT( !CONST_CTOR_USED( bad2 ));
    char* bad3 = bad2;
    CPPUNIT_ASSERT( !CONST_CTOR_USED( bad3 ));
    const char* bad4[] = { "test1" };
    CPPUNIT_ASSERT( !CONST_CTOR_USED( bad4[ 0 ] ));
    testcall( good1 );
#ifndef _MSC_VER
    // this is actually not supposed to work (see discussion in stringutils.hxx),
    // but gcc and clang somehow manage, so keep it used, just in case some other problem
    // shows up somewhen in the future
    CPPUNIT_ASSERT( !CONST_CTOR_USED( bad5 )); // size is not known here
    CPPUNIT_ASSERT( !CONST_CTOR_USED( bad6 ));
#endif

// This one is technically broken, since the first element is 6 characters test\0\0,
// but there does not appear a way to detect this by compile time (runtime will complain).
// RTL_CONSTASCII_USTRINGPARAM() has the same flaw.
    const char bad7[][ 6 ] = { "test", "test2" };
    CPPUNIT_ASSERT( CONST_CTOR_USED( bad7[ 0 ] ));
    CPPUNIT_ASSERT( CONST_CTOR_USED( bad7[ 1 ] ));

// Check that contents are correct and equal to the case when const char* ctor is used.
    CPPUNIT_ASSERT( rtl::OString( (const char*)"" ) == rtl::OString( "" ));
    CPPUNIT_ASSERT( rtl::OString( (const char*)"ab" ) == rtl::OString( "ab" ));

// Check that contents are correct and equal to the case when RTL_CONSTASCII_STRINGPARAM is used.
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "" )) == rtl::OString( "" ));
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "ab" )) == rtl::OString( "ab" ));
#if 0
// This is currently disabled because it can't be consistent with HAVE_SFINAE_ANONYMOUS_BROKEN.
// Since the situation wasn't quite consistent even before, there should be no big harm.

// Check also that embedded \0 is included (RTL_CONSTASCII_STRINGPARAM does the same,
// const char* ctor does not, but it seems to make more sense to include it when
// it's explicitly mentioned in the string literal).
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "\0" )) == rtl::OString( "\0" ));
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "a\0b" )) == rtl::OString( "a\0b" ));
#endif
#endif
}

const char test::ostring::StringLiterals::bad5[] = "test";
char test::ostring::StringLiterals::bad6[] = "test";

void test::ostring::StringLiterals::testcall( const char str[] )
{
#ifndef _MSC_VER
    bool result_tmp;
    CPPUNIT_ASSERT( !CONST_CTOR_USED( str ));
#else
    // MSVC just errors out on this for some reason, which is fine as well
    (void)str;
#endif
}

void test::ostring::StringLiterals::checkUsage()
{
// simply check that all string literal based calls work as expected
// also check that they really use string literal overload and do not convert to OString
    rtl::OString foo( "foo" );
    rtl::OString FoO( "FoO" );
    rtl::OString foobarfoo( "foobarfoo" );
    rtl::OString foobar( "foobar" );
    rtl::OString FooBaRfoo( "FooBaRfoo" );
    rtl::OString FooBaR( "FooBaR" );
    rtl::OString bar( "bar" );
    rtl::OString test( "test" );

    rtl_string_unittest_const_literal = false; // start checking for OString conversions
    rtl_string_unittest_non_const_literal_function = false; // and check for non-const variants
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT_EQUAL( foo, rtl::OString() = "foo" );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
#ifndef HAVE_SFINAE_ANONYMOUS_BROKEN
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( FoO.equalsIgnoreAsciiCase( "fOo" ));
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
#endif
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( foobarfoo.match( "bar", 3 ));
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( foobar.match( "foo" ));
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( FooBaRfoo.matchIgnoreAsciiCase( "bAr", 3 ));
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( FooBaR.matchIgnoreAsciiCase( "fOo" ));
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( foobar.startsWith( "foo" ));
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( foobar.endsWith( "bar" ));
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
//    rtl_string_unittest_const_literal_function = false;
//    CPPUNIT_ASSERT( FooBaR.endsWithIgnoreAsciiCase( "bar" ));
//    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( foo == "foo" );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( "foo" == foo );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( foo != "bar" );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( "foo" != bar );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( foobarfoo.indexOf( "foo", 1 ) == 6 );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
//    rtl_string_unittest_const_literal_function = false;
//    CPPUNIT_ASSERT( foobarfoo.lastIndexOf( "foo" ) == 6 );
//    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    // if this is not true, some of the calls above converted to OString
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal == false );
    // if this is not true, some of the calls above used non-const variants
    CPPUNIT_ASSERT( rtl_string_unittest_non_const_literal_function == false );
}

void test::ostring::StringLiterals::checkNonConstUsage()
{
// check that (non-const) char[] overloads work and do not use const char[] overloads
    rtl::OString foo( "foo" );
    rtl::OString FoO( "FoO" );
    rtl::OString foobarfoo( "foobarfoo" );
    rtl::OString foobar( "foobar" );
    rtl::OString FooBaRfoo( "FooBaRfoo" );
    rtl::OString FooBaR( "FooBaR" );
    rtl::OString bar( "bar" );
    rtl::OString test( "test" );
    char foo_c[] = "foo";
    char bar_c[] = "bar";
    char fOo_c[] = "fOo";
    char bAr_c[] = "bAr";

    rtl_string_unittest_const_literal = false; // start checking for OString conversions
    rtl_string_unittest_const_literal_function = false; // and check for const variants
    CPPUNIT_ASSERT_EQUAL( foo, rtl::OString() = (const char*)foo_c );
    CPPUNIT_ASSERT_EQUAL( foo, rtl::OString() = foo_c );
    CPPUNIT_ASSERT( FoO.equalsIgnoreAsciiCase( (const char*)fOo_c ));
    CPPUNIT_ASSERT( FoO.equalsIgnoreAsciiCase( fOo_c ));
    CPPUNIT_ASSERT( foobarfoo.match( (const char*)bar_c, 3 ));
    CPPUNIT_ASSERT( foobarfoo.match( bar_c, 3 ));
    CPPUNIT_ASSERT( foobar.match( (const char*)foo_c ));
    CPPUNIT_ASSERT( foobar.match( foo_c ));
    CPPUNIT_ASSERT( FooBaRfoo.matchIgnoreAsciiCase( (const char*)bAr_c, 3 ));
    CPPUNIT_ASSERT( FooBaRfoo.matchIgnoreAsciiCase( bAr_c, 3 ));
    CPPUNIT_ASSERT( FooBaR.matchIgnoreAsciiCase( (const char*)fOo_c ));
    CPPUNIT_ASSERT( FooBaR.matchIgnoreAsciiCase( fOo_c ));
    CPPUNIT_ASSERT( foobar.startsWith( (const char*)foo_c ));
    CPPUNIT_ASSERT( foobar.startsWith( foo_c ));
    CPPUNIT_ASSERT( foobar.endsWith( (const char*)bar_c ));
    CPPUNIT_ASSERT( foobar.endsWith( bar_c ));
//    CPPUNIT_ASSERT( FooBaR.endsWithIgnoreAsciiCase( (const char*)bar_c ));
//    CPPUNIT_ASSERT( FooBaR.endsWithIgnoreAsciiCase( bar_c ));
    CPPUNIT_ASSERT( foo == (const char*)foo_c );
    CPPUNIT_ASSERT( foo == foo_c );
    CPPUNIT_ASSERT( (const char*)foo_c == foo );
    CPPUNIT_ASSERT( foo_c == foo );
    CPPUNIT_ASSERT( foo != (const char*)bar_c );
    CPPUNIT_ASSERT( foo != bar_c );
    CPPUNIT_ASSERT( (const char*)foo_c != bar );
    CPPUNIT_ASSERT( foo_c != bar );
    CPPUNIT_ASSERT( foobarfoo.indexOf( (const char*)foo_c, 1 ) == 6 );
    CPPUNIT_ASSERT( foobarfoo.indexOf( foo_c, 1 ) == 6 );
//    CPPUNIT_ASSERT( foobarfoo.lastIndexOf( (const char*)foo_c ) == 6 );
//    CPPUNIT_ASSERT( foobarfoo.lastIndexOf( foo_c ) == 6 );
    // if this is not true, some of the calls above used const variants
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal == false );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == false );
}

void test::ostring::StringLiterals::checkBuffer()
{
    rtl::OStringBuffer buf;
#ifndef HAVE_SFINAE_ANONYMOUS_BROKEN
    rtl_string_unittest_const_literal_function = false;
    buf.append( "foo" );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    CPPUNIT_ASSERT_EQUAL( rtl::OString( "foo" ), buf.toString());
    rtl_string_unittest_const_literal_function = false;
    buf.append( "bar" );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    CPPUNIT_ASSERT_EQUAL( rtl::OString( "foobar" ), buf.toString());
    rtl_string_unittest_const_literal_function = false;
    buf.insert( 3, "baz" );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    CPPUNIT_ASSERT_EQUAL( rtl::OString( "foobazbar" ), buf.toString());
#else
    buf.append( "foobazbar" );
#endif

    rtl::OString foobazbard( "foobazbard" );
    rtl::OString foodbazbard( "foodbazbard" );
    rtl_string_unittest_const_literal = false; // start checking for OString conversions
    rtl_string_unittest_const_literal_function = false; // and check for const variants
    char d[] = "d";
    CPPUNIT_ASSERT_EQUAL( foobazbard, buf.append( d ).toString());
    CPPUNIT_ASSERT_EQUAL( foodbazbard, buf.insert( 3, d ).toString() );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal == false );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == false );
}

#undef CONST_CTOR_USED

}} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::ostring::StringLiterals);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
