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

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <systools/win32/comtools.hxx>

class COMObject : public IUnknown
{
public:
    COMObject() : ref_count_(0)
    {
    }

    virtual ~COMObject()
    {
    }

    ULONG __stdcall AddRef() override
    {
        ref_count_++;
        return ref_count_;
    }

    ULONG __stdcall Release() override
    {
        ULONG cnt = --ref_count_;
        if (cnt == 0)
            delete this;
        return cnt;
    }

    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppv) override
    {
        if (riid == IID_IUnknown)
        {
            AddRef();
            *ppv = this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG GetRefCount() const
    {
        return ref_count_;
    }

private:
    ULONG ref_count_;
};

static sal::systools::COMReference<IUnknown> comObjectSource()
{
    return sal::systools::COMReference<IUnknown>(new COMObject);
}

static bool comObjectSink(sal::systools::COMReference<IUnknown> r, ULONG expectedRefCountOnReturn)
{
    r = sal::systools::COMReference<IUnknown>();
    COMObject* p = reinterpret_cast<COMObject*>(r.get());
    if (p)
        return (p->GetRefCount() == expectedRefCountOnReturn);
    else
        return (0 == expectedRefCountOnReturn);
}

static void comObjectSource2(LPVOID* ppv)
{
    COMObject* p = new COMObject;
    p->AddRef();
    *ppv = p;
}

namespace test_comtools
{

    class test_COMReference : public CppUnit::TestFixture
    {

    public:
        /// test of COMReference<IUnknown> r;
        void default_ctor()
        {
            sal::systools::COMReference<IUnknown> r;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("COMReference should be empty", static_cast<IUnknown *>(nullptr), r.get());
        }

        void test_ctor_manual_AddRef()
        {
            COMObject* p = new COMObject;
            p->AddRef();
            sal::systools::COMReference<IUnknown> r(p, false);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r.get())->GetRefCount());
        }

        void test_copy_ctor()
        {
            sal::systools::COMReference<IUnknown> r(comObjectSource());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r.get())->GetRefCount());
        }

        void test_copy_assignment()
        {
            sal::systools::COMReference<IUnknown> r;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("COMReference should be empty", static_cast<IUnknown *>(nullptr), r.get());

            r = comObjectSource();
            CPPUNIT_ASSERT_MESSAGE("COMReference should be empty", r.get() != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r.get())->GetRefCount());
        }

        void test_ref_to_ref_assignment()
        {
            sal::systools::COMReference<IUnknown> r1 = comObjectSource();
            sal::systools::COMReference<IUnknown> r2 = r1;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 2 is expected", ULONG(2), reinterpret_cast<COMObject*>(r2.get())->GetRefCount());
        }

        void test_pointer_to_ref_assignment()
        {
            sal::systools::COMReference<IUnknown> r;
            r = new COMObject;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r.get())->GetRefCount());
        }

        void test_pointer_to_ref_assignment2()
        {
            sal::systools::COMReference<IUnknown> r = comObjectSource();
            r = new COMObject;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r.get())->GetRefCount());
        }

        void test_source_sink()
        {
            CPPUNIT_ASSERT_MESSAGE("Wrong reference count, 0 is expected", comObjectSink(comObjectSource(), 0));
        }

        void test_address_operator()
        {
            sal::systools::COMReference<IUnknown> r;
            comObjectSource2(reinterpret_cast<LPVOID*>(&r));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count, 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r.get())->GetRefCount());
        }

        void test_address_operator2()
        {
            sal::systools::COMReference<IUnknown> r1 = comObjectSource();
            sal::systools::COMReference<IUnknown> r2 = r1;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 2 is expected", ULONG(2), reinterpret_cast<COMObject*>(r2.get())->GetRefCount());
            comObjectSource2(reinterpret_cast<LPVOID*>(&r1));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r1.get())->GetRefCount());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r2.get())->GetRefCount());
        }

        void test_clear()
        {
            sal::systools::COMReference<IUnknown> r = comObjectSource();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count 1 is expected", ULONG(1), reinterpret_cast<COMObject*>(r.get())->GetRefCount());
            r.clear();
            CPPUNIT_ASSERT_MESSAGE("Expect reference to be empty", !r.is());
        }

        void test_query_interface()
        {
            try
            {
                sal::systools::COMReference<IUnknown> r1 = comObjectSource();
                sal::systools::COMReference<IUnknown> r2 = r1.QueryInterface<IUnknown>(IID_IUnknown);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong reference count, 2 is expected", ULONG(2), reinterpret_cast<COMObject*>(r2.get())->GetRefCount());
            }
            catch(const sal::systools::ComError&)
            {
                CPPUNIT_ASSERT_MESSAGE("Exception should not have been thrown", false);
            }
        }

        void test_query_interface_throw()
        {
            try
            {
                sal::systools::COMReference<IUnknown> r1 = comObjectSource();
                sal::systools::COMReference<IPersistFile> r2 = r1.QueryInterface<IPersistFile>(IID_IPersistFile);
            }
            catch(const sal::systools::ComError&)
            {
                return;
            }
            CPPUNIT_ASSERT_MESSAGE("Exception should have been thrown", false);
        }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(test_COMReference);
        CPPUNIT_TEST(default_ctor);
        CPPUNIT_TEST(test_ctor_manual_AddRef);
        CPPUNIT_TEST(test_copy_ctor);
        CPPUNIT_TEST(test_copy_assignment);
        CPPUNIT_TEST(test_ref_to_ref_assignment);
        CPPUNIT_TEST(test_pointer_to_ref_assignment);
        CPPUNIT_TEST(test_pointer_to_ref_assignment2);
        CPPUNIT_TEST(test_source_sink);
        CPPUNIT_TEST(test_address_operator);
        CPPUNIT_TEST(test_address_operator2);
        CPPUNIT_TEST(test_clear);
        CPPUNIT_TEST(test_query_interface);
        CPPUNIT_TEST(test_query_interface_throw);
        CPPUNIT_TEST_SUITE_END();
    };

CPPUNIT_TEST_SUITE_REGISTRATION(test_comtools::test_COMReference);

} // namespace rtl_OUString

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
