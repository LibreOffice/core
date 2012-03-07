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

#include "sal/config.h"
#include "sal/precppunit.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/string.h"
#include "rtl/string.hxx"

namespace test { namespace ostring {

class StringLiterals: public CppUnit::TestFixture
{
private:
    void checkCtors();

    void testcall( const char str[] );

CPPUNIT_TEST_SUITE(StringLiterals);
CPPUNIT_TEST(checkCtors);
CPPUNIT_TEST_SUITE_END();
};

// reset the flag, call OString ctor with the given argument and return
// whether the string literal ctor was used
#define CONST_CTOR_USED( argument ) \
    ( \
    rtl_string_unittest_const_literal = false, \
    ( void ) rtl::OString( argument ), \
    rtl_string_unittest_const_literal )

void test::ostring::StringLiterals::checkCtors()
{
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

// This one is technically broken, since the first element is 6 characters test\0\0,
// but there does not appear a way to detect this by compile time (runtime will complain).
// RTL_CONSTASCII_USTRINGPARAM() has the same flaw.
    const char bad5[][ 6 ] = { "test", "test2" };
    CPPUNIT_ASSERT( CONST_CTOR_USED( bad5[ 0 ] ));
    CPPUNIT_ASSERT( CONST_CTOR_USED( bad5[ 1 ] ));

// Check that contents are correct and equal to the case when const char* ctor is used.
    CPPUNIT_ASSERT( rtl::OString( (const char*)"" ) == rtl::OString( "" ));
    CPPUNIT_ASSERT( rtl::OString( (const char*)"ab" ) == rtl::OString( "ab" ));

// Check that contents are correct and equal to the case when RTL_CONSTASCII_STRINGPARAM is used.
// Check also that embedded \0 is included (RTL_CONSTASCII_STRINGPARAM does the same,
// const char* ctor does not, but it seems to make more sense to include it when
// it's explicitly mentioned in the string literal).
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "" )) == rtl::OString( "" ));
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "\0" )) == rtl::OString( "\0" ));
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "ab" )) == rtl::OString( "ab" ));
    CPPUNIT_ASSERT( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "a\0b" )) == rtl::OString( "a\0b" ));
}

void test::ostring::StringLiterals::testcall( const char str[] )
{
    CPPUNIT_ASSERT( !CONST_CTOR_USED( str ));
}

#undef CONST_CTOR_USED

}} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::ostring::StringLiterals);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
