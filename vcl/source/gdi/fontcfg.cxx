/*************************************************************************
 *
 *  $RCSfile: fontcfg.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:27:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VCL_FONTCFG_HXX
#include <fontcfg.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include <unotools/configpathes.hxx>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <string.h>
#include <list>
#include <algorithm>

#define DEFAULTFONT_CONFIGNODE "VCL/DefaultFonts"
#define SUBSTFONT_CONFIGNODE "VCL/FontSubstitutions"
#define SETTINGS_CONFIGNODE "VCL/Settings"

using namespace vcl;
using namespace rtl;
using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

struct KeyMap
{
    int             nEnum;
    const char* pName;
    int             nLen;
};

#define KM_Entry( n, s ) { n, s, sizeof( s )-1 }

static const KeyMap aKeyMap[] =
{
    KM_Entry( DEFAULTFONT_CJK_DISPLAY         , "CJK_DISPLAY" ),
    KM_Entry( DEFAULTFONT_CJK_HEADING         , "CJK_HEADING" ),
    KM_Entry( DEFAULTFONT_CJK_PRESENTATION    , "CJK_PRESENTATION" ),
    KM_Entry( DEFAULTFONT_CJK_SPREADSHEET     , "CJK_SPREADSHEET" ),
    KM_Entry( DEFAULTFONT_CJK_TEXT            , "CJK_TEXT" ),
    KM_Entry( DEFAULTFONT_CTL_DISPLAY         , "CTL_DISPLAY" ),
    KM_Entry( DEFAULTFONT_CTL_HEADING         , "CTL_HEADING" ),
    KM_Entry( DEFAULTFONT_CTL_PRESENTATION    , "CTL_PRESENTATION" ),
    KM_Entry( DEFAULTFONT_CTL_SPREADSHEET     , "CTL_SPREADSHEET" ),
    KM_Entry( DEFAULTFONT_CTL_TEXT            , "CTL_TEXT" ),
    KM_Entry( DEFAULTFONT_FIXED               , "FIXED" ),
    KM_Entry( DEFAULTFONT_LATIN_DISPLAY       , "LATIN_DISPLAY" ),
    KM_Entry( DEFAULTFONT_LATIN_FIXED         , "LATIN_FIXED" ),
    KM_Entry( DEFAULTFONT_LATIN_HEADING       , "LATIN_HEADING" ),
    KM_Entry( DEFAULTFONT_LATIN_PRESENTATION  , "LATIN_PRESENTATION" ),
    KM_Entry( DEFAULTFONT_LATIN_SPREADSHEET   , "LATIN_SPREADSHEET" ),
    KM_Entry( DEFAULTFONT_LATIN_TEXT          , "LATIN_TEXT" ),
    KM_Entry( DEFAULTFONT_SANS                , "SANS" ),
    KM_Entry( DEFAULTFONT_SANS_UNICODE        , "SANS_UNICODE" ),
    KM_Entry( DEFAULTFONT_SERIF               , "SERIF" ),
    KM_Entry( DEFAULTFONT_SYMBOL              , "SYMBOL" ),
    KM_Entry( DEFAULTFONT_UI_FIXED            , "UI_FIXED" ),
    KM_Entry( DEFAULTFONT_UI_SANS             , "UI_SANS" ),
};

extern "C" {
    static int compare_key( const void* pLeft, const void* pRight )
    {
        return strcmp( ((const KeyMap*)pLeft)->pName, ((const KeyMap*)pRight)->pName );
    }
}

int DefaultFontConfigItem::getKeyType( const OUString& rKey )
{
    const int nItems = sizeof( aKeyMap )/sizeof( aKeyMap[0] );
#if OSL_DEBUG_LEVEL > 1
    static bool bOnce = false;
    if( ! bOnce )
    {
        bOnce = true;
        for( int i = 0; i < nItems-1; i++ )
            if( strcmp( aKeyMap[i].pName, aKeyMap[i+1].pName ) > 0 )
                fprintf( stderr, "ERROR: map table entry %d in fontcfg.cxx is not string sorted\n", i );
    }
#endif

    OString aKey( rKey.getStr(), rKey.getLength(), RTL_TEXTENCODING_ASCII_US );

    KeyMap aSearch;
    aSearch.pName = aKey.getStr();
    const KeyMap* pEntry = (const KeyMap*)bsearch( &aSearch, aKeyMap, nItems, sizeof(KeyMap), compare_key );
    return pEntry ? pEntry->nEnum : -1;
}

/*
 *  DefaultFontConfigItem::get
 */

DefaultFontConfigItem* DefaultFontConfigItem::get()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->maGDIData.mpDefFontConfig )
        pSVData->maGDIData.mpDefFontConfig = new DefaultFontConfigItem();
    return pSVData->maGDIData.mpDefFontConfig;
}

/*
 *  DefaultFontConfigItem constructor
 */

DefaultFontConfigItem::DefaultFontConfigItem()
        :
        ConfigItem( OUString( RTL_CONSTASCII_USTRINGPARAM( DEFAULTFONT_CONFIGNODE ) ),
                    CONFIG_MODE_DELAYED_UPDATE )
{
    getValues();
}

/*
 *  DefaultFontConfigItem destructor
 */

DefaultFontConfigItem::~DefaultFontConfigItem()
{
    if( IsModified() )
        Commit();
}

/*
 *  DefaultFontConfigItem::Commit
 */

