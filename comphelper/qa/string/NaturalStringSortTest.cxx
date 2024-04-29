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

#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XCollator.hpp>

#include <unotest/bootstrapfixturebase.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/ustring.hxx>

using namespace css;

namespace
{
class TestStringNaturalCompare : public test::BootstrapFixtureBase
{
public:
    void testNatural()
    {
        lang::Locale aLocale;
        aLocale.Language = "en";
        aLocale.Country = "US";

        comphelper::string::NaturalStringSorter aSorter(comphelper::getProcessComponentContext(),
                                                        aLocale);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(+0), aSorter.compare(u"ABC"_ustr, u"ABC"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"ABC"_ustr, u"abc"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"abc"_ustr, u"ABC"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1),
                             aSorter.compare(u"alongstring"_ustr, u"alongerstring"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1),
                             aSorter.compare(u"alongerstring"_ustr, u"alongstring"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"Heading 9"_ustr, u"Heading 10"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"Heading 10"_ustr, u"Heading 9"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1),
                             aSorter.compare(u"July, the 4th"_ustr, u"July, the 10th"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1),
                             aSorter.compare(u"July, the 10th"_ustr, u"July, the 4th"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"abc08"_ustr, u"abc010"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"abc010"_ustr, u"abc08"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+0),
                             aSorter.compare(u"apple10apple"_ustr, u"apple10apple"_ustr));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"KA1"_ustr, u"KA0"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+0), aSorter.compare(u"KA1"_ustr, u"KA1"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"KA1"_ustr, u"KA2"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"KA50"_ustr, u"KA5"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"KA50"_ustr, u"KA100"_ustr));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"1"_ustr, u"0"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+0), aSorter.compare(u"1"_ustr, u"1"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"1"_ustr, u"2"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"11"_ustr, u"1"_ustr));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"50"_ustr, u"100"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"0"_ustr, u"100000"_ustr));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"0"_ustr, u"A"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"A"_ustr, u"0"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"A"_ustr, u"99"_ustr));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"00ABC2"_ustr, u"00ABC1"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"00ABC1"_ustr, u"00ABC2"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare(u"00ABC11"_ustr, u"00ABC2"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare(u"00ABC2"_ustr, u"00ABC11"_ustr));
    }

    CPPUNIT_TEST_SUITE(TestStringNaturalCompare);
    CPPUNIT_TEST(testNatural);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestStringNaturalCompare);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
