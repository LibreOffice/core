/*************************************************************************
 *
 *  $RCSfile: fontcfg.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 10:46:08 $
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

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#if defined DEBUG_TIMING
#include <stdio.h>
#include <osl/time.h>
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

    ::std::map< int, ::std::map< int, OUString > >::const_iterator lang;

    for( lang = m_aDefaults.begin(); lang != m_aDefaults.end(); ++lang )
    {
        int nLanguage = lang->first;
        if( nLanguage == LANGUAGE_DONTKNOW      ||
            nLanguage == LANGUAGE_NONE          ||
            nLanguage == LANGUAGE_SYSTEM        ||
            ( nLanguage >= LANGUAGE_USER1 && nLanguage <= LANGUAGE_USER9 )
            )
            continue;
        String aLanguage( ConvertLanguageToIsoString( nLanguage, '-' ).ToLowerAscii() );
        if( ! aLanguage.Len() )
            continue;

        String aKeyName( aLanguage );
        sal_Bool bAdded = AddNode( OUString(), aLanguage ); // defaults for a not yet configured language may be added
        Sequence< PropertyValue > aValues( nKeys );
        PropertyValue* pValues = aValues.getArray();
        int nIndex = 0;
        for( i = 0; i < nKeys; i++ )
        {
            ::std::map< int, OUString >::const_iterator it = lang->second.find( aKeyMap[i].nEnum );
            if( it == lang->second.end() )
                continue;

            String aName( aKeyName );
            aName.Append( '/' );
            aName.AppendAscii( aKeyMap[i].pName, aKeyMap[i].nLen );
            pValues[nIndex].Name    = aName;
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
#ifdef DEBUG_TIMING
static double fTimeCorrection = 0.0;
#define START_TIMING() osl_getSystemTime( &aTmpTV1 );
#define END_TIMING()\
    osl_getSystemTime( &aTmpTV2 );\
    fTmp = (double)aTmpTV2.Seconds + 1e-9*(double)aTmpTV2.Nanosec - (double)aTmpTV1.Seconds -1e-9*(double)aTmpTV1.Nanosec;\
    if( fTmp > 0.0005 )\
        fConfigTime += fTmp;\
    else\
        fprintf( stderr, "warning: timining inaccurate: %g\n", fTmp );

#else
#define START_TIMING()
#define END_TIMING()
#endif

void DefaultFontConfigItem::getValues()
{
    if( ! IsValidConfigMgr() )
        return;

#ifdef DEBUG_TIMING
    static bool bOnce = false;
    TimeValue aStart;
    TimeValue aStop;
    TimeValue aTmpTV1, aTmpTV2;
    double fConfigTime = 0.0;
    double fTmp;
    if( ! bOnce )
    {
        bOnce = true;
        START_TIMING();
        for( int i = 0; i < 100000; i++ )
            osl_getSystemTime( &aStop );
        END_TIMING();
        fTimeCorrection = fTmp/100000.0;
        fprintf( stderr, "osl_getSystemTime takes an average of %g seconds\n", fTimeCorrection );
    }
    osl_getSystemTime( &aStart );
    fConfigTime = 0.0;
#endif

    m_aDefaults.clear();

    int i, j;
    START_TIMING();
    Sequence< OUString > aNames( GetNodeNames( OUString() ) );
    END_TIMING();
    for( j = 0; j < aNames.getLength(); j++ )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found localized default font data for \"%s\"\n",
                 OUStringToOString( aNames.getConstArray()[j], RTL_TEXTENCODING_ASCII_US ).getStr()
                 );
#endif
        String aKeyName( aNames.getConstArray()[j] );
        START_TIMING();
        Sequence< OUString > aKeys( GetNodeNames( aKeyName ) );
        END_TIMING();
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
        START_TIMING();
        Sequence< Any > aValues( GetProperties( aLocaleKeys ) );
        END_TIMING();
        int nLanguageType = ConvertIsoStringToLanguage( aNames.getConstArray()[j], '-' );
        const Any* pValue = aValues.getConstArray();
        for( i = 0; i < aValues.getLength(); i++, pValue++ )
        {
            if( pValue->getValueTypeClass() == TypeClass_STRING )
            {
                const OUString* pLine = (const OUString*)pValue->getValue();
                if( pLine->getLength() )
                    m_aDefaults[ nLanguageType ][ getKeyType( aKeys.getConstArray()[i] ) ] = *pLine;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "   \"%s\"=\"%.30s\"\n",
                         OUStringToOString( aKeys.getConstArray()[i], RTL_TEXTENCODING_ASCII_US ).getStr(),
                         OUStringToOString( *pLine, RTL_TEXTENCODING_ASCII_US ).getStr()
                         );
#endif
            }
        }
    }
#ifdef DEBUG_TIMING
    osl_getSystemTime( &aStop );
    double fElapse = (double)aStop.Seconds + 1e-9*(double)aStop.Nanosec;
    fElapse -= (double)aStart.Seconds + 1e-9*(double)aStart.Nanosec;
    fprintf( stderr, "DefaultFontConfigItem::getValues needed %g seconds\n", fElapse );
    fprintf( stderr, "time spent in configuration: %g seconds (%g%%)\n", fConfigTime, fConfigTime/fElapse*100.0 );
#endif
}

/*
 *  DefaultFontConfigItem::getDefaultFont
 */

