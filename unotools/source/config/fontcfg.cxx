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
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/any.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/fontcfg.hxx>
#include <unotools/fontdefs.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <unotools/syslocale.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <string.h>
#include <algorithm>

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::configuration;

/*
 * DefaultFontConfiguration
 */

static OUString getKeyType( DefaultFontType nKeyType )
{
    switch( nKeyType )
    {
    case DefaultFontType::CJK_DISPLAY: return u"CJK_DISPLAY"_ustr;
    case DefaultFontType::CJK_HEADING: return u"CJK_HEADING"_ustr;
    case DefaultFontType::CJK_PRESENTATION: return u"CJK_PRESENTATION"_ustr;
    case DefaultFontType::CJK_SPREADSHEET: return u"CJK_SPREADSHEET"_ustr;
    case DefaultFontType::CJK_TEXT: return u"CJK_TEXT"_ustr;
    case DefaultFontType::CTL_DISPLAY: return u"CTL_DISPLAY"_ustr;
    case DefaultFontType::CTL_HEADING: return u"CTL_HEADING"_ustr;
    case DefaultFontType::CTL_PRESENTATION: return u"CTL_PRESENTATION"_ustr;
    case DefaultFontType::CTL_SPREADSHEET: return u"CTL_SPREADSHEET"_ustr;
    case DefaultFontType::CTL_TEXT: return u"CTL_TEXT"_ustr;
    case DefaultFontType::FIXED: return u"FIXED"_ustr;
    case DefaultFontType::LATIN_DISPLAY: return u"LATIN_DISPLAY"_ustr;
    case DefaultFontType::LATIN_FIXED: return u"LATIN_FIXED"_ustr;
    case DefaultFontType::LATIN_HEADING: return u"LATIN_HEADING"_ustr;
    case DefaultFontType::LATIN_PRESENTATION: return u"LATIN_PRESENTATION"_ustr;
    case DefaultFontType::LATIN_SPREADSHEET: return u"LATIN_SPREADSHEET"_ustr;
    case DefaultFontType::LATIN_TEXT: return u"LATIN_TEXT"_ustr;
    case DefaultFontType::SANS: return u"SANS"_ustr;
    case DefaultFontType::SANS_UNICODE: return u"SANS_UNICODE"_ustr;
    case DefaultFontType::SERIF: return u"SERIF"_ustr;
    case DefaultFontType::SYMBOL: return u"SYMBOL"_ustr;
    case DefaultFontType::UI_FIXED: return u"UI_FIXED"_ustr;
    case DefaultFontType::UI_SANS: return u"UI_SANS"_ustr;
    default:
        OSL_FAIL( "unmatched type" );
        return u""_ustr;
    }
}

DefaultFontConfiguration& DefaultFontConfiguration::get()
{
    static DefaultFontConfiguration theDefaultFontConfiguration;
    return theDefaultFontConfiguration;
}

