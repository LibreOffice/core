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
    void testOUBool();
    void testOBool();
    void testOUChar();
    void testOChar();
    void testOUInt();
    void testOInt();
    void testOUFloat();
    void testOFloat();
    void testOUDouble();
    void testODouble();

    CPPUNIT_TEST_SUITE(valueX);
    CPPUNIT_TEST(testOUBool);
    CPPUNIT_TEST(testOBool);
    CPPUNIT_TEST(testOUChar);
    CPPUNIT_TEST(testOChar);
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
void testBool() {
    T val1, val2;

    val1 = T::valueOf( sal_True );
    val2 = T::valueOfBool( sal_True );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
    val1 = T::valueOf( sal_False );
    val2 = T::valueOfBool( sal_False );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    val1 = T::valueOf( static_cast<sal_Bool>(true) );
    val2 = T::valueOfBool( true );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
    val1 = T::valueOf( static_cast<sal_Bool>(false) );
    val2 = T::valueOfBool( false );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    val1 = T::valueOf( static_cast<sal_Bool>(1) );
    val2 = T::valueOfBool( 1 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
    val1 = T::valueOf( static_cast<sal_Bool>(0) );
    val2 = T::valueOfBool( 0 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    val1 = T::valueOf( static_cast<sal_Bool>('X') );
    val2 = T::valueOfBool( 'X' );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );

    val1 = T::valueOf( static_cast<sal_Bool>(NULL) );
    val2 = T::valueOfBool( NULL );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
}

void test::strings::valueX::testOUBool() {
    testBool<rtl::OUString>();
}

void test::strings::valueX::testOBool() {
    testBool<rtl::OString>();
}

void test::strings::valueX::testOUChar() {
    rtl::OUString val1 = rtl::OUString::valueOf( static_cast<sal_Unicode>('X') );
    rtl::OUString val2 = rtl::OUString::valueOfChar( 'X' );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
}

void test::strings::valueX::testOChar() {
    rtl::OString val1 = rtl::OString::valueOf( static_cast<sal_Char>('X') );
    rtl::OString val2 = rtl::OString::valueOfChar( 'X' );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
}

template< typename T >
void testInt() {
    T val1 = T::valueOf( 30039062 );
    T val2 = T::valueOfInt( 30039062 );
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
    T val2 = T::valueOfFloat( 30039062.0f );
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
    T val2 = T::valueOfDouble( 30039062.0 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
}

void test::strings::valueX::testOUDouble() {
    testDouble<rtl::OUString>();
}

void test::strings::valueX::testODouble() {
    testDouble<rtl::OString>();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
