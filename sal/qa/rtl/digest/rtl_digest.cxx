/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/digest.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <string.h>

// sample, how to use digest

rtl::OUString CreateMD5FromString( const rtl::OUString& aMsg )
{
    // PRE: aStr "file"
    // BACK: Str "ababab....0f" Hexcode String

    rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if ( handle > 0 )
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
        for ( sal_uInt32 i = 0; i < nMD5KeyLen; ++i )
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

class create : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void create_001()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD5", handle != 0);
            rtl_digest_destroy( handle );
        }
    void create_002()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", handle != 0);
            rtl_digest_destroy( handle );
        }
    void create_003()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmSHA", handle != 0);
            rtl_digest_destroy( handle );
        }
    void create_004()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmSHA1", handle != 0);
            rtl_digest_destroy( handle );
        }
    void create_005()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_MD5 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmHMAC_MD5", handle != 0);
            rtl_digest_destroy( handle );
        }
    void create_006()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_SHA1 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmHMAC_SHA1", handle != 0);
            rtl_digest_destroy( handle );
        }

    void create_007()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );
            printf("Handle is %p\n", handle);
            CPPUNIT_ASSERT_MESSAGE("create with NULL", handle == 0);
            rtl_digest_destroy( handle );
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(create);
    CPPUNIT_TEST(create_001);
    CPPUNIT_TEST(create_002);
    CPPUNIT_TEST(create_003);
    CPPUNIT_TEST(create_004);
    CPPUNIT_TEST(create_005);
    CPPUNIT_TEST(create_006);
    CPPUNIT_TEST(create_007);
    CPPUNIT_TEST_SUITE_END();
}; // class create




// -----------------------------------------------------------------------------

class createMD5 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void createMD5_001()
        {
            rtlDigest handle = rtl_digest_createMD5();

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmMD5 == aAlgo);

            rtl_digest_destroy( handle );
        }
    CPPUNIT_TEST_SUITE(createMD5);
    CPPUNIT_TEST(createMD5_001);
    CPPUNIT_TEST_SUITE_END();
}; // class create


// -----------------------------------------------------------------------------

class createMD2 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void createMD2_001()
        {
            rtlDigest handle = rtl_digest_createMD2( );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmMD2 == aAlgo);

            rtl_digest_destroy( handle );
        }
    CPPUNIT_TEST_SUITE(createMD2);
    CPPUNIT_TEST(createMD2_001);
    CPPUNIT_TEST_SUITE_END();
}; // class create

// -----------------------------------------------------------------------------

class createSHA : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void createSHA_001()
        {
            rtlDigest handle = rtl_digest_createSHA( );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmSHA == aAlgo);

            rtl_digest_destroy( handle );
        }
    CPPUNIT_TEST_SUITE(createSHA);
    CPPUNIT_TEST(createSHA_001);
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------

class createSHA1 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void createSHA1_001()
        {
            rtlDigest handle = rtl_digest_createSHA1();

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmSHA1 == aAlgo);

            rtl_digest_destroy( handle );
        }
    CPPUNIT_TEST_SUITE(createSHA1);
    CPPUNIT_TEST(createSHA1_001);
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------

class createHMAC_MD5 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void createHMAC_MD5_001()
        {
            rtlDigest handle = rtl_digest_createHMAC_MD5();

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmHMAC_MD5 == aAlgo);

            rtl_digest_destroy( handle );
        }
    CPPUNIT_TEST_SUITE(createHMAC_MD5);
    CPPUNIT_TEST(createHMAC_MD5_001);
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------

class createHMAC_SHA1 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void createHMAC_SHA1_001()
        {
            rtlDigest handle = rtl_digest_createHMAC_SHA1();

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmHMAC_SHA1 == aAlgo);

            rtl_digest_destroy( handle );
        }


    CPPUNIT_TEST_SUITE(createHMAC_SHA1);
    CPPUNIT_TEST(createHMAC_SHA1_001);
    CPPUNIT_TEST_SUITE_END();
}; // class create

// -----------------------------------------------------------------------------

