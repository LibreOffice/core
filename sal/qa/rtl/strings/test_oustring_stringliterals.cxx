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
extern bool rtl_string_unittest_const_literal;
extern bool rtl_string_unittest_invalid_conversion;
extern bool rtl_string_unittest_const_literal_function;
extern bool rtl_string_unittest_non_const_literal_function;

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/string.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"

namespace test { namespace oustring {

class StringLiterals: public CppUnit::TestFixture
{
private:
    void checkCtors();
    void checkUsage();
    void checkExtraIntArgument();
    void checkNonconstChar();
    void checkBuffer();

    void testcall( const char str[] );

CPPUNIT_TEST_SUITE(StringLiterals);
CPPUNIT_TEST(checkCtors);
CPPUNIT_TEST(checkUsage);
CPPUNIT_TEST(checkExtraIntArgument);
CPPUNIT_TEST(checkNonconstChar);
CPPUNIT_TEST(checkBuffer);
CPPUNIT_TEST_SUITE_END();
};

// reset the flag, evaluate the expression and return
// whether the string literal ctor was used (i.e. whether the conversion was valid)
#define VALID_CONVERSION( expression ) \
    ( \
    rtl_string_unittest_invalid_conversion = false, \
    ( void ) rtl::OUString( expression ), \
    ( void ) rtl::OUStringBuffer( expression ), \
    !rtl_string_unittest_invalid_conversion )

void test::oustring::StringLiterals::checkCtors()
{
    CPPUNIT_ASSERT( VALID_CONVERSION( "test" ));
    const char good1[] = "test";
    CPPUNIT_ASSERT( VALID_CONVERSION( good1 ));

    CPPUNIT_ASSERT( !VALID_CONVERSION( (const char*) "test" ));
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
// but there does not appear a way to detect this by compile time (runtime will complain).
// RTL_CONSTASCII_USTRINGPARAM() has the same flaw.
    const char bad5[][ 6 ] = { "test", "test2" };
//    CPPUNIT_ASSERT( VALID_CONVERSION( bad5[ 0 ] ));
    CPPUNIT_ASSERT( VALID_CONVERSION( bad5[ 1 ] ));

// Check that contents are correct and equal to the case when RTL_CONSTASCII_USTRINGPARAM is used.
// Also check that embedded \0 is included.
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "" ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "" )));
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "\0" ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\0" )));
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "ab" ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ab" )));
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "a\0b" ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "a\0b" )));
}

void test::oustring::StringLiterals::testcall( const char str[] )
{
    CPPUNIT_ASSERT( !VALID_CONVERSION( rtl::OUString( str )));
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
    CPPUNIT_ASSERT( foo == "foo" );
    CPPUNIT_ASSERT( "foo" == foo );
    CPPUNIT_ASSERT( foo != "bar" );
    CPPUNIT_ASSERT( "foo" != bar );
    CPPUNIT_ASSERT( foobarfoo.indexOf( "foo", 1 ) == 6 );
    CPPUNIT_ASSERT( foobarfoo.lastIndexOf( "foo" ) == 6 );
    CPPUNIT_ASSERT( foobarfoo.replaceFirst( "foo", test ) == "testbarfoo" );
    CPPUNIT_ASSERT( foobarfoo.replaceFirst( "foo", "test" ) == "testbarfoo" );
    CPPUNIT_ASSERT( foobarfoo.replaceAll( "foo", test ) == "testbartest" );
    CPPUNIT_ASSERT( foobarfoo.replaceAll( "foo", "test" ) == "testbartest" );
    // if this is not true, some of the calls above converted to OUString
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal == false );
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
    CPPUNIT_ASSERT( !VALID_CONVERSION( rtl::OUString( "footest" ).replaceAll( test, bar )));
    CPPUNIT_ASSERT( !VALID_CONVERSION( rtl::OUString( "footest" ).replaceAll( consttest, bar )));
    CPPUNIT_ASSERT( !VALID_CONVERSION( rtl::OUString( "footest" ).replaceAll( test, constbar )));
    CPPUNIT_ASSERT( rtl::OUString( "foobar" ) == rtl::OUString( "footest" ).replaceAll( consttest, constbar ));
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
    CPPUNIT_ASSERT( !VALID_CONVERSION( buf.append( d )));
    CPPUNIT_ASSERT( !VALID_CONVERSION( buf.insert( 0, d )));
}

}} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::StringLiterals);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