void DefaultFontConfigItem::Commit()
{
    if( ! IsValidConfigMgr() )
        return;

    int i, nKeys = sizeof(aKeyMap)/sizeof(aKeyMap[0]);

    std::hash_map< Locale, std::hash_map< int, OUString >, LocaleHash >::const_iterator lang;

    for( lang = m_aDefaults.begin(); lang != m_aDefaults.end(); ++lang )
    {
        OUStringBuffer aLanguage(16);
        aLanguage.append( lang->first.Language.toAsciiLowerCase() );
        if( lang->first.Country.getLength() )
        {
            aLanguage.append( sal_Unicode('-') );
            aLanguage.append( lang->first.Country.toAsciiLowerCase() );
            if( lang->first.Variant.getLength() )
            {
                aLanguage.append( sal_Unicode('-') );
                aLanguage.append( lang->first.Variant.toAsciiLowerCase() );
            }
        }
        if( ! aLanguage.getLength() )
            continue;

        OUString aKeyName = aLanguage.makeStringAndClear();
        AddNode( OUString(), aKeyName ); // defaults for a not yet configured language may be added
        Sequence< PropertyValue > aValues( nKeys );
        PropertyValue* pValues = aValues.getArray();
        int nIndex = 0;
        for( i = 0; i < nKeys; i++ )
        {
            std::hash_map< int, OUString >::const_iterator it = lang->second.find( aKeyMap[i].nEnum );
            if( it == lang->second.end() )
                continue;

            OUStringBuffer aName( 64 );
            aName.append( aKeyName );
            aName.append( sal_Unicode('/') );
            aName.appendAscii( aKeyMap[i].pName, aKeyMap[i].nLen );
            pValues[nIndex].Name    = aName.makeStringAndClear();
            pValues[nIndex].Handle  = it->first;
            pValues[nIndex].Value <<= it->second;
            pValues[nIndex].State   = PropertyState_DIRECT_VALUE;
            nIndex++;
        }
        aValues.realloc( nIndex );
        ReplaceSetProperties( aKeyName, aValues );
    }
    ClearModified();
}

/*
 *  DefaultFontConfigItem::Notify
 */

void DefaultFontConfigItem::Notify( const Sequence< OUString >& rPropertyNames )
{
    getValues();
}

/*
 *  DefaultFontConfigItem::getValues
 */
void DefaultFontConfigItem::getValues()
{
    if( ! IsValidConfigMgr() )
        return;

    m_aDefaults.clear();

    int i, j;
    Sequence< OUString > aNames( GetNodeNames( OUString() ) );
    for( j = 0; j < aNames.getLength(); j++ )
    {
        String aKeyName( aNames.getConstArray()[j] );
        Sequence< OUString > aKeys( GetNodeNames( aKeyName ) );
        Sequence< OUString > aLocaleKeys( aKeys.getLength() );
        const OUString* pFrom = aKeys.getConstArray();
        OUString* pTo = aLocaleKeys.getArray();
        for( int m = 0; m < aKeys.getLength(); m++ )
        {
            String aName( aKeyName );
            aName.Append( '/' );
            aName.Append( String( pFrom[m] ) );
            pTo[m] = aName;
        }
        Sequence< Any > aValues( GetProperties( aLocaleKeys ) );
        Locale aLocale;
        sal_Int32 nIndex = 0;
        const OUString* pIso = aNames.getConstArray() + j;
        aLocale.Language = pIso->getToken( 0, '-', nIndex ).toAsciiLowerCase();
        if( nIndex > 0 )
        {
            aLocale.Country = pIso->getToken( 0, '-', nIndex ).toAsciiLowerCase();
            if( nIndex > 0 )
                aLocale.Variant = pIso->copy( nIndex ).toAsciiLowerCase();
        }
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "found localized default font data for \"%s\" = %s-%s-%s\n",
                 OUStringToOString( *pIso, osl_getThreadTextEncoding() ).getStr(),
                 OUStringToOString( aLocale.Language, osl_getThreadTextEncoding() ).getStr(),
                 OUStringToOString( aLocale.Country, osl_getThreadTextEncoding() ).getStr(),
                 OUStringToOString( aLocale.Variant, osl_getThreadTextEncoding() ).getStr()

                 );
#endif
        const Any* pValue = aValues.getConstArray();
        for( i = 0; i < aValues.getLength(); i++, pValue++ )
        {
            if( pValue->getValueTypeClass() == TypeClass_STRING )
            {
                const OUString* pLine = (const OUString*)pValue->getValue();
                if( pLine->getLength() )
                    m_aDefaults[ aLocale ][ getKeyType( aKeys.getConstArray()[i] ) ] = *pLine;
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "   \"%s\"=\"%.30s\"\n",
                         OUStringToOString( aKeys.getConstArray()[i], RTL_TEXTENCODING_ASCII_US ).getStr(),
                         OUStringToOString( *pLine, RTL_TEXTENCODING_ASCII_US ).getStr()
                         );
#endif
            }
        }
    }
}

/*
 *  DefaultFontConfigItem::getDefaultFont
 */

const OUString& DefaultFontConfigItem::getDefaultFont( const Locale& rLocale, int nType ) const
{
    Locale aLocale;
    aLocale.Language = rLocale.Language.toAsciiLowerCase();
    aLocale.Country = rLocale.Country.toAsciiLowerCase();
    aLocale.Variant = rLocale.Variant.toAsciiLowerCase();
    std::hash_map< Locale, std::hash_map< int, OUString >, vcl::LocaleHash >::const_iterator lang = m_aDefaults.find( aLocale );
    if( lang == m_aDefaults.end() || lang->second.find( nType ) == lang->second.end() )
    {
        aLocale.Variant = OUString();
        lang = m_aDefaults.find( aLocale );
        if( lang == m_aDefaults.end() || lang->second.find( nType ) == lang->second.end() )
        {
            aLocale.Country = OUString();

            lang = m_aDefaults.find( aLocale );
            if( lang == m_aDefaults.end() || lang->second.find( nType ) == lang->second.end() )
            {
                aLocale.Language = OUString( RTL_CONSTASCII_USTRINGPARAM( "en" ) );
                lang = m_aDefaults.find( aLocale );
                if( lang == m_aDefaults.end() || lang->second.find( nType ) == lang->second.end() )
                {
                    static OUString aEmpty;
                    return aEmpty;
                }
            }
        }
    }
    return lang->second.find(nType)->second;
}

