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

#include <math.h>
#include <stdio.h>

#include <rtl/uuid.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

#ifdef WNT
#define WIN32_LEAN_AND_MEAN
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#endif

#ifdef UNX
#include <unistd.h>
#include <time.h>
#endif

#include "gtest/gtest.h"

using namespace rtl;

/** print a UNI_CODE String. And also print some comments of the string.
*/
inline void printUString( const ::rtl::OUString & str, const sal_Char * msg = "" )
{
    printf("#%s #printUString_u# ", msg );
    rtl::OString aString;
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", (char *)aString.getStr( ) );
}

/************************************************************************
 * For diagnostics( from sal/test/testuuid.cxx )
 ************************************************************************/

void printUuid( sal_uInt8 *pNode )
{
    for( sal_Int32 i1 = 0 ; i1 < 4 ; i1++ )
    {
        for( sal_Int32 i2 = 0 ; i2 < 4 ; i2++ )
        {
            printf( "%02x" , pNode[i1*4 +i2] );
        }
        if( i1 == 3 )
            break;
        printf( "-" );
    }

    printf( "\n# " );
}

namespace rtl_Uuid
{
class createUuid : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }

}; // class createUuid

#define TEST_UUID 20
TEST_F(createUuid, createUuid_001)
{
    sal_uInt8 aNode[TEST_UUID][16];
    sal_Int32 i,i2;
    for( i = 0 ; i < TEST_UUID ; i ++ )
    {
        rtl_createUuid( aNode[i], 0, sal_False );
    }
    sal_Bool bRes = sal_True;
    for( i = 0 ; i < TEST_UUID ; i ++ )
    {
        for( i2 = i+1 ; i2 < TEST_UUID ; i2 ++ )
        {
            if ( rtl_compareUuid( aNode[i] , aNode[i2] ) == 0  )
            {
                bRes = sal_False;
                break;
            }
        }
        if ( bRes == sal_False )
            break;
    }
    ASSERT_TRUE(bRes == sal_True) << "createUuid: every uuid must be different.";
}

/*
TEST_F(createUuid, createUuid_002)
{
    sal_uInt8 pNode[16];
    sal_uInt8 aNode[TEST_UUID][16];
    sal_Int32 i,i2;
    for( i = 0 ; i < TEST_UUID ; i ++ )
    {
        rtl_createUuid( aNode[i], pNode, sal_True );
    }
    sal_Bool bRes = sal_True;
    for( i = 0 ; i < TEST_UUID ; i ++ )
    {
        //printUuid( aNode[i] );
        for( i2 = i+1 ; i2 < TEST_UUID ; i2 ++ )
        {
            if ( rtl_compareUuid( aNode[i] , aNode[i2] ) == 0  )
            {
                bRes = sal_False;
                break;
            }
        }
        if ( bRes == sal_False )
            break;
    }
    ASSERT_TRUE(bRes == sal_True) << "createUuid: every uuid must be different.";
}*/

namespace ThreadHelper
{
     void thread_sleep(sal_Int32 _nSec)
    {
#ifdef WNT      //Windows
        Sleep(_nSec * 10 );
#endif
#if ( defined UNX ) || ( defined OS2 )  //Unix
        sleep( _nSec );
#endif
    }
}

class createNamedUuid : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class createNamedUuid

TEST_F(createNamedUuid, createNamedUuid_001)
{
    sal_uInt8 NameSpace_DNS[16] = RTL_UUID_NAMESPACE_DNS;
    sal_uInt8 NameSpace_URL[16] = RTL_UUID_NAMESPACE_URL;
    sal_uInt8 pPriorCalculatedUUID[16] = {
        0x52,0xc9,0x30,0xa5,
        0xd1,0x61,0x3b,0x16,
        0x98,0xc5,0xf8,0xd1,
        0x10,0x10,0x6d,0x4d };

    sal_uInt8 pNamedUUID[16], pNamedUUID2[16];

    // Same name does generate the same uuid
    rtl_String *pName = 0;
    rtl_string_newFromStr( &pName , "this is a bla.blubs.DNS-Name" );
    rtl_createNamedUuid( pNamedUUID , NameSpace_DNS , pName );
    rtl_createNamedUuid( pNamedUUID2 , NameSpace_DNS , pName );
    ASSERT_TRUE(! memcmp( pNamedUUID , pNamedUUID2 , 16 ) && rtl_compareUuid( pNamedUUID , pNamedUUID2 ) == 0)
        <<  "Same name should generate the same uuid";
    ASSERT_TRUE(! memcmp( pNamedUUID  , pPriorCalculatedUUID , 16 )) <<  "Same name should generate the same uuid";

    // Different names does not generate the same uuid
    rtl_string_newFromStr( &pName , "this is a bla.blubs.DNS-Namf" );
    rtl_createNamedUuid( pNamedUUID2 , NameSpace_DNS , pName );
    ASSERT_TRUE(memcmp( pNamedUUID , pNamedUUID2 , 16 )) << "Different names does not generate the same uuid.";

    // the same name with different namespace uuid produces different uuids
    rtl_createNamedUuid( pNamedUUID , NameSpace_URL , pName );
    ASSERT_TRUE(memcmp( pNamedUUID , pNamedUUID2 , 16 ) && rtl_compareUuid( pNamedUUID , pNamedUUID2 ) != 0)
        <<  " same name with different namespace uuid produces different uuids";

    //test compareUuid
    if ( rtl_compareUuid( pNamedUUID , pNamedUUID2 ) > 0 )
    {   ASSERT_TRUE(rtl_compareUuid( pNamedUUID2 , pNamedUUID ) < 0) <<  " compare uuids";
    }
    else
        ASSERT_TRUE(rtl_compareUuid( pNamedUUID2 , pNamedUUID ) > 0) <<  " compare uuids";

    rtl_string_release( pName );
}

} // namespace rtl_Uuid

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
