/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <rtl/ustring.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>

using namespace com::sun::star;

namespace {

class TestLanguageTag : public CppUnit::TestFixture
{
public:
    TestLanguageTag() {}

    void testAllTags();
    void testAllIsoLangEntries();

    CPPUNIT_TEST_SUITE(TestLanguageTag);
    CPPUNIT_TEST(testAllTags);
    CPPUNIT_TEST(testAllIsoLangEntries);
    CPPUNIT_TEST_SUITE_END();
};

void TestLanguageTag::testAllTags()
{
    {
        LanguageTag de_DE( "de-Latn-DE", true );
        OUString aBcp47 = de_DE.getBcp47();
        lang::Locale aLocale = de_DE.getLocale();
        LanguageType nLanguageType = de_DE.getLanguageType();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Default script should be stripped after canonicalize.", OUString("de-DE"), aBcp47 );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("DE"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, nLanguageType );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), de_DE.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("DE"), de_DE.getCountry() );
        CPPUNIT_ASSERT( de_DE.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), de_DE.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), de_DE.makeFallback().getBcp47() );
    }

    {
        LanguageTag klingon( "i-klingon", true );
        lang::Locale aLocale = klingon.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("tlh"), klingon.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("tlh"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( klingon.getLanguageType()) );
        CPPUNIT_ASSERT( klingon.isValidBcp47() );
        CPPUNIT_ASSERT( klingon.isIsoLocale() );
        CPPUNIT_ASSERT( klingon.isIsoODF() );
        LanguageType nLang = klingon.getLanguageType();
        LanguageTag klingon_id( nLang);
        CPPUNIT_ASSERT_EQUAL( OUString("tlh"), klingon_id.getBcp47() );
    }

    {
        OUString s_sr_RS( "sr-RS" );
        LanguageTag sr_RS( s_sr_RS, true );
        lang::Locale aLocale = sr_RS.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_sr_RS, sr_RS.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("sr"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("RS"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_SERBIAN_CYRILLIC_SERBIA, sr_RS.getLanguageType() );
        CPPUNIT_ASSERT( sr_RS.isValidBcp47() );
        CPPUNIT_ASSERT( sr_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_RS.isIsoODF() );
    }

    {
        OUString s_sr_Latn_RS( "sr-Latn-RS" );
        LanguageTag sr_RS( s_sr_Latn_RS, true );
        lang::Locale aLocale = sr_RS.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_sr_Latn_RS, sr_RS.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("RS"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_sr_Latn_RS, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_SERBIAN_LATIN_SERBIA, sr_RS.getLanguageType() );
        CPPUNIT_ASSERT( sr_RS.isValidBcp47() );
        CPPUNIT_ASSERT( !sr_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_RS.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("sr"), sr_RS.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("RS"), sr_RS.getCountry() );
        CPPUNIT_ASSERT_EQUAL( OUString("Latn"), sr_RS.getScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn"), sr_RS.getLanguageAndScript() );
    }

    {
        OUString s_sr_Latn_CS( "sr-Latn-CS" );
        LanguageTag sr_Latn_CS( s_sr_Latn_CS, true );
        lang::Locale aLocale = sr_Latn_CS.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_sr_Latn_CS, sr_Latn_CS.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("CS"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_sr_Latn_CS, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_SERBIAN_LATIN_SAM, sr_Latn_CS.getLanguageType() );
        CPPUNIT_ASSERT( sr_Latn_CS.isValidBcp47() );
        CPPUNIT_ASSERT( !sr_Latn_CS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_Latn_CS.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("sr"), sr_Latn_CS.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("CS"), sr_Latn_CS.getCountry() );
        CPPUNIT_ASSERT_EQUAL( OUString("Latn"), sr_Latn_CS.getScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn"), sr_Latn_CS.getLanguageAndScript() );
        ::std::vector< OUString > sr_Latn_CS_Fallbacks( sr_Latn_CS.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(9), sr_Latn_CS_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn-CS"), sr_Latn_CS_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn-YU"), sr_Latn_CS_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("sh-CS"), sr_Latn_CS_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( OUString("sh-YU"), sr_Latn_CS_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn"), sr_Latn_CS_Fallbacks[4]);
        CPPUNIT_ASSERT_EQUAL( OUString("sh"), sr_Latn_CS_Fallbacks[5]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr-CS"), sr_Latn_CS_Fallbacks[6]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr-YU"), sr_Latn_CS_Fallbacks[7]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr"), sr_Latn_CS_Fallbacks[8]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn-CS"), sr_Latn_CS.makeFallback().getBcp47());
    }

    // 'sh-RS' has an internal override to 'sr-Latn-RS'
    {
        LanguageTag sh_RS( "sh-RS", true );
        lang::Locale aLocale = sh_RS.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn-RS"), sh_RS.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString(I18NLANGTAG_QLT) , aLocale.Language);
        CPPUNIT_ASSERT_EQUAL( OUString("RS"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn-RS"), aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_SERBIAN_LATIN_SERBIA, sh_RS.getLanguageType() );
        CPPUNIT_ASSERT( sh_RS.isValidBcp47() );
        CPPUNIT_ASSERT( !sh_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sh_RS.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("sr"), sh_RS.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("RS"), sh_RS.getCountry() );
        CPPUNIT_ASSERT_EQUAL( OUString("Latn"), sh_RS.getScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn"), sh_RS.getLanguageAndScript() );
        ::std::vector< OUString > sh_RS_Fallbacks( sh_RS.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(6), sh_RS_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn-RS"), sh_RS_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("sh-RS"), sh_RS_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn"), sh_RS_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( OUString("sh"), sh_RS_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr-RS"), sh_RS_Fallbacks[4]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr"), sh_RS_Fallbacks[5]);
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn-RS"), sh_RS.makeFallback().getBcp47());
        CPPUNIT_ASSERT_EQUAL( OUString("sr-Latn-RS"), sh_RS.getBcp47());
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_SERBIAN_LATIN_SERBIA, sh_RS.getLanguageType() );
    }

    // 'bs-Latn-BA' with 'Latn' suppress-script, we map that ourselves for a
    // known LangID with an override and canonicalization should work the same
    // without liblangtag.
    {
        LanguageTag bs_Latn_BA( "bs-Latn-BA", true );
        lang::Locale aLocale = bs_Latn_BA.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("bs-BA"), bs_Latn_BA.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("bs"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("BA"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA, bs_Latn_BA.getLanguageType() );
        CPPUNIT_ASSERT( bs_Latn_BA.isValidBcp47() );
        CPPUNIT_ASSERT( bs_Latn_BA.isIsoLocale() );
        CPPUNIT_ASSERT( bs_Latn_BA.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("bs"), bs_Latn_BA.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("BA"), bs_Latn_BA.getCountry() );
        CPPUNIT_ASSERT( bs_Latn_BA.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("bs"), bs_Latn_BA.getLanguageAndScript() );
        ::std::vector< OUString > bs_Latn_BA_Fallbacks( bs_Latn_BA.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), bs_Latn_BA_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( OUString("bs-BA"), bs_Latn_BA_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("bs"), bs_Latn_BA_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("bs-BA"), bs_Latn_BA.makeFallback().getBcp47());
        CPPUNIT_ASSERT_EQUAL( OUString("bs-BA"), bs_Latn_BA.getBcp47());
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA, bs_Latn_BA.getLanguageType() );
    }

    {
        OUString s_ca_ES_valencia( "ca-ES-valencia" );
        LanguageTag ca_ES_valencia( s_ca_ES_valencia, true );
        lang::Locale aLocale = ca_ES_valencia.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_ca_ES_valencia, ca_ES_valencia.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("ES"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_ca_ES_valencia, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_CATALAN_VALENCIAN, ca_ES_valencia.getLanguageType() );
        CPPUNIT_ASSERT( ca_ES_valencia.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_ES_valencia.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_ES_valencia.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_ES_valencia.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("ES"), ca_ES_valencia.getCountry() );
        CPPUNIT_ASSERT( ca_ES_valencia.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_ES_valencia.getLanguageAndScript() );
        ::std::vector< OUString > ca_ES_valencia_Fallbacks( ca_ES_valencia.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), ca_ES_valencia_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( OUString("ca-ES-valencia"), ca_ES_valencia_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-XV"), ca_ES_valencia_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-valencia"), ca_ES_valencia_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-ES"), ca_ES_valencia_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_ES_valencia_Fallbacks[4]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-ES-valencia"), ca_ES_valencia.makeFallback().getBcp47());
    }

    {
        OUString s_ca_valencia( "ca-valencia" );
        LanguageTag ca_valencia( s_ca_valencia, true );
        lang::Locale aLocale = ca_valencia.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_ca_valencia, ca_valencia.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( s_ca_valencia, aLocale.Variant );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( ca_valencia.getLanguageType()) );
        CPPUNIT_ASSERT( ca_valencia.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_valencia.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_valencia.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_valencia.getLanguage() );
        CPPUNIT_ASSERT( ca_valencia.getCountry().isEmpty() );
        CPPUNIT_ASSERT( ca_valencia.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_valencia.getLanguageAndScript() );
        ::std::vector< OUString > ca_valencia_Fallbacks( ca_valencia.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), ca_valencia_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( OUString("ca-valencia"), ca_valencia_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_valencia_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-ES-valencia"), ca_valencia.makeFallback().getBcp47());
    }

    // 'ca-XV' has an internal override to 'ca-ES-valencia'
    {
        OUString s_ca_ES_valencia( "ca-ES-valencia" );
        LanguageTag ca_XV( "ca-XV", true );
        lang::Locale aLocale = ca_XV.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_ca_ES_valencia, ca_XV.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString(I18NLANGTAG_QLT) , aLocale.Language);
        CPPUNIT_ASSERT_EQUAL( OUString("ES"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_ca_ES_valencia, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_CATALAN_VALENCIAN, ca_XV.getLanguageType() );
        CPPUNIT_ASSERT( ca_XV.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_XV.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_XV.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_XV.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("ES"), ca_XV.getCountry() );
        CPPUNIT_ASSERT( ca_XV.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_XV.getLanguageAndScript() );
        ::std::vector< OUString > ca_XV_Fallbacks( ca_XV.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), ca_XV_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( OUString("ca-ES-valencia"), ca_XV_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-XV"), ca_XV_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-valencia"), ca_XV_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-ES"), ca_XV_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca"), ca_XV_Fallbacks[4]);
        CPPUNIT_ASSERT_EQUAL( OUString("ca-ES-valencia"), ca_XV.makeFallback().getBcp47());
    }

    {
        OUString s_de_DE( "de-DE" );
        LanguageTag de_DE( s_de_DE, true );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_de_DE, de_DE.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("DE"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, de_DE.getLanguageType() );
        CPPUNIT_ASSERT( de_DE.isValidBcp47() );
        CPPUNIT_ASSERT( de_DE.isIsoLocale() );
        CPPUNIT_ASSERT( de_DE.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), de_DE.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("DE"), de_DE.getCountry() );
        CPPUNIT_ASSERT( de_DE.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), de_DE.getLanguageAndScript() );
        ::std::vector< OUString > de_DE_Fallbacks( de_DE.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), de_DE_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), de_DE_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("de"), de_DE_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), de_DE.makeFallback().getBcp47());
    }

    {
        LanguageTag de_DE( lang::Locale( "de", "DE", "" ) );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), de_DE.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("DE"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, de_DE.getLanguageType() );
    }

    {
        LanguageTag de_DE( LANGUAGE_GERMAN );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), de_DE.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("DE"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, de_DE.getLanguageType() );
    }

    // Unmapped but known language-only.
    {
        OUString s_de( "de" );
        LanguageTag de( s_de, true );
        lang::Locale aLocale = de.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_de, de.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        LanguageType de_LangID = de.getLanguageType();
        CPPUNIT_ASSERT( de_LangID != LANGUAGE_GERMAN );
        CPPUNIT_ASSERT_EQUAL( MsLangId::getPrimaryLanguage( LANGUAGE_GERMAN) , de_LangID);
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), de.makeFallback().getBcp47());
        // Check registered mapping.
        LanguageTag de_l( de_LangID);
        CPPUNIT_ASSERT_EQUAL( s_de, de_l.getBcp47() );
    }

    // "bo" and "dz" share the same primary language ID, only one gets it
    // assigned, "dz" language-only has a special mapping.
    {
        LanguageTag bo( "bo", true );
        CPPUNIT_ASSERT_EQUAL( MsLangId::getPrimaryLanguage( LANGUAGE_TIBETAN), bo.getLanguageType() );
        LanguageTag dz( "dz", true );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_DZONGKHA_MAP_LONLY, dz.getLanguageType() );
    }

    // "no", "nb" and "nn" share the same primary language ID, which even is
    // assigned to "no-NO" for legacy so none gets it assigned, all on-the-fly
    // except if there is a defined MS-LCID for LanguageScriptOnly (LSO).
    {
        LanguageTag no( "no", true );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( no.getLanguageType()) );
        LanguageTag nb( "nb", true );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_NORWEGIAN_BOKMAL_LSO, nb.getLanguageType() );
        LanguageTag nn( "nn", true );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_NORWEGIAN_NYNORSK_LSO, nn.getLanguageType() );
        LanguageTag no_NO( "no-NO", true );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_NORWEGIAN, no_NO.getLanguageType() );
    }

    // 'de-1901' derived from 'de-DE-1901' grandfathered to check that it is
    // accepted as (DIGIT 3ALNUM) variant
    {
        OUString s_de_1901( "de-1901" );
        LanguageTag de_1901( s_de_1901 );
        lang::Locale aLocale = de_1901.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_de_1901, de_1901.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( s_de_1901, aLocale.Variant );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( de_1901.getLanguageType()) );
        CPPUNIT_ASSERT( de_1901.isValidBcp47() );
        CPPUNIT_ASSERT( !de_1901.isIsoLocale() );
        CPPUNIT_ASSERT( !de_1901.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), de_1901.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("1901"), de_1901.getVariants() );
        ::std::vector< OUString > de_1901_Fallbacks( de_1901.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), de_1901_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( OUString("de-1901"), de_1901_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("de"), de_1901_Fallbacks[1]);
    }

    // 'en-GB-oed' is known grandfathered for English, Oxford English
    // Dictionary spelling.
    // Deprecated as of 2015-04-17, prefer en-GB-oxendict instead.
    // As of 2017-03-14 we also alias to en-GB-oxendict.
    {
        OUString s_en_GB_oxendict( "en-GB-oxendict" );
        LanguageTag en_GB_oed( "en-GB-oed" );
        lang::Locale aLocale = en_GB_oed.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_en_GB_oxendict, en_GB_oed.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("GB"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_en_GB_oxendict, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_ENGLISH_UK_OXENDICT, en_GB_oed.getLanguageType() );
        CPPUNIT_ASSERT( en_GB_oed.isValidBcp47() );
        CPPUNIT_ASSERT( !en_GB_oed.isIsoLocale() );
        CPPUNIT_ASSERT( !en_GB_oed.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("en"), en_GB_oed.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("oxendict"), en_GB_oed.getVariants() );
        ::std::vector< OUString > en_GB_oed_Fallbacks( en_GB_oed.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), en_GB_oed_Fallbacks.size() );
        CPPUNIT_ASSERT_EQUAL( OUString("en-GB-oxendict"), en_GB_oed_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("en-GB-oed"), en_GB_oed_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("en-oxendict"), en_GB_oed_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( OUString("en-GB"), en_GB_oed_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( OUString("en"), en_GB_oed_Fallbacks[4]);
        // 'en-oed' is not a valid fallback!
    }

    // 'en-GB-oxendict' as preferred over 'en-GB-oed'.
    {
        OUString s_en_GB_oxendict( "en-GB-oxendict" );
        LanguageTag en_GB_oxendict( s_en_GB_oxendict );
        lang::Locale aLocale = en_GB_oxendict.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_en_GB_oxendict, en_GB_oxendict.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("GB"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_en_GB_oxendict, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_ENGLISH_UK_OXENDICT, en_GB_oxendict.getLanguageType() );
        CPPUNIT_ASSERT( en_GB_oxendict.isValidBcp47() );
        CPPUNIT_ASSERT( !en_GB_oxendict.isIsoLocale() );
        CPPUNIT_ASSERT( !en_GB_oxendict.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("en"), en_GB_oxendict.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("oxendict"), en_GB_oxendict.getVariants() );
        ::std::vector< OUString > en_GB_oxendict_Fallbacks( en_GB_oxendict.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), en_GB_oxendict_Fallbacks.size() );
        CPPUNIT_ASSERT_EQUAL( OUString("en-GB-oxendict"), en_GB_oxendict_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("en-GB-oed"), en_GB_oxendict_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("en-oxendict"), en_GB_oxendict_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( OUString("en-GB"), en_GB_oxendict_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( OUString("en"), en_GB_oxendict_Fallbacks[4]);
    }

    // 'es-ES-u-co-trad' is a valid (and known) Extension U tag
    {
        OUString s_es_ES_u_co_trad( "es-ES-u-co-trad" );
        LanguageTag es_ES_u_co_trad( s_es_ES_u_co_trad );
        lang::Locale aLocale = es_ES_u_co_trad.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_es_ES_u_co_trad, es_ES_u_co_trad.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("ES"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_es_ES_u_co_trad, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_SPANISH_DATED, es_ES_u_co_trad.getLanguageType() );
        CPPUNIT_ASSERT( es_ES_u_co_trad.isValidBcp47() );
        CPPUNIT_ASSERT( !es_ES_u_co_trad.isIsoLocale() );
        CPPUNIT_ASSERT( !es_ES_u_co_trad.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("es"), es_ES_u_co_trad.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("u-co-trad"), es_ES_u_co_trad.getVariants() );
        ::std::vector< OUString > es_ES_u_co_trad_Fallbacks( es_ES_u_co_trad.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(4), es_ES_u_co_trad_Fallbacks.size() );
        CPPUNIT_ASSERT_EQUAL( OUString("es-ES-u-co-trad"), es_ES_u_co_trad_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("es-u-co-trad"), es_ES_u_co_trad_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("es-ES"), es_ES_u_co_trad_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( OUString("es"), es_ES_u_co_trad_Fallbacks[3]);
        // Map to broken MS.
        CPPUNIT_ASSERT_EQUAL( OUString("es-ES_tradnl"), es_ES_u_co_trad.getBcp47MS() );
    }

    // 'es-ES_tradnl' (broken MS) maps to 'es-ES-u-co-trad'
    {
        OUString s_es_ES_u_co_trad( "es-ES-u-co-trad" );
        OUString s_es_ES_tradnl( "es-ES_tradnl" );
        LanguageTag es_ES_tradnl( s_es_ES_tradnl );
        lang::Locale aLocale = es_ES_tradnl.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_es_ES_u_co_trad, es_ES_tradnl.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("ES"), aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_es_ES_u_co_trad, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_SPANISH_DATED, es_ES_tradnl.getLanguageType() );
        CPPUNIT_ASSERT( es_ES_tradnl.isValidBcp47() );
        CPPUNIT_ASSERT( !es_ES_tradnl.isIsoLocale() );
        CPPUNIT_ASSERT( !es_ES_tradnl.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( OUString("es"), es_ES_tradnl.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( OUString("u-co-trad"), es_ES_tradnl.getVariants() );
        ::std::vector< OUString > es_ES_tradnl_Fallbacks( es_ES_tradnl.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(4), es_ES_tradnl_Fallbacks.size() );
        CPPUNIT_ASSERT_EQUAL( OUString("es-ES-u-co-trad"), es_ES_tradnl_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( OUString("es-u-co-trad"), es_ES_tradnl_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( OUString("es-ES"), es_ES_tradnl_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( OUString("es"), es_ES_tradnl_Fallbacks[3]);
        // Map back to broken MS.
        CPPUNIT_ASSERT_EQUAL( s_es_ES_tradnl, es_ES_tradnl.getBcp47MS() );
    }

    // 'zh-yue-HK' uses redundant 'zh-yue' and should be preferred 'yue-HK'
#if 0
    /* XXX Disabled because liblangtag in lt_tag_canonicalize() after replacing
     * 'zh-yue' with the preferred 'yue' does:
     * "If the language tag starts with a primary language subtag that is also
     * an extlang subtag, then the language tag is prepended with the extlang's
     * 'Prefix'."
     * Primary language 'yue' is also extlang 'yue' for which the prefix
     * happens to be 'zh' ... so the result is 'zh-yue-HK' again. */
    {
        OUString s_zh_yue_HK( "zh-yue-HK" );
        LanguageTag zh_yue_HK( s_zh_yue_HK );
        lang::Locale aLocale = zh_yue_HK.getLocale();
        CPPUNIT_ASSERT( zh_yue_HK.getBcp47() == "yue-HK" );
        CPPUNIT_ASSERT( aLocale.Language == "yue" );
        CPPUNIT_ASSERT( aLocale.Country == "HK" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( zh_yue_HK.getLanguageType() == LANGUAGE_YUE_CHINESE_HONGKONG );
        CPPUNIT_ASSERT( zh_yue_HK.isValidBcp47() == true );
        CPPUNIT_ASSERT( zh_yue_HK.isIsoLocale() == true );
        CPPUNIT_ASSERT( zh_yue_HK.isIsoODF() == true );
        CPPUNIT_ASSERT( zh_yue_HK.getLanguageAndScript() == "yue" );
        CPPUNIT_ASSERT( zh_yue_HK.getVariants() == "" );
        ::std::vector< OUString > zh_yue_HK_Fallbacks( zh_yue_HK.getFallbackStrings( true));
        CPPUNIT_ASSERT( zh_yue_HK_Fallbacks.size() == 2);
        CPPUNIT_ASSERT( zh_yue_HK_Fallbacks[0] == "yue-HK");
        CPPUNIT_ASSERT( zh_yue_HK_Fallbacks[1] == "yue");
    }
#endif

    // 'qtz' is a local use known pseudolocale for key ID resource
    {
        OUString s_qtz( "qtz" );
        LanguageTag qtz( s_qtz );
        lang::Locale aLocale = qtz.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_qtz, qtz.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qtz"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_KEYID, qtz.getLanguageType() );
    }

    // 'qty' is a local use unknown locale
    {
        OUString s_qty( "qty" );
        LanguageTag qty( s_qty );
        lang::Locale aLocale = qty.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_qty, qty.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qty"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( qty.getLanguageType()) );
    }

    // 'qtx' is an unknown new mslangid
    {
        LanguageTag qtx( "qtx" );
        qtx.setScriptType( LanguageTag::ScriptType::RTL );
        LanguageType n_qtx = qtx.getLanguageType();
        CPPUNIT_ASSERT_EQUAL( css::i18n::ScriptType::COMPLEX, MsLangId::getScriptType(n_qtx) );
        CPPUNIT_ASSERT( MsLangId::isRightToLeft(n_qtx) );
        CPPUNIT_ASSERT( !MsLangId::isCJK(n_qtx) );
    }

    // 'x-comment' is a privateuse known "locale"
    {
        OUString s_xcomment( "x-comment" );
        LanguageTag xcomment( s_xcomment );
        lang::Locale aLocale = xcomment.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_xcomment, xcomment.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("x-comment"), aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_PRIV_COMMENT, xcomment.getLanguageType() );
    }

    // 'x-foobar' is a privateuse unknown "locale"
    {
        OUString s_xfoobar( "x-foobar" );
        LanguageTag xfoobar( s_xfoobar );
        lang::Locale aLocale = xfoobar.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_xfoobar, xfoobar.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("x-foobar"), aLocale.Variant );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( xfoobar.getLanguageType()) );
    }

    // '*' the dreaded jolly joker is a "privateuse" known "locale"
    {
        OUString s_joker( "*" );
        LanguageTag joker( s_joker );
        lang::Locale aLocale = joker.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_joker, joker.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("*"), aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_PRIV_JOKER, joker.getLanguageType() );

        joker.reset( LANGUAGE_USER_PRIV_JOKER );
        aLocale = joker.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_joker, joker.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( OUString("*"), aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_PRIV_JOKER, joker.getLanguageType() );
    }

    // 'C' locale shall map to 'en-US'
    {
        LanguageTag aTag( "C" );
        CPPUNIT_ASSERT_EQUAL( OUString("en"), aTag.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("US"), aTag.getCountry() );
        lang::Locale aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("en-US"), aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("en"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("US"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_ENGLISH_US, aTag.getLanguageType() );
    }
    {
        LanguageTag aTag( lang::Locale("C","","") );
        CPPUNIT_ASSERT_EQUAL( OUString("en"), aTag.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( OUString("US"), aTag.getCountry() );
        lang::Locale aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("en-US"), aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("en"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("US"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_ENGLISH_US, aTag.getLanguageType() );
    }

    // test reset() methods
    {
        LanguageTag aTag( LANGUAGE_DONTKNOW );
        lang::Locale aLocale;

        aTag.reset( LANGUAGE_GERMAN );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("DE"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, aTag.getLanguageType() );

        aTag.reset( "en-US" );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("en-US"), aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("en"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("US"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_ENGLISH_US, aTag.getLanguageType() );

        aTag.reset( lang::Locale( "de", "DE", "" ) );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("de"), aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( OUString("DE"), aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, aTag.getLanguageType() );
    }

    {
        OUString s_uab( "unreg-and-bad" );
        LanguageTag uab( s_uab, true );
        lang::Locale aLocale = uab.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_uab, uab.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( OUString("qlt"), aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( s_uab, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_DONTKNOW, uab.getLanguageType() );
        CPPUNIT_ASSERT( !uab.isValidBcp47() );
        CPPUNIT_ASSERT( !uab.isIsoLocale() );
        CPPUNIT_ASSERT( !uab.isIsoODF() );
    }

    // test static isValidBcp47() method
    {
        OUString aCanonicalized;
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "en-US", &aCanonicalized) );
        CPPUNIT_ASSERT_EQUAL( OUString("en-US"), aCanonicalized );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "x-foobar", &aCanonicalized) );
        CPPUNIT_ASSERT_EQUAL( OUString("x-foobar"), aCanonicalized );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "qaa", &aCanonicalized) );
        CPPUNIT_ASSERT_EQUAL( OUString("qaa"), aCanonicalized );
        CPPUNIT_ASSERT( !LanguageTag::isValidBcp47( "unreg-and-bad", &aCanonicalized) );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "en-US", &aCanonicalized, true) );
        CPPUNIT_ASSERT_EQUAL( OUString("en-US"), aCanonicalized );
        CPPUNIT_ASSERT( !LanguageTag::isValidBcp47( "x-foobar", &aCanonicalized, true) );
        CPPUNIT_ASSERT_EQUAL( OUString("x-foobar"), aCanonicalized );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "qaa", &aCanonicalized, true) );
        CPPUNIT_ASSERT_EQUAL( OUString("qaa"), aCanonicalized );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "de-Latn-DE", &aCanonicalized) );
        CPPUNIT_ASSERT_EQUAL( OUString("de-DE"), aCanonicalized );
        /* TODO: at least some (those we know) grandfathered tags should be
         * recognized by the replacement code. */
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "en-GB-oed", &aCanonicalized) );
        // en-GB-oed has become deprecated in updated language-subtag-registry database
        // (liblangtag 0.5.7)
        CPPUNIT_ASSERT( ( aCanonicalized == "en-GB-oxendict" ) || ( aCanonicalized == "en-GB-oed" ) );
    }
}

