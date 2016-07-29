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

#include <connectivity/FValue.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

namespace connectivity { namespace commontools {

class FValueTest: public test::BootstrapFixture
{
public:
    FValueTest() : test::BootstrapFixture(false, false) {};

    void test_Bool();

    void test_Int8();
    void test_uInt8();

    void test_Int16();
    void test_uInt16();

    void test_Int32();
    void test_uInt32();

    void test_Int64();
    void test_uInt64();

    void test_float();
    void test_double();

    void test_bool_getString();
    void test_bit_getString();

    void test_bool_creation();

    CPPUNIT_TEST_SUITE(FValueTest);

    CPPUNIT_TEST(test_Bool);

    CPPUNIT_TEST(test_Int8);
    CPPUNIT_TEST(test_uInt8);

    CPPUNIT_TEST(test_Int16);
    CPPUNIT_TEST(test_uInt16);

    CPPUNIT_TEST(test_Int32);
    CPPUNIT_TEST(test_uInt32);

    CPPUNIT_TEST(test_Int64);
    CPPUNIT_TEST(test_uInt64);

    CPPUNIT_TEST(test_float);
    CPPUNIT_TEST(test_double);

    CPPUNIT_TEST(test_bool_getString);
    CPPUNIT_TEST(test_bit_getString);
    CPPUNIT_TEST(test_bool_creation);
    CPPUNIT_TEST_SUITE_END();
};

void FValueTest::test_Bool()
{
    bool src_Bool = true;
    ORowSetValue v(src_Bool);
    bool trg_Bool = v.getBool();

    std::cerr << "src_Bool: " << src_Bool << std::endl;
    std::cerr << "trg_Bool: " << trg_Bool << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("bool conversion to ORowSetValue didn't work", trg_Bool, src_Bool);

    Any any_Bool = v.makeAny();
    ORowSetValue t;
    t.fill(any_Bool);
    trg_Bool = t.getBool();

    std::cerr << "trg_Bool: " << trg_Bool << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("bool conversion from Any didn't work", trg_Bool, src_Bool);
}

void FValueTest::test_Int8()
{
    sal_Int8 src_salInt8 = 127;
    ORowSetValue v(src_salInt8);
    sal_Int8 trg_salInt8 = v.getInt8();

    std::cerr << "src_salInt8: " << static_cast<short>(src_salInt8) << std::endl;
    std::cerr << "trg_salInt8: " << static_cast<short>(trg_salInt8) << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_Int8 conversion to ORowSetValue didn't work", trg_salInt8, src_salInt8);

    Any any_Int8 = v.makeAny();
    ORowSetValue t;
    t.fill(any_Int8);
    trg_salInt8 = t.getInt8();

    std::cerr << "trg_salInt8: " << static_cast<short>(trg_salInt8) << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_Int8 conversion from Any didn't work", trg_salInt8, src_salInt8);
}

void FValueTest::test_uInt8()
{
    sal_uInt8 src_saluInt8 = 255;
    ORowSetValue v(src_saluInt8);
    sal_uInt8 trg_saluInt8 = v.getUInt8();

    std::cerr << "src_saluInt8: " << static_cast<short>(src_saluInt8) << std::endl;
    std::cerr << "trg_saluInt8: " << static_cast<short>(trg_saluInt8) << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_uInt8 conversion to ORowSetValue didn't work", trg_saluInt8, src_saluInt8);

    Any any_uInt8 = v.makeAny();
    ORowSetValue t;
    t.fill(any_uInt8);
    trg_saluInt8 = t.getUInt8();

    std::cerr << "trg_saluInt8: " << static_cast<short>(trg_saluInt8) << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_uInt8 conversion from Any didn't work", trg_saluInt8, src_saluInt8);
}

void FValueTest::test_Int16()
{
    sal_Int16 src_salInt16 = -10001;
    ORowSetValue v(src_salInt16);
    sal_Int16 trg_salInt16 = v.getInt16();

    std::cerr << "src_salInt16: " << src_salInt16 << std::endl;
    std::cerr << "trg_salInt16: " << trg_salInt16 << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_Int16 conversion to ORowSetValue didn't work", trg_salInt16, src_salInt16);

    Any any_Int16 = v.makeAny();
    ORowSetValue t;
    t.fill(any_Int16);
    trg_salInt16 = t.getInt16();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_Int16 conversion from Any didn't work", trg_salInt16, src_salInt16);
}

void FValueTest::test_uInt16()
{
    sal_uInt16 src_saluInt16 = 10001;
    ORowSetValue v(src_saluInt16);
    sal_uInt16 trg_saluInt16 = v.getUInt16();

    std::cerr << "src_saluInt16: " << src_saluInt16 << std::endl;
    std::cerr << "trg_saluInt16: " << trg_saluInt16 << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_uInt16 conversion to ORowSetValue didn't work", trg_saluInt16, src_saluInt16);

    Any any_uInt16 = v.makeAny();
    ORowSetValue t;
    t.fill(any_uInt16);
    trg_saluInt16 = t.getUInt16();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_uInt16 conversion from Any didn't work", trg_saluInt16, src_saluInt16);
}

void FValueTest::test_Int32()
{
    sal_Int32 src_salInt32 = -10000001;
    ORowSetValue v(src_salInt32);
    sal_Int32 trg_salInt32 = v.getInt32();

    std::cerr << "src_salInt32: " << src_salInt32 << std::endl;
    std::cerr << "trg_salInt32: " << trg_salInt32 << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_Int32 conversion to ORowSetValue didn't work", trg_salInt32, src_salInt32);

    Any any_Int32 = v.makeAny();
    ORowSetValue t;
    t.fill(any_Int32);
    trg_salInt32 = t.getInt32();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_Int32 conversion from Any didn't work", trg_salInt32, src_salInt32);
}

void FValueTest::test_uInt32()
{
    sal_uInt32 src_saluInt32 = 100000001;
    ORowSetValue v(src_saluInt32);
    sal_uInt32 trg_saluInt32 = v.getUInt32();

    std::cerr << "src_saluInt32: " << src_saluInt32 << std::endl;
    std::cerr << "trg_saluInt32: " << trg_saluInt32 << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_uInt32 conversion to ORowSetValue didn't work", trg_saluInt32, src_saluInt32);

    Any any_uInt32 = v.makeAny();
    ORowSetValue t;
    t.fill(any_uInt32);
    trg_saluInt32 = t.getUInt32();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_uInt32 conversion from Any didn't work", trg_saluInt32, src_saluInt32);
}

void FValueTest::test_Int64()
{
    sal_Int64 src_salInt64 = -1000000000000000001LL;
    ORowSetValue v(src_salInt64);
    sal_Int64 trg_salInt64 = v.getLong();

    std::cerr << "src_salInt64: " << src_salInt64 << std::endl;
    std::cerr << "trg_salInt64: " << trg_salInt64 << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_Int64 conversion to ORowSetValue didn't work", trg_salInt64, src_salInt64);

    Any any_Int64 = v.makeAny();
    ORowSetValue t;
    t.fill(any_Int64);
    trg_salInt64 = t.getLong();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_Int64 conversion from Any didn't work", trg_salInt64, src_salInt64);
}

void FValueTest::test_uInt64()
{
    sal_uInt64 src_saluInt64 = 10000000000000000001ULL;
    ORowSetValue v(src_saluInt64);
    sal_uInt64 trg_saluInt64 = v.getULong();

    std::cerr << "src_saluInt64: " << src_saluInt64 << std::endl;
    std::cerr << "trg_saluInt64: " << trg_saluInt64 << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_uInt64 conversion to ORowSetValue didn't work", trg_saluInt64, src_saluInt64);

    Any any_uInt64 = v.makeAny();
    ORowSetValue t;
    t.fill(any_uInt64);
    trg_saluInt64 = t.getULong();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sal_uInt64 conversion from Any didn't work", trg_saluInt64, src_saluInt64);
}

void FValueTest::test_float()
{
    float src_float = 1.234f;
    ORowSetValue v(src_float);
    float trg_float = v.getFloat();

    std::cerr << "src_float: " << src_float << std::endl;
    std::cerr << "trg_float: " << trg_float << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("float conversion to ORowSetValue didn't work", trg_float, src_float);

    Any any_float = v.makeAny();
    ORowSetValue t;
    t.fill(any_float);
    trg_float = t.getFloat();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("float conversion from Any didn't work", trg_float, src_float);
}

void FValueTest::test_double()
{
    double src_double = 1.23456789;
    ORowSetValue v(src_double);
    double trg_double = v.getDouble();

    std::cerr << "src_double: " << src_double << std::endl;
    std::cerr << "trg_double: " << trg_double << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("double conversion to ORowSetValue didn't work", trg_double, src_double);

    Any any_double = v.makeAny();
    ORowSetValue t;
    t.fill(any_double);
    trg_double = t.getDouble();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("double conversion from Any didn't work", trg_double, src_double);
}

void FValueTest::test_bool_getString()
{
    bool src_bool_1 = true;
    ORowSetValue v_1(src_bool_1);
    OUString trg_bool_1 = v_1.getString();

    std::cerr << "src_bool_1: " << src_bool_1 << std::endl;
    std::cerr << "trg_bool_1: " << trg_bool_1 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("ORowSetValue bool to string conversion didn't work", bool(trg_bool_1 == "true"));

    bool src_bool_0 = false;
    ORowSetValue v_0(src_bool_0);
    OUString trg_bool_0 = v_0.getString();

    std::cerr << "src_bool_0: " << src_bool_0 << std::endl;
    std::cerr << "trg_bool_0: " << trg_bool_0 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("ORowSetValue bool to string conversion didn't work", bool(trg_bool_0 == "false"));
}

void FValueTest::test_bit_getString()
{
    bool src_bool_1 = true;
    ORowSetValue v_1(src_bool_1);
    v_1.setTypeKind(DataType::BIT);
    OUString trg_bool_1 = v_1.getString();

    std::cerr << "src_bit_1: " << src_bool_1 << std::endl;
    std::cerr << "trg_bit_1: " << trg_bool_1 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("ORowSetValue bit to string conversion didn't work", bool(trg_bool_1 == "1"));

    bool src_bool_0 = false;
    ORowSetValue v_0(src_bool_0);
    v_0.setTypeKind(DataType::BIT);
    OUString trg_bool_0 = v_0.getString();

    std::cerr << "src_bit_0: " << src_bool_0 << std::endl;
    std::cerr << "trg_bit_0: " << trg_bool_0 << std::endl;

    CPPUNIT_ASSERT_MESSAGE("ORowSetValue bit to string conversion didn't work", bool(trg_bool_0 == "0"));
}

void FValueTest::test_bool_creation()
{
    ORowSetValue vTrue(true);
    ORowSetValue vFalse(false);

    {
        ORowSetValue v(OUString("1"));
        v.setTypeKind(DataType::BOOLEAN);
        CPPUNIT_ASSERT_MESSAGE("ORowSetValue bool creation from string didn't work", bool(v == vTrue));
    }

    {
        ORowSetValue v(OUString("0"));
        v.setTypeKind(DataType::BOOLEAN);
        CPPUNIT_ASSERT_MESSAGE("ORowSetValue bool creation from string didn't work", bool(v == vFalse));
    }

    {
        ORowSetValue v(OUString("true"));
        v.setTypeKind(DataType::BOOLEAN);
        CPPUNIT_ASSERT_MESSAGE("ORowSetValue bool creation from string didn't work", bool(v == vTrue));
    }

    {
        ORowSetValue v(OUString("tRuE"));
        v.setTypeKind(DataType::BOOLEAN);
        CPPUNIT_ASSERT_MESSAGE("ORowSetValue bool creation from string didn't work", bool(v == vTrue));
    }

    {
        ORowSetValue v(OUString("false"));
        v.setTypeKind(DataType::BOOLEAN);
        CPPUNIT_ASSERT_MESSAGE("ORowSetValue bool creation from string didn't work", bool(v == vFalse));
    }

    {
        ORowSetValue v(OUString("0"));
        v.setTypeKind(DataType::BOOLEAN);
        CPPUNIT_ASSERT_MESSAGE("ORowSetValue bool creation from string didn't work", bool(v == vFalse));
    }

}

CPPUNIT_TEST_SUITE_REGISTRATION(FValueTest);

}}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
