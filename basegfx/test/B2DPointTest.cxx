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
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <cmath>
#include <sstream>

class B2DPointTest : public CppUnit::TestFixture
{
public:
    void testCreation();
    void testSet();
    void testTimesEquals();
    void testMultipy();
    void testAssignment();
    void testGetEmptyPoint();
    void testOutputOperator();

    CPPUNIT_TEST_SUITE(B2DPointTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testSet);
    CPPUNIT_TEST(testTimesEquals);
    CPPUNIT_TEST(testMultipy);
    CPPUNIT_TEST(testAssignment);
    CPPUNIT_TEST(testGetEmptyPoint);
    CPPUNIT_TEST(testOutputOperator);
    CPPUNIT_TEST_SUITE_END();
};

void B2DPointTest::testCreation()
{
    basegfx::B2DPoint aPointDefault;
    CPPUNIT_ASSERT_EQUAL(0.0, aPointDefault.getX());
    CPPUNIT_ASSERT_EQUAL(0.0, aPointDefault.getY());

    basegfx::B2DPoint aPoint1(5.0, 2.0);
    CPPUNIT_ASSERT_EQUAL(5.0, aPoint1.getX());
    CPPUNIT_ASSERT_EQUAL(2.0, aPoint1.getY());

    basegfx::B2DPoint aPointCopy(aPoint1);
    CPPUNIT_ASSERT_EQUAL(5.0, aPointCopy.getX());
    CPPUNIT_ASSERT_EQUAL(2.0, aPointCopy.getY());

    basegfx::B2DPoint aPoint2 = { 5.0, 2.0 };
    CPPUNIT_ASSERT_EQUAL(5.0, aPoint2.getX());
    CPPUNIT_ASSERT_EQUAL(2.0, aPoint2.getY());

    basegfx::B2IPoint aPointI1(1, 2);
    basegfx::B2DPoint aPointFromI(aPointI1);
    CPPUNIT_ASSERT_EQUAL(1.0, aPointFromI.getX());
    CPPUNIT_ASSERT_EQUAL(2.0, aPointFromI.getY());

    basegfx::B2DTuple aTuple(3.5, 4.5);
    basegfx::B2DPoint aPointFromTuple(aTuple);
    CPPUNIT_ASSERT_EQUAL(3.5, aPointFromTuple.getX());
    CPPUNIT_ASSERT_EQUAL(4.5, aPointFromTuple.getY());

    std::vector<basegfx::B2DPoint> aPointVector{
        { 5.0, 2.0 },
        { 4.0, 3.0 },
    };
    CPPUNIT_ASSERT_EQUAL(5.0, aPointVector[0].getX());
    CPPUNIT_ASSERT_EQUAL(2.0, aPointVector[0].getY());
    CPPUNIT_ASSERT_EQUAL(4.0, aPointVector[1].getX());
    CPPUNIT_ASSERT_EQUAL(3.0, aPointVector[1].getY());
}

void B2DPointTest::testSet()
{
    basegfx::B2DPoint aPoint;
    aPoint.setX(1.1);
    aPoint.setY(2.2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.1, aPoint.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.2, aPoint.getY(), 0.0000001);
}

void B2DPointTest::testTimesEquals()
{
    basegfx::B2DPoint aPoint1(1.1, 2.2);
    basegfx::B2DPoint aPoint2(3.0, 4.0);
    aPoint1 *= aPoint2;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, aPoint1.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.8, aPoint1.getY(), 0.0000001);

    aPoint2 *= 1.5;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.5, aPoint2.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, aPoint2.getY(), 0.0000001);

    basegfx::B2DHomMatrix aMatrix;
    aMatrix.identity();
    aPoint1 *= aMatrix;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, aPoint1.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.8, aPoint1.getY(), 0.0000001);

    aMatrix.translate(1.0, 2.0);
    aPoint1 *= aMatrix;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.3, aPoint1.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.8, aPoint1.getY(), 0.0000001);

    aMatrix.identity();
    aMatrix.rotate(-M_PI_4);
    aPoint1.setX(1.0);
    aPoint1.setY(1.0);
    aPoint1 *= aMatrix;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sqrt(2.0), aPoint1.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aPoint1.getY(), 0.0000001);

    aMatrix.identity();
    aMatrix.translate(0.0, 1.0);
    aMatrix.rotate(M_PI_4);
    aMatrix.scale(2.0, 2.0);
    aMatrix.shearX(2.0);
    aMatrix.shearY(3.0);
    aPoint1.setX(0);
    aPoint1.setY(0);
    aPoint1 *= aMatrix;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sqrt(2.0), aPoint1.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0 * sqrt(2.0), aPoint1.getY(), 0.0000001);
}
void B2DPointTest::testMultipy()
{
    basegfx::B2DPoint aPoint(1, 2);
    basegfx::B2DHomMatrix aMatrix;
    aMatrix.identity();
    aMatrix.rotate(M_PI);
    basegfx::B2DPoint aResult = aMatrix * aPoint;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1, aResult.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-2, aResult.getY(), 0.0000001);
}

void B2DPointTest::testAssignment()
{
    basegfx::B2DTuple aTuple(2.5, 5.5);
    basegfx::B2DPoint aPoint(0, 0);
    aPoint = aTuple;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.5, aPoint.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, aPoint.getY(), 0.0000001);

    basegfx::B2DPoint aPoint2(3.2, 6.2);
    aPoint = aPoint2;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.2, aPoint.getX(), 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.2, aPoint.getY(), 0.0000001);
}
void B2DPointTest::testGetEmptyPoint()
{
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, basegfx::B2DPoint::getEmptyPoint().getX(), .0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, basegfx::B2DPoint::getEmptyPoint().getY(), .0000001);
}

void B2DPointTest::testOutputOperator()
{
    std::ostringstream aOut;
    basegfx::B2DPoint aPoint(2.0, 3.0);
    aOut << aPoint;
    std::string aResult = aOut.str();
    CPPUNIT_ASSERT_EQUAL(std::string("(2,3)"), aResult);
}

CPPUNIT_TEST_SUITE_REGISTRATION(B2DPointTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
