/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#include "sal/config.h"

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
    buf[0] = static_cast< sal_Int8 >(20 | 0x80); // sequence type | cache flag
    buf[1] = static_cast< sal_Int8 >(binaryurp::cache::ignore >> 8);
    buf[2] = static_cast< sal_Int8 >(binaryurp::cache::ignore & 0xFF);
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
    buf[0] = static_cast< sal_Int8 >(20 | 0x80); // sequence type | cache flag
    buf[1] = static_cast< sal_Int8 >(binaryurp::cache::ignore >> 8);
    buf[2] = static_cast< sal_Int8 >(binaryurp::cache::ignore & 0xFF);
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
    } catch (css::io::IOException &) {}
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();
