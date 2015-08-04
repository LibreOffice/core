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
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>

#include <com/sun/star/lang/Locale.hpp>

using namespace com::sun::star;

// To test the replacement code add '&& 0' and also in
// source/languagetag/languagetag.cxx
#if defined(ENABLE_LIBLANGTAG)
#define USE_LIBLANGTAG 1
#else
#define USE_LIBLANGTAG 0
#endif

namespace {

class TestLanguageTag : public CppUnit::TestFixture
{
public:
    TestLanguageTag() {}
    virtual ~TestLanguageTag() {}

    void testAllTags();
    void testAllIsoLangEntries();
    void testAllrfc3066bisConverter();

    CPPUNIT_TEST_SUITE(TestLanguageTag);
    CPPUNIT_TEST(testAllTags);
    CPPUNIT_TEST(testAllIsoLangEntries);
    CPPUNIT_TEST(testAllrfc3066bisConverter);
    CPPUNIT_TEST_SUITE_END();
};

void TestLanguageTag::testAllTags()
{
    {
        OUString s_de_Latn_DE( "de-Latn-DE" );
        LanguageTag de_DE( s_de_Latn_DE, true );
        OUString aBcp47 = de_DE.getBcp47();
        lang::Locale aLocale = de_DE.getLocale();
        LanguageType nLanguageType = de_DE.getLanguageType();
#if USE_LIBLANGTAG
        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "de-DE" );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( nLanguageType == LANGUAGE_GERMAN );
        CPPUNIT_ASSERT( de_DE.getLanguage() == "de" );
        CPPUNIT_ASSERT( de_DE.getCountry() == "DE" );
        CPPUNIT_ASSERT( de_DE.getScript().isEmpty() );
        CPPUNIT_ASSERT( de_DE.getLanguageAndScript() == "de" );
#else
        // The simple replacement code doesn't do any fancy stuff.
        CPPUNIT_ASSERT_MESSAGE("Default script was stripped after canonicalize!?!", aBcp47 == s_de_Latn_DE );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant == "de-Latn-DE" );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( nLanguageType) );    // XXX not canonicalized!
        CPPUNIT_ASSERT( de_DE.getLanguage() == "de" );
        CPPUNIT_ASSERT( de_DE.getCountry() == "DE" );
        CPPUNIT_ASSERT( de_DE.getScript() == "Latn" );
        CPPUNIT_ASSERT( de_DE.getLanguageAndScript() == "de-Latn" );
#endif
        CPPUNIT_ASSERT( de_DE.makeFallback().getBcp47() == "de-DE");
    }

    {
        OUString s_klingon( "i-klingon" );
        LanguageTag klingon( s_klingon, true );
        lang::Locale aLocale = klingon.getLocale();
#if USE_LIBLANGTAG
        CPPUNIT_ASSERT( klingon.getBcp47() == "tlh" );
        CPPUNIT_ASSERT( aLocale.Language == "tlh" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( klingon.getLanguageType()) );
        CPPUNIT_ASSERT( klingon.isValidBcp47() );
        CPPUNIT_ASSERT( klingon.isIsoLocale() );
        CPPUNIT_ASSERT( klingon.isIsoODF() );
        LanguageType nLang = klingon.getLanguageType();
        LanguageTag klingon_id( nLang);
        CPPUNIT_ASSERT( klingon_id.getBcp47() == "tlh" );
#else
        CPPUNIT_ASSERT( klingon.getBcp47() == s_klingon );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == s_klingon );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( klingon.getLanguageType()) );
        CPPUNIT_ASSERT( klingon.isValidBcp47() == true );
        CPPUNIT_ASSERT( klingon.isIsoLocale() == false );
        CPPUNIT_ASSERT( klingon.isIsoODF() == false );
        LanguageType nLang = klingon.getLanguageType();
        LanguageTag klingon_id( nLang);
        CPPUNIT_ASSERT( klingon_id.getBcp47() == s_klingon );
