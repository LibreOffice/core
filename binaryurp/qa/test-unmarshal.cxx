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

#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppu/unotype.hxx"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

#include "../source/bridge.hxx"
#include "../source/cache.hxx"
#include "../source/readerstate.hxx"
#include "../source/unmarshal.hxx"

namespace {

namespace css = com::sun::star;

class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testTypeOfBooleanSequence);
    CPPUNIT_TEST(testTypeOfVoidSequence);
    CPPUNIT_TEST_SUITE_END();

    void testTypeOfBooleanSequence();

    void testTypeOfVoidSequence();
};

void Test::testTypeOfBooleanSequence() {
    binaryurp::ReaderState state;
    css::uno::Sequence< sal_Int8 > buf(13);
    buf[0] = static_cast< sal_Int8 >(static_cast< sal_uInt8 >(20 | 0x80)); // sequence type | cache flag
    buf[1] = static_cast< sal_Int8 >(static_cast< sal_uInt8 >(binaryurp::cache::ignore >> 8));
    buf[2] = static_cast< sal_Int8 >(static_cast< sal_uInt8 >(binaryurp::cache::ignore & 0xFF));
    buf[3] = RTL_CONSTASCII_LENGTH("[]boolean");
    buf[4] = '[';
    buf[5] = ']';
    buf[6] = 'b';
    buf[7] = 'o';
    buf[8] = 'o';
    buf[9] = 'l';
    buf[10] = 'e';
    buf[11] = 'a';
    buf[12] = 'n';
    binaryurp::Unmarshal m(rtl::Reference< binaryurp::Bridge >(), state, buf);
    css::uno::TypeDescription t(m.readType());
    CPPUNIT_ASSERT(
        t.equals(
            css::uno::TypeDescription(
                cppu::UnoType< css::uno::Sequence< bool > >::get())));
    m.done();
}

void Test::testTypeOfVoidSequence() {
    binaryurp::ReaderState state;
    css::uno::Sequence< sal_Int8 > buf(10);
    buf[0] = static_cast< sal_Int8 >(static_cast< sal_uInt8 >(20 | 0x80)); // sequence type | cache flag
    buf[1] = static_cast< sal_Int8 >(static_cast< sal_uInt8 >(binaryurp::cache::ignore >> 8));
    buf[2] = static_cast< sal_Int8 >(static_cast< sal_uInt8 >(binaryurp::cache::ignore & 0xFF));
    buf[3] = RTL_CONSTASCII_LENGTH("[]void");
    buf[4] = '[';
    buf[5] = ']';
    buf[6] = 'v';
    buf[7] = 'o';
    buf[8] = 'i';
    buf[9] = 'd';
    binaryurp::Unmarshal m(rtl::Reference< binaryurp::Bridge >(), state, buf);
    try {
        m.readType();
        CPPUNIT_FAIL("exception expected");
    } catch (const css::io::IOException &) {}
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
