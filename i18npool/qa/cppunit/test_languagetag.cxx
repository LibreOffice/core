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

#include <i18npool/languagetag.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>

#include <com/sun/star/lang/Locale.hpp>

using namespace com::sun::star;

// To test the replacement code add '&& 0' and also in
// source/languagetag/languagetag.cxx
#define USE_LIBLANGTAG (!defined(ANDROID) && !defined(IOS))

namespace {

class TestLanguageTag : public CppUnit::TestFixture
{
public:
    TestLanguageTag() {}
    virtual ~TestLanguageTag() {}

    void testInit();
    void testAllTags();

    CPPUNIT_TEST_SUITE(TestLanguageTag);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testAllTags);
    CPPUNIT_TEST_SUITE_END();
};

void TestLanguageTag::testInit()
{
#if USE_LIBLANGTAG && !defined(SYSTEM_LIBLANGTAG)
    // file:///$SRCDIR/solver/$INPATH/share/liblangtag
    // file:///$OUTDIR/share/liblangtag
    OUStringBuffer aBuf(128);
    const char* pEnv = getenv("OUTDIR");
    CPPUNIT_ASSERT_MESSAGE("No $OUTDIR", pEnv);
    aBuf.append( "file:///").append( OStringToOUString( pEnv, RTL_TEXTENCODING_UTF8)).append( "/share/liblangtag");
    OUString aPath( aBuf.makeStringAndClear());
    OUString aData( aPath);
    aData += "/language-subtag-registry.xml";
    osl::DirectoryItem aDirItem;
    CPPUNIT_ASSERT_MESSAGE("liblangtag data not found",
            osl::DirectoryItem::get( aData, aDirItem) == osl::DirectoryItem::E_None);
    LanguageTag::overrideDataPath( aPath);
#endif
}

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
#else
        // The simple replacement code doesn't do any fancy stuff.
        CPPUNIT_ASSERT_MESSAGE("Default script was stripped after canonicalize!?!", aBcp47 == s_de_Latn_DE );
        CPPUNIT_ASSERT( aLocale.Language == "qlt" );
        CPPUNIT_ASSERT( aLocale.Country == "DE" );
        CPPUNIT_ASSERT( aLocale.Variant == "de-Latn-DE" );
        (void)nLanguageType; //XXX CPPUNIT_ASSERT( nLanguageType == LANGUAGE_GERMAN );
#endif
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

CPPUNIT_TEST_SUITE_REGISTRATION( TestLanguageTag );

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
