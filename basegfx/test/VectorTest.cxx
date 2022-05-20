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

#include <basegfx/vector/b2dvector.hxx>

namespace basegfx
{
class VectorTest : public CppUnit::TestFixture
{
public:
    void testCross()
    {
        B2DVector aVector(1.0, 1.0);
        double aResult = aVector.cross(B2DVector(1.0, 1.0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult, 1E-12);
    }

    void testScalar()
    {
        {
            B2DVector aVector(1.0, 1.0);
            double aResult = aVector.scalar(B2DVector(1.0, 1.0));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult, 1E-12);
        }
        {
            B2IVector aVector(1, 1);
            double aResult = aVector.scalar(B2IVector(1, 1));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult, 1E-12);
        }
    }

    void testSetLength()
    {
        {
            B2DVector aVector(1.0, 1.0);
            aVector = aVector.setLength(std::sqrt(2.0));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aVector.getX(), 1E-12);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aVector.getY(), 1E-12);
        }
        {
            B2IVector aVector(1, 1);
            aVector = aVector.setLength(std::sqrt(2.0));
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aVector.getX());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aVector.getY());
        }
    }

    void testGetLength()
    {
        B2DVector aVector(1.0, 1.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(std::sqrt(2.0), aVector.getLength(), 1E-12);
    }

    CPPUNIT_TEST_SUITE(VectorTest);
    CPPUNIT_TEST(testCross);
    CPPUNIT_TEST(testScalar);
    CPPUNIT_TEST(testSetLength);
    CPPUNIT_TEST(testGetLength);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::VectorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
