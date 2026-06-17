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

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <memory>
#include <string_view>

#include <rtl/digest.h>
#include <rtl/string.h>
#include <rtl/strbuf.hxx>

#include <string.h>

using namespace rtl;

namespace
{

constexpr OString sSampleString               ("This is a sample sentence, which we use to check some crypto functions in sal."_ostr);

const rtlDigestAlgorithm constDigestAlgorithms[] =
{
    rtl_Digest_AlgorithmSHA1_StarOfficeBug,
};

const sal_uInt32 constDigestAlgorithmLengths[] =
{
    RTL_DIGEST_LENGTH_SHA1,
};

const std::string_view constSampleStringSums[] =
{
    std::string_view("2bc5bdb7506a2cdc2fd27fc8b9889343012d5008"),
};

// Create hex-value string from the digest value to keep the string size minimal
OString createHex(const sal_uInt8* pKeyBuffer, sal_uInt32 nKeyLen)
{
    OStringBuffer aBuffer(nKeyLen * 2 + 1);
    for (sal_uInt32 i = 0; i < nKeyLen; ++i)
    {
        sal_Int32 nValue = static_cast<sal_Int32>(pKeyBuffer[i]);
        if (nValue < 16)
            aBuffer.append('0');
        aBuffer.append(nValue, 16);
    }
    return aBuffer.makeStringAndClear();
}

OString getDigest(const OString& aMessage, rtlDigestAlgorithm aAlgorithm)
{
    rtlDigest handle = rtl_digest_create(aAlgorithm);

    const sal_uInt8* pData = reinterpret_cast<const sal_uInt8*>(aMessage.getStr());
    sal_uInt32       nSize = aMessage.getLength();

    rtl_digest_init(handle, pData, nSize);
    rtl_digest_update(handle, pData, nSize);

    sal_uInt32 nKeyLen = rtl_digest_queryLength(handle);
    std::unique_ptr<sal_uInt8[]> pKeyBuffer(new sal_uInt8[nKeyLen]);

    rtl_digest_get(handle, pKeyBuffer.get(), nKeyLen);
    OString aSum = createHex(pKeyBuffer.get(), nKeyLen);

    rtl_digest_destroy( handle );
    return aSum;
}

class DigestTest : public CppUnit::TestFixture
{
public:
    void testCreate()
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(constDigestAlgorithms); i++)
        {
            rtlDigest handle = rtl_digest_create( constDigestAlgorithms[i] );
            CPPUNIT_ASSERT_MESSAGE("create digest", handle != nullptr);
            rtl_digest_destroy( handle );
        }

        rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("create invalid digest", static_cast<rtlDigest>(nullptr), handle);
        rtl_digest_destroy( handle );
    }

    void testQuery()
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(constDigestAlgorithms); i++)
        {
            rtlDigest handle = rtl_digest_create(constDigestAlgorithms[i]);
            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("query handle", aAlgo, constDigestAlgorithms[i]);
            rtl_digest_destroy( handle );
        }

    }

    void testQueryLength()
    {
        rtlDigest handle;
        sal_uInt32 nAlgoLength;

        for (size_t i = 0; i < SAL_N_ELEMENTS(constDigestAlgorithms); i++)
        {
            handle = rtl_digest_create(constDigestAlgorithms[i]);
            nAlgoLength = rtl_digest_queryLength(handle);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("query Length", nAlgoLength, constDigestAlgorithmLengths[i]);
            rtl_digest_destroy( handle );
        }

        handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );
        nAlgoLength = rtl_digest_queryLength(handle);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("query length", static_cast<sal_uInt32>(0), nAlgoLength);
        rtl_digest_destroy( handle );
    }

    void testInit()
    {
        rtlDigestError aError;
        rtlDigest handle;

        handle = nullptr;
        aError = rtl_digest_init(handle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("init(NULL, 0, 0)", rtl_Digest_E_Argument, aError);

        handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1_StarOfficeBug );
        aError = rtl_digest_init(handle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("init(handle, 0, 0)", rtl_Digest_E_None, aError);
        rtl_digest_destroy( handle );

        for (size_t i = 0; i < SAL_N_ELEMENTS(constDigestAlgorithms); i++)
        {
            handle = rtl_digest_create(constDigestAlgorithms[i]);

            OString aMessage = sSampleString;
            const sal_uInt8* pData = reinterpret_cast<const sal_uInt8*>(aMessage.getStr());
            sal_uInt32       nSize = aMessage.getLength();

            aError = rtl_digest_init(handle, pData, nSize);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("init(handle, pData, nSize)", rtl_Digest_E_None, aError);

            rtl_digest_update(handle, pData, nSize);

            sal_uInt32 nKeyLen = rtl_digest_queryLength( handle );
            std::unique_ptr<sal_uInt8[]> pKeyBuffer(new sal_uInt8[nKeyLen]);

            rtl_digest_get( handle, pKeyBuffer.get(), nKeyLen );
            createHex(pKeyBuffer.get(), nKeyLen);

            rtl_digest_destroy( handle );
        }
    }

    void testEqual()
    {
        {
            OString aSum1 = getDigest(sSampleString, rtl_Digest_AlgorithmSHA1_StarOfficeBug );
            OString aSum2 = getDigest(sSampleString, rtl_Digest_AlgorithmSHA1_StarOfficeBug );

            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "digest must have a length", sal_Int32(40), aSum1.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "digest must have a length", sal_Int32(40), aSum2.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("source is the same, dest must be also the same", aSum1, aSum2);
        }

        {
            OString aSum1 = getDigest(sSampleString, rtl_Digest_AlgorithmSHA1_StarOfficeBug );
            OString aSum2 = getDigest("This is a sample sentence. which we use to check some crypto functions in sal."_ostr, rtl_Digest_AlgorithmSHA1_StarOfficeBug);

            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "digest must have a length", sal_Int32(40), aSum1.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "digest must have a length", sal_Int32(40), aSum2.getLength() );
            CPPUNIT_ASSERT_MESSAGE("differ only in one char", aSum1 != aSum2);
        }
    }

    void testCheckSum()
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(constDigestAlgorithms); i++)
        {
            OString aSum = getDigest(sSampleString, constDigestAlgorithms[i]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Checksum of sample string is wrong.", OString(constSampleStringSums[i]), aSum);
        }
    }

    OString runCheckPBKDF2(OString& sPassword, bool bClearSalt, sal_uInt32 nCount)
    {
        sal_uInt32 nKeyLen = RTL_DIGEST_LENGTH_SHA1;
        std::unique_ptr<sal_uInt8[]> pKeyBuffer(new sal_uInt8[nKeyLen]);

        memset(pKeyBuffer.get(), 0, nKeyLen);

        sal_uInt8 const * pPassword = reinterpret_cast<sal_uInt8 const *>(sPassword.getStr());
        sal_Int32  nPasswordLen = sPassword.getLength();

        sal_uInt32   nSaltDataLen = RTL_DIGEST_LENGTH_SHA1;
        std::unique_ptr<sal_uInt8[]> pSaltData(new sal_uInt8[nSaltDataLen]);
        memset(pSaltData.get(), 0, nSaltDataLen);

        if (!bClearSalt)
        {
            // wilful contamination
            pSaltData[0] = 1;
        }

        rtlDigestError aError = rtl_digest_PBKDF2(pKeyBuffer.get(), nKeyLen, pPassword, nPasswordLen, pSaltData.get(), nSaltDataLen, nCount);

        CPPUNIT_ASSERT_EQUAL(rtl_Digest_E_None, aError);

        OString aKey = createHex(pKeyBuffer.get(), nKeyLen);

        // OString sSalt = createHex(pSaltData, nSaltDataLen);
        // printf("Salt: %s\n", sSalt.getStr());

        // CPPUNIT_ASSERT_MESSAGE("md5sum of sample string is wrong. Code changes or sample problems, please check.", aStr.equals(sSampleString_PBKDF2) );
        return aKey;
    }

    void testPBKDF2()
    {
        OString  aPassword = "Password"_ostr;

        // all permutations
        runCheckPBKDF2(aPassword, false, 1);
        runCheckPBKDF2(aPassword, false, 2);
        runCheckPBKDF2(aPassword, true,  1);
        runCheckPBKDF2(aPassword, true,  2);
        runCheckPBKDF2(aPassword, false, 3);
        runCheckPBKDF2(aPassword, false, 4);
        runCheckPBKDF2(aPassword, true,  3);
        runCheckPBKDF2(aPassword, true,  4);
    }

    void testUpdate()
    {
        rtlDigestError aError;
        rtlDigest aHandle;

        aHandle = nullptr;
        aError = rtl_digest_update(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("does not handle wrong parameter", rtl_Digest_E_Argument, aError);

        aHandle = nullptr;
        aError = rtl_digest_updateSHA1(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("does not handle wrong parameter", rtl_Digest_E_Argument, aError);

        aHandle = rtl_digest_create( rtl_Digest_AlgorithmSHA1_StarOfficeBug );
        CPPUNIT_ASSERT(aHandle != nullptr);

        const sal_uInt8* pData = reinterpret_cast<const sal_uInt8*>(sSampleString.getStr());

        aError = rtl_digest_updateSHA1(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'pData' wrong", rtl_Digest_E_Argument, aError);

        aError = rtl_digest_updateSHA1(aHandle, pData, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'nSize' wrong", rtl_Digest_E_None, aError);

        rtl_digest_destroySHA1(aHandle);
    }

    void testGet()
    {
        rtlDigest aHandle;
        rtlDigestError aError;

        aHandle = nullptr;
        aError = rtl_digest_get(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("does not handle wrong parameter", rtl_Digest_E_Argument, aError);

        aHandle = nullptr;
        aError = rtl_digest_getSHA1(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("does not handle wrong parameter", rtl_Digest_E_Argument, aError);

        // test with wrong algorithm
        aHandle = rtl_digest_create(rtl_Digest_AlgorithmInvalid);
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmInvalid", aHandle == nullptr);

        aHandle = rtl_digest_create(rtl_Digest_AlgorithmSHA1_StarOfficeBug);
        CPPUNIT_ASSERT(aHandle != nullptr);

        sal_uInt32 nKeyLen = rtl_digest_queryLength(aHandle);
        std::unique_ptr<sal_uInt8[]> pKeyBuffer(new sal_uInt8[nKeyLen]);

        aError = rtl_digest_getSHA1(aHandle, nullptr, nKeyLen);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'pData' wrong", rtl_Digest_E_Argument, aError);

        aError = rtl_digest_getSHA1(aHandle, pKeyBuffer.get(), 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'nSize' wrong", rtl_Digest_E_BufferSize, aError);

        rtl_digest_destroySHA1(aHandle);
    }

    void testSHA1SumForBiggerInputData()
    {
        // The test data was extracted from oox encrypted document (salt + password).
        // First  case: 16 bytes salt + 34 bytes password (12345678901234567)
        // Second case: 16 bytes salt + 36 bytes password (123456789012345678)
        {
            const unsigned char aData[] = {
                    0x37, 0x5f, 0x47, 0x7a, 0xd2, 0x13, 0xbe, 0xd2, 0x3c, 0x23, 0x33, 0x39,
                    0x68, 0x21, 0x03, 0x6d, 0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00,
                    0x35, 0x00, 0x36, 0x00, 0x37, 0x00, 0x38, 0x00, 0x39, 0x00, 0x30, 0x00,
                    0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00, 0x35, 0x00, 0x36, 0x00,
                    0x37, 0x00
            };

            std::unique_ptr<sal_uInt8[]> pResult(new sal_uInt8[RTL_DIGEST_LENGTH_SHA1]);

            rtl_digest_SHA1_StarOfficeBug(aData, sizeof(aData), pResult.get(), RTL_DIGEST_LENGTH_SHA1);

            OString sKey = createHex(pResult.get(), RTL_DIGEST_LENGTH_SHA1);

            CPPUNIT_ASSERT_EQUAL("06f460d693aecdd3b5cbe8365408eccfc570f32a"_ostr, sKey);
        }

        // tdf#114939, verify that rtl_digest_SHA1_StarOfficeBug computes broken results for certain input (which
        // is not fixed for compatibility reasons):
        {
            sal_uInt8 result[RTL_DIGEST_LENGTH_SHA1];
            rtl_digest_SHA1_StarOfficeBug(
                RTL_CONSTASCII_STRINGPARAM("1012345678901234567890123456789012345678901234567890"),
                result, RTL_DIGEST_LENGTH_SHA1);
            // Rather than correct "9cb1dab34448c1ea460da1f8736869c8852f212f":
            CPPUNIT_ASSERT_EQUAL(
                "90a461ee9cc69cedaeb25c2dc5cc62544ebd5241"_ostr,
                createHex(result, RTL_DIGEST_LENGTH_SHA1));
        }
    }

    CPPUNIT_TEST_SUITE(DigestTest);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testQuery);
    CPPUNIT_TEST(testQueryLength);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testEqual);
    CPPUNIT_TEST(testCheckSum);
    CPPUNIT_TEST(testPBKDF2);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testSHA1SumForBiggerInputData);

    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DigestTest);

} // namespace rtl_digest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
