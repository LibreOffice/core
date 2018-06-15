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
#include <vcl/font/Feature.hxx>

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

    CPPUNIT_TEST_SUITE(FontFeatureTest);
    CPPUNIT_TEST(testGetFontFeatures);
    CPPUNIT_TEST_SUITE_END();
};

void FontFeatureTest::testGetFontFeatures()
{
    ScopedVclPtrInstance<VirtualDevice> aVDev(*Application::GetDefaultDevice(),
                                              DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    aVDev->SetOutputSizePixel(Size(10, 10));

    vcl::Font aFont = aVDev->GetFont();
    aFont.SetFamilyName("Linux Libertine G");
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

    CPPUNIT_ASSERT_EQUAL(size_t(31), rDefaultFontFeatures.size());

    OUString aExpectedFeaturesString = "aalt c2sc case ccmp dlig fina frac hlig "
                                       "liga lnum nalt onum pnum salt sinf smcp "
                                       "ss01 ss02 ss03 ss04 ss05 ss06 sups tnum "
                                       "zero cpsp kern lfbd mark mkmk rtbd ";
    CPPUNIT_ASSERT_EQUAL(aExpectedFeaturesString, aFeaturesString);

    // Check C2SC feature
    {
        vcl::font::Feature& rFeature = rDefaultFontFeatures[1];
        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("c2sc"), rFeature.m_aID.m_aFeatureCode);

        vcl::font::FeatureDefinition& rFracFeatureDefinition = rFeature.m_aDefinition;
        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("c2sc"), rFracFeatureDefinition.getCode());
        CPPUNIT_ASSERT(!rFracFeatureDefinition.getDescription().isEmpty());
        CPPUNIT_ASSERT_EQUAL(vcl::font::FeatureParameterType::BOOL,
                             rFracFeatureDefinition.getType());

        CPPUNIT_ASSERT_EQUAL(size_t(0), rFracFeatureDefinition.getEnumParameters().size());
    }

    // Check FRAC feature
    {
        vcl::font::Feature& rFeature = rDefaultFontFeatures[6];
        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("frac"), rFeature.m_aID.m_aFeatureCode);

        vcl::font::FeatureDefinition& rFracFeatureDefinition = rFeature.m_aDefinition;
        CPPUNIT_ASSERT_EQUAL(vcl::font::featureCode("frac"), rFracFeatureDefinition.getCode());
        CPPUNIT_ASSERT(!rFracFeatureDefinition.getDescription().isEmpty());
        CPPUNIT_ASSERT_EQUAL(vcl::font::FeatureParameterType::ENUM,
                             rFracFeatureDefinition.getType());

        CPPUNIT_ASSERT_EQUAL(size_t(2), rFracFeatureDefinition.getEnumParameters().size());

        vcl::font::FeatureParameter const& rParameter1
            = rFracFeatureDefinition.getEnumParameters()[0];
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), rParameter1.getCode());
        CPPUNIT_ASSERT(!rParameter1.getDescription().isEmpty());

        vcl::font::FeatureParameter const& rParameter2
            = rFracFeatureDefinition.getEnumParameters()[1];
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(2), rParameter2.getCode());
        CPPUNIT_ASSERT(!rParameter2.getDescription().isEmpty());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(FontFeatureTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
