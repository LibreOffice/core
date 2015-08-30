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
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include "cow_wrapper_clients.hxx"

using namespace ::o3tl;
using namespace ::o3tltests;


class cow_wrapper_test : public CppUnit::TestFixture
{
public:
    template< class T > void test( T& rTestObj1, T& rTestObj2, T& rTestObj3 )
    {
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 is unique",
                               rTestObj1.is_unique() );
        CPPUNIT_ASSERT_MESSAGE("rTestObj2 is unique",
                               rTestObj2.is_unique() );
        CPPUNIT_ASSERT_MESSAGE("rTestObj3 is unique",
                               rTestObj3.is_unique() );

        CPPUNIT_ASSERT_MESSAGE("rTestObj1 != rTestObj2",
                               rTestObj1 != rTestObj2 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj2 != rTestObj3",
                               rTestObj2 != rTestObj3 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 != rTestObj3",
                               rTestObj1 != rTestObj3 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 < rTestObj2",
                               rTestObj1 < rTestObj2 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj2 < rTestObj3",
                               rTestObj2 < rTestObj3 );

        rTestObj2 = rTestObj1;
        rTestObj3 = rTestObj1;
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 == rTestObj2",
                               rTestObj1 == rTestObj2 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 == rTestObj3",
                               rTestObj1 == rTestObj3 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1.use_count() == 3",
                               rTestObj1.use_count() == 3 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj2.use_count() == 3",
                               rTestObj2.use_count() == 3 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj3.use_count() == 3",
                               rTestObj3.use_count() == 3 );

        rTestObj2.makeUnique();
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 == rTestObj2",
                               rTestObj1 == rTestObj2 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 == rTestObj3",
                               rTestObj1 == rTestObj3 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1.use_count() == 2",
                               rTestObj1.use_count() == 2 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj2.use_count() == 1",
                               rTestObj2.use_count() == 1 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj2.is_unique()",
                               rTestObj2.is_unique() );
        CPPUNIT_ASSERT_MESSAGE("rTestObj3.use_count() == 2",
                               rTestObj3.use_count() == 2 );

        rTestObj2.swap( rTestObj3 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 == rTestObj2",
                               rTestObj1 == rTestObj2 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1 == rTestObj3",
                               rTestObj1 == rTestObj3 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj1.use_count() == 2",
                               rTestObj1.use_count() == 2 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj2.use_count() == 2",
                               rTestObj2.use_count() == 2 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj3.use_count() == 1",
                               rTestObj3.use_count() == 1 );
        CPPUNIT_ASSERT_MESSAGE("rTestObj3.is_unique()",
                               rTestObj3.is_unique() );
    }

    void testCowWrapper()
    {
        // setup
        cow_wrapper_client1 aTestObj1;
        cow_wrapper_client1 aTestObj2;
        cow_wrapper_client1 aTestObj3;

        cow_wrapper_client2 aTestObj4;
        cow_wrapper_client2 aTestObj5;
        cow_wrapper_client2 aTestObj6;

        cow_wrapper_client3 aTestObj7;
        cow_wrapper_client3 aTestObj8;
        cow_wrapper_client3 aTestObj9;

        {
            aTestObj1 = cow_wrapper_client1( 1 );
            CPPUNIT_ASSERT_EQUAL(aTestObj1.queryUnmodified(), 1);
            aTestObj2.modify( 2 );
            CPPUNIT_ASSERT_EQUAL(aTestObj2.queryUnmodified(), 2);
            aTestObj3.modify( 3 );
            CPPUNIT_ASSERT_EQUAL(aTestObj3.queryUnmodified(), 3);

            aTestObj4 = cow_wrapper_client2( 4 );
            CPPUNIT_ASSERT_EQUAL(aTestObj4.queryUnmodified(), 4);
            aTestObj5.modify( 5 );
            CPPUNIT_ASSERT_EQUAL(aTestObj5.queryUnmodified(), 5);
            aTestObj6.modify( 6 );
            CPPUNIT_ASSERT_EQUAL(aTestObj6.queryUnmodified(), 6);

            aTestObj7 = cow_wrapper_client3( 7 );
            CPPUNIT_ASSERT_EQUAL(aTestObj7.queryUnmodified(), 7);
            aTestObj8.modify( 8 );
            CPPUNIT_ASSERT_EQUAL(aTestObj8.queryUnmodified(), 8);
            aTestObj9.modify( 9 );
            CPPUNIT_ASSERT_EQUAL(aTestObj9.queryUnmodified(), 9);
        }
        // all three temporaries are dead now

        // test
        test( aTestObj1, aTestObj2, aTestObj3 );
        test( aTestObj4, aTestObj5, aTestObj6 );
        test( aTestObj7, aTestObj8, aTestObj9 );
    }

    void testStaticDefault()
    {
        cow_wrapper_client4 aTestObj1;
        cow_wrapper_client4 aTestObj2;
        cow_wrapper_client4 aTestObj3(4);

        CPPUNIT_ASSERT_MESSAGE("aTestObj1.is_default()",
                               aTestObj1.is_default() );
        CPPUNIT_ASSERT_MESSAGE("aTestObj2.is_default()",
                               aTestObj2.is_default() );
        CPPUNIT_ASSERT_MESSAGE("!aTestObj3.is_default()",
                               !aTestObj3.is_default() );
        aTestObj1 = aTestObj2;
        CPPUNIT_ASSERT_MESSAGE("aTestObj1.is_default() #2",
                               aTestObj1.is_default() );
        CPPUNIT_ASSERT_MESSAGE("aTestObj2.is_default() #2",
                               aTestObj2.is_default() );
        aTestObj1 = aTestObj3;
        CPPUNIT_ASSERT_MESSAGE("!aTestObj1.is_default()",
                               !aTestObj1.is_default() );
    }

    void testRefCounting()
    {
        // add scope to ensure appropriate number of calls to
        // the reference counting policy have been made
        {
            // if any incrementing/decrementing occurs a failure
            // will occur
            cow_wrapper_client5 aTestObj1(1);
            cow_wrapper_client5 aTestObj2( std::move( aTestObj1 ) );
            CPPUNIT_ASSERT_MESSAGE("aTestObj2.use_count() == 1",
                                   aTestObj2.use_count() == 1 );

            // the following should increment
            BogusRefCountPolicy::s_bShouldIncrement = true;
            cow_wrapper_client5 aTestObj3( aTestObj2 );
            CPPUNIT_ASSERT_MESSAGE("s_bShouldIncrement == 0",
                                   !BogusRefCountPolicy::s_bShouldIncrement );

            CPPUNIT_ASSERT_MESSAGE("aTestObj3.use_count() == 2",
                                   aTestObj3.use_count() == 2 );
            {
                cow_wrapper_client5 aTestObj4;
                // the following should decrement the lvalue and then increment the rvalue
                BogusRefCountPolicy::s_bShouldIncrement = true;
                BogusRefCountPolicy::s_bShouldDecrement = true;
                aTestObj4 = aTestObj2;
                CPPUNIT_ASSERT_MESSAGE("s_bShouldIncrement == 0",
                                       !BogusRefCountPolicy::s_bShouldIncrement );
                CPPUNIT_ASSERT_MESSAGE("s_bShouldDecrement == 0",
                                       !BogusRefCountPolicy::s_bShouldDecrement );

                CPPUNIT_ASSERT_MESSAGE("aTestObj2.use_count() == 3",
                                       aTestObj2.use_count() == 3 );
                CPPUNIT_ASSERT_MESSAGE("aTestObj3.use_count() == 3",
                                       aTestObj3.use_count() == 3 );
                CPPUNIT_ASSERT_MESSAGE("aTestObj4.use_count() == 3",
                                       aTestObj4.use_count() == 3 );
                CPPUNIT_ASSERT_MESSAGE("aTestObj2 == aTestObj3",
                                       aTestObj2 == aTestObj3 );
                CPPUNIT_ASSERT_MESSAGE("aTestObj3 == aTestObj4",
                                       aTestObj3 == aTestObj4 );
                CPPUNIT_ASSERT_MESSAGE("aTestObj2 == aTestObj4",
                                       aTestObj2 == aTestObj4 );

                // only decrement the lvalue before assignment
                BogusRefCountPolicy::s_bShouldDecrement = true;
                aTestObj4 = cow_wrapper_client5( 4 );
                CPPUNIT_ASSERT_MESSAGE("s_bShouldIncrement == 0",
                                       !BogusRefCountPolicy::s_bShouldIncrement );

                // only one call should be made to the ref counting policy's
                // decrementing function at the end of the scope
                BogusRefCountPolicy::s_bShouldDecrement = true;
            }
            CPPUNIT_ASSERT_MESSAGE("s_bShouldDecrement == 0",
                                   !BogusRefCountPolicy::s_bShouldDecrement );

            // self assignment
            // aTestObj2 is defunct afterwards, one decrement happens
            BogusRefCountPolicy::s_bShouldDecrement = true;
            aTestObj3 = std::move( aTestObj2 );
            CPPUNIT_ASSERT_MESSAGE("aTestObj2.use_count() == 0",
                                   aTestObj2.use_count() == 0 );
            CPPUNIT_ASSERT_MESSAGE("aTestObj3.use_count() == 1",
                                   aTestObj3.use_count() == 1 );

            cow_wrapper_client5 aTestObj5;

            // only decrement the lvalue before assignment
            BogusRefCountPolicy::s_bShouldDecrement = true;
            aTestObj3 = std::move( aTestObj5 );
            CPPUNIT_ASSERT_MESSAGE("s_bShouldDecrement == 0",
                                   !BogusRefCountPolicy::s_bShouldDecrement);

            // one call should be made to the ref-counting policy's
            // decrementing function at the end of the scope. Only
            // aTestObj3 still holds a valid instance
            BogusRefCountPolicy::s_nEndOfScope = 1;
        }
        CPPUNIT_ASSERT_MESSAGE("s_EndOfScope == 0",
                               BogusRefCountPolicy::s_nEndOfScope == 0 );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(cow_wrapper_test);
    CPPUNIT_TEST(testCowWrapper);
    CPPUNIT_TEST(testStaticDefault);
    CPPUNIT_TEST(testRefCounting);
    CPPUNIT_TEST_SUITE_END();
};


CPPUNIT_TEST_SUITE_REGISTRATION(cow_wrapper_test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
