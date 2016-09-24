/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/ustring.hxx"
#include <iostream>

namespace test { namespace strings {

class valueX : public CppUnit::TestFixture {
public:
    void testOBoolean();
    void testOUBoolean();
    void testOUInt();
    void testOInt();
    void testOUFloat();
    void testOFloat();

    CPPUNIT_TEST_SUITE(valueX);
    CPPUNIT_TEST(testOBoolean);
    CPPUNIT_TEST(testOUBoolean);
    CPPUNIT_TEST(testOUInt);
    CPPUNIT_TEST(testOInt);
    CPPUNIT_TEST(testOUFloat);
    CPPUNIT_TEST(testOFloat);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::strings::valueX);

namespace {

template< typename T >
void testBoolean() {
    CPPUNIT_ASSERT_EQUAL( T( "false" ), T::boolean( false ) );
    CPPUNIT_ASSERT_EQUAL( T( "true" ), T::boolean( true ) );
}

}

void test::strings::valueX::testOBoolean() {
    testBoolean<rtl::OString>();
}

void test::strings::valueX::testOUBoolean() {
    testBoolean<rtl::OUString>();
}

template< typename T >
void testInt() {
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( 30039062 ));

    // test the overloading resolution

    CPPUNIT_ASSERT_EQUAL( T( "30" ), T::number( static_cast< signed char >( 30 )));
    CPPUNIT_ASSERT_EQUAL( T( "30" ), T::number( static_cast< unsigned char >( 30 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039" ), T::number( static_cast< short >( 30039 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039" ), T::number( static_cast< unsigned short >( 30039 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< int >( 30039062 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< unsigned int >( 30039062 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< long >( 30039062 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< unsigned long >( 30039062 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< long long >( 30039062 )));
    // The highest bit set in unsigned long long may not actually work.
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< unsigned long long >( 30039062 )));

    CPPUNIT_ASSERT_EQUAL( T( "30" ), T::number( static_cast< sal_Int8 >( 30 )));
    CPPUNIT_ASSERT_EQUAL( T( "30" ), T::number( static_cast< sal_uInt8 >( 30 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039" ), T::number( static_cast< sal_Int16 >( 30039 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039" ), T::number( static_cast< sal_uInt16 >( 30039 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< sal_Int32 >( 30039062 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< sal_uInt32 >( 30039062 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< sal_Int64 >( 30039062 )));
    CPPUNIT_ASSERT_EQUAL( T( "30039062" ), T::number( static_cast< sal_uInt64 >( 30039062 )));

    // The implementation internally uses sal_Int64 etc. types, so check ranges.
    assert( sizeof( int ) <= sizeof( sal_Int32 ));
    assert( sizeof( long ) <= sizeof( sal_Int64 ));
    assert( sizeof( long long ) <= sizeof( sal_Int64 ));
    assert( sizeof( unsigned int ) < sizeof( sal_Int64 ));
}

void test::strings::valueX::testOUInt() {
    testInt<rtl::OUString>();
}

void test::strings::valueX::testOInt() {
    testInt<rtl::OString>();
}

template< typename T >
void testFloat() {
    CPPUNIT_ASSERT_EQUAL( T( "39062.2" ), T::number( 39062.2f ));
    CPPUNIT_ASSERT_EQUAL( T( "30039062.2" ), T::number( 30039062.2 ));
    // long double not supported
}

void test::strings::valueX::testOUFloat() {
    testFloat<rtl::OUString>();
}

void test::strings::valueX::testOFloat() {
    testFloat<rtl::OString>();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