/*
 *  DefaultFontConfigItem::getUserInterfaceFont
 */

const OUString& DefaultFontConfigItem::getUserInterfaceFont( const Locale& rLocale ) const
{
    Locale aLocale;
    aLocale.Language = rLocale.Language.toAsciiLowerCase();
    aLocale.Country = rLocale.Country.toAsciiLowerCase();
    aLocale.Variant = rLocale.Variant.toAsciiLowerCase();

    if( ! aLocale.Language.getLength() )
        aLocale = Application::GetSettings().GetUILocale();

    const OUString& aUIFont = getDefaultFont( aLocale, DEFAULTFONT_UI_SANS );

    if( aUIFont.getLength() )
        return aUIFont;

    // fallback mechanism (either no configuration or no entry in configuration

    #define FALLBACKFONT_UI_SANS "Andale Sans UI;Tahoma;Arial Unicode MS;Arial;Bitstream Vera Sans;gnu-unifont;Interface User;Geneva;WarpSans;Dialog;Swiss;Lucida;Helvetica;Charcoal;Chicago;MS Sans Serif;Helv;Times;Times New Roman;Interface System"
    #define FALLBACKFONT_UI_SANS_LATIN2 "Andale Sans UI;Tahoma;Arial Unicode MS;Arial;Luxi Sans;Bitstream Vera Sans;Interface User;Geneva;WarpSans;Dialog;Swiss;Lucida;Helvetica;Charcoal;Chicago;MS Sans Serif;Helv;Times;Times New Roman;Interface System"
    #define FALLBACKFONT_UI_SANS_ARABIC "Tahoma;Traditional Arabic;Simplified Arabic;Lucidasans;Lucida Sans;Supplement;Andale Sans UI;clearlyU;Interface User;Arial Unicode MS;Lucida Sans Unicode;WarpSans;Geneva;MS Sans Serif;Helv;Dialog;Albany;Lucida;Helvetica;Charcoal;Chicago;Arial;Helmet;Interface System;Sans Serif"
    #define FALLBACKFONT_UI_SANS_THAI "OONaksit;Tahoma;Lucidasans;Arial Unicode MS"
    #define FALLBACKFONT_UI_SANS_KOREAN "SunGulim;BaekmukGulim;Gulim;Roundgothic;Arial Unicode MS;Lucida Sans Unicode;gnu-unifont;Andale Sans UI"
    #define FALLBACKFONT_UI_SANS_JAPANESE1 "Andale Sans UI;HG Mincho Light J;"
    #define FALLBACKFONT_UI_SANS_JAPANESE2 "kochigothic;gothic"
    #define FALLBACKFONT_UI_SANS_CHINSIM "Andale Sans UI;Arial Unicode MS;ZYSong18030;AR PL SungtiL GB;AR PL KaitiM GB;SimSun;Lucida Sans Unicode;Fangsong;Hei;Song;Kai;Ming;gnu-unifont;Interface User;"
    #define FALLBACKFONT_UI_SANS_CHINTRD "Andale Sans UI;Arial Unicode MS;AR PL Mingti2L Big5;AR PL KaitiM Big5;Kai;PMingLiU;MingLiU;Ming;Lucida Sans Unicode;gnu-unifont;Interface User;"

    // we need localized names for japanese fonts
    static sal_Unicode const aMSGothic[] = { 0xFF2D, 0xFF33, ' ', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
    static sal_Unicode const aMSPGothic[] = { 0xFF2D, 0xFF33, ' ', 0xFF30, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
    static sal_Unicode const aTLPGothic[] = { 0x0054, 0x004C, 0x0050, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
    static sal_Unicode const aLXGothic[] = { 0x004C, 0x0058, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
    static sal_Unicode const aKochiGothic[] = { 0x6771, 0x98A8, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };

    String aFallBackJapaneseLocalized( RTL_CONSTASCII_USTRINGPARAM( "MS UI Gothic;" ) );
    aFallBackJapaneseLocalized += String( RTL_CONSTASCII_USTRINGPARAM( FALLBACKFONT_UI_SANS_JAPANESE1 ) );
    aFallBackJapaneseLocalized += String( aMSPGothic );
    aFallBackJapaneseLocalized += String(RTL_CONSTASCII_USTRINGPARAM( ";" ) );
    aFallBackJapaneseLocalized += String( aMSGothic );
    aFallBackJapaneseLocalized += String(RTL_CONSTASCII_USTRINGPARAM( ";" ) );
    aFallBackJapaneseLocalized += String( aTLPGothic );
    aFallBackJapaneseLocalized += String(RTL_CONSTASCII_USTRINGPARAM( ";" ) );
    aFallBackJapaneseLocalized += String( aLXGothic );
    aFallBackJapaneseLocalized += String(RTL_CONSTASCII_USTRINGPARAM( ";" ) );
    aFallBackJapaneseLocalized += String( aKochiGothic );
    aFallBackJapaneseLocalized += String(RTL_CONSTASCII_USTRINGPARAM( ";" ) );
    aFallBackJapaneseLocalized += String(RTL_CONSTASCII_USTRINGPARAM( FALLBACKFONT_UI_SANS_JAPANESE2 ) );
    static const OUString aFallBackJapanese( aFallBackJapaneseLocalized );
    static const OUString aFallback (RTL_CONSTASCII_USTRINGPARAM(FALLBACKFONT_UI_SANS));
    static const OUString aFallbackLatin2 (RTL_CONSTASCII_USTRINGPARAM(FALLBACKFONT_UI_SANS_LATIN2));
    static const OUString aFallBackArabic (RTL_CONSTASCII_USTRINGPARAM( FALLBACKFONT_UI_SANS_ARABIC ) );
    static const OUString aFallBackThai (RTL_CONSTASCII_USTRINGPARAM( FALLBACKFONT_UI_SANS_THAI ) );
    static const OUString aFallBackChineseSIM (RTL_CONSTASCII_USTRINGPARAM( FALLBACKFONT_UI_SANS_CHINSIM ) );
    static const OUString aFallBackChineseTRD (RTL_CONSTASCII_USTRINGPARAM( FALLBACKFONT_UI_SANS_CHINTRD ) );

    // we need localized names for korean fonts
    static sal_Unicode const aSunGulim[] = { 0xC36C, 0xAD74, 0xB9BC, 0 };
    static sal_Unicode const aBaekmukGulim[] = { 0xBC31, 0xBC35, 0xAD74, 0xB9BC, 0 };
    String aFallBackKoreanLocalized( aSunGulim );
    aFallBackKoreanLocalized += String(RTL_CONSTASCII_USTRINGPARAM( ";" ) );
    aFallBackKoreanLocalized += String( aBaekmukGulim );
    aFallBackKoreanLocalized += String(RTL_CONSTASCII_USTRINGPARAM( ";" ) );
    aFallBackKoreanLocalized += String(RTL_CONSTASCII_USTRINGPARAM( FALLBACKFONT_UI_SANS_KOREAN ) );
    static const OUString aFallBackKorean( aFallBackKoreanLocalized );

    // optimize font list for some locales, as long as Andale Sans UI does not support them
    if( aLocale.Language.equalsAscii( "ar" ) ||
        aLocale.Language.equalsAscii( "he" ) ||
        aLocale.Language.equalsAscii( "iw" ) )
    {
        return aFallBackArabic;
    }
    else if( aLocale.Language.equalsAscii( "th" ) )
    {
        return aFallBackThai;
    }
    else if( aLocale.Language.equalsAscii( "ko" ) )
    {
        return aFallBackKorean;
    }
    else if( aLocale.Language.equalsAscii( "cs" ) ||
             aLocale.Language.equalsAscii( "hu" ) ||
             aLocale.Language.equalsAscii( "pl" ) ||
             aLocale.Language.equalsAscii( "ro" ) ||
             aLocale.Language.equalsAscii( "rm" ) ||
             aLocale.Language.equalsAscii( "hr" ) ||
             aLocale.Language.equalsAscii( "sk" ) ||
             aLocale.Language.equalsAscii( "sl" ) ||
             aLocale.Language.equalsAscii( "sb" ) )
    {
        return aFallbackLatin2;
    }
    else if( aLocale.Language.equalsAscii( "zh" ) )
    {
        if( ! aLocale.Country.equalsAscii( "cn" ) )
            return aFallBackChineseTRD;
        else
            return aFallBackChineseSIM;
    }
    else if( aLocale.Language.equalsAscii( "ja" ) )
    {
        return aFallBackJapanese;
    }

   return aFallback;
}

/*
 *  DefaultFontConfigItem::setDefaultFont
 */

void DefaultFontConfigItem::setDefaultFont( const Locale& rLocale, int nType, const OUString& rFont )
{
    Locale aLocale;
    aLocale.Language = rLocale.Language.toAsciiLowerCase();
    aLocale.Country = rLocale.Country.toAsciiLowerCase();
    aLocale.Variant = rLocale.Variant.toAsciiLowerCase();

    bool bModified = m_aDefaults[ aLocale ][ nType ] != rFont;
    if( bModified )
    {
        m_aDefaults[ aLocale ][ nType ] = rFont;
        SetModified();
    }
}

// ------------------------------------------------------------------------------------

/*
 *  FontSubstConfigItem::get
 */

FontSubstConfigItem* FontSubstConfigItem::get()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->maGDIData.mpFontSubstConfig )
        pSVData->maGDIData.mpFontSubstConfig = new FontSubstConfigItem();
    return pSVData->maGDIData.mpFontSubstConfig;
}

/*
 *  FontSubstConfigItem::FontSubstConfigItem
 */

FontSubstConfigItem::FontSubstConfigItem() :
        ConfigItem( OUString( RTL_CONSTASCII_USTRINGPARAM( SUBSTFONT_CONFIGNODE ) ),
                    CONFIG_MODE_DELAYED_UPDATE )
{
    getValues();
}

/*
 *  FontSubstConfigItem::~FontSubstConfigItem
 */

FontSubstConfigItem::~FontSubstConfigItem()
{
    if( IsModified() )
        Commit();
}

/*
 *  FontSubstConfigItem::Commit
 */

void FontSubstConfigItem::Commit()
{
    ClearModified();
}

/*
 *  FontSubstConfigItem::Notify
 */

void FontSubstConfigItem::Notify( const Sequence< OUString >& rPropertyNames )
{
    getValues();
}

/*
 *  FontSubstConfigItem::getMapName
 */
// =======================================================================

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
    NULL
};

// -----------------------------------------------------------------------

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
    NULL
};