#endif
    }

    {
        OUString s_sr_RS( "sr-RS" );
        LanguageTag sr_RS( s_sr_RS, true );
        lang::Locale aLocale = sr_RS.getLocale();
        CPPUNIT_ASSERT( sr_RS.getBcp47() == s_sr_RS );
        CPPUNIT_ASSERT( aLocale.Language == "sr" );
        CPPUNIT_ASSERT( aLocale.Country == "RS" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( sr_RS.getLanguageType() == LANGUAGE_USER_SERBIAN_CYRILLIC_SERBIA );
        CPPUNIT_ASSERT( sr_RS.isValidBcp47() );
        CPPUNIT_ASSERT( sr_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_RS.isIsoODF() );
    }

    {
        OUString s_sr_Latn_RS( "sr-Latn-RS" );
        LanguageTag sr_RS( s_sr_Latn_RS, true );
        lang::Locale aLocale = sr_RS.getLocale();
        CPPUNIT_ASSERT( sr_RS.getBcp47() == s_sr_Latn_RS );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "RS" );
        CPPUNIT_ASSERT( aLocale.Variant == s_sr_Latn_RS );
        CPPUNIT_ASSERT( sr_RS.getLanguageType() == LANGUAGE_USER_SERBIAN_LATIN_SERBIA );
        CPPUNIT_ASSERT( sr_RS.isValidBcp47() );
        CPPUNIT_ASSERT( !sr_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_RS.isIsoODF() );
        CPPUNIT_ASSERT( sr_RS.getLanguage() == "sr" );
        CPPUNIT_ASSERT( sr_RS.getCountry() == "RS" );
        CPPUNIT_ASSERT( sr_RS.getScript() == "Latn" );
        CPPUNIT_ASSERT( sr_RS.getLanguageAndScript() == "sr-Latn" );
    }

    {
        OUString s_sr_Latn_CS( "sr-Latn-CS" );
        LanguageTag sr_Latn_CS( s_sr_Latn_CS, true );
        lang::Locale aLocale = sr_Latn_CS.getLocale();
        CPPUNIT_ASSERT( sr_Latn_CS.getBcp47() == s_sr_Latn_CS );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "CS" );
        CPPUNIT_ASSERT( aLocale.Variant == s_sr_Latn_CS );
        CPPUNIT_ASSERT( sr_Latn_CS.getLanguageType() == LANGUAGE_SERBIAN_LATIN_SAM );
        CPPUNIT_ASSERT( sr_Latn_CS.isValidBcp47() );
        CPPUNIT_ASSERT( !sr_Latn_CS.isIsoLocale() );
        CPPUNIT_ASSERT( sr_Latn_CS.isIsoODF() );
        CPPUNIT_ASSERT( sr_Latn_CS.getLanguage() == "sr" );
        CPPUNIT_ASSERT( sr_Latn_CS.getCountry() == "CS" );
        CPPUNIT_ASSERT( sr_Latn_CS.getScript() == "Latn" );
        CPPUNIT_ASSERT( sr_Latn_CS.getLanguageAndScript() == "sr-Latn" );
        ::std::vector< OUString > sr_Latn_CS_Fallbacks( sr_Latn_CS.getFallbackStrings( true));
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks.size() == 9);
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[0] == "sr-Latn-CS");
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[1] == "sr-Latn-YU");
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[2] == "sh-CS");
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[3] == "sh-YU");
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[4] == "sr-Latn");
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[5] == "sh");
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[6] == "sr-CS");
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[7] == "sr-YU");
        CPPUNIT_ASSERT( sr_Latn_CS_Fallbacks[8] == "sr");
        CPPUNIT_ASSERT( sr_Latn_CS.makeFallback().getBcp47() == "sr-Latn-CS");
    }

    // 'sh-RS' has an internal override to 'sr-Latn-RS'
    {
        OUString s_sh_RS( "sh-RS" );
        LanguageTag sh_RS( s_sh_RS, true );
        lang::Locale aLocale = sh_RS.getLocale();
        CPPUNIT_ASSERT( sh_RS.getBcp47() == "sr-Latn-RS" );
        CPPUNIT_ASSERT( aLocale.Language == I18NLANGTAG_QLT );
        CPPUNIT_ASSERT( aLocale.Country == "RS" );
        CPPUNIT_ASSERT( aLocale.Variant == "sr-Latn-RS" );
        CPPUNIT_ASSERT( sh_RS.getLanguageType() == LANGUAGE_USER_SERBIAN_LATIN_SERBIA );
        CPPUNIT_ASSERT( sh_RS.isValidBcp47() );
        CPPUNIT_ASSERT( !sh_RS.isIsoLocale() );
        CPPUNIT_ASSERT( sh_RS.isIsoODF() );
        CPPUNIT_ASSERT( sh_RS.getLanguage() == "sr" );
        CPPUNIT_ASSERT( sh_RS.getCountry() == "RS" );
        CPPUNIT_ASSERT( sh_RS.getScript() == "Latn" );
        CPPUNIT_ASSERT( sh_RS.getLanguageAndScript() == "sr-Latn" );
        ::std::vector< OUString > sh_RS_Fallbacks( sh_RS.getFallbackStrings( true));
        CPPUNIT_ASSERT( sh_RS_Fallbacks.size() == 6);
        CPPUNIT_ASSERT( sh_RS_Fallbacks[0] == "sr-Latn-RS");
        CPPUNIT_ASSERT( sh_RS_Fallbacks[1] == "sh-RS");
        CPPUNIT_ASSERT( sh_RS_Fallbacks[2] == "sr-Latn");
        CPPUNIT_ASSERT( sh_RS_Fallbacks[3] == "sh");
        CPPUNIT_ASSERT( sh_RS_Fallbacks[4] == "sr-RS");
        CPPUNIT_ASSERT( sh_RS_Fallbacks[5] == "sr");
        CPPUNIT_ASSERT( sh_RS.makeFallback().getBcp47() == "sr-Latn-RS");
        CPPUNIT_ASSERT( sh_RS.getBcp47() == "sr-Latn-RS");
        CPPUNIT_ASSERT( sh_RS.getLanguageType() == LANGUAGE_USER_SERBIAN_LATIN_SERBIA );
    }

    // 'bs-Latn-BA' with 'Latn' suppress-script, we map that ourselves for a
    // known LangID with an override and canonicalization should work the same
    // without liblangtag.
    {
        OUString s_bs_Latn_BA( "bs-Latn-BA" );
        LanguageTag bs_Latn_BA( s_bs_Latn_BA, true );
        lang::Locale aLocale = bs_Latn_BA.getLocale();
        CPPUNIT_ASSERT( bs_Latn_BA.getBcp47() == "bs-BA" );
        CPPUNIT_ASSERT( aLocale.Language == "bs" );
        CPPUNIT_ASSERT( aLocale.Country == "BA" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( bs_Latn_BA.getLanguageType() == LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA );
        CPPUNIT_ASSERT( bs_Latn_BA.isValidBcp47() );
        CPPUNIT_ASSERT( bs_Latn_BA.isIsoLocale() );
        CPPUNIT_ASSERT( bs_Latn_BA.isIsoODF() );
        CPPUNIT_ASSERT( bs_Latn_BA.getLanguage() == "bs" );
        CPPUNIT_ASSERT( bs_Latn_BA.getCountry() == "BA" );
        CPPUNIT_ASSERT( bs_Latn_BA.getScript().isEmpty() );
        CPPUNIT_ASSERT( bs_Latn_BA.getLanguageAndScript() == "bs" );
        ::std::vector< OUString > bs_Latn_BA_Fallbacks( bs_Latn_BA.getFallbackStrings( true));
        CPPUNIT_ASSERT( bs_Latn_BA_Fallbacks.size() == 2);
        CPPUNIT_ASSERT( bs_Latn_BA_Fallbacks[0] == "bs-BA");
        CPPUNIT_ASSERT( bs_Latn_BA_Fallbacks[1] == "bs");
        CPPUNIT_ASSERT( bs_Latn_BA.makeFallback().getBcp47() == "bs-BA");
        CPPUNIT_ASSERT( bs_Latn_BA.getBcp47() == "bs-BA");
        CPPUNIT_ASSERT( bs_Latn_BA.getLanguageType() == LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA );
    }

    {
        OUString s_ca_ES_valencia( "ca-ES-valencia" );
        LanguageTag ca_ES_valencia( s_ca_ES_valencia, true );
        lang::Locale aLocale = ca_ES_valencia.getLocale();
        CPPUNIT_ASSERT( ca_ES_valencia.getBcp47() == s_ca_ES_valencia );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "ES" );
        CPPUNIT_ASSERT( aLocale.Variant == s_ca_ES_valencia );
        CPPUNIT_ASSERT( ca_ES_valencia.getLanguageType() == LANGUAGE_CATALAN_VALENCIAN );
        CPPUNIT_ASSERT( ca_ES_valencia.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_ES_valencia.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_ES_valencia.isIsoODF() );
        CPPUNIT_ASSERT( ca_ES_valencia.getLanguage() == "ca" );
        CPPUNIT_ASSERT( ca_ES_valencia.getCountry() == "ES" );
        CPPUNIT_ASSERT( ca_ES_valencia.getScript().isEmpty() );
        CPPUNIT_ASSERT( ca_ES_valencia.getLanguageAndScript() == "ca" );
        ::std::vector< OUString > ca_ES_valencia_Fallbacks( ca_ES_valencia.getFallbackStrings( true));
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks.size() == 5);
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[0] == "ca-ES-valencia");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[1] == "ca-XV");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[2] == "ca-valencia");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[3] == "ca-ES");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[4] == "ca");
        CPPUNIT_ASSERT( ca_ES_valencia.makeFallback().getBcp47() == "ca-ES-valencia");
    }

    {
        OUString s_ca_valencia( "ca-valencia" );
        LanguageTag ca_valencia( s_ca_valencia, true );
        lang::Locale aLocale = ca_valencia.getLocale();
        CPPUNIT_ASSERT( ca_valencia.getBcp47() == s_ca_valencia );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant == s_ca_valencia );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( ca_valencia.getLanguageType()) );
        CPPUNIT_ASSERT( ca_valencia.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_valencia.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_valencia.isIsoODF() );
        CPPUNIT_ASSERT( ca_valencia.getLanguage() == "ca" );
        CPPUNIT_ASSERT( ca_valencia.getCountry().isEmpty() );
        CPPUNIT_ASSERT( ca_valencia.getScript().isEmpty() );
        CPPUNIT_ASSERT( ca_valencia.getLanguageAndScript() == "ca" );
        ::std::vector< OUString > ca_valencia_Fallbacks( ca_valencia.getFallbackStrings( true));
        CPPUNIT_ASSERT( ca_valencia_Fallbacks.size() == 2);
        CPPUNIT_ASSERT( ca_valencia_Fallbacks[0] == "ca-valencia");
        CPPUNIT_ASSERT( ca_valencia_Fallbacks[1] == "ca");
        CPPUNIT_ASSERT( ca_valencia.makeFallback().getBcp47() == "ca-ES-valencia");
    }

    // 'ca-XV' has an internal override to 'ca-ES-valencia'
    {
        OUString s_ca_XV( "ca-XV" );
        OUString s_ca_ES_valencia( "ca-ES-valencia" );
        LanguageTag ca_XV( s_ca_XV, true );
        lang::Locale aLocale = ca_XV.getLocale();
        CPPUNIT_ASSERT( ca_XV.getBcp47() == s_ca_ES_valencia );
        CPPUNIT_ASSERT( aLocale.Language == I18NLANGTAG_QLT );
        CPPUNIT_ASSERT( aLocale.Country == "ES" );
        CPPUNIT_ASSERT( aLocale.Variant == s_ca_ES_valencia );
        CPPUNIT_ASSERT( ca_XV.getLanguageType() == LANGUAGE_CATALAN_VALENCIAN );
        CPPUNIT_ASSERT( ca_XV.isValidBcp47() );
        CPPUNIT_ASSERT( !ca_XV.isIsoLocale() );
        CPPUNIT_ASSERT( !ca_XV.isIsoODF() );
        CPPUNIT_ASSERT( ca_XV.getLanguage() == "ca" );
        CPPUNIT_ASSERT( ca_XV.getCountry() == "ES" );
        CPPUNIT_ASSERT( ca_XV.getScript().isEmpty() );
        CPPUNIT_ASSERT( ca_XV.getLanguageAndScript() == "ca" );
        ::std::vector< OUString > ca_XV_Fallbacks( ca_XV.getFallbackStrings( true));
        CPPUNIT_ASSERT( ca_XV_Fallbacks.size() == 5);
        CPPUNIT_ASSERT( ca_XV_Fallbacks[0] == "ca-ES-valencia");
        CPPUNIT_ASSERT( ca_XV_Fallbacks[1] == "ca-XV");
        CPPUNIT_ASSERT( ca_XV_Fallbacks[2] == "ca-valencia");
        CPPUNIT_ASSERT( ca_XV_Fallbacks[3] == "ca-ES");
        CPPUNIT_ASSERT( ca_XV_Fallbacks[4] == "ca");
        CPPUNIT_ASSERT( ca_XV.makeFallback().getBcp47() == "ca-ES-valencia");
    }

    {
        OUString s_de_DE( "de-DE" );
        LanguageTag de_DE( s_de_DE, true );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT( de_DE.getBcp47() == s_de_DE );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( de_DE.getLanguageType() == LANGUAGE_GERMAN );
        CPPUNIT_ASSERT( de_DE.isValidBcp47() );
        CPPUNIT_ASSERT( de_DE.isIsoLocale() );
        CPPUNIT_ASSERT( de_DE.isIsoODF() );
        CPPUNIT_ASSERT( de_DE.getLanguage() == "de" );
        CPPUNIT_ASSERT( de_DE.getCountry() == "DE" );
        CPPUNIT_ASSERT( de_DE.getScript().isEmpty() );
        CPPUNIT_ASSERT( de_DE.getLanguageAndScript() == "de" );
        ::std::vector< OUString > de_DE_Fallbacks( de_DE.getFallbackStrings( true));
        CPPUNIT_ASSERT( de_DE_Fallbacks.size() == 2);
        CPPUNIT_ASSERT( de_DE_Fallbacks[0] == "de-DE");
        CPPUNIT_ASSERT( de_DE_Fallbacks[1] == "de");
        CPPUNIT_ASSERT( de_DE.makeFallback().getBcp47() == "de-DE");
    }

    {
        OUString s_de_DE( "de-DE" );
        LanguageTag de_DE( lang::Locale( "de", "DE", "" ) );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT( de_DE.getBcp47() == s_de_DE );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( de_DE.getLanguageType() == LANGUAGE_GERMAN );
    }

    {
        OUString s_de_DE( "de-DE" );
        LanguageTag de_DE( LANGUAGE_GERMAN );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT( de_DE.getBcp47() == s_de_DE );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( de_DE.getLanguageType() == LANGUAGE_GERMAN );
    }

    // Unmapped but known language-only.
    {
        OUString s_de( "de" );
        LanguageTag de( s_de, true );
        lang::Locale aLocale = de.getLocale();
        CPPUNIT_ASSERT( de.getBcp47() == s_de );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        LanguageType de_LangID = de.getLanguageType();
        CPPUNIT_ASSERT( de_LangID != LANGUAGE_GERMAN );
        CPPUNIT_ASSERT( de_LangID == MsLangId::getPrimaryLanguage( LANGUAGE_GERMAN) );
        CPPUNIT_ASSERT( de.makeFallback().getBcp47() == "de-DE");
        // Check registered mapping.
        LanguageTag de_l( de_LangID);
        CPPUNIT_ASSERT( de_l.getBcp47() == s_de );
    }

    // "bo" and "dz" share the same primary language ID, only one gets it
    // assigned, "dz" language-only has a special mapping.
    {
        LanguageTag bo( "bo", true );
        CPPUNIT_ASSERT( bo.getLanguageType() == MsLangId::getPrimaryLanguage( LANGUAGE_TIBETAN) );
        LanguageTag dz( "dz", true );
        CPPUNIT_ASSERT( dz.getLanguageType() == LANGUAGE_USER_DZONGKHA_MAP_LONLY );
    }

    // "no", "nb" and "nn" share the same primary language ID, which even is
    // assigned to "no-NO" for legacy so none gets it assigned, all on-the-fly
    // except if there is a defined MS-LCID for LanguageScriptOnly (LSO).
    {
        LanguageTag no( "no", true );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( no.getLanguageType()) );
        LanguageTag nb( "nb", true );
        CPPUNIT_ASSERT( nb.getLanguageType() == LANGUAGE_NORWEGIAN_BOKMAL_LSO );
        LanguageTag nn( "nn", true );
        CPPUNIT_ASSERT( nn.getLanguageType() == LANGUAGE_NORWEGIAN_NYNORSK_LSO );
        LanguageTag no_NO( "no-NO", true );
        CPPUNIT_ASSERT( no_NO.getLanguageType() == LANGUAGE_NORWEGIAN );
    }

    // 'de-1901' derived from 'de-DE-1901' grandfathered to check that it is
    // accepted as (DIGIT 3ALNUM) variant
    {
        OUString s_de_1901( "de-1901" );
        LanguageTag de_1901( s_de_1901 );
        lang::Locale aLocale = de_1901.getLocale();
        CPPUNIT_ASSERT( de_1901.getBcp47() == s_de_1901 );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant == s_de_1901 );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( de_1901.getLanguageType()) );
        CPPUNIT_ASSERT( de_1901.isValidBcp47() );
        CPPUNIT_ASSERT( !de_1901.isIsoLocale() );
        CPPUNIT_ASSERT( !de_1901.isIsoODF() );
        CPPUNIT_ASSERT( de_1901.getLanguageAndScript() == "de" );
        CPPUNIT_ASSERT( de_1901.getVariants() == "1901" );
        ::std::vector< OUString > de_1901_Fallbacks( de_1901.getFallbackStrings( true));
        CPPUNIT_ASSERT( de_1901_Fallbacks.size() == 2);
        CPPUNIT_ASSERT( de_1901_Fallbacks[0] == "de-1901");
        CPPUNIT_ASSERT( de_1901_Fallbacks[1] == "de");
    }

    // 'en-GB-oed' is known grandfathered for English, Oxford English
    // Dictionary spelling.
    // Deprecated as of 2015-04-17, prefer en-GB-oxendict instead.
    {
        OUString s_en_GB_oed( "en-GB-oed" );
        LanguageTag en_GB_oed( s_en_GB_oed );
        lang::Locale aLocale = en_GB_oed.getLocale();
        CPPUNIT_ASSERT( en_GB_oed.getBcp47() == s_en_GB_oed );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "GB" );  // only 'GB' because we handle it, liblangtag would not fill this
        CPPUNIT_ASSERT( aLocale.Variant == s_en_GB_oed );
        CPPUNIT_ASSERT( en_GB_oed.getLanguageType() == LANGUAGE_USER_ENGLISH_UK_OED );
        CPPUNIT_ASSERT( en_GB_oed.isValidBcp47() );
        CPPUNIT_ASSERT( !en_GB_oed.isIsoLocale() );
        CPPUNIT_ASSERT( !en_GB_oed.isIsoODF() );
        CPPUNIT_ASSERT( en_GB_oed.getLanguageAndScript() == "en" );
        CPPUNIT_ASSERT( en_GB_oed.getVariants() == "oed" );
        ::std::vector< OUString > en_GB_oed_Fallbacks( en_GB_oed.getFallbackStrings( true));
        CPPUNIT_ASSERT( en_GB_oed_Fallbacks.size() == 4);
        CPPUNIT_ASSERT( en_GB_oed_Fallbacks[0] == "en-GB-oed");
        CPPUNIT_ASSERT( en_GB_oed_Fallbacks[1] == "en-GB-oxendict");
        CPPUNIT_ASSERT( en_GB_oed_Fallbacks[2] == "en-GB");
        CPPUNIT_ASSERT( en_GB_oed_Fallbacks[3] == "en");
        // 'en-oed' is not a valid fallback!
    }

    // 'en-GB-oxendict' as preferred over 'en-GB-oed'.
    {
        OUString s_en_GB_oxendict( "en-GB-oxendict" );
        LanguageTag en_GB_oxendict( s_en_GB_oxendict );
        lang::Locale aLocale = en_GB_oxendict.getLocale();
        CPPUNIT_ASSERT( en_GB_oxendict.getBcp47() == s_en_GB_oxendict );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "GB" );
        CPPUNIT_ASSERT( aLocale.Variant == s_en_GB_oxendict );
        CPPUNIT_ASSERT( en_GB_oxendict.getLanguageType() == LANGUAGE_USER_ENGLISH_UK_OXENDICT );
        CPPUNIT_ASSERT( en_GB_oxendict.isValidBcp47() );
        CPPUNIT_ASSERT( !en_GB_oxendict.isIsoLocale() );
        CPPUNIT_ASSERT( !en_GB_oxendict.isIsoODF() );
        CPPUNIT_ASSERT( en_GB_oxendict.getLanguageAndScript() == "en" );
        CPPUNIT_ASSERT( en_GB_oxendict.getVariants() == "oxendict" );
        ::std::vector< OUString > en_GB_oxendict_Fallbacks( en_GB_oxendict.getFallbackStrings( true));
        CPPUNIT_ASSERT( en_GB_oxendict_Fallbacks.size() == 5);
        CPPUNIT_ASSERT( en_GB_oxendict_Fallbacks[0] == "en-GB-oxendict");
        CPPUNIT_ASSERT( en_GB_oxendict_Fallbacks[1] == "en-GB-oed");
        CPPUNIT_ASSERT( en_GB_oxendict_Fallbacks[2] == "en-oxendict");
        CPPUNIT_ASSERT( en_GB_oxendict_Fallbacks[3] == "en-GB");
        CPPUNIT_ASSERT( en_GB_oxendict_Fallbacks[4] == "en");
    }

