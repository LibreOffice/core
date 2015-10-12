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
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/crc.h>

namespace rtl_CRC32
{

class test : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp() override
    {
    }

    void tearDown() override
    {
    }

    // insert your test code here.
    void rtl_crc32_001()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        sal_uInt32 nCRC = 0;

        char buf[] = {0};
        int num = 0;

        nCRC = rtl_crc32(nCRC, buf, num);

        CPPUNIT_ASSERT_MESSAGE("empty crc buffer", nCRC == 0);
    }

    void rtl_crc32_002()
    {
        sal_uInt32 nCRC = 0;

        char buf[] = {0,0};
        int num = sizeof(buf);

        nCRC = rtl_crc32(nCRC, buf, num);

        CPPUNIT_ASSERT_MESSAGE("buffer contain 2 empty bytes, crc is zero", nCRC != 0);
    }

    void rtl_crc32_002_1()
    {
        sal_uInt32 nCRC = 0;

        char buf[] = {0,0,0};
        int num = sizeof(buf);

        nCRC = rtl_crc32(nCRC, buf, num);

        CPPUNIT_ASSERT_MESSAGE("buffer contain 3 empty bytes, crc is zero", nCRC != 0);
    }

    /**
     * crc32 check:
     * Build checksum on two buffers with same size but different content,
     * the result (crc32 checksum) must differ
     */

    void rtl_crc32_003()
    {
        sal_uInt32 nCRC1 = 0;
        char buf1[] = {2};
        int num1 = sizeof(buf1);

        nCRC1 = rtl_crc32(nCRC1, buf1, num1);

        sal_uInt32 nCRC2 = 0;
        char buf2[] = {3};
        int num2 = sizeof(buf2);

        nCRC2 = rtl_crc32(nCRC2, buf2, num2);

        CPPUNIT_ASSERT_MESSAGE("checksum should differ for buf1 and buf2", nCRC1 != nCRC2);
    }

    /** check if the crc32 only use as much values, as given
     *
     */
    void rtl_crc32_003_1()
    {
        sal_uInt32 nCRC1 = 0;
        char buf1[] = {2,1};
        int num1 = sizeof(buf1) - 1;

        nCRC1 = rtl_crc32(nCRC1, buf1, num1);

        sal_uInt32 nCRC2 = 0;
        char buf2[] = {2,2};
        int num2 = sizeof(buf2) - 1;

        nCRC2 = rtl_crc32(nCRC2, buf2, num2);

        CPPUNIT_ASSERT_MESSAGE("checksum leave it's bounds", nCRC1 == nCRC2);
    }

    /** check if the crc32 differ at same content in reverse order
     *
     */
    void rtl_crc32_003_2()
    {
        sal_uInt32 nCRC1 = 0;
        char buf1[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
        int num1 = sizeof(buf1);

        nCRC1 = rtl_crc32(nCRC1, buf1, num1);

        sal_uInt32 nCRC2 = 0;
        char buf2[] = {20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
        int num2 = sizeof(buf2);

        nCRC2 = rtl_crc32(nCRC2, buf2, num2);

        CPPUNIT_ASSERT_MESSAGE("checksum should differ", nCRC1 != nCRC2);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(test);
    CPPUNIT_TEST(rtl_crc32_001);
    CPPUNIT_TEST(rtl_crc32_002);
    CPPUNIT_TEST(rtl_crc32_002_1);
    CPPUNIT_TEST(rtl_crc32_003);
    CPPUNIT_TEST(rtl_crc32_003_1);
    CPPUNIT_TEST(rtl_crc32_003_2);
    CPPUNIT_TEST_SUITE_END();
}; // class test

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_CRC32::test);
} // namespace rtl_CRC32

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