// -----------------------------------------------------------------------

static const char* const aImplKillTrailingWithExceptionsList[] =
{
    "ce", "monospa", "oldfa", NULL,
    "ps", "ca", NULL,
    NULL
};

// -----------------------------------------------------------------------

struct ImplFontAttrWeightSearchData
{
    const char*             mpStr;
    FontWeight              meWeight;
};

static ImplFontAttrWeightSearchData const aImplWeightAttrSearchList[] =
{
{   "extrablack",           WEIGHT_BLACK },
{   "ultrablack",           WEIGHT_BLACK },
{   "black",                WEIGHT_BLACK },
{   "heavy",                WEIGHT_BLACK },
{   "ultrabold",            WEIGHT_ULTRABOLD },
{   "semibold",             WEIGHT_SEMIBOLD },
{   "bold",                 WEIGHT_BOLD },
{   "ultralight",           WEIGHT_ULTRALIGHT },
{   "semilight",            WEIGHT_SEMILIGHT },
{   "light",                WEIGHT_LIGHT },
{   "demi",                 WEIGHT_SEMIBOLD },
{   "medium",               WEIGHT_MEDIUM },
{   NULL,                   WEIGHT_DONTKNOW },
};

// -----------------------------------------------------------------------

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
{   NULL,                   WIDTH_DONTKNOW },
};

