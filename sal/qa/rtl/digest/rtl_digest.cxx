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

#include <rtl/digest.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

// sample, how to use digest

rtl::OUString CreateMD5FromString( const rtl::OUString& aMsg )
{
    // PRE: aStr "file"
    // BACK: Str "ababab....0f" Hexcode String

    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if ( handle != NULL )
    {
        const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();
        sal_uInt32       nSize = ( aMsg.getLength() * sizeof( sal_Unicode ));
        sal_uInt32       nMD5KeyLen = rtl_digest_queryLength( handle );
        sal_uInt8*       pMD5KeyBuffer = new sal_uInt8[ nMD5KeyLen ];

        rtl_digest_init( handle, pData, nSize );
        rtl_digest_update( handle, pData, nSize );
        rtl_digest_get( handle, pMD5KeyBuffer, nMD5KeyLen );
        rtl_digest_destroy( handle );

        // Create hex-value string from the MD5 value to keep the string size minimal
        rtl::OUStringBuffer aBuffer( nMD5KeyLen * 2 + 1 );
        for ( sal_uInt32 i = 0; i < nMD5KeyLen; i++ )
            aBuffer.append( (sal_Int32)pMD5KeyBuffer[i], 16 );

        delete [] pMD5KeyBuffer;
        return aBuffer.makeStringAndClear();
    }

    return rtl::OUString();
}

// -----------------------------------------------------------------------------
namespace rtl_digest
{

    rtl::OString sSampleString               = "This is a sample sentence, which we use to check some crypto functions in sal.";
    rtl::OString sSampleString_MD2           = "647ee6c9d4aa5fdd374ed9d7a156acbf";
    rtl::OString sSampleString_MD5           = "b16b903e6fc0b62ae389013ed93fe531";
    rtl::OString sSampleString_SHA           = "eab2814429b2613301c8a077b806af3680548914";
    rtl::OString sSampleString_SHA1          = "2bc5bdb7506a2cdc2fd27fc8b9889343012d5008";
    rtl::OString sSampleString_HMAC_MD5      = "dd9cba48c972fba0a882baa72b079674";
    rtl::OString sSampleString_HMAC_SHA1     = "5d7f43ce6abd1de4438d7e69e01495864490cf3e";

    rtl::OString sSampleString_only_one_diff = "This is a sample sentence. which we use to check some crypto functions in sal.";

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
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    ASSERT_TRUE(handle != 0) << "create with rtl_Digest_AlgorithmMD5";
    rtl_digest_destroy( handle );
}
TEST_F(create, create_002)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
    ASSERT_TRUE(handle != 0) << "create with rtl_Digest_AlgorithmMD2";
    rtl_digest_destroy( handle );
}
TEST_F(create, create_003)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA );
    ASSERT_TRUE(handle != 0) << "create with rtl_Digest_AlgorithmSHA";
    rtl_digest_destroy( handle );
}
TEST_F(create, create_004)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    ASSERT_TRUE(handle != 0) << "create with rtl_Digest_AlgorithmSHA1";
    rtl_digest_destroy( handle );
}
TEST_F(create, create_005)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_MD5 );
    ASSERT_TRUE(handle != 0) << "create with rtl_Digest_AlgorithmHMAC_MD5";
    rtl_digest_destroy( handle );
}
TEST_F(create, create_006)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_SHA1 );
    ASSERT_TRUE(handle != 0) << "create with rtl_Digest_AlgorithmHMAC_SHA1";
    rtl_digest_destroy( handle );
}
TEST_F(create, create_007)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );
    printf("Handle is %p\n", handle);
    ASSERT_TRUE(handle == 0) << "create with NULL";
    rtl_digest_destroy( handle );
}



// -----------------------------------------------------------------------------

class createMD5 : public ::testing::Test
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

TEST_F(createMD5, createMD5_001)
{
    rtlDigest handle = rtl_digest_createMD5();

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmMD5 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}


// -----------------------------------------------------------------------------

class createMD2 : public ::testing::Test
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

TEST_F(createMD2, createMD2_001)
{
    rtlDigest handle = rtl_digest_createMD2( );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmMD2 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}

// -----------------------------------------------------------------------------

class createSHA : public ::testing::Test
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

TEST_F(createSHA, createSHA_001)
{
    rtlDigest handle = rtl_digest_createSHA( );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmSHA == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}

// -----------------------------------------------------------------------------

