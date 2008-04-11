/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: test_string.cxx,v $
 * $Revision: 1.3 $
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

#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include "comphelper/string.hxx"
#include "cppunit/simpleheader.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

class Test: public CppUnit::TestFixture {
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test() {
    rtl::OUString s1(RTL_CONSTASCII_USTRINGPARAM("foobarbar"));
    sal_Int32 n1;
    rtl::OUString s2(
        comphelper::string::searchAndReplace(
            s1, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baaz")), 0, &n1));
    CPPUNIT_ASSERT(
        s2 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foobaazbar")));
    CPPUNIT_ASSERT(n1 == 3);
    sal_Int32 n2;
    rtl::OUString s3(
        comphelper::string::searchAndReplace(
            s2, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bz")),
            n1 + RTL_CONSTASCII_LENGTH("baaz"), &n2));
    CPPUNIT_ASSERT(
        s3 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foobaazbz")));
    CPPUNIT_ASSERT(n2 == 7);
    sal_Int32 n3;
    rtl::OUString s4(
        comphelper::string::searchAndReplace(
            s3, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baz")),
            n2 + RTL_CONSTASCII_LENGTH("bz"), &n3));
    CPPUNIT_ASSERT(s4 == s3);
    CPPUNIT_ASSERT(n3 == -1);
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