const OUString& DefaultFontConfigItem::getDefaultFont( int nLanguage, int nType ) const
{
    ::std::map< int, ::std::map< int, OUString > >::const_iterator lang = m_aDefaults.find( nLanguage );
    if( lang == m_aDefaults.end() || lang->second.find( nType ) == lang->second.end() )
    {
        String aLang, aCountry;
        ConvertLanguageToIsoNames( nLanguage, aLang, aCountry );
        nLanguage = ConvertIsoStringToLanguage( aLang );
        lang = m_aDefaults.find( nLanguage );
        if( lang == m_aDefaults.end() || lang->second.find( nType ) == lang->second.end() )
        {
            nLanguage = LANGUAGE_ENGLISH;
            lang = m_aDefaults.find( nLanguage );
            if( lang == m_aDefaults.end() || lang->second.find( nType ) == lang->second.end() )
            {
                static OUString aEmpty;
                return aEmpty;
            }
        }
    }
    return lang->second.find(nType)->second;
}

/*
 *  DefaultFontConfigItem::getUserInterfaceFont
 */

const OUString& DefaultFontConfigItem::getUserInterfaceFont( int nLanguage ) const
{
    if( nLanguage == LANGUAGE_SYSTEM )
        nLanguage = Application::GetSettings().GetUILanguage();
    const OUString& aUIFont = getDefaultFont( nLanguage, DEFAULTFONT_UI_SANS );

    if( aUIFont.getLength() )
        return aUIFont;

    // fallback mechanism (either no configuration or no entry in configuration

    #define FALLBACKFONT_UI_SANS "Andale Sans UI;Tahoma;Arial Unicode MS;Arial;Bitstream Vera Sans;gnu-unifont;Interface User;Geneva;WarpSans;Dialog;Swiss;Lucida;Helvetica;Charcoal;Chicago;MS Sans Serif;Helv;Times;Times New Roman;Interface System"
    #define FALLBACKFONT_UI_SANS_LATIN2 "Andale Sans UI;Tahoma;Arial Unicode MS;Arial;Luxi Sans;Bitstream Vera Sans;Interface User;Geneva;WarpSans;Dialog;Swiss;Lucida;Helvetica;Charcoal;Chicago;MS Sans Serif;Helv;Times;Times New Roman;Interface System"
    #define FALLBACKFONT_UI_SANS_ARABIC "Tahoma;Traditional Arabic;Simplified Arabic;Lucidasans;Lucida Sans;Supplement;Andale Sans UI;clearlyU;Interface User;Arial Unicode MS;Lucida Sans Unicode;WarpSans;Geneva;MS Sans Serif;Helv;Dialog;Albany;Lucida;Helvetica;Charcoal;Chicago;Arial;Helmet;Interface System;Sans Serif"
    #define FALLBACKFONT_UI_SANS_THAI "OONaksit;Tahoma;Lucidasans;Arial Unicode MS"
    #define FALLBACKFONT_UI_SANS_KOREAN "SunGulim;Gulim;Roundgothic;Arial Unicode MS;Lucida Sans Unicode;Andale Sans UI;gnu-unifont"
   #define FALLBACKFONT_UI_SANS_JAPANESE1 "Andale Sans UI;HG Mincho Light J;"
   #define FALLBACKFONT_UI_SANS_JAPANESE2 "Kochi Gochi;Gothic"
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
   static sal_Unicode const aSunGulim[] = { 0xC36C, 0xAD74, 0xB9BC, 0, 0 };
   String aFallBackKoreanLocalized( aSunGulim );
   aFallBackKoreanLocalized += String(RTL_CONSTASCII_USTRINGPARAM( ";" ) );
   aFallBackKoreanLocalized += String(RTL_CONSTASCII_USTRINGPARAM( FALLBACKFONT_UI_SANS_KOREAN ) );
   static const OUString aFallBackKorean( aFallBackKoreanLocalized );

   // optimize font list for some locales, as long as Andale Sans UI does not support them
   switch( nLanguage )
   {
       case LANGUAGE_ARABIC:
       case LANGUAGE_ARABIC_SAUDI_ARABIA:
       case LANGUAGE_ARABIC_IRAQ:
       case LANGUAGE_ARABIC_EGYPT:
       case LANGUAGE_ARABIC_LIBYA:
       case LANGUAGE_ARABIC_ALGERIA:
       case LANGUAGE_ARABIC_MOROCCO:
       case LANGUAGE_ARABIC_TUNISIA:
       case LANGUAGE_ARABIC_OMAN:
       case LANGUAGE_ARABIC_YEMEN:
       case LANGUAGE_ARABIC_SYRIA:
       case LANGUAGE_ARABIC_JORDAN:
       case LANGUAGE_ARABIC_LEBANON:
       case LANGUAGE_ARABIC_KUWAIT:
       case LANGUAGE_ARABIC_UAE:
       case LANGUAGE_ARABIC_BAHRAIN:
       case LANGUAGE_ARABIC_QATAR:
       case LANGUAGE_HEBREW:
           return aFallBackArabic;
           break;
       case LANGUAGE_THAI:
           return aFallBackThai;
           break;
       case LANGUAGE_KOREAN:
       case LANGUAGE_KOREAN_JOHAB:
           return aFallBackKorean;
           break;
            // latin-2 (use luxi before vera)
       case LANGUAGE_CZECH:
       case LANGUAGE_HUNGARIAN:
       case LANGUAGE_POLISH:
       case LANGUAGE_ROMANIAN:
       case LANGUAGE_CROATIAN:
       case LANGUAGE_SLOVAK:
       case LANGUAGE_SLOVENIAN:
       case LANGUAGE_SORBIAN:
           return aFallbackLatin2;
           break;

       case LANGUAGE_CHINESE:
       case LANGUAGE_CHINESE_SIMPLIFIED:
            return aFallBackChineseSIM;
            break;
       case LANGUAGE_CHINESE_TRADITIONAL:
       case LANGUAGE_CHINESE_HONGKONG:
       case LANGUAGE_CHINESE_SINGAPORE:
       case LANGUAGE_CHINESE_MACAU:
           return aFallBackChineseTRD;
           break;
       case LANGUAGE_JAPANESE:
           return aFallBackJapanese;
           break;
       default:
           break;
   }

   return aFallback;
}