#if USE_LIBLANGTAG
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
#endif

    // 'qtz' is a local use known pseudolocale for key ID resource
    {
        OUString s_qtz( "qtz" );
        LanguageTag qtz( s_qtz );
        lang::Locale aLocale = qtz.getLocale();
        CPPUNIT_ASSERT( qtz.getBcp47() == s_qtz );
        CPPUNIT_ASSERT( aLocale.Language == "qtz" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( qtz.getLanguageType() == LANGUAGE_USER_KEYID );
    }

    // 'qty' is a local use unknown locale
    {
        OUString s_qty( "qty" );
        LanguageTag qty( s_qty );
        lang::Locale aLocale = qty.getLocale();
        CPPUNIT_ASSERT( qty.getBcp47() == s_qty );
        CPPUNIT_ASSERT( aLocale.Language == "qty" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( qty.getLanguageType()) );
    }

    // 'x-comment' is a privateuse known "locale"
    {
        OUString s_xcomment( "x-comment" );
        LanguageTag xcomment( s_xcomment );
        lang::Locale aLocale = xcomment.getLocale();
        CPPUNIT_ASSERT( xcomment.getBcp47() == s_xcomment );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant == "x-comment" );
        CPPUNIT_ASSERT( xcomment.getLanguageType() == LANGUAGE_USER_PRIV_COMMENT );
    }

    // 'x-foobar' is a privateuse unknown "locale"
    {
        OUString s_xfoobar( "x-foobar" );
        LanguageTag xfoobar( s_xfoobar );
        lang::Locale aLocale = xfoobar.getLocale();
        CPPUNIT_ASSERT( xfoobar.getBcp47() == s_xfoobar );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant == "x-foobar" );
        CPPUNIT_ASSERT( LanguageTag::isOnTheFlyID( xfoobar.getLanguageType()) );
    }

    // '*' the dreaded jolly joker is a "privateuse" known "locale"
    {
        OUString s_joker( "*" );
        LanguageTag joker( s_joker );
        lang::Locale aLocale = joker.getLocale();
        CPPUNIT_ASSERT( joker.getBcp47() == s_joker );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant == "*" );
        CPPUNIT_ASSERT( joker.getLanguageType() == LANGUAGE_USER_PRIV_JOKER );

        joker.reset( LANGUAGE_USER_PRIV_JOKER );
        aLocale = joker.getLocale();
        CPPUNIT_ASSERT( joker.getBcp47() == s_joker );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant == "*" );
        CPPUNIT_ASSERT( joker.getLanguageType() == LANGUAGE_USER_PRIV_JOKER );
    }

    // test reset() methods
    {
        LanguageTag aTag( LANGUAGE_DONTKNOW );
        lang::Locale aLocale;

        aTag.reset( LANGUAGE_GERMAN );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT( aTag.getBcp47() == "de-DE" );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( aTag.getLanguageType() == LANGUAGE_GERMAN );

        aTag.reset( "en-US" );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT( aTag.getBcp47() == "en-US" );
        CPPUNIT_ASSERT( aLocale.Language == "en" );
        CPPUNIT_ASSERT( aLocale.Country == "US" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( aTag.getLanguageType() == LANGUAGE_ENGLISH_US );

        aTag.reset( lang::Locale( "de", "DE", "" ) );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT( aTag.getBcp47() == "de-DE" );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant.isEmpty() );
        CPPUNIT_ASSERT( aTag.getLanguageType() == LANGUAGE_GERMAN );
    }

    {
        OUString s_uab( "unreg-and-bad" );
        LanguageTag uab( s_uab, true );
        lang::Locale aLocale = uab.getLocale();
        CPPUNIT_ASSERT( uab.getBcp47() == s_uab );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country.isEmpty() );
        CPPUNIT_ASSERT( aLocale.Variant == s_uab );
        CPPUNIT_ASSERT( uab.getLanguageType() == LANGUAGE_DONTKNOW );
        CPPUNIT_ASSERT( !uab.isValidBcp47() );
        CPPUNIT_ASSERT( !uab.isIsoLocale() );
        CPPUNIT_ASSERT( !uab.isIsoODF() );
    }

    // test static isValidBcp47() method
    {
        OUString aCanonicalized;
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "en-US", &aCanonicalized) && aCanonicalized == "en-US" );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "x-foobar", &aCanonicalized) && aCanonicalized == "x-foobar" );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "qaa", &aCanonicalized) && aCanonicalized == "qaa" );
        CPPUNIT_ASSERT( !LanguageTag::isValidBcp47( "unreg-and-bad", &aCanonicalized) );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "en-US", &aCanonicalized, true) && aCanonicalized == "en-US" );
        CPPUNIT_ASSERT( !LanguageTag::isValidBcp47( "x-foobar", &aCanonicalized, true) && aCanonicalized == "x-foobar" );
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "qaa", &aCanonicalized, true) && aCanonicalized == "qaa" );
#if USE_LIBLANGTAG
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "de-Latn-DE", &aCanonicalized) && aCanonicalized == "de-DE" );
        /* TODO: at least some (those we know) grandfathered tags should be
         * recognized by the replacement code. */
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "en-GB-oed", &aCanonicalized) );
        // en-GB-oed has become deprecated in updated language-subtag-registry database
        // (liblangtag 0.5.7)
        CPPUNIT_ASSERT( ( aCanonicalized == "en-GB-oxendict" ) || ( aCanonicalized == "en-GB-oed" ) );
