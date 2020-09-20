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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <memory>
#include <string_view>

#include <rtl/digest.h>
#include <rtl/string.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <string.h>

using namespace rtl;

namespace
{

constexpr OStringLiteral sSampleString               ("This is a sample sentence, which we use to check some crypto functions in sal.");
constexpr OStringLiteral sSampleString_only_one_diff ("This is a sample sentence. which we use to check some crypto functions in sal.");

const rtlDigestAlgorithm constDigestAlgorithms[] =
{
    rtl_Digest_AlgorithmMD2,
    rtl_Digest_AlgorithmMD5,
    rtl_Digest_AlgorithmSHA,
    rtl_Digest_AlgorithmSHA1,
    rtl_Digest_AlgorithmHMAC_MD5,
    rtl_Digest_AlgorithmHMAC_SHA1,
};

const sal_uInt32 constDigestAlgorithmLengths[] =
{
    RTL_DIGEST_LENGTH_MD2,
    RTL_DIGEST_LENGTH_MD5,
    RTL_DIGEST_LENGTH_SHA,
    RTL_DIGEST_LENGTH_SHA1,
    RTL_DIGEST_LENGTH_HMAC_MD5,
    RTL_DIGEST_LENGTH_HMAC_SHA1,
};

const std::string_view constSampleStringSums[] =
{
    std::string_view("647ee6c9d4aa5fdd374ed9d7a156acbf"),
    std::string_view("b16b903e6fc0b62ae389013ed93fe531"),
    std::string_view("eab2814429b2613301c8a077b806af3680548914"),
    std::string_view("2bc5bdb7506a2cdc2fd27fc8b9889343012d5008"),
    std::string_view("0b1b0e1a6f2e4420326354b031063605"),
    std::string_view("1998c6a556915be76451bfb587fa7c34d849936e")
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

        handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
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
            OString aMsg1 = sSampleString;
            OString aMsg2 = sSampleString;

            OString aSum1 = getDigest(aMsg1, rtl_Digest_AlgorithmMD5);
            OString aSum2 = getDigest(aMsg2, rtl_Digest_AlgorithmMD5);

            CPPUNIT_ASSERT_MESSAGE("md5sum must have a length", aSum1.getLength() == 32 && aSum2.getLength() == 32 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("source is the same, dest must be also the same", aSum1, aSum2);
        }

        {
            OString aMsg1 = sSampleString;
            OString aMsg2 = sSampleString_only_one_diff;

            OString aSum1 = getDigest(aMsg1, rtl_Digest_AlgorithmMD5);
            OString aSum2 = getDigest(aMsg2, rtl_Digest_AlgorithmMD5);

            CPPUNIT_ASSERT_MESSAGE("md5sum must have a length", aSum1.getLength() == 32 && aSum2.getLength() == 32 );
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
        sal_uInt32 nKeyLen = RTL_DIGEST_LENGTH_HMAC_SHA1;
        std::unique_ptr<sal_uInt8[]> pKeyBuffer(new sal_uInt8[nKeyLen]);

        memset(pKeyBuffer.get(), 0, nKeyLen);

        sal_uInt8 const * pPassword = reinterpret_cast<sal_uInt8 const *>(sPassword.getStr());
        sal_Int32  nPasswordLen = sPassword.getLength();

        sal_uInt32   nSaltDataLen = RTL_DIGEST_LENGTH_HMAC_SHA1;
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
        OString  aPassword = "Password";

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
        aError = rtl_digest_updateMD2(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("does not handle wrong parameter", rtl_Digest_E_Argument, aError);

        aError = rtl_digest_updateMD5(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("does not handle wrong parameter", rtl_Digest_E_Argument, aError);

        aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != nullptr);

        const sal_uInt8* pData = reinterpret_cast<const sal_uInt8*>(sSampleString.getStr());
        sal_uInt32       nSize = sSampleString.getLength();

        aError = rtl_digest_updateMD2(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'pData' wrong", rtl_Digest_E_Argument, aError);

        aError = rtl_digest_updateMD2(aHandle, pData, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'nSize' wrong", rtl_Digest_E_None, aError);

        rtl_digest_destroyMD2(aHandle);

        // use wrong Algorithm!!! This is volitional!
        aHandle = rtl_digest_create(rtl_Digest_AlgorithmMD2);
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != nullptr);

        aError = rtl_digest_updateMD5(aHandle, pData, nSize);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'handle' wrong", rtl_Digest_E_Algorithm, aError);

        rtl_digest_destroyMD5(aHandle);

        aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD5", aHandle != nullptr);

        aError = rtl_digest_updateMD5(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'pData' wrong", rtl_Digest_E_Argument, aError);

        aError = rtl_digest_updateMD5(aHandle, pData, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'nSize' wrong", rtl_Digest_E_None, aError);

        rtl_digest_destroyMD5(aHandle);
    }

    void testGet()
    {
        rtlDigest aHandle;
        rtlDigestError aError;

        aHandle = nullptr;
        aError = rtl_digest_get(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("does not handle wrong parameter", rtl_Digest_E_Argument, aError);

        aHandle = nullptr;
        aError = rtl_digest_getMD5(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("does not handle wrong parameter", rtl_Digest_E_Argument, aError);

        // test with wrong algorithm
        aHandle = rtl_digest_create(rtl_Digest_AlgorithmMD2);
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != nullptr);

        sal_uInt32 nKeyLen = rtl_digest_queryLength(aHandle);
        std::unique_ptr<sal_uInt8[]> pKeyBuffer(new sal_uInt8[nKeyLen]);

        aError = rtl_digest_getMD5(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle 2. parameter wrong", rtl_Digest_E_Argument, aError);

        aError = rtl_digest_getMD5(aHandle, pKeyBuffer.get(), 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'handle' wrong", rtl_Digest_E_Algorithm, aError);

        rtl_digest_destroyMD2(aHandle);

        aHandle = rtl_digest_create(rtl_Digest_AlgorithmMD5);
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD5", aHandle != nullptr);

        aError = rtl_digest_getMD5(aHandle, nullptr, nKeyLen);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'pData' wrong", rtl_Digest_E_Argument, aError);

        aError = rtl_digest_getMD5(aHandle, pKeyBuffer.get(), 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("handle parameter 'nSize' wrong", rtl_Digest_E_BufferSize, aError);

        rtl_digest_destroyMD5(aHandle);
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

            OString sExpected = "06f460d693aecdd3b5cbe8365408eccfc570f32a";

            rtl_digest_SHA1(aData, sizeof(aData), pResult.get(), RTL_DIGEST_LENGTH_SHA1);

            OString sKey = createHex(pResult.get(), RTL_DIGEST_LENGTH_SHA1);

            CPPUNIT_ASSERT_EQUAL(sExpected, sKey);
        }

        // tdf#114939, verify that rtl_digest_SHA1 computes broken results for certain input (which
        // is not fixed for compatibility reasons):
        {
            sal_uInt8 result[RTL_DIGEST_LENGTH_SHA1];
            rtl_digest_SHA1(
                RTL_CONSTASCII_STRINGPARAM("1012345678901234567890123456789012345678901234567890"),
                result, RTL_DIGEST_LENGTH_SHA1);
            // Rather than correct "9cb1dab34448c1ea460da1f8736869c8852f212f":
            CPPUNIT_ASSERT_EQUAL(
                OString("90a461ee9cc69cedaeb25c2dc5cc62544ebd5241"),
                createHex(result, RTL_DIGEST_LENGTH_SHA1));
        }
    }

    void testMD5()
    {
        unsigned char const data[] = {
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
        };
        OString const expected[] = {
            "d41d8cd98f00b204e9800998ecf8427e",
            "cfcd208495d565ef66e7dff9f98764da",
            "b4b147bc522828731f1a016bfa72c073",
            "c6f057b86584942e415435ffb1fa93d4",
            "4a7d1ed414474e4033ac29ccb8653d9b",
            "dcddb75469b4b4875094e14561e573d8",
            "670b14728ad9902aecba32e22fa4f6bd",
            "29c3eea3f305d6b823f562ac4be35217",
            "dd4b21e9ef71e1291183a46b913ae6f2",
            "4c93008615c2d041e33ebac605d14b5b",
            "f1b708bba17f1ce948dc979f4d7092bc",
            "645a8aca5a5b84527c57ee2f153f1946",
            "35b9ab5a36f3234dd26db357fd4a0dc1",
            "4aad0d9ff11812ebdd5e376fdbef6222",
            "c47532bbb1e2883c902071591ae1ec9b",
            "5284047f4ffb4e04824a2fd1d1f0cd62",
            "1e4a1b03d1b6cd8a174a826f76e009f4",
            "0e7b9f29a828b6f953b482fc299e536b",
            "3ea032bf79e8c116b05f4698d5a8e044",
            "15f47c8a3e5e9685307dd65a653b8dc0",
            "cc545187d0745132de1e9941db0ef6ce",
            "0585e303e79acd837c3a3e2a2bec8b18",
            "b28ccfdee4b9f39ba18b58a4f61a03d1",
            "d018229b1183c926c10ea688350afec8",
            "660719b4a7591769583a7c8d20c6dfa4",
            "1e2432adacf481836265fcc62ee8f3e3",
            "6e88e2af74c1d9d7d7d652b90d03751e",
            "780ca685003cec1d617beaa6f346e1be",
            "7f2e1dcfd6e2a3f5c38f31e640136ff6",
            "1a3dee46117aeb8010cf365b8653faa8",
            "1d0064395af3c745f6c3194e92373d7a",
            "b52582043219f2deb2d3c9cb05d6448a",
            "cd9e459ea708a948d5c2f5a6ca8838cf",
            "00de800ecd7a4fb2813986c987e46d51",
            "15336d4b38561a82bd24c9398b781aed",
            "5fe699d3c461ab5a795505f59d5adf15",
            "c5e0eb03cbb4bea95ce3f8f48fca77d5",
            "355c1410373ef02fff2b03844d72c7d4",
            "02df97da8207de2b3afa69c151ca8958",
            "82c66dbf3e73f87ffc9564b2098d6a4f",
            "b373e3ddc3438d7c10c76f3ad9d4c401",
            "fac901a4a3dbc4461541731a33a31d15",
            "f573e011b414bf3f9dd284f7dad29592",
            "11694570cc5dda099669f2ba3660a70d",
            "60997cc8aef7fedd9995e6b3ca89ce26",
            "63c5fcf83c2275fe64e880dd8dfc5cd6",
            "c7a0a100057ebbfc63ee169562026aea",
            "42c2dec247919384edece38033458627",
            "b505acf9fc996902b0c547a2abfc62b2",
            "2fa7a1321d6b5fa0e04ad46785f574f3",
            "86d2bfc0bab44eecf21e1432be7b3efc",
            "7ca318f12a0955a3e637dc5645a2f96e",
            "3eda02765b8fb8bb9b20c735f4537827",
            "26dead12262c9a5c115b01e0a3c805b6",
            "978b0444e93c5f7d714575f28a77dca1",
            "d7fe636bd28e2ee2ba4d6c5898318699",
            "ce992c2ad906967c63c3f9ab0c2294a9",
            "1f3b814e9d417e9fd8750299982feb1f",
            "1a2f42174eaa78ce6a67d75e98a59cb6",
            "17c772c45c9a09f6e56b7228ddd161a7",
            "5b19445b70b493c78f3bc06eb7962315",
            "e590c24cc612bdedd522dfe23bb29b42",
            "4d78c699a0167bc0cfce8a5c5a715c0e",
            "5703db92acb9d45e3975822c9206453f",
            "10eab6008d5642cf42abd2aa41f847cb",
        };
        rtlDigest digest = rtl_digest_createMD5();
        for (size_t i = 0; i < sizeof(data); ++i)
        {
            rtl_digest_updateMD5(digest, &data, i);
            sal_uInt8 buf[RTL_DIGEST_LENGTH_MD5];
            rtl_digest_getMD5(digest, &buf[0], sizeof(buf));
            OString const sResult = createHex(&buf[0], sizeof(buf));
            CPPUNIT_ASSERT_EQUAL(expected[i], sResult);
        }
        rtl_digest_destroyMD5(digest);
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
    CPPUNIT_TEST(testMD5);

    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DigestTest);

} // namespace rtl_digest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