/*
 *  DefaultFontConfigItem::setDefaultFont
 */

void DefaultFontConfigItem::setDefaultFont( int nLanguage, int nType, const OUString& rFont )
{
    bool bModified = m_aDefaults[ nLanguage ][ nType ] != rFont;
    if( bModified )
    {
        m_aDefaults[ nLanguage ][ nType ] = rFont;
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

static BOOL ImplKillLeading( String& rName, const char* const* ppStr )
{
    while ( *ppStr )
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
            rName.Erase( 0, (xub_StrLen)(pNameStr-rName.GetBuffer()) );
            return TRUE;
        }

        ppStr++;
    }

    // special case for Baekmuk
    // TODO: allow non-ASCII KillLeading list
    const xub_Unicode* pNameStr = rName.GetBuffer();
    if( (pNameStr[0]==0xBC31) && (pNameStr[1]==0xBC35) && (pNameStr[2]==0x0020) )
    {
        rName.Erase( 0, 3 );
        return TRUE;
    }

    return FALSE;
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

void FontSubstConfigItem::getMapName( const String& rOrgName, String& rShortName, String& rFamilyName, FontWeight& rWeight, FontWidth& rWidth, unsigned long& rType )
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

struct StrictStringSort : public ::std::binary_function< const FontSubstConfigItem::FontNameAttr&, const FontSubstConfigItem::FontNameAttr&, bool >
{
    bool operator()( const FontSubstConfigItem::FontNameAttr& rLeft, const FontSubstConfigItem::FontNameAttr& rRight )
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

#ifdef DEBUG_TIMING
    TimeValue aStart;
    TimeValue aStop;
    TimeValue aTmpTV1, aTmpTV2;
    osl_getSystemTime( &aStart );

    double fConfigTime = 0.0;
    double fTmp;
#endif

    START_TIMING();
    Sequence< OUString > aLocales( GetNodeNames( OUString( ) ) );
    END_TIMING();
    for( int j = 0; j < aLocales.getLength(); j++ )
    {
        String aKeyName( aLocales.getConstArray()[j] );
        int nLanguageType = ConvertIsoStringToLanguage( aLocales.getConstArray()[j], '-' );
        START_TIMING();
        Sequence< OUString > aSubstFonts( GetNodeNames( aKeyName ) );
        END_TIMING();
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "reading %d font substitutions for locale %s\n",
                 aSubstFonts.getLength(),
                 ByteString( aKeyName, RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
#endif
        for( int i = 0; i < aSubstFonts.getLength(); i++ )
        {
            FontNameAttr aAttributes;
            aAttributes.Name = aSubstFonts.getConstArray()[i];

            String aFontKey( aKeyName );
            aFontKey.Append( '/' );
            // must use the wrapped name in config paths due to non-ASCII chars (eg korean font names) !!!
            String wrappedName = wrapConfigurationElementName( aAttributes.Name );
            aFontKey.Append( wrappedName );
            Sequence< OUString > aKeys( 7 );
            OUString* pKeys = aKeys.getArray();
            aFontKey.Append( '/' );
            pKeys[0] = aFontKey.Copy().AppendAscii( "SubstFonts" );
            pKeys[1] = aFontKey.Copy().AppendAscii( "SubstFontsMS" );
            pKeys[2] = aFontKey.Copy().AppendAscii( "SubstFontsPS" );
            pKeys[3] = aFontKey.Copy().AppendAscii( "SubstFontsHTML" );
            pKeys[4] = aFontKey.Copy().AppendAscii( "FontWidth" );
            pKeys[5] = aFontKey.Copy().AppendAscii( "FontWeight" );
            pKeys[6] = aFontKey.Copy().AppendAscii( "FontType" );
            START_TIMING();
            Sequence< Any > aValues( GetProperties( aKeys ) );
            END_TIMING();
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

            m_aSubstitutions[nLanguageType].push_back( aAttributes );
        }
        ::std::sort( m_aSubstitutions[nLanguageType].begin(), m_aSubstitutions[nLanguageType].end(), StrictStringSort() );
    }
#ifdef DEBUG_TIMING
    osl_getSystemTime( &aStop );
    double fElapse = (double)aStop.Seconds + 1e-9*(double)aStop.Nanosec;
    fElapse -= (double)aStart.Seconds + 1e-9*(double)aStart.Nanosec;
    fprintf( stderr, "FontSubstConfigItem::getValues needed %g seconds\n", fElapse );
    fprintf( stderr, "time spent in configuration: %g seconds (%g%%)\n", fConfigTime, fConfigTime/fElapse*100.0 );
#endif
}

const FontSubstConfigItem::FontNameAttr* FontSubstConfigItem::getSubstInfo( const String& rFontName, int nLanguage ) const
{
    if( !rFontName.Len() )
        return NULL;

    // search if a  (language dep.) replacement table for the given font exists
    // fallback is english
    String aSearchFont( rFontName );
    aSearchFont.ToLowerAscii();
    FontNameAttr aSearchAttr;
    aSearchAttr.Name = aSearchFont;
    for( int i = 0; i < 3; i++ )
    {
        ::std::map< int, ::std::vector< FontNameAttr > >::const_iterator lang = m_aSubstitutions.find( nLanguage );
        if( lang != m_aSubstitutions.end() )
        {
            // try to find an exact match
            // because the list is sorted this will also find fontnames of the form searchfontname*
            ::std::vector< FontNameAttr >::const_iterator it = ::std::lower_bound( lang->second.begin(), lang->second.end(), aSearchAttr, StrictStringSort() );
            if( it != lang->second.end() && aSearchFont.CompareTo( it->Name, aSearchFont.Len() ) == COMPARE_EQUAL )
                return &(*it);
        }
        switch( i )
        {
            case 0:
            {
                String aLang, aCountry;
                ConvertLanguageToIsoNames( nLanguage, aLang, aCountry );
                int nNewLanguage = ConvertIsoStringToLanguage( aLang );
                if( nNewLanguage == nLanguage || nNewLanguage == LANGUAGE_DONTKNOW || nNewLanguage == LANGUAGE_NONE )
                    nNewLanguage = LANGUAGE_ENGLISH;
                nLanguage = nNewLanguage;
            }
            break;
            case 1:
                nLanguage = LANGUAGE_ENGLISH;
                break;
        }
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

    ::std::hash_map< OUString, ::std::hash_map< OUString, OUString, OUStringHash >, OUStringHash >::const_iterator group;

    for( group = m_aSettings.begin(); group != m_aSettings.end(); ++group )
    {
        String aKeyName( group->first );
        sal_Bool bAdded = AddNode( OUString(), aKeyName );
        Sequence< PropertyValue > aValues( group->second.size() );
        PropertyValue* pValues = aValues.getArray();
        int nIndex = 0;
        ::std::hash_map< OUString, OUString, OUStringHash >::const_iterator it;
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
#if OSL_DEBUG_LEVEL > 1
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
#if OSL_DEBUG_LEVEL > 1
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

