/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include "gtest/gtest.h"
#include <rtl/strbuf.hxx>
#include <rtl/cipher.h>

// -----------------------------------------------------------------------------
namespace rtl_cipher
{

rtl::OString createHex(sal_uInt8 *_pKeyBuffer, sal_uInt32 _nKeyLen)
{
    // Create hex-value string from the  value to keep the string size minimal
    rtl::OStringBuffer aBuffer( _nKeyLen * 2 + 1 );
    for ( sal_uInt32 i = 0; i < _nKeyLen; i++ )
    {
        sal_Int32 nValue = (sal_Int32)_pKeyBuffer[i];
        if (nValue < 16)                         // maximul hex value for 1 byte
        {
            aBuffer.append( sal_Int32(0), sal_Int16(16) /* radix */ );
        }
        aBuffer.append( nValue, 16 /* radix */ );
    }

    return aBuffer.makeStringAndClear();
}

// -----------------------------------------------------------------------------

class create : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class create

TEST_F(create, create_001)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";
    rtl_cipher_destroy(aCipher);
}
TEST_F(create, create_002)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmInvalid, rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher == NULL) << "create provide wrong object.";
}
TEST_F(create, create_003)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeCBC);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";
    rtl_cipher_destroy(aCipher);
}
TEST_F(create, create_004)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmInvalid, rtl_Cipher_ModeCBC);
    ASSERT_TRUE(aCipher == NULL) << "create provide wrong object.";
}
TEST_F(create, create_005)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";
    rtl_cipher_destroy(aCipher);
}
TEST_F(create, create_006)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmInvalid, rtl_Cipher_ModeStream);
    ASSERT_TRUE(aCipher == NULL) << "create provide wrong object.";
}
TEST_F(create, create_007)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeInvalid);
    ASSERT_TRUE(aCipher == NULL) << "create provide wrong object.";
}
TEST_F(create, create_008)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmInvalid, rtl_Cipher_ModeInvalid);
    ASSERT_TRUE(aCipher == NULL) << "create provide wrong object.";
}
// -----------------------------------------------------------------------------
class createBF : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class createBF

