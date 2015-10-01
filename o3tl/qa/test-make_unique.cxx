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

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <sal/types.h>
#include <memory>
#include <type_traits>
#include <o3tl/make_unique.hxx>

class make_unique_test : public CppUnit::TestFixture
{
public:
    void testMakeUnique();
    // These macros are needed by auto register mechanism.
    CPPUNIT_TEST_SUITE(make_unique_test);
    CPPUNIT_TEST(testMakeUnique);
    CPPUNIT_TEST_SUITE_END();
}; // class make_unique_test

// simple test to ensure the appropriate specialization of
// make_unique is called
void make_unique_test::testMakeUnique() {
    std::unique_ptr< sal_Int32 > test1 = ::o3tl::make_unique< sal_Int32 >( 4 );

    std::unique_ptr< sal_uInt32 > test2 = ::o3tl::make_unique< sal_uInt32 >( 4 );

    std::unique_ptr< sal_Int32[] > test3 = ::o3tl::make_unique< sal_Int32[] >( 10 );

    for( sal_Int32 i = 0; i < 10; ++i ) {
        test3[i] = i;
    }
    for( sal_Int32 i = 0; i < 10; ++i ) {
        CPPUNIT_ASSERT_MESSAGE( "Values in array do not match expected value",
                                i == test3[i] );
    }

    {
        std::unique_ptr< sal_uInt32[] > test4 = ::o3tl::make_unique< sal_uInt32[] >( 10 );

        for( sal_Int32 i = 0; i < 10; ++i ) {
            test4[i] = i;
        }
        for( sal_uInt32 i = 0; i < 10; ++i ) {
            std::cout << test4[i] << std::endl;
            CPPUNIT_ASSERT_MESSAGE( "Values in array do not match expected value",
                                    i == test4[i] );
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(make_unique_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