class queryAlgorithm : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void query_001()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmMD5 == aAlgo);

            rtl_digest_destroy( handle );
        }
    void query_002()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmMD2 == aAlgo);

            rtl_digest_destroy( handle );
        }
    void query_003()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmSHA == aAlgo);

            rtl_digest_destroy( handle );
        }
    void query_004()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmSHA1 == aAlgo);

            rtl_digest_destroy( handle );
        }
    void query_005()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_MD5 );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmHMAC_MD5 == aAlgo);

            rtl_digest_destroy( handle );
        }
    void query_006()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_SHA1 );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmHMAC_SHA1 == aAlgo);

            rtl_digest_destroy( handle );
        }
    void query_007()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );

            rtlDigestAlgorithm aAlgo = rtl_digest_queryAlgorithm(handle);
            CPPUNIT_ASSERT_MESSAGE("query handle", rtl_Digest_AlgorithmInvalid == aAlgo);

            rtl_digest_destroy( handle );
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(queryAlgorithm);
    CPPUNIT_TEST( query_001 );
    CPPUNIT_TEST( query_002 );
    CPPUNIT_TEST( query_003 );
    CPPUNIT_TEST( query_004 );
    CPPUNIT_TEST( query_005 );
    CPPUNIT_TEST( query_006 );
    CPPUNIT_TEST( query_007 );
    CPPUNIT_TEST_SUITE_END();
}; // class create


// -----------------------------------------------------------------------------
class queryLength : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void queryLength_MD5()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

            sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
            // printf("nAlgoLength:=%d\n", nAlgoLength);
            CPPUNIT_ASSERT_MESSAGE("query Length", RTL_DIGEST_LENGTH_MD5 == nAlgoLength);

            rtl_digest_destroy( handle );
        }
    void queryLength_MD2()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );

            sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
            // printf("nAlgoLength:=%d\n", nAlgoLength);
            CPPUNIT_ASSERT_MESSAGE("query length", RTL_DIGEST_LENGTH_MD2 == nAlgoLength);

            rtl_digest_destroy( handle );
        }
    void queryLength_SHA()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA );

            sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
            // printf("nAlgoLength:=%d\n", nAlgoLength);
            CPPUNIT_ASSERT_MESSAGE("query length", RTL_DIGEST_LENGTH_SHA == nAlgoLength);

            rtl_digest_destroy( handle );
        }
    void queryLength_SHA1()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );

            sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
            // printf("nAlgoLength:=%d\n", nAlgoLength);
            CPPUNIT_ASSERT_MESSAGE("query length", RTL_DIGEST_LENGTH_SHA1 == nAlgoLength);

            rtl_digest_destroy( handle );
        }
    void queryLength_HMAC_MD5()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_MD5 );

            sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
            // printf("nAlgoLength:=%d\n", nAlgoLength);
            CPPUNIT_ASSERT_MESSAGE("query length", RTL_DIGEST_LENGTH_HMAC_MD5 == nAlgoLength);

            rtl_digest_destroy( handle );
        }
    void queryLength_HMAC_SHA1()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_SHA1 );

            sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
            // printf("nAlgoLength:=%d\n", nAlgoLength);
            CPPUNIT_ASSERT_MESSAGE("query length", RTL_DIGEST_LENGTH_HMAC_SHA1 == nAlgoLength);

            rtl_digest_destroy( handle );
        }

    void queryLength_Illegal()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmInvalid );

            sal_uInt32 nAlgoLength = rtl_digest_queryLength(handle);
            // printf("nAlgoLength:=%d\n", nAlgoLength);
            CPPUNIT_ASSERT_MESSAGE("query length", 0 == nAlgoLength);

            rtl_digest_destroy( handle );
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(queryLength);
    CPPUNIT_TEST( queryLength_MD2 );
    CPPUNIT_TEST( queryLength_MD5 );
    CPPUNIT_TEST( queryLength_SHA );
    CPPUNIT_TEST( queryLength_SHA1 );
    CPPUNIT_TEST( queryLength_HMAC_MD5 );
    CPPUNIT_TEST( queryLength_HMAC_SHA1 );
    CPPUNIT_TEST( queryLength_Illegal );
    CPPUNIT_TEST_SUITE_END();
}; // class create

// -----------------------------------------------------------------------------

rtl::OString createHex(sal_uInt8 *_pMD5KeyBuffer, sal_uInt32 _nMD5KeyLen)
{
    // Create hex-value string from the MD5 value to keep the string size minimal
    rtl::OStringBuffer aBuffer( _nMD5KeyLen * 2 + 1 );
    for ( sal_uInt32 i = 0; i < _nMD5KeyLen; ++i )
    {
        sal_Int32 nValue = (sal_Int32)_pMD5KeyBuffer[i];
        if (nValue < 16)                         // maximul hex value for 1 byte
            aBuffer.append( static_cast<sal_Int32>(0), static_cast<sal_Int16>(16) /* radix */ );
        aBuffer.append( nValue, 16 /* radix */ );
    }

    return aBuffer.makeStringAndClear();
}


