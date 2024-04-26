/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

namespace test::oustringbuffer {

class ToString: public CppUnit::TestFixture {
private:
    void testEmptyToString();
    void testToString();

    CPPUNIT_TEST_SUITE(ToString);
    CPPUNIT_TEST(testEmptyToString);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST_SUITE_END();
};

}

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustringbuffer::ToString);

void test::oustringbuffer::ToString::testEmptyToString() {
    OUStringBuffer sb;
    OUString str = sb.toString();
    CPPUNIT_ASSERT_EQUAL(OUString(), str);
}

void test::oustringbuffer::ToString::testToString() {
    OUStringBuffer sb("test string");
    OUString str = sb.toString();
    CPPUNIT_ASSERT_EQUAL( u"test string"_ustr, str );
    // returned OUString must be independent from sb
    sb.append( 'a' );
    CPPUNIT_ASSERT_EQUAL( u"test string"_ustr, str );
    sb.setLength(0);
    CPPUNIT_ASSERT_EQUAL( u"test string"_ustr, str );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
