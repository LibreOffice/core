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

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), static_cast<sal_uInt8>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(), static_cast<sal_uInt8>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(), static_cast<sal_uInt8>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
}

void BitmapColorTest::colorValueConstructor()
{
    {
        BitmapColor aBmpColor(0, 0, 0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), static_cast<sal_uInt8>(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(),
                                     static_cast<sal_uInt8>(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(), static_cast<sal_uInt8>(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
    }

    {
        BitmapColor aBmpColor(128, 128, 128);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), static_cast<sal_uInt8>(128));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(),
                                     static_cast<sal_uInt8>(128));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(),
                                     static_cast<sal_uInt8>(128));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
    }

    {
        BitmapColor aBmpColor(255, 255, 255);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), static_cast<sal_uInt8>(255));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(),
                                     static_cast<sal_uInt8>(255));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(),
                                     static_cast<sal_uInt8>(255));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
    }
}

void BitmapColorTest::colorClassConstructor()
{
    {
        Color aColor(0, 0, 0);
        BitmapColor aBmpColor(aColor);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), static_cast<sal_uInt8>(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(),
                                     static_cast<sal_uInt8>(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(), static_cast<sal_uInt8>(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
    }

    {
        Color aColor(127, 127, 127);
        BitmapColor aBmpColor(aColor);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), static_cast<sal_uInt8>(127));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(),
                                     static_cast<sal_uInt8>(127));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(),
                                     static_cast<sal_uInt8>(127));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
    }

    {
        Color aColor(255, 255, 255);
        BitmapColor aBmpColor(aColor);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Red wrong", aBmpColor.GetRed(), static_cast<sal_uInt8>(255));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Green wrong", aBmpColor.GetGreen(),
                                     static_cast<sal_uInt8>(255));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue wrong", aBmpColor.GetBlue(),
                                     static_cast<sal_uInt8>(255));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Index wrong", aBmpColor.IsIndex(), false);
    }
}

void BitmapColorTest::setValue()
{
    BitmapColor aBmpColor;

    aBmpColor.SetRed(127);
    CPPUNIT_ASSERT_EQUAL(aBmpColor.GetRed(), static_cast<sal_uInt8>(127));

    aBmpColor.SetGreen(127);
    CPPUNIT_ASSERT_EQUAL(aBmpColor.GetGreen(), static_cast<sal_uInt8>(127));

    aBmpColor.SetBlue(127);
    CPPUNIT_ASSERT_EQUAL(aBmpColor.GetBlue(), static_cast<sal_uInt8>(127));
}

void BitmapColorTest::invert()
{
    BitmapColor aBmpColor(255, 255, 255);
    BitmapColor aInvertedColor = aBmpColor.Invert();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Inverted red wrong", aInvertedColor.GetRed(),
                                 static_cast<sal_uInt8>(0));
    CPPUNIT_ASSERT_EQUAL(aInvertedColor.GetGreen(), static_cast<sal_uInt8>(0));
    CPPUNIT_ASSERT_EQUAL(aInvertedColor.GetBlue(), static_cast<sal_uInt8>(0));
}

void BitmapColorTest::getLuminance()
{
    {
        BitmapColor aBmpColor(Color(COL_BLACK));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(0), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_BLUE));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(14), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_GREEN));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(75), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_CYAN));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(89), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_RED));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(38), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_MAGENTA));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(52), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_BROWN));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(114), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_GRAY));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(128), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_LIGHTGRAY));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(192), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_LIGHTBLUE));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(27), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_LIGHTGREEN));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(150), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_LIGHTCYAN));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(178), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_LIGHTRED));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(76), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_LIGHTMAGENTA));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(104), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_YELLOW));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(227), aBmpColor.GetLuminance());
    }

    {
        BitmapColor aBmpColor(Color(COL_WHITE));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255), aBmpColor.GetLuminance());
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapColorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
