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

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/string.h"
#include "rtl/ustring.hxx"

namespace test { namespace oustring {

class Compare: public CppUnit::TestFixture
{
private:
    void equalsIgnoreAsciiCaseAscii();

    void compareToIgnoreAsciiCase();

CPPUNIT_TEST_SUITE(Compare);
CPPUNIT_TEST(equalsIgnoreAsciiCaseAscii);
CPPUNIT_TEST(compareToIgnoreAsciiCase);
CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::Compare);

void test::oustring::Compare::equalsIgnoreAsciiCaseAscii()
{
    const char* const abc = "abc";
    const char* const abcd = "abcd";
    const char* const empty = "";
    CPPUNIT_ASSERT(!rtl::OUString().equalsIgnoreAsciiCaseAscii(abc));
    CPPUNIT_ASSERT(!rtl::OUString().equalsIgnoreAsciiCaseAsciiL(abc,3));
    CPPUNIT_ASSERT(!rtl::OUString("abc").
                   equalsIgnoreAsciiCaseAscii(empty));
    CPPUNIT_ASSERT(!rtl::OUString("abc").
                   equalsIgnoreAsciiCaseAsciiL(empty,0));

    CPPUNIT_ASSERT(rtl::OUString("abc").
                   equalsIgnoreAsciiCaseAscii(abc));
    CPPUNIT_ASSERT(!rtl::OUString("abcd").
                   equalsIgnoreAsciiCaseAscii(abc));
    CPPUNIT_ASSERT(!rtl::OUString("abc").
                   equalsIgnoreAsciiCaseAscii(abcd));
}

void test::oustring::Compare::compareToIgnoreAsciiCase()
{
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        rtl::OUString("abc").compareToIgnoreAsciiCase("ABC"));
    CPPUNIT_ASSERT(
        rtl::OUString("ABC").compareToIgnoreAsciiCase("abcdef")
        < 0);
    CPPUNIT_ASSERT(
        rtl::OUString("A").compareToIgnoreAsciiCase("_") > 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
