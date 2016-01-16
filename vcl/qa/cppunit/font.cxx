/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>

#include <osl/file.hxx>
#include <osl/process.h>

#include <vcl/font.hxx>

class VclFontTest : public test::BootstrapFixture
{
public:
    VclFontTest() : BootstrapFixture(true, false) {}

    void testName();
    void testWeight();
    void testSymbolFlagAndCharSet();

    CPPUNIT_TEST_SUITE(VclFontTest);
    CPPUNIT_TEST(testName);
    CPPUNIT_TEST(testWeight);
    CPPUNIT_TEST(testSymbolFlagAndCharSet);
    CPPUNIT_TEST_SUITE_END();
};

void VclFontTest::testName()
{
    vcl::Font aFont;

    CPPUNIT_ASSERT_MESSAGE( "Family name should be empty", aFont.GetName().isEmpty());
    CPPUNIT_ASSERT_MESSAGE( "Style name should be empty", aFont.GetStyleName().isEmpty());
    aFont.SetName("Test family name");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Family name should not be empty", OUString("Test family name"), aFont.GetName());
    aFont.SetStyleName("Test style name");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Style name should not be empty", OUString("Test style name"), aFont.GetStyleName());
}

void VclFontTest::testWeight()
{
    vcl::Font aFont;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Weight should be WEIGHT_DONTKNOW", FontWeight::WEIGHT_DONTKNOW, aFont.GetWeight());

    aFont.SetWeight(FontWeight::WEIGHT_BLACK);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Weight should be WEIGHT_BLACK", FontWeight::WEIGHT_BLACK, aFont.GetWeight());
}


void VclFontTest::testSymbolFlagAndCharSet()
{
    // default constructor should set scalable flag to false
    vcl::Font aFont;

    CPPUNIT_ASSERT_MESSAGE( "Should not be seen as a symbol font after default constructor called", !aFont.IsSymbolFont() );
    CPPUNIT_ASSERT_MESSAGE( "Character set should be RTL_TEXTENCODING_DONTKNOW after default constructor called",
                            aFont.GetCharSet() == RTL_TEXTENCODING_DONTKNOW );

    aFont.SetSymbolFlag(true);

    CPPUNIT_ASSERT_MESSAGE( "Test 1: Symbol font flag should be on", aFont.IsSymbolFont() );
    CPPUNIT_ASSERT_MESSAGE( "Test 1: Character set should be RTL_TEXTENCODING_SYMBOL",
                            aFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL );

    aFont.SetSymbolFlag(false);

    CPPUNIT_ASSERT_MESSAGE( "Test 2: Symbol font flag should be off", !aFont.IsSymbolFont() );
    CPPUNIT_ASSERT_MESSAGE( "Test 2: Character set should be RTL_TEXTENCODING_DONTKNOW",
                            aFont.GetCharSet() == RTL_TEXTENCODING_DONTKNOW );

    aFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );

    CPPUNIT_ASSERT_MESSAGE( "Test 3: Symbol font flag should be on", aFont.IsSymbolFont() );
    CPPUNIT_ASSERT_MESSAGE( "Test 3: Character set should be RTL_TEXTENCODING_SYMBOL",
                            aFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL );

    aFont.SetCharSet( RTL_TEXTENCODING_UNICODE );

    CPPUNIT_ASSERT_MESSAGE( "Test 4: Symbol font flag should be off", !aFont.IsSymbolFont() );
    CPPUNIT_ASSERT_MESSAGE( "Test 4: Character set should be RTL_TEXTENCODING_UNICODE",
                            aFont.GetCharSet() == RTL_TEXTENCODING_UNICODE );
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclFontTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