#else
        CPPUNIT_ASSERT( LanguageTag::isValidBcp47( "de-Latn-DE", &aCanonicalized) && aCanonicalized == "de-Latn-DE" );
#endif
    }
}

static bool checkMapping( const OUString& rStr1, const OUString& rStr2 )
{
    if (rStr1 == "la-Latn"     ) return rStr2 == "la";
    if (rStr1 == "tzm-Latn-DZ" ) return rStr2 == "kab-DZ";
    if (rStr1 == "bs-Latn-BA"  ) return rStr2 == "bs-BA";
    if (rStr1 == "bs-Latn"     ) return rStr2 == "bs";
    if (rStr1 == "cz"          ) return rStr2 == "cs-CZ";
    if (rStr1 == "iw-IL"       ) return rStr2 == "he-IL";
    if (rStr1 == "in-ID"       ) return rStr2 == "id-ID";
    if (rStr1 == "sr-YU"       ) return rStr2 == "sr-CS";
    if (rStr1 == "sh-RS"       ) return rStr2 == "sr-Latn-RS";
    if (rStr1 == "sh-YU"       ) return rStr2 == "sr-Latn-CS";
    if (rStr1 == "sh-CS"       ) return rStr2 == "sr-Latn-CS";
    if (rStr1 == "sh-ME"       ) return rStr2 == "sr-Latn-ME";
    if (rStr1 == "sh-BA"       ) return rStr2 == "sr-Latn-BA";
    if (rStr1 == "sh"          ) return rStr2 == "sr-Latn";
    if (rStr1 == "lah-PK"      ) return rStr2 == "pnb-Arab-PK";
    if (rStr1 == "pa-PK"       ) return rStr2 == "pnb-Arab-PK";
    if (rStr1 == "ca-XV"       ) return rStr2 == "ca-ES-valencia";
    if (rStr1 == "qcv-ES"      ) return rStr2 == "ca-ES-valencia";
    if (rStr1 == "ns-ZA"       ) return rStr2 == "nso-ZA";
    if (rStr1 == "ven-ZA"      ) return rStr2 == "ve-ZA";
    if (rStr1 == "qu-EC"       ) return rStr2 == "quz-EC";
    if (rStr1 == "qu-PE"       ) return rStr2 == "quz-PE";
    if (rStr1 == "ff-NG"       ) return rStr2 == "fuv-NG";
    if (rStr1 == "ji-IL"       ) return rStr2 == "yi-IL";
    if (rStr1 == "iu-CA"       ) return rStr2 == "iu-Latn-CA";
    if (rStr1 == "iu"          ) return rStr2 == "iu-Latn";
    if (rStr1 == "gbz-AF"      ) return rStr2 == "prs-AF";
    if (rStr1 == "ber-DZ"      ) return rStr2 == "kab-DZ";
    if (rStr1 == "tmz-MA"      ) return rStr2 == "tzm-Tfng-MA";
    if (rStr1 == "ber-MA"      ) return rStr2 == "tzm-Tfng-MA";
    if (rStr1 == "mg-MG"       ) return rStr2 == "plt-MG";
    if (rStr1 == "pli"         ) return rStr2 == "pi-Latn";
    if (rStr1 == "ks"          ) return rStr2 == "ks-Arab";
    if (rStr1 == "chr-US"      ) return rStr2 == "chr-Cher-US";
    if (rStr1 == "sd-PK"       ) return rStr2 == "sd-Arab-PK";
    if (rStr1 == "sr-Cyrl-RS"  ) return rStr2 == "sr-RS";
    if (rStr1 == "sr-Cyrl-ME"  ) return rStr2 == "sr-ME";
    if (rStr1 == "sr-Cyrl-BA"  ) return rStr2 == "sr-BA";
    if (rStr1 == "sr-Cyrl-CS"  ) return rStr2 == "sr-CS";
    if (rStr1 == "sr-Cyrl"     ) return rStr2 == "sr";
    if (rStr1 == "yi-Hebr-US"  ) return rStr2 == "yi-US";
    if (rStr1 == "yi-Hebr-IL"  ) return rStr2 == "yi-IL";
    if (rStr1 == "ha-NG"       ) return rStr2 == "ha-Latn-NG";
    if (rStr1 == "ha-GH"       ) return rStr2 == "ha-Latn-GH";
    if (rStr1 == "ku-Arab-IQ"  ) return rStr2 == "ckb-IQ";
    if (rStr1 == "ku-Arab"     ) return rStr2 == "ckb";
    if (rStr1 == "kmr-TR"      ) return rStr2 == "kmr-Latn-TR";
    if (rStr1 == "ku-TR"       ) return rStr2 == "kmr-Latn-TR";
    if (rStr1 == "kmr-SY"      ) return rStr2 == "kmr-Latn-SY";
    if (rStr1 == "ku-SY"       ) return rStr2 == "kmr-Latn-SY";
    if (rStr1 == "ku-IQ"       ) return rStr2 == "ckb-IQ";
    if (rStr1 == "ku-IR"       ) return rStr2 == "ckb-IR";
    return rStr1 == rStr2;
}