// -----------------------------------------------------------------------------
class init : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void init_000()
        {
            rtlDigest handle = NULL;

            rtlDigestError aError = rtl_digest_init(handle, NULL, 0);

            CPPUNIT_ASSERT_MESSAGE("init(NULL, 0, 0)", aError == rtl_Digest_E_Argument);
        }

    void init_001()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

            rtlDigestError aError = rtl_digest_init(handle, NULL, 0);

            CPPUNIT_ASSERT_MESSAGE("init(handle, 0, 0)", aError == rtl_Digest_E_None);

            rtl_digest_destroy( handle );
        }

    // ------------------------------------
    void init_MD2()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );

            rtl::OString aMsg = sSampleString;
            const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
            sal_uInt32       nSize = ( aMsg.getLength() );

            rtlDigestError aError = rtl_digest_init(handle, pData, nSize);

            CPPUNIT_ASSERT_MESSAGE("init(handle, pData, nSize)", aError == rtl_Digest_E_None);

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

    void init_MD5()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );

            rtl::OString aMsg = sSampleString;
            const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
            sal_uInt32       nSize = ( aMsg.getLength() );

            rtlDigestError aError = rtl_digest_init(handle, pData, nSize);

            CPPUNIT_ASSERT_MESSAGE("init(handle, pData, nSize)", aError == rtl_Digest_E_None);

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

    void init_SHA()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA );

            rtl::OString aMsg = sSampleString;
            const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
            sal_uInt32       nSize = ( aMsg.getLength() );

            rtlDigestError aError = rtl_digest_init(handle, pData, nSize);

            CPPUNIT_ASSERT_MESSAGE("init(handle, pData, nSize)", aError == rtl_Digest_E_None);

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
    void init_SHA1()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );

            rtl::OString aMsg = sSampleString;
            const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
            sal_uInt32       nSize = ( aMsg.getLength() );

            rtlDigestError aError = rtl_digest_init(handle, pData, nSize);

            CPPUNIT_ASSERT_MESSAGE("init(handle, pData, nSize)", aError == rtl_Digest_E_None);

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
    void init_HMAC_MD5()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_MD5 );

            rtl::OString aMsg = sSampleString;
            const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
            sal_uInt32       nSize = ( aMsg.getLength() );

            sal_uInt32     nKeyLen = rtl_digest_queryLength( handle );
            CPPUNIT_ASSERT_MESSAGE( "Keylen must be greater 0", nKeyLen );

            sal_uInt8    *pKeyBuffer = new sal_uInt8[ nKeyLen ];
            CPPUNIT_ASSERT( pKeyBuffer );
            memset(pKeyBuffer, 0, nKeyLen);

            rtlDigestError aError = rtl_digest_init(handle, pKeyBuffer, nKeyLen );

            CPPUNIT_ASSERT_MESSAGE("init(handle, pData, nSize)", aError == rtl_Digest_E_None);

            rtl_digest_update( handle, pData, nSize );

            rtl_digest_get( handle, pKeyBuffer, nKeyLen );
            rtl::OString aSum = createHex(pKeyBuffer, nKeyLen);
            delete [] pKeyBuffer;

            printf("HMAC_MD5 Sum: %s\n", aSum.getStr());
            // LLA: how to check right values
            // samples?

            rtl_digest_destroy( handle );
        }
    void init_HMAC_SHA1()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmHMAC_SHA1 );

            rtl::OString aMsg = sSampleString;
            const sal_uInt8 *pData = (const sal_uInt8*)aMsg.getStr();
            sal_uInt32       nSize = ( aMsg.getLength() );

            sal_uInt32     nKeyLen = rtl_digest_queryLength( handle );
            CPPUNIT_ASSERT_MESSAGE( "Keylen must be greater 0", nKeyLen );

            sal_uInt8    *pKeyBuffer = new sal_uInt8[ nKeyLen ];
            CPPUNIT_ASSERT( pKeyBuffer );
            memset(pKeyBuffer, 0, nKeyLen);

            rtlDigestError aError = rtl_digest_init(handle, pKeyBuffer, nKeyLen );

            CPPUNIT_ASSERT_MESSAGE("init(handle, pData, nSize)", aError == rtl_Digest_E_None);

            rtl_digest_update( handle, pData, nSize );

            rtl_digest_get( handle, pKeyBuffer, nKeyLen );
            rtl::OString aSum = createHex(pKeyBuffer, nKeyLen);
            delete [] pKeyBuffer;

            printf("HMAC_SHA1 Sum: %s\n", aSum.getStr());
            // LLA: how to check right values
            // samples?

            rtl_digest_destroy( handle );
        }


    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(init);
    CPPUNIT_TEST( init_000 );
    CPPUNIT_TEST( init_001 );
    CPPUNIT_TEST( init_MD2 );
    CPPUNIT_TEST( init_MD5 );
    CPPUNIT_TEST( init_SHA );
    CPPUNIT_TEST( init_SHA1 );
    CPPUNIT_TEST( init_HMAC_MD5 );
    CPPUNIT_TEST( init_HMAC_SHA1 );
    CPPUNIT_TEST_SUITE_END();
}; // class init

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

