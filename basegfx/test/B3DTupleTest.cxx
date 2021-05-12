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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/tuple/b3ituple.hxx>

namespace basegfx
{
class B3DTupleTest : public CppUnit::TestFixture
{
public:
    void testEmpty()
    {
        B3DTuple aTuple;
        CPPUNIT_ASSERT_EQUAL(true, aTuple.equalZero());
    }

    void testEquals()
    {
        B3DTuple aTuple(1.0, 1.0, 1.0);
        CPPUNIT_ASSERT_EQUAL(true, aTuple.equal({ 1.0, 1.0, 1.0 }));
        CPPUNIT_ASSERT_EQUAL(false, aTuple.equal({ 0.99, 0.99, 0.99 }));
    }

    void testOperatorAddition()
    {
        B3DTuple aTuple(4.0, 8.0, 1.0);
        aTuple += { 2.0, 3.0, 4.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, aTuple.getY(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, aTuple.getZ(), 1e-2);

        B3ITuple aTupleInt(4, 8, 1);
        aTupleInt += { 2, 3, 4 };

        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aTupleInt.getY());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aTupleInt.getZ());
    }

    void testOperatorSubstraction()
    {
        B3DTuple aTuple(4.0, 8.0, 1.0);
        aTuple -= { 2.0, 3.0, 4.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, aTuple.getY(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-3.0, aTuple.getZ(), 1e-2);

        B3ITuple aTupleInt(4, 8, 1);
        aTupleInt -= { 2, 3, 4 };

        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aTupleInt.getY());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-3), aTupleInt.getZ());
    }

    void testOperatorMultiply()
    {
        B3DTuple aTuple(4.0, 8.0, 1.0);
        aTuple *= { 2.0, 3.0, 4.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(24.0, aTuple.getY(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, aTuple.getZ(), 1e-2);

        aTuple *= 2.0;

        CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(48.0, aTuple.getY(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, aTuple.getZ(), 1e-2);

        B3ITuple aTupleInt(4, 8, 1);
        aTupleInt *= { 2, 3, 4 };

        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(24), aTupleInt.getY());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aTupleInt.getZ());

        aTupleInt *= 2.0;

        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(48), aTupleInt.getY());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aTupleInt.getZ());
    }

    void testOperatorDivide()
    {
        B3DTuple aTuple(4.0, 8.0, 9.0);
        aTuple /= { 2.0, 8.0, 3.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aTuple.getY(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aTuple.getZ(), 1e-2);

        B3ITuple aTupleInt(4, 8, 9);
        aTupleInt /= { 2, 8, 3 };

        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aTupleInt.getY());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aTupleInt.getZ());
    }

    void testOperatorEqualUnequal()
    {
        B3DTuple aTuple(4.0, 8.0, 9.0);
        B3DTuple aTuple2 = aTuple;

        CPPUNIT_ASSERT_EQUAL(true, aTuple == aTuple);
        CPPUNIT_ASSERT_EQUAL(true, aTuple == aTuple2);
        CPPUNIT_ASSERT_EQUAL(true, aTuple == B3DTuple(4.0, 8.0, 9.0));
        CPPUNIT_ASSERT_EQUAL(false, aTuple == B3DTuple(4.0, 7.99, 9.0));
        CPPUNIT_ASSERT_EQUAL(false, aTuple == B3DTuple(3.99, 8.0, 9.0));

        CPPUNIT_ASSERT_EQUAL(false, aTuple != aTuple);
        CPPUNIT_ASSERT_EQUAL(false, aTuple != aTuple2);
        CPPUNIT_ASSERT_EQUAL(false, aTuple != B3DTuple(4.0, 8.0, 9.0));
        CPPUNIT_ASSERT_EQUAL(true, aTuple != B3DTuple(4.0, 7.99, 9.0));
        CPPUNIT_ASSERT_EQUAL(true, aTuple != B3DTuple(3.99, 8.0, 9.0));

        B3ITuple aTupleInt(4, 8, 9);
        B3ITuple aTupleInt2 = aTupleInt;

        CPPUNIT_ASSERT_EQUAL(true, aTupleInt == aTupleInt);
        CPPUNIT_ASSERT_EQUAL(true, aTupleInt == aTupleInt2);
        CPPUNIT_ASSERT_EQUAL(true, aTupleInt == B3ITuple(4, 8, 9));
        CPPUNIT_ASSERT_EQUAL(false, aTupleInt == B3ITuple(4, 7, 9));
        CPPUNIT_ASSERT_EQUAL(false, aTupleInt == B3ITuple(3, 8, 9));

        CPPUNIT_ASSERT_EQUAL(false, aTupleInt != aTupleInt);
        CPPUNIT_ASSERT_EQUAL(false, aTupleInt != aTupleInt2);
        CPPUNIT_ASSERT_EQUAL(false, aTupleInt != B3ITuple(4, 8, 9));
        CPPUNIT_ASSERT_EQUAL(true, aTupleInt != B3ITuple(4, 7, 9));
        CPPUNIT_ASSERT_EQUAL(true, aTupleInt != B3ITuple(3, 8, 9));
    }

    void testOperatorMinus()
    {
        B3DTuple aTupleMinus = -B3DTuple(4.0, 8.0, 1.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-4.0, aTupleMinus.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-8.0, aTupleMinus.getY(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aTupleMinus.getZ(), 1e-2);
        B3DTuple aTupleZero = -B3DTuple(0.0, 0.0, 0.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aTupleZero.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aTupleZero.getY(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aTupleZero.getZ(), 1e-2);

        B3ITuple aTupleIntMinus = -B3ITuple(4, 8, 1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-4), aTupleIntMinus.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-8), aTupleIntMinus.getY());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aTupleIntMinus.getZ());
        B3ITuple aTupleIntZero = -B3ITuple(0, 0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTupleIntZero.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTupleIntZero.getY());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTupleIntZero.getZ());
    }

    CPPUNIT_TEST_SUITE(B3DTupleTest);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testEquals);
    CPPUNIT_TEST(testOperatorAddition);
    CPPUNIT_TEST(testOperatorSubstraction);
    CPPUNIT_TEST(testOperatorMultiply);
    CPPUNIT_TEST(testOperatorDivide);
    CPPUNIT_TEST(testOperatorEqualUnequal);
    CPPUNIT_TEST(testOperatorMinus);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::B3DTupleTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
