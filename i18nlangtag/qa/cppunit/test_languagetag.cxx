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
#include <i18nlangtag/languagetagicu.hxx>

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
    void testDisplayNames();
    void testLanguagesWithoutHyphenation();

    CPPUNIT_TEST_SUITE(TestLanguageTag);
    CPPUNIT_TEST(testAllTags);
    CPPUNIT_TEST(testAllIsoLangEntries);
    CPPUNIT_TEST(testDisplayNames);
    CPPUNIT_TEST(testLanguagesWithoutHyphenation);
    CPPUNIT_TEST_SUITE_END();
};

void TestLanguageTag::testAllTags()
{
    {
        LanguageTag de_DE( u"de-Latn-DE"_ustr, true );
        OUString aBcp47 = de_DE.getBcp47();
        lang::Locale aLocale = de_DE.getLocale();
        LanguageType nLanguageType = de_DE.getLanguageType();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Default script should be stripped after canonicalize.", u"de-DE"_ustr, aBcp47 );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"DE"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, nLanguageType );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, de_DE.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"DE"_ustr, de_DE.getCountry() );
        CPPUNIT_ASSERT( de_DE.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, de_DE.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, de_DE.makeFallback().getBcp47() );
    }

    {
        LanguageTag klingon( u"i-klingon"_ustr, true );
        lang::Locale aLocale = klingon.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"tlh"_ustr, klingon.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"tlh"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( klingon.isValidBcp47() );
        CPPUNIT_ASSERT( klingon.isIsoLocale() );
        CPPUNIT_ASSERT( klingon.isIsoODF() );
        LanguageType nLang = klingon.getLanguageType();
        LanguageTag klingon_id( nLang);
        CPPUNIT_ASSERT_EQUAL( u"tlh"_ustr, klingon_id.getBcp47() );
    }

    {
        OUString s_sr_RS( u"sr-RS"_ustr );
        LanguageTag sr_RS( s_sr_RS, true );
        lang::Locale aLocale = sr_RS.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_sr_RS, sr_RS.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"sr"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"RS"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_SERBIAN_CYRILLIC_SERBIA, sr_RS.getLanguageType() );
        CPPUNIT_ASSERT( sr_RS.isValidBcp47() );
        CPPUNIT_ASSERT( sr_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_RS.isIsoODF() );
    }

    {
        OUString s_sr_Latn_RS( u"sr-Latn-RS"_ustr );
        LanguageTag sr_RS( s_sr_Latn_RS, true );
        lang::Locale aLocale = sr_RS.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_sr_Latn_RS, sr_RS.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"RS"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_sr_Latn_RS, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_SERBIAN_LATIN_SERBIA, sr_RS.getLanguageType() );
        CPPUNIT_ASSERT( sr_RS.isValidBcp47() );
        CPPUNIT_ASSERT( !sr_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_RS.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"sr"_ustr, sr_RS.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"RS"_ustr, sr_RS.getCountry() );
        CPPUNIT_ASSERT_EQUAL( u"Latn"_ustr, sr_RS.getScript() );
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn"_ustr, sr_RS.getLanguageAndScript() );
    }

    {
        OUString s_sr_Latn_CS( u"sr-Latn-CS"_ustr );
        LanguageTag sr_Latn_CS( s_sr_Latn_CS, true );
        lang::Locale aLocale = sr_Latn_CS.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_sr_Latn_CS, sr_Latn_CS.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"CS"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_sr_Latn_CS, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_SERBIAN_LATIN_SAM, sr_Latn_CS.getLanguageType() );
        CPPUNIT_ASSERT( sr_Latn_CS.isValidBcp47() );
        CPPUNIT_ASSERT( !sr_Latn_CS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_Latn_CS.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"sr"_ustr, sr_Latn_CS.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"CS"_ustr, sr_Latn_CS.getCountry() );
        CPPUNIT_ASSERT_EQUAL( u"Latn"_ustr, sr_Latn_CS.getScript() );
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn"_ustr, sr_Latn_CS.getLanguageAndScript() );
        ::std::vector< OUString > sr_Latn_CS_Fallbacks( sr_Latn_CS.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(9), sr_Latn_CS_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn-CS"_ustr, sr_Latn_CS_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn-YU"_ustr, sr_Latn_CS_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"sh-CS"_ustr, sr_Latn_CS_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"sh-YU"_ustr, sr_Latn_CS_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn"_ustr, sr_Latn_CS_Fallbacks[4]);
        CPPUNIT_ASSERT_EQUAL( u"sh"_ustr, sr_Latn_CS_Fallbacks[5]);
        CPPUNIT_ASSERT_EQUAL( u"sr-CS"_ustr, sr_Latn_CS_Fallbacks[6]);
        CPPUNIT_ASSERT_EQUAL( u"sr-YU"_ustr, sr_Latn_CS_Fallbacks[7]);
        CPPUNIT_ASSERT_EQUAL( u"sr"_ustr, sr_Latn_CS_Fallbacks[8]);
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn-CS"_ustr, sr_Latn_CS.makeFallback().getBcp47());
    }

    // 'sh-RS' has an internal override to 'sr-Latn-RS'
    {
        LanguageTag sh_RS( u"sh-RS"_ustr, true );
        lang::Locale aLocale = sh_RS.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn-RS"_ustr, sh_RS.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( I18NLANGTAG_QLT , aLocale.Language);
        CPPUNIT_ASSERT_EQUAL( u"RS"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn-RS"_ustr, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_SERBIAN_LATIN_SERBIA, sh_RS.getLanguageType() );
        CPPUNIT_ASSERT( sh_RS.isValidBcp47() );
        CPPUNIT_ASSERT( !sh_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sh_RS.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"sr"_ustr, sh_RS.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"RS"_ustr, sh_RS.getCountry() );
        CPPUNIT_ASSERT_EQUAL( u"Latn"_ustr, sh_RS.getScript() );
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn"_ustr, sh_RS.getLanguageAndScript() );
        ::std::vector< OUString > sh_RS_Fallbacks( sh_RS.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(6), sh_RS_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn-RS"_ustr, sh_RS_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"sh-RS"_ustr, sh_RS_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn"_ustr, sh_RS_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"sh"_ustr, sh_RS_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( u"sr-RS"_ustr, sh_RS_Fallbacks[4]);
        CPPUNIT_ASSERT_EQUAL( u"sr"_ustr, sh_RS_Fallbacks[5]);
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn-RS"_ustr, sh_RS.makeFallback().getBcp47());
        CPPUNIT_ASSERT_EQUAL( u"sr-Latn-RS"_ustr, sh_RS.getBcp47());
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_SERBIAN_LATIN_SERBIA, sh_RS.getLanguageType() );
    }

    // 'bs-Latn-BA' with 'Latn' suppress-script, we map that ourselves for a
    // known LangID with an override and canonicalization should work the same
    // without liblangtag.
    {
        LanguageTag bs_Latn_BA( u"bs-Latn-BA"_ustr, true );
        lang::Locale aLocale = bs_Latn_BA.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"bs-BA"_ustr, bs_Latn_BA.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"bs"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"BA"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA, bs_Latn_BA.getLanguageType() );
        CPPUNIT_ASSERT( bs_Latn_BA.isValidBcp47() );
        CPPUNIT_ASSERT( bs_Latn_BA.isIsoLocale() );
        CPPUNIT_ASSERT( bs_Latn_BA.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"bs"_ustr, bs_Latn_BA.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"BA"_ustr, bs_Latn_BA.getCountry() );
        CPPUNIT_ASSERT( bs_Latn_BA.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"bs"_ustr, bs_Latn_BA.getLanguageAndScript() );
        ::std::vector< OUString > bs_Latn_BA_Fallbacks( bs_Latn_BA.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), bs_Latn_BA_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"bs-BA"_ustr, bs_Latn_BA_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"bs"_ustr, bs_Latn_BA_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"bs-BA"_ustr, bs_Latn_BA.makeFallback().getBcp47());
        CPPUNIT_ASSERT_EQUAL( u"bs-BA"_ustr, bs_Latn_BA.getBcp47());
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA, bs_Latn_BA.getLanguageType() );
    }

    {
        OUString s_ca_ES_valencia( u"ca-ES-valencia"_ustr );
        LanguageTag ca_ES_valencia( s_ca_ES_valencia, true );
        lang::Locale aLocale = ca_ES_valencia.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_ca_ES_valencia, ca_ES_valencia.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"ES"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_ca_ES_valencia, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_CATALAN_VALENCIAN, ca_ES_valencia.getLanguageType() );
        CPPUNIT_ASSERT( ca_ES_valencia.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_ES_valencia.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_ES_valencia.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_ES_valencia.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"ES"_ustr, ca_ES_valencia.getCountry() );
        CPPUNIT_ASSERT( ca_ES_valencia.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_ES_valencia.getLanguageAndScript() );
        ::std::vector< OUString > ca_ES_valencia_Fallbacks( ca_ES_valencia.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), ca_ES_valencia_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"ca-ES-valencia"_ustr, ca_ES_valencia_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"ca-XV"_ustr, ca_ES_valencia_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"ca-valencia"_ustr, ca_ES_valencia_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"ca-ES"_ustr, ca_ES_valencia_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_ES_valencia_Fallbacks[4]);
        CPPUNIT_ASSERT_EQUAL( u"ca-ES-valencia"_ustr, ca_ES_valencia.makeFallback().getBcp47());
    }

    {
        OUString s_ca_valencia( u"ca-valencia"_ustr );
        LanguageTag ca_valencia( s_ca_valencia, true );
        lang::Locale aLocale = ca_valencia.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_ca_valencia, ca_valencia.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( s_ca_valencia, aLocale.Variant );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( ca_valencia.getLanguageType()) );
        CPPUNIT_ASSERT( ca_valencia.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_valencia.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_valencia.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_valencia.getLanguage() );
        CPPUNIT_ASSERT( ca_valencia.getCountry().isEmpty() );
        CPPUNIT_ASSERT( ca_valencia.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_valencia.getLanguageAndScript() );
        ::std::vector< OUString > ca_valencia_Fallbacks( ca_valencia.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), ca_valencia_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"ca-valencia"_ustr, ca_valencia_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_valencia_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"ca-ES-valencia"_ustr, ca_valencia.makeFallback().getBcp47());
    }

    // 'ca-XV' has an internal override to 'ca-ES-valencia'
    {
        OUString s_ca_ES_valencia( u"ca-ES-valencia"_ustr );
        LanguageTag ca_XV( u"ca-XV"_ustr, true );
        lang::Locale aLocale = ca_XV.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_ca_ES_valencia, ca_XV.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( I18NLANGTAG_QLT , aLocale.Language);
        CPPUNIT_ASSERT_EQUAL( u"ES"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_ca_ES_valencia, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_CATALAN_VALENCIAN, ca_XV.getLanguageType() );
        CPPUNIT_ASSERT( ca_XV.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_XV.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_XV.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_XV.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"ES"_ustr, ca_XV.getCountry() );
        CPPUNIT_ASSERT( ca_XV.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_XV.getLanguageAndScript() );
        ::std::vector< OUString > ca_XV_Fallbacks( ca_XV.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), ca_XV_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"ca-ES-valencia"_ustr, ca_XV_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"ca-XV"_ustr, ca_XV_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"ca-valencia"_ustr, ca_XV_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"ca-ES"_ustr, ca_XV_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( u"ca"_ustr, ca_XV_Fallbacks[4]);
        CPPUNIT_ASSERT_EQUAL( u"ca-ES-valencia"_ustr, ca_XV.makeFallback().getBcp47());
    }

    {
        OUString s_de_DE( u"de-DE"_ustr );
        LanguageTag de_DE( s_de_DE, true );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_de_DE, de_DE.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"DE"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, de_DE.getLanguageType() );
        CPPUNIT_ASSERT( de_DE.isValidBcp47() );
        CPPUNIT_ASSERT( de_DE.isIsoLocale() );
        CPPUNIT_ASSERT( de_DE.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, de_DE.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"DE"_ustr, de_DE.getCountry() );
        CPPUNIT_ASSERT( de_DE.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, de_DE.getLanguageAndScript() );
        ::std::vector< OUString > de_DE_Fallbacks( de_DE.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), de_DE_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, de_DE_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, de_DE_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, de_DE.makeFallback().getBcp47());
    }

    {
        LanguageTag de_DE( lang::Locale( u"de"_ustr, u"DE"_ustr, u""_ustr ) );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, de_DE.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"DE"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, de_DE.getLanguageType() );
    }

    {
        LanguageTag de_DE( LANGUAGE_GERMAN );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, de_DE.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"DE"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, de_DE.getLanguageType() );
    }

    // Unmapped but known language-only.
    {
        OUString s_de( u"de"_ustr );
        LanguageTag de( s_de, true );
        lang::Locale aLocale = de.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_de, de.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        LanguageType de_LangID = de.getLanguageType();
        CPPUNIT_ASSERT( de_LangID != LANGUAGE_GERMAN );
        CPPUNIT_ASSERT_EQUAL( MsLangId::getPrimaryLanguage( LANGUAGE_GERMAN) , de_LangID);
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, de.makeFallback().getBcp47());
        // Check registered mapping.
        LanguageTag de_l( de_LangID);
        CPPUNIT_ASSERT_EQUAL( s_de, de_l.getBcp47() );
    }

    // "bo" and "dz" share the same primary language ID, only one gets it
    // assigned, "dz" language-only has a special mapping.
    {
        LanguageTag bo( u"bo"_ustr, true );
        CPPUNIT_ASSERT_EQUAL( MsLangId::getPrimaryLanguage( LANGUAGE_TIBETAN), bo.getLanguageType() );
        LanguageTag dz( u"dz"_ustr, true );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_DZONGKHA_MAP_LONLY, dz.getLanguageType() );
    }

    // "no", "nb" and "nn" share the same primary language ID, which even is
    // assigned to "no-NO" for legacy so none gets it assigned, all on-the-fly
    // except if there is a defined MS-LCID for LanguageScriptOnly (LSO).
    {
        LanguageTag no( u"no"_ustr, true );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( no.getLanguageType()) );
        LanguageTag nb( u"nb"_ustr, true );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_NORWEGIAN_BOKMAL_LSO, nb.getLanguageType() );
        LanguageTag nn( u"nn"_ustr, true );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_NORWEGIAN_NYNORSK_LSO, nn.getLanguageType() );
        LanguageTag no_NO( u"no-NO"_ustr, true );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_NORWEGIAN, no_NO.getLanguageType() );
    }

    // 'de-1901' derived from 'de-DE-1901' grandfathered to check that it is
    // accepted as (DIGIT 3ALNUM) variant
    {
        OUString s_de_1901( u"de-1901"_ustr );
        LanguageTag de_1901( s_de_1901 );
        lang::Locale aLocale = de_1901.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_de_1901, de_1901.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( s_de_1901, aLocale.Variant );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( de_1901.getLanguageType()) );
        CPPUNIT_ASSERT( de_1901.isValidBcp47() );
        CPPUNIT_ASSERT( !de_1901.isIsoLocale() );
        CPPUNIT_ASSERT( !de_1901.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, de_1901.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( u"1901"_ustr, de_1901.getVariants() );
        ::std::vector< OUString > de_1901_Fallbacks( de_1901.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), de_1901_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"de-1901"_ustr, de_1901_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, de_1901_Fallbacks[1]);
    }

    // 'en-GB-oed' is known grandfathered for English, Oxford English
    // Dictionary spelling.
    // Deprecated as of 2015-04-17, prefer en-GB-oxendict instead.
    // As of 2017-03-14 we also alias to en-GB-oxendict.
    {
        OUString s_en_GB_oxendict( u"en-GB-oxendict"_ustr );
        LanguageTag en_GB_oed( u"en-GB-oed"_ustr );
        lang::Locale aLocale = en_GB_oed.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_en_GB_oxendict, en_GB_oed.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"GB"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_en_GB_oxendict, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_ENGLISH_UK_OXENDICT, en_GB_oed.getLanguageType() );
        CPPUNIT_ASSERT( en_GB_oed.isValidBcp47() );
        CPPUNIT_ASSERT( !en_GB_oed.isIsoLocale() );
        CPPUNIT_ASSERT( !en_GB_oed.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, en_GB_oed.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( u"oxendict"_ustr, en_GB_oed.getVariants() );
        ::std::vector< OUString > en_GB_oed_Fallbacks( en_GB_oed.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), en_GB_oed_Fallbacks.size() );
        CPPUNIT_ASSERT_EQUAL( u"en-GB-oxendict"_ustr, en_GB_oed_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"en-GB-oed"_ustr, en_GB_oed_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"en-oxendict"_ustr, en_GB_oed_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"en-GB"_ustr, en_GB_oed_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, en_GB_oed_Fallbacks[4]);
        // 'en-oed' is not a valid fallback!
    }

    // 'en-GB-oxendict' as preferred over 'en-GB-oed'.
    {
        OUString s_en_GB_oxendict( u"en-GB-oxendict"_ustr );
        LanguageTag en_GB_oxendict( s_en_GB_oxendict );
        lang::Locale aLocale = en_GB_oxendict.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_en_GB_oxendict, en_GB_oxendict.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"GB"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_en_GB_oxendict, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_ENGLISH_UK_OXENDICT, en_GB_oxendict.getLanguageType() );
        CPPUNIT_ASSERT( en_GB_oxendict.isValidBcp47() );
        CPPUNIT_ASSERT( !en_GB_oxendict.isIsoLocale() );
        CPPUNIT_ASSERT( !en_GB_oxendict.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, en_GB_oxendict.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( u"oxendict"_ustr, en_GB_oxendict.getVariants() );
        ::std::vector< OUString > en_GB_oxendict_Fallbacks( en_GB_oxendict.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), en_GB_oxendict_Fallbacks.size() );
        CPPUNIT_ASSERT_EQUAL( u"en-GB-oxendict"_ustr, en_GB_oxendict_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"en-GB-oed"_ustr, en_GB_oxendict_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"en-oxendict"_ustr, en_GB_oxendict_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"en-GB"_ustr, en_GB_oxendict_Fallbacks[3]);
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, en_GB_oxendict_Fallbacks[4]);
    }

    // 'es-ES-u-co-trad' is a valid (and known) Extension U tag
    {
        OUString s_es_ES_u_co_trad( u"es-ES-u-co-trad"_ustr );
        LanguageTag es_ES_u_co_trad( s_es_ES_u_co_trad );
        lang::Locale aLocale = es_ES_u_co_trad.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_es_ES_u_co_trad, es_ES_u_co_trad.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"ES"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_es_ES_u_co_trad, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_SPANISH_DATED, es_ES_u_co_trad.getLanguageType() );
        CPPUNIT_ASSERT( es_ES_u_co_trad.isValidBcp47() );
        CPPUNIT_ASSERT( !es_ES_u_co_trad.isIsoLocale() );
        CPPUNIT_ASSERT( !es_ES_u_co_trad.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"es"_ustr, es_ES_u_co_trad.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( u"u-co-trad"_ustr, es_ES_u_co_trad.getVariants() );
        ::std::vector< OUString > es_ES_u_co_trad_Fallbacks( es_ES_u_co_trad.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(4), es_ES_u_co_trad_Fallbacks.size() );
        CPPUNIT_ASSERT_EQUAL( u"es-ES-u-co-trad"_ustr, es_ES_u_co_trad_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"es-u-co-trad"_ustr, es_ES_u_co_trad_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"es-ES"_ustr, es_ES_u_co_trad_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"es"_ustr, es_ES_u_co_trad_Fallbacks[3]);
        // Map to broken MS.
        CPPUNIT_ASSERT_EQUAL( u"es-ES_tradnl"_ustr, es_ES_u_co_trad.getBcp47MS() );
    }

    // 'es-ES_tradnl' (broken MS) maps to 'es-ES-u-co-trad'
    {
        OUString s_es_ES_u_co_trad( u"es-ES-u-co-trad"_ustr );
        OUString s_es_ES_tradnl( u"es-ES_tradnl"_ustr );
        LanguageTag es_ES_tradnl( s_es_ES_tradnl );
        lang::Locale aLocale = es_ES_tradnl.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_es_ES_u_co_trad, es_ES_tradnl.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"ES"_ustr, aLocale.Country );
        CPPUNIT_ASSERT_EQUAL( s_es_ES_u_co_trad, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_SPANISH_DATED, es_ES_tradnl.getLanguageType() );
        CPPUNIT_ASSERT( es_ES_tradnl.isValidBcp47() );
        CPPUNIT_ASSERT( !es_ES_tradnl.isIsoLocale() );
        CPPUNIT_ASSERT( !es_ES_tradnl.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"es"_ustr, es_ES_tradnl.getLanguageAndScript() );
        CPPUNIT_ASSERT_EQUAL( u"u-co-trad"_ustr, es_ES_tradnl.getVariants() );
        ::std::vector< OUString > es_ES_tradnl_Fallbacks( es_ES_tradnl.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(4), es_ES_tradnl_Fallbacks.size() );
        CPPUNIT_ASSERT_EQUAL( u"es-ES-u-co-trad"_ustr, es_ES_tradnl_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"es-u-co-trad"_ustr, es_ES_tradnl_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"es-ES"_ustr, es_ES_tradnl_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"es"_ustr, es_ES_tradnl_Fallbacks[3]);
        // Map back to broken MS.
        CPPUNIT_ASSERT_EQUAL( s_es_ES_tradnl, es_ES_tradnl.getBcp47MS() );
    }

    // 'zh-Hans-CN' maps to 'zh-CN' and fallbacks
    {
        LanguageTag zh_Hans_CN( u"zh-Hans-CN"_ustr, true );
        lang::Locale aLocale = zh_Hans_CN.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"zh-CN"_ustr, zh_Hans_CN.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"CN"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_CHINESE_SIMPLIFIED, zh_Hans_CN.getLanguageType() );
        CPPUNIT_ASSERT( zh_Hans_CN.isValidBcp47() );
        CPPUNIT_ASSERT( zh_Hans_CN.isIsoLocale() );
        CPPUNIT_ASSERT( zh_Hans_CN.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_Hans_CN.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"CN"_ustr, zh_Hans_CN.getCountry() );
        CPPUNIT_ASSERT( zh_Hans_CN.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_Hans_CN.getLanguageAndScript() );
        ::std::vector< OUString > zh_Hans_CN_Fallbacks( zh_Hans_CN.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), zh_Hans_CN_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"zh-CN"_ustr, zh_Hans_CN_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_Hans_CN_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"zh-CN"_ustr, zh_Hans_CN.makeFallback().getBcp47());
    }

    // 'zh-Hant-TW' maps to 'zh-TW' and fallbacks
    {
        LanguageTag zh_Hant_TW( u"zh-Hant-TW"_ustr, true );
        lang::Locale aLocale = zh_Hant_TW.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"zh-TW"_ustr, zh_Hant_TW.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"TW"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_CHINESE_TRADITIONAL, zh_Hant_TW.getLanguageType() );
        CPPUNIT_ASSERT( zh_Hant_TW.isValidBcp47() );
        CPPUNIT_ASSERT( zh_Hant_TW.isIsoLocale() );
        CPPUNIT_ASSERT( zh_Hant_TW.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_Hant_TW.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"TW"_ustr, zh_Hant_TW.getCountry() );
        CPPUNIT_ASSERT( zh_Hant_TW.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_Hant_TW.getLanguageAndScript() );
        ::std::vector< OUString > zh_Hant_TW_Fallbacks( zh_Hant_TW.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(3), zh_Hant_TW_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"zh-TW"_ustr, zh_Hant_TW_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"zh-CN"_ustr, zh_Hant_TW_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_Hant_TW_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"zh-TW"_ustr, zh_Hant_TW.makeFallback().getBcp47());
    }

    // 'zh-SG' and fallbacks
    {
        OUString s_zh_SG( u"zh-SG"_ustr );
        LanguageTag zh_SG( s_zh_SG, true );
        lang::Locale aLocale = zh_SG.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_zh_SG, zh_SG.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"SG"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_CHINESE_SINGAPORE, zh_SG.getLanguageType() );
        CPPUNIT_ASSERT( zh_SG.isValidBcp47() );
        CPPUNIT_ASSERT( zh_SG.isIsoLocale() );
        CPPUNIT_ASSERT( zh_SG.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_SG.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"SG"_ustr, zh_SG.getCountry() );
        CPPUNIT_ASSERT( zh_SG.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_SG.getLanguageAndScript() );
        ::std::vector< OUString > zh_SG_Fallbacks( zh_SG.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(3), zh_SG_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"zh-SG"_ustr, zh_SG_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"zh-CN"_ustr, zh_SG_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_SG_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"zh-SG"_ustr, zh_SG.makeFallback().getBcp47());
    }

    // 'zh-HK' and fallbacks
    {
        OUString s_zh_HK( u"zh-HK"_ustr );
        LanguageTag zh_HK( s_zh_HK, true );
        lang::Locale aLocale = zh_HK.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_zh_HK, zh_HK.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"HK"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_CHINESE_HONGKONG, zh_HK.getLanguageType() );
        CPPUNIT_ASSERT( zh_HK.isValidBcp47() );
        CPPUNIT_ASSERT( zh_HK.isIsoLocale() );
        CPPUNIT_ASSERT( zh_HK.isIsoODF() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_HK.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"HK"_ustr, zh_HK.getCountry() );
        CPPUNIT_ASSERT( zh_HK.getScript().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_HK.getLanguageAndScript() );
        ::std::vector< OUString > zh_HK_Fallbacks( zh_HK.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(3), zh_HK_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"zh-HK"_ustr, zh_HK_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"zh-TW"_ustr, zh_HK_Fallbacks[1]);
        CPPUNIT_ASSERT_EQUAL( u"zh"_ustr, zh_HK_Fallbacks[2]);
        CPPUNIT_ASSERT_EQUAL( u"zh-HK"_ustr, zh_HK.makeFallback().getBcp47());
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

    // 'es-419' fallbacks
    {
        OUString s_es_419( u"es-419"_ustr );
        LanguageTag es_419( s_es_419 );
        lang::Locale aLocale = es_419.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_es_419, es_419.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"es-419"_ustr, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_SPANISH_LATIN_AMERICA, es_419.getLanguageType() );
        ::std::vector< OUString > es_419_Fallbacks( es_419.getFallbackStrings( true));
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), es_419_Fallbacks.size());
        CPPUNIT_ASSERT_EQUAL( u"es-419"_ustr, es_419_Fallbacks[0]);
        CPPUNIT_ASSERT_EQUAL( u"es"_ustr, es_419_Fallbacks[1]);
    }

    // 'qtz' is a local use known pseudolocale for key ID resource
    {
        OUString s_qtz( u"qtz"_ustr );
        LanguageTag qtz( s_qtz );
        lang::Locale aLocale = qtz.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_qtz, qtz.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qtz"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_KEYID, qtz.getLanguageType() );
    }

    // 'qty' is a local use unknown locale
    {
        OUString s_qty( u"qty"_ustr );
        LanguageTag qty( s_qty );
        lang::Locale aLocale = qty.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_qty, qty.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qty"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( qty.getLanguageType()) );
    }

    // 'qtx' is an unknown new mslangid
    {
        LanguageTag qtx( u"qtx"_ustr );
        qtx.setScriptType( LanguageTag::ScriptType::RTL );
        LanguageType n_qtx = qtx.getLanguageType();
        CPPUNIT_ASSERT_EQUAL( css::i18n::ScriptType::COMPLEX, MsLangId::getScriptType(n_qtx) );
        CPPUNIT_ASSERT( MsLangId::isRightToLeft(n_qtx) );
        CPPUNIT_ASSERT( !MsLangId::isCJK(n_qtx) );
    }

    // 'x-comment' is a privateuse known "locale"
    {
        OUString s_xcomment( u"x-comment"_ustr );
        LanguageTag xcomment( s_xcomment );
        lang::Locale aLocale = xcomment.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_xcomment, xcomment.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"x-comment"_ustr, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_PRIV_COMMENT, xcomment.getLanguageType() );
    }

    // 'x-foobar' is a privateuse unknown "locale"
    {
        OUString s_xfoobar( u"x-foobar"_ustr );
        LanguageTag xfoobar( s_xfoobar );
        lang::Locale aLocale = xfoobar.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_xfoobar, xfoobar.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"x-foobar"_ustr, aLocale.Variant );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( xfoobar.getLanguageType()) );
    }

    // '*' the dreaded jolly joker is a "privateuse" known "locale"
    {
        OUString s_joker( u"*"_ustr );
        LanguageTag joker( s_joker );
        lang::Locale aLocale = joker.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_joker, joker.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"*"_ustr, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_PRIV_JOKER, joker.getLanguageType() );

        joker.reset( LANGUAGE_USER_PRIV_JOKER );
        aLocale = joker.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_joker, joker.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( u"*"_ustr, aLocale.Variant );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_USER_PRIV_JOKER, joker.getLanguageType() );
    }

    // 'C' locale shall map to 'en-US'
    {
        LanguageTag aTag( u"C"_ustr );
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, aTag.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"US"_ustr, aTag.getCountry() );
        lang::Locale aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"en-US"_ustr, aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"US"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_ENGLISH_US, aTag.getLanguageType() );
    }
    {
        LanguageTag aTag( lang::Locale(u"C"_ustr,u""_ustr,u""_ustr) );
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, aTag.getLanguage() );
        CPPUNIT_ASSERT_EQUAL( u"US"_ustr, aTag.getCountry() );
        lang::Locale aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"en-US"_ustr, aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"US"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_ENGLISH_US, aTag.getLanguageType() );
    }

    // test reset() methods
    {
        LanguageTag aTag( LANGUAGE_DONTKNOW );
        lang::Locale aLocale;

        aTag.reset( LANGUAGE_GERMAN );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"DE"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, aTag.getLanguageType() );

        aTag.reset( u"en-US"_ustr );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"en-US"_ustr, aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"en"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"US"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_ENGLISH_US, aTag.getLanguageType() );

        aTag.reset( lang::Locale( u"de"_ustr, u"DE"_ustr, u""_ustr ) );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, aTag.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"de"_ustr, aLocale.Language );
        CPPUNIT_ASSERT_EQUAL( u"DE"_ustr, aLocale.Country );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT_EQUAL( LANGUAGE_GERMAN, aTag.getLanguageType() );
    }

    {
        OUString s_uab( u"unreg-and-bad"_ustr );
        LanguageTag uab( s_uab, true );
        lang::Locale aLocale = uab.getLocale();
        CPPUNIT_ASSERT_EQUAL( s_uab, uab.getBcp47() );
        CPPUNIT_ASSERT_EQUAL( u"qlt"_ustr, aLocale.Language );
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
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( u"en-US"_ustr, &aCanonicalized) );
        CPPUNIT_ASSERT_EQUAL( u"en-US"_ustr, aCanonicalized );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( u"x-foobar"_ustr, &aCanonicalized) );
        CPPUNIT_ASSERT_EQUAL( u"x-foobar"_ustr, aCanonicalized );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( u"qaa"_ustr, &aCanonicalized) );
        CPPUNIT_ASSERT_EQUAL( u"qaa"_ustr, aCanonicalized );
        CPPUNIT_ASSERT( !LanguageTag::isValidBcp47( u"unreg-and-bad"_ustr, &aCanonicalized) );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( u"en-US"_ustr, &aCanonicalized, LanguageTag::PrivateUse::DISALLOW) );
        CPPUNIT_ASSERT_EQUAL( u"en-US"_ustr, aCanonicalized );
        CPPUNIT_ASSERT( !LanguageTag::isValidBcp47( u"x-foobar"_ustr, &aCanonicalized, LanguageTag::PrivateUse::DISALLOW) );
        CPPUNIT_ASSERT_EQUAL( u"x-foobar"_ustr, aCanonicalized );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( u"qaa"_ustr, &aCanonicalized, LanguageTag::PrivateUse::DISALLOW) );
        CPPUNIT_ASSERT_EQUAL( u"qaa"_ustr, aCanonicalized );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( u"de-Latn-DE"_ustr, &aCanonicalized) );
        CPPUNIT_ASSERT_EQUAL( u"de-DE"_ustr, aCanonicalized );
        /* TODO: at least some (those we know) grandfathered tags should be
         * recognized by the replacement code. */
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( u"en-GB-oed"_ustr, &aCanonicalized) );
        // en-GB-oed has become deprecated in updated language-subtag-registry database
        // (liblangtag 0.5.7)
        CPPUNIT_ASSERT( ( aCanonicalized == "en-GB-oxendict" ) || ( aCanonicalized == "en-GB-oed" ) );
    }
}

