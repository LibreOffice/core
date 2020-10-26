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

namespace tools
{
class BigIntTest : public CppUnit::TestFixture
{
public:
    void testConstructionFromLongLong();

    CPPUNIT_TEST_SUITE(BigIntTest);
    CPPUNIT_TEST(testConstructionFromLongLong);
    CPPUNIT_TEST_SUITE_END();
};

void BigIntTest::testConstructionFromLongLong()
{
    // small positive number
    {
        BigInt bi(static_cast<sal_Int64>(42));
        CPPUNIT_ASSERT(bi.IsSet());
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(!bi.IsNeg());
        CPPUNIT_ASSERT(bi.IsLong());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(42), static_cast<sal_Int32>(bi));
    }

    // small negative number
    {
        BigInt bi(static_cast<sal_Int64>(-42));
        CPPUNIT_ASSERT(bi.IsSet());
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(bi.IsNeg());
        CPPUNIT_ASSERT(bi.IsLong());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-42), static_cast<sal_Int32>(bi));
    }

#if SAL_TYPES_SIZEOFLONG < SAL_TYPES_SIZEOFLONGLONG
    // positive number just fitting to long
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<tools::Long>::max()));
        CPPUNIT_ASSERT(bi.IsSet());
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(!bi.IsNeg());
        CPPUNIT_ASSERT(bi.IsLong());
        CPPUNIT_ASSERT_EQUAL(std::numeric_limits<tools::Long>::max(), static_cast<tools::Long>(bi));
    }

    // negative number just fitting to long
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<tools::Long>::min()));
        CPPUNIT_ASSERT(bi.IsSet());
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(bi.IsNeg());
        CPPUNIT_ASSERT(bi.IsLong());
        CPPUNIT_ASSERT_EQUAL(std::numeric_limits<tools::Long>::min(), static_cast<tools::Long>(bi));
    }

    // positive number not fitting to long
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<tools::Long>::max())
                  + static_cast<sal_Int64>(1));
        CPPUNIT_ASSERT(bi.IsSet());
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(!bi.IsNeg());
        CPPUNIT_ASSERT(!bi.IsLong());
    }

    // negative number not fitting to long
    {
        BigInt bi(static_cast<sal_Int64>(std::numeric_limits<tools::Long>::min())
                  - static_cast<sal_Int64>(1));
        CPPUNIT_ASSERT(bi.IsSet());
        CPPUNIT_ASSERT(!bi.IsZero());
        CPPUNIT_ASSERT(bi.IsNeg());
        CPPUNIT_ASSERT(!bi.IsLong());
    }
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(BigIntTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
