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

#include <basegfx/vector/b2dsize.hxx>

namespace basegfx
{
class B2DSizeTest : public CppUnit::TestFixture
{
public:
    void testOperatorAddition()
    {
        B2DSize aSize(4.0, 8.0);
        aSize += { 2.0, 3.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, aSize.getWidth(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, aSize.getHeight(), 1e-2);
    }

    void testOperatorSubstraction()
    {
        B2DSize aSize(4.0, 8.0);
        aSize -= { 2.0, 3.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aSize.getWidth(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, aSize.getHeight(), 1e-2);
    }

    void testOperatorMultiply()
    {
        B2DSize aSize(4.0, 8.0);
        aSize *= { 2.0, 3.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, aSize.getWidth(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(24.0, aSize.getHeight(), 1e-2);

        aSize *= 2.0;

        CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, aSize.getWidth(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(48.0, aSize.getHeight(), 1e-2);
    }

    void testOperatorDivide()
    {
        B2DSize aSize(4.0, 8.0);
        aSize /= { 2.0, 8.0 };

        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aSize.getWidth(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aSize.getHeight(), 1e-2);
    }

    void testOperatorEqualUnequal()
    {
        B2DSize aSize(4.0, 8.0);
        B2DSize aSize2 = aSize;

        CPPUNIT_ASSERT_EQUAL(true, aSize == aSize);
        CPPUNIT_ASSERT_EQUAL(true, aSize == aSize2);
        CPPUNIT_ASSERT_EQUAL(true, aSize == B2DSize(4.0, 8.0));
        CPPUNIT_ASSERT_EQUAL(false, aSize == B2DSize(4.0, 7.99));
        CPPUNIT_ASSERT_EQUAL(false, aSize == B2DSize(3.99, 8.0));
    }

    void testOperatorMinus()
    {
        B2DSize aSizeMinus = -B2DSize(4.0, 8.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-4.0, aSizeMinus.getWidth(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-8.0, aSizeMinus.getHeight(), 1e-2);

        B2DSize aSizeZero = -B2DSize(0.0, 0.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aSizeZero.getWidth(), 1e-2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aSizeZero.getHeight(), 1e-2);
    }

    CPPUNIT_TEST_SUITE(B2DSizeTest);
    CPPUNIT_TEST(testOperatorAddition);
    CPPUNIT_TEST(testOperatorSubstraction);
    CPPUNIT_TEST(testOperatorMultiply);
    CPPUNIT_TEST(testOperatorDivide);
    CPPUNIT_TEST(testOperatorEqualUnequal);
    CPPUNIT_TEST(testOperatorMinus);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::B2DSizeTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
