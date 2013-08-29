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

    CPPUNIT_TEST_SUITE(TestLanguageTag);
    CPPUNIT_TEST(testAllTags);
    CPPUNIT_TEST(testAllIsoLangEntries);
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
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( nLanguageType == LANGUAGE_GERMAN );
        CPPUNIT_ASSERT( de_DE.getLanguage() == "de" );
        CPPUNIT_ASSERT( de_DE.getCountry() == "DE" );
        CPPUNIT_ASSERT( de_DE.getScript() == "" );
        CPPUNIT_ASSERT( de_DE.getLanguageAndScript() == "de" );
#else
        // The simple replacement code doesn't do any fancy stuff.
        CPPUNIT_ASSERT_MESSAGE("Default script was stripped after canonicalize!?!", aBcp47 == s_de_Latn_DE );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant == "de-Latn-DE" );
        CPPUNIT_ASSERT( nLanguageType == LANGUAGE_SYSTEM );     // XXX not resolved!
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
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( klingon.getLanguageType() == LANGUAGE_SYSTEM );
        CPPUNIT_ASSERT( klingon.isValidBcp47() == true );
        CPPUNIT_ASSERT( klingon.isIsoLocale() == true );
        CPPUNIT_ASSERT( klingon.isIsoODF() == true );
#else
        CPPUNIT_ASSERT( klingon.getBcp47() == s_klingon );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == s_klingon );
        CPPUNIT_ASSERT( klingon.getLanguageType() == LANGUAGE_SYSTEM );
        CPPUNIT_ASSERT( klingon.isValidBcp47() == true );
        CPPUNIT_ASSERT( klingon.isIsoLocale() == false );
        CPPUNIT_ASSERT( klingon.isIsoODF() == false );
#endif
    }

    {
        OUString s_sr_RS( "sr-RS" );
        LanguageTag sr_RS( s_sr_RS, true );
        lang::Locale aLocale = sr_RS.getLocale();
        CPPUNIT_ASSERT( sr_RS.getBcp47() == s_sr_RS );
        CPPUNIT_ASSERT( aLocale.Language == "sr" );
        CPPUNIT_ASSERT( aLocale.Country == "RS" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( sr_RS.getLanguageType() == LANGUAGE_USER_SERBIAN_CYRILLIC_SERBIA );
        CPPUNIT_ASSERT( sr_RS.isValidBcp47() == true );
        CPPUNIT_ASSERT( sr_RS.isIsoLocale() == true );
        CPPUNIT_ASSERT( sr_RS.isIsoODF() == true );
    }

    {
        OUString s_sr_Latn_RS( "sr-Latn-RS" );
        LanguageTag sr_RS( s_sr_Latn_RS, true );
        lang::Locale aLocale = sr_RS.getLocale();
        CPPUNIT_ASSERT( sr_RS.getBcp47() == s_sr_Latn_RS );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "RS" );
        CPPUNIT_ASSERT( aLocale.Variant == s_sr_Latn_RS );
        /* TODO: conversion doesn't know this yet, once it does activate test. */
#if 0
        CPPUNIT_ASSERT( sr_RS.getLanguageType() == LANGUAGE_USER_SERBIAN_LATIN_SERBIA );
#else
        CPPUNIT_ASSERT( sr_RS.getLanguageType() == LANGUAGE_SYSTEM );
#endif
        CPPUNIT_ASSERT( sr_RS.isValidBcp47() == true );
        CPPUNIT_ASSERT( sr_RS.isIsoLocale() == false );
        CPPUNIT_ASSERT( sr_RS.isIsoODF() == true );
        CPPUNIT_ASSERT( sr_RS.getLanguage() == "sr" );
        CPPUNIT_ASSERT( sr_RS.getCountry() == "RS" );
        CPPUNIT_ASSERT( sr_RS.getScript() == "Latn" );
        CPPUNIT_ASSERT( sr_RS.getLanguageAndScript() == "sr-Latn" );
    }

    {
        OUString s_ca_ES_valencia( "ca-ES-valencia" );
        LanguageTag ca_ES_valencia( s_ca_ES_valencia, true );
        lang::Locale aLocale = ca_ES_valencia.getLocale();
        CPPUNIT_ASSERT( ca_ES_valencia.getBcp47() == s_ca_ES_valencia );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "ES" );
        CPPUNIT_ASSERT( aLocale.Variant == s_ca_ES_valencia );
        /* TODO: conversion doesn't know this yet, once it does activate test. */
#if 0
        CPPUNIT_ASSERT( ca_ES_valencia.getLanguageType() == LANGUAGE_USER_CATALAN_VALENCIAN );