struct ImplFontAttrTypeSearchData
{
    const char*             mpStr;
    ULONG                   mnType;
};

static ImplFontAttrTypeSearchData const aImplTypeAttrSearchList[] =
{
{   "monotype",             0 },
{   "linotype",             0 },
{   "titling",              IMPL_FONT_ATTR_TITLING },
{   "captitals",            IMPL_FONT_ATTR_CAPITALS },
{   "captital",             IMPL_FONT_ATTR_CAPITALS },
{   "caps",                 IMPL_FONT_ATTR_CAPITALS },
{   "italic",               IMPL_FONT_ATTR_ITALIC },
{   "oblique",              IMPL_FONT_ATTR_ITALIC },
{   "rounded",              IMPL_FONT_ATTR_ROUNDED },
{   "outline",              IMPL_FONT_ATTR_OUTLINE },
{   "shadow",               IMPL_FONT_ATTR_SHADOW },
{   "handwriting",          IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_SCRIPT },
{   "hand",                 IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_SCRIPT },
{   "signet",               IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_SCRIPT },
{   "script",               IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_SCRIPT },
{   "calligraphy",          IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_SCRIPT },
{   "chancery",             IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_SCRIPT },
{   "corsiva",              IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_SCRIPT },
{   "gothic",               IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_GOTHIC },
{   "schoolbook",           IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "schlbk",               IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "typewriter",           IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "lineprinter",          IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "monospaced",           IMPL_FONT_ATTR_FIXED },
{   "monospace",            IMPL_FONT_ATTR_FIXED },
{   "mono",                 IMPL_FONT_ATTR_FIXED },
{   "fixed",                IMPL_FONT_ATTR_FIXED },
{   "sansserif",            IMPL_FONT_ATTR_SANSSERIF },
{   "sans",                 IMPL_FONT_ATTR_SANSSERIF },
{   "swiss",                IMPL_FONT_ATTR_SANSSERIF },
{   "serif",                IMPL_FONT_ATTR_SERIF },
{   "bright",               IMPL_FONT_ATTR_SERIF },
{   "symbols",              IMPL_FONT_ATTR_SYMBOL },
{   "symbol",               IMPL_FONT_ATTR_SYMBOL },
{   "dingbats",             IMPL_FONT_ATTR_SYMBOL },
{   "dings",                IMPL_FONT_ATTR_SYMBOL },
{   "ding",                 IMPL_FONT_ATTR_SYMBOL },
{   "bats",                 IMPL_FONT_ATTR_SYMBOL },
{   "math",                 IMPL_FONT_ATTR_SYMBOL },
{   "oldstyle",             IMPL_FONT_ATTR_OTHERSTYLE },
{   "oldface",              IMPL_FONT_ATTR_OTHERSTYLE },
{   "old",                  IMPL_FONT_ATTR_OTHERSTYLE },
{   "new",                  0 },
{   "modern",               0 },
{   "lucida",               0 },
{   "regular",              0 },
{   "extended",             0 },
{   "extra",                IMPL_FONT_ATTR_OTHERSTYLE },
{   "ext",                  0 },
{   "scalable",             0 },
{   "scale",                0 },
{   "nimbus",               0 },
{   "adobe",                0 },
{   "itc",                  0 },
{   "amt",                  0 },
{   "mt",                   0 },
{   "ms",                   0 },
{   "cpi",                  0 },
{   "no",                   0 },
{   NULL,                   0 },
};

// -----------------------------------------------------------------------

