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

        CPPUNIT_ASSERT_EQUAL(sal_Int32(+0), aSorter.compare("ABC", "ABC"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("ABC", "abc"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("abc", "ABC"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("alongstring", "alongerstring"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("alongerstring", "alongstring"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("Heading 9", "Heading 10"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("Heading 10", "Heading 9"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("July, the 4th", "July, the 10th"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("July, the 10th", "July, the 4th"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("abc08", "abc010"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("abc010", "abc08"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+0), aSorter.compare("apple10apple", "apple10apple"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("KA1", "KA0"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+0), aSorter.compare("KA1", "KA1"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("KA1", "KA2"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("KA50", "KA5"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("KA50", "KA100"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("1", "0"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+0), aSorter.compare("1", "1"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("1", "2"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("11", "1"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("50", "100"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("0", "100000"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("0", "A"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("A", "0"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("A", "99"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("00ABC2", "00ABC1"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("00ABC1", "00ABC2"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(+1), aSorter.compare("00ABC11", "00ABC2"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aSorter.compare("00ABC2", "00ABC11"));
    }

    CPPUNIT_TEST_SUITE(TestStringNaturalCompare);
    CPPUNIT_TEST(testNatural);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestStringNaturalCompare);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
