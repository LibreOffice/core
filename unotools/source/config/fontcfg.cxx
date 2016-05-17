/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <i18nlangtag/mslangid.hxx>
#include <unotools/fontcfg.hxx>
#include <unotools/fontdefs.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <unotools/configpaths.hxx>
#include <unotools/syslocale.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <osl/diagnose.h>
#include <sal/macros.h>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <string.h>
#include <list>
#include <algorithm>

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::configuration;

/*
 * DefaultFontConfiguration
 */

static const char* getKeyType( DefaultFontType nKeyType )
{
    switch( nKeyType )
    {
    case DefaultFontType::CJK_DISPLAY: return "CJK_DISPLAY";
    case DefaultFontType::CJK_HEADING: return "CJK_HEADING";
    case DefaultFontType::CJK_PRESENTATION: return "CJK_PRESENTATION";
    case DefaultFontType::CJK_SPREADSHEET: return "CJK_SPREADSHEET";
    case DefaultFontType::CJK_TEXT: return "CJK_TEXT";
    case DefaultFontType::CTL_DISPLAY: return "CTL_DISPLAY";
    case DefaultFontType::CTL_HEADING: return "CTL_HEADING";
    case DefaultFontType::CTL_PRESENTATION: return "CTL_PRESENTATION";
    case DefaultFontType::CTL_SPREADSHEET: return "CTL_SPREADSHEET";
    case DefaultFontType::CTL_TEXT: return "CTL_TEXT";
    case DefaultFontType::FIXED: return "FIXED";
    case DefaultFontType::LATIN_DISPLAY: return "LATIN_DISPLAY";
    case DefaultFontType::LATIN_FIXED: return "LATIN_FIXED";
    case DefaultFontType::LATIN_HEADING: return "LATIN_HEADING";
    case DefaultFontType::LATIN_PRESENTATION: return "LATIN_PRESENTATION";
    case DefaultFontType::LATIN_SPREADSHEET: return "LATIN_SPREADSHEET";
    case DefaultFontType::LATIN_TEXT: return "LATIN_TEXT";
    case DefaultFontType::SANS: return "SANS";
    case DefaultFontType::SANS_UNICODE: return "SANS_UNICODE";
    case DefaultFontType::SERIF: return "SERIF";
    case DefaultFontType::SYMBOL: return "SYMBOL";
    case DefaultFontType::UI_FIXED: return "UI_FIXED";
    case DefaultFontType::UI_SANS: return "UI_SANS";
    default:
        OSL_FAIL( "unmatched type" );
        return "";
    }
}

namespace
{
    class theDefaultFontConfiguration
        : public rtl::Static<DefaultFontConfiguration,
            theDefaultFontConfiguration>
    {
    };
}

DefaultFontConfiguration& DefaultFontConfiguration::get()
{
    return theDefaultFontConfiguration::get();
}

DefaultFontConfiguration::DefaultFontConfiguration()
{
    try
    {
        // get service provider
        Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
        // create configuration hierarchical access name
        try
        {
            m_xConfigProvider = theDefaultProvider::get( xContext );
            Sequence< Any > aArgs(1);
            PropertyValue aVal;
            aVal.Name = "nodepath";
            aVal.Value <<= OUString( "/org.openoffice.VCL/DefaultFonts" );
            aArgs.getArray()[0] <<= aVal;
            m_xConfigAccess =
                Reference< XNameAccess >(
                    m_xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess",
                                                                    aArgs ),
                    UNO_QUERY );
            if( m_xConfigAccess.is() )
            {
                Sequence< OUString > aLocales = m_xConfigAccess->getElementNames();
                // fill config hash with empty interfaces
                int nLocales = aLocales.getLength();
                const OUString* pLocaleStrings = aLocales.getConstArray();
                for( int i = 0; i < nLocales; i++ )
                {
                    // Feed through LanguageTag for casing.
                    OUString aLoc( LanguageTag( pLocaleStrings[i], true).getBcp47( false));
                    m_aConfig[ aLoc ] = LocaleAccess();
                    m_aConfig[ aLoc ].aConfigLocaleString = pLocaleStrings[i];
                }
            }
        }
        catch (const Exception&)
        {
            // configuration is awry
            m_xConfigProvider.clear();
            m_xConfigAccess.clear();
        }
    }
    catch (const WrappedTargetException&)
    {
    }
    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "config provider: %s, config access: %s\n",
             m_xConfigProvider.is() ? "true" : "false",
             m_xConfigAccess.is() ? "true" : "false"
             );
    #endif
}

DefaultFontConfiguration::~DefaultFontConfiguration()
{
    // release all nodes
    m_aConfig.clear();
    // release top node
    m_xConfigAccess.clear();
    // release config provider
    m_xConfigProvider.clear();
}

