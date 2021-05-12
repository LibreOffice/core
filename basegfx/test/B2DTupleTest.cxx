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

#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>

namespace basegfx
{
class B2DTupleTest : public CppUnit::TestFixture
{
public:
    void testEmpty()
    {
        B2DTuple aTuple;
        CPPUNIT_ASSERT_EQUAL(true, aTuple.equalZero());

        B2ITuple aTupleInteger;
        CPPUNIT_ASSERT_EQUAL(true, aTupleInteger.equalZero());

        B2I64Tuple aTupleLong;
        CPPUNIT_ASSERT_EQUAL(true, aTupleLong.equalZero());
    }

    void testEquals()
    {
        B2DTuple aTuple(1.0, 1.0);
        CPPUNIT_ASSERT_EQUAL(true, aTuple.equal({ 1.0, 1.0 }));
        CPPUNIT_ASSERT_EQUAL(false, aTuple.equal({ 0.99, 0.99 }));

        B2ITuple aTupleInteger(1, 1);
        CPPUNIT_ASSERT_EQUAL(true, aTupleInteger.equal({ 1, 1 }));
        CPPUNIT_ASSERT_EQUAL(false, aTupleInteger.equal({ 1, 0 }));
        CPPUNIT_ASSERT_EQUAL(false, aTupleInteger.equal({ 0, 1 }));

        B2I64Tuple aTupleLong(1, 1);
        CPPUNIT_ASSERT_EQUAL(true, aTupleLong.equal({ 1, 1 }));
        CPPUNIT_ASSERT_EQUAL(false, aTupleLong.equal({ 1, 0 }));
        CPPUNIT_ASSERT_EQUAL(false, aTupleLong.equal({ 0, 1 }));
    }

    void testOperatorAddition()
    {
        B2DTuple aTuple(4.0, 8.0);
        aTuple += { 2.0, 3.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, aTuple.getY(), 1e-2);

        B2ITuple aTupleInt(4, 8);
        aTupleInt += { 2, 3 };

        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aTupleInt.getY());

        B2I64Tuple aTuple64(4, 8);
        aTuple64 += { 2, 3 };

        CPPUNIT_ASSERT_EQUAL(sal_Int64(6), aTuple64.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(11), aTuple64.getY());
    }

    void testOperatorSubstraction()
    {
        B2DTuple aTuple(4.0, 8.0);
        aTuple -= { 2.0, 3.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, aTuple.getY(), 1e-2);

        B2ITuple aTupleInt(4, 8);
        aTupleInt -= { 2, 3 };

        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aTupleInt.getY());

        B2I64Tuple aTuple64(4, 8);
        aTuple64 -= { 2, 3 };

        CPPUNIT_ASSERT_EQUAL(sal_Int64(2), aTuple64.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(5), aTuple64.getY());
    }

    void testOperatorMultiply()
    {
        B2DTuple aTuple(4.0, 8.0);
        aTuple *= { 2.0, 3.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(24.0, aTuple.getY(), 1e-2);

        aTuple *= 2.0;

        CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(48.0, aTuple.getY(), 1e-2);

        B2ITuple aTupleInt(4, 8);
        aTupleInt *= { 2, 3 };

        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(24), aTupleInt.getY());

        aTupleInt *= 2.0;

        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(48), aTupleInt.getY());

        B2I64Tuple aTuple64(4, 8);
        aTuple64 *= { 2, 3 };

        CPPUNIT_ASSERT_EQUAL(sal_Int64(8), aTuple64.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(24), aTuple64.getY());

        aTuple64 *= 2.0;

        CPPUNIT_ASSERT_EQUAL(sal_Int64(16), aTuple64.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(48), aTuple64.getY());
    }

    void testOperatorDivide()
    {
        B2DTuple aTuple(4.0, 8.0);
        aTuple /= { 2.0, 8.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aTuple.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aTuple.getY(), 1e-2);

        B2ITuple aTupleInt(4, 8);
        aTupleInt /= { 2, 8 };

        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aTupleInt.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aTupleInt.getY());

        B2I64Tuple aTuple64(4, 8);
        aTuple64 /= { 2, 8 };

        CPPUNIT_ASSERT_EQUAL(sal_Int64(2), aTuple64.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(1), aTuple64.getY());
    }

    void testOperatorEqualUnequal()
    {
        B2DTuple aTuple(4.0, 8.0);
        B2DTuple aTuple2 = aTuple;

        CPPUNIT_ASSERT_EQUAL(true, aTuple == aTuple);
        CPPUNIT_ASSERT_EQUAL(true, aTuple == aTuple2);
        CPPUNIT_ASSERT_EQUAL(true, aTuple == B2DTuple(4.0, 8.0));
        CPPUNIT_ASSERT_EQUAL(false, aTuple == B2DTuple(4.0, 7.99));
        CPPUNIT_ASSERT_EQUAL(false, aTuple == B2DTuple(3.99, 8.0));

        B2ITuple aTupleInt(4, 8);
        B2ITuple aTupleInt2 = aTupleInt;

        CPPUNIT_ASSERT_EQUAL(true, aTupleInt == aTupleInt);
        CPPUNIT_ASSERT_EQUAL(true, aTupleInt == aTupleInt2);
        CPPUNIT_ASSERT_EQUAL(true, aTupleInt == B2ITuple(4, 8));
        CPPUNIT_ASSERT_EQUAL(false, aTupleInt == B2ITuple(4, 7));
        CPPUNIT_ASSERT_EQUAL(false, aTupleInt == B2ITuple(3, 8));

        B2I64Tuple aTuple64(4, 8);
        B2I64Tuple aTuple64_2 = aTuple64;

        CPPUNIT_ASSERT_EQUAL(true, aTuple64 == aTuple64);
        CPPUNIT_ASSERT_EQUAL(true, aTuple64 == aTuple64_2);
        CPPUNIT_ASSERT_EQUAL(true, aTuple64 == B2I64Tuple(4, 8));
        CPPUNIT_ASSERT_EQUAL(false, aTuple64 == B2I64Tuple(4, 7));
        CPPUNIT_ASSERT_EQUAL(false, aTuple64 == B2I64Tuple(3, 8));
    }

    void testOperatorMinus()
    {
        B2DTuple aTupleMinus = -B2DTuple(4.0, 8.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-4.0, aTupleMinus.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-8.0, aTupleMinus.getY(), 1e-2);
        B2DTuple aTupleZero = -B2DTuple(0.0, 0.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aTupleZero.getX(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aTupleZero.getY(), 1e-2);

        B2ITuple aTupleIntMinus = -B2ITuple(4, 8);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-4), aTupleIntMinus.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-8), aTupleIntMinus.getY());
        B2ITuple aTupleIntZero = -B2ITuple(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTupleIntZero.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTupleIntZero.getY());

        B2I64Tuple aTuple64Minus = -B2I64Tuple(4, 8);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-4), aTuple64Minus.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-8), aTuple64Minus.getY());
        B2I64Tuple aTuple64Zero = -B2I64Tuple(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(0), aTuple64Zero.getX());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(0), aTuple64Zero.getY());
    }

    CPPUNIT_TEST_SUITE(B2DTupleTest);
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

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::B2DTupleTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
