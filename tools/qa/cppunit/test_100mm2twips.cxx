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

#include <tools/helpers.hxx>
#include <tools/UnitConversion.hxx>

class UnitConversionTest : public CppUnit::TestFixture
{
public:
    void testSanitiseMm100ToTwip()
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int64(145), sanitiseMm100ToTwip(255));
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-145), sanitiseMm100ToTwip(-255));
    }

    void testConvertMm100ToTwip()
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int64(145), convertMm100ToTwip(255));
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-145), convertMm100ToTwip(-255));
    }

    void testConvertPointToMm100()
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(599.72, convertPointToMm100(17.0), 1E-2);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(600), convertPointToMm100(sal_Int64(17)));

        CPPUNIT_ASSERT_DOUBLES_EQUAL(22930.55, convertPointToMm100(650.0), 1E-2);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(22931), convertPointToMm100(sal_Int64(650)));
    }

    CPPUNIT_TEST_SUITE(UnitConversionTest);
    CPPUNIT_TEST(testSanitiseMm100ToTwip);
    CPPUNIT_TEST(testConvertMm100ToTwip);
    CPPUNIT_TEST(testConvertPointToMm100);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UnitConversionTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