class equalTests : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // ------------------------------------
    void equal_001()
        {
            rtl::OString aMsg1 = sSampleString;
            rtl::OString aMsg2 = sSampleString;

            rtl::OString aMsgMD5Sum1 = getMD5Sum(aMsg1);
            rtl::OString aMsgMD5Sum2 = getMD5Sum(aMsg2);

            CPPUNIT_ASSERT_MESSAGE("md5sum must have a length", aMsgMD5Sum1.getLength() == 32 && aMsgMD5Sum2.getLength() == 32 );
            CPPUNIT_ASSERT_MESSAGE("source is the same, dest must be also the same", aMsgMD5Sum1.equals(aMsgMD5Sum2) == sal_True);
        }
    // ------------------------------------
    void equal_002()
        {
            rtl::OString aMsg1 = sSampleString;
            rtl::OString aMsg2 = sSampleString_only_one_diff;

            rtl::OString aMsgMD5Sum1 = getMD5Sum(aMsg1);
            rtl::OString aMsgMD5Sum2 = getMD5Sum(aMsg2);

            CPPUNIT_ASSERT_MESSAGE("md5sum must have a length", aMsgMD5Sum1.getLength() == 32 && aMsgMD5Sum2.getLength() == 32 );
            CPPUNIT_ASSERT_MESSAGE("differ only in one char", aMsgMD5Sum1.equals(aMsgMD5Sum2) == sal_False);
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(equalTests);
    CPPUNIT_TEST( equal_001 );
    CPPUNIT_TEST( equal_002 );
    CPPUNIT_TEST_SUITE_END();
}; // class create


// -----------------------------------------------------------------------------
class digest_MD2 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // ------------------------------------
    void MD2_001()
        {
            rtl::OString  aMsg1 = sSampleString;

            sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_MD2 ];
            CPPUNIT_ASSERT( pBuffer );
            memset(pBuffer, 0, RTL_DIGEST_LENGTH_MD2 );

            sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
            sal_Int32     nLen  = aMsg1.getLength();

            rtlDigestError aError = rtl_digest_MD2(pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_MD2);

            CPPUNIT_ASSERT(aError == rtl_Digest_E_None );

            rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_MD2);
            printf("Decrypt MD2: %s\n", aStr.getStr());
            CPPUNIT_ASSERT_MESSAGE("checksum of sample string is wrong. Code changes or sample problems, please check.", aStr.equals(sSampleString_MD2) );

            delete [] pBuffer;
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(digest_MD2);
    CPPUNIT_TEST( MD2_001 );
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------
class digest_MD5 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }
    // ------------------------------------
    void MD5_001()
        {
            rtl::OString  aMsg1 = sSampleString;

            sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_MD5 ];
            CPPUNIT_ASSERT( pBuffer );
            memset(pBuffer, 0, RTL_DIGEST_LENGTH_MD5 );

            sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
            sal_Int32     nLen  = aMsg1.getLength();

            rtlDigestError aError = rtl_digest_MD5(pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_MD5);

            CPPUNIT_ASSERT(aError == rtl_Digest_E_None );

            rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_MD5);
            printf("Decrypt MD5: %s\n", aStr.getStr());
            CPPUNIT_ASSERT_MESSAGE("checksum of sample string is wrong. Code changes or sample problems, please check.", aStr.equals(sSampleString_MD5) );

            delete [] pBuffer;
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(digest_MD5);
    CPPUNIT_TEST( MD5_001 );
    CPPUNIT_TEST_SUITE_END();
}; // class create