bool checkMapping( std::u16string_view rStr1, std::u16string_view rStr2 )
{
    if (rStr1 == u"la-Latn"     ) return rStr2 == u"la";
    if (rStr1 == u"tzm-Latn-DZ" ) return rStr2 == u"kab-DZ";
    if (rStr1 == u"bs-Latn-BA"  ) return rStr2 == u"bs-BA";
    if (rStr1 == u"bs-Latn"     ) return rStr2 == u"bs";
    if (rStr1 == u"cz"          ) return rStr2 == u"cs-CZ";
    if (rStr1 == u"iw-IL"       ) return rStr2 == u"he-IL";
    if (rStr1 == u"in-ID"       ) return rStr2 == u"id-ID";
    if (rStr1 == u"sr-YU"       ) return rStr2 == u"sr-CS";
    if (rStr1 == u"sh-RS"       ) return rStr2 == u"sr-Latn-RS";
    if (rStr1 == u"sh-YU"       ) return rStr2 == u"sr-Latn-CS";
    if (rStr1 == u"sh-CS"       ) return rStr2 == u"sr-Latn-CS";
    if (rStr1 == u"sh-ME"       ) return rStr2 == u"sr-Latn-ME";
    if (rStr1 == u"sh-BA"       ) return rStr2 == u"sr-Latn-BA";
    if (rStr1 == u"sh"          ) return rStr2 == u"sr-Latn";
    if (rStr1 == u"lah-PK"      ) return rStr2 == u"pnb-Arab-PK";
    if (rStr1 == u"pa-PK"       ) return rStr2 == u"pnb-Arab-PK";
    if (rStr1 == u"ca-XV"       ) return rStr2 == u"ca-ES-valencia";
    if (rStr1 == u"qcv-ES"      ) return rStr2 == u"ca-ES-valencia";
    if (rStr1 == u"ns-ZA"       ) return rStr2 == u"nso-ZA";
    if (rStr1 == u"ven-ZA"      ) return rStr2 == u"ve-ZA";
    if (rStr1 == u"qu-EC"       ) return rStr2 == u"quz-EC";
    if (rStr1 == u"qu-PE"       ) return rStr2 == u"quz-PE";
    if (rStr1 == u"ff-NG"       ) return rStr2 == u"fuv-NG";
    if (rStr1 == u"ji-IL"       ) return rStr2 == u"yi-IL";
    if (rStr1 == u"iu-CA"       ) return rStr2 == u"iu-Latn-CA";
    if (rStr1 == u"iu"          ) return rStr2 == u"iu-Latn";
    if (rStr1 == u"gbz-AF"      ) return rStr2 == u"prs-AF";
    if (rStr1 == u"ber-DZ"      ) return rStr2 == u"kab-DZ";
    if (rStr1 == u"tmz-MA"      ) return rStr2 == u"tzm-Tfng-MA";
    if (rStr1 == u"ber-MA"      ) return rStr2 == u"tzm-Tfng-MA";
    if (rStr1 == u"mg-MG"       ) return rStr2 == u"plt-MG";
    if (rStr1 == u"pli"         ) return rStr2 == u"pi-Latn";
    if (rStr1 == u"ks"          ) return rStr2 == u"ks-Arab";
    if (rStr1 == u"chr-US"      ) return rStr2 == u"chr-Cher-US";
    if (rStr1 == u"sd-PK"       ) return rStr2 == u"sd-Arab-PK";
    if (rStr1 == u"sr-Cyrl-RS"  ) return rStr2 == u"sr-RS";
    if (rStr1 == u"sr-Cyrl-ME"  ) return rStr2 == u"sr-ME";
    if (rStr1 == u"sr-Cyrl-BA"  ) return rStr2 == u"sr-BA";
    if (rStr1 == u"sr-Cyrl-CS"  ) return rStr2 == u"sr-CS";
    if (rStr1 == u"sr-Cyrl"     ) return rStr2 == u"sr";
    if (rStr1 == u"yi-Hebr-US"  ) return rStr2 == u"yi-US";
    if (rStr1 == u"yi-Hebr-IL"  ) return rStr2 == u"yi-IL";
    if (rStr1 == u"ha-NG"       ) return rStr2 == u"ha-Latn-NG";
    if (rStr1 == u"ha-GH"       ) return rStr2 == u"ha-Latn-GH";
    if (rStr1 == u"ku-Arab-IQ"  ) return rStr2 == u"ckb-IQ";
    if (rStr1 == u"ku-Arab"     ) return rStr2 == u"ckb";
    if (rStr1 == u"kmr-TR"      ) return rStr2 == u"kmr-Latn-TR";
    if (rStr1 == u"ku-TR"       ) return rStr2 == u"kmr-Latn-TR";
    if (rStr1 == u"kmr-SY"      ) return rStr2 == u"kmr-Latn-SY";
    if (rStr1 == u"ku-SY"       ) return rStr2 == u"kmr-Latn-SY";
    if (rStr1 == u"ku-IQ"       ) return rStr2 == u"ckb-IQ";
    if (rStr1 == u"ku-IR"       ) return rStr2 == u"ckb-IR";
    if (rStr1 == u"eu"          ) return rStr2 == u"eu-ES";
    if (rStr1 == u"crk-Latn-CN" ) return rStr2 == u"crk-Latn-CA";
    if (rStr1 == u"crk-Cans-CN" ) return rStr2 == u"crk-Cans-CA";
    if (rStr1 == u"en-GB-oed"   ) return rStr2 == u"en-GB-oxendict";
    if (rStr1 == u"es-ES_tradnl") return rStr2 == u"es-ES-u-co-trad";
    if (rStr1 == u"sd-IN"       ) return rStr2 == u"sd-Deva-IN";
    if (rStr1 == u"cmn-CN"      ) return rStr2 == u"zh-CN";
    if (rStr1 == u"cmn-TW"      ) return rStr2 == u"zh-TW";
    return rStr1 == rStr2;
}

