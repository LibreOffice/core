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

#include <o3tl/nonstaticstring.hxx>
#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "../../../rtl/strimp.hxx"

namespace rtl_alloc
{

    // small memory check routine, which return false, if there is a problem

    static bool checkMemory(const char* _pMemory, sal_uInt32 _nSize, char _n)
    {
        bool bOk = true;

        for (sal_uInt32 i=0;i<_nSize;i++)
        {
            if (_pMemory[i] != _n)
            {
                bOk = false;
            }
        }
        return bOk;
    }

class Memory : public CppUnit::TestFixture
{
    // for normal alloc functions
    char       *m_pMemory;
    static const sal_uInt32  m_nSizeOfMemory = 1024;

public:
    Memory()
        : m_pMemory(nullptr)
    {
    }

    // initialise your test code values here.
    void setUp() override
    {
        m_pMemory = static_cast<char*>(rtl_allocateMemory( m_nSizeOfMemory ));
    }

    void tearDown() override
    {
        rtl_freeMemory(m_pMemory);
        m_pMemory = nullptr;
    }

    void rtl_allocateMemory_001()
    {
        CPPUNIT_ASSERT_MESSAGE( "Can get zero memory.", m_pMemory != nullptr);
        memset(m_pMemory, 1, m_nSizeOfMemory);
        CPPUNIT_ASSERT_MESSAGE( "memory contains wrong value.", checkMemory(m_pMemory, m_nSizeOfMemory, 1));
    }

    void rtl_reallocateMemory_001()
    {
        sal_uInt32 nSize = 2 * 1024;
        m_pMemory = static_cast<char*>(rtl_reallocateMemory(m_pMemory, nSize));

        CPPUNIT_ASSERT_MESSAGE( "Can reallocate memory.", m_pMemory != nullptr);
        memset(m_pMemory, 2, nSize);
        CPPUNIT_ASSERT_MESSAGE( "memory contains wrong value.", checkMemory(m_pMemory, nSize, 2));
    }

    CPPUNIT_TEST_SUITE(Memory);
    CPPUNIT_TEST(rtl_allocateMemory_001);
    CPPUNIT_TEST(rtl_reallocateMemory_001);
    CPPUNIT_TEST_SUITE_END();
}; // class test

class TestZeroMemory : public CppUnit::TestFixture
{
    // for zero functions
    char       *m_pZeroMemory;
    static const sal_uInt32  m_nSizeOfZeroMemory = 50 * 1024 * 1024;

public:
    TestZeroMemory()
        : m_pZeroMemory(nullptr)
    {
    }

    // initialise your test code values here.
    void setUp() override
    {
        m_pZeroMemory = static_cast<char*>(rtl_allocateZeroMemory( m_nSizeOfZeroMemory ));
    }

    void tearDown() override
    {
        rtl_freeZeroMemory(m_pZeroMemory, m_nSizeOfZeroMemory);
        // LLA: no check possible, may GPF if there is something wrong.
        // CPPUNIT_ASSERT_MESSAGE( "Can get zero memory.", pZeroMemory != NULL);
    }

    // insert your test code here.

    void rtl_allocateZeroMemory_001()
    {
        CPPUNIT_ASSERT_MESSAGE( "Can get zero memory.", m_pZeroMemory != nullptr);
        CPPUNIT_ASSERT_MESSAGE( "memory contains wrong value.", checkMemory(m_pZeroMemory, m_nSizeOfZeroMemory, 0));

        memset(m_pZeroMemory, 3, m_nSizeOfZeroMemory);
        CPPUNIT_ASSERT_MESSAGE( "memory contains wrong value.", checkMemory(m_pZeroMemory, m_nSizeOfZeroMemory, 3));
    }

    CPPUNIT_TEST_SUITE(TestZeroMemory);
    CPPUNIT_TEST(rtl_allocateZeroMemory_001);
    CPPUNIT_TEST_SUITE_END();
};

class TestPreinit : public CppUnit::TestFixture
{
public:
    TestPreinit()
    {
    }

    // initialise your test code values here.
    void setUp() override
    {
    }

    void tearDown() override
    {
    }

    // insert your test code here.

    void test()
    {
        const char sample[] = "Hello World";
        std::vector<OUString> aStrings;

        rtl_alloc_preInit(true);

        OUString aFoo(o3tl::nonStaticString(u"foo"));

        // fill some cache bits
        for (int iter = 0; iter < 4; iter++)
        {
            for (int i = 1; i < 4096; i += 8)
            {
                OUStringBuffer aBuf(i);
                aBuf.appendAscii(sample, (i/8) % (SAL_N_ELEMENTS(sample)-1));
                OUString aStr = aBuf.makeStringAndClear();
                aStrings.push_back(aStr);
            }
            // free some pieces to make holes
            for (size_t i = iter; i < aStrings.size(); i += 17)
                aStrings[i] = aFoo;
        }

        for (size_t i = 0; i < aStrings.size(); ++i)
        {
            CPPUNIT_ASSERT_MESSAGE( "not static before.", !(aStrings[i].pData->refCount & SAL_STRING_STATIC_FLAG) );
        }

        // should static-ize all the strings.
        rtl_alloc_preInit(false);

        for (size_t i = 0; i < aStrings.size(); ++i)
            CPPUNIT_ASSERT_MESSAGE( "static after.", (aStrings[i].pData->refCount & SAL_STRING_STATIC_FLAG) );
    }

    void test2()
    {
        // should never happen but lets try it again.
        test();
    }

    CPPUNIT_TEST_SUITE(TestPreinit);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(test2);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_alloc::Memory);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_alloc::TestZeroMemory);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_alloc::TestPreinit);
} // namespace rtl_alloc

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
