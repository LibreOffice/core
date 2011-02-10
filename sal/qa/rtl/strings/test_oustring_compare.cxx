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
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/string.h"
#include "rtl/ustring.hxx"

namespace test { namespace oustring {

class Compare: public CppUnit::TestFixture
{
private:
    void equalsIgnoreAsciiCaseAscii();
    void compareNumeric();

CPPUNIT_TEST_SUITE(Compare);
CPPUNIT_TEST(equalsIgnoreAsciiCaseAscii);
CPPUNIT_TEST(compareNumeric);
CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::Compare);

void test::oustring::Compare::equalsIgnoreAsciiCaseAscii()
{
    CPPUNIT_ASSERT(!rtl::OUString().equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString().equalsIgnoreAsciiCaseAsciiL(
                       RTL_CONSTASCII_STRINGPARAM("abc")));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii(""));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("")));

    CPPUNIT_ASSERT(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abcd")).
                   equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii("abcd"));
}

void test::oustring::Compare::compareNumeric()
{
// --- Some generic tests to ensure we do not alter original behavior
// outside what we want
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("ABC"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("ABC")))) == 0
    );
    // Case sensitivity
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("ABC"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc")))) < 0
    );
    // Reverse
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("ABC")))) > 0
    );
    // First shorter
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("alongstring"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("alongerstring")))) > 0
    );
    // Second shorter
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("alongerstring"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("alongstring")))) < 0
    );
// -- Here we go on natural order, each one is followed by classic compare and the reverse comparison
    // That's why we originally made the patch
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 9"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 10")))) < 0
    );
    // Original behavior
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 9"))).compareTo(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 10")))) > 0
    );
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 10"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("Heading 9")))) > 0
    );
    // Harder
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 4th"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 10th")))) < 0
    );
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 4th"))).compareTo(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 10th")))) > 0
    );
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 10th"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("July, the 4th")))) > 0
    );
    // Hardest
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc08"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc010")))) < 0
    );
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc08"))).compareTo(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc010")))) > 0
    );
    CPPUNIT_ASSERT(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc010"))).compareToNumeric(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(("abc08")))) > 0
    );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
