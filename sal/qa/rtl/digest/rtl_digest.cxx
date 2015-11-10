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

#include <rtl/digest.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <string.h>

using namespace rtl;

namespace
{

const OString sSampleString               = "This is a sample sentence, which we use to check some crypto functions in sal.";
const OString sSampleString_only_one_diff = "This is a sample sentence. which we use to check some crypto functions in sal.";

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

const OString constSampleStringSums[] =
{
    "647ee6c9d4aa5fdd374ed9d7a156acbf",
    "b16b903e6fc0b62ae389013ed93fe531",
    "eab2814429b2613301c8a077b806af3680548914",
    "2bc5bdb7506a2cdc2fd27fc8b9889343012d5008",
    "0b1b0e1a6f2e4420326354b031063605",
    "1998c6a556915be76451bfb587fa7c34d849936e"
};

// Create hex-value string from the digest value to keep the string size minimal
OString createHex(sal_uInt8* pKeyBuffer, sal_uInt32 nKeyLen)
{
    OStringBuffer aBuffer(nKeyLen * 2 + 1);
    for (sal_uInt32 i = 0; i < nKeyLen; ++i)
    {
        sal_Int32 nValue = (sal_Int32) pKeyBuffer[i];
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
        int aAlgorithmSize = sizeof(constDigestAlgorithms) / sizeof(constDigestAlgorithms[0]);

        for (int i = 0; i < aAlgorithmSize; i++)
        {
            rtlDigest handle = rtl_digest_create( constDigestAlgorithms[i] );
            CPPUNIT_ASSERT_MESSAGE("create digest", handle != nullptr);
            rtl_digest_destroy( handle );
        }

        rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );
        CPPUNIT_ASSERT_MESSAGE("create invalid digest", handle == nullptr);
        rtl_digest_destroy( handle );
    }

    void testQuery()
    {
        int aAlgorithmSize = sizeof(constDigestAlgorithms) / sizeof(constDigestAlgorithms[0]);

        for (int i = 0; i < aAlgorithmSize; i++)
        {
            rtlDigest handle = rtl_digest_create(constDigestAlgorithms[i]);
            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", constDigestAlgorithms[i] == aAlgo);
            rtl_digest_destroy( handle );
        }

    }