static bool ImplKillLeading( String& rName, const char* const* ppStr )
{
    for(; *ppStr; ++ppStr )
    {
        const char*         pStr = *ppStr;
        const xub_Unicode*  pNameStr = rName.GetBuffer();
        while ( (*pNameStr == (xub_Unicode)(unsigned char)*pStr) && *pStr )
        {
            pNameStr++;
            pStr++;
        }
        if ( !*pStr )
        {
            xub_StrLen nLen = pNameStr - rName.GetBuffer();
            rName.Erase( 0, nLen );
            return true;
        }
    }

    // special case for Baekmuk
    // TODO: allow non-ASCII KillLeading list
    const xub_Unicode* pNameStr = rName.GetBuffer();
    if( (pNameStr[0]==0xBC31) && (pNameStr[1]==0xBC35) )
    {
        xub_StrLen nLen = (pNameStr[2]==0x0020) ? 3 : 2;
        rName.Erase( 0, nLen );
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------

static xub_StrLen ImplIsTrailing( const sal_Unicode* pEndName, const char* pStr )
{
    const char* pTempStr = pStr;
    while ( *pTempStr )
        pTempStr++;

    xub_StrLen nStrLen = (xub_StrLen)(pTempStr-pStr);
    const sal_Unicode* pNameStr = pEndName-nStrLen;
    while ( (*pNameStr == (xub_Unicode)(unsigned char)*pStr) && *pStr )
    {
        pNameStr++;
        pStr++;
    }

    if ( *pStr )
        return 0;
    else
        return nStrLen;
}

// -----------------------------------------------------------------------

static BOOL ImplKillTrailing( String& rName, const char* const* ppStr )
{
    const xub_Unicode* pEndName = rName.GetBuffer()+rName.Len();
    while ( *ppStr )
    {
        xub_StrLen nTrailLen = ImplIsTrailing( pEndName, *ppStr );
        if ( nTrailLen )
        {
            rName.Erase( rName.Len()-nTrailLen );
            return TRUE;
        }

        ppStr++;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static BOOL ImplKillTrailingWithExceptions( String& rName, const char* const* ppStr )
{
    const xub_Unicode* pEndName = rName.GetBuffer()+rName.Len();
    while ( *ppStr )
    {
        xub_StrLen nTrailLen = ImplIsTrailing( pEndName, *ppStr );
        if ( nTrailLen )
        {
            const xub_Unicode* pEndNameTemp = pEndName-nTrailLen;
            while ( *ppStr )
            {
                if ( ImplIsTrailing( pEndNameTemp, *ppStr ) )
                    return FALSE;
                ppStr++;
            }

            rName.Erase( rName.Len()-nTrailLen );
            return TRUE;
        }
        else
        {
            while ( *ppStr )
                ppStr++;
        }

        ppStr++;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static BOOL ImplFindAndErase( String& rName, const char* pStr )
{
    xub_StrLen nPos = rName.SearchAscii( pStr );
    if ( nPos == STRING_NOTFOUND )
        return FALSE;

    const char* pTempStr = pStr;
    while ( *pTempStr )
        pTempStr++;
    rName.Erase( nPos, (xub_StrLen)(pTempStr-pStr) );
    return TRUE;
}

// =======================================================================

void FontSubstConfigItem::getMapName( const String& rOrgName, String& rShortName,
    String& rFamilyName, FontWeight& rWeight, FontWidth& rWidth, ULONG& rType )
{
    rShortName = rOrgName;

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
    rType = 0;
    const ImplFontAttrTypeSearchData* pTypeList = aImplTypeAttrSearchList;
    while ( pTypeList->mpStr )
    {
        if ( ImplFindAndErase( rFamilyName, pTypeList->mpStr ) )
            rType |= pTypeList->mnType;
        pTypeList++;
    }

    // Remove numbers
    xub_StrLen i = 0;
    while ( i < rFamilyName.Len() )
    {
        sal_Unicode c = rFamilyName.GetChar( i );
        if ( (c >= 0x0030) && (c <= 0x0039) )
            rFamilyName.Erase( i, 1 );
        else
            i++;
    }
}


/*
 *  FontSubstConfigItem::getValues
 */

struct StrictStringSort : public ::std::binary_function< const FontNameAttr&, const FontNameAttr&, bool >
{
    bool operator()( const FontNameAttr& rLeft, const FontNameAttr& rRight )
    { return rLeft.Name.CompareTo( rRight.Name ) == COMPARE_LESS ; }
};

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

void FontSubstConfigItem::getValues()
{
    if( ! IsValidConfigMgr() )
        return;

    Sequence< OUString > aLocales( GetNodeNames( OUString( ) ) );
    for( int j = 0; j < aLocales.getLength(); j++ )
    {
        const OUString* pIso = aLocales.getConstArray() +j;
        Locale aLocale;
        sal_Int32 nIndex = 0;
        aLocale.Language = pIso->getToken( 0, '-', nIndex ).toAsciiLowerCase();
        if( nIndex > 0 )
        {
            aLocale.Country = pIso->getToken( 0, '-', nIndex ).toAsciiLowerCase();
            if( nIndex > 0 )
                aLocale.Variant = pIso->copy( nIndex ).toAsciiLowerCase();
        }
        Sequence< OUString > aSubstFonts( GetNodeNames( *pIso ) );
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "reading %d font substitutions for locale %s\n",
                 aSubstFonts.getLength(),
                 OUStringToOString( *pIso, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
        for( int i = 0; i < aSubstFonts.getLength(); i++ )
        {
            FontNameAttr aAttributes;
            aAttributes.Name = aSubstFonts.getConstArray()[i];

            OUStringBuffer aFontKey( 128 );
            aFontKey.append( *pIso );
            aFontKey.append( sal_Unicode('/') );
            // must use the wrapped name in config paths due to non-ASCII chars (eg korean font names) !!!
            OUString wrappedName = wrapConfigurationElementName( aAttributes.Name );
            aFontKey.append( wrappedName );
            Sequence< OUString > aKeys( 7 );
            OUString* pKeys = aKeys.getArray();
            aFontKey.append( sal_Unicode('/') );
            OUString aPath = aFontKey.makeStringAndClear();
            aFontKey.append( aPath );
            aFontKey.appendAscii( "SubstFonts" );
            pKeys[0] = aFontKey.makeStringAndClear();
            aFontKey.append( aPath );
            aFontKey.appendAscii( "SubstFontsMS" );
            pKeys[1] = aFontKey.makeStringAndClear();
            aFontKey.append( aPath );
            aFontKey.appendAscii( "SubstFontsPS" );
            pKeys[2] = aFontKey.makeStringAndClear();
            aFontKey.append( aPath );
            aFontKey.appendAscii( "SubstFontsHTML" );
            pKeys[3] = aFontKey.makeStringAndClear();
            aFontKey.append( aPath );
            aFontKey.appendAscii( "FontWidth" );
            pKeys[4] = aFontKey.makeStringAndClear();
            aFontKey.append( aPath );
            aFontKey.appendAscii( "FontWeight" );
            pKeys[5] = aFontKey.makeStringAndClear();
            aFontKey.append( aPath );
            aFontKey.appendAscii( "FontType" );
            pKeys[6] = aFontKey.makeStringAndClear();
            Sequence< Any > aValues( GetProperties( aKeys ) );
            const Any* pValues = aValues.getConstArray();
            sal_Int32 nIndex = 0, width = -1, weight = -1, type = 0;
            const OUString* pLine;
            if( pValues[0].getValueTypeClass() == TypeClass_STRING )
            {
                pLine = (const OUString*)pValues[0].getValue();
                if( pLine->getLength() )
                    while( nIndex != -1 )
                    {
                        String aSubst( pLine->getToken( 0, ';', nIndex ) );
                        aSubst.ToLowerAscii();
                        aSubst.EraseAllChars( ' ' );
                        if( aSubst.Len() )
                            aAttributes.Substitutions.push_back( aSubst );
                    }
            }
            if( pValues[1].getValueTypeClass() == TypeClass_STRING )
            {
                pLine = (const OUString*)pValues[1].getValue();
                nIndex = 0;
                if( pLine->getLength() )
                    while( nIndex != -1 )
                    {
                        String aSubst( pLine->getToken( 0, ';', nIndex ) );
                        if( aSubst.Len() )
                            aAttributes.MSSubstitutions.push_back( aSubst );
                    }
            }
            if( pValues[2].getValueTypeClass() == TypeClass_STRING )
            {
                pLine = (const OUString*)pValues[2].getValue();
                nIndex = 0;
                if( pLine->getLength() )
                    while( nIndex != -1 )
                    {
                        String aSubst( pLine->getToken( 0, ';', nIndex ) );
                        if( aSubst.Len() )
                            aAttributes.PSSubstitutions.push_back( aSubst );
                    }
            }
            if( pValues[3].getValueTypeClass() == TypeClass_STRING )
            {
                pLine = (const OUString*)pValues[3].getValue();
                nIndex = 0;
                if( pLine->getLength() )
                    while( nIndex != -1 )
                    {
                        String aSubst( pLine->getToken( 0, ';', nIndex ) );
                        if( aSubst.Len() )
                            aAttributes.HTMLSubstitutions.push_back( aSubst );
                    }
            }
            if( pValues[4].getValueTypeClass() == TypeClass_STRING )
            {
                pLine = (const OUString*)pValues[4].getValue();
                if( pLine->getLength() )
                {
                    for( width=sizeof(pWidthNames)/sizeof(pWidthNames[0])-1; width >= 0; width-- )
                        if( pLine->equalsIgnoreAsciiCaseAscii( pWidthNames[width].pName ) )
                            break;
                }
#if OSL_DEBUG_LEVEL > 1
                if( width < 0 )
                    fprintf( stderr, "Error: invalid width %s for font %s\n",
                             OUStringToOString( *pLine, RTL_TEXTENCODING_ASCII_US ).getStr(),
                             ByteString( aAttributes.Name, RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
#endif
            }
            aAttributes.Width = (FontWidth)( width >= 0 ? pWidthNames[width].nEnum : WIDTH_DONTKNOW );
            if( pValues[5].getValueTypeClass() == TypeClass_STRING )
            {
                pLine = (const OUString*)pValues[5].getValue();
                if( pLine->getLength() )
                {
                    for( weight=sizeof(pWeightNames)/sizeof(pWeightNames[0])-1; weight >= 0; weight-- )
                        if( pLine->equalsIgnoreAsciiCaseAscii( pWeightNames[weight].pName ) )
                            break;
                }
#if OSL_DEBUG_LEVEL > 1
                if( width < 0 )
                    fprintf( stderr, "Error: invalid weight %s for font %s\n",
                             OUStringToOString( *pLine, RTL_TEXTENCODING_ASCII_US ).getStr(),
                             ByteString( aAttributes.Name, RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
#endif
            }
            aAttributes.Weight = (FontWeight)( weight >= 0 ? pWeightNames[weight].nEnum : WEIGHT_DONTKNOW );
            if( pValues[6].getValueTypeClass() == TypeClass_STRING )
            {
                pLine = (const OUString*)pValues[6].getValue();
                if( pLine->getLength() )
                {
                    nIndex = 0;
                    while( nIndex != -1 )
                    {
                        String aToken( pLine->getToken( 0, ',', nIndex ) );
                        for( int k = 0; k < 32; k++ )
                            if( aToken.EqualsIgnoreCaseAscii( pAttribNames[k] ) )
                            {
                                type |= 1 << k;
                                break;
                            }
                    }
                }
            }
            aAttributes.Type = type;

            m_aSubstitutions[aLocale].push_back( aAttributes );
        }
        std::sort( m_aSubstitutions[aLocale].begin(), m_aSubstitutions[aLocale].end(), StrictStringSort() );
    }
}

const FontNameAttr* FontSubstConfigItem::getSubstInfo( const String& rFontName, const Locale& rLocale ) const
{
    if( !rFontName.Len() )
        return NULL;

    // search if a  (language dep.) replacement table for the given font exists
    // fallback is english
    String aSearchFont( rFontName );
    aSearchFont.ToLowerAscii();
    FontNameAttr aSearchAttr;
    aSearchAttr.Name = aSearchFont;

    Locale aLocale;
    aLocale.Language = rLocale.Language.toAsciiLowerCase();
    aLocale.Country = rLocale.Country.toAsciiLowerCase();
    aLocale.Variant = rLocale.Variant.toAsciiLowerCase();

    if( ! aLocale.Language.getLength() )
        aLocale = Application::GetSettings().GetUILocale();

    while( aLocale.Language.getLength() )
    {
        std::hash_map< Locale, std::vector< FontNameAttr >, LocaleHash >::const_iterator lang = m_aSubstitutions.find( aLocale );
        if( lang != m_aSubstitutions.end() )
        {
            // try to find an exact match
            // because the list is sorted this will also find fontnames of the form searchfontname*
            ::std::vector< FontNameAttr >::const_iterator it = ::std::lower_bound( lang->second.begin(), lang->second.end(), aSearchAttr, StrictStringSort() );
            if( it != lang->second.end() && aSearchFont.CompareTo( it->Name, aSearchFont.Len() ) == COMPARE_EQUAL )
                return &(*it);
        }
        // gradually become more unspecific
        if( aLocale.Variant.getLength() )
            aLocale.Variant = OUString();
        else if( aLocale.Country.getLength() )
            aLocale.Country = OUString();
        else if( ! aLocale.Language.equalsAscii( "en" ) )
            aLocale.Language = OUString( RTL_CONSTASCII_USTRINGPARAM( "en" ) );
        else
            aLocale.Language = OUString();
    }
    return NULL;
}

/*
 *  SettingsConfigItem::get
 */

SettingsConfigItem* SettingsConfigItem::get()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSettingsConfigItem )
        pSVData->mpSettingsConfigItem = new SettingsConfigItem();
    return pSVData->mpSettingsConfigItem;
}

/*
 *  SettignsConfigItem constructor
 */

SettingsConfigItem::SettingsConfigItem()
        :
        ConfigItem( OUString( RTL_CONSTASCII_USTRINGPARAM( SETTINGS_CONFIGNODE ) ),
                    CONFIG_MODE_DELAYED_UPDATE )
{
    getValues();
}

/*
 *  SettingsConfigItem destructor
 */

SettingsConfigItem::~SettingsConfigItem()
{
    if( IsModified() )
        Commit();
}

/*
 *  SettingsConfigItem::Commit
 */

void SettingsConfigItem::Commit()
{
    if( ! IsValidConfigMgr() )
        return;

    std::hash_map< OUString, std::hash_map< OUString, OUString, OUStringHash >, OUStringHash >::const_iterator group;

    for( group = m_aSettings.begin(); group != m_aSettings.end(); ++group )
    {
        String aKeyName( group->first );
        sal_Bool bAdded = AddNode( OUString(), aKeyName );
        Sequence< PropertyValue > aValues( group->second.size() );
        PropertyValue* pValues = aValues.getArray();
        int nIndex = 0;
        std::hash_map< OUString, OUString, OUStringHash >::const_iterator it;
        for( it = group->second.begin(); it != group->second.end(); ++it )
        {
            String aName( aKeyName );
            aName.Append( '/' );
            aName.Append( String( it->first ) );
            pValues[nIndex].Name    = aName;
            pValues[nIndex].Handle  = 0;
            pValues[nIndex].Value <<= it->second;
            pValues[nIndex].State   = PropertyState_DIRECT_VALUE;
            nIndex++;
        }
        ReplaceSetProperties( aKeyName, aValues );
    }
}

/*
 *  SettingsConfigItem::Notify
 */

void SettingsConfigItem::Notify( const Sequence< OUString >& rPropertyNames )
{
    getValues();
}

/*
 *  SettingsConfigItem::getValues
 */
void SettingsConfigItem::getValues()
{
    if( ! IsValidConfigMgr() )
        return;

    m_aSettings.clear();

    int i, j;
    Sequence< OUString > aNames( GetNodeNames( OUString() ) );
    for( j = 0; j < aNames.getLength(); j++ )
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "found settings data for \"%s\"\n",
                 OUStringToOString( aNames.getConstArray()[j], RTL_TEXTENCODING_ASCII_US ).getStr()
                 );
#endif
        String aKeyName( aNames.getConstArray()[j] );
        Sequence< OUString > aKeys( GetNodeNames( aKeyName ) );
        Sequence< OUString > aSettingsKeys( aKeys.getLength() );
        const OUString* pFrom = aKeys.getConstArray();
        OUString* pTo = aSettingsKeys.getArray();
        for( int m = 0; m < aKeys.getLength(); m++ )
        {
            String aName( aKeyName );
            aName.Append( '/' );
            aName.Append( String( pFrom[m] ) );
            pTo[m] = aName;
        }
        Sequence< Any > aValues( GetProperties( aSettingsKeys ) );
        const Any* pValue = aValues.getConstArray();
        for( i = 0; i < aValues.getLength(); i++, pValue++ )
        {
            if( pValue->getValueTypeClass() == TypeClass_STRING )
            {
                const OUString* pLine = (const OUString*)pValue->getValue();
                if( pLine->getLength() )
                    m_aSettings[ aKeyName ][ pFrom[i] ] = *pLine;
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "   \"%s\"=\"%.30s\"\n",
                         OUStringToOString( aKeys.getConstArray()[i], RTL_TEXTENCODING_ASCII_US ).getStr(),
                         OUStringToOString( *pLine, RTL_TEXTENCODING_ASCII_US ).getStr()
                         );
#endif
            }
        }
    }
}

/*
 *  SettingsConfigItem::getDefaultFont
 */

const OUString& SettingsConfigItem::getValue( const OUString& rGroup, const OUString& rKey ) const
{
    ::std::hash_map< OUString, ::std::hash_map< OUString, OUString, OUStringHash >, OUStringHash >::const_iterator group = m_aSettings.find( rGroup );
    if( group == m_aSettings.end() || group->second.find( rKey ) == group->second.end() )
    {
        static OUString aEmpty;
        return aEmpty;
    }
    return group->second.find(rKey)->second;
}

/*
 *  SettingsConfigItem::setDefaultFont
 */

void SettingsConfigItem::setValue( const OUString& rGroup, const OUString& rKey, const OUString& rValue )
{
    bool bModified = m_aSettings[ rGroup ][ rKey ] != rValue;
    if( bModified )
    {
        m_aSettings[ rGroup ][ rKey ] = rValue;
        SetModified();
    }
}

