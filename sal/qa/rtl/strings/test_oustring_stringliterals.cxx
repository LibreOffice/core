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

#include "sal/config.h"
#include "sal/precppunit.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/string.h"
#include "rtl/ustring.hxx"

namespace test { namespace oustring {

class StringLiterals: public CppUnit::TestFixture
{
private:
    void checkCtors();

    void testcall( const char str[] );
    // invalid conversions will trigger templated OUString ctor that creates an empty string
    // (see RTL_STRING_UNITTEST)
    bool validConversion( const rtl::OUString& str ) { return !str.isEmpty(); }

CPPUNIT_TEST_SUITE(StringLiterals);
CPPUNIT_TEST(checkCtors);
CPPUNIT_TEST_SUITE_END();
};

void test::oustring::StringLiterals::checkCtors()
{
    CPPUNIT_ASSERT( validConversion( rtl::OUString( "test" )));
    const char good1[] = "test";
    CPPUNIT_ASSERT( validConversion( rtl::OUString( good1 )));

    CPPUNIT_ASSERT( !validConversion( rtl::OUString( (const char*) "test" )));
    const char* bad1 = good1;
    CPPUNIT_ASSERT( !validConversion( rtl::OUString( bad1 )));
    char bad2[] = "test";
    CPPUNIT_ASSERT( !validConversion( rtl::OUString( bad2 )));
    char* bad3 = bad2;
    CPPUNIT_ASSERT( !validConversion( rtl::OUString( bad3 )));
    const char* bad4[] = { "test1" };
    CPPUNIT_ASSERT( !validConversion( rtl::OUString( bad4[ 0 ] )));
    testcall( good1 );

// This one is technically broken, since the first element is 6 characters test\0\0,
// but there does not appear a way to detect this by compile time (runtime will complain).
// RTL_CONSTASCII_USTRINGPARAM() has the same flaw.
    const char bad5[][ 6 ] = { "test", "test2" };
//    CPPUNIT_ASSERT( validConversion( rtl::OUString( bad5[ 0 ] )));
    CPPUNIT_ASSERT( validConversion( rtl::OUString( bad5[ 1 ] )));
}

void test::oustring::StringLiterals::testcall( const char str[] )
{
    CPPUNIT_ASSERT( !validConversion( rtl::OUString( str )));
}

}} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::StringLiterals);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
