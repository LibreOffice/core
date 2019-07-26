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
#include <basegfx/point/b2dpoint.hxx>

class B2DPointTest : public CppUnit::TestFixture
{
public:
    void testCreation();

    CPPUNIT_TEST_SUITE(B2DPointTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST_SUITE_END();
};

void B2DPointTest::testCreation()
{
    basegfx::B2DPoint aPoint1(5.0, 2.0);
    CPPUNIT_ASSERT_EQUAL(5.0, aPoint1.getX());
    CPPUNIT_ASSERT_EQUAL(2.0, aPoint1.getY());

    basegfx::B2DPoint aPoint2 = { 5.0, 2.0 };
    CPPUNIT_ASSERT_EQUAL(5.0, aPoint2.getX());
    CPPUNIT_ASSERT_EQUAL(2.0, aPoint2.getY());

    std::vector<basegfx::B2DPoint> aPointVector{
        { 5.0, 2.0 },
        { 4.0, 3.0 },
    };
    CPPUNIT_ASSERT_EQUAL(5.0, aPointVector[0].getX());
    CPPUNIT_ASSERT_EQUAL(2.0, aPointVector[0].getY());
    CPPUNIT_ASSERT_EQUAL(4.0, aPointVector[1].getX());
    CPPUNIT_ASSERT_EQUAL(3.0, aPointVector[1].getY());
}

CPPUNIT_TEST_SUITE_REGISTRATION(B2DPointTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
