/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
