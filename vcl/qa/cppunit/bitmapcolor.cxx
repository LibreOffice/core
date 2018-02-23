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

constexpr sal_uInt8 LUMINANCE(sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue)
{
    return (static_cast<sal_uInt32>(cBlue) * 28 + static_cast<sal_uInt32>(cGreen) * 151
            + static_cast<sal_uInt32>(cRed) * 77)
           >> 8;
}

void BitmapColorTest::getLuminance()
{
    {
        BitmapColor aBmpColor(0, 0, 0);

        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetLuminance(),
                             LUMINANCE(static_cast<sal_uInt8>(0), static_cast<sal_uInt8>(0),
                                       static_cast<sal_uInt8>(0)));
    }

    {
        BitmapColor aBmpColor(127, 127, 127);

        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetLuminance(),
                             LUMINANCE(static_cast<sal_uInt8>(127), static_cast<sal_uInt8>(127),
                                       static_cast<sal_uInt8>(127)));
    }

    {
        BitmapColor aBmpColor(255, 255, 255);

        CPPUNIT_ASSERT_EQUAL(aBmpColor.GetLuminance(),
                             LUMINANCE(static_cast<sal_uInt8>(255), static_cast<sal_uInt8>(255),
                                       static_cast<sal_uInt8>(255)));
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapColorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
