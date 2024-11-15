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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tools/bigint.hxx>

#include <limits>

namespace tools
{
class BigIntTest : public CppUnit::TestFixture
{
public:
    void testConstructionFromLongLong();
    void testLenB1();

    CPPUNIT_TEST_SUITE(BigIntTest);
    CPPUNIT_TEST(testConstructionFromLongLong);
    CPPUNIT_TEST(testLenB1);
    CPPUNIT_TEST_SUITE_END();
};

void BigIntTest::testConstructionFromLongLong()
{
    // small positive number
    {
        BigInt bi(static_cast<sal_Int64>(42));
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(!bi.IsNeg());
        CPPUNIT_ASSERT(!bi.IsBig());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(42), static_cast<sal_Int32>(bi));
    }

    // small negative number
    {
        BigInt bi(static_cast<sal_Int64>(-42));
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(bi.IsNeg());
        CPPUNIT_ASSERT(!bi.IsBig());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-42), static_cast<sal_Int32>(bi));
    }

    // positive number just fitting to sal_Int32
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<sal_Int32>::max()));
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(!bi.IsNeg());
        CPPUNIT_ASSERT(!bi.IsBig());
        CPPUNIT_ASSERT_EQUAL(std::numeric_limits<sal_Int32>::max(), static_cast<sal_Int32>(bi));
    }

    // negative number just fitting to sal_Int32
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<sal_Int32>::min()));
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(bi.IsNeg());
        CPPUNIT_ASSERT(!bi.IsBig());
        CPPUNIT_ASSERT_EQUAL(std::numeric_limits<sal_Int32>::min(), static_cast<sal_Int32>(bi));
    }

    // positive number not fitting to sal_Int32
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<sal_Int32>::max()) + 1);
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(!bi.IsNeg());
        CPPUNIT_ASSERT(bi.IsBig());
    }

    // negative number not fitting to sal_Int32
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<sal_Int32>::min()) - 1);
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(bi.IsNeg());
        CPPUNIT_ASSERT(bi.IsBig());
    }

    // max possible sal_Int64 negative number
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<sal_Int64>::min()));
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(bi.IsNeg());
        CPPUNIT_ASSERT(bi.IsBig());
        CPPUNIT_ASSERT_EQUAL(std::numeric_limits<sal_Int64>::min(), static_cast<sal_Int64>(bi));
    }

    // max possible sal_Int64 positive number
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<sal_Int64>::max()));
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(!bi.IsNeg());
        CPPUNIT_ASSERT(bi.IsBig());
        CPPUNIT_ASSERT_EQUAL(std::numeric_limits<sal_Int64>::max(), static_cast<sal_Int64>(bi));
    }
}

void BigIntTest::testLenB1()
{
    BigInt dy(2634022912);
    sal_Int64 md(-4177526784);
    sal_Int64 mn(2634022912);
    dy *= md;
    dy -= (mn - 1) / 2;
    dy /= mn;

    CPPUNIT_ASSERT_EQUAL(sal_Int64(-4177526784), sal_Int64(dy));
}

CPPUNIT_TEST_SUITE_REGISTRATION(BigIntTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
