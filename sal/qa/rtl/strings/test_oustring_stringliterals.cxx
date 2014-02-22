/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */


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




    const char bad5[][ 6 ] = { "test", "test2" };

    CPPUNIT_ASSERT( VALID_CONVERSION( bad5[ 1 ] ));


    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "" ), rtl::OUString( "" ));
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "ab" ), rtl::OUString( "ab" ));
#if 0


    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "\0" ), rtl::OUString( "\0" ));
    CPPUNIT_ASSERT_EQUAL( rtl::OUString( "a\0b" ), rtl::OUString( "a\0b" ));
#endif
}

void test::oustring::StringLiterals::testcall( const char str[] )
{
    CPPUNIT_ASSERT( !VALID_CONVERSION( rtl::OUString( str )));
}

void test::oustring::StringLiterals::checkUsage()
{


    rtl::OUString foo( "foo" );
    rtl::OUString FoO( "FoO" );
    rtl::OUString foobarfoo( "foobarfoo" );
    rtl::OUString foobar( "foobar" );
    rtl::OUString FooBaRfoo( "FooBaRfoo" );
    rtl::OUString FooBaR( "FooBaR" );
    rtl::OUString bar( "bar" );
    rtl::OUString test( "test" );

    rtl_string_unittest_const_literal = false; 
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
    CPPUNIT_ASSERT( foo.reverseCompareTo( "foo" ) == 0 );
    
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal == false );
}

void test::oustring::StringLiterals::checkExtraIntArgument()
{
    
    
    
    
    
    
    CPPUNIT_ASSERT( !rtl::OUString("foobar").match( "bar" ));
    CPPUNIT_ASSERT( !rtl::OUString("foobar").match( RTL_CONSTASCII_STRINGPARAM( "bar" )));
}

void test::oustring::StringLiterals::checkNonconstChar()
{ 
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
    CPPUNIT_ASSERT( !VALID_CONVERSION( buf.append( rtl::OUString( d ))));
    CPPUNIT_ASSERT( !VALID_CONVERSION( buf.append( rtl::OUStringBuffer( d ))));
}

}} 

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::StringLiterals);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