class createSHA1 : public ::testing::Test
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

TEST_F(createSHA1, createSHA1_001)
{
    rtlDigest handle = rtl_digest_createSHA1();

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmSHA1 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}

// -----------------------------------------------------------------------------

class createHMAC_MD5 : public ::testing::Test
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

TEST_F(createHMAC_MD5, createHMAC_MD5_001)
{
    rtlDigest handle = rtl_digest_createHMAC_MD5();

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmHMAC_MD5 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}

// -----------------------------------------------------------------------------

class createHMAC_SHA1 : public ::testing::Test
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

TEST_F(createHMAC_SHA1, createHMAC_SHA1_001)
{
    rtlDigest handle = rtl_digest_createHMAC_SHA1();

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmHMAC_SHA1 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}

// -----------------------------------------------------------------------------

class queryAlgorithm : public ::testing::Test
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

TEST_F(queryAlgorithm, query_001)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmMD5 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}
TEST_F(queryAlgorithm, query_002)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmMD2 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}
TEST_F(queryAlgorithm, query_003)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmSHA == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}
TEST_F(queryAlgorithm, query_004)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmSHA1 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}
TEST_F(queryAlgorithm, query_005)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_MD5 );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmHMAC_MD5 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}
TEST_F(queryAlgorithm, query_006)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_SHA1 );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmHMAC_SHA1 == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}
TEST_F(queryAlgorithm, query_007)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );

    rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
    ASSERT_TRUE(rtl_Digest_AlgorithmInvalid == aAlgo) << "query handle";

    rtl_digest_destroy( handle );
}

// -----------------------------------------------------------------------------
class queryLength : public ::testing::Test
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

TEST_F(queryLength, queryLength_MD5)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

    sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
    // printf("nAlgoLength:=%d\n", nAlgoLength);
    ASSERT_TRUE(RTL_DIGEST_LENGTH_MD5 == nAlgoLength) << "query Length";

    rtl_digest_destroy( handle );
}
TEST_F(queryLength, queryLength_MD2)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );

    sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
    // printf("nAlgoLength:=%d\n", nAlgoLength);
    ASSERT_TRUE(RTL_DIGEST_LENGTH_MD2 == nAlgoLength) << "query length";

    rtl_digest_destroy( handle );
}
TEST_F(queryLength, queryLength_SHA)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA );

    sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
    // printf("nAlgoLength:=%d\n", nAlgoLength);
    ASSERT_TRUE(RTL_DIGEST_LENGTH_SHA == nAlgoLength) << "query length";

    rtl_digest_destroy( handle );
}
TEST_F(queryLength, queryLength_SHA1)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );

    sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
    // printf("nAlgoLength:=%d\n", nAlgoLength);
    ASSERT_TRUE(RTL_DIGEST_LENGTH_SHA1 == nAlgoLength) << "query length";

    rtl_digest_destroy( handle );
}
TEST_F(queryLength, queryLength_HMAC_MD5)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_MD5 );

    sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
    // printf("nAlgoLength:=%d\n", nAlgoLength);
    ASSERT_TRUE(RTL_DIGEST_LENGTH_HMAC_MD5 == nAlgoLength) << "query length";

    rtl_digest_destroy( handle );
}
TEST_F(queryLength, queryLength_HMAC_SHA1)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_SHA1 );

    sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
    // printf("nAlgoLength:=%d\n", nAlgoLength);
    ASSERT_TRUE(RTL_DIGEST_LENGTH_HMAC_SHA1 == nAlgoLength) << "query length";

    rtl_digest_destroy( handle );
}

TEST_F(queryLength, queryLength_Illegal)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );

    sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
    // printf("nAlgoLength:=%d\n", nAlgoLength);
    ASSERT_TRUE(0 == nAlgoLength) << "query length";

    rtl_digest_destroy( handle );
}

// -----------------------------------------------------------------------------