bool checkMapping( std::u16string_view rStr1, std::u16string_view rStr2 )
{
    if (rStr1 == u"la"          ) return rStr2 == u"la-VA";
    if (rStr1 == u"la-Latn"     ) return rStr2 == u"la-VA";
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
    if (rStr1 == u"ff-Latn-NG"  ) return rStr2 == u"ff-NG";
    if (rStr1 == u"fuv-NG"      ) return rStr2 == u"ff-NG";
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
    if (rStr1 == u"ks-IN"       ) return rStr2 == u"ks-Deva-IN";
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
    if (rStr1 == u"kw-UK"       ) return rStr2 == u"kw-GB";
    if (rStr1 == u"oc-FR"       ) return rStr2 == u"oc-FR-lengadoc";
    if (rStr1 == u"oc-ES"       ) return rStr2 == u"oc-ES-aranes";
    if (rStr1 == u"zh-Hans-CN"  ) return rStr2 == u"zh-CN";
    if (rStr1 == u"zh-Hant-TW"  ) return rStr2 == u"zh-TW";
    if (rStr1 == u"zh-Hans-SG"  ) return rStr2 == u"zh-SG";
    if (rStr1 == u"zh-Hant-HK"  ) return rStr2 == u"zh-HK";
    if (rStr1 == u"zh-Hant-MO"  ) return rStr2 == u"zh-MO";
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
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "defined elem to tag", elem.maBcp47, aTagString.getBcp47() );
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

