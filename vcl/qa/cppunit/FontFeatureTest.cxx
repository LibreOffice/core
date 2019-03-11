/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <config_features.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>

#include <vcl/font/Feature.hxx>
#include <vcl/font/FeatureParser.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>

class FontFeatureTest : public test::BootstrapFixture
{
public:
    FontFeatureTest()
        : BootstrapFixture(true, false)
    {
    }

    void testGetFontFeatures();
    void testParseFeature();

    CPPUNIT_TEST_SUITE(FontFeatureTest);
    CPPUNIT_TEST(testGetFontFeatures);
    CPPUNIT_TEST(testParseFeature);
    CPPUNIT_TEST_SUITE_END();
};

void FontFeatureTest::testGetFontFeatures()
{
// "Linux Libertine G" is a font bundled with LO, but sometimes the
// bundled fonts aren't available so we need to disable test in that case
#if HAVE_MORE_FONTS
    ScopedVclPtrInstance<VirtualDevice> aVDev(*Application::GetDefaultDevice(),
                                              DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    aVDev->SetOutputSizePixel(Size(10, 10));

    OUString aFontName("Linux Libertine G");
    if (aVDev->IsFontAvailable(aFontName))
        return; // Can't test this because the font is not available, so exit

    vcl::Font aFont = aVDev->GetFont();
    aFont.SetFamilyName(aFontName);
    aFont.SetWeight(FontWeight::WEIGHT_NORMAL);
    aFont.SetItalic(FontItalic::ITALIC_NORMAL);
    aFont.SetWidthType(FontWidth::WIDTH_NORMAL);
    aVDev->SetFont(aFont);

    std::vector<vcl::font::Feature> rFontFeatures;
    CPPUNIT_ASSERT(aVDev->GetFontFeatures(rFontFeatures));

    // We're interested only in defaults here
    std::vector<vcl::font::Feature> rDefaultFontFeatures;
    OUString aFeaturesString;
    for (vcl::font::Feature const& rFeature : rFontFeatures)
    {
        if (rFeature.m_aID.m_aScriptCode == vcl::font::featureCode("DFLT")
            && rFeature.m_aID.m_aLanguageCode == vcl::font::featureCode("dflt"))
        {
            rDefaultFontFeatures.push_back(rFeature);
            aFeaturesString += vcl::font::featureCodeAsString(rFeature.m_aID.m_aFeatureCode) + " ";
        }
    }

#if !defined(_WIN32)
    // periodically fails on windows tinderbox like tb77 with a value of 27
    CPPUNIT_ASSERT_EQUAL(size_t(53), rDefaultFontFeatures.size());

    OUString aExpectedFeaturesString = "c2sc case dlig fina frac hlig liga lnum "
                                       "locl onum pnum sa01 sa02 sa03 sa04 sa05 "
                                       "sa06 sa07 sa08 salt sinf smcp ss01 ss02 "
                                       "ss03 sups tnum zero ingl cpsp lith litt "
                                       "itlc para algn arti circ dash dbls foot "
                                       "frsp grkn hang lng minu nfsp name quot "
                                       "texm thou vari caps ligc ";

    // periodically fails on windows tinderbox like tb72 with a missing "ss02"
    CPPUNIT_ASSERT_EQUAL(aExpectedFeaturesString, aFeaturesString);

    // Check C2SC feature
    {
        // periodically fails on the Windows tb72 tinderbox with
        // equality assertion failed
        //- Expected: 1664250723
        //- Actual  : 1684826471
        vcl::font::Feature& rFeature = rDefaultFontFeatures[0];
        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("c2sc"), rFeature.m_aID.m_aFeatureCode);

        vcl::font::FeatureDefinition& rFracFeatureDefinition = rFeature.m_aDefinition;
        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("c2sc"), rFracFeatureDefinition.getCode());
        CPPUNIT_ASSERT(!rFracFeatureDefinition.getDescription().isEmpty());
        CPPUNIT_ASSERT_EQUAL(vcl::font::FeatureParameterType::BOOL,
                             rFracFeatureDefinition.getType());

        CPPUNIT_ASSERT_EQUAL(size_t(0), rFracFeatureDefinition.getEnumParameters().size());
    }