OUString DefaultFontConfiguration::tryLocale( const OUString& rBcp47, const OUString& rType ) const
{
    OUString aRet;

    std::unordered_map< OUString, LocaleAccess, OUStringHash >::const_iterator it = m_aConfig.find( rBcp47 );
    if( it != m_aConfig.end() )
    {
        if( !it->second.xAccess.is() )
        {
            try
            {
                Reference< XNameAccess > xNode;
                if ( m_xConfigAccess->hasByName( it->second.aConfigLocaleString ) )
                {
                    Any aAny = m_xConfigAccess->getByName( it->second.aConfigLocaleString );
                    if( aAny >>= xNode )
                        it->second.xAccess = xNode;
                }
            }
            catch (const NoSuchElementException&)
            {
            }
            catch (const WrappedTargetException&)
            {
            }
        }
        if( it->second.xAccess.is() )
        {
            try
            {
                if ( it->second.xAccess->hasByName( rType ) )
                {
                    Any aAny = it->second.xAccess->getByName( rType );
                    aAny >>= aRet;
                }
            }
            catch (const NoSuchElementException&)
            {
            }
            catch (const WrappedTargetException&)
            {
            }
        }
    }

    return aRet;
}

OUString DefaultFontConfiguration::getDefaultFont( const LanguageTag& rLanguageTag, DefaultFontType nType ) const
{
    OUString aType = OUString::createFromAscii( getKeyType( nType ) );
    // Try the simple cases first without constructing fallbacks.
    OUString aRet = tryLocale( rLanguageTag.getBcp47(), aType );
    if (aRet.isEmpty())
    {
        if (rLanguageTag.isIsoLocale())
        {
            if (!rLanguageTag.getCountry().isEmpty())
            {
                aRet = tryLocale( rLanguageTag.getLanguage(), aType );
            }
        }
        else
        {
            ::std::vector< OUString > aFallbacks( rLanguageTag.getFallbackStrings( false));
            for (::std::vector< OUString >::const_iterator it( aFallbacks.begin());
                    it != aFallbacks.end() && aRet.isEmpty(); ++it)
            {
                aRet = tryLocale( *it, aType );
            }
        }
    }
    if( aRet.isEmpty() )
    {
        aRet = tryLocale( "en", aType );
    }
    return aRet;
}