rtl::OString createHex(sal_uInt8 *_pMD5KeyBuffer, sal_uInt32 _nMD5KeyLen)
{
    // Create hex-value string from the MD5 value to keep the string size minimal
    rtl::OStringBuffer aBuffer( _nMD5KeyLen * 2 + 1 );
    for ( sal_uInt32 i = 0; i < _nMD5KeyLen; i++ )
    {
        sal_Int32 nValue = (sal_Int32)_pMD5KeyBuffer[i];
        if (nValue < 16)                         // maximul hex value for 1 byte
        {
            aBuffer.append( sal_Int32(0), sal_Int16(16) /* radix */ );
        }
        aBuffer.append( nValue, 16 /* radix */ );
    }

    return aBuffer.makeStringAndClear();
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

TEST_F(init, init_000)
{
    rtlDigest handle = NULL;

    rtlDigestError aError = rtl_digest_init(handle, NULL, 0);

    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "init(NULL, 0, 0)";
}

TEST_F(init, init_001)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

    rtlDigestError aError = rtl_digest_init(handle, NULL, 0);

    ASSERT_TRUE(aError == rtl_Digest_E_None) << "init(handle, 0, 0)";

    rtl_digest_destroy( handle );
}

// ------------------------------------
TEST_F(init, init_MD2)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );

    rtl::OString aMsg = sSampleString;
    const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
    sal_uInt32       nSize = ( aMsg.getLength() );

    rtlDigestError aError = rtl_digest_init(handle, pData, nSize);

    ASSERT_TRUE(aError == rtl_Digest_E_None) << "init(handle, pData, nSize)";

    rtl_digest_update( handle, pData, nSize );

    sal_uInt32     nKeyLen = rtl_digest_queryLength( handle );
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];

    rtl_digest_get( handle, pKeyBuffer, nKeyLen );
    rtl::OString aSum = createHex(pKeyBuffer, nKeyLen);
    delete [] pKeyBuffer;

    printf("MD2 Sum: %s\n", aSum.getStr());
    // LLA: how to check right values
    // samples?

    rtl_digest_destroy( handle );
}

TEST_F(init, init_MD5)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

    rtl::OString aMsg = sSampleString;
    const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
    sal_uInt32       nSize = ( aMsg.getLength() );

    rtlDigestError aError = rtl_digest_init(handle, pData, nSize);

    ASSERT_TRUE(aError == rtl_Digest_E_None) << "init(handle, pData, nSize)";

    rtl_digest_update( handle, pData, nSize );

    sal_uInt32     nKeyLen = rtl_digest_queryLength( handle );
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];

    rtl_digest_get( handle, pKeyBuffer, nKeyLen );
    rtl::OString aSum = createHex(pKeyBuffer, nKeyLen);
    delete [] pKeyBuffer;

    printf("MD5 Sum: %s\n", aSum.getStr());
    // LLA: how to check right values
    // samples?

    rtl_digest_destroy( handle );
}