void TestLanguageTag::testAllIsoLangEntries()
{
    const ::std::vector< MsLangId::LanguagetagMapping > aList( MsLangId::getDefinedLanguagetags());
    for (auto const& elem : aList)
    {
        bool b=false;
        if (elem.maBcp47 == "la-VA")
            b=true;
        (void)b;

        LanguageTag aTagString( elem.maBcp47, true);
        LanguageTag aTagID( elem.mnLang);
        if (!checkMapping( elem.maBcp47, aTagString.getBcp47()))
        {
            OString aMessage( OUStringToOString( elem.maBcp47, RTL_TEXTENCODING_ASCII_US));
            aMessage += " -> " + OUStringToOString( aTagString.getBcp47(), RTL_TEXTENCODING_ASCII_US);
            CPPUNIT_ASSERT_EQUAL_MESSAGE( aMessage.getStr(), aTagString.getBcp47(), elem.maBcp47 );
        }
        if (elem.maBcp47 != aTagID.getBcp47())
        {
            // There are multiple mappings, ID must be equal after conversions.
            LanguageTag aTagBack( aTagID.getBcp47(), true);
            if (aTagString.getLanguageType() != aTagBack.getLanguageType())
            {
                OString aMessage( OUStringToOString( elem.maBcp47, RTL_TEXTENCODING_ASCII_US));
                aMessage += " " + OUStringToOString( aTagString.getBcp47(), RTL_TEXTENCODING_ASCII_US) + ": " +
                    OUStringToOString( aTagString.getBcp47(), RTL_TEXTENCODING_ASCII_US) + " " +
                    OString::number( static_cast<sal_uInt16>(aTagString.getLanguageType()), 16) +
                    " -> " + OUStringToOString( aTagBack.getBcp47(), RTL_TEXTENCODING_ASCII_US) + " " +
                    OString::number( static_cast<sal_uInt16>(aTagBack.getLanguageType()), 16);
                CPPUNIT_ASSERT_EQUAL_MESSAGE( aMessage.getStr(), aTagBack.getLanguageType(), aTagString.getLanguageType());
            }
        }
#if 0
        // This does not hold, there are cases like 'ar'
        // LANGUAGE_ARABIC_PRIMARY_ONLY that when mapped back results in
        // 'ar-SA' as default locale.
        if (elem.mnLang != aTagString.getLanguageType())
        {
            // There are multiple mappings, string must be equal after conversions.
            LanguageTag aTagBack( aTagString.getLanguageType());
            if (aTagID.getBcp47() != aTagBack.getBcp47())
            {
                OString aMessage( OUStringToOString( elem.maBcp47, RTL_TEXTENCODING_ASCII_US));
                aMessage += " " + OUStringToOString( aTagID.getBcp47(), RTL_TEXTENCODING_ASCII_US) +
                    " -> " + OUStringToOString( aTagBack.getBcp47(), RTL_TEXTENCODING_ASCII_US);
                CPPUNIT_ASSERT_MESSAGE( aMessage.getStr(), aTagID.getBcp47() == aTagBack.getBcp47());
            }
        }
#endif
    }

    // Uncommenting this makes the test break and output SAL_WARN/INFO
    //CPPUNIT_ASSERT( true == false );
}

CPPUNIT_TEST_SUITE_REGISTRATION( TestLanguageTag );

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