TEST_F(createBF, createBF_001)
{
    rtlCipher aCipher = rtl_cipher_createBF(rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";
    rtl_cipher_destroy(aCipher);
}
TEST_F(createBF, createBF_002)
{
    rtlCipher aCipher = rtl_cipher_createBF(rtl_Cipher_ModeCBC);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";
    rtl_cipher_destroy(aCipher);
}
TEST_F(createBF, createBF_003)
{
    rtlCipher aCipher = rtl_cipher_createBF(rtl_Cipher_ModeStream);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";
    rtl_cipher_destroy(aCipher);
}
TEST_F(createBF, createBF_004)
{
    rtlCipher aCipher = rtl_cipher_createBF(rtl_Cipher_ModeInvalid);
    ASSERT_TRUE(aCipher == NULL) << "create provide wrong object.";
    // rtl_cipher_destroy(aCipher);
}
// -----------------------------------------------------------------------------
class decode : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    void test_encode(sal_uInt8 _nKeyValue, sal_uInt8 _nArgValue, rtl::OString const& _sPlainTextStr)
    {
        rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
        ASSERT_TRUE(aCipher != NULL) << "create failed.";

        sal_uInt32     nKeyLen = 16;
        sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];
        memset(pKeyBuffer, 0, nKeyLen);
        pKeyBuffer[0] = _nKeyValue;

        sal_uInt32     nArgLen = 16;
        sal_uInt8     *pArgBuffer = new sal_uInt8[ nArgLen ];
        memset(pArgBuffer, 0, nArgLen);
        pArgBuffer[0] = _nArgValue;

        printf("  init Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
        printf("  init Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

        rtlCipherError aError = rtl_cipher_init(aCipher, rtl_Cipher_DirectionEncode, pKeyBuffer, nKeyLen, pArgBuffer, nArgLen);
        ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong init";

        sal_uInt32     nPlainTextLen = 16;
        sal_uInt8     *pPlainTextBuffer = new sal_uInt8[ nPlainTextLen ];
        memset(pPlainTextBuffer, 0, nPlainTextLen);
        strncpy((char*)pPlainTextBuffer, _sPlainTextStr.getStr(), 16);

        sal_uInt32     nCipherLen = 16;
        sal_uInt8     *pCipherBuffer = new sal_uInt8[ nCipherLen ];
        memset(pCipherBuffer, 0, nCipherLen);

        /* rtlCipherError */ aError = rtl_cipher_encode(aCipher, pPlainTextBuffer, nPlainTextLen, pCipherBuffer, nCipherLen);
        ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong encode";

        printf("       Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
        printf("       Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());
        printf("     Plain: %s\n", createHex(pPlainTextBuffer, nPlainTextLen).getStr());
        printf(           "Cipher Buf: %s\n", createHex(pCipherBuffer, nCipherLen).getStr());

        sal_uInt32     nPlainText2Len = 16;
        sal_uInt8     *pPlainText2Buffer = new sal_uInt8[ nPlainText2Len ];
        memset(pPlainText2Buffer, 0, nPlainText2Len);

        /* rtlCipherError */ aError = rtl_cipher_decode(aCipher, pCipherBuffer, nCipherLen, pPlainText2Buffer, nPlainText2Len);
        ASSERT_TRUE(aError != rtl_Cipher_E_None) << "decode should not work";

        // rtl::OString sPlainText2Str((char*)pPlainText2Buffer, nPlainText2Len);
        // printf(" Plain: %s\n", createHex(pPlainText2Buffer, nPlainText2Len).getStr());
        // printf(" ascii: %s\n", sPlainText2Str.getStr());
        //
        // // printf("   Buf: %s\n", createHex(pCipherBuffer, nCipherLen).getStr());
        //
        // sal_Int32 nCompare = memcmp(pPlainTextBuffer, pPlainText2Buffer, 16);
        //
        // ASSERT_TRUE(nCompare == 0) << "compare between plain and decoded plain failed";
        //
        // delete [] pPlainText2Buffer;
        //
        // delete [] pCipherBuffer;
        // delete [] pPlainTextBuffer;
        //
        // delete [] pArgBuffer;
        // delete [] pKeyBuffer;
        //
        // rtl_cipher_destroy(aCipher);
    }

    void test_encode_and_decode(sal_uInt8 _nKeyValue, sal_uInt8 _nArgValue, rtl::OString const& _sPlainTextStr)
    {
        rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
        ASSERT_TRUE(aCipher != NULL) << "create failed.";

        sal_uInt32     nKeyLen = 16;
        sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];
        memset(pKeyBuffer, 0, nKeyLen);
        pKeyBuffer[0] = _nKeyValue;

        sal_uInt32     nArgLen = 16;
        sal_uInt8     *pArgBuffer = new sal_uInt8[ nArgLen ];
        memset(pArgBuffer, 0, nArgLen);
        pArgBuffer[0] = _nArgValue;

        printf("  init Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
        printf("  init Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

        rtlCipherError aError = rtl_cipher_init(aCipher, rtl_Cipher_DirectionBoth, pKeyBuffer, nKeyLen, pArgBuffer, nArgLen);
        ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong init";

        sal_uInt32     nPlainTextLen = 16;
        sal_uInt8     *pPlainTextBuffer = new sal_uInt8[ nPlainTextLen ];
        memset(pPlainTextBuffer, 0, nPlainTextLen);
        strncpy((char*)pPlainTextBuffer, _sPlainTextStr.getStr(), 16);

        sal_uInt32     nCipherLen = 16;
        sal_uInt8     *pCipherBuffer = new sal_uInt8[ nCipherLen ];
        memset(pCipherBuffer, 0, nCipherLen);

        /* rtlCipherError */ aError = rtl_cipher_encode(aCipher, pPlainTextBuffer, nPlainTextLen, pCipherBuffer, nCipherLen);
        ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong encode";

        printf("       Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
        printf("       Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());
        printf("     Plain: %s\n", createHex(pPlainTextBuffer, nPlainTextLen).getStr());
        printf(           "Cipher Buf: %s\n", createHex(pCipherBuffer, nCipherLen).getStr());

        sal_uInt32     nPlainText2Len = 16;
        sal_uInt8     *pPlainText2Buffer = new sal_uInt8[ nPlainText2Len ];
        memset(pPlainText2Buffer, 0, nPlainText2Len);

        /* rtlCipherError */ aError = rtl_cipher_decode(aCipher, pCipherBuffer, nCipherLen, pPlainText2Buffer, nPlainText2Len);
        ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong decode";

        rtl::OString sPlainText2Str((char*)pPlainText2Buffer, nPlainText2Len);
        printf("     Plain: %s\n", createHex(pPlainText2Buffer, nPlainText2Len).getStr());
        printf("  as ascii: %s\n", sPlainText2Str.getStr());

        // printf("   Buf: %s\n", createHex(pCipherBuffer, nCipherLen).getStr());

        sal_Int32 nCompare = memcmp(pPlainTextBuffer, pPlainText2Buffer, 16);

        ASSERT_TRUE(nCompare == 0) << "compare between plain and decoded plain failed";

        delete [] pPlainText2Buffer;

        delete [] pCipherBuffer;
        delete [] pPlainTextBuffer;

        delete [] pArgBuffer;
        delete [] pKeyBuffer;

        rtl_cipher_destroy(aCipher);
    }
}; // class decode

TEST_F(decode, decode_001)
{
    test_encode_and_decode(0,0,"");
    test_encode_and_decode(0,0,"hallo");
    test_encode_and_decode(1,0,"B2Aahg5B");
    test_encode_and_decode(1,2,"Longer text string");
}

TEST_F(decode, decode_002)
{
    test_encode(0,0,"");
    test_encode(0,0,"hallo");
    test_encode(1,0,"B2Aahg5B");
    test_encode(1,2,"Longer text string");
}
// -----------------------------------------------------------------------------
class decodeBF : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class decodeBF

TEST_F(decodeBF, decodeBF_001)
{
}

// -----------------------------------------------------------------------------
class destroy : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class destroy

TEST_F(destroy, destroy_001)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeCBC);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";
    rtl_cipher_destroy(aCipher);
}

// -----------------------------------------------------------------------------
class destroyBF : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class destroyBF

TEST_F(destroyBF, destroyBF_001)
{
    rtlCipher aCipher = rtl_cipher_createBF(rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";
    rtl_cipher_destroyBF(aCipher);
    // more proforma
    // should not GPF
}

// -----------------------------------------------------------------------------
class encode : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class encode

void test_encode(sal_uInt8 _nKeyValue, sal_uInt8 _nArgValue, sal_uInt8 _nDataValue)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";

    sal_uInt32     nKeyLen = 16;
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];
    memset(pKeyBuffer, 0, nKeyLen);
    pKeyBuffer[0] = _nKeyValue;

    sal_uInt32     nArgLen = 16;
    sal_uInt8     *pArgBuffer = new sal_uInt8[ nArgLen ];
    memset(pArgBuffer, 0, nArgLen);
    pArgBuffer[0] = _nArgValue;

    printf("init Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("init Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    rtlCipherError aError = rtl_cipher_init(aCipher, rtl_Cipher_DirectionEncode, pKeyBuffer, nKeyLen, pArgBuffer, nArgLen);
    ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong init";

    sal_uInt32     nDataLen = 16;
    sal_uInt8     *pDataBuffer = new sal_uInt8[ nDataLen ];
    memset(pDataBuffer, 0, nDataLen);
    pDataBuffer[0] = _nDataValue;

    sal_uInt32     nLen = 16;
    sal_uInt8     *pBuffer = new sal_uInt8[ nLen ];
    memset(pBuffer, 0, nLen);

    /* rtlCipherError */ aError = rtl_cipher_encode(aCipher, pDataBuffer, nDataLen, pBuffer, nLen);
    ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong encode";

    printf(" Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf(" Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());
    printf("Data: %s\n", createHex(pDataBuffer, nDataLen).getStr());
    printf(" Buf: %s\n", createHex(pBuffer, nLen).getStr());

    delete [] pBuffer;
    delete [] pDataBuffer;

    delete [] pArgBuffer;
    delete [] pKeyBuffer;

    rtl_cipher_destroy(aCipher);
}

void encode_001()
{
    test_encode(0,0,0);
    test_encode(1,0,0);
    test_encode(0,1,0);
    test_encode(1,1,0);

    test_encode(0,0,1);
    test_encode(1,0,1);
    test_encode(0,1,1);
    test_encode(1,1,1);
}
// -----------------------------------------------------------------------------
class encodeBF : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class encodeBF

TEST_F(encodeBF, encodeBF_001)
{
}

// -----------------------------------------------------------------------------
class init : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class init

TEST_F(init, init_001)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";

    sal_uInt32     nKeyLen = 16;
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];
    memset(pKeyBuffer, 0, nKeyLen);

    sal_uInt32     nArgLen = 16;
    sal_uInt8     *pArgBuffer = new sal_uInt8[ nArgLen ];
    memset(pArgBuffer, 0, nArgLen);

    printf("Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    rtlCipherError aError = rtl_cipher_init(aCipher, rtl_Cipher_DirectionEncode, pKeyBuffer, nKeyLen, pArgBuffer, nArgLen);
    ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong init";

    printf("Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    delete [] pArgBuffer;
    delete [] pKeyBuffer;

    rtl_cipher_destroy(aCipher);
}

TEST_F(init, init_002)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";

    sal_uInt32     nKeyLen = 16;
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];
    memset(pKeyBuffer, 0, nKeyLen);
    pKeyBuffer[0] = 1;

    sal_uInt32     nArgLen = 16;
    sal_uInt8     *pArgBuffer = new sal_uInt8[ nArgLen ];
    memset(pArgBuffer, 0, nArgLen);

    printf("Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    rtlCipherError aError = rtl_cipher_init(aCipher, rtl_Cipher_DirectionEncode, pKeyBuffer, nKeyLen, pArgBuffer, nArgLen);
    ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong init";

    printf("Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    delete [] pArgBuffer;
    delete [] pKeyBuffer;

    rtl_cipher_destroy(aCipher);
}
TEST_F(init, init_003)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";

    sal_uInt32     nKeyLen = 16;
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];
    memset(pKeyBuffer, 0, nKeyLen);

    sal_uInt32     nArgLen = 16;
    sal_uInt8     *pArgBuffer = new sal_uInt8[ nArgLen ];
    memset(pArgBuffer, 0, nArgLen);
    pArgBuffer[0] = 1;

    printf("Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    rtlCipherError aError = rtl_cipher_init(aCipher, rtl_Cipher_DirectionEncode, pKeyBuffer, nKeyLen, pArgBuffer, nArgLen);
    ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong init";

    printf("Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    delete [] pArgBuffer;
    delete [] pKeyBuffer;

    rtl_cipher_destroy(aCipher);
}
TEST_F(init, init_004)
{
    rtlCipher aCipher = rtl_cipher_create(rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
    ASSERT_TRUE(aCipher != NULL) << "create failed.";

    sal_uInt32     nKeyLen = 16;
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];
    memset(pKeyBuffer, 0, nKeyLen);
    pKeyBuffer[0] = 1;

    sal_uInt32     nArgLen = 16;
    sal_uInt8     *pArgBuffer = new sal_uInt8[ nArgLen ];
    memset(pArgBuffer, 0, nArgLen);
    pArgBuffer[0] = 1;

    printf("Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    rtlCipherError aError = rtl_cipher_init(aCipher, rtl_Cipher_DirectionEncode, pKeyBuffer, nKeyLen, pArgBuffer, nArgLen);
    ASSERT_TRUE(aError == rtl_Cipher_E_None) << "wrong init";

    printf("Key: %s\n", createHex(pKeyBuffer, nKeyLen).getStr());
    printf("Arg: %s\n", createHex(pArgBuffer, nArgLen).getStr());

    delete [] pArgBuffer;
    delete [] pKeyBuffer;

    rtl_cipher_destroy(aCipher);
}
// -----------------------------------------------------------------------------
class initBF : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class initBF

TEST_F(initBF, initBF_001)
{
    // seems to be the same as init, so empty
}


} // namespace rtl_cipher

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
