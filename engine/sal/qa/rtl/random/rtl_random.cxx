/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <memory>
#include <algorithm>

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/random.h>

#include <string.h>

namespace rtl_random
{

namespace {

class Statistics
{
    int m_nDispensation[256];

    int m_nMin;
    int m_nMax;
    int m_nAverage;
    int m_nMinDeviation;
    int m_nMaxDeviation;

public:
    void clearDispensation()
    {
        for (int i = 0;i < 256;++i)                        // clear array
        {
            m_nDispensation[i] = 0;
        }
    }
    Statistics()
    : m_nMin(0)
    , m_nMax(0)
    , m_nAverage(0)
    , m_nMinDeviation(0)
    , m_nMaxDeviation(0)
    {
        clearDispensation();
    }

    void addValue(sal_uInt8 _nIndex, sal_Int32 _nValue)
    {
        m_nDispensation[_nIndex] += _nValue;
    }

    void build(sal_Int32 _nCountMax)
    {
        m_nMin = _nCountMax;
        m_nMax = 0;

        m_nAverage = _nCountMax / 256;

        m_nMinDeviation = _nCountMax;
        m_nMaxDeviation = 0;

        for (int i = 0;i < 256;++i)                        // show dispensation
        {
            m_nMin = std::min(m_nMin, m_nDispensation[i]);
            m_nMax = std::max(m_nMax, m_nDispensation[i]);

            m_nMinDeviation = std::min(m_nMinDeviation, abs(m_nAverage - m_nDispensation[i]));
            m_nMaxDeviation = std::max(m_nMaxDeviation, abs(m_nAverage - m_nDispensation[i]));
        }
    }

    void print()
    {
        // LLA: these are only info values
        printf("\nSome statistics\n");
        printf("Min: %d\n", m_nMin);
        printf("Max: %d\n", m_nMax);
        printf("Average: %d\n", m_nAverage);
        printf("Min abs deviation: %d\n", m_nMinDeviation);
        printf("Max abs deviation: %d\n", m_nMaxDeviation);
    }

    sal_Int32 getAverage() const {return m_nAverage;}
    sal_Int32 getMaxDeviation() const {return m_nMaxDeviation;}

};

}

class getBytes : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    void getBytes_000()
    {
        sal_uInt32  nBufLen = 4;
        std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nBufLen ] );
        memset(pBuffer.get(), 0, nBufLen);

        rtl_random_getBytes(pBuffer.get(), nBufLen);
    }

    void getBytes_003()
    {
        sal_uInt32  nBufLen = 1;
        std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nBufLen ] );
        memset(pBuffer.get(), 0, nBufLen);

        Statistics aStat;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("memset failed", static_cast<sal_uInt8>(0), pBuffer[0]);

        int nCount = 0;

        int nCountMax = 1000000;
        for(nCount = 0;nCount < nCountMax; ++nCount)                  // run 100000000 through getBytes(...)
        {
            rtl_random_getBytes(pBuffer.get(), nBufLen);

            aStat.addValue(pBuffer[0], 1);
        }

        aStat.build(nCountMax);
        aStat.print();

        CPPUNIT_ASSERT_MESSAGE("deviation should be less average", aStat.getMaxDeviation() < aStat.getAverage());
    }

    void getBytes_003_1()
    {
        sal_uInt32  nBufLen = 256;
        std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nBufLen ] );
        memset(pBuffer.get(), 0, nBufLen);

        Statistics aStat;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("memset failed", static_cast<sal_uInt8>(0), pBuffer[0]);

        int nCount = 0;

        int nCountMax = 10000;
        for(nCount = 0;nCount < nCountMax; ++nCount)                  // run 100000000 through getBytes(...)
        {
            rtl_random_getBytes(pBuffer.get(), nBufLen);

            for (sal_uInt32 i=0;i<nBufLen;++i)
                aStat.addValue(pBuffer[i], 1);
        }

        aStat.build(nCountMax * nBufLen);
        aStat.print();

        CPPUNIT_ASSERT_MESSAGE("deviation should be less average", aStat.getMaxDeviation() < aStat.getAverage());
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(getBytes);
    CPPUNIT_TEST(getBytes_000);
    CPPUNIT_TEST(getBytes_003);
    CPPUNIT_TEST(getBytes_003_1);
    CPPUNIT_TEST_SUITE_END();
}; // class getBytes

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_random::getBytes);
} // namespace rtl_random

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
