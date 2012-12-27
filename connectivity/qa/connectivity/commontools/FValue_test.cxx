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

#include <test/bootstrapfixture.hxx>

#include "connectivity/FValue.hxx"
using namespace ::com::sun::star::uno;

//#define ERROR_CONVERSION_FROM_UINT8_TO_ANY_AND_BACK 1

namespace connectivity { namespace commontools {

class FValueTest: public test::BootstrapFixture
{
public:
    FValueTest() : test::BootstrapFixture(false, false) {};

    void test_Int8();
    void test_uInt8();

    void test_Int16();
    void test_uInt16();

    void test_Int32();
    void test_uInt32();

    void test_Int64();
    void test_uInt64();

    CPPUNIT_TEST_SUITE(FValueTest);

    CPPUNIT_TEST(test_Int8);
    CPPUNIT_TEST(test_uInt8);

    CPPUNIT_TEST(test_Int16);
    CPPUNIT_TEST(test_uInt16);

    CPPUNIT_TEST(test_Int32);
    CPPUNIT_TEST(test_uInt32);

    CPPUNIT_TEST(test_Int64);
    CPPUNIT_TEST(test_uInt64);

    CPPUNIT_TEST_SUITE_END();
};

void FValueTest::test_Int8()
{
    sal_Int8 src_salInt8 = 127;
    ORowSetValue v(src_salInt8);
    sal_Int8 trg_salInt8 = v.getInt8();

    std::cerr << "src_salInt8: " << static_cast<short>(src_salInt8) << std::endl;
    std::cerr << "trg_salInt8: " << static_cast<short>(trg_salInt8) << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_Int8 conversion to ORowSetValued didn't work", src_salInt8 == trg_salInt8);

    Any any_Int8 = v.makeAny();
    ORowSetValue t;
    t.fill(any_Int8);
    trg_salInt8 = t.getInt8();

    std::cerr << "trg_salInt8: " << static_cast<short>(trg_salInt8) << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_Int8 conversion from Any didn't work", src_salInt8 == trg_salInt8);
}

void FValueTest::test_uInt8()
{
    sal_uInt8 src_saluInt8 = 255;
    ORowSetValue v(src_saluInt8);
    sal_uInt8 trg_saluInt8 = v.getUInt8();

    std::cerr << "src_saluInt8: " << static_cast<short>(src_saluInt8) << std::endl;
    std::cerr << "trg_saluInt8: " << static_cast<short>(trg_saluInt8) << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_uInt8 conversion to ORowSetValued didn't work", src_saluInt8 == trg_saluInt8);

//To Any and from Any conversion doesn't not work for sal_uInt8, because of missing UNSIGNED_BYTE type in Any
#ifdef ERROR_CONVERSION_FROM_UINT8_TO_ANY_AND_BACK
    Any any_uInt8 = v.makeAny();
    ORowSetValue t;
    t.fill(any_uInt8);
    trg_saluInt8 = t.getUInt8();

    std::cerr << "trg_saluInt8: " << static_cast<short>(trg_saluInt8) << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_uInt8 conversion from Any didn't work", src_saluInt8 == trg_saluInt8);
#endif
}

void FValueTest::test_Int16()
{
    sal_Int16 src_salInt16 = -10001;
    ORowSetValue v(src_salInt16);
    sal_Int16 trg_salInt16 = v.getInt16();

    std::cerr << "src_salInt16: " << src_salInt16 << std::endl;
    std::cerr << "trg_salInt16: " << trg_salInt16 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_Int16 conversion to ORowSetValued didn't work", src_salInt16 == trg_salInt16);

    Any any_Int16 = v.makeAny();
    ORowSetValue t;
    t.fill(any_Int16);
    trg_salInt16 = t.getInt16();

    CPPUNIT_ASSERT_MESSAGE("sal_Int16 conversion from Any didn't work", src_salInt16 == trg_salInt16);
}

void FValueTest::test_uInt16()
{
    sal_uInt16 src_saluInt16 = 10001;
    ORowSetValue v(src_saluInt16);
    sal_uInt16 trg_saluInt16 = v.getUInt16();

    std::cerr << "src_saluInt16: " << src_saluInt16 << std::endl;
    std::cerr << "trg_saluInt16: " << trg_saluInt16 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_uInt16 conversion to ORowSetValued didn't work", src_saluInt16 == trg_saluInt16);

    Any any_uInt16 = v.makeAny();
    ORowSetValue t;
    t.fill(any_uInt16);
    trg_saluInt16 = t.getUInt16();

    CPPUNIT_ASSERT_MESSAGE("sal_uInt16 conversion from Any didn't work", src_saluInt16 == trg_saluInt16);
}

void FValueTest::test_Int32()
{
    sal_Int32 src_salInt32 = -10000001;
    ORowSetValue v(src_salInt32);
    sal_Int32 trg_salInt32 = v.getInt32();

    std::cerr << "src_salInt32: " << src_salInt32 << std::endl;
    std::cerr << "trg_salInt32: " << trg_salInt32 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_Int32 conversion to ORowSetValued didn't work", src_salInt32 == trg_salInt32);

    Any any_Int32 = v.makeAny();
    ORowSetValue t;
    t.fill(any_Int32);
    trg_salInt32 = t.getInt32();

    CPPUNIT_ASSERT_MESSAGE("sal_Int32 conversion from Any didn't work", src_salInt32 == trg_salInt32);
}

void FValueTest::test_uInt32()
{
    sal_uInt32 src_saluInt32 = 100000001;
    ORowSetValue v(src_saluInt32);
    sal_uInt32 trg_saluInt32 = v.getUInt32();

    std::cerr << "src_saluInt32: " << src_saluInt32 << std::endl;
    std::cerr << "trg_saluInt32: " << trg_saluInt32 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_uInt32 conversion to ORowSetValued didn't work", src_saluInt32 == trg_saluInt32);

    Any any_uInt32 = v.makeAny();
    ORowSetValue t;
    t.fill(any_uInt32);
    trg_saluInt32 = t.getUInt32();

    CPPUNIT_ASSERT_MESSAGE("sal_uInt32 conversion from Any didn't work", src_saluInt32 == trg_saluInt32);
}

void FValueTest::test_Int64()
{
    sal_Int64 src_salInt64 = -1000000000000000001LL;
    ORowSetValue v(src_salInt64);
    sal_Int64 trg_salInt64 = v.getLong();

    std::cerr << "src_salInt64: " << src_salInt64 << std::endl;
    std::cerr << "trg_salInt64: " << trg_salInt64 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_Int64 conversion to ORowSetValued didn't work", src_salInt64 == trg_salInt64);

    Any any_Int64 = v.makeAny();
    ORowSetValue t;
    t.fill(any_Int64);
    trg_salInt64 = t.getLong();

    CPPUNIT_ASSERT_MESSAGE("sal_Int64 conversion from Any didn't work", src_salInt64 == trg_salInt64);
}

void FValueTest::test_uInt64()
{
    sal_uInt64 src_saluInt64 = 10000000000000000001ULL;
    ORowSetValue v(src_saluInt64);
    sal_uInt64 trg_saluInt64 = v.getULong();

    std::cerr << "src_saluInt64: " << src_saluInt64 << std::endl;
    std::cerr << "trg_saluInt64: " << trg_saluInt64 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("sal_uInt64 conversion to ORowSetValued didn't work", src_saluInt64 == trg_saluInt64);

    Any any_uInt64 = v.makeAny();
    ORowSetValue t;
    t.fill(any_uInt64);
    trg_saluInt64 = t.getULong();

    CPPUNIT_ASSERT_MESSAGE("sal_uInt64 conversion from Any didn't work", src_saluInt64 == trg_saluInt64);
}

CPPUNIT_TEST_SUITE_REGISTRATION(FValueTest);

}}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