    void testQueryLength()
    {
        int aAlgorithmSize = sizeof(constDigestAlgorithms) / sizeof(constDigestAlgorithms[0]);
        rtlDigest handle;
        sal_uInt32 nAlgoLength;

        for (int i = 0; i < aAlgorithmSize; i++)
        {
            handle = rtl_digest_create(constDigestAlgorithms[i]);
            nAlgoLength = rtl_digest_queryLength(handle);
            CPPUNIT_ASSERT_MESSAGE("query Length", constDigestAlgorithmLengths[i] == nAlgoLength);
            rtl_digest_destroy( handle );
        }

        handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );
        nAlgoLength = rtl_digest_queryLength(handle);
        CPPUNIT_ASSERT_MESSAGE("query length", 0 == nAlgoLength);
        rtl_digest_destroy( handle );
    }

    void testInit()
    {
        rtlDigestError aError;
        rtlDigest handle;

        handle = nullptr;
        aError = rtl_digest_init(handle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("init(NULL, 0, 0)", aError == rtl_Digest_E_Argument);

        handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
        aError = rtl_digest_init(handle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("init(handle, 0, 0)", aError == rtl_Digest_E_None);
        rtl_digest_destroy( handle );

        int aAlgorithmSize = sizeof(constDigestAlgorithms) / sizeof(constDigestAlgorithms[0]);

        for (int i = 0; i < aAlgorithmSize; i++)
        {
            handle = rtl_digest_create(constDigestAlgorithms[i]);

            OString aMessage = sSampleString;
            const sal_uInt8* pData = reinterpret_cast<const sal_uInt8*>(aMessage.getStr());
            sal_uInt32       nSize = aMessage.getLength();

            aError = rtl_digest_init(handle, pData, nSize);
            CPPUNIT_ASSERT_MESSAGE("init(handle, pData, nSize)", aError == rtl_Digest_E_None);

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
            CPPUNIT_ASSERT_MESSAGE("source is the same, dest must be also the same", aSum1.equals(aSum2));
        }

        {
            OString aMsg1 = sSampleString;
            OString aMsg2 = sSampleString_only_one_diff;

            OString aSum1 = getDigest(aMsg1, rtl_Digest_AlgorithmMD5);
            OString aSum2 = getDigest(aMsg2, rtl_Digest_AlgorithmMD5);

            CPPUNIT_ASSERT_MESSAGE("md5sum must have a length", aSum1.getLength() == 32 && aSum2.getLength() == 32 );
            CPPUNIT_ASSERT_MESSAGE("differ only in one char", !aSum1.equals(aSum2));
        }
    }

    void testCheckSum()
    {
        int aAlgorithmSize = sizeof(constDigestAlgorithms) / sizeof(constDigestAlgorithms[0]);

        for (int i = 0; i < aAlgorithmSize; i++)
        {
            OString aSum = getDigest(sSampleString, constDigestAlgorithms[i]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Checksum of sample string is wrong.", constSampleStringSums[i], aSum);
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

        CPPUNIT_ASSERT(aError == rtl_Digest_E_None );

        rtl::OString aKey = createHex(pKeyBuffer.get(), nKeyLen);

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
        CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument);

        aHandle = nullptr;
        aError = rtl_digest_updateMD2(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument);

        aError = rtl_digest_updateMD5(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument);

        aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != nullptr);

        const sal_uInt8* pData = reinterpret_cast<const sal_uInt8*>(sSampleString.getStr());
        sal_uInt32       nSize = sSampleString.getLength();

        aError = rtl_digest_updateMD2(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("handle parameter 'pData' wrong", aError == rtl_Digest_E_Argument);

        aError = rtl_digest_updateMD2(aHandle, pData, 0);
        CPPUNIT_ASSERT_MESSAGE("handle parameter 'nSize' wrong", aError == rtl_Digest_E_None);

        rtl_digest_destroyMD2(aHandle);

        // use wrong Algorithm!!! This is volitional!
        aHandle = rtl_digest_create(rtl_Digest_AlgorithmMD2);
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != nullptr);

        aError = rtl_digest_updateMD5(aHandle, pData, nSize);
        CPPUNIT_ASSERT_MESSAGE("handle parameter 'handle' wrong", aError == rtl_Digest_E_Algorithm);

        rtl_digest_destroyMD5(aHandle);

        aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD5", aHandle != nullptr);

        aError = rtl_digest_updateMD5(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("handle parameter 'pData' wrong", aError == rtl_Digest_E_Argument);

        aError = rtl_digest_updateMD5(aHandle, pData, 0);
        CPPUNIT_ASSERT_MESSAGE("handle parameter 'nSize' wrong", aError == rtl_Digest_E_None);

        rtl_digest_destroyMD5(aHandle);
    }

    void testGet()
    {
        rtlDigest aHandle;
        rtlDigestError aError;

        aHandle = nullptr;
        aError = rtl_digest_get(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument);

        aHandle = nullptr;
        aError = rtl_digest_getMD5(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument);

        // test with wrong algorithm
        aHandle = rtl_digest_create(rtl_Digest_AlgorithmMD2);
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != nullptr);

        sal_uInt32 nKeyLen = rtl_digest_queryLength(aHandle);
        std::unique_ptr<sal_uInt8[]> pKeyBuffer(new sal_uInt8[nKeyLen]);

        aError = rtl_digest_getMD5(aHandle, nullptr, 0);
        CPPUNIT_ASSERT_MESSAGE("handle 2. parameter wrong", aError == rtl_Digest_E_Argument);

        aError = rtl_digest_getMD5(aHandle, pKeyBuffer.get(), 0);
        CPPUNIT_ASSERT_MESSAGE("handle parameter 'handle' wrong", aError == rtl_Digest_E_Algorithm);

        rtl_digest_destroyMD2(aHandle);

        aHandle = rtl_digest_create(rtl_Digest_AlgorithmMD5);
        CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD5", aHandle != nullptr);

        aError = rtl_digest_getMD5(aHandle, nullptr, nKeyLen);
        CPPUNIT_ASSERT_MESSAGE("handle parameter 'pData' wrong", aError == rtl_Digest_E_Argument );

        aError = rtl_digest_getMD5(aHandle, pKeyBuffer.get(), 0);
        CPPUNIT_ASSERT_MESSAGE("handle parameter 'nSize' wrong", aError == rtl_Digest_E_BufferSize );

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

        {
#if 0 // Don't remove, but instead fix the test or something

            // With this test case rtl_digest_SHA1 computes the wrong sum. This was confirmed
            // by decrytion of a MSO encrypted document. Replacing the rtl_digest_SHA1 calculation
            // with sha1 calculation from NSS was able to decrypt the document.

            const unsigned char aData[] = {
                    0x37, 0x5f, 0x47, 0x7a, 0xd2, 0x13, 0xbe, 0xd2, 0x3c, 0x23, 0x33, 0x39,
                    0x68, 0x21, 0x03, 0x6d, 0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00,
                    0x35, 0x00, 0x36, 0x00, 0x37, 0x00, 0x38, 0x00, 0x39, 0x00, 0x30, 0x00,
                    0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00, 0x35, 0x00, 0x36, 0x00,
                    0x37, 0x00, 0x38, 0x00
            };

            std::unique_ptr<sal_uInt8[]> pResult(new sal_uInt8[RTL_DIGEST_LENGTH_SHA1]);

            OString sExpected = "0bfe41eb7fb3edf5f5a6de57192de4ba1b925758";

            rtl_digest_SHA1(aData, sizeof(aData), pResult.get(), RTL_DIGEST_LENGTH_SHA1);

            OString sKey = createHex(pResult.get(), RTL_DIGEST_LENGTH_SHA1);

            CPPUNIT_ASSERT_EQUAL(sExpected, sKey);
#endif
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
