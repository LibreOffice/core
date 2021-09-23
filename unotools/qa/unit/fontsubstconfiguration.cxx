/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <unotools/fontdefs.hxx>
#include <unotools/fontcfg.hxx>

class TestFontSubstConfiguration : public CppUnit::TestFixture
{
public:
    void testGetMapName_ShouldKillLeadingString();
    void testGetMapName_ShouldKillLeadingStringWhenSame();
    void testGetMapName_ShouldKillTrailingString();
    void testGetMapName_ShouldDetectWeight();
    void testGetMapName_ShouldNotDetectWeight();
    void testGetMapName_ShouldDetectWidth();
    void testGetMapName_ShouldDetectWidthOnEndOnly();
    void testGetMapName_ShouldNotDetectWidth();
    void testGetMapName_ShouldDetectFamilyType();
    void testGetMapName_ShouldNotDetectFamilyType();

    CPPUNIT_TEST_SUITE(TestFontSubstConfiguration);
    CPPUNIT_TEST(testGetMapName_ShouldKillLeadingString);
    CPPUNIT_TEST(testGetMapName_ShouldKillLeadingStringWhenSame);
    CPPUNIT_TEST(testGetMapName_ShouldKillTrailingString);
    CPPUNIT_TEST(testGetMapName_ShouldDetectWeight);
    CPPUNIT_TEST(testGetMapName_ShouldDetectWidthOnEndOnly);
    CPPUNIT_TEST(testGetMapName_ShouldNotDetectWeight);
    CPPUNIT_TEST(testGetMapName_ShouldDetectWidth);
    CPPUNIT_TEST(testGetMapName_ShouldNotDetectWidth);
    CPPUNIT_TEST(testGetMapName_ShouldDetectFamilyType);
    CPPUNIT_TEST(testGetMapName_ShouldNotDetectFamilyType);

    CPPUNIT_TEST_SUITE_END();
};

void TestFontSubstConfiguration::testGetMapName_ShouldKillLeadingString()
{
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName("microsofttestfont", rShortName, rFamilyName, eWeight,
                                            eWidth, eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("leading string not stripped", OUString("testfont"), rShortName);
}

void TestFontSubstConfiguration::testGetMapName_ShouldKillLeadingStringWhenSame()
{
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName("microsoft", rShortName, rFamilyName, eWeight, eWidth,
                                            eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("leading string not stripped (same as name to strip)",
                                 OUString(""), rShortName);
}

void TestFontSubstConfiguration::testGetMapName_ShouldKillTrailingString()
{
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName("testfontwe", rShortName, rFamilyName, eWeight, eWidth,
                                            eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("trailing string not stripped", OUString("testfont"), rShortName);
}

void TestFontSubstConfiguration::testGetMapName_ShouldDetectWeight()
{
    // NOTE: you must initialize eWeight with WEIGHT_DONTKNOW or WEIGHT_NORMAL before
    // trying to detect the font weight in the name
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName("testdemi", rShortName, rFamilyName, eWeight, eWidth,
                                            eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("family name not found", OUString("test"), rFamilyName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("weight not found", WEIGHT_SEMIBOLD, eWeight);
}

void TestFontSubstConfiguration::testGetMapName_ShouldNotDetectWeight()
{
    // NOTE: you must initialize eWeight with WEIGHT_DONTKNOW or WEIGHT_NORMAL before
    // trying to detect the font weight in the name
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName("demigod", rShortName, rFamilyName, eWeight, eWidth,
                                            eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("family name not found", OUString("demigod"), rFamilyName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("weight not found", WEIGHT_DONTKNOW, eWeight);
}

void TestFontSubstConfiguration::testGetMapName_ShouldDetectWidth()
{
    // NOTE: you must initialize eWeight with WEIGHT_DONTKNOW or WEIGHT_NORMAL before
    // trying to detect the font weight in the name
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName("testnarrow", rShortName, rFamilyName, eWeight, eWidth,
                                            eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("family name not found", OUString("test"), rFamilyName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width not found", WIDTH_CONDENSED, eWidth);
}

void TestFontSubstConfiguration::testGetMapName_ShouldNotDetectWidth()
{
    // NOTE: you must initialize eWeight with WEIGHT_DONTKNOW or WEIGHT_NORMAL before
    // trying to detect the font weight in the name
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName(GetEnglishSearchFontName("Picnic For Two _ Cosy"),
                                            rShortName, rFamilyName, eWeight, eWidth, eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("family name not found", OUString("picnicfortwocosy"),
                                 rFamilyName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width not found", WIDTH_DONTKNOW, eWidth);
}

void TestFontSubstConfiguration::testGetMapName_ShouldDetectWidthOnEndOnly()
{
    // NOTE: you must initialize eWeight with WEIGHT_DONTKNOW or WEIGHT_NORMAL before
    // trying to detect the font weight in the name
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName(GetEnglishSearchFontName("Picnic For Two _ Cosycn"),
                                            rShortName, rFamilyName, eWeight, eWidth, eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("family name not found", OUString("picnicfortwocosy"),
                                 rFamilyName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width not found", WIDTH_CONDENSED, eWidth);
}

void TestFontSubstConfiguration::testGetMapName_ShouldDetectFamilyType()
{
    // NOTE: you must initialize eWeight with WEIGHT_DONTKNOW or WEIGHT_NORMAL before
    // trying to detect the font weight in the name
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName("testold", rShortName, rFamilyName, eWeight, eWidth,
                                            eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("family name not found", OUString("test"), rFamilyName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("family type not found", ImplFontAttrs::OtherStyle, eType);
}

void TestFontSubstConfiguration::testGetMapName_ShouldNotDetectFamilyType()
{
    // NOTE: you must initialize eWeight with WEIGHT_DONTKNOW or WEIGHT_NORMAL before
    // trying to detect the font weight in the name
    OUString rShortName;
    OUString rFamilyName;
    FontWeight eWeight = WEIGHT_DONTKNOW;
    FontWidth eWidth = WIDTH_DONTKNOW;
    ImplFontAttrs eType = ImplFontAttrs::None;

    utl::FontSubstConfiguration::getMapName(GetEnglishSearchFontName("Sold Out"), rShortName,
                                            rFamilyName, eWeight, eWidth, eType);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("family name not found", OUString("soldout"), rFamilyName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("family type not found", ImplFontAttrs::None, eType);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestFontSubstConfiguration);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