TEST_F(init, init_SHA)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA );

    rtl::OString aMsg = sSampleString;
    const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
    sal_uInt32       nSize = ( aMsg.getLength() );

    rtlDigestError aError = rtl_digest_init(handle, pData, nSize);

    ASSERT_TRUE(aError == rtl_Digest_E_None) << "init(handle, pData, nSize)";

    rtl_digest_update( handle, pData, nSize );

    sal_uInt32     nKeyLen = rtl_digest_queryLength( handle );
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];

    rtl_digest_get( handle, pKeyBuffer, nKeyLen );
    rtl::OString aSum = createHex(pKeyBuffer, nKeyLen);
    delete [] pKeyBuffer;

    printf("SHA Sum: %s\n", aSum.getStr());
    // LLA: how to check right values
    // samples?

    rtl_digest_destroy( handle );
}
TEST_F(init, init_SHA1)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );

    rtl::OString aMsg = sSampleString;
    const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
    sal_uInt32       nSize = ( aMsg.getLength() );

    rtlDigestError aError = rtl_digest_init(handle, pData, nSize);

    ASSERT_TRUE(aError == rtl_Digest_E_None) << "init(handle, pData, nSize)";

    rtl_digest_update( handle, pData, nSize );

    sal_uInt32     nKeyLen = rtl_digest_queryLength( handle );
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];

    rtl_digest_get( handle, pKeyBuffer, nKeyLen );
    rtl::OString aSum = createHex(pKeyBuffer, nKeyLen);
    delete [] pKeyBuffer;

    printf("SHA1 Sum: %s\n", aSum.getStr());
    // LLA: how to check right values
    // samples?

    rtl_digest_destroy( handle );
}
TEST_F(init, init_HMAC_MD5)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_MD5 );

    rtl::OString aMsg = sSampleString;
    const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
    sal_uInt32       nSize = ( aMsg.getLength() );

    sal_uInt32     nKeyLen = rtl_digest_queryLength( handle );
    ASSERT_TRUE(nKeyLen) << "Keylen must be greater 0";

    sal_uInt8    *pKeyBuffer = new sal_uInt8[ nKeyLen ];
    ASSERT_TRUE( pKeyBuffer );
    memset(pKeyBuffer, 0, nKeyLen);

    rtlDigestError aError = rtl_digest_init(handle, pKeyBuffer, nKeyLen );

    ASSERT_TRUE(aError == rtl_Digest_E_None) << "init(handle, pData, nSize)";

    rtl_digest_update( handle, pData, nSize );

    rtl_digest_get( handle, pKeyBuffer, nKeyLen );
    rtl::OString aSum = createHex(pKeyBuffer, nKeyLen);
    delete [] pKeyBuffer;

    printf("HMAC_MD5 Sum: %s\n", aSum.getStr());
    // LLA: how to check right values
    // samples?

    rtl_digest_destroy( handle );
}
TEST_F(init, init_HMAC_SHA1)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_SHA1 );

    rtl::OString aMsg = sSampleString;
    const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
    sal_uInt32       nSize = ( aMsg.getLength() );

    sal_uInt32     nKeyLen = rtl_digest_queryLength( handle );
    ASSERT_TRUE(nKeyLen) << "Keylen must be greater 0";

    sal_uInt8    *pKeyBuffer = new sal_uInt8[ nKeyLen ];
    ASSERT_TRUE( pKeyBuffer );
    memset(pKeyBuffer, 0, nKeyLen);

    rtlDigestError aError = rtl_digest_init(handle, pKeyBuffer, nKeyLen );

    ASSERT_TRUE(aError == rtl_Digest_E_None) << "init(handle, pData, nSize)";

    rtl_digest_update( handle, pData, nSize );

    rtl_digest_get( handle, pKeyBuffer, nKeyLen );
    rtl::OString aSum = createHex(pKeyBuffer, nKeyLen);
    delete [] pKeyBuffer;

    printf("HMAC_SHA1 Sum: %s\n", aSum.getStr());
    // LLA: how to check right values
    // samples?

    rtl_digest_destroy( handle );
}
// ------------------------------------

rtl::OString getMD5Sum(rtl::OString const& _aMsg )
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

    const sal_uInt8 *pData = (const sal_uInt8*)_aMsg.getStr();
    sal_uInt32       nSize = ( _aMsg.getLength() );

    rtl_digest_init(handle, pData, nSize);
    rtl_digest_update( handle, pData, nSize );

    sal_uInt32     nMD5KeyLen = rtl_digest_queryLength( handle );
    sal_uInt8     *pMD5KeyBuffer = new sal_uInt8[ nMD5KeyLen ];

    rtl_digest_get( handle, pMD5KeyBuffer, nMD5KeyLen );
    rtl::OString aMD5Sum = createHex(pMD5KeyBuffer, nMD5KeyLen);
    delete [] pMD5KeyBuffer;

    rtl_digest_destroy( handle );
    return aMD5Sum;
}

// -----------------------------------------------------------------------------

class equalTests : public ::testing::Test
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

TEST_F(equalTests, equal_001)
{
    rtl::OString aMsg1 = sSampleString;
    rtl::OString aMsg2 = sSampleString;

    rtl::OString aMsgMD5Sum1 = getMD5Sum(aMsg1);
    rtl::OString aMsgMD5Sum2 = getMD5Sum(aMsg2);

    ASSERT_TRUE(aMsgMD5Sum1.getLength() == 32 && aMsgMD5Sum2.getLength() == 32) << "md5sum must have a length";
    ASSERT_TRUE(aMsgMD5Sum1.equals(aMsgMD5Sum2) == sal_True) << "source is the same, dest must be also the same";
}
// ------------------------------------
TEST_F(equalTests, equal_002)
{
    rtl::OString aMsg1 = sSampleString;
    rtl::OString aMsg2 = sSampleString_only_one_diff;

    rtl::OString aMsgMD5Sum1 = getMD5Sum(aMsg1);
    rtl::OString aMsgMD5Sum2 = getMD5Sum(aMsg2);

    ASSERT_TRUE(aMsgMD5Sum1.getLength() == 32 && aMsgMD5Sum2.getLength() == 32) << "md5sum must have a length";
    ASSERT_TRUE(aMsgMD5Sum1.equals(aMsgMD5Sum2) == sal_False) << "differ only in one char";
}