void TestLanguageTag::testAllIsoLangEntries()
{
    const ::std::vector< MsLangId::LanguagetagMapping > aList( MsLangId::getDefinedLanguagetags());
    for (::std::vector< MsLangId::LanguagetagMapping >::const_iterator it( aList.begin()); it != aList.end(); ++it)
    {
        bool b=false;
        if ((*it).maBcp47 == "la-VA")
            b=true;
        (void)b;

        LanguageTag aTagString( (*it).maBcp47, true);
        LanguageTag aTagID( (*it).mnLang);
        if (!checkMapping( (*it).maBcp47, aTagString.getBcp47()))
        {
            OString aMessage( OUStringToOString( (*it).maBcp47, RTL_TEXTENCODING_ASCII_US));
            aMessage += " -> " + OUStringToOString( aTagString.getBcp47(), RTL_TEXTENCODING_ASCII_US);
            CPPUNIT_ASSERT_MESSAGE( aMessage.getStr(), (*it).maBcp47 == aTagString.getBcp47());
        }
        if ((*it).maBcp47 != aTagID.getBcp47())
        {
            // There are multiple mappings, ID must be equal after conversions.
            LanguageTag aTagBack( aTagID.getBcp47(), true);
            if (aTagString.getLanguageType() != aTagBack.getLanguageType())
            {
                OString aMessage( OUStringToOString( (*it).maBcp47, RTL_TEXTENCODING_ASCII_US));
                aMessage += " " + OUStringToOString( aTagString.getBcp47(), RTL_TEXTENCODING_ASCII_US) + ": " +
                    OUStringToOString( aTagString.getBcp47(), RTL_TEXTENCODING_ASCII_US) + " " +
                    OString::number( aTagString.getLanguageType(), 16) +
                    " -> " + OUStringToOString( aTagBack.getBcp47(), RTL_TEXTENCODING_ASCII_US) + " " +
                    OString::number( aTagBack.getLanguageType(), 16);
                CPPUNIT_ASSERT_MESSAGE( aMessage.getStr(), aTagString.getLanguageType() == aTagBack.getLanguageType());
            }
        }
#if 0
        // This does not hold, there are cases like 'ar'
        // LANGUAGE_ARABIC_PRIMARY_ONLY that when mapped back results in
        // 'ar-SA' as default locale.
        if ((*it).mnLang != aTagString.getLanguageType())
        {
            // There are multiple mappings, string must be equal after conversions.
            LanguageTag aTagBack( aTagString.getLanguageType());
            if (aTagID.getBcp47() != aTagBack.getBcp47())
            {
                OString aMessage( OUStringToOString( (*it).maBcp47, RTL_TEXTENCODING_ASCII_US));
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

void TestLanguageTag::testAllrfc3066bisConverter()
{
    {
        OUString s_ja_JP( "ja-JP" );
        LanguageTag ja_JP( s_ja_JP, true );
        OUString aBcp47 = ja_JP.getBcp47();
        lang::Locale aLocale = ja_JP.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "ja-JP" );
        CPPUNIT_ASSERT( ja_JP.getRFC3066bisLanguage() == "ja" );
    }

    {
        OUString s_zh_CN( "zh-CN" );
        LanguageTag zh_CN( s_zh_CN, true );
        OUString aBcp47 = zh_CN.getBcp47();
        lang::Locale aLocale = zh_CN.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "zh-CN" );
        CPPUNIT_ASSERT( zh_CN.getRFC3066bisLanguage() == "zh-Hans" );
    }

    {
        OUString s_zh_TW( "zh-TW" );
        LanguageTag zh_TW( s_zh_TW, true );
        OUString aBcp47 = zh_TW.getBcp47();
        lang::Locale aLocale = zh_TW.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "zh-TW" );
        CPPUNIT_ASSERT( zh_TW.getRFC3066bisLanguage() == "zh-Hant" );
    }

    {
        OUString s_de_DE( "de-DE" );
        LanguageTag de_DE( s_de_DE, true );
        OUString aBcp47 = de_DE.getBcp47();
        lang::Locale aLocale = de_DE.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "de-DE" );
        CPPUNIT_ASSERT( de_DE.getRFC3066bisLanguage() == "de" );
    }

    {
        OUString s_fr_FR( "fr-FR" );
        LanguageTag fr_FR( s_fr_FR, true );
        OUString aBcp47 = fr_FR.getBcp47();
        lang::Locale aLocale = fr_FR.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "fr-FR" );
        CPPUNIT_ASSERT( fr_FR.getRFC3066bisLanguage() == "fr" );
    }

    {
        OUString s_es_ES( "es-ES" );
        LanguageTag es_ES( s_es_ES, true );
        OUString aBcp47 = es_ES.getBcp47();
        lang::Locale aLocale = es_ES.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "es-ES" );
        CPPUNIT_ASSERT( es_ES.getRFC3066bisLanguage() == "es" );
    }

    {
        OUString s_it_IT( "it-IT" );
        LanguageTag it_IT( s_it_IT, true );
        OUString aBcp47 = it_IT.getBcp47();
        lang::Locale aLocale = it_IT.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "it-IT" );
        CPPUNIT_ASSERT( it_IT.getRFC3066bisLanguage() == "it" );
    }

    {
        OUString s_nl_NL( "nl-NL" );
        LanguageTag nl_NL( s_nl_NL, true );
        OUString aBcp47 = nl_NL.getBcp47();
        lang::Locale aLocale = nl_NL.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "nl-NL" );
        CPPUNIT_ASSERT( nl_NL.getRFC3066bisLanguage() == "nl" );
    }

    {
        OUString s_ko_KR( "ko-KR" );
        LanguageTag ko_KR( s_ko_KR, true );
        OUString aBcp47 = ko_KR.getBcp47();
        lang::Locale aLocale = ko_KR.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "ko-KR" );
        CPPUNIT_ASSERT( ko_KR.getRFC3066bisLanguage() == "ko" );
    }

    {
        OUString s_pt_BR( "pt-BR" );
        LanguageTag pt_BR( s_pt_BR, true );
        OUString aBcp47 = pt_BR.getBcp47();
        lang::Locale aLocale = pt_BR.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "pt-BR" );
        CPPUNIT_ASSERT( pt_BR.getRFC3066bisLanguage() == "pt" );
    }

    {
        OUString s_pt_PT( "pt-PT" );
        LanguageTag pt_PT( s_pt_PT, true );
        OUString aBcp47 = pt_PT.getBcp47();
        lang::Locale aLocale = pt_PT.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "pt-PT" );
        CPPUNIT_ASSERT( pt_PT.getRFC3066bisLanguage() == "pt-PT" );
    }

    {
        OUString s_da_DK( "da-DK" );
        LanguageTag da_DK( s_da_DK, true );
        OUString aBcp47 = da_DK.getBcp47();
        lang::Locale aLocale = da_DK.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "da-DK" );
        CPPUNIT_ASSERT( da_DK.getRFC3066bisLanguage() == "da" );
    }

    {
        OUString s_fi_FI( "fi-FI" );
        LanguageTag fi_FI( s_fi_FI, true );
        OUString aBcp47 = fi_FI.getBcp47();
        lang::Locale aLocale = fi_FI.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "fi-FI" );
        CPPUNIT_ASSERT( fi_FI.getRFC3066bisLanguage() == "fi" );
    }

    {
        OUString s_nb_NO( "nb-NO" );
        LanguageTag nb_NO( s_nb_NO, true );
        OUString aBcp47 = nb_NO.getBcp47();
        lang::Locale aLocale = nb_NO.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "nb-NO" );
        CPPUNIT_ASSERT( nb_NO.getRFC3066bisLanguage() == "nb" );
    }

    {
        OUString s_sv_SE( "sv-SE" );
        LanguageTag sv_SE( s_sv_SE, true );
        OUString aBcp47 = sv_SE.getBcp47();
        lang::Locale aLocale = sv_SE.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "sv-SE" );
        CPPUNIT_ASSERT( sv_SE.getRFC3066bisLanguage() == "sv" );
    }

    {
        OUString s_ru_RU( "ru-RU" );
        LanguageTag ru_RU( s_ru_RU, true );
        OUString aBcp47 = ru_RU.getBcp47();
        lang::Locale aLocale = ru_RU.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "ru-RU" );
        CPPUNIT_ASSERT( ru_RU.getRFC3066bisLanguage() == "ru" );
    }

    {
        OUString s_pl_PL( "pl-PL" );
        LanguageTag pl_PL( s_pl_PL, true );
        OUString aBcp47 = pl_PL.getBcp47();
        lang::Locale aLocale = pl_PL.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "pl-PL" );
        CPPUNIT_ASSERT( pl_PL.getRFC3066bisLanguage() == "pl" );
    }

    {
        OUString s_tr_TR( "tr-TR" );
        LanguageTag tr_TR( s_tr_TR, true );
        OUString aBcp47 = tr_TR.getBcp47();
        lang::Locale aLocale = tr_TR.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "tr-TR" );
        CPPUNIT_ASSERT( tr_TR.getRFC3066bisLanguage() == "tr" );
    }

    {
        OUString s_ar_SA( "ar-SA" );
        LanguageTag ar_SA( s_ar_SA, true );
        OUString aBcp47 = ar_SA.getBcp47();
        lang::Locale aLocale = ar_SA.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "ar-SA" );
        CPPUNIT_ASSERT( ar_SA.getRFC3066bisLanguage() == "ar" );
    }

    {
        OUString s_th_TH( "th-TH" );
        LanguageTag th_TH( s_th_TH, true );
        OUString aBcp47 = th_TH.getBcp47();
        lang::Locale aLocale = th_TH.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "th-TH" );
        CPPUNIT_ASSERT( th_TH.getRFC3066bisLanguage() == "th" );
    }

    {
        OUString s_cs_CZ( "cs-CZ" );
        LanguageTag cs_CZ( s_cs_CZ, true );
        OUString aBcp47 = cs_CZ.getBcp47();
        lang::Locale aLocale = cs_CZ.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "cs-CZ" );
        CPPUNIT_ASSERT( cs_CZ.getRFC3066bisLanguage() == "cs" );
    }

    {
        OUString s_hu_HU( "hu-HU" );
        LanguageTag hu_HU( s_hu_HU, true );
        OUString aBcp47 = hu_HU.getBcp47();
        lang::Locale aLocale = hu_HU.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "hu-HU" );
        CPPUNIT_ASSERT( hu_HU.getRFC3066bisLanguage() == "hu" );
    }

    {
        OUString s_ca_ES( "ca-ES" );
        LanguageTag ca_ES( s_ca_ES, true );
        OUString aBcp47 = ca_ES.getBcp47();
        lang::Locale aLocale = ca_ES.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "ca-ES" );
        CPPUNIT_ASSERT( ca_ES.getRFC3066bisLanguage() == "ca" );
    }

    {
        OUString s_hr_HR( "hr-HR" );
        LanguageTag hr_HR( s_hr_HR, true );
        OUString aBcp47 = hr_HR.getBcp47();
        lang::Locale aLocale = hr_HR.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "hr-HR" );
        CPPUNIT_ASSERT( hr_HR.getRFC3066bisLanguage() == "hr" );
    }

    {
        OUString s_el_GR( "el-GR" );
        LanguageTag el_GR( s_el_GR, true );
        OUString aBcp47 = el_GR.getBcp47();
        lang::Locale aLocale = el_GR.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "el-GR" );
        CPPUNIT_ASSERT( el_GR.getRFC3066bisLanguage() == "el" );
    }

    {
        OUString s_he_IL( "he-IL" );
        LanguageTag he_IL( s_he_IL, true );
        OUString aBcp47 = he_IL.getBcp47();
        lang::Locale aLocale = he_IL.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "he-IL" );
        CPPUNIT_ASSERT( he_IL.getRFC3066bisLanguage() == "he" );
    }

    {
        OUString s_ro_RO( "ro-RO" );
        LanguageTag ro_RO( s_ro_RO, true );
        OUString aBcp47 = ro_RO.getBcp47();
        lang::Locale aLocale = ro_RO.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "ro-RO" );
        CPPUNIT_ASSERT( ro_RO.getRFC3066bisLanguage() == "ro" );
    }

    {
        OUString s_sk_SK( "sk-SK" );
        LanguageTag sk_SK( s_sk_SK, true );
        OUString aBcp47 = sk_SK.getBcp47();
        lang::Locale aLocale = sk_SK.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "sk-SK" );
        CPPUNIT_ASSERT( sk_SK.getRFC3066bisLanguage() == "sk" );
    }

    {
        OUString s_uk_UA( "uk-UA" );
        LanguageTag uk_UA( s_uk_UA, true );
        OUString aBcp47 = uk_UA.getBcp47();
        lang::Locale aLocale = uk_UA.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "uk-UA" );
        CPPUNIT_ASSERT( uk_UA.getLanguageAndScript() == "uk" );
    }

    {
        OUString s_id_ID( "id-ID" );
        LanguageTag id_ID( s_id_ID, true );
        OUString aBcp47 = id_ID.getBcp47();
        lang::Locale aLocale = id_ID.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "id-ID" );
        CPPUNIT_ASSERT( id_ID.getRFC3066bisLanguage() == "id" );
    }

    {
        OUString s_vi_VN( "vi-VN" );
        LanguageTag vi_VN( s_vi_VN, true );
        OUString aBcp47 = vi_VN.getBcp47();
        lang::Locale aLocale = vi_VN.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "vi-VN" );
        CPPUNIT_ASSERT( vi_VN.getRFC3066bisLanguage() == "vi" );
    }

    {
        OUString s_en_GB( "en-GB" );
        LanguageTag en_GB( s_en_GB, true );
        OUString aBcp47 = en_GB.getBcp47();
        lang::Locale aLocale = en_GB.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "en-GB" );
        CPPUNIT_ASSERT( en_GB.getRFC3066bisLanguage() == "en-GB" );
    }

    {
        OUString s_is_IS( "is-IS" );
        LanguageTag is_IS( s_is_IS, true );
        OUString aBcp47 = is_IS.getBcp47();
        lang::Locale aLocale = is_IS.getLocale();

        CPPUNIT_ASSERT_MESSAGE("Default script should be stripped after canonicalize.", aBcp47 == "is-IS" );
        CPPUNIT_ASSERT( is_IS.getRFC3066bisLanguage() == "is" );
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION( TestLanguageTag );

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
