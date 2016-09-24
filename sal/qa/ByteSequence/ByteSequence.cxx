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
#include "rtl/byteseq.hxx"

namespace {

class Test: public CppUnit::TestFixture {
public:
    void test_default() {
        rtl::ByteSequence s;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s.getLength());
    }

    void test_size0() {
        rtl::ByteSequence s(sal_Int32(0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s.getLength());
    }

    void test_size5() {
        rtl::ByteSequence s(5);
        sal_Int8 const * p = s.getConstArray();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), s.getLength());
        CPPUNIT_ASSERT_EQUAL(sal_Int8(0), p[0]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(0), p[1]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(0), p[2]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(0), p[3]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(0), p[4]);
    }

    void test_noinit0() {
        rtl::ByteSequence s(0, rtl::BYTESEQ_NODEFAULT);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s.getLength());
    }

    void test_noinit5() {
        rtl::ByteSequence s(5, rtl::BYTESEQ_NODEFAULT);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), s.getLength());
    }

    void test_elem0() {
        rtl::ByteSequence s(nullptr, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s.getLength());
    }

    void test_elem5() {
        sal_Int8 const a[5] = { 0, 1, 2, 3, 4 };
        rtl::ByteSequence s(a, 5);
        sal_Int8 const * p = s.getConstArray();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), s.getLength());
        CPPUNIT_ASSERT_EQUAL(sal_Int8(0), p[0]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(1), p[1]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(2), p[2]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(3), p[3]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(4), p[4]);
    }

    void test_copy() {
        rtl::ByteSequence s1(5);
        {
            rtl::ByteSequence s2(s1);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(5), s2.getLength());
            CPPUNIT_ASSERT_EQUAL(s1.getConstArray(), s2.getConstArray());
            CPPUNIT_ASSERT_EQUAL(s1.getHandle(), s2.getHandle());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), s1.getHandle()->nRefCount);
        }
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s1.getHandle()->nRefCount);
    }

    void test_fromC() {
        sal_Sequence c = { 1, 1, { 0 } };
        {
            rtl::ByteSequence s(&c);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s.getLength());
            CPPUNIT_ASSERT_EQUAL(
                static_cast< void const * >(c.elements),
                static_cast< void const * >(s.getConstArray()));
            CPPUNIT_ASSERT_EQUAL(&c, s.getHandle());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), c.nRefCount);
        }
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), c.nRefCount);
    }

    void test_noacquire() {
        sal_Sequence c = { 2, 1, { 0 } };
        {
            rtl::ByteSequence s(&c, rtl::BYTESEQ_NOACQUIRE);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s.getLength());
            CPPUNIT_ASSERT_EQUAL(
                static_cast< void const * >(c.elements),
                static_cast< void const * >(s.getConstArray()));
            CPPUNIT_ASSERT_EQUAL(&c, s.getHandle());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), c.nRefCount);
        }
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), c.nRefCount);
    }

    void test_getArray() {
        sal_Int8 const a[5] = { 0, 1, 2, 3, 4 };
        rtl::ByteSequence s1(a, 5);
        rtl::ByteSequence s2(s1);
        sal_Int8 * p = s2.getArray();
        p[2] = 10;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s1.getHandle()->nRefCount);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s2.getHandle()->nRefCount);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(2), s1.getConstArray()[2]);
        CPPUNIT_ASSERT_EQUAL(sal_Int8(10), s2.getConstArray()[2]);
    }

    void test_same0() {
        rtl::ByteSequence s1;
        rtl::ByteSequence s2;
        CPPUNIT_ASSERT_EQUAL(true, s1 == s2);
        CPPUNIT_ASSERT_EQUAL(true, s2 == s1);
        CPPUNIT_ASSERT_EQUAL(false, s1 != s2);
        CPPUNIT_ASSERT_EQUAL(false, s2 != s1);
    }

    void test_diffLen() {
        sal_Int8 const a[5] = { 0, 1, 2, 3, 4 };
        rtl::ByteSequence s1(a, 5);
        rtl::ByteSequence s2(a, 4);
        CPPUNIT_ASSERT_EQUAL(false, s1 == s2);
        CPPUNIT_ASSERT_EQUAL(false, s2 == s1);
        CPPUNIT_ASSERT_EQUAL(true, s1 != s2);
        CPPUNIT_ASSERT_EQUAL(true, s2 != s1);
    }

    void test_diffElem() {
        sal_Int8 const a1[5] = { 0, 1, 2, 3, 4 };
        rtl::ByteSequence s1(a1, 5);
        sal_Int8 const a2[5] = { 0, 1, 10, 3, 4 };
        rtl::ByteSequence s2(a2, 5);
        CPPUNIT_ASSERT_EQUAL(false, s1 == s2);
        CPPUNIT_ASSERT_EQUAL(false, s2 == s1);
        CPPUNIT_ASSERT_EQUAL(true, s1 != s2);
        CPPUNIT_ASSERT_EQUAL(true, s2 != s1);
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_default);
    CPPUNIT_TEST(test_size0);
    CPPUNIT_TEST(test_size5);
    CPPUNIT_TEST(test_noinit0);
    CPPUNIT_TEST(test_noinit5);
    CPPUNIT_TEST(test_elem0);
    CPPUNIT_TEST(test_elem5);
    CPPUNIT_TEST(test_copy);
    CPPUNIT_TEST(test_fromC);
    CPPUNIT_TEST(test_noacquire);
    CPPUNIT_TEST(test_getArray);
    CPPUNIT_TEST(test_same0);
    CPPUNIT_TEST(test_diffLen);
    CPPUNIT_TEST(test_diffElem);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