void TestLanguageTag::testDisplayNames()
{
    OUString aStr;
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-US"_ustr), LanguageTag(u"en-US"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"English (United States)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-US"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"English (United States)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"English"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-US"_ustr), LanguageTag(u"de-DE"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Englisch (Vereinigte Staaten)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-US"_ustr), LanguageTag(u"de"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Englisch (Vereinigte Staaten)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en"_ustr), LanguageTag(u"de"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Englisch"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"de-DE"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"German (Germany)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"de"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"German"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"de-DE-1901"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"German (Germany, Traditional German orthography)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"de-DE-1901"_ustr), LanguageTag(u"de-DE"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Deutsch (Deutschland, Alte deutsche Rechtschreibung)"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-GB"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"English (United Kingdom)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-GB-oxendict"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"English (United Kingdom, Oxford English Dictionary spelling)"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"ca-ES-valencia"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Catalan (Spain, Valencian)"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"sr-Cyrl-RS"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Serbian (Serbia)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"sr-Latn-RS"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Serbian (Latin, Serbia)"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"tlh"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Klingon"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"und"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Unknown language"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"zxx"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"No linguistic content"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"x-lala"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Unknown language (Private-Use=lala)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"x-lala"_ustr), LanguageTag(u"de"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"Unbekannte Sprache (Privatnutzung=lala)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-GB"_ustr), LanguageTag(u"x-lala"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"en (GB)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-GB-oxendict"_ustr), LanguageTag(u"x-lala"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"en (GB, OXENDICT)"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"unreg-and-bad"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"unreg (Andorra, BAD)"_ustr, aStr);  // a tad of a surprise..

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"en-029"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"English (Caribbean)"_ustr, aStr);
    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"fr-015"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"French (Northern Africa)"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"qtz"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"qtz"_ustr, aStr);

    aStr = LanguageTagIcu::getDisplayName( LanguageTag(u"*"_ustr), LanguageTag(u"en"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"*"_ustr, aStr);
}

