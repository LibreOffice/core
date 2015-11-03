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

#include <rtl/ref.hxx>
#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace rtl_ref
{

class MoveTestClass
{
private:
    bool m_bIncFlag;
    long m_nRef;
public:
    MoveTestClass(): m_bIncFlag(false), m_nRef(0) { }

    // There should never be more than two references to this class as it
    // is used as a test class for move functions. One reference being the
    // original reference and the second being the test reference
    void acquire()
    {
        if(m_bIncFlag)
        {
            ++m_nRef;
            m_bIncFlag = false;
        }
        else
            CPPUNIT_FAIL("RC was incremented when in should not have been");
    }

    void release() { --m_nRef; }

    long use_count() { return m_nRef; }

    void set_inc_flag() { m_bIncFlag = true; }
};

rtl::Reference< MoveTestClass > get_reference( MoveTestClass* pcTestClass )
{
    // constructor will increment the reference count
    pcTestClass->set_inc_flag();
    rtl::Reference< MoveTestClass > tmp(pcTestClass);
    return tmp;
}

class TestReferenceRefCounting : public CppUnit::TestFixture
{
    void testMove()
    {
        MoveTestClass cTestClass;

        // constructor will increment the reference count
        cTestClass.set_inc_flag();
        rtl::Reference< MoveTestClass > test1( &cTestClass );

        // move should not increment the reference count
        rtl::Reference< MoveTestClass > test2( std::move(test1) );
        CPPUNIT_ASSERT_MESSAGE("test2.use_count() == 1",
                               test2->use_count() == 1);

        // test1 now contains a null pointer
        CPPUNIT_ASSERT_MESSAGE("!test1.is()",
                               !test1.is());

        // function return should move the reference
        test2 = get_reference( &cTestClass );
        CPPUNIT_ASSERT_MESSAGE("test2.use_count() == 1",
                               test2->use_count() == 1);

        // normal copy
        test2->set_inc_flag();
        test1 = test2;
        CPPUNIT_ASSERT_MESSAGE("test2.use_count() == 2",
                               test2->use_count() == 2);

        // use count should decrement
        test2 = rtl::Reference< MoveTestClass >(nullptr);
        CPPUNIT_ASSERT_MESSAGE("test1.use_count() == 1",
                               test1->use_count() == 1);

        // move of a null pointer should not cause an error
        test1 = std::move(test2);

        CPPUNIT_ASSERT_MESSAGE("!test1.is()",
                               !test1.is());
        CPPUNIT_ASSERT_MESSAGE("!test2.is()",
                               !test2.is());

        CPPUNIT_ASSERT_MESSAGE("cTestClass.use_count() == 0",
                               cTestClass.use_count() == 0);
    }

    CPPUNIT_TEST_SUITE(TestReferenceRefCounting);
    CPPUNIT_TEST(testMove);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace rtl_ref
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ref::TestReferenceRefCounting);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
