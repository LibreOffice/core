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

#include <string.h>

#include <rtl/uuid.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif


namespace rtl_Uuid
{
class createUuid : public CppUnit::TestFixture
{
public:
#define TEST_UUID 20
    void createUuid_001()
    {
    sal_uInt8 aNode[TEST_UUID][16];
    sal_Int32 i,i2;
    for( i = 0 ; i < TEST_UUID ; i ++ )
    {
        rtl_createUuid( aNode[i], nullptr, false );
    }
    bool bRes = true;
    for( i = 0 ; i < TEST_UUID ; i ++ )
    {
        for( i2 = i+1 ; i2 < TEST_UUID ; i2 ++ )
        {
            if ( rtl_compareUuid( aNode[i] , aNode[i2] ) == 0  )
            {
                bRes = false;
                break;
            }
        }
        if ( !bRes )
            break;
    }
    CPPUNIT_ASSERT_MESSAGE("createUuid: every uuid must be different.", bRes);
    }
   /*
    void createUuid_002()
    {
    sal_uInt8 pNode[16];
    sal_uInt8 aNode[TEST_UUID][16];
    sal_Int32 i,i2;
    for( i = 0 ; i < TEST_UUID ; i ++ )
    {
        rtl_createUuid( aNode[i], pNode, true );
    }
    bool bRes = true;
    for( i = 0 ; i < TEST_UUID ; i ++ )
    {
        //printUuid( aNode[i] );
        for( i2 = i+1 ; i2 < TEST_UUID ; i2 ++ )
        {
            if ( rtl_compareUuid( aNode[i] , aNode[i2] ) == 0  )
            {
                bRes = false;
                break;
            }
        }
        if ( bRes == false )
            break;
    }
    CPPUNIT_ASSERT_MESSAGE("createUuid: every uuid must be different.", bRes == true );
    }*/

    CPPUNIT_TEST_SUITE(createUuid);
    CPPUNIT_TEST(createUuid_001);
    //CPPUNIT_TEST(createUuid_002);
    CPPUNIT_TEST_SUITE_END();
}; // class createUuid

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_Uuid::createUuid);
} // namespace rtl_Uuid

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
