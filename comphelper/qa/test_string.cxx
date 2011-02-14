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

#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include "comphelper/string.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

class TestString: public CppUnit::TestFixture
{
public:
    void test();
    void testNatural();

    CPPUNIT_TEST_SUITE(TestString);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testNatural);
    CPPUNIT_TEST_SUITE_END();
};

void TestString::test()
{
    rtl::OUString s1(RTL_CONSTASCII_USTRINGPARAM("foobarbar"));
    sal_Int32 n1;
    rtl::OUString s2(
        comphelper::string::searchAndReplaceAsciiL(
            s1, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baaz")), 0, &n1));
    CPPUNIT_ASSERT(
        s2 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foobaazbar")));
    CPPUNIT_ASSERT(n1 == 3);
    sal_Int32 n2;
    rtl::OUString s3(
        comphelper::string::searchAndReplaceAsciiL(
            s2, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bz")),
            n1 + RTL_CONSTASCII_LENGTH("baaz"), &n2));
    CPPUNIT_ASSERT(
        s3 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foobaazbz")));
    CPPUNIT_ASSERT(n2 == 7);
    sal_Int32 n3;
    rtl::OUString s4(
        comphelper::string::searchAndReplaceAsciiL(
            s3, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baz")),
            n2 + RTL_CONSTASCII_LENGTH("bz"), &n3));
    CPPUNIT_ASSERT(s4 == s3);
    CPPUNIT_ASSERT(n3 == -1);
}

void TestString::testNatural()
{
    using namespace comphelper::string;
// --- Some generic tests to ensure we do not alter original behavior
// outside what we want
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("ABC"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("ABC")))) == 0
    );
    // Case sensitivity
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("ABC"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc")))) < 0
    );
    // Reverse
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("ABC")))) > 0
    );
    // First shorter
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("alongstring"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("alongerstring")))) > 0
    );
    // Second shorter
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("alongerstring"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("alongstring")))) < 0
    );
// -- Here we go on natural order, each one is followed by classic compare and the reverse comparison
    // That's why we originally made the patch
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 9"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 10")))) < 0
    );
    // Original behavior
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 9"))).compareTo(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 10")))) > 0
    );
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 10"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 9")))) > 0
    );
    // Harder
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 4th"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 10th")))) < 0
    );
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 4th"))).compareTo(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 10th")))) > 0
    );
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 10th"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 4th")))) > 0
    );
    // Hardest
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc08"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc010")))) < 0
    );
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc08"))).compareTo(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc010")))) > 0
    );
    CPPUNIT_ASSERT(
        compareNatural(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc010"))), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc08")))) > 0
    );
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestString);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
