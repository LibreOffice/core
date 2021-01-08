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

#include <vcl/BitmapColor.hxx>

namespace
{
class BitmapColorTest : public test::BootstrapFixture
{
public:
    BitmapColorTest()
        : BootstrapFixture(true, false)
    {
    }

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

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", static_cast<sal_uInt8>(0), aBmpColor.GetRed());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", static_cast<sal_uInt8>(0), aBmpColor.GetGreen());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", static_cast<sal_uInt8>(0), aBmpColor.GetBlue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Transparency wrong", static_cast<sal_uInt8>(0),
                                 aBmpColor.GetTransparency());
}

void BitmapColorTest::colorValueConstructor()
{
    {
        BitmapColor aBmpColor(0, 0, 0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", static_cast<sal_uInt8>(0), aBmpColor.GetRed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", static_cast<sal_uInt8>(0),
                                     aBmpColor.GetGreen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", static_cast<sal_uInt8>(0), aBmpColor.GetBlue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Transparency wrong", static_cast<sal_uInt8>(0),
                                     aBmpColor.GetTransparency());
    }

    {
        BitmapColor aBmpColor(128, 128, 128);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", static_cast<sal_uInt8>(128), aBmpColor.GetRed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", static_cast<sal_uInt8>(128),
                                     aBmpColor.GetGreen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", static_cast<sal_uInt8>(128),
                                     aBmpColor.GetBlue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Transparency wrong", static_cast<sal_uInt8>(0),
                                     aBmpColor.GetTransparency());
    }

    {
        BitmapColor aBmpColor(255, 255, 255);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", static_cast<sal_uInt8>(255), aBmpColor.GetRed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", static_cast<sal_uInt8>(255),
                                     aBmpColor.GetGreen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", static_cast<sal_uInt8>(255),
                                     aBmpColor.GetBlue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Transparency wrong", static_cast<sal_uInt8>(0),
                                     aBmpColor.GetTransparency());
    }
}

void BitmapColorTest::colorClassConstructor()
{
    {
        BitmapColor aBmpColor(Color(0, 0, 0));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", static_cast<sal_uInt8>(0), aBmpColor.GetRed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", static_cast<sal_uInt8>(0),
                                     aBmpColor.GetGreen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", static_cast<sal_uInt8>(0), aBmpColor.GetBlue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Transparency wrong", static_cast<sal_uInt8>(0),
                                     aBmpColor.GetTransparency());
    }

    {
        BitmapColor aBmpColor(Color(127, 127, 127));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", static_cast<sal_uInt8>(127), aBmpColor.GetRed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", static_cast<sal_uInt8>(127),
                                     aBmpColor.GetGreen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", static_cast<sal_uInt8>(127),
                                     aBmpColor.GetBlue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Transparency wrong", static_cast<sal_uInt8>(0),
                                     aBmpColor.GetTransparency());
    }

    {
        BitmapColor aBmpColor(Color(255, 255, 255));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", static_cast<sal_uInt8>(255), aBmpColor.GetRed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", static_cast<sal_uInt8>(255),
                                     aBmpColor.GetGreen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", static_cast<sal_uInt8>(255),
                                     aBmpColor.GetBlue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Transparency wrong", static_cast<sal_uInt8>(0),
                                     aBmpColor.GetTransparency());
    }

    // Transparency / Alpha
    {
        BitmapColor aBmpColor(Color(255, 128, 64, 0));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", static_cast<sal_uInt8>(128), aBmpColor.GetRed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", static_cast<sal_uInt8>(64),
                                     aBmpColor.GetGreen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", static_cast<sal_uInt8>(0), aBmpColor.GetBlue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Transparency wrong", static_cast<sal_uInt8>(255),
                                     aBmpColor.GetTransparency());
    }
}

void BitmapColorTest::setValue()
{
    BitmapColor aBmpColor;

    aBmpColor.SetRed(127);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(127), aBmpColor.GetRed());

    aBmpColor.SetGreen(127);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(127), aBmpColor.GetGreen());

    aBmpColor.SetBlue(127);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(127), aBmpColor.GetBlue());
}

void BitmapColorTest::invert()
{
    BitmapColor aBmpColor(255, 255, 255);
    BitmapColor aInvertedColor(aBmpColor);
    aInvertedColor.Invert();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(0), aInvertedColor.GetRed());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(0), aInvertedColor.GetGreen());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(0), aInvertedColor.GetBlue());
}

void BitmapColorTest::getLuminance()
{
    {
        BitmapColor aBmpColor(COL_BLACK);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(0), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_BLUE);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(14), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_GREEN);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(75), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_CYAN);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(90), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_RED);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(38), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_MAGENTA);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(52), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_BROWN);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(113), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_GRAY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(128), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_LIGHTGRAY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(192), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_LIGHTBLUE);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(28), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_LIGHTGREEN);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(150), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_LIGHTCYAN);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(179), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_LIGHTRED);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(75), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_LIGHTMAGENTA);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(104), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_YELLOW);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(226), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(COL_WHITE);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255), aBmpColor.GetLuminance());
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapColorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