// -----------------------------------------------------------------------------
class digest_SHA : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // ------------------------------------
    void SHA_001()
        {
            rtl::OString  aMsg1 = sSampleString;

            sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_SHA ];
            CPPUNIT_ASSERT( pBuffer );
            memset(pBuffer, 0, RTL_DIGEST_LENGTH_SHA);

            sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
            sal_Int32     nLen  = aMsg1.getLength();

            rtlDigestError aError = rtl_digest_SHA(pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_SHA);

            CPPUNIT_ASSERT(aError == rtl_Digest_E_None );

            rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_SHA);
            printf("Decrypt SHA: %s\n", aStr.getStr());
            CPPUNIT_ASSERT_MESSAGE("checksum of sample string is wrong. Code changes or sample problems, please check.", aStr.equals(sSampleString_SHA) );

            delete [] pBuffer;
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(digest_SHA);
    CPPUNIT_TEST( SHA_001 );
    CPPUNIT_TEST_SUITE_END();
}; // class create

// -----------------------------------------------------------------------------
class digest_SHA1 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // ------------------------------------
    void SHA1_001()
        {
            rtl::OString  aMsg1 = sSampleString;

            sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_SHA1 ];
            CPPUNIT_ASSERT( pBuffer );
            memset(pBuffer, 0, RTL_DIGEST_LENGTH_SHA1);

            sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
            sal_Int32     nLen  = aMsg1.getLength();

            rtlDigestError aError = rtl_digest_SHA1(pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_SHA1);

            CPPUNIT_ASSERT(aError == rtl_Digest_E_None );

            rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_SHA1);
            printf("Decrypt SHA1: %s\n", aStr.getStr());
            CPPUNIT_ASSERT_MESSAGE("checksum of sample string is wrong. Code changes or sample problems, please check.", aStr.equals(sSampleString_SHA1) );

            delete [] pBuffer;
        }
    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(digest_SHA1);
    CPPUNIT_TEST( SHA1_001 );
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------
class digest_HMAC_MD5 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // ------------------------------------
    void HMAC_MD5_001()
        {
            rtl::OString  aMsg1 = sSampleString;

            sal_uInt8    *pKeyBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_HMAC_MD5 ];
            CPPUNIT_ASSERT( pKeyBuffer );
            memset(pKeyBuffer, 0, RTL_DIGEST_LENGTH_HMAC_MD5);

            sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_HMAC_MD5 ];
            CPPUNIT_ASSERT( pBuffer );
            memset(pBuffer, 0, RTL_DIGEST_LENGTH_HMAC_MD5);

            sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
            sal_Int32     nLen  = aMsg1.getLength();

            rtlDigestError aError = rtl_digest_HMAC_MD5(pKeyBuffer, RTL_DIGEST_LENGTH_HMAC_MD5, pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_HMAC_MD5);

            CPPUNIT_ASSERT(aError == rtl_Digest_E_None );

            rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_HMAC_MD5);
            printf("Decrypt HMAC_MD5: %s\n", aStr.getStr());
            CPPUNIT_ASSERT_MESSAGE("md5sum of sample string is wrong. Code changes or sample problems, please check.", aStr.equals(sSampleString_HMAC_MD5) );

            delete [] pBuffer;
        }
    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(digest_HMAC_MD5);
    CPPUNIT_TEST( HMAC_MD5_001 );
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------
class digest_HMAC_SHA1 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // ------------------------------------
    void HMAC_SHA1_001()
        {
            rtl::OString  aMsg1 = sSampleString;

            sal_uInt8    *pKeyBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_HMAC_SHA1 ];
            CPPUNIT_ASSERT( pKeyBuffer );
            memset(pKeyBuffer, 0, RTL_DIGEST_LENGTH_HMAC_SHA1);

            sal_uInt8    *pBuffer = new sal_uInt8[ RTL_DIGEST_LENGTH_HMAC_SHA1 ];
            CPPUNIT_ASSERT( pBuffer );
            memset(pBuffer, 0, RTL_DIGEST_LENGTH_HMAC_SHA1);

            sal_uInt8    *pMsg1 = (sal_uInt8*)aMsg1.getStr();
            sal_Int32     nLen  = aMsg1.getLength();

            rtlDigestError aError = rtl_digest_HMAC_SHA1(pKeyBuffer, RTL_DIGEST_LENGTH_HMAC_SHA1, pMsg1, nLen, pBuffer, RTL_DIGEST_LENGTH_HMAC_SHA1);

            CPPUNIT_ASSERT(aError == rtl_Digest_E_None );

            rtl::OString aStr = createHex(pBuffer, RTL_DIGEST_LENGTH_HMAC_SHA1);
            printf("Decrypt HMAC_SHA1: %s\n", aStr.getStr());
            CPPUNIT_ASSERT_MESSAGE("md5sum of sample string is wrong. Code changes or sample problems, please check.", aStr.equals(sSampleString_HMAC_SHA1) );

            delete [] pBuffer;
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(digest_HMAC_SHA1);
    CPPUNIT_TEST( HMAC_SHA1_001 );
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------
class digest_PBKDF2 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // ------------------------------------
    rtl::OString /* key */ run_check_PBKDF2(rtl::OString const& _sPassword, bool _bClearSalt, sal_uInt32 _nCount)
        {
            sal_uInt32   nKeyLen = RTL_DIGEST_LENGTH_HMAC_SHA1;
            sal_uInt8    *pKeyBuffer = new sal_uInt8[ nKeyLen ];
            CPPUNIT_ASSERT( pKeyBuffer );
            memset(pKeyBuffer, 0, nKeyLen);

            sal_uInt8    *pPassword    = (sal_uInt8*)_sPassword.getStr();
            sal_Int32     nPasswordLen = _sPassword.getLength();

            sal_uInt32   nSaltDataLen = RTL_DIGEST_LENGTH_HMAC_SHA1;
            sal_uInt8    *pSaltData = new sal_uInt8[ nSaltDataLen ];
            CPPUNIT_ASSERT( pSaltData );
            memset(pSaltData, 0, nSaltDataLen);

            if (! _bClearSalt)
            {
                // wilful contamination
                pSaltData[0] = 1;
            }

            rtlDigestError aError = rtl_digest_PBKDF2(pKeyBuffer, nKeyLen, pPassword, nPasswordLen, pSaltData, nSaltDataLen, _nCount);

            CPPUNIT_ASSERT(aError == rtl_Digest_E_None );

            rtl::OString aKey = createHex(pKeyBuffer, nKeyLen);
            printf("Key: %s\n", aKey.getStr());

            // rtl::OString sSalt = createHex(pSaltData, nSaltDataLen);
            // printf("Salt: %s\n", sSalt.getStr());

            // CPPUNIT_ASSERT_MESSAGE("md5sum of sample string is wrong. Code changes or sample problems, please check.", aStr.equals(sSampleString_PBKDF2) );

            delete [] pSaltData;
            delete [] pKeyBuffer;
            return aKey;
        }

    void PBKDF2_001()
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
    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(digest_PBKDF2);
    CPPUNIT_TEST( PBKDF2_001 );
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------