void TestLanguageTag::testLanguagesWithoutHyphenation()
{
    // Arabic
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_PRIMARY_ONLY));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_ALGERIA));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_BAHRAIN));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_EGYPT));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_IRAQ));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_JORDAN));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_KUWAIT));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_LEBANON));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_LIBYA));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_MOROCCO));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_OMAN));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_QATAR));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_SAUDI_ARABIA));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_SYRIA));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_TUNISIA));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_UAE));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_ARABIC_YEMEN));

    // Pashto
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_PASHTO));

    // Persian/Farsi
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_FARSI));

    // Kashmiri
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_KASHMIRI));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_KASHMIRI_INDIA));

    // Central Kurdish (Sorani), uses Arabic script, does not have hyphenation
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_KURDISH_ARABIC_IRAQ));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_KURDISH_ARABIC_LSO));

    // Northern Kurdish (Kurmanji) -> uses Latin script, has hyphenation
    CPPUNIT_ASSERT(MsLangId::usesHyphenation(LANGUAGE_USER_KURDISH_SYRIA));
    CPPUNIT_ASSERT(MsLangId::usesHyphenation(LANGUAGE_USER_KURDISH_TURKEY));

    // Southern Kurdish -> uses Arabic script, does not have hyphenation
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_USER_KURDISH_SOUTHERN_IRAN));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_USER_KURDISH_SOUTHERN_IRAQ));

    // Punjabi
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_PUNJABI));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_PUNJABI_ARABIC_LSO));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_PUNJABI_PAKISTAN));

    // Sindhi
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_SINDHI));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_SINDHI_ARABIC_LSO));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_SINDHI_PAKISTAN));

    // Malai
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_MALAY_BRUNEI_DARUSSALAM));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_MALAY_MALAYSIA));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_USER_MALAY_ARABIC_BRUNEI));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_USER_MALAY_ARABIC_MALAYSIA));

    // Somali
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_SOMALI));

    // Swahili
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_SWAHILI));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_USER_SWAHILI_TANZANIA));

    // Urdu
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_URDU_INDIA));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_URDU_PAKISTAN));

    // CJK
    // Chinese
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE_HONGKONG));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE_LSO));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE_MACAU));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE_SIMPLIFIED));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE_SIMPLIFIED_LEGACY));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE_SINGAPORE));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE_TRADITIONAL));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_CHINESE_TRADITIONAL_LSO));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_YUE_CHINESE_HONGKONG));
    // Japanese
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_JAPANESE));
    // Korean
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_KOREAN));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_KOREAN_JOHAB));
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_USER_KOREAN_NORTH));

    // Vietnamese
    CPPUNIT_ASSERT(!MsLangId::usesHyphenation(LANGUAGE_VIETNAMESE));
}

CPPUNIT_TEST_SUITE_REGISTRATION( TestLanguageTag );

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
