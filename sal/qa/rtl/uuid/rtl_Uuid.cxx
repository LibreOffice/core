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


#include <math.h>
#include <stdio.h>
#include <string.h>

#include <rtl/uuid.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#ifdef UNX
#include <unistd.h>
#include <time.h>
#endif

#define t_print printf

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;
/** print a UNI_CODE String. And also print some comments of the string.
*/
inline void printUString( const ::rtl::OUString & str, const sal_Char * msg = "" )
{
    t_print("#%s #printUString_u# ", msg );
    rtl::OString aString;
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", (char *)aString.getStr( ) );
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
class createUuid : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

#define TEST_UUID 20
    void createUuid_001()
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
    CPPUNIT_ASSERT_MESSAGE("createUuid: every uuid must be different.", bRes == sal_True );
    }
   /*
    void createUuid_002()
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
    CPPUNIT_ASSERT_MESSAGE("createUuid: every uuid must be different.", bRes == sal_True );
    }*/

    CPPUNIT_TEST_SUITE(createUuid);
    CPPUNIT_TEST(createUuid_001);
    //CPPUNIT_TEST(createUuid_002);
    CPPUNIT_TEST_SUITE_END();
}; // class createUuid

namespace ThreadHelper
{
     void thread_sleep(sal_Int32 _nSec)
    {
#ifdef WNT      //Windows
        Sleep(_nSec * 10 );
#endif
#if ( defined UNX )
        sleep( _nSec );
#endif
    }
}

class createNamedUuid : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void createNamedUuid_001()
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
        CPPUNIT_ASSERT_MESSAGE( "Same name should generate the same uuid", ! memcmp( pNamedUUID , pNamedUUID2 , 16 ) && rtl_compareUuid( pNamedUUID , pNamedUUID2 ) == 0 );
        CPPUNIT_ASSERT_MESSAGE( "Same name should generate the same uuid", ! memcmp( pNamedUUID  , pPriorCalculatedUUID , 16 ) );

        // Different names does not generate the same uuid
        rtl_string_newFromStr( &pName , "this is a bla.blubs.DNS-Namf" );
        rtl_createNamedUuid( pNamedUUID2 , NameSpace_DNS , pName );
        CPPUNIT_ASSERT_MESSAGE("Different names does not generate the same uuid.", memcmp( pNamedUUID , pNamedUUID2 , 16 ) );

        // the same name with different namespace uuid produces different uuids
        rtl_createNamedUuid( pNamedUUID , NameSpace_URL , pName );
        CPPUNIT_ASSERT_MESSAGE( " same name with different namespace uuid produces different uuids", memcmp( pNamedUUID , pNamedUUID2 , 16 ) && rtl_compareUuid( pNamedUUID , pNamedUUID2 ) != 0);

        //test compareUuid
        if ( rtl_compareUuid( pNamedUUID , pNamedUUID2 ) > 0 )
        {   CPPUNIT_ASSERT_MESSAGE( " compare uuids", rtl_compareUuid( pNamedUUID2 , pNamedUUID ) < 0);
        }
        else
            CPPUNIT_ASSERT_MESSAGE( " compare uuids", rtl_compareUuid( pNamedUUID2 , pNamedUUID ) > 0);

        rtl_string_release( pName );
    }

    CPPUNIT_TEST_SUITE(createNamedUuid);
    CPPUNIT_TEST(createNamedUuid_001);
    CPPUNIT_TEST_SUITE_END();
}; // class createNamedUuid

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_Uuid::createUuid);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_Uuid::createNamedUuid);
} // namespace rtl_Uuid

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
