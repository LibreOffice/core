/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */


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
    
    
    
    CPPUNIT_ASSERT( !CONST_CTOR_USED( bad5 )); 
    CPPUNIT_ASSERT( !CONST_CTOR_USED( bad6 ));
#endif




    const char bad7[][ 6 ] = { "test", "test2" };

    CPPUNIT_ASSERT( CONST_CTOR_USED( bad7[ 1 ] ));


    CPPUNIT_ASSERT( rtl::OString( (const char*)"" ) == rtl::OString( "" ));
    CPPUNIT_ASSERT( rtl::OString( (const char*)"ab" ) == rtl::OString( "ab" ));


    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "" )) == rtl::OString( "" ));
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "ab" )) == rtl::OString( "ab" ));
}

const char test::ostring::StringLiterals::bad5[] = "test";
char test::ostring::StringLiterals::bad6[] = "test";

void test::ostring::StringLiterals::testcall( const char str[] )
{
#ifndef _MSC_VER
    bool result_tmp;
    CPPUNIT_ASSERT( !CONST_CTOR_USED( str ));
#else
    
    (void)str;
#endif
}

void test::ostring::StringLiterals::checkUsage()
{


    rtl::OString foo( "foo" );
    rtl::OString FoO( "FoO" );
    rtl::OString foobarfoo( "foobarfoo" );
    rtl::OString foobar( "foobar" );
    rtl::OString FooBaRfoo( "FooBaRfoo" );
    rtl::OString FooBaR( "FooBaR" );
    rtl::OString bar( "bar" );

    rtl_string_unittest_const_literal = false; 
    rtl_string_unittest_non_const_literal_function = false; 
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT_EQUAL( foo, rtl::OString() = "foo" );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
    rtl_string_unittest_const_literal_function = false;
    CPPUNIT_ASSERT( FoO.equalsIgnoreAsciiCase( "fOo" ));
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == true );
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



    
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal == false );
    
    CPPUNIT_ASSERT( rtl_string_unittest_non_const_literal_function == false );
}

void test::ostring::StringLiterals::checkNonConstUsage()
{

    rtl::OString foo( "foo" );
    rtl::OString FoO( "FoO" );
    rtl::OString foobarfoo( "foobarfoo" );
    rtl::OString foobar( "foobar" );
    rtl::OString FooBaRfoo( "FooBaRfoo" );
    rtl::OString FooBaR( "FooBaR" );
    rtl::OString bar( "bar" );
    char foo_c[] = "foo";
    char bar_c[] = "bar";
    char fOo_c[] = "fOo";
    char bAr_c[] = "bAr";

    rtl_string_unittest_const_literal = false; 
    rtl_string_unittest_const_literal_function = false; 
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


    
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal == false );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == false );
}

void test::ostring::StringLiterals::checkBuffer()
{
    rtl::OStringBuffer buf;
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

    rtl::OString foobazbard( "foobazbard" );
    rtl::OString foodbazbard( "foodbazbard" );
    rtl_string_unittest_const_literal = false; 
    rtl_string_unittest_const_literal_function = false; 
    char d[] = "d";
    CPPUNIT_ASSERT_EQUAL( foobazbard, buf.append( d ).toString());
    CPPUNIT_ASSERT_EQUAL( foodbazbard, buf.insert( 3, d ).toString() );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal == false );
    CPPUNIT_ASSERT( rtl_string_unittest_const_literal_function == false );
}

#undef CONST_CTOR_USED

}} 

CPPUNIT_TEST_SUITE_REGISTRATION(test::ostring::StringLiterals);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