#else
        CPPUNIT_ASSERT( ca_ES_valencia.getLanguageType() == LANGUAGE_SYSTEM );
#endif
        CPPUNIT_ASSERT( ca_ES_valencia.isValidBcp47() == true );
        CPPUNIT_ASSERT( ca_ES_valencia.isIsoLocale() == false );
        CPPUNIT_ASSERT( ca_ES_valencia.isIsoODF() == false );
        CPPUNIT_ASSERT( ca_ES_valencia.getLanguage() == "ca" );
        CPPUNIT_ASSERT( ca_ES_valencia.getCountry() == "ES" );
        CPPUNIT_ASSERT( ca_ES_valencia.getScript() == "" );
        CPPUNIT_ASSERT( ca_ES_valencia.getLanguageAndScript() == "ca" );
        ::std::vector< OUString > ca_ES_valencia_Fallbacks( ca_ES_valencia.getFallbackStrings());
        /* TODO: replacement doesn't handle variants yet. */
#if USE_LIBLANGTAG
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks.size() == 4);
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[0] == "ca-ES-valencia");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[1] == "ca-valencia");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[2] == "ca-ES");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[3] == "ca");
#else
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks.size() == 3);
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[0] == "ca-ES-valencia");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[1] == "ca-ES");
        CPPUNIT_ASSERT( ca_ES_valencia_Fallbacks[2] == "ca");
#endif
        /* TODO: conversion doesn't know this yet, once it does activate test. */
#if 0
        CPPUNIT_ASSERT( ca_ES_valencia.makeFallback().getBcp47() == "ca-ES-valencia");
#else
        CPPUNIT_ASSERT( ca_ES_valencia.makeFallback().getBcp47() == "ca-ES");
#endif
    }

    {
        /* TODO: replacement doesn't handle variants yet. */
#if USE_LIBLANGTAG
        OUString s_ca_valencia( "ca-valencia" );
        LanguageTag ca_valencia( s_ca_valencia, true );
        lang::Locale aLocale = ca_valencia.getLocale();
        CPPUNIT_ASSERT( ca_valencia.getBcp47() == s_ca_valencia );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == s_ca_valencia );
        CPPUNIT_ASSERT( ca_valencia.getLanguageType() == LANGUAGE_SYSTEM );
        CPPUNIT_ASSERT( ca_valencia.isValidBcp47() == true );
        CPPUNIT_ASSERT( ca_valencia.isIsoLocale() == false );
        CPPUNIT_ASSERT( ca_valencia.isIsoODF() == false );
        CPPUNIT_ASSERT( ca_valencia.getLanguage() == "ca" );
        CPPUNIT_ASSERT( ca_valencia.getCountry() == "" );
        CPPUNIT_ASSERT( ca_valencia.getScript() == "" );
        CPPUNIT_ASSERT( ca_valencia.getLanguageAndScript() == "ca" );
        ::std::vector< OUString > ca_valencia_Fallbacks( ca_valencia.getFallbackStrings());
        CPPUNIT_ASSERT( ca_valencia_Fallbacks.size() == 2);
        CPPUNIT_ASSERT( ca_valencia_Fallbacks[0] == "ca-valencia");
        CPPUNIT_ASSERT( ca_valencia_Fallbacks[1] == "ca");
        /* TODO: conversion doesn't know this yet, once it does activate test. */
#if 0
        CPPUNIT_ASSERT( ca_valencia.makeFallback().getBcp47() == "ca-ES-valencia");
#else
        CPPUNIT_ASSERT( ca_valencia.makeFallback().getBcp47() == "ca-ES");
