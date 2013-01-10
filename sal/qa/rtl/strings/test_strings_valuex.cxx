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
    T val2 = T::fromInt( 30039062 );
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
    T val2 = T::fromFloat( 30039062.0f );
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
    T val2 = T::fromDouble( 30039062.0 );
    CPPUNIT_ASSERT_EQUAL( val1, val2 );
}

void test::strings::valueX::testOUDouble() {
    testDouble<rtl::OUString>();
}

void test::strings::valueX::testODouble() {
    testDouble<rtl::OString>();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