DefaultFontConfiguration::DefaultFontConfiguration()
{
    if (comphelper::IsFuzzing())
        return;
    // create configuration hierarchical access name
    try
    {
        // get service provider
        m_xConfigProvider = theDefaultProvider::get(comphelper::getProcessComponentContext());
        Sequence<Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", Any(OUString( "/org.openoffice.VCL/DefaultFonts" ))}
        }));
        m_xConfigAccess =
            Reference< XNameAccess >(
                m_xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess",
                                                                aArgs ),
                UNO_QUERY );
        if( m_xConfigAccess.is() )
        {
            const Sequence< OUString > aLocales = m_xConfigAccess->getElementNames();
            // fill config hash with empty interfaces
            for( const OUString& rLocaleString : aLocales )
            {
                // Feed through LanguageTag for casing.
                OUString aLoc( LanguageTag( rLocaleString, true).getBcp47( false));
                m_aConfig[ aLoc ] = LocaleAccess();
                m_aConfig[ aLoc ].aConfigLocaleString = rLocaleString;
            }
        }
    }
    catch (const Exception&)
    {
        // configuration is awry
        m_xConfigProvider.clear();
        m_xConfigAccess.clear();
    }
    SAL_INFO("unotools.config", "config provider: " << m_xConfigProvider.is()
            << ", config access: " << m_xConfigAccess.is());
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

    std::unordered_map< OUString, LocaleAccess >::const_iterator it = m_aConfig.find( rBcp47 );
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
    OUString aType = getKeyType( nType );
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
            for (const auto& rFallback : aFallbacks)
            {
                aRet = tryLocale( rFallback, aType );
                if (!aRet.isEmpty())
                    break;
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

    static constexpr OUStringLiteral FALLBACKFONT_UI_SANS = u"Andale Sans UI;Albany;Albany AMT;Tahoma;Arial Unicode MS;Arial;Nimbus Sans L;Bitstream Vera Sans;gnu-unifont;Interface User;Geneva;WarpSans;Dialog;Swiss;Lucida;Helvetica;Charcoal;Chicago;MS Sans Serif;Helv;Times;Times New Roman;Interface System";
    static constexpr OUStringLiteral FALLBACKFONT_UI_SANS_LATIN2 = u"Andale Sans UI;Albany;Albany AMT;Tahoma;Arial Unicode MS;Arial;Nimbus Sans L;Luxi Sans;Bitstream Vera Sans;Interface User;Geneva;WarpSans;Dialog;Swiss;Lucida;Helvetica;Charcoal;Chicago;MS Sans Serif;Helv;Times;Times New Roman;Interface System";
    static constexpr OUStringLiteral FALLBACKFONT_UI_SANS_ARABIC = u"Tahoma;Traditional Arabic;Simplified Arabic;Lucidasans;Lucida Sans;Supplement;Andale Sans UI;clearlyU;Interface User;Arial Unicode MS;Lucida Sans Unicode;WarpSans;Geneva;MS Sans Serif;Helv;Dialog;Albany;Lucida;Helvetica;Charcoal;Chicago;Arial;Helmet;Interface System;Sans Serif";
    static constexpr OUStringLiteral FALLBACKFONT_UI_SANS_THAI = u"OONaksit;Tahoma;Lucidasans;Arial Unicode MS";
    static constexpr OUStringLiteral FALLBACKFONT_UI_SANS_KOREAN = u"Noto Sans KR;Noto Sans CJK KR;Noto Serif KR;Noto Serif CJK KR;Source Han Sans KR;NanumGothic;NanumBarunGothic;NanumBarunGothic YetHangul;KoPubWorld Dotum;Malgun Gothic;Apple SD Gothic Neo;Dotum;DotumChe;Gulim;GulimChe;Batang;BatangChe;Apple Gothic;UnDotum;Baekmuk Gulim;Arial Unicode MS;Lucida Sans Unicode;gnu-unifont;Andale Sans UI";
    static constexpr OUStringLiteral FALLBACKFONT_UI_SANS_JAPANESE = u"Noto Sans CJK JP;Noto Sans JP;Source Han Sans;Source Han Sans JP;Yu Gothic UI;Yu Gothic;YuGothic;Hiragino Sans;Hiragino Kaku Gothic ProN;Hiragino Kaku Gothic Pro;Hiragino Kaku Gothic StdN;Meiryo UI;Meiryo;IPAexGothic;IPAPGothic;IPAGothic;MS UI Gothic;MS PGothic;MS Gothic;Osaka;Unifont;gnu-unifont;Arial Unicode MS;Interface System";
    static constexpr OUStringLiteral FALLBACKFONT_UI_SANS_CHINSIM = u"Andale Sans UI;Arial Unicode MS;ZYSong18030;AR PL SungtiL GB;AR PL KaitiM GB;SimSun;Lucida Sans Unicode;Fangsong;Hei;Song;Kai;Ming;gnu-unifont;Interface User;";
    static constexpr OUStringLiteral FALLBACKFONT_UI_SANS_CHINTRD = u"Andale Sans UI;Arial Unicode MS;AR PL Mingti2L Big5;AR PL KaitiM Big5;Kai;PMingLiU;MingLiU;Ming;Lucida Sans Unicode;gnu-unifont;Interface User;";

    const OUString aLanguage( aLanguageTag.getLanguage());

    // optimize font list for some locales, as long as Andale Sans UI does not support them
    if( aLanguage == "ar" || aLanguage == "he" || aLanguage == "iw"  )
    {
        return FALLBACKFONT_UI_SANS_ARABIC;
    }
    else if ( aLanguage == "th" )
    {
        return FALLBACKFONT_UI_SANS_THAI;
    }
    else if ( aLanguage == "ko" )
    {
        return FALLBACKFONT_UI_SANS_KOREAN;
    }
    else if ( aLanguage == "ja" )
    {
        return FALLBACKFONT_UI_SANS_JAPANESE;
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
        return FALLBACKFONT_UI_SANS_LATIN2;
    }
    else
    {
        const Locale& aLocale( aLanguageTag.getLocale());
        if (MsLangId::isTraditionalChinese(aLocale))
            return FALLBACKFONT_UI_SANS_CHINTRD;
        else if (MsLangId::isSimplifiedChinese(aLocale))
            return FALLBACKFONT_UI_SANS_CHINSIM;
    }

    return FALLBACKFONT_UI_SANS;
}

/*
 *  FontSubstConfigItem::get
 */

FontSubstConfiguration& FontSubstConfiguration::get()
{
    static FontSubstConfiguration theFontSubstConfiguration;
    return theFontSubstConfiguration;
}