// -----------------------------------------------------------------------------
class digest_MD2 : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class digest_MD2

TEST_F(digest_MD2, MD2_001)
{
    rtl::OString  aMsg1 = sSampleString;

    sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_MD2 ];
    ASSERT_TRUE( pBuffer );
    memset(pBuffer, 0, RTL_DIGEST_LENGTH_MD2 );

    sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
    sal_Int32     nLen  = aMsg1.getLength();

    rtlDigestError aError = rtl_digest_MD2(pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_MD2);

    ASSERT_TRUE(aError == rtl_Digest_E_None );

    rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_MD2);
    printf("Decrypt MD2: %s\n", aStr.getStr());
    ASSERT_TRUE(aStr.equals(sSampleString_MD2)) <<
            "checksum of sample string is wrong. Code changes or sample problems, please check.";

    delete [] pBuffer;
}
// -----------------------------------------------------------------------------
class digest_MD5 : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class digest_MD5

TEST_F(digest_MD5, MD5_001)
{
    rtl::OString  aMsg1 = sSampleString;

    sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_MD5 ];
    ASSERT_TRUE( pBuffer );
    memset(pBuffer, 0, RTL_DIGEST_LENGTH_MD5 );

    sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
    sal_Int32     nLen  = aMsg1.getLength();

    rtlDigestError aError = rtl_digest_MD5(pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_MD5);

    ASSERT_TRUE(aError == rtl_Digest_E_None );

    rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_MD5);
    printf("Decrypt MD5: %s\n", aStr.getStr());
    ASSERT_TRUE(aStr.equals(sSampleString_MD5) ) <<
            "checksum of sample string is wrong. Code changes or sample problems, please check.";

    delete [] pBuffer;
}

// -----------------------------------------------------------------------------
class digest_SHA : public ::testing::Test
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

TEST_F(digest_SHA, SHA_001)
{
    rtl::OString  aMsg1 = sSampleString;

    sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_SHA ];
    ASSERT_TRUE( pBuffer );
    memset(pBuffer, 0, RTL_DIGEST_LENGTH_SHA);

    sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
    sal_Int32     nLen  = aMsg1.getLength();

    rtlDigestError aError = rtl_digest_SHA(pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_SHA);

    ASSERT_TRUE(aError == rtl_Digest_E_None );

    rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_SHA);
    printf("Decrypt SHA: %s\n", aStr.getStr());
    ASSERT_TRUE(aStr.equals(sSampleString_SHA)) <<
            "checksum of sample string is wrong. Code changes or sample problems, please check.";

    delete [] pBuffer;
}
// -----------------------------------------------------------------------------
class digest_SHA1 : public ::testing::Test
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

TEST_F(digest_SHA1, SHA1_001)
{
    rtl::OString  aMsg1 = sSampleString;

    sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_SHA1 ];
    ASSERT_TRUE( pBuffer );
    memset(pBuffer, 0, RTL_DIGEST_LENGTH_SHA1);

    sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
    sal_Int32     nLen  = aMsg1.getLength();

    rtlDigestError aError = rtl_digest_SHA1(pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_SHA1);

    ASSERT_TRUE(aError == rtl_Digest_E_None );

    rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_SHA1);
    printf("Decrypt SHA1: %s\n", aStr.getStr());
    ASSERT_TRUE(aStr.equals(sSampleString_SHA1)) <<
            "checksum of sample string is wrong. Code changes or sample problems, please check.";

    delete [] pBuffer;
}
// -----------------------------------------------------------------------------
class digest_HMAC_MD5 : public ::testing::Test
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

TEST_F(digest_HMAC_MD5, HMAC_MD5_001)
{
    rtl::OString  aMsg1 = sSampleString;

    sal_uInt8    *pKeyBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_HMAC_MD5 ];
    ASSERT_TRUE( pKeyBuffer );
    memset(pKeyBuffer, 0, RTL_DIGEST_LENGTH_HMAC_MD5);

    sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_HMAC_MD5 ];
    ASSERT_TRUE( pBuffer );
    memset(pBuffer, 0, RTL_DIGEST_LENGTH_HMAC_MD5);

    sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
    sal_Int32     nLen  = aMsg1.getLength();

    rtlDigestError aError = rtl_digest_HMAC_MD5(pKeyBuffer, RTL_DIGEST_LENGTH_HMAC_MD5, pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_HMAC_MD5);

    ASSERT_TRUE(aError == rtl_Digest_E_None );

    rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_HMAC_MD5);
    printf("Decrypt HMAC_MD5: %s\n", aStr.getStr());
    ASSERT_TRUE(aStr.equals(sSampleString_HMAC_MD5)) <<
            "md5sum of sample string is wrong. Code changes or sample problems, please check.";

    delete [] pBuffer;
}

