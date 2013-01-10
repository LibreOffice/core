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

    CPPUNIT_TEST_SUITE(valueX);
    CPPUNIT_TEST(testOUBool);
    CPPUNIT_TEST(testOBool);
    CPPUNIT_TEST(testOUChar);
    CPPUNIT_TEST(testOChar);
    CPPUNIT_TEST(testOUInt);
    CPPUNIT_TEST(testOInt);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::strings::valueX);

void test::strings::valueX::testOUBool() {
    rtl::OUString val1, val2;

    val1 = rtl::OUString::valueOf( sal_True );
    val2 = rtl::OUString::valueOfBool( sal_True );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( sal_False );
    val2 = rtl::OUString::valueOfBool( sal_False );
    CPPUNIT_ASSERT( val1 == val2 );

    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>(true) );
    val2 = rtl::OUString::valueOfBool( true );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>(false) );
    val2 = rtl::OUString::valueOfBool( false );

    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>(1) );
    val2 = rtl::OUString::valueOfBool( 1 );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>(0) );
    val2 = rtl::OUString::valueOfBool( 0 );
    CPPUNIT_ASSERT( val1 == val2 );

    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>('X') );
    val2 = rtl::OUString::valueOfBool( 'X' );
    CPPUNIT_ASSERT( val1 == val2 );

    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>(NULL) );
    val2 = rtl::OUString::valueOfBool( NULL );
    CPPUNIT_ASSERT( val1 == val2 );
}

void test::strings::valueX::testOBool() {
    rtl::OString val1, val2;

    val1 = rtl::OString::valueOf( sal_True );
    val2 = rtl::OString::valueOfBool( sal_True );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OString::valueOf( sal_False );
    val2 = rtl::OString::valueOfBool( sal_False );
    CPPUNIT_ASSERT( val1 == val2 );

    val1 = rtl::OString::valueOf( static_cast<sal_Bool>(true) );
    val2 = rtl::OString::valueOfBool( true );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OString::valueOf( static_cast<sal_Bool>(false) );
    val2 = rtl::OString::valueOfBool( false );

    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OString::valueOf( static_cast<sal_Bool>(1) );
    val2 = rtl::OString::valueOfBool( 1 );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OString::valueOf( static_cast<sal_Bool>(0) );
    val2 = rtl::OString::valueOfBool( 0 );
    CPPUNIT_ASSERT( val1 == val2 );

    val1 = rtl::OString::valueOf( static_cast<sal_Bool>('X') );
    val2 = rtl::OString::valueOfBool( 'X' );
    CPPUNIT_ASSERT( val1 == val2 );

    val1 = rtl::OString::valueOf( static_cast<sal_Bool>(NULL) );
    val2 = rtl::OString::valueOfBool( NULL );
    CPPUNIT_ASSERT( val1 == val2 );
}

void test::strings::valueX::testOUChar() {
    rtl::OUString val1 = rtl::OUString::valueOf( static_cast<sal_Unicode>('X') );
    rtl::OUString val2 = rtl::OUString::valueOfChar( 'X' );
    CPPUNIT_ASSERT( val1 == val2 );
}

void test::strings::valueX::testOChar() {
    rtl::OString val1 = rtl::OString::valueOf( static_cast<sal_Char>('X') );
    rtl::OString val2 = rtl::OString::valueOfChar( 'X' );
    CPPUNIT_ASSERT( val1 == val2 );
}

void test::strings::valueX::testOUInt() {
    rtl::OUString val1 = rtl::OUString::valueOf( 30039062 );
    rtl::OUString val2 = rtl::OUString::valueOfInt( 30039062 );
    CPPUNIT_ASSERT( val1 == val2 );
}

void test::strings::valueX::testOInt() {
    rtl::OString val1 = rtl::OString::valueOf( 30039062 );
    rtl::OString val2 = rtl::OString::valueOfInt( 30039062 );
    CPPUNIT_ASSERT( val1 == val2 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