OUString DefaultFontConfiguration::getUserInterfaceFont( const LanguageTag& rLanguageTag ) const
{
    LanguageTag aLanguageTag( rLanguageTag);
    if( aLanguageTag.isSystemLocale() )
        aLanguageTag = SvtSysLocale().GetUILanguageTag();

    OUString aUIFont = getDefaultFont( aLanguageTag, DefaultFontType::UI_SANS );

    if( !aUIFont.isEmpty() )
        return aUIFont;

    // fallback mechanism (either no configuration or no entry in configuration

    #define FALLBACKFONT_UI_SANS "Andale Sans UI;Albany;Albany AMT;Tahoma;Arial Unicode MS;Arial;Nimbus Sans L;Bitstream Vera Sans;gnu-unifont;Interface User;Geneva;WarpSans;Dialog;Swiss;Lucida;Helvetica;Charcoal;Chicago;MS Sans Serif;Helv;Times;Times New Roman;Interface System"
    #define FALLBACKFONT_UI_SANS_LATIN2 "Andale Sans UI;Albany;Albany AMT;Tahoma;Arial Unicode MS;Arial;Nimbus Sans L;Luxi Sans;Bitstream Vera Sans;Interface User;Geneva;WarpSans;Dialog;Swiss;Lucida;Helvetica;Charcoal;Chicago;MS Sans Serif;Helv;Times;Times New Roman;Interface System"
    #define FALLBACKFONT_UI_SANS_ARABIC "Tahoma;Traditional Arabic;Simplified Arabic;Lucidasans;Lucida Sans;Supplement;Andale Sans UI;clearlyU;Interface User;Arial Unicode MS;Lucida Sans Unicode;WarpSans;Geneva;MS Sans Serif;Helv;Dialog;Albany;Lucida;Helvetica;Charcoal;Chicago;Arial;Helmet;Interface System;Sans Serif"
    #define FALLBACKFONT_UI_SANS_THAI "OONaksit;Tahoma;Lucidasans;Arial Unicode MS"
    #define FALLBACKFONT_UI_SANS_KOREAN "SunGulim;BaekmukGulim;Gulim;Roundgothic;Arial Unicode MS;Lucida Sans Unicode;gnu-unifont;Andale Sans UI"
    #define FALLBACKFONT_UI_SANS_JAPANESE1 "HG-GothicB-Sun;Andale Sans UI;HG MhinchoLightJ"
    #define FALLBACKFONT_UI_SANS_JAPANESE2 "Kochi Gothic;Gothic"
    #define FALLBACKFONT_UI_SANS_CHINSIM "Andale Sans UI;Arial Unicode MS;ZYSong18030;AR PL SungtiL GB;AR PL KaitiM GB;SimSun;Lucida Sans Unicode;Fangsong;Hei;Song;Kai;Ming;gnu-unifont;Interface User;"
    #define FALLBACKFONT_UI_SANS_CHINTRD "Andale Sans UI;Arial Unicode MS;AR PL Mingti2L Big5;AR PL KaitiM Big5;Kai;PMingLiU;MingLiU;Ming;Lucida Sans Unicode;gnu-unifont;Interface User;"

    const OUString aLanguage( aLanguageTag.getLanguage());

    // optimize font list for some locales, as long as Andale Sans UI does not support them
    if( aLanguage == "ar" || aLanguage == "he" || aLanguage == "iw"  )
    {
        return OUString(FALLBACKFONT_UI_SANS_ARABIC);
    }
    else if ( aLanguage == "th" )
    {
        return OUString(FALLBACKFONT_UI_SANS_THAI);
    }
    else if ( aLanguage == "ko" )
    {
        // we need localized names for korean fonts
        const sal_Unicode aSunGulim[] = { 0xC36C, 0xAD74, 0xB9BC, 0 };
        const sal_Unicode aBaekmukGulim[] = { 0xBC31, 0xBC35, 0xAD74, 0xB9BC, 0 };

        OUStringBuffer aFallBackKoreanLocalized;
        aFallBackKoreanLocalized.append(aSunGulim);
        aFallBackKoreanLocalized.append(';');
        aFallBackKoreanLocalized.append(aBaekmukGulim);
        aFallBackKoreanLocalized.append(";");
        aFallBackKoreanLocalized.append(FALLBACKFONT_UI_SANS_KOREAN);

        return aFallBackKoreanLocalized.makeStringAndClear();
    }
    else if( aLanguage == "cs" ||
             aLanguage == "hu" ||
             aLanguage == "pl" ||
             aLanguage == "ro" ||
             aLanguage == "rm" ||
             aLanguage == "hr" ||
             aLanguage == "sk" ||
             aLanguage == "sl" ||
             aLanguage == "sb")
    {
        return OUString(FALLBACKFONT_UI_SANS_LATIN2);
    }
    else if ( aLanguage == "ja" )
    {
        // we need localized names for japanese fonts
        const sal_Unicode aMSGothic[] = { 0xFF2D, 0xFF33, ' ', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
        const sal_Unicode aMSPGothic[] = { 0xFF2D, 0xFF33, ' ', 0xFF30, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
        const sal_Unicode aTLPGothic[] = { 0x0054, 0x004C, 0x0050, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
        const sal_Unicode aLXGothic[] = { 0x004C, 0x0058, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
        const sal_Unicode aKochiGothic[] = { 0x6771, 0x98A8, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };

        OUStringBuffer aFallBackJapaneseLocalized;
        aFallBackJapaneseLocalized.append("MS UI Gothic;");
        aFallBackJapaneseLocalized.append(FALLBACKFONT_UI_SANS_JAPANESE1);
        aFallBackJapaneseLocalized.append(aMSPGothic);
        aFallBackJapaneseLocalized.append(';');
        aFallBackJapaneseLocalized.append(aMSGothic);
        aFallBackJapaneseLocalized.append(';');
        aFallBackJapaneseLocalized.append(aTLPGothic);
        aFallBackJapaneseLocalized.append(';');
        aFallBackJapaneseLocalized.append(aLXGothic);
        aFallBackJapaneseLocalized.append(';');
        aFallBackJapaneseLocalized.append(aKochiGothic);
        aFallBackJapaneseLocalized.append(';');
        aFallBackJapaneseLocalized.append(FALLBACKFONT_UI_SANS_JAPANESE2);

        return aFallBackJapaneseLocalized.makeStringAndClear();
    }
    else
    {
        Locale aLocale( aLanguageTag.getLocale());
        if (MsLangId::isTraditionalChinese(aLocale))
            return OUString(FALLBACKFONT_UI_SANS_CHINTRD);
        else if (MsLangId::isSimplifiedChinese(aLocale))
            return OUString(FALLBACKFONT_UI_SANS_CHINSIM);
    }

    return OUString(FALLBACKFONT_UI_SANS);
}

/*
 *  FontSubstConfigItem::get
 */

namespace
{
    class theFontSubstConfiguration
        : public rtl::Static<FontSubstConfiguration, theFontSubstConfiguration>
    {
    };
}

FontSubstConfiguration& FontSubstConfiguration::get()
{
    return theFontSubstConfiguration::get();
}

/*
 *  FontSubstConfigItem::FontSubstConfigItem
 */

FontSubstConfiguration::FontSubstConfiguration() :
    maSubstHash( 300 )
{
    try
    {
        // get service provider
        Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
        // create configuration hierarchical access name
        m_xConfigProvider = theDefaultProvider::get( xContext );
        Sequence< Any > aArgs(1);
        PropertyValue aVal;
        aVal.Name = "nodepath";
        aVal.Value <<= OUString( "/org.openoffice.VCL/FontSubstitutions" );
        aArgs.getArray()[0] <<= aVal;
        m_xConfigAccess =
            Reference< XNameAccess >(
                m_xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess",
                                                                aArgs ),
                UNO_QUERY );
        if( m_xConfigAccess.is() )
        {
            Sequence< OUString > aLocales = m_xConfigAccess->getElementNames();
            // fill config hash with empty interfaces
            int nLocales = aLocales.getLength();
            const OUString* pLocaleStrings = aLocales.getConstArray();
            for( int i = 0; i < nLocales; i++ )
            {
                // Feed through LanguageTag for casing.
                OUString aLoc( LanguageTag( pLocaleStrings[i], true).getBcp47( false));
                m_aSubst[ aLoc ] = LocaleSubst();
                m_aSubst[ aLoc ].aConfigLocaleString = pLocaleStrings[i];
            }
        }
    }
    catch (const Exception&)
    {
        // configuration is awry
        m_xConfigProvider.clear();
        m_xConfigAccess.clear();
    }
    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "config provider: %s, config access: %s\n",
             m_xConfigProvider.is() ? "true" : "false",
             m_xConfigAccess.is() ? "true" : "false"
             );
    #endif
}

/*
 *  FontSubstConfigItem::~FontSubstConfigItem
 */

FontSubstConfiguration::~FontSubstConfiguration()
{
    // release config access
    m_xConfigAccess.clear();
    // release config provider
    m_xConfigProvider.clear();
}

/*
 *  FontSubstConfigItem::getMapName
 */

static const char* const aImplKillLeadingList[] =
{
    "microsoft",
    "monotype",
    "linotype",
    "baekmuk",
    "adobe",
    "nimbus",
    "zycjk",
    "itc",
    "sun",
    "amt",
    "ms",
    "mt",
    "cg",
    "hg",
    "fz",
    "ipa",
    "sazanami",
    "kochi",
    nullptr
};

static const char* const aImplKillTrailingList[] =
{
    "microsoft",
    "monotype",
    "linotype",
    "adobe",
    "nimbus",
    "itc",
    "sun",
    "amt",
    "ms",
    "mt",
    "clm",
    // Scripts, for compatibility with older versions
    "we",
    "cyr",
    "tur",
    "wt",
    "greek",
    "wl",
    // CJK extensions
    "gb",
    "big5",
    "pro",
    "z01",
    "z02",
    "z03",
    "z13",
    "b01",
    "w3x12",
    // Old Printer Fontnames
    "5cpi",
    "6cpi",
    "7cpi",
    "8cpi",
    "9cpi",
    "10cpi",
    "11cpi",
    "12cpi",
    "13cpi",
    "14cpi",
    "15cpi",
    "16cpi",
    "18cpi",
    "24cpi",
    "scale",
    "pc",
    nullptr
};

static const char* const aImplKillTrailingWithExceptionsList[] =
{
    "ce", "monospace", "oldface", nullptr,
    "ps", "caps", nullptr,
    nullptr
};

struct ImplFontAttrWeightSearchData
{
    const char*             mpStr;
    FontWeight              meWeight;
};

static ImplFontAttrWeightSearchData const aImplWeightAttrSearchList[] =
{
// the attribute names are ordered by "first match wins"
// e.g. "semilight" should wins over "semi"
{   "extrablack",           WEIGHT_BLACK },
{   "ultrablack",           WEIGHT_BLACK },
{   "ultrabold",            WEIGHT_ULTRABOLD },
{   "semibold",             WEIGHT_SEMIBOLD },
{   "semilight",            WEIGHT_SEMILIGHT },
{   "semi",                 WEIGHT_SEMIBOLD },
{   "demi",                 WEIGHT_SEMIBOLD },
{   "black",                WEIGHT_BLACK },
{   "bold",                 WEIGHT_BOLD },
{   "heavy",                WEIGHT_BLACK },
{   "ultralight",           WEIGHT_ULTRALIGHT },
{   "light",                WEIGHT_LIGHT },
{   "medium",               WEIGHT_MEDIUM },
{   nullptr,                   WEIGHT_DONTKNOW },
};

struct ImplFontAttrWidthSearchData
{
    const char*             mpStr;
    FontWidth               meWidth;
};

static ImplFontAttrWidthSearchData const aImplWidthAttrSearchList[] =
{
{   "narrow",               WIDTH_CONDENSED },
{   "semicondensed",        WIDTH_SEMI_CONDENSED },
{   "ultracondensed",       WIDTH_ULTRA_CONDENSED },
{   "semiexpanded",         WIDTH_SEMI_EXPANDED },
{   "ultraexpanded",        WIDTH_ULTRA_EXPANDED },
{   "expanded",             WIDTH_EXPANDED },
{   "wide",                 WIDTH_ULTRA_EXPANDED },
{   "condensed",            WIDTH_CONDENSED },
{   "cond",                 WIDTH_CONDENSED },
{   "cn",                   WIDTH_CONDENSED },
{   nullptr,                   WIDTH_DONTKNOW },
};

struct ImplFontAttrTypeSearchData
{
    const char*             mpStr;
    ImplFontAttrs           mnType;
};

static ImplFontAttrTypeSearchData const aImplTypeAttrSearchList[] =
{
{   "monotype",             ImplFontAttrs::None },
{   "linotype",             ImplFontAttrs::None },
{   "titling",              ImplFontAttrs::Titling },
{   "captitals",            ImplFontAttrs::Capitals },
{   "captital",             ImplFontAttrs::Capitals },
{   "caps",                 ImplFontAttrs::Capitals },
{   "italic",               ImplFontAttrs::Italic },
{   "oblique",              ImplFontAttrs::Italic },
{   "rounded",              ImplFontAttrs::Rounded },
{   "outline",              ImplFontAttrs::Outline },
{   "shadow",               ImplFontAttrs::Shadow },
{   "handwriting",          ImplFontAttrs::Handwriting | ImplFontAttrs::Script },
{   "hand",                 ImplFontAttrs::Handwriting | ImplFontAttrs::Script },
{   "signet",               ImplFontAttrs::Handwriting | ImplFontAttrs::Script },
{   "script",               ImplFontAttrs::BrushScript | ImplFontAttrs::Script },
{   "calligraphy",          ImplFontAttrs::Chancery | ImplFontAttrs::Script },
{   "chancery",             ImplFontAttrs::Chancery | ImplFontAttrs::Script },
{   "corsiva",              ImplFontAttrs::Chancery | ImplFontAttrs::Script },
{   "gothic",               ImplFontAttrs::SansSerif | ImplFontAttrs::Gothic },
{   "schoolbook",           ImplFontAttrs::Serif | ImplFontAttrs::Schoolbook },
{   "schlbk",               ImplFontAttrs::Serif | ImplFontAttrs::Schoolbook },
{   "typewriter",           ImplFontAttrs::Typewriter | ImplFontAttrs::Fixed },
{   "lineprinter",          ImplFontAttrs::Typewriter | ImplFontAttrs::Fixed },
{   "monospaced",           ImplFontAttrs::Fixed },
{   "monospace",            ImplFontAttrs::Fixed },
{   "mono",                 ImplFontAttrs::Fixed },
{   "fixed",                ImplFontAttrs::Fixed },
{   "sansserif",            ImplFontAttrs::SansSerif },
{   "sans",                 ImplFontAttrs::SansSerif },
{   "swiss",                ImplFontAttrs::SansSerif },
{   "serif",                ImplFontAttrs::Serif },
{   "bright",               ImplFontAttrs::Serif },
{   "symbols",              ImplFontAttrs::Symbol },
{   "symbol",               ImplFontAttrs::Symbol },
{   "dingbats",             ImplFontAttrs::Symbol },
{   "dings",                ImplFontAttrs::Symbol },
{   "ding",                 ImplFontAttrs::Symbol },
{   "bats",                 ImplFontAttrs::Symbol },
{   "math",                 ImplFontAttrs::Symbol },
{   "oldstyle",             ImplFontAttrs::OtherStyle },
{   "oldface",              ImplFontAttrs::OtherStyle },
{   "old",                  ImplFontAttrs::OtherStyle },
{   "new",                  ImplFontAttrs::None },
{   "modern",               ImplFontAttrs::None },
{   "lucida",               ImplFontAttrs::None },
{   "regular",              ImplFontAttrs::None },
{   "extended",             ImplFontAttrs::None },
{   "extra",                ImplFontAttrs::OtherStyle },
{   "ext",                  ImplFontAttrs::None },
{   "scalable",             ImplFontAttrs::None },
{   "scale",                ImplFontAttrs::None },
{   "nimbus",               ImplFontAttrs::None },
{   "adobe",                ImplFontAttrs::None },
{   "itc",                  ImplFontAttrs::None },
{   "amt",                  ImplFontAttrs::None },
{   "mt",                   ImplFontAttrs::None },
{   "ms",                   ImplFontAttrs::None },
{   "cpi",                  ImplFontAttrs::None },
{   "no",                   ImplFontAttrs::None },
{   nullptr,                   ImplFontAttrs::None },
};

static bool ImplKillLeading( OUString& rName, const char* const* ppStr )
{
    for(; *ppStr; ++ppStr )
    {
        const char*         pStr = *ppStr;
        const sal_Unicode*  pNameStr = rName.getStr();
        while ( (*pNameStr == (sal_Unicode)(unsigned char)*pStr) && *pStr )
        {
            pNameStr++;
            pStr++;
        }
        if ( !*pStr )
        {
            sal_Int32 nLen = (sal_Int32)(pNameStr - rName.getStr());
            rName = rName.copy(nLen);
            return true;
        }
    }

    // special case for Baekmuk
    // TODO: allow non-ASCII KillLeading list
    const sal_Unicode* pNameStr = rName.getStr();
    if( (pNameStr[0]==0xBC31) && (pNameStr[1]==0xBC35) )
    {
        sal_Int32 nLen = (pNameStr[2]==0x0020) ? 3 : 2;
        rName = rName.copy(nLen);
        return true;
    }

    return false;
}

static sal_Int32 ImplIsTrailing( const OUString& rName, const char* pStr )
{
    sal_Int32 nStrLen = (sal_Int32)strlen( pStr );
    if( nStrLen >= rName.getLength() )
        return 0;

    const sal_Unicode* pEndName = rName.getStr() + rName.getLength();
    const sal_Unicode* pNameStr = pEndName - nStrLen;
    do if( *(pNameStr++) != *(pStr++) )
        return 0;
    while( *pStr );

    return nStrLen;
}

static bool ImplKillTrailing( OUString& rName, const char* const* ppStr )
{
    for(; *ppStr; ++ppStr )
    {
        sal_Int32 nTrailLen = ImplIsTrailing( rName, *ppStr );
        if( nTrailLen )
        {
            rName = rName.copy(0, rName.getLength() - nTrailLen );
            return true;
        }
    }

    return false;
}

static bool ImplKillTrailingWithExceptions( OUString& rName, const char* const* ppStr )
{
    for(; *ppStr; ++ppStr )
    {
        sal_Int32 nTrailLen = ImplIsTrailing( rName, *ppStr );
        if( nTrailLen )
        {
            // check string match against string exceptions
            while( *++ppStr )
                if( ImplIsTrailing( rName, *ppStr ) )
                    return false;

            rName = rName.copy(0, rName.getLength() - nTrailLen );
            return true;
        }
        else
        {
            // skip exception strings
            while( *++ppStr ) {}
        }
    }

    return false;
}

static bool ImplFindAndErase( OUString& rName, const char* pStr )
{
    sal_Int32 nLen = (sal_Int32)strlen(pStr);
    sal_Int32 nPos = rName.indexOfAsciiL(pStr, nLen );
    if ( nPos < 0 )
        return false;

    OUStringBuffer sBuff(rName);
    sBuff.remove(nPos, nLen);
    rName = sBuff.makeStringAndClear();
    return true;
}

void FontSubstConfiguration::getMapName( const OUString& rOrgName, OUString& rShortName,
                                         OUString& rFamilyName, FontWeight& rWeight,
                                         FontWidth& rWidth, ImplFontAttrs& rType )
{
    rShortName = rOrgName;

    // TODO: get rid of the crazy O(N*strlen) searches below
    // they should be possible in O(strlen)

    // Kill leading vendor names and other unimportant data
    ImplKillLeading( rShortName, aImplKillLeadingList );

    // Kill trailing vendor names and other unimportant data
    ImplKillTrailing( rShortName, aImplKillTrailingList );
    ImplKillTrailingWithExceptions( rShortName, aImplKillTrailingWithExceptionsList );

    rFamilyName = rShortName;

    // Kill attributes from the name and update the data
    // Weight
    const ImplFontAttrWeightSearchData* pWeightList = aImplWeightAttrSearchList;
    while ( pWeightList->mpStr )
    {
        if ( ImplFindAndErase( rFamilyName, pWeightList->mpStr ) )
        {
            if ( (rWeight == WEIGHT_DONTKNOW) || (rWeight == WEIGHT_NORMAL) )
                rWeight = pWeightList->meWeight;
            break;
        }
        pWeightList++;
    }

    // Width
    const ImplFontAttrWidthSearchData* pWidthList = aImplWidthAttrSearchList;
    while ( pWidthList->mpStr )
    {
        if ( ImplFindAndErase( rFamilyName, pWidthList->mpStr ) )
        {
            if ( (rWidth == WIDTH_DONTKNOW) || (rWidth == WIDTH_NORMAL) )
                rWidth = pWidthList->meWidth;
            break;
        }
        pWidthList++;
    }

    // Type
    rType = ImplFontAttrs::None;
    const ImplFontAttrTypeSearchData* pTypeList = aImplTypeAttrSearchList;
    while ( pTypeList->mpStr )
    {
        if ( ImplFindAndErase( rFamilyName, pTypeList->mpStr ) )
            rType |= pTypeList->mnType;
        pTypeList++;
    }

    // Remove numbers
    // TODO: also remove localized and fullwidth digits
    sal_Int32 i = 0;
    OUStringBuffer sBuff(rFamilyName);
    while ( i < sBuff.getLength() )
    {
        sal_Unicode c = sBuff[ i ];
        if ( (c >= 0x0030) && (c <= 0x0039) )
            sBuff.remove(i, 1);
        else
            i++;
    }
}

struct StrictStringSort : public ::std::binary_function< const FontNameAttr&, const FontNameAttr&, bool >
{
    bool operator()( const FontNameAttr& rLeft, const FontNameAttr& rRight )
    { return rLeft.Name.compareTo( rRight.Name ) < 0; }
};

// The entries in this table must match the bits in the ImplFontAttrs enum.

static const char* const pAttribNames[] =
{
    "default",
    "standard",
    "normal",
    "symbol",
    "fixed",
    "sansserif",
    "serif",
    "decorative",
    "special",
    "italic",
    "title",
    "capitals",
    "cjk",
    "cjk_jp",
    "cjk_sc",
    "cjk_tc",
    "cjk_kr",
    "ctl",
    "nonelatin",
    "full",
    "outline",
    "shadow",
    "rounded",
    "typewriter",
    "script",
    "handwriting",
    "chancery",
    "comic",
    "brushscript",
    "gothic",
    "schoolbook",
    "other"
};

struct enum_convert
{
    const char* pName;
    int          nEnum;
};

static const enum_convert pWeightNames[] =
{
    { "normal", WEIGHT_NORMAL },
    { "medium", WEIGHT_MEDIUM },
    { "bold", WEIGHT_BOLD },
    { "black", WEIGHT_BLACK },
    { "semibold", WEIGHT_SEMIBOLD },
    { "light", WEIGHT_LIGHT },
    { "semilight", WEIGHT_SEMILIGHT },
    { "ultrabold", WEIGHT_ULTRABOLD },
    { "semi", WEIGHT_SEMIBOLD },
    { "demi", WEIGHT_SEMIBOLD },
    { "heavy", WEIGHT_BLACK },
    { "unknown", WEIGHT_DONTKNOW },
    { "thin", WEIGHT_THIN },
    { "ultralight", WEIGHT_ULTRALIGHT }
};

static const enum_convert pWidthNames[] =
{
    { "normal", WIDTH_NORMAL },
    { "condensed", WIDTH_CONDENSED },
    { "expanded", WIDTH_EXPANDED },
    { "unknown", WIDTH_DONTKNOW },
    { "ultracondensed", WIDTH_ULTRA_CONDENSED },
    { "extracondensed", WIDTH_EXTRA_CONDENSED },
    { "semicondensed", WIDTH_SEMI_CONDENSED },
    { "semiexpanded", WIDTH_SEMI_EXPANDED },
    { "extraexpanded", WIDTH_EXTRA_EXPANDED },
    { "ultraexpanded", WIDTH_ULTRA_EXPANDED }
};

void FontSubstConfiguration::fillSubstVector( const css::uno::Reference< XNameAccess >& rFont,
                                              const OUString& rType,
                                              std::vector< OUString >& rSubstVector ) const
{
    try
    {
        Any aAny = rFont->getByName( rType );
        if( aAny.getValueTypeClass() == TypeClass_STRING )
        {
            const OUString* pLine = static_cast<const OUString*>(aAny.getValue());
            sal_Int32 nLength = pLine->getLength();
            if( nLength )
            {
                const sal_Unicode* pStr = pLine->getStr();
                sal_Int32 nTokens = 0;
                // count tokens
                while( nLength-- )
                {
                    if( *pStr++ == ';' )
                        nTokens++;
                }
                rSubstVector.clear();
                // optimize performance, heap fragmentation
                rSubstVector.reserve( nTokens );
                sal_Int32 nIndex = 0;
                while( nIndex != -1 )
                {
                    OUString aSubst( pLine->getToken( 0, ';', nIndex ) );
                    if( !aSubst.isEmpty() )
                    {
                        UniqueSubstHash::iterator aEntry = maSubstHash.find( aSubst );
                        if (aEntry != maSubstHash.end())
                            aSubst = *aEntry;
                        else
                            maSubstHash.insert( aSubst );
                        rSubstVector.push_back( aSubst );
                    }
                }
            }
        }
    }
    catch (const NoSuchElementException&)
    {
    }
    catch (const WrappedTargetException&)
    {
    }
}

FontWeight FontSubstConfiguration::getSubstWeight( const css::uno::Reference< XNameAccess >& rFont,
                                                   const OUString& rType ) const
{
    int weight = -1;
    try
    {
        Any aAny = rFont->getByName( rType );
        if( aAny.getValueTypeClass() == TypeClass_STRING )
        {
            const OUString* pLine = static_cast<const OUString*>(aAny.getValue());
            if( !pLine->isEmpty() )
            {
                for( weight=SAL_N_ELEMENTS(pWeightNames)-1; weight >= 0; weight-- )
                    if( pLine->equalsIgnoreAsciiCaseAscii( pWeightNames[weight].pName ) )
                        break;
            }
#if OSL_DEBUG_LEVEL > 1
            if( weight < 0 )
                fprintf( stderr, "Error: invalid weight %s\n",
                         OUStringToOString( *pLine, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
        }
    }
    catch (const NoSuchElementException&)
    {
    }
    catch (const WrappedTargetException&)
    {
    }
    return (FontWeight)( weight >= 0 ? pWeightNames[weight].nEnum : WEIGHT_DONTKNOW );
}

FontWidth FontSubstConfiguration::getSubstWidth( const css::uno::Reference< XNameAccess >& rFont,
                                                 const OUString& rType ) const
{
    int width = -1;
    try
    {
        Any aAny = rFont->getByName( rType );
        if( aAny.getValueTypeClass() == TypeClass_STRING )
        {
            const OUString* pLine = static_cast<const OUString*>(aAny.getValue());
            if( !pLine->isEmpty() )
            {
                for( width=SAL_N_ELEMENTS(pWidthNames)-1; width >= 0; width-- )
                    if( pLine->equalsIgnoreAsciiCaseAscii( pWidthNames[width].pName ) )
                        break;
            }
#if OSL_DEBUG_LEVEL > 1
            if( width < 0 )
                fprintf( stderr, "Error: invalid width %s\n",
                         OUStringToOString( *pLine, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
        }
    }
    catch (const NoSuchElementException&)
    {
    }
    catch (const WrappedTargetException&)
    {
    }
    return (FontWidth)( width >= 0 ? pWidthNames[width].nEnum : WIDTH_DONTKNOW );
}

ImplFontAttrs FontSubstConfiguration::getSubstType( const css::uno::Reference< XNameAccess >& rFont,
                                                    const OUString& rType ) const
{
    sal_uLong type = 0;
    try
    {
        Any aAny = rFont->getByName( rType );
        if( aAny.getValueTypeClass() != TypeClass_STRING )
            return ImplFontAttrs::None;
        const OUString* pLine = static_cast<const OUString*>(aAny.getValue());
        if( pLine->isEmpty() )
            return ImplFontAttrs::None;
        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            OUString aToken( pLine->getToken( 0, ',', nIndex ) );
            for( int k = 0; k < 32; k++ )
                if( aToken.equalsIgnoreAsciiCaseAscii( pAttribNames[k] ) )
                {
                    type |= sal_uLong(1) << k;
                    break;
                }
        }
        assert(((type & ~o3tl::typed_flags<ImplFontAttrs>::mask) == 0) && "invalid font attributes");
    }
    catch (const NoSuchElementException&)
    {
    }
    catch (const WrappedTargetException&)
    {
    }

    return static_cast<ImplFontAttrs>(type);
}

void FontSubstConfiguration::readLocaleSubst( const OUString& rBcp47 ) const
{
    std::unordered_map< OUString, LocaleSubst, OUStringHash >::const_iterator it = m_aSubst.find( rBcp47 );
    if( it != m_aSubst.end() )
    {
        if( ! it->second.bConfigRead )
        {
            it->second.bConfigRead = true;
            Reference< XNameAccess > xNode;
            try
            {
                Any aAny = m_xConfigAccess->getByName( it->second.aConfigLocaleString );
                aAny >>= xNode;
            }
            catch (const NoSuchElementException&)
            {
            }
            catch (const WrappedTargetException&)
            {
            }
            if( xNode.is() )
            {
                Sequence< OUString > aFonts = xNode->getElementNames();
                int nFonts = aFonts.getLength();
                const OUString* pFontNames = aFonts.getConstArray();
                // improve performance, heap fragmentation
                it->second.aSubstAttributes.reserve( nFonts );

                // strings for subst retrieval, construct only once
                OUString aSubstFontsStr     ( "SubstFonts" );
                OUString aSubstFontsMSStr   ( "SubstFontsMS" );
                OUString aSubstFontsPSStr   ( "SubstFontsPS" );
                OUString aSubstFontsHTMLStr ( "SubstFontsHTML" );
                OUString aSubstWeightStr    ( "FontWeight" );
                OUString aSubstWidthStr     ( "FontWidth" );
                OUString aSubstTypeStr      ( "FontType" );
                for( int i = 0; i < nFonts; i++ )
                {
                    Reference< XNameAccess > xFont;
                    try
                    {
                        Any aAny = xNode->getByName( pFontNames[i] );
                        aAny >>= xFont;
                    }
                    catch (const NoSuchElementException&)
                    {
                    }
                    catch (const WrappedTargetException&)
                    {
                    }
                    if( ! xFont.is() )
                    {
                        #if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "did not get font attributes for %s\n",
                                 OUStringToOString( pFontNames[i], RTL_TEXTENCODING_UTF8 ).getStr() );
                        #endif
                        continue;
                    }

                    FontNameAttr aAttr;
                    // read subst attributes from config
                    aAttr.Name = pFontNames[i];
                    fillSubstVector( xFont, aSubstFontsStr, aAttr.Substitutions );
                    fillSubstVector( xFont, aSubstFontsMSStr, aAttr.MSSubstitutions );
                    fillSubstVector( xFont, aSubstFontsPSStr, aAttr.PSSubstitutions );
                    fillSubstVector( xFont, aSubstFontsHTMLStr, aAttr.HTMLSubstitutions );
                    aAttr.Weight = getSubstWeight( xFont, aSubstWeightStr );
                    aAttr.Width = getSubstWidth( xFont, aSubstWidthStr );
                    aAttr.Type = getSubstType( xFont, aSubstTypeStr );

                    // finally insert this entry
                    it->second.aSubstAttributes.push_back( aAttr );
                }
                std::sort( it->second.aSubstAttributes.begin(), it->second.aSubstAttributes.end(), StrictStringSort() );
            }
        }
    }
}

const FontNameAttr* FontSubstConfiguration::getSubstInfo( const OUString& rFontName,
        const LanguageTag& rLanguageTag ) const
{
    if( rFontName.isEmpty() )
        return nullptr;

    // search if a  (language dep.) replacement table for the given font exists
    // fallback is english
    OUString aSearchFont( rFontName.toAsciiLowerCase() );
    FontNameAttr aSearchAttr;
    aSearchAttr.Name = aSearchFont;

    LanguageTag aLanguageTag( rLanguageTag);

    if( aLanguageTag.isSystemLocale() )
        aLanguageTag = SvtSysLocale().GetUILanguageTag();

    ::std::vector< OUString > aFallbacks( aLanguageTag.getFallbackStrings( true));
    if (aLanguageTag.getLanguage() != "en")
        aFallbacks.push_back("en");

    for (::std::vector< OUString >::const_iterator fb( aFallbacks.begin()); fb != aFallbacks.end(); ++fb)
    {
        std::unordered_map< OUString, LocaleSubst, OUStringHash >::const_iterator lang = m_aSubst.find( *fb );
        if( lang != m_aSubst.end() )
        {
            if( ! lang->second.bConfigRead )
                readLocaleSubst( *fb );
            // try to find an exact match
            // because the list is sorted this will also find fontnames of the form searchfontname*
            std::vector< FontNameAttr >::const_iterator it = ::std::lower_bound( lang->second.aSubstAttributes.begin(), lang->second.aSubstAttributes.end(), aSearchAttr, StrictStringSort() );
            if( it != lang->second.aSubstAttributes.end())
            {
                const FontNameAttr& rFoundAttr = *it;
                // a search for "abcblack" may match with an entry for "abc"
                // the reverse is not a good idea (e.g. #i112731# alba->albani)
                if( rFoundAttr.Name.getLength() <= aSearchFont.getLength() )
                    if( aSearchFont.startsWith( rFoundAttr.Name))
                        return &rFoundAttr;
            }
        }
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