// -----------------------------------------------------------------------------
class digest_HMAC_SHA1 : public ::testing::Test
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

TEST_F(digest_HMAC_SHA1, HMAC_SHA1_001)
{
    rtl::OString  aMsg1 = sSampleString;

    sal_uInt8    *pKeyBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_HMAC_SHA1 ];
    ASSERT_TRUE( pKeyBuffer );
    memset(pKeyBuffer, 0, RTL_DIGEST_LENGTH_HMAC_SHA1);

    sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_HMAC_SHA1 ];
    ASSERT_TRUE( pBuffer );
    memset(pBuffer, 0, RTL_DIGEST_LENGTH_HMAC_SHA1);

    sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
    sal_Int32     nLen  = aMsg1.getLength();

    rtlDigestError aError = rtl_digest_HMAC_SHA1(pKeyBuffer, RTL_DIGEST_LENGTH_HMAC_SHA1, pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_HMAC_SHA1);

    ASSERT_TRUE(aError == rtl_Digest_E_None );

    rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_HMAC_SHA1);
    printf("Decrypt HMAC_SHA1: %s\n", aStr.getStr());
    ASSERT_TRUE(aStr.equals(sSampleString_HMAC_SHA1)) <<
            "md5sum of sample string is wrong. Code changes or sample problems, please check.";

    delete [] pBuffer;
}
// -----------------------------------------------------------------------------
class digest_PBKDF2 : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    rtl::OString /* key */ run_check_PBKDF2(rtl::OString const& _sPassword, bool _bClearSalt, sal_uInt32 _nCount)
    {
        sal_uInt32   nKeyLen = RTL_DIGEST_LENGTH_HMAC_SHA1;
        sal_uInt8    *pKeyBuffer = new sal_uInt8[ nKeyLen ];
        EXPECT_TRUE( pKeyBuffer );
        memset(pKeyBuffer, 0, nKeyLen);

        sal_uInt8    *pPassword    = (sal_uInt8*)_sPassword.getStr();
        sal_Int32     nPasswordLen = _sPassword.getLength();

        sal_uInt32   nSaltDataLen = RTL_DIGEST_LENGTH_HMAC_SHA1;
        sal_uInt8    *pSaltData = new sal_uInt8[ nSaltDataLen ];
        EXPECT_TRUE( pSaltData );
        memset(pSaltData, 0, nSaltDataLen);

        if (! _bClearSalt)
        {
            // wilful contamination
            pSaltData[0] = 1;
        }

        rtlDigestError aError = rtl_digest_PBKDF2(pKeyBuffer, nKeyLen, pPassword, nPasswordLen, pSaltData, nSaltDataLen, _nCount);

        EXPECT_TRUE(aError == rtl_Digest_E_None );

        rtl::OString aKey = createHex(pKeyBuffer, nKeyLen);
        printf("Key: %s\n", aKey.getStr());

        // rtl::OString sSalt = createHex(pSaltData, nSaltDataLen);
        // printf("Salt: %s\n", sSalt.getStr());

        // EXPECT_TRUE(aStr.equals(sSampleString_PBKDF2)) <<
        //      "md5sum of sample string is wrong. Code changes or sample problems, please check.";

        delete [] pSaltData;
        delete [] pKeyBuffer;
        return aKey;
    }
}; // class create

TEST_F(digest_PBKDF2, PBKDF2_001)
{
    rtl::OString  aPassword = "Password";

    // all permutations
    run_check_PBKDF2(aPassword, false, 1);
    run_check_PBKDF2(aPassword, false, 2);
    run_check_PBKDF2(aPassword, true,  1);
    run_check_PBKDF2(aPassword, true,  2);
    run_check_PBKDF2(aPassword, false, 3);
    run_check_PBKDF2(aPassword, false, 4);
    run_check_PBKDF2(aPassword, true,  3);
    run_check_PBKDF2(aPassword, true,  4);
}

