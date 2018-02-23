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

// bootstrap stuff
#include <test/bootstrapfixture.hxx>

#include <vcl/salbtype.hxx>

namespace
{

class BitmapColorTest : public test::BootstrapFixture
{
public:
    BitmapColorTest() : BootstrapFixture(true, false) {}

    void defaultConstructor();
    void colorValueConstructor();
    void colorClassConstructor();

    void setValue();

    void invert();
    void getLuminance();

    CPPUNIT_TEST_SUITE(BitmapColorTest);
    CPPUNIT_TEST(defaultConstructor);
    CPPUNIT_TEST(colorValueConstructor);
    CPPUNIT_TEST(colorClassConstructor);
    CPPUNIT_TEST(setValue);
    CPPUNIT_TEST(invert);
    CPPUNIT_TEST(getLuminance);
    CPPUNIT_TEST_SUITE_END();
};


void BitmapColorTest::defaultConstructor()
{
    BitmapColor aBmpColor;
    const sal_uInt8 cTest=0;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), cTest);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(), cTest);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(), cTest);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue or index wrong", aBmpColor.GetBlueOrIndex(), cTest);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
}

void BitmapColorTest::colorValueConstructor()
{
    sal_uInt8 cRed=0, cGreen=0, cBlue=0;

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cRed = (i==0 ? 0 : 128*i-1);

        for (sal_uInt8 j=0; j<=2; j++)
        {
            cGreen = (j==0 ? 0 : 128*j-1);

            for (sal_uInt8 k=0; k<=2; k++)
            {
                cBlue = (k==0 ? 0 : 128*k-1);

                BitmapColor aBmpColor(cRed, cGreen, cBlue);

                CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), cRed);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(), cGreen);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(), cBlue);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue or index wrong", aBmpColor.GetBlueOrIndex(), cBlue);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
            }
        }
    }
}

void BitmapColorTest::colorClassConstructor()
{
    sal_uInt8 cRed=0, cGreen=0, cBlue=0;

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cRed = (i==0 ? 0 : 128*i-1);

        for (sal_uInt8 j=0; j<=2; j++)
        {
            cGreen = (j==0 ? 0 : 128*j-1);

            for (sal_uInt8 k=0; k<=2; k++)
            {
                Color aColor(cRed, cGreen, cBlue);
                BitmapColor aBmpColor(aColor);

                CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), cRed);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(), cGreen);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(), cBlue);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue or index wrong", aBmpColor.GetBlueOrIndex(), cBlue);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
            }
        }
    }


}

void BitmapColorTest::setValue()
{
    sal_uInt8 cRed=0, cGreen=0, cBlue=0;
    BitmapColor aBmpColor;

    // test red

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cRed = (i==0 ? 0 : 128*i-1);
        aBmpColor.SetRed(cRed);
        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetRed(), cRed);
    }

    // test green

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cGreen = (i==0 ? 0 : 128*i-1);
        aBmpColor.SetGreen(cGreen);
        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetGreen(), cGreen);
    }

    // test blue

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cBlue = (i==0 ? 0 : 128*i-1);
        aBmpColor.SetBlue(cBlue);
        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetBlue(), cBlue);
    }
}


void BitmapColorTest::invert()
{
    sal_uInt8 cRed=0, cGreen=0, cBlue=0;
    BitmapColor aBmpColor;

    // test red

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cRed = (i==0 ? 0 : 128*i-1);
        aBmpColor.SetRed(cRed);
        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetRed(), cRed);
    }

    // test green

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cGreen = (i==0 ? 0 : 128*i-1);
        aBmpColor.SetGreen(cGreen);
        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetGreen(), cGreen);
    }

    // test blue

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cBlue = (i==0 ? 0 : 128*i-1);
        aBmpColor.SetBlue(cBlue);
        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetBlue(), cBlue);
    }
}


void BitmapColorTest::getLuminance()
{
    sal_uInt8 cRed=0, cGreen=0, cBlue=0;

    for (sal_uInt8 i=0; i<=2; i++)
    {
        cRed = (i==0 ? 0 : 128*i-1);

        for (sal_uInt8 j=0; j<=2; j++)
        {
            cGreen = (j==0 ? 0 : 128*j-1);

            for (sal_uInt8 k=0; k<=2; k++)
            {
                cBlue = (k==0 ? 0 : 128*k-1);

                BitmapColor aBmpColor(cRed, cGreen, cBlue);
                sal_uInt32 cLuminance = (
                      static_cast<sal_uInt32>(cBlue) * 28
                    + static_cast<sal_uInt32>(cGreen) * 151
                    + static_cast<sal_uInt32>(cRed) * 77) >> 8;

                CPPUNIT_ASSERT_EQUAL(aBmpColor.GetLuminance(), static_cast<sal_uInt8>(cLuminance));
            }
        }
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapColorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
