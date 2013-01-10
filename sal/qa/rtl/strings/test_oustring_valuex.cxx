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

namespace test { namespace oustring {

class valueX : public CppUnit::TestFixture {
public:
    void testBool();
    void testChar();
    void testInt();

    CPPUNIT_TEST_SUITE(valueX);
    CPPUNIT_TEST(testBool);
    CPPUNIT_TEST(testChar);
    CPPUNIT_TEST(testInt);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::valueX);

void test::oustring::valueX::testBool() {
    rtl::OUString val1 = rtl::OUString::valueOf( sal_True );
    rtl::OUString val2 = rtl::OUString::valueBool( sal_True );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( sal_False );
    val2 = rtl::OUString::valueBool( sal_False );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>(1) );
    val2 = rtl::OUString::valueBool( 1 );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>(0) );
    val2 = rtl::OUString::valueBool( 0 );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>('X') );
    val2 = rtl::OUString::valueBool( 'X' );
    CPPUNIT_ASSERT( val1 == val2 );
    val1 = rtl::OUString::valueOf( static_cast<sal_Bool>(NULL) );
    val2 = rtl::OUString::valueBool( NULL );
    CPPUNIT_ASSERT( val1 == val2 );
}

void test::oustring::valueX::testChar() {
    rtl::OUString val1 = rtl::OUString::valueOf( static_cast<sal_Unicode>('X') );
    rtl::OUString val2 = rtl::OUString::valueChar( 'X' );
    CPPUNIT_ASSERT( val1 == val2 );
}

void test::oustring::valueX::testInt() {
    rtl::OUString val1 = rtl::OUString::valueOf( 30039062 );
    rtl::OUString val2 = rtl::OUString::valueInt( 30039062 );
    CPPUNIT_ASSERT( val1 == val2 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
