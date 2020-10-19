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

namespace tools
{
class MinMaxTest : public CppUnit::TestFixture
{
public:
    void testSignedMinMax()
    {
        sal_Int32 nSignedVal = -10;
        tools::Long nMin = 1;
        tools::Long nMax = 10;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(nMin),
                             static_cast<sal_Int32>(MinMax(nSignedVal, nMin, nMax)));

        nSignedVal = -10;
        nMin = -15;
        nMax = 10;
        CPPUNIT_ASSERT_EQUAL(nSignedVal, static_cast<sal_Int32>(MinMax(nSignedVal, nMin, nMax)));

        nSignedVal = -15;
        nMin = -15;
        nMax = 10;
        CPPUNIT_ASSERT_EQUAL(nSignedVal, static_cast<sal_Int32>(MinMax(nSignedVal, nMin, nMax)));
    }

    void testUnsignedMinMax()
    {
        sal_uInt32 nUnsignedVal = 5;
        tools::Long nMin = 1;
        tools::Long nMax = 10;
        CPPUNIT_ASSERT_EQUAL(nUnsignedVal,
                             static_cast<sal_uInt32>(MinMax(nUnsignedVal, nMin, nMax)));

        nUnsignedVal = 5;
        nMin = 1;
        nMax = 5;
        CPPUNIT_ASSERT_EQUAL(nUnsignedVal,
                             static_cast<sal_uInt32>(MinMax(nUnsignedVal, nMin, nMax)));

        nUnsignedVal = 5;
        nMin = 5;
        nMax = 5;
        CPPUNIT_ASSERT_EQUAL(nUnsignedVal,
                             static_cast<sal_uInt32>(MinMax(nUnsignedVal, nMin, nMax)));

        nUnsignedVal = 10;
        nMin = -20;
        nMax = 15;
        CPPUNIT_ASSERT_EQUAL(nUnsignedVal,
                             static_cast<sal_uInt32>(MinMax(nUnsignedVal, nMin, nMax)));

        nUnsignedVal = 20;
        nMin = 10;
        nMax = 15;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(nMax),
                             static_cast<sal_uInt32>(MinMax(nUnsignedVal, nMin, nMax)));

        nUnsignedVal = 5;
        nMin = 10;
        nMax = 15; // irrelevant, but cannot be less than nMin
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(nMin),
                             static_cast<sal_uInt32>(MinMax(nUnsignedVal, nMin, nMax)));
    }

    CPPUNIT_TEST_SUITE(MinMaxTest);
    CPPUNIT_TEST(testSignedMinMax);
    CPPUNIT_TEST(testUnsignedMinMax);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MinMaxTest);
} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
