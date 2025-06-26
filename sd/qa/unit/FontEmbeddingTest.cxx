/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <test/unoapi_test.hxx>
#include <config_eot.h>
#include <config_fonts.h>
#include <vcl/embeddedfontshelper.hxx>

using namespace css;

class FontEmbeddingTest : public SdModelTestBase
{
public:
    FontEmbeddingTest()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

namespace
{
constexpr OString xPath_Presentation = "/p:presentation"_ostr;
constexpr OString xPath_EmbeddedFont
    = "/p:presentation/p:embeddedFontLst/p:embeddedFont/p:font"_ostr;
}

CPPUNIT_TEST_FIXTURE(FontEmbeddingTest, testRoundtripEmbeddedFontsPPTX)
{
#if ENABLE_EOT
    // Test Import first

    // Expect the font to not be available
    {
        OUString aUrl = EmbeddedFontsHelper::fontFileUrl(
            u"Boldonse", FAMILY_ROMAN, ITALIC_NONE, WEIGHT_NORMAL, PITCH_VARIABLE,
            EmbeddedFontsHelper::FontRights::ViewingAllowed);
        CPPUNIT_ASSERT(aUrl.isEmpty());
    }

    // Load the document
    createSdImpressDoc("BoldonseFontEmbedded.pptx");

// FIXME: for some reason it doesn't work in macOS (see testSubsettedFullEmbeddedFont for a similar case)
#if !defined(MACOSX)
    // Expect the font to be available now, if we imported the embedded fonts correctly
    {
        OUString aUrl = EmbeddedFontsHelper::fontFileUrl(
            u"Boldonse", FAMILY_ROMAN, ITALIC_NONE, WEIGHT_NORMAL, PITCH_VARIABLE,
            EmbeddedFontsHelper::FontRights::ViewingAllowed);

        CPPUNIT_ASSERT(!aUrl.isEmpty());
    }
#endif

    // Check the document settings for embedded fonts are as expected
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xFactory.is());
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xSettings.is());

        bool bEmbedFonts = false;
        bool bEmbedUsedOnly = false;
        bool bEmbedLatinScript = false;
        bool bEmbedAsianScript = false;
        bool bEmbedComplexScript = false;

        xSettings->getPropertyValue(u"EmbedFonts"_ustr) >>= bEmbedFonts;
        xSettings->getPropertyValue(u"EmbedOnlyUsedFonts"_ustr) >>= bEmbedUsedOnly;
        xSettings->getPropertyValue(u"EmbedLatinScriptFonts"_ustr) >>= bEmbedLatinScript;
        xSettings->getPropertyValue(u"EmbedAsianScriptFonts"_ustr) >>= bEmbedAsianScript;
        xSettings->getPropertyValue(u"EmbedComplexScriptFonts"_ustr) >>= bEmbedComplexScript;

        CPPUNIT_ASSERT_EQUAL(true, bEmbedFonts);
        CPPUNIT_ASSERT_EQUAL(true, bEmbedUsedOnly);
        CPPUNIT_ASSERT_EQUAL(true, bEmbedLatinScript);
        CPPUNIT_ASSERT_EQUAL(true, bEmbedAsianScript);
        CPPUNIT_ASSERT_EQUAL(true, bEmbedComplexScript);
    }

    // Test Export
    save(u"Impress Office Open XML"_ustr);

    // Check the exported document has the font
    {
        xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);
        assertXPath(pXmlDoc, xPath_Presentation, "embedTrueTypeFonts", u"1");
        assertXPath(pXmlDoc, xPath_EmbeddedFont + "[@typeface='Boldonse']");
    }
#endif
}

CPPUNIT_TEST_FIXTURE(FontEmbeddingTest, testExportEmbeddedFontsPPTX)
{
#if HAVE_MORE_FONTS

    createSdImpressDoc("TestEmbeddedFonts_DejaVu.odp");

    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(
        xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);

    // Check state of the font embedding settings
    CPPUNIT_ASSERT_EQUAL(false, xProps->getPropertyValue(u"EmbedFonts"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(false, xProps->getPropertyValue(u"EmbedOnlyUsedFonts"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue(u"EmbedLatinScriptFonts"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue(u"EmbedAsianScriptFonts"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(true,
                         xProps->getPropertyValue(u"EmbedComplexScriptFonts"_ustr).get<bool>());

    // Enable font embedding, only used and latin script fonts
    xProps->setPropertyValue(u"EmbedFonts"_ustr, uno::Any(true));
    xProps->setPropertyValue(u"EmbedOnlyUsedFonts"_ustr, uno::Any(true));
    xProps->setPropertyValue(u"EmbedLatinScriptFonts"_ustr, uno::Any(true));
    xProps->setPropertyValue(u"EmbedAsianScriptFonts"_ustr, uno::Any(false));
    xProps->setPropertyValue(u"EmbedComplexScriptFonts"_ustr, uno::Any(false));

    save(u"Impress Office Open XML"_ustr);

    // Check the exported document has the embedded fonts enabled and has entries
    {
        xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);
        assertXPath(pXmlDoc, xPath_Presentation, "embedTrueTypeFonts", u"1");
        assertXPath(pXmlDoc, xPath_EmbeddedFont + "[@typeface='DejaVu Sans']");
    }
#endif
}

CPPUNIT_TEST_FIXTURE(FontEmbeddingTest, testTdf167214)
{
    auto verify = [this]() {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);

        // Check state of the font embedding settings
        CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue(u"EmbedFonts"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(true,
                             xProps->getPropertyValue(u"EmbedOnlyUsedFonts"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(true,
                             xProps->getPropertyValue(u"EmbedLatinScriptFonts"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(true,
                             xProps->getPropertyValue(u"EmbedAsianScriptFonts"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(true,
                             xProps->getPropertyValue(u"EmbedComplexScriptFonts"_ustr).get<bool>());
    };

    createSdImpressDoc("pptx/tdf167214.pptx");
    verify();
    //Without the fix in place, it would crash at export time
    saveAndReload(u"Impress Office Open XML"_ustr);
    verify();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