#endif

    // Check FRAC feature
    {
        vcl::font::Feature& rFeature = rDefaultFontFeatures[4];
        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("frac"), rFeature.m_aID.m_aFeatureCode);

        vcl::font::FeatureDefinition& rFracFeatureDefinition = rFeature.m_aDefinition;
        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("frac"), rFracFeatureDefinition.getCode());
        CPPUNIT_ASSERT(!rFracFeatureDefinition.getDescription().isEmpty());
        CPPUNIT_ASSERT_EQUAL(vcl::font::FeatureParameterType::ENUM,
                             rFracFeatureDefinition.getType());

        CPPUNIT_ASSERT_EQUAL(size_t(3), rFracFeatureDefinition.getEnumParameters().size());

        vcl::font::FeatureParameter const& rParameter1
            = rFracFeatureDefinition.getEnumParameters()[0];
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), rParameter1.getCode());
        CPPUNIT_ASSERT(!rParameter1.getDescription().isEmpty());

        vcl::font::FeatureParameter const& rParameter2
            = rFracFeatureDefinition.getEnumParameters()[1];
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), rParameter2.getCode());
        CPPUNIT_ASSERT(!rParameter2.getDescription().isEmpty());

        vcl::font::FeatureParameter const& rParameter3
            = rFracFeatureDefinition.getEnumParameters()[2];
        CPPUNIT_ASSERT_EQUAL(uint32_t(2), rParameter3.getCode());
        CPPUNIT_ASSERT(!rParameter2.getDescription().isEmpty());
    }
#endif // HAVE_MORE_FONTS
}

void FontFeatureTest::testParseFeature()
{
    { // No font features specified
        vcl::font::FeatureParser aParser("Font name with no features");
        CPPUNIT_ASSERT_EQUAL(size_t(0), aParser.getFeatures().size());
    }
    { // One feature specified, no value
        vcl::font::FeatureParser aParser("Font name:abcd");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
    }
    { // One feature specified, explicit value
        vcl::font::FeatureParser aParser("Font name:abcd=5");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(5), aFeatures[0].m_nValue);
    }
    { // One feature specified, explicit zero value
        vcl::font::FeatureParser aParser("Font name:abcd=0");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), aFeatures[0].m_nValue);
    }
    { // One feature specified, using plus prefix
        vcl::font::FeatureParser aParser("Font name:+abcd");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
    }
    { // One feature specified, using minus prefix
        vcl::font::FeatureParser aParser("Font name:-abcd");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), aFeatures[0].m_nValue);
    }
    { // One feature specified, with empty character range
        vcl::font::FeatureParser aParser("Font name:abcd[]");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(-1), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with empty character range
        vcl::font::FeatureParser aParser("Font name:abcd[:]");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(-1), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with start character range
        vcl::font::FeatureParser aParser("Font name:abcd[3:]");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(-1), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with end character range
        vcl::font::FeatureParser aParser("Font name:abcd[:3]");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with character range
        vcl::font::FeatureParser aParser("Font name:abcd[3:6]");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(6), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with character range
        vcl::font::FeatureParser aParser("Font name:abcd[3]");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with character range and value
        vcl::font::FeatureParser aParser("Font name:abcd[3:6]=2");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(2), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(6), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with character range and 0 value
        vcl::font::FeatureParser aParser("Font name:abcd[3:6]=0");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(6), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with character range and minus prefix
        vcl::font::FeatureParser aParser("Font name:-abcd[3:6]");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), aFeatures[0].m_nValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), aFeatures[0].m_nStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(6), aFeatures[0].m_nEnd);
    }
    { // One feature specified, with CSS on
        vcl::font::FeatureParser aParser("Font name:\"abcd\" on");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);
    }
    { // One feature specified, with CSS off
        vcl::font::FeatureParser aParser("Font name:'abcd' off");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), aFeatures[0].m_nValue);
    }
    { // One feature specified, with CSS value
        vcl::font::FeatureParser aParser("Font name:\"abcd\" 2");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(2), aFeatures[0].m_nValue);
    }
    { // Multiple features specified, no values
        vcl::font::FeatureParser aParser("Font name:abcd&bcde&efgh");
        CPPUNIT_ASSERT_EQUAL(size_t(3), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("bcde"), aFeatures[1].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[1].m_nValue);

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("efgh"), aFeatures[2].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[2].m_nValue);
    }
    {
        // Multiple features specified, explicit values
        // Only 4 char parameter names supported - "toolong" is too long and ignored
        vcl::font::FeatureParser aParser("Font name:abcd=1&bcde=0&toolong=1&cdef=3");
        CPPUNIT_ASSERT_EQUAL(size_t(3), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("bcde"), aFeatures[1].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), aFeatures[1].m_nValue);

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("cdef"), aFeatures[2].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(3), aFeatures[2].m_nValue);
    }
    {
        // Special case - "lang" is parsed specially and access separately not as a feature.

        vcl::font::FeatureParser aParser("Font name:abcd=1&lang=slo");
        CPPUNIT_ASSERT_EQUAL(size_t(1), aParser.getFeatures().size());
        auto aFeatures = aParser.getFeatures();

        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("abcd"), aFeatures[0].m_nTag);
        CPPUNIT_ASSERT_EQUAL(uint32_t(1), aFeatures[0].m_nValue);

        CPPUNIT_ASSERT_EQUAL(OUString("slo"), aParser.getLanguage());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(FontFeatureTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
