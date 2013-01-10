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
    void testOUInt();
    void testOInt();
    void testOUFloat();
    void testOFloat();
    void testOUDouble();
    void testODouble();

    CPPUNIT_TEST_SUITE(valueX);
    CPPUNIT_TEST(testOUInt);
    CPPUNIT_TEST(testOInt);
    CPPUNIT_TEST(testOUFloat);
    CPPUNIT_TEST(testOFloat);
    CPPUNIT_TEST(testOUDouble);
    CPPUNIT_TEST(testODouble);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::strings::valueX);

template< typename T >
void testInt() {
    T val1 = T::valueOf( 30039062 );
    T val2 = T::number( 30039062 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    val1 = T::valueOf( 30039062L );
    val2 = T::number( 30039062L );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    // test the overloading resolution

    sal_uInt32 u32 = 30039062;
    val1 = T::valueOf( static_cast<sal_Int32>(u32) );
    val2 = T::number( u32 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    sal_Int32 s32 = 30039062;
    val1 = T::valueOf( s32 );
    val2 = T::number( s32 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    sal_uInt64 u64 = 30039062;
    val1 = T::valueOf( static_cast<sal_Int64>(u64) );
    val2 = T::number( u64 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    sal_Int64 s64 = 30039062;
    val1 = T::valueOf( s64 );
    val2 = T::number( s64 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
}

void test::strings::valueX::testOUInt() {
    testInt<rtl::OUString>();
}

void test::strings::valueX::testOInt() {
    testInt<rtl::OString>();
}

template< typename T >
void testFloat() {
    T val1 = T::valueOf( 30039062.0f );
    T val2 = T::number( 30039062.0f );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
}

void test::strings::valueX::testOUFloat() {
    testFloat<rtl::OUString>();
}

void test::strings::valueX::testOFloat() {
    testFloat<rtl::OString>();
}

template< typename T >
void testDouble() {
    T val1 = T::valueOf( 30039062.0 );
    T val2 = T::number( 30039062.0 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
}

void test::strings::valueX::testOUDouble() {
    testDouble<rtl::OUString>();
}

void test::strings::valueX::testODouble() {
    testDouble<rtl::OString>();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
