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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"

namespace test { namespace oustringbuffer {

class ToString: public CppUnit::TestFixture {
private:
    void testToString();

    CPPUNIT_TEST_SUITE(ToString);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustringbuffer::ToString);

void test::oustringbuffer::ToString::testToString() {
    rtl::OUStringBuffer sb(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test string")));
    rtl::OUString str = sb.toString();
    CPPUNIT_ASSERT(str.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("test string")));
    // returned OUString must be independent from sb
    sb.append( (sal_Unicode)'a' );
    CPPUNIT_ASSERT(str.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("test string")));
    sb.setLength(0);
    CPPUNIT_ASSERT(str.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("test string")));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
