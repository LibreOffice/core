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

#include <memory>
#include <algorithm>

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/random.h>

#include <string.h>

namespace rtl_random
{

class createPool : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void createPool_001()
    {
        // this is demonstration code

        rtlRandomPool aPool = rtl_random_createPool();

        // LLA: seems to be that another test is not possible for createPool()
        CPPUNIT_ASSERT_MESSAGE("create failed", aPool != nullptr);

        rtl_random_destroyPool(aPool);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(createPool);
    CPPUNIT_TEST(createPool_001);
    CPPUNIT_TEST_SUITE_END();
}; // class createPool

class destroyPool : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    void destroyPool_000()
    {
        // GPF, if failed
        rtl_random_destroyPool(nullptr);
    }

    void destroyPool_001()
    {
        rtlRandomPool aPool = rtl_random_createPool();

        // LLA: seems to be that another test is not possible for createPool()
        CPPUNIT_ASSERT_MESSAGE("create failed", aPool != nullptr);

        rtl_random_destroyPool(aPool);
    }
    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(destroyPool);
    CPPUNIT_TEST(destroyPool_000);
    CPPUNIT_TEST(destroyPool_001);
    CPPUNIT_TEST_SUITE_END();
}; // class destroyPool

class addBytes : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void addBytes_000()
    {
        rtlRandomPool aPool = rtl_random_createPool();

        sal_uInt32  nBufLen = 4;
        std::unique_ptr<sal_uInt8[]>  pBuffer( new sal_uInt8[ nBufLen ] );
        memset(pBuffer.get(), 0, nBufLen);

        rtlRandomError aError = rtl_random_addBytes(nullptr, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong parameter", rtl_Random_E_Argument, aError);

        /* rtlRandomError */ aError = rtl_random_addBytes(aPool, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong parameter", rtl_Random_E_Argument, aError);

        /* rtlRandomError */ aError = rtl_random_addBytes(aPool, pBuffer.get(), nBufLen);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong parameter", rtl_Random_E_None, aError);

        rtl_random_destroyPool(aPool);
    }

    void addBytes_001()
        {
            rtlRandomPool aPool = rtl_random_createPool();

            sal_uInt32  nBufLen = 4;
            std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nBufLen ] );

            memset(pBuffer.get(), 0, nBufLen);

            rtl_random_addBytes(aPool, pBuffer.get(), nBufLen);

            printf("%2x %2x %2x %2x\n", pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]);

            rtl_random_destroyPool(aPool);
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(addBytes);
    CPPUNIT_TEST(addBytes_000);
    CPPUNIT_TEST(addBytes_001);
    CPPUNIT_TEST_SUITE_END();
}; // class addBytes

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
        rtlRandomPool aPool = rtl_random_createPool();

        sal_uInt32  nBufLen = 4;
        std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nBufLen ] );
        memset(pBuffer.get(), 0, nBufLen);

        rtlRandomError aError = rtl_random_getBytes(nullptr, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong parameter", rtl_Random_E_Argument, aError);

        /* rtlRandomError */ aError = rtl_random_getBytes(aPool, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong parameter", rtl_Random_E_Argument, aError);

        /* rtlRandomError */ aError = rtl_random_getBytes(aPool, pBuffer.get(), nBufLen);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong parameter", rtl_Random_E_None, aError);

        rtl_random_destroyPool(aPool);
    }

    void getBytes_001()
    {
        rtlRandomPool aPool = rtl_random_createPool();

        sal_uInt32  nBufLen = 4;
        std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nBufLen ] );
        memset(pBuffer.get(), 0, nBufLen);

        rtlRandomError aError = rtl_random_getBytes(aPool, pBuffer.get(), nBufLen);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong parameter", rtl_Random_E_None, aError);

        printf("%2x %2x %2x %2x\n", pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]);

        rtl_random_destroyPool(aPool);
    }

    void getBytes_002()
    {
        rtlRandomPool aPool = rtl_random_createPool();

        sal_uInt32  nBufLen = 4;
        std::unique_ptr<sal_uInt8[]>  pBuffer( new sal_uInt8[ nBufLen << 1 ] );
        memset(pBuffer.get(), 0, nBufLen << 1);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("memset failed", sal_uInt8(0), pBuffer[4]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("memset failed", sal_uInt8(0), pBuffer[5]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("memset failed", sal_uInt8(0), pBuffer[6]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("memset failed", sal_uInt8(0), pBuffer[7]);

        rtlRandomError aError = rtl_random_getBytes(aPool, pBuffer.get(), nBufLen);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong parameter", rtl_Random_E_None, aError);

        printf("%2x %2x %2x %2x %2x %2x %2x %2x\n", pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3], pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7]);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("internal memory overwrite", sal_uInt8(0), pBuffer[4]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("internal memory overwrite", sal_uInt8(0), pBuffer[5]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("internal memory overwrite", sal_uInt8(0), pBuffer[6]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("internal memory overwrite", sal_uInt8(0), pBuffer[7]);

        rtl_random_destroyPool(aPool);
    }

    void getBytes_003()
    {
        rtlRandomPool aPool = rtl_random_createPool();

        sal_uInt32  nBufLen = 1;
        std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nBufLen ] );
        memset(pBuffer.get(), 0, nBufLen);

        Statistics aStat;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("memset failed", static_cast<sal_uInt8>(0), pBuffer[0]);

        int nCount = 0;

        int nCountMax = 1000000;
        for(nCount = 0;nCount < nCountMax; ++nCount)                  // run 100000000 through getBytes(...)
        {
            /* rtlRandomError aError = */ rtl_random_getBytes(aPool, pBuffer.get(), nBufLen);
            /* CPPUNIT_ASSERT_MESSAGE("wrong parameter", aError == rtl_Random_E_None); */

            aStat.addValue(pBuffer[0], 1);
        }

        aStat.build(nCountMax);
        aStat.print();

        CPPUNIT_ASSERT_MESSAGE("deviation should be less average", aStat.getMaxDeviation() < aStat.getAverage());

        rtl_random_destroyPool(aPool);
    }

    void getBytes_003_1()
    {
        rtlRandomPool aPool = rtl_random_createPool();

        sal_uInt32  nBufLen = 256;
        std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nBufLen ] );
        memset(pBuffer.get(), 0, nBufLen);

        Statistics aStat;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("memset failed", static_cast<sal_uInt8>(0), pBuffer[0]);

        int nCount = 0;

        int nCountMax = 10000;
        for(nCount = 0;nCount < nCountMax; ++nCount)                  // run 100000000 through getBytes(...)
        {
            /* rtlRandomError aError = */ rtl_random_getBytes(aPool, pBuffer.get(), nBufLen);
            // CPPUNIT_ASSERT_MESSAGE("wrong parameter", aError == rtl_Random_E_None);

            for (sal_uInt32 i=0;i<nBufLen;++i)
                aStat.addValue(pBuffer[i], 1);
        }

        aStat.build(nCountMax * nBufLen);
        aStat.print();

        CPPUNIT_ASSERT_MESSAGE("deviation should be less average", aStat.getMaxDeviation() < aStat.getAverage());

        rtl_random_destroyPool(aPool);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(getBytes);
    CPPUNIT_TEST(getBytes_000);
    CPPUNIT_TEST(getBytes_001);
    CPPUNIT_TEST(getBytes_002);
    CPPUNIT_TEST(getBytes_003);
    CPPUNIT_TEST(getBytes_003_1);
    CPPUNIT_TEST_SUITE_END();
}; // class getBytes

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_random::createPool);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_random::destroyPool);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_random::addBytes);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_random::getBytes);
} // namespace rtl_random

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