/*
 *  FontSubstConfigItem::FontSubstConfigItem
 */

FontSubstConfiguration::FontSubstConfiguration() :
    maSubstHash( 300 ),
    maLanguageTag("en")
{
    if (comphelper::IsFuzzing())
        return;
    try
    {
        // get service provider
        Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
        // create configuration hierarchical access name
        m_xConfigProvider = theDefaultProvider::get( xContext );
        Sequence<Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", Any(OUString( "/org.openoffice.VCL/FontSubstitutions" ))}
        }));
        m_xConfigAccess =
            Reference< XNameAccess >(
                m_xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess",
                                                                aArgs ),
                UNO_QUERY );
        if( m_xConfigAccess.is() )
        {
            const Sequence< OUString > aLocales = m_xConfigAccess->getElementNames();
            // fill config hash with empty interfaces
            for( const OUString& rLocaleString : aLocales )
            {
                // Feed through LanguageTag for casing.
                OUString aLoc( LanguageTag( rLocaleString, true).getBcp47( false));
                m_aSubst[ aLoc ] = LocaleSubst();
                m_aSubst[ aLoc ].aConfigLocaleString = rLocaleString;
            }
        }
    }
    catch (const Exception&)
    {
        // configuration is awry
        m_xConfigProvider.clear();
        m_xConfigAccess.clear();
    }
    SAL_INFO("unotools.config", "config provider: " << m_xConfigProvider.is()
            << ", config access: " << m_xConfigAccess.is());

    if( maLanguageTag.isSystemLocale() )
        maLanguageTag = SvtSysLocale().GetUILanguageTag();
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

const char* const aImplKillLeadingList[] =
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

const char* const aImplKillTrailingList[] =
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

const char* const aImplKillTrailingWithExceptionsList[] =
{
    "ce", "monospace", "oldface", nullptr,
    "ps", "caps", nullptr,
    nullptr
};

namespace {

struct ImplFontAttrWeightSearchData
{
    const char*             mpStr;
    FontWeight              meWeight;
};

}

ImplFontAttrWeightSearchData const aImplWeightAttrSearchList[] =
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

namespace {

struct ImplFontAttrWidthSearchData
{
    const char*             mpStr;
    FontWidth               meWidth;
};

}

ImplFontAttrWidthSearchData const aImplWidthAttrSearchList[] =
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

namespace {

struct ImplFontAttrTypeSearchData
{
    const char*             mpStr;
    ImplFontAttrs           mnType;
};

}