class update : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void update_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_update(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }

    void updateMD2_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_updateMD2(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }

    void updateMD2_001()
        {
            rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != 0);

            rtl::OString aMsg = sSampleString;
            const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();

            rtlDigestError aError = rtl_digest_updateMD2(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("handle parameter 'pData' wrong", aError == rtl_Digest_E_Argument );

            /* rtlDigestError */ aError = rtl_digest_updateMD2(aHandle, pData, 0);
            CPPUNIT_ASSERT_MESSAGE("handle parameter 'nSize' wrong", aError == rtl_Digest_E_None );

            rtl_digest_destroyMD2(aHandle);
        }
    void updateMD5_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_updateMD5(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }

    void updateMD5_001()
        {
            // use wrong Algorithm!!! This is volitional!
            rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != 0);

            rtl::OString aMsg = sSampleString;
            const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();
            sal_uInt32       nSize = ( aMsg.getLength() );

            rtlDigestError aError = rtl_digest_updateMD5(aHandle, pData, nSize);
            CPPUNIT_ASSERT_MESSAGE("handle parameter 'handle' wrong", aError == rtl_Digest_E_Algorithm );

            rtl_digest_destroyMD5(aHandle);
        }

    void updateMD5_002()
        {
            rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD5", aHandle != 0);

            rtl::OString aMsg = sSampleString;
            const sal_uInt8* pData = (const sal_uInt8*)aMsg.getStr();

            rtlDigestError aError = rtl_digest_updateMD5(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("handle parameter 'pData' wrong", aError == rtl_Digest_E_Argument );

            /* rtlDigestError */ aError = rtl_digest_updateMD5(aHandle, pData, 0);
            CPPUNIT_ASSERT_MESSAGE("handle parameter 'nSize' wrong", aError == rtl_Digest_E_None );

            rtl_digest_destroyMD5(aHandle);
        }

    void updateSHA_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_updateSHA(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }

    void updateSHA1_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_updateSHA1(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }

    void updateHMAC_MD5_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_updateHMAC_MD5(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }

    void updateHMAC_SHA1_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_updateHMAC_SHA1(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(update);
    CPPUNIT_TEST(update_000);
    CPPUNIT_TEST(updateMD5_000);
    CPPUNIT_TEST(updateMD5_001);
    CPPUNIT_TEST(updateMD5_002);
    CPPUNIT_TEST(updateMD5_000);
    CPPUNIT_TEST(updateSHA_000);
    CPPUNIT_TEST(updateSHA1_000);
    CPPUNIT_TEST(updateHMAC_MD5_000);
    CPPUNIT_TEST(updateHMAC_SHA1_000);
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------

class get : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void get_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_get(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }
    void getMD5_000()
        {
            rtlDigest aHandle = NULL;
            rtlDigestError aError = rtl_digest_getMD5(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("does not handle wrong parameter", aError == rtl_Digest_E_Argument );
        }
    void getMD5_001()
        {
            // test with wrong algorithm
            rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD2 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD2", aHandle != 0);

            sal_uInt32     nKeyLen = rtl_digest_queryLength( aHandle );
            sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];

            rtlDigestError aError = rtl_digest_getMD5(aHandle, NULL, 0);
            CPPUNIT_ASSERT_MESSAGE("handle 2. parameter wrong", aError == rtl_Digest_E_Argument );

            /* rtlDigestError */ aError = rtl_digest_getMD5(aHandle, pKeyBuffer, 0);
            CPPUNIT_ASSERT_MESSAGE("handle parameter 'handle' wrong", aError == rtl_Digest_E_Algorithm );

            rtl_digest_destroyMD2(aHandle);
        }

    void getMD5_002()
        {
            rtlDigest aHandle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD5", aHandle != 0);

            sal_uInt32     nKeyLen = rtl_digest_queryLength( aHandle );
            sal_uInt8     *pKeyBuffer = new sal_uInt8[ nKeyLen ];

            rtlDigestError aError = rtl_digest_getMD5(aHandle, NULL /* pKeyBuffer */ , nKeyLen);
            CPPUNIT_ASSERT_MESSAGE("handle parameter 'pData' wrong", aError == rtl_Digest_E_Argument );

            /* rtlDigestError */ aError = rtl_digest_getMD5(aHandle, pKeyBuffer, 0);
            CPPUNIT_ASSERT_MESSAGE("handle parameter 'nSize' wrong", aError == rtl_Digest_E_BufferSize );

            rtl_digest_destroyMD5(aHandle);
            delete [] pKeyBuffer;
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(get);
    CPPUNIT_TEST(get_000);
    CPPUNIT_TEST(getMD5_000);
    CPPUNIT_TEST(getMD5_001);
    CPPUNIT_TEST(getMD5_002);
    CPPUNIT_TEST_SUITE_END();
}; // class create

// -----------------------------------------------------------------------------
class destroy : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void destroy_001()
        {
            rtlDigest handle = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
            CPPUNIT_ASSERT_MESSAGE("create with rtl_Digest_AlgorithmMD5", handle != 0);

            // not really testable
            // LLA: good will test.
            rtl_digest_destroy( handle );
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(destroy);
    CPPUNIT_TEST(destroy_001);
    CPPUNIT_TEST_SUITE_END();
}; // class create
// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::create);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::createMD2);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::createMD5);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::createSHA);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::createSHA1);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::createHMAC_MD5);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::createHMAC_SHA1);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::destroy);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::queryAlgorithm);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::queryLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::init);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::equalTests);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::digest_MD2);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::digest_MD5);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::digest_SHA);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::digest_SHA1);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::digest_HMAC_MD5);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::digest_HMAC_SHA1);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::digest_PBKDF2);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::update);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::get);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_digest::destroy);
} // namespace rtl_digest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