#endif
#endif
    }

    {
        OUString s_de_DE( "de-DE" );
        LanguageTag de_DE( s_de_DE, true );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT( de_DE.getBcp47() == s_de_DE );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( de_DE.getLanguageType() == LANGUAGE_GERMAN );
        CPPUNIT_ASSERT( de_DE.isValidBcp47() == true );
        CPPUNIT_ASSERT( de_DE.isIsoLocale() == true );
        CPPUNIT_ASSERT( de_DE.isIsoODF() == true );
        CPPUNIT_ASSERT( de_DE.getLanguage() == "de" );
        CPPUNIT_ASSERT( de_DE.getCountry() == "DE" );
        CPPUNIT_ASSERT( de_DE.getScript() == "" );
        CPPUNIT_ASSERT( de_DE.getLanguageAndScript() == "de" );
    }

    {
        OUString s_de_DE( "de-DE" );
        LanguageTag de_DE( lang::Locale( "de", "DE", "" ) );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT( de_DE.getBcp47() == s_de_DE );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( de_DE.getLanguageType() == LANGUAGE_GERMAN );
    }

    {
        OUString s_de_DE( "de-DE" );
        LanguageTag de_DE( LANGUAGE_GERMAN );
        lang::Locale aLocale = de_DE.getLocale();
        CPPUNIT_ASSERT( de_DE.getBcp47() == s_de_DE );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( de_DE.getLanguageType() == LANGUAGE_GERMAN );
    }

    // 'qtz' is a local use known pseudolocale for key ID resource
    {
        OUString s_qtz( "qtz" );
        LanguageTag qtz( s_qtz );
        lang::Locale aLocale = qtz.getLocale();
        CPPUNIT_ASSERT( qtz.getBcp47() == s_qtz );
        CPPUNIT_ASSERT( aLocale.Language == "qtz" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( qtz.getLanguageType() == LANGUAGE_USER_KEYID );
    }

    // 'qty' is a local use unknown locale
    {
        OUString s_qty( "qty" );
        LanguageTag qty( s_qty );
        lang::Locale aLocale = qty.getLocale();
        CPPUNIT_ASSERT( qty.getBcp47() == s_qty );
        CPPUNIT_ASSERT( aLocale.Language == "qty" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( qty.getLanguageType() == LANGUAGE_SYSTEM );
    }

    // 'x-comment' is a privateuse known "locale"
    {
        OUString s_xcomment( "x-comment" );
        LanguageTag xcomment( s_xcomment );
        lang::Locale aLocale = xcomment.getLocale();
        CPPUNIT_ASSERT( xcomment.getBcp47() == s_xcomment );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
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
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == "x-foobar" );
        CPPUNIT_ASSERT( xfoobar.getLanguageType() == LANGUAGE_SYSTEM );
    }

    // '*' the dreaded jolly joker is a "privateuse" known "locale"
    {
        OUString s_joker( "*" );
        LanguageTag joker( s_joker );
        lang::Locale aLocale = joker.getLocale();
        CPPUNIT_ASSERT( joker.getBcp47() == s_joker );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == "*" );
        CPPUNIT_ASSERT( joker.getLanguageType() == LANGUAGE_USER_PRIV_JOKER );

        joker.reset( LANGUAGE_USER_PRIV_JOKER );
        aLocale = joker.getLocale();
        CPPUNIT_ASSERT( joker.getBcp47() == s_joker );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
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
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( aTag.getLanguageType() == LANGUAGE_GERMAN );

        aTag.reset( "en-US" );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT( aTag.getBcp47() == "en-US" );
        CPPUNIT_ASSERT( aLocale.Language == "en" );
        CPPUNIT_ASSERT( aLocale.Country == "US" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( aTag.getLanguageType() == LANGUAGE_ENGLISH_US );

        aTag.reset( lang::Locale( "de", "DE", "" ) );
        aLocale = aTag.getLocale();
        CPPUNIT_ASSERT( aTag.getBcp47() == "de-DE" );
        CPPUNIT_ASSERT( aLocale.Language == "de" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant == "" );
        CPPUNIT_ASSERT( aTag.getLanguageType() == LANGUAGE_GERMAN );
    }

    {
        OUString s_uab( "unreg-and-bad" );
        LanguageTag uab( s_uab, true );
        lang::Locale aLocale = uab.getLocale();
        CPPUNIT_ASSERT( uab.getBcp47() == s_uab );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "" );
        CPPUNIT_ASSERT( aLocale.Variant == s_uab );
        CPPUNIT_ASSERT( uab.getLanguageType() == LANGUAGE_SYSTEM );
        CPPUNIT_ASSERT( uab.isValidBcp47() == false );
        CPPUNIT_ASSERT( uab.isIsoLocale() == false );
        CPPUNIT_ASSERT( uab.isIsoODF() == false );
    }
}

void TestLanguageTag::testAllIsoLangEntries()
{
    const ::std::vector< MsLangId::LanguagetagMapping > aList( MsLangId::getDefinedLanguagetags());
    for (::std::vector< MsLangId::LanguagetagMapping >::const_iterator it( aList.begin()); it != aList.end(); ++it)
    {
        LanguageTag aTagString( (*it).maBcp47, true);
        LanguageTag aTagID( (*it).mnLang);
        if ((*it).maBcp47 != aTagString.getBcp47())
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
                aMessage += " " + OString::number( aTagString.getLanguageType(), 16) +
                    " -> " + OString::number( aTagBack.getLanguageType(), 16);
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

}

CPPUNIT_TEST_SUITE_REGISTRATION( TestLanguageTag );

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