ImplFontAttrTypeSearchData const aImplTypeAttrSearchList[] =
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
        while ( (*pNameStr == static_cast<sal_Unicode>(static_cast<unsigned char>(*pStr))) && *pStr )
        {
            pNameStr++;
            pStr++;
        }
        if ( !*pStr )
        {
            sal_Int32 nLen = static_cast<sal_Int32>(pNameStr - rName.getStr());
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

static sal_Int32 ImplIsTrailing( std::u16string_view rName, const char* pStr )
{
    size_t nStrLen = strlen( pStr );
    if( nStrLen >= rName.size() )
        return 0;

    const sal_Unicode* pEndName = rName.data() + rName.size();
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
    sal_Int32 nLen = static_cast<sal_Int32>(strlen(pStr));
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

namespace {

struct StrictStringSort
{
    bool operator()( const FontNameAttr& rLeft, const FontNameAttr& rRight )
    { return rLeft.Name.compareTo( rRight.Name ) < 0; }
};

}

// The entries in this table must match the bits in the ImplFontAttrs enum.

const char* const pAttribNames[] =
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

namespace {

struct enum_convert
{
    const char* pName;
    int          nEnum;
};

}

const enum_convert pWeightNames[] =
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

const enum_convert pWidthNames[] =
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
        if( auto pLine = o3tl::tryAccess<OUString>(aAny) )
        {
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
                        auto itPair = maSubstHash.insert( aSubst );
                        if (!itPair.second)
                            aSubst = *itPair.first;
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

// static
FontWeight FontSubstConfiguration::getSubstWeight( const css::uno::Reference< XNameAccess >& rFont,
                                                   const OUString& rType )
{
    int weight = -1;
    try
    {
        Any aAny = rFont->getByName( rType );
        if( auto pLine = o3tl::tryAccess<OUString>(aAny) )
        {
            if( !pLine->isEmpty() )
            {
                for( weight=std::size(pWeightNames)-1; weight >= 0; weight-- )
                    if( pLine->equalsIgnoreAsciiCaseAscii( pWeightNames[weight].pName ) )
                        break;
            }
            SAL_WARN_IF(weight < 0, "unotools.config", "Error: invalid weight " << *pLine);
        }
    }
    catch (const NoSuchElementException&)
    {
    }
    catch (const WrappedTargetException&)
    {
    }
    return static_cast<FontWeight>( weight >= 0 ? pWeightNames[weight].nEnum : WEIGHT_DONTKNOW );
}

// static
FontWidth FontSubstConfiguration::getSubstWidth( const css::uno::Reference< XNameAccess >& rFont,
                                                 const OUString& rType )
{
    int width = -1;
    try
    {
        Any aAny = rFont->getByName( rType );
        if( auto pLine = o3tl::tryAccess<OUString>(aAny) )
        {
            if( !pLine->isEmpty() )
            {
                for( width=std::size(pWidthNames)-1; width >= 0; width-- )
                    if( pLine->equalsIgnoreAsciiCaseAscii( pWidthNames[width].pName ) )
                        break;
            }
            SAL_WARN_IF( width < 0, "unotools.config", "Error: invalid width " << *pLine);
        }
    }
    catch (const NoSuchElementException&)
    {
    }
    catch (const WrappedTargetException&)
    {
    }
    return static_cast<FontWidth>( width >= 0 ? pWidthNames[width].nEnum : WIDTH_DONTKNOW );
}

// static
ImplFontAttrs FontSubstConfiguration::getSubstType( const css::uno::Reference< XNameAccess >& rFont,
                                                    const OUString& rType )
{
    sal_uInt32 type = 0;
    try
    {
        Any aAny = rFont->getByName( rType );
        auto pLine = o3tl::tryAccess<OUString>(aAny);
        if( !pLine )
            return ImplFontAttrs::None;
        if( pLine->isEmpty() )
            return ImplFontAttrs::None;
        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            OUString aToken( pLine->getToken( 0, ',', nIndex ) );
            for( int k = 0; k < 32; k++ )
                if( aToken.equalsIgnoreAsciiCaseAscii( pAttribNames[k] ) )
                {
                    type |= sal_uInt32(1) << k;
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
    std::unordered_map< OUString, LocaleSubst >::const_iterator it = m_aSubst.find( rBcp47 );
    if( it == m_aSubst.end() )
        return;

    if(  it->second.bConfigRead )
        return;

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
    if( !xNode.is() )
        return;

    const Sequence< OUString > aFonts = xNode->getElementNames();
    int nFonts = aFonts.getLength();
    // improve performance, heap fragmentation
    it->second.aSubstAttributes.reserve( nFonts );

    // strings for subst retrieval, construct only once
    static constexpr OUStringLiteral aSubstFontsStr  ( u"SubstFonts" );
    static constexpr OUStringLiteral aSubstFontsMSStr( u"SubstFontsMS" );
    static constexpr OUStringLiteral aSubstWeightStr ( u"FontWeight" );
    static constexpr OUStringLiteral aSubstWidthStr  ( u"FontWidth" );
    static constexpr OUStringLiteral aSubstTypeStr   ( u"FontType" );
    for( const OUString& rFontName : aFonts )
    {
        Reference< XNameAccess > xFont;
        try
        {
            Any aAny = xNode->getByName( rFontName );
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
            SAL_WARN("unotools.config", "did not get font attributes for " << rFontName);
            continue;
        }

        FontNameAttr aAttr;
        // read subst attributes from config
        aAttr.Name = rFontName;
        fillSubstVector( xFont, aSubstFontsStr, aAttr.Substitutions );
        fillSubstVector( xFont, aSubstFontsMSStr, aAttr.MSSubstitutions );
        aAttr.Weight = getSubstWeight( xFont, aSubstWeightStr );
        aAttr.Width = getSubstWidth( xFont, aSubstWidthStr );
        aAttr.Type = getSubstType( xFont, aSubstTypeStr );

        // finally insert this entry
        it->second.aSubstAttributes.push_back( aAttr );
    }
    std::sort( it->second.aSubstAttributes.begin(), it->second.aSubstAttributes.end(), StrictStringSort() );
}

const FontNameAttr* FontSubstConfiguration::getSubstInfo( const OUString& rFontName ) const
{
    if( rFontName.isEmpty() )
        return nullptr;

    // search if a  (language dep.) replacement table for the given font exists
    // fallback is english
    OUString aSearchFont( rFontName.toAsciiLowerCase() );
    FontNameAttr aSearchAttr;
    aSearchAttr.Name = aSearchFont;

    ::std::vector< OUString > aFallbacks( maLanguageTag.getFallbackStrings( true));
    if (maLanguageTag.getLanguage() != "en")
        aFallbacks.emplace_back("en");

    for (const auto& rFallback : aFallbacks)
    {
        std::unordered_map< OUString, LocaleSubst >::const_iterator lang = m_aSubst.find( rFallback );
        if( lang != m_aSubst.end() )
        {
            if( ! lang->second.bConfigRead )
                readLocaleSubst( rFallback );
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