// -----------------------------------------------------------------------------

class update : public ::testing::Test
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

TEST_F(update, update_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_update(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}

TEST_F(update, updateMD2_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_updateMD2(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}

TEST_F(update, updateMD2_001)
{
    rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
    ASSERT_TRUE(aHandle != 0) << "create with rtl_Digest_AlgorithmMD2";

    rtl::OString aMsg = sSampleString;
    const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();

    rtlDigestError aError = rtl_digest_updateMD2(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "handle parameter 'pData' wrong";

    /* rtlDigestError */ aError = rtl_digest_updateMD2(aHandle, pData, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_None) << "handle parameter 'nSize' wrong";

    rtl_digest_destroyMD2(aHandle);
}
TEST_F(update, updateMD5_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_updateMD5(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}

TEST_F(update, updateMD5_001)
{
    // use wrong Algorithm!!! This is volitional!
    rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
    ASSERT_TRUE(aHandle != 0) << "create with rtl_Digest_AlgorithmMD2";

    rtl::OString aMsg = sSampleString;
    const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();
    sal_uInt32       nSize = ( aMsg.getLength() );

    rtlDigestError aError = rtl_digest_updateMD5(aHandle, pData, nSize);
    ASSERT_TRUE(aError == rtl_Digest_E_Algorithm) << "handle parameter 'handle' wrong";

    rtl_digest_destroyMD5(aHandle);
}

TEST_F(update, updateMD5_002)
{
    rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    ASSERT_TRUE(aHandle != 0) << "create with rtl_Digest_AlgorithmMD5";

    rtl::OString aMsg = sSampleString;
    const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();

    rtlDigestError aError = rtl_digest_updateMD5(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "handle parameter 'pData' wrong";

    /* rtlDigestError */ aError = rtl_digest_updateMD5(aHandle, pData, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_None) << "handle parameter 'nSize' wrong";

    rtl_digest_destroyMD5(aHandle);
}

TEST_F(update, updateSHA_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_updateSHA(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}

TEST_F(update, updateSHA1_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_updateSHA1(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}

TEST_F(update, updateHMAC_MD5_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_updateHMAC_MD5(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}

TEST_F(update, updateHMAC_SHA1_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_updateHMAC_SHA1(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}

// -----------------------------------------------------------------------------

class get : public ::testing::Test
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

TEST_F(get, get_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_get(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}
TEST_F(get, getMD5_000)
{
    rtlDigest aHandle = NULL;
    rtlDigestError aError = rtl_digest_getMD5(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "does not handle wrong parameter";
}
TEST_F(get, getMD5_001)
{
    // test with wrong algorithm
    rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
    ASSERT_TRUE(aHandle != 0) << "create with rtl_Digest_AlgorithmMD2";

    sal_uInt32     nKeyLen = rtl_digest_queryLength( aHandle );
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];

    rtlDigestError aError = rtl_digest_getMD5(aHandle, NULL, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "handle 2. parameter wrong";

    /* rtlDigestError */ aError = rtl_digest_getMD5(aHandle, pKeyBuffer, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_Algorithm) << "handle parameter 'handle' wrong";

    rtl_digest_destroyMD2(aHandle);
}

TEST_F(get, getMD5_002)
{
    rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    ASSERT_TRUE(aHandle != 0) << "create with rtl_Digest_AlgorithmMD5";

    sal_uInt32     nKeyLen = rtl_digest_queryLength( aHandle );
    sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];

    rtlDigestError aError = rtl_digest_getMD5(aHandle, NULL /* pKeyBuffer */ , nKeyLen);
    ASSERT_TRUE(aError == rtl_Digest_E_Argument) << "handle parameter 'pData' wrong";

    /* rtlDigestError */ aError = rtl_digest_getMD5(aHandle, pKeyBuffer, 0);
    ASSERT_TRUE(aError == rtl_Digest_E_BufferSize) << "handle parameter 'nSize' wrong";

    rtl_digest_destroyMD5(aHandle);
    delete [] pKeyBuffer;
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
}; // class create

TEST_F(destroy, destroy_001)
{
    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    ASSERT_TRUE(handle != 0) << "create with rtl_Digest_AlgorithmMD5";

    // not really testable
    // LLA: good will test.
    rtl_digest_destroy( handle );
}
// -----------------------------------------------------------------------------

} // namespace rtl_digest

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
