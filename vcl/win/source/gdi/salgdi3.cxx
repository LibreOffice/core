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

#include <sal/types.h>
#include <config_folders.h>

#include <algorithm>
#include <malloc.h>
#include <map>
#include <set>
#include <string.h>
#include <svsys.h>
#include <vector>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <unotools/fontcfg.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/metric.hxx>

#include "fontsubset.hxx"
#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "sft.hxx"
#include "win/saldata.hxx"
#include "win/salgdi.h"

using namespace vcl;

static const int MAXFONTHEIGHT = 2048;

// - Inlines -

inline FIXED FixedFromDouble( double d )
{
    const long l = (long) ( d * 65536. );
    return *(FIXED*) &l;
}

inline int IntTimes256FromFixed(FIXED f)
{
    int nFixedTimes256 = (f.value << 8) + ((f.fract+0x80) >> 8);
    return nFixedTimes256;
}

// these variables can be static because they store system wide settings
static bool bImplSalCourierScalable = false;
static bool bImplSalCourierNew = false;

// TODO: also support temporary TTC font files
typedef std::map< OUString, ImplDevFontAttributes > FontAttrMap;

class ImplFontAttrCache
{
private:
    FontAttrMap     aFontAttributes;
    OUString        aCacheFileName;
    OUString        aBaseURL;
    bool        bModified;

protected:
    OUString OptimizeURL( const OUString& rURL ) const;

    enum{ MAGIC = 0x12349876 }; // change if fontattrcache format changes

public:
            ImplFontAttrCache(const OUString& rCacheFileName,
                              const OUString& rBaseURL);
            ~ImplFontAttrCache();

    ImplDevFontAttributes  GetFontAttr( const OUString& rFontFileName ) const;
    void                   AddFontAttr( const OUString& rFontFileName, const ImplDevFontAttributes& );
};

ImplFontAttrCache::ImplFontAttrCache( const OUString& rFileNameURL, const OUString& rBaseURL ) : aBaseURL( rBaseURL )
{
    bModified = FALSE;
    aBaseURL = aBaseURL.toAsciiLowerCase();    // Windows only, no problem...

    // open the cache file
    osl::FileBase::getSystemPathFromFileURL( rFileNameURL, aCacheFileName );
    SvFileStream aCacheFile( aCacheFileName, StreamMode::READ );
    if( !aCacheFile.IsOpen() )
        return;

    // check the cache version
    sal_uInt32 nCacheMagic;
    aCacheFile.ReadUInt32(nCacheMagic);
    if( nCacheMagic != ImplFontAttrCache::MAGIC )
        return;  // ignore cache and rewrite if no match

    // read the cache entries from the file
    OUString aFontFileURL;
    ImplDevFontAttributes aDFA;
    for(;;)
    {
        aFontFileURL = read_uInt16_lenPrefixed_uInt8s_ToOUString(aCacheFile, RTL_TEXTENCODING_UTF8);
        if( aFontFileURL.isEmpty() )
            break;
        aDFA.SetFamilyName(read_uInt16_lenPrefixed_uInt8s_ToOUString(aCacheFile, RTL_TEXTENCODING_UTF8));

        short n;
        aCacheFile.ReadInt16(n);
        aDFA.SetWeight(static_cast<FontWeight>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetItalic(static_cast<FontItalic>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetPitch(static_cast<FontPitch>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetWidthType(static_cast<FontWidth>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetFamilyType(static_cast<FontFamily>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetSymbolFlag(n != 0);

        OUString styleName;
        aCacheFile.ReadByteStringLine( styleName, RTL_TEXTENCODING_UTF8 );
        aDFA.SetStyleName( styleName );

        aFontAttributes[ aFontFileURL ] = aDFA;
    }
}

ImplFontAttrCache::~ImplFontAttrCache()
{
    if ( bModified )
    {
        SvFileStream aCacheFile( aCacheFileName, StreamMode::WRITE|StreamMode::TRUNC );
        if ( aCacheFile.IsWritable() )
        {
            sal_uInt32 nCacheMagic = ImplFontAttrCache::MAGIC;
            aCacheFile.WriteUInt32( nCacheMagic );

            // write the cache entries to the file
            FontAttrMap::const_iterator aIter = aFontAttributes.begin();
            while ( aIter != aFontAttributes.end() )
            {
                const OUString rFontFileURL( (*aIter).first );
                const ImplDevFontAttributes& rDFA( (*aIter).second );
                write_uInt16_lenPrefixed_uInt8s_FromOUString(aCacheFile, rFontFileURL, RTL_TEXTENCODING_UTF8);
                write_uInt16_lenPrefixed_uInt8s_FromOUString(aCacheFile, rDFA.GetFamilyName(), RTL_TEXTENCODING_UTF8);

                aCacheFile.WriteInt16(rDFA.GetWeight());
                aCacheFile.WriteInt16(rDFA.GetSlant());
                aCacheFile.WriteInt16(rDFA.GetPitch());
                aCacheFile.WriteInt16(rDFA.GetWidthType());
                aCacheFile.WriteInt16(rDFA.GetFamilyType());
                aCacheFile.WriteInt16(rDFA.IsSymbolFont());

                write_uInt16_lenPrefixed_uInt8s_FromOUString(aCacheFile, rDFA.GetStyleName(), RTL_TEXTENCODING_UTF8);

                ++aIter;
            }
            // EOF Marker
            write_uInt16_lenPrefixed_uInt8s_FromOString(aCacheFile, OString());
        }
    }
}

OUString ImplFontAttrCache::OptimizeURL( const OUString& rURL ) const
{
    OUString aOptimizedFontFileURL( rURL.toAsciiLowerCase() );
    if ( aOptimizedFontFileURL.startsWith( aBaseURL ) )
        aOptimizedFontFileURL = aOptimizedFontFileURL.copy( aBaseURL.getLength() );
    return aOptimizedFontFileURL;
}

ImplDevFontAttributes ImplFontAttrCache::GetFontAttr( const OUString& rFontFileName ) const
{
    ImplDevFontAttributes aDFA;
    FontAttrMap::const_iterator it = aFontAttributes.find( OptimizeURL( rFontFileName ) );
    if( it != aFontAttributes.end() )
    {
        aDFA = it->second;
    }
    return aDFA;
}

void ImplFontAttrCache::AddFontAttr( const OUString& rFontFileName, const ImplDevFontAttributes& rDFA )
{
    SAL_WARN_IF(rFontFileName.isEmpty() || rDFA.GetFamilyName().isEmpty(),
        "vcl.gdi", "ImplFontNameCache::AddFontName - invalid data!");
    if ( !rFontFileName.isEmpty() && !rDFA.GetFamilyName().isEmpty() )
    {
        aFontAttributes.insert( FontAttrMap::value_type( OptimizeURL( rFontFileName ), rDFA ) );
        bModified = TRUE;
    }
}

// raw font data with a scoped lifetime
class RawFontData
{
public:
    explicit    RawFontData( HDC, DWORD nTableTag=0 );
                ~RawFontData() { delete[] mpRawBytes; }
    const unsigned char*    get() const { return mpRawBytes; }
    const unsigned char*    steal() { unsigned char* p = mpRawBytes; mpRawBytes = NULL; return p; }
    int               size() const { return mnByteCount; }

private:
    unsigned char*  mpRawBytes;
    unsigned        mnByteCount;
};

RawFontData::RawFontData( HDC hDC, DWORD nTableTag )
:   mpRawBytes( NULL )
,   mnByteCount( 0 )
{
    // get required size in bytes
    mnByteCount = ::GetFontData( hDC, nTableTag, 0, NULL, 0 );
    if( mnByteCount == GDI_ERROR )
        return;
    else if( !mnByteCount )
        return;

    // allocate the array
    mpRawBytes = new unsigned char[ mnByteCount ];

    // get raw data in chunks small enough for GetFontData()
    unsigned nRawDataOfs = 0;
    DWORD nMaxChunkSize = 0x100000;
    for(;;)
    {
        // calculate remaining raw data to get
        DWORD nFDGet = mnByteCount - nRawDataOfs;
        if( nFDGet <= 0 )
            break;
        // #i56745# limit GetFontData requests
        if( nFDGet > nMaxChunkSize )
            nFDGet = nMaxChunkSize;
        const DWORD nFDGot = ::GetFontData( hDC, nTableTag, nRawDataOfs,
            (void*)(mpRawBytes + nRawDataOfs), nFDGet );
        if( !nFDGot )
            break;
        else if( nFDGot != GDI_ERROR )
            nRawDataOfs += nFDGot;
        else
        {
            // was the chunk too big? reduce it
            nMaxChunkSize /= 2;
            if( nMaxChunkSize < 0x10000 )
                break;
        }
    }

    // cleanup if the raw data is incomplete
    if( nRawDataOfs != mnByteCount )
    {
        delete[] mpRawBytes;
        mpRawBytes = NULL;
    }
}

// platform specific font substitution hooks for glyph fallback enhancement
// TODO: move into i18n module (maybe merge with svx/ucsubset.*
//       or merge with i18nutil/source/utility/unicode_data.h)
struct Unicode2LangType
{
    sal_UCS4 mnMinCode;
    sal_UCS4 mnMaxCode;
    LanguageType mnLangID;
};

// entries marked with default-CJK get replaced with the default-CJK language
#define LANGUAGE_DEFAULT_CJK 0xFFF0

// map unicode ranges to languages supported by OOo
// NOTE: due to the binary search used this list must be sorted by mnMinCode
static Unicode2LangType aLangFromCodeChart[]= {
    {0x0000, 0x007F, LANGUAGE_ENGLISH},             // Basic Latin
    {0x0080, 0x024F, LANGUAGE_ENGLISH},             // Latin Extended-A and Latin Extended-B
    {0x0250, 0x02AF, LANGUAGE_SYSTEM},              // IPA Extensions
    {0x0370, 0x03FF, LANGUAGE_GREEK},               // Greek
    {0x0590, 0x05FF, LANGUAGE_HEBREW},              // Hebrew
    {0x0600, 0x06FF, LANGUAGE_ARABIC_PRIMARY_ONLY}, // Arabic
    {0x0900, 0x097F, LANGUAGE_HINDI},               // Devanagari
    {0x0980, 0x09FF, LANGUAGE_BENGALI},             // Bengali
    {0x0A80, 0x0AFF, LANGUAGE_GUJARATI},            // Gujarati
    {0x0B00, 0x0B7F, LANGUAGE_ODIA},                // Odia
    {0x0B80, 0x0BFF, LANGUAGE_TAMIL},               // Tamil
    {0x0C00, 0x0C7F, LANGUAGE_TELUGU},              // Telugu
    {0x0C80, 0x0CFF, LANGUAGE_KANNADA},             // Kannada
    {0x0D00, 0x0D7F, LANGUAGE_MALAYALAM},           // Malayalam
    {0x0D80, 0x0D7F, LANGUAGE_SINHALESE_SRI_LANKA}, // Sinhala
    {0x0E00, 0x0E7F, LANGUAGE_THAI},                // Thai
    {0x0E80, 0x0EFF, LANGUAGE_LAO},                 // Lao
    {0x0F00, 0x0FFF, LANGUAGE_TIBETAN},             // Tibetan
    {0x1000, 0x109F, LANGUAGE_BURMESE},             // Burmese
    {0x10A0, 0x10FF, LANGUAGE_GEORGIAN},            // Georgian
    {0x1100, 0x11FF, LANGUAGE_KOREAN},              // Hangul Jamo, Korean-specific
//  {0x1200, 0x139F, LANGUAGE_AMHARIC_ETHIOPIA},    // Ethiopic
//  {0x1200, 0x139F, LANGUAGE_TIGRIGNA_ETHIOPIA},   // Ethiopic
    {0x13A0, 0x13FF, LANGUAGE_CHEROKEE_UNITED_STATES}, // Cherokee
//  {0x1400, 0x167F, LANGUAGE_CANADIAN_ABORIGINAL}, // Canadian Aboriginial Syllabics
//  {0x1680, 0x169F, LANGUAGE_OGHAM},               // Ogham
//  {0x16A0, 0x16F0, LANGUAGE_RUNIC},               // Runic
//  {0x1700, 0x171F, LANGUAGE_TAGALOG},             // Tagalog
//  {0x1720, 0x173F, LANGUAGE_HANUNOO},             // Hanunoo
//  {0x1740, 0x175F, LANGUAGE_BUHID},               // Buhid
//  {0x1760, 0x177F, LANGUAGE_TAGBANWA},            // Tagbanwa
    {0x1780, 0x17FF, LANGUAGE_KHMER},               // Khmer
    {0x18A0, 0x18AF, LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA}, // Mongolian
//  {0x1900, 0x194F, LANGUAGE_LIMBU},               // Limbu
//  {0x1950, 0x197F, LANGUAGE_TAILE},               // Tai Le
//  {0x1980, 0x19DF, LANGUAGE_TAILUE},              // Tai Lue
    {0x19E0, 0x19FF, LANGUAGE_KHMER},               // Khmer Symbols
//  {0x1A00, 0x1A1F, LANGUAGE_BUGINESE},            // Buginese/Lontara
//  {0x1B00, 0x1B7F, LANGUAGE_BALINESE},            // Balinese
//  {0x1D00, 0x1DFF, LANGUAGE_NONE},                // Phonetic Symbols
    {0x1E00, 0x1EFF, LANGUAGE_ENGLISH},             // Latin Extended Additional
    {0x1F00, 0x1FFF, LANGUAGE_GREEK},               // Greek Extended
    {0x2C60, 0x2C7F, LANGUAGE_ENGLISH},             // Latin Extended-C
    {0x2E80, 0x2FFf, LANGUAGE_CHINESE_SIMPLIFIED},  // CJK Radicals Supplement + Kangxi Radical + Ideographic Description Characters
    {0x3000, 0x303F, LANGUAGE_DEFAULT_CJK},         // CJK Symbols and punctuation
    {0x3040, 0x30FF, LANGUAGE_JAPANESE},            // Japanese Hiragana + Katakana
    {0x3100, 0x312F, LANGUAGE_CHINESE_TRADITIONAL}, // Bopomofo
    {0x3130, 0x318F, LANGUAGE_KOREAN},              // Hangul Compatibility Jamo, Kocrean-specific
    {0x3190, 0x319F, LANGUAGE_JAPANESE},            // Kanbun
    {0x31A0, 0x31BF, LANGUAGE_CHINESE_TRADITIONAL}, // Bopomofo Extended
    {0x31C0, 0x31EF, LANGUAGE_DEFAULT_CJK},         // CJK Ideographs
    {0x31F0, 0x31FF, LANGUAGE_JAPANESE},            // Japanese Katakana Phonetic Extensions
    {0x3200, 0x321F, LANGUAGE_KOREAN},              // Parenthesized Hangul
    {0x3220, 0x325F, LANGUAGE_DEFAULT_CJK},         // Parenthesized Ideographs
    {0x3260, 0x327F, LANGUAGE_KOREAN},              // Circled Hangul
    {0x3280, 0x32CF, LANGUAGE_DEFAULT_CJK},         // Circled Ideographs
    {0x32d0, 0x32FF, LANGUAGE_JAPANESE},            // Japanese Circled Katakana
    {0x3400, 0x4DBF, LANGUAGE_DEFAULT_CJK},         // CJK Unified Ideographs Extension A
    {0x4E00, 0x9FCF, LANGUAGE_DEFAULT_CJK},         // Unified CJK Ideographs
    {0xA720, 0xA7FF, LANGUAGE_ENGLISH},             // Latin Extended-D
    {0xAC00, 0xD7AF, LANGUAGE_KOREAN},              // Hangul Syllables, Korean-specific
    {0xF900, 0xFAFF, LANGUAGE_DEFAULT_CJK},         // CJK Compatibility Ideographs
    {0xFB00, 0xFB4F, LANGUAGE_HEBREW},              // Hebrew Presentation Forms
    {0xFB50, 0xFDFF, LANGUAGE_ARABIC_PRIMARY_ONLY}, // Arabic Presentation Forms-A
    {0xFE70, 0xFEFE, LANGUAGE_ARABIC_PRIMARY_ONLY}, // Arabic Presentation Forms-B
    {0xFF65, 0xFF9F, LANGUAGE_JAPANESE},            // Japanese Halfwidth Katakana variant
    {0xFFA0, 0xFFDC, LANGUAGE_KOREAN},              // Kocrean halfwidth hangual variant
    {0x10140, 0x1018F, LANGUAGE_GREEK},             // Ancient Greak numbers
    {0x1D200, 0x1D24F, LANGUAGE_GREEK},             // Ancient Greek Musical
    {0x20000, 0x2A6DF, LANGUAGE_DEFAULT_CJK},       // CJK Unified Ideographs Extension B
    {0x2F800, 0x2FA1F, LANGUAGE_DEFAULT_CJK}        // CJK Compatibility Ideographs Supplement
};

// get language matching to the missing char
LanguageType MapCharToLanguage( sal_UCS4 uChar )
{
    // entries marked with default-CJK get replaced with the preferred CJK language
    static bool bFirst = true;
    if( bFirst )
    {
        bFirst = false;

        // use method suggested in #i97086# to determnine the systems default language
        // TODO: move into i18npool or sal/osl/w32/nlsupport.c
        LanguageType nDefaultLang = 0;
        HKEY hKey = NULL;
        LONG lResult = ::RegOpenKeyExA( HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Nls\\Language",
            0, KEY_QUERY_VALUE, &hKey );
        char aKeyValBuf[16];
        DWORD nKeyValSize = sizeof(aKeyValBuf);
        if( ERROR_SUCCESS == lResult )
            lResult = RegQueryValueExA( hKey, "Default", NULL, NULL, (LPBYTE)aKeyValBuf, &nKeyValSize );
        aKeyValBuf[ sizeof(aKeyValBuf)-1 ] = '\0';
        if( ERROR_SUCCESS == lResult )
            nDefaultLang = (LanguageType)rtl_str_toInt32( aKeyValBuf, 16 );

        // TODO: use the default-CJK language selected in
        //  Tools->Options->LangSettings->Languages when it becomes available here
        if( !nDefaultLang )
            nDefaultLang = Application::GetSettings().GetUILanguageTag().getLanguageType();

        LanguageType nDefaultCJK = MsLangId::isCJK(nDefaultLang) ? nDefaultLang : LANGUAGE_CHINESE;

        // change the marked entries to preferred language
        static const int nCount = SAL_N_ELEMENTS(aLangFromCodeChart);
        for( int i = 0; i < nCount; ++i )
        {
            if( aLangFromCodeChart[ i].mnLangID == LANGUAGE_DEFAULT_CJK )
                aLangFromCodeChart[ i].mnLangID = nDefaultCJK;
        }
    }

    // binary search
    int nLow = 0;
    int nHigh = SAL_N_ELEMENTS(aLangFromCodeChart) - 1;
    while( nLow <= nHigh )
    {
        int nMiddle = (nHigh + nLow) / 2;
        if( uChar < aLangFromCodeChart[ nMiddle].mnMinCode )
            nHigh = nMiddle - 1;
        else if( uChar > aLangFromCodeChart[ nMiddle].mnMaxCode )
            nLow = nMiddle + 1;
        else
            return aLangFromCodeChart[ nMiddle].mnLangID;
    }

    return LANGUAGE_DONTKNOW;
}

class WinGlyphFallbackSubstititution
:    public ImplGlyphFallbackFontSubstitution
{
public:
    explicit    WinGlyphFallbackSubstititution( HDC );

    bool FindFontSubstitute( FontSelectPattern&, OUString& rMissingChars ) const;
private:
    HDC mhDC;
    bool HasMissingChars( PhysicalFontFace*, const OUString& rMissingChars ) const;
};

inline WinGlyphFallbackSubstititution::WinGlyphFallbackSubstititution( HDC hDC )
:   mhDC( hDC )
{}

void ImplGetLogFontFromFontSelect( HDC, const FontSelectPattern*,
    LOGFONTW&, bool /*bTestVerticalAvail*/ );

// does a font face hold the given missing characters?
bool WinGlyphFallbackSubstititution::HasMissingChars( PhysicalFontFace* pFace, const OUString& rMissingChars ) const
{
    ImplWinFontData* pWinFont = static_cast< ImplWinFontData* >(pFace);
    FontCharMapPtr pCharMap = pWinFont->GetFontCharMap();
    if( !pCharMap )
    {
        // construct a Size structure as the parameter of constructor of class FontSelectPattern
        const Size aSize( pFace->GetWidth(), pFace->GetHeight() );
        // create a FontSelectPattern object for getting s LOGFONT
        const FontSelectPattern aFSD( *pFace, aSize, (float)aSize.Height(), 0, false );
        // construct log font
        LOGFONTW aLogFont;
        ImplGetLogFontFromFontSelect( mhDC, &aFSD, aLogFont, true );

        // create HFONT from log font
        HFONT hNewFont = ::CreateFontIndirectW( &aLogFont );
        // select the new font into device
        HFONT hOldFont = ::SelectFont( mhDC, hNewFont );

        // read CMAP table to update their pCharMap
        pWinFont->UpdateFromHDC( mhDC );

        // cleanup temporary font
        ::SelectFont( mhDC, hOldFont );
        ::DeleteFont( hNewFont );

        // get the new charmap
        pCharMap = pWinFont->GetFontCharMap();
    }

    // avoid fonts with unknown CMAP subtables for glyph fallback
    if( !pCharMap || pCharMap->IsDefaultMap() )
        return false;

    int nMatchCount = 0;
    // static const int nMaxMatchCount = 1; // TODO: tolerate more missing characters?
    const sal_Int32 nStrLen = rMissingChars.getLength();
    for( sal_Int32 nStrIdx = 0; nStrIdx < nStrLen; /* ++nStrIdx unreachable code, see the 'break' below */ )
    {
        const sal_UCS4 uChar = rMissingChars.iterateCodePoints( &nStrIdx );
        nMatchCount += pCharMap->HasChar( uChar );
        break; // for now
    }

    pCharMap = 0;

    const bool bHasMatches = (nMatchCount > 0);
    return bHasMatches;
}

namespace
{
    //used by 2-level font fallback
    PhysicalFontFamily* findDevFontListByLocale(const PhysicalFontCollection &rFontCollection,
                                                const LanguageTag& rLanguageTag )
    {
        // get the default font for a specified locale
        const utl::DefaultFontConfiguration& rDefaults = utl::DefaultFontConfiguration::get();
        const OUString aDefault = rDefaults.getUserInterfaceFont(rLanguageTag);
        return rFontCollection.ImplFindByTokenNames(aDefault);
    }
}

// find a fallback font for missing characters
// TODO: should stylistic matches be searched and preferred?
bool WinGlyphFallbackSubstititution::FindFontSubstitute( FontSelectPattern& rFontSelData, OUString& rMissingChars ) const
{
    // guess a locale matching to the missing chars
    LanguageType eLang = LANGUAGE_DONTKNOW;
    LanguageTag aLanguageTag( eLang);

    sal_Int32 nStrIdx = 0;
    const sal_Int32 nStrLen = rMissingChars.getLength();
    while( nStrIdx < nStrLen )
    {
        const sal_UCS4 uChar = rMissingChars.iterateCodePoints( &nStrIdx );
        eLang = MapCharToLanguage( uChar );
        if( eLang == LANGUAGE_DONTKNOW )
            continue;
        aLanguageTag.reset( eLang);
        break;
    }

    // fall back to default UI locale if the missing characters are inconclusive
    if( eLang == LANGUAGE_DONTKNOW )
        aLanguageTag = Application::GetSettings().GetUILanguageTag();

    // first level fallback:
    // try use the locale specific default fonts defined in VCL.xcu
    const PhysicalFontCollection* pFontCollection = ImplGetSVData()->maGDIData.mpScreenFontList;
    PhysicalFontFamily* pFontFamily = findDevFontListByLocale(*pFontCollection, aLanguageTag);
    if( pFontFamily )
    {
        PhysicalFontFace* pFace = pFontFamily->FindBestFontFace( rFontSelData );
        if( HasMissingChars( pFace, rMissingChars ) )
        {
            rFontSelData.maSearchName = pFontFamily->GetSearchName();
            return true;
        }
    }

    // are the missing characters symbols?
    pFontFamily = pFontCollection->ImplFindByAttributes( ImplFontAttrs::Symbol,
                                                         rFontSelData.GetWeight(),
                                                         rFontSelData.GetWidthType(),
                                                         rFontSelData.GetSlant(),
                                                         rFontSelData.maSearchName );
    if( pFontFamily )
    {
        PhysicalFontFace* pFace = pFontFamily->FindBestFontFace( rFontSelData );
        if( HasMissingChars( pFace, rMissingChars ) )
        {
            rFontSelData.maSearchName = pFontFamily->GetSearchName();
            return true;
        }
    }

    // last level fallback, check each font type face one by one
    ImplGetDevFontList* pTestFontList = pFontCollection->GetDevFontList();
    // limit the count of fonts to be checked to prevent hangs
    static const int MAX_GFBFONT_COUNT = 600;
    int nTestFontCount = pTestFontList->Count();
    if( nTestFontCount > MAX_GFBFONT_COUNT )
        nTestFontCount = MAX_GFBFONT_COUNT;

    bool bFound = false;
    for( int i = 0; i < nTestFontCount; ++i )
    {
        PhysicalFontFace* pFace = pTestFontList->Get( i );
        bFound = HasMissingChars( pFace, rMissingChars );
        if( !bFound )
            continue;
        rFontSelData.maSearchName = pFace->GetFamilyName();
        break;
    }

    delete pTestFontList;

    return bFound;
}

struct ImplEnumInfo
{
    HDC                 mhDC;
    PhysicalFontCollection* mpList;
    OUString*           mpName;
    LOGFONTA*           mpLogFontA;
    LOGFONTW*           mpLogFontW;
    UINT                mnPreferredCharSet;
    bool                mbCourier;
    bool                mbImplSalCourierScalable;
    bool                mbImplSalCourierNew;
    bool                mbPrinter;
    int                 mnFontCount;
};

static rtl_TextEncoding ImplCharSetToSal( BYTE nCharSet )
{
    rtl_TextEncoding eTextEncoding;

    if ( nCharSet == OEM_CHARSET )
    {
        UINT nCP = (sal_uInt16)GetOEMCP();
        switch ( nCP )
        {
            // It is unclear why these two (undefined?) code page numbers are
            // handled specially here:
            case 1004:  eTextEncoding = RTL_TEXTENCODING_MS_1252; break;
            case 65400: eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;
            default:
                eTextEncoding = rtl_getTextEncodingFromWindowsCodePage(nCP);
                break;
        }
    }
    else
    {
        if( nCharSet )
            eTextEncoding = rtl_getTextEncodingFromWindowsCharset( nCharSet );
        else
            eTextEncoding = RTL_TEXTENCODING_UNICODE;
    }

    return eTextEncoding;
}

static FontFamily ImplFamilyToSal( BYTE nFamily )
{
    switch ( nFamily & 0xF0 )
    {
        case FF_DECORATIVE:
            return FAMILY_DECORATIVE;

        case FF_MODERN:
            return FAMILY_MODERN;

        case FF_ROMAN:
            return FAMILY_ROMAN;

        case FF_SCRIPT:
            return FAMILY_SCRIPT;

        case FF_SWISS:
            return FAMILY_SWISS;

        default:
            break;
    }

    return FAMILY_DONTKNOW;
}

static BYTE ImplFamilyToWin( FontFamily eFamily )
{
    switch ( eFamily )
    {
        case FAMILY_DECORATIVE:
            return FF_DECORATIVE;

        case FAMILY_MODERN:
            return FF_MODERN;

        case FAMILY_ROMAN:
            return FF_ROMAN;

        case FAMILY_SCRIPT:
            return FF_SCRIPT;

        case FAMILY_SWISS:
            return FF_SWISS;

        case FAMILY_SYSTEM:
            return FF_SWISS;

        default:
            break;
    }

    return FF_DONTCARE;
}

static FontWeight ImplWeightToSal( int nWeight )
{
    if ( nWeight <= FW_THIN )
        return WEIGHT_THIN;
    else if ( nWeight <= FW_ULTRALIGHT )
        return WEIGHT_ULTRALIGHT;
    else if ( nWeight <= FW_LIGHT )
        return WEIGHT_LIGHT;
    else if ( nWeight < FW_MEDIUM )
        return WEIGHT_NORMAL;
    else if ( nWeight == FW_MEDIUM )
        return WEIGHT_MEDIUM;
    else if ( nWeight <= FW_SEMIBOLD )
        return WEIGHT_SEMIBOLD;
    else if ( nWeight <= FW_BOLD )
        return WEIGHT_BOLD;
    else if ( nWeight <= FW_ULTRABOLD )
        return WEIGHT_ULTRABOLD;
    else
        return WEIGHT_BLACK;
}

static int ImplWeightToWin( FontWeight eWeight )
{
    switch ( eWeight )
    {
        case WEIGHT_THIN:
            return FW_THIN;

        case WEIGHT_ULTRALIGHT:
            return FW_ULTRALIGHT;

        case WEIGHT_LIGHT:
            return FW_LIGHT;

        case WEIGHT_SEMILIGHT:
        case WEIGHT_NORMAL:
            return FW_NORMAL;

        case WEIGHT_MEDIUM:
            return FW_MEDIUM;

        case WEIGHT_SEMIBOLD:
            return FW_SEMIBOLD;

        case WEIGHT_BOLD:
            return FW_BOLD;

        case WEIGHT_ULTRABOLD:
            return FW_ULTRABOLD;

        case WEIGHT_BLACK:
            return FW_BLACK;

        default:
            break;
    }

    return 0;
}

inline FontPitch ImplLogPitchToSal( BYTE nPitch )
{
    if ( nPitch & FIXED_PITCH )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}

inline FontPitch ImplMetricPitchToSal( BYTE nPitch )
{
    // Grrrr! See NT help
    if ( !(nPitch & TMPF_FIXED_PITCH) )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}

inline BYTE ImplPitchToWin( FontPitch ePitch )
{
    if ( ePitch == PITCH_FIXED )
        return FIXED_PITCH;
    else if ( ePitch == PITCH_VARIABLE )
        return VARIABLE_PITCH;
    else
        return DEFAULT_PITCH;
}

static ImplDevFontAttributes WinFont2DevFontAttributes( const ENUMLOGFONTEXA& rEnumFont,
    const NEWTEXTMETRICA& rMetric, DWORD nFontType )
{
    ImplDevFontAttributes aDFA;

    const LOGFONTA rLogFont = rEnumFont.elfLogFont;

    // get font face attributes
    aDFA.SetFamilyType(ImplFamilyToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetWidthType(WIDTH_DONTKNOW);
    aDFA.SetWeight(ImplWeightToSal( rLogFont.lfWeight ));
    aDFA.SetItalic((rLogFont.lfItalic) ? ITALIC_NORMAL : ITALIC_NONE);
    aDFA.SetPitch(ImplLogPitchToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetSymbolFlag(rLogFont.lfCharSet == SYMBOL_CHARSET);

    // get the font face name
    aDFA.SetFamilyName(ImplSalGetUniString( rLogFont.lfFaceName ));

    // use the face's style name only if it looks reasonable
    const char* pStyleName = (const char*)rEnumFont.elfStyle;
    const char* pEnd = pStyleName + sizeof( rEnumFont.elfStyle );
    const char* p = pStyleName;
    for(; *p && (p < pEnd); ++p )
        if( (0x00 < *p) && (*p < 0x20) )
            break;
    if( p < pEnd )
        aDFA.SetStyleName(ImplSalGetUniString( pStyleName ));

    // get device specific font attributes
    aDFA.mbOrientation  = (nFontType & RASTER_FONTTYPE) == 0;
    aDFA.mbDevice       = (rMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;

    aDFA.mbEmbeddable   = false;
    aDFA.mbSubsettable  = false;
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE))
     || 0 != (rMetric.tmPitchAndFamily & TMPF_TRUETYPE))
        aDFA.mbSubsettable = true;
    else if( 0 != (rMetric.ntmFlags & NTM_TYPE1) ) // TODO: implement subsetting for type1 too
        aDFA.mbEmbeddable = true;

    // heuristics for font quality
    // -   standard-type1 > opentypeTT > truetype > non-standard-type1 > raster
    // -   subsetting > embedding > none
    aDFA.mnQuality = 0;
    if( rMetric.tmPitchAndFamily & TMPF_TRUETYPE )
        aDFA.mnQuality += 50;
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE)) )
        aDFA.mnQuality += 10;
    if( aDFA.mbSubsettable )
        aDFA.mnQuality += 200;
    else if( aDFA.mbEmbeddable )
        aDFA.mnQuality += 100;

    // #i38665# prefer Type1 versions of the standard postscript fonts
    if( aDFA.mbEmbeddable )
    {
        if( aDFA.GetFamilyName() == "AvantGarde"
        ||  aDFA.GetFamilyName() == "Bookman"
        ||  aDFA.GetFamilyName() == "Courier"
        ||  aDFA.GetFamilyName() == "Helvetica"
        ||  aDFA.GetFamilyName() == "NewCenturySchlbk"
        ||  aDFA.GetFamilyName() == "Palatino"
        ||  aDFA.GetFamilyName() == "Symbol"
        ||  aDFA.GetFamilyName() == "Times"
        ||  aDFA.GetFamilyName() == "ZapfChancery"
        ||  aDFA.GetFamilyName() == "ZapfDingbats" )
            aDFA.mnQuality += 500;
    }

    // TODO: add alias names
    return aDFA;
}

static ImplDevFontAttributes WinFont2DevFontAttributes( const ENUMLOGFONTEXW& rEnumFont,
    const NEWTEXTMETRICW& rMetric, DWORD nFontType )
{
    ImplDevFontAttributes aDFA;

    const LOGFONTW rLogFont = rEnumFont.elfLogFont;

    // get font face attributes
    aDFA.SetFamilyType(ImplFamilyToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetWidthType(WIDTH_DONTKNOW);
    aDFA.SetWeight(ImplWeightToSal( rLogFont.lfWeight ));
    aDFA.SetItalic((rLogFont.lfItalic) ? ITALIC_NORMAL : ITALIC_NONE);
    aDFA.SetPitch(ImplLogPitchToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetSymbolFlag(rLogFont.lfCharSet == SYMBOL_CHARSET);

    // get the font face name
    aDFA.SetFamilyName(OUString(reinterpret_cast<const sal_Unicode*>(rLogFont.lfFaceName)));

    // use the face's style name only if it looks reasonable
    const wchar_t* pStyleName = rEnumFont.elfStyle;
    const wchar_t* pEnd = pStyleName + sizeof(rEnumFont.elfStyle)/sizeof(*rEnumFont.elfStyle);
    const wchar_t* p = pStyleName;
    for(; *p && (p < pEnd); ++p )
        if( *p < 0x0020 )
            break;
    if( p < pEnd )
        aDFA.SetStyleName(OUString(reinterpret_cast<const sal_Unicode*>(pStyleName)));

    // get device specific font attributes
    aDFA.mbOrientation  = (nFontType & RASTER_FONTTYPE) == 0;
    aDFA.mbDevice       = (rMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;

    aDFA.mbEmbeddable   = false;
    aDFA.mbSubsettable  = false;
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE))
     || 0 != (rMetric.tmPitchAndFamily & TMPF_TRUETYPE))
        aDFA.mbSubsettable = true;
    else if( 0 != (rMetric.ntmFlags & NTM_TYPE1) ) // TODO: implement subsetting for type1 too
        aDFA.mbEmbeddable = true;

    // heuristics for font quality
    // -   standard-type1 > opentypeTT > truetype > non-standard-type1 > raster
    // -   subsetting > embedding > none
    aDFA.mnQuality = 0;
    if( rMetric.tmPitchAndFamily & TMPF_TRUETYPE )
        aDFA.mnQuality += 50;
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE)) )
        aDFA.mnQuality += 10;
    if( aDFA.mbSubsettable )
        aDFA.mnQuality += 200;
    else if( aDFA.mbEmbeddable )
        aDFA.mnQuality += 100;

    // #i38665# prefer Type1 versions of the standard postscript fonts
    if( aDFA.mbEmbeddable )
    {
        if( aDFA.GetFamilyName() == "AvantGarde"
        ||  aDFA.GetFamilyName() == "Bookman"
        ||  aDFA.GetFamilyName() == "Courier"
        ||  aDFA.GetFamilyName() == "Helvetica"
        ||  aDFA.GetFamilyName() == "NewCenturySchlbk"
        ||  aDFA.GetFamilyName() == "Palatino"
        ||  aDFA.GetFamilyName() == "Symbol"
        ||  aDFA.GetFamilyName() == "Times"
        ||  aDFA.GetFamilyName() == "ZapfChancery"
        ||  aDFA.GetFamilyName() == "ZapfDingbats" )
            aDFA.mnQuality += 500;
    }

    // TODO: add alias names
    return aDFA;
}

static ImplWinFontData* ImplLogMetricToDevFontDataA( const ENUMLOGFONTEXA* pLogFont,
                                         const NEWTEXTMETRICA* pMetric,
                                         DWORD nFontType )
{
    int nHeight = 0;
    if ( nFontType & RASTER_FONTTYPE )
        nHeight = pMetric->tmHeight - pMetric->tmInternalLeading;

    ImplWinFontData* pData = new ImplWinFontData(
        WinFont2DevFontAttributes(*pLogFont, *pMetric, nFontType),
        nHeight,
        pLogFont->elfLogFont.lfCharSet,
        pMetric->tmPitchAndFamily );

    return pData;
}

static ImplWinFontData* ImplLogMetricToDevFontDataW( const ENUMLOGFONTEXW* pLogFont,
                                         const NEWTEXTMETRICW* pMetric,
                                         DWORD nFontType )
{
    int nHeight = 0;
    if ( nFontType & RASTER_FONTTYPE )
        nHeight = pMetric->tmHeight - pMetric->tmInternalLeading;

    ImplWinFontData* pData = new ImplWinFontData(
        WinFont2DevFontAttributes(*pLogFont, *pMetric, nFontType),
        nHeight,
        pLogFont->elfLogFont.lfCharSet,
        pMetric->tmPitchAndFamily );

    return pData;
}

void ImplSalLogFontToFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont )
{
    OUString aFontName( reinterpret_cast<const sal_Unicode*>(rLogFont.lfFaceName) );
    if (!aFontName.isEmpty())
    {
        rFont.SetName( aFontName );
        rFont.SetCharSet( ImplCharSetToSal( rLogFont.lfCharSet ) );
        rFont.SetFamily( ImplFamilyToSal( rLogFont.lfPitchAndFamily ) );
        rFont.SetPitch( ImplLogPitchToSal( rLogFont.lfPitchAndFamily ) );
        rFont.SetWeight( ImplWeightToSal( rLogFont.lfWeight ) );

        long nFontHeight = rLogFont.lfHeight;
        if ( nFontHeight < 0 )
            nFontHeight = -nFontHeight;
        long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );
        if( !nDPIY )
            nDPIY = 600;
        nFontHeight *= 72;
        nFontHeight += nDPIY/2;
        nFontHeight /= nDPIY;
        rFont.SetSize( Size( 0, nFontHeight ) );
        rFont.SetOrientation( (short)rLogFont.lfEscapement );
        if ( rLogFont.lfItalic )
            rFont.SetItalic( ITALIC_NORMAL );
        else
            rFont.SetItalic( ITALIC_NONE );
        if ( rLogFont.lfUnderline )
            rFont.SetUnderline( UNDERLINE_SINGLE );
        else
            rFont.SetUnderline( UNDERLINE_NONE );
        if ( rLogFont.lfStrikeOut )
            rFont.SetStrikeout( STRIKEOUT_SINGLE );
        else
            rFont.SetStrikeout( STRIKEOUT_NONE );
    }
}

#if ENABLE_GRAPHITE

#ifdef DEBUG
static FILE * grLogFile = NULL;
static FILE * grLog()
{
    std::string logFileName(getenv("TEMP"));
    logFileName.append("\\grface.log");
    if (grLogFile == NULL) grLogFile = fopen(logFileName.c_str(),"w");
    else fflush(grLogFile);
    return grLogFile;
}
#undef NDEBUG
#endif

const void * getGrTable(const void* appFaceHandle, unsigned int name, size_t *len)
{
    const GrFontData * fontTables = reinterpret_cast<const GrFontData*>(appFaceHandle);
    return fontTables->getTable(name, len);
}

GrFontData::GrFontData(HDC hDC) :
    mhDC(hDC), mpFace(NULL), mnRefCount(1)
{
    // The face options ensure that the tables are all read at construction
    // time so there is no need to keep the hDC uptodate
    static const char* pGraphiteCacheStr = getenv( "SAL_GRAPHITE_CACHE_SIZE" );
    unsigned long graphiteSegCacheSize = pGraphiteCacheStr ? (atoi(pGraphiteCacheStr)) : 0;
    if (graphiteSegCacheSize > 500)
        mpFace = gr_make_face_with_seg_cache(this, getGrTable,
            graphiteSegCacheSize, gr_face_preloadGlyphs | gr_face_cacheCmap);
    else
        mpFace = gr_make_face(this, getGrTable,
            gr_face_preloadGlyphs | gr_face_cacheCmap);
#ifdef DEBUG
        fprintf(grLog(), "gr_make_face %lx for WinFontData %lx\n", (unsigned long)mpFace,
            (unsigned long)this);
#endif
    mhDC = NULL;
}

GrFontData::~GrFontData()
{
    if (mpFace)
    {
#ifdef DEBUG
        fprintf(grLog(), "gr_face_destroy %lx for WinFontData %lx\n", (unsigned long)mpFace,
            (unsigned long)this);
#endif
        gr_face_destroy(mpFace);
        mpFace = NULL;
    }
    std::vector<RawFontData*>::iterator i = mvData.begin();
    while (i != mvData.end())
    {
        delete *i;
        ++i;
    }
    mvData.clear();
}

const void * GrFontData::getTable(unsigned int name, size_t *len) const
{
#ifdef DEBUG
#undef NDEBUG
#endif
    assert(mhDC);
    // swap the bytes
    union TtfTag {
        unsigned int i;
        unsigned char c[4];
    };
    TtfTag littleEndianTag;
    littleEndianTag.i = name;
    TtfTag bigEndianTag;
    bigEndianTag.c[0] = littleEndianTag.c[3];
    bigEndianTag.c[1] = littleEndianTag.c[2];
    bigEndianTag.c[2] = littleEndianTag.c[1];
    bigEndianTag.c[3] = littleEndianTag.c[0];
    mvData.push_back(new RawFontData(mhDC, bigEndianTag.i));
    const RawFontData * data = mvData[mvData.size()-1];
    if (data && (data->size() > 0))
    {
        if (len)
            *len = data->size();
        return reinterpret_cast<const void *>(data->get());
    }
    else
    {
        if (len)
            *len = 0;
        return NULL;
    }
}
#endif

ImplWinFontData::ImplWinFontData( const ImplDevFontAttributes& rDFS,
    int nHeight, BYTE eWinCharSet, BYTE nPitchAndFamily )
:   PhysicalFontFace( rDFS, 0 ),
    mnId( 0 ),
    mbHasKoreanRange( false ),
    mbHasCJKSupport( false ),
#if ENABLE_GRAPHITE
    mpGraphiteData(NULL),
    mbHasGraphiteSupport( false ),
#endif
    mbHasArabicSupport ( false ),
    mbFontCapabilitiesRead( false ),
    mpUnicodeMap( NULL ),
    mpEncodingVector( NULL ),
    meWinCharSet( eWinCharSet ),
    mnPitchAndFamily( nPitchAndFamily ),
    mbAliasSymbolsHigh( false ),
    mbAliasSymbolsLow( false ),
    mbGsubRead( false )
{
    SetBitmapSize( 0, nHeight );

    if( eWinCharSet == SYMBOL_CHARSET )
    {
        if( (nPitchAndFamily & TMPF_TRUETYPE) != 0 )
        {
            // truetype fonts need their symbols as U+F0xx
            mbAliasSymbolsHigh = true;
        }
        else if( (nPitchAndFamily & (TMPF_VECTOR|TMPF_DEVICE))
                                 == (TMPF_VECTOR|TMPF_DEVICE) )
        {
            // scalable device fonts (e.g. builtin printer fonts)
            // need their symbols as U+00xx
            mbAliasSymbolsLow  = true;
        }
        else if( (nPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE)) == 0 )
        {
            // bitmap fonts need their symbols as U+F0xx
            mbAliasSymbolsHigh = true;
        }
    }
#ifdef DEBUG
    fprintf(grLog(), "ImplWinFontData::ImplWinFontData() %lx\n", (unsigned long)this);
#endif
}

ImplWinFontData::~ImplWinFontData()
{
    if( mpUnicodeMap )
        mpUnicodeMap = 0;
#if ENABLE_GRAPHITE
    if (mpGraphiteData)
        mpGraphiteData->DeReference();
#ifdef DEBUG
    fprintf(grLog(), "ImplWinFontData::~ImplWinFontData %lx\n", (unsigned long)this);
#endif
#endif // ENABLE_GRAPHITE
    delete mpEncodingVector;
}

sal_IntPtr ImplWinFontData::GetFontId() const
{
    return mnId;
}

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static inline DWORD CalcTag( const char p[4]) { return (p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24)); }

void ImplWinFontData::UpdateFromHDC( HDC hDC ) const
{
    // short circuit if already initialized
    if( mpUnicodeMap != NULL )
        return;

    ReadCmapTable( hDC );
    GetFontCapabilities( hDC );
#if ENABLE_GRAPHITE
    static const char* pDisableGraphiteText = getenv( "SAL_DISABLE_GRAPHITE" );
    if( !pDisableGraphiteText || (pDisableGraphiteText[0] == '0') )
    {
        const DWORD nSilfTag = CalcTag("Silf");
        const RawFontData aRawFontData( hDC, nSilfTag );
        mbHasGraphiteSupport = (aRawFontData.size() > 0);
        if (mbHasGraphiteSupport)
        {
#ifdef DEBUG
            fprintf(grLog(), "ImplWinFontData::UpdateFromHDC %lx\n",
            (unsigned long)this);
#endif
            if (mpGraphiteData == NULL)
            {
                mpGraphiteData = new GrFontData(hDC);
                if (!mpGraphiteData->getFace())
                {
                    mbHasGraphiteSupport = false;
                    delete mpGraphiteData;
                    mpGraphiteData = NULL;
                }
            }
        }
    }
#endif
}

#if ENABLE_GRAPHITE
const gr_face* ImplWinFontData::GraphiteFace() const
{
#ifdef DEBUG
    fprintf(grLog(), "ImplWinFontData::GraphiteFace %lx has face %lx\n",
        (unsigned long)this, mpGraphiteData? mpGraphiteData->getFace(): 0);
#endif
    return (mpGraphiteData)? mpGraphiteData->getFace() : NULL;
}
#endif

bool ImplWinFontData::HasGSUBstitutions( HDC hDC ) const
{
    if( !mbGsubRead )
        ReadGsubTable( hDC );
    return !maGsubTable.empty();
}

bool ImplWinFontData::IsGSUBstituted( sal_UCS4 cChar ) const
{
    return( maGsubTable.find( cChar ) != maGsubTable.end() );
}

FontCharMapPtr ImplWinFontData::GetFontCharMap() const
{
    if( !mpUnicodeMap )
        return NULL;
    return mpUnicodeMap;
}

bool ImplWinFontData::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    rFontCapabilities = maFontCapabilities;
    return !rFontCapabilities.maUnicodeRange.empty() || !rFontCapabilities.maCodePageRange.empty();
}

void ImplWinFontData::ReadGsubTable( HDC hDC ) const
{
    mbGsubRead = true;

    // check the existence of a GSUB table
    const DWORD GsubTag = CalcTag( "GSUB" );
    DWORD nRC = ::GetFontData( hDC, GsubTag, 0, NULL, 0 );
    if( (nRC == GDI_ERROR) || !nRC )
        return;

    // parse the GSUB table through sft
    // TODO: parse it directly

    // sft needs the full font file data => get it
    const RawFontData aRawFontData( hDC );
    if( !aRawFontData.get() )
        return;

    // open font file
    sal_uInt32 nFaceNum = 0;
    if( !*aRawFontData.get() )  // TTC candidate
        nFaceNum = ~0U;  // indicate "TTC font extracts only"

    TrueTypeFont* pTTFont = NULL;
    ::OpenTTFontBuffer( (void*)aRawFontData.get(), aRawFontData.size(), nFaceNum, &pTTFont );
    if( !pTTFont )
        return;

    // add vertically substituted characters to list
    static const sal_Unicode aGSUBCandidates[] = {
        0x0020, 0x0080, // ASCII
        0x2000, 0x2600, // misc
        0x3000, 0x3100, // CJK punctutation
        0x3300, 0x3400, // squared words
        0xFF00, 0xFFF0, // halfwidth|fullwidth forms
    0 };

    for( const sal_Unicode* pPair = aGSUBCandidates; *pPair; pPair += 2 )
        for( sal_Unicode cChar = pPair[0]; cChar < pPair[1]; ++cChar )
            if( ::MapChar( pTTFont, cChar, false ) != ::MapChar( pTTFont, cChar, true ) )
                maGsubTable.insert( cChar ); // insert GSUBbed unicodes

    CloseTTFont( pTTFont );
}

void ImplWinFontData::ReadCmapTable( HDC hDC ) const
{
    if( mpUnicodeMap != NULL )
        return;

    bool bIsSymbolFont = (meWinCharSet == SYMBOL_CHARSET);
    // get the CMAP table from the font which is selected into the DC
    const DWORD nCmapTag = CalcTag( "cmap" );
    const RawFontData aRawFontData( hDC, nCmapTag );
    // parse the CMAP table if available
    if( aRawFontData.get() ) {
        CmapResult aResult;
        ParseCMAP( aRawFontData.get(), aRawFontData.size(), aResult );
        aResult.mbSymbolic = bIsSymbolFont;
        if( aResult.mnRangeCount > 0 )
        {
            FontCharMapPtr pUnicodeMap(new FontCharMap(aResult));
            mpUnicodeMap = pUnicodeMap;
        }
    }

    if( !mpUnicodeMap )
    {
        mpUnicodeMap = FontCharMap::GetDefaultMap( bIsSymbolFont );
    }
}

void ImplWinFontData::GetFontCapabilities( HDC hDC ) const
{
    // read this only once per font
    if( mbFontCapabilitiesRead )
        return;

    mbFontCapabilitiesRead = true;

    // GSUB table
    DWORD nLength;
    const DWORD GsubTag = CalcTag( "GSUB" );
    nLength = ::GetFontData( hDC, GsubTag, 0, NULL, 0 );
    if( (nLength != GDI_ERROR) && nLength )
    {
        std::vector<unsigned char> aTable( nLength );
        unsigned char* pTable = &aTable[0];
        ::GetFontData( hDC, GsubTag, 0, pTable, nLength );
        vcl::getTTScripts(maFontCapabilities.maGSUBScriptTags, pTable, nLength);
    }

    // OS/2 table
    const DWORD OS2Tag = CalcTag( "OS/2" );
    nLength = ::GetFontData( hDC, OS2Tag, 0, NULL, 0 );
    if( (nLength != GDI_ERROR) && nLength )
    {
        std::vector<unsigned char> aTable( nLength );
        unsigned char* pTable = &aTable[0];
        ::GetFontData( hDC, OS2Tag, 0, pTable, nLength );
        if (vcl::getTTCoverage(maFontCapabilities.maUnicodeRange, maFontCapabilities.maCodePageRange, pTable, nLength))
        {
            // Check for CJK capabilities of the current font
            // TODO, we have this info already from getTT, decode bits to
            // a readable dynamic_bitset
            sal_uInt32 ulUnicodeRange1 = GetUInt( pTable + 42 );
            sal_uInt32 ulUnicodeRange2 = GetUInt( pTable + 46 );

            mbHasCJKSupport = (ulUnicodeRange2 & 0x2DF00000);
            mbHasKoreanRange= (ulUnicodeRange1 & 0x10000000)
                            | (ulUnicodeRange2 & 0x01100000);
            mbHasArabicSupport = (ulUnicodeRange1 & 0x00002000);
        }
    }
}

void WinSalGraphics::SetTextColor( SalColor nSalColor )
{
    COLORREF aCol = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                SALCOLOR_GREEN( nSalColor ),
                                SALCOLOR_BLUE( nSalColor ) );

    if( !mbPrinter &&
        GetSalData()->mhDitherPal &&
        ImplIsSysColorEntry( nSalColor ) )
    {
        aCol = PALRGB_TO_RGB( aCol );
    }

    ::SetTextColor( getHDC(), aCol );
}

int CALLBACK SalEnumQueryFontProcExW( const ENUMLOGFONTEXW*,
                                      const NEWTEXTMETRICEXW*,
                                      DWORD, LPARAM lParam )
{
    *((bool*)(void*)lParam) = true;
    return 0;
}

void ImplGetLogFontFromFontSelect( HDC hDC,
                                   const FontSelectPattern* pFont,
                                   LOGFONTW& rLogFont,
                                   bool /*bTestVerticalAvail*/ )
{
    OUString   aName;
    if ( pFont->mpFontData )
        aName = pFont->mpFontData->GetFamilyName();
    else
        aName = pFont->GetFamilyName().getToken( 0, ';' );

    UINT nNameLen = aName.getLength();
    if ( nNameLen > (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
        nNameLen = (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1;
    memcpy( rLogFont.lfFaceName, aName.getStr(), nNameLen*sizeof( wchar_t ) );
    rLogFont.lfFaceName[nNameLen] = 0;

    if( !pFont->mpFontData )
    {
        rLogFont.lfCharSet = pFont->IsSymbolFont() ? SYMBOL_CHARSET : DEFAULT_CHARSET;
        rLogFont.lfPitchAndFamily = ImplPitchToWin( pFont->GetPitch() )
                                  | ImplFamilyToWin( pFont->GetFamilyType() );
    }
    else
    {
        const ImplWinFontData* pWinFontData = static_cast<const ImplWinFontData*>( pFont->mpFontData );
        rLogFont.lfCharSet        = pWinFontData->GetCharSet();
        rLogFont.lfPitchAndFamily = pWinFontData->GetPitchAndFamily();
    }

    rLogFont.lfWeight          = ImplWeightToWin( pFont->GetWeight() );
    rLogFont.lfHeight          = (LONG)-pFont->mnHeight;
    rLogFont.lfWidth           = (LONG)pFont->mnWidth;
    rLogFont.lfUnderline       = 0;
    rLogFont.lfStrikeOut       = 0;
    rLogFont.lfItalic          = (pFont->GetSlant()) != ITALIC_NONE;
    rLogFont.lfEscapement      = pFont->mnOrientation;
    rLogFont.lfOrientation     = rLogFont.lfEscapement;
    rLogFont.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
    rLogFont.lfQuality         = DEFAULT_QUALITY;
    rLogFont.lfOutPrecision    = OUT_TT_PRECIS;
    if ( pFont->mnOrientation )
        rLogFont.lfClipPrecision |= CLIP_LH_ANGLES;

    // disable antialiasing if requested
    if ( pFont->mbNonAntialiased )
        rLogFont.lfQuality = NONANTIALIASED_QUALITY;

    // select vertical mode if requested and available
    if( pFont->mbVertical && nNameLen )
    {
        // vertical fonts start with an '@'
        memmove( &rLogFont.lfFaceName[1], &rLogFont.lfFaceName[0],
            sizeof(rLogFont.lfFaceName)-sizeof(rLogFont.lfFaceName[0]) );
        rLogFont.lfFaceName[0] = '@';

        // check availability of vertical mode for this font
        bool bAvailable = false;
        EnumFontFamiliesExW( hDC, &rLogFont, (FONTENUMPROCW)SalEnumQueryFontProcExW,
                         (LPARAM)&bAvailable, 0 );

        if( !bAvailable )
        {
            // restore non-vertical name if not vertical mode isn't available
            memcpy( &rLogFont.lfFaceName[0], aName.getStr(), nNameLen*sizeof(wchar_t) );
            if( nNameLen < LF_FACESIZE )
                rLogFont.lfFaceName[nNameLen] = '\0';
        }
    }
}

HFONT WinSalGraphics::ImplDoSetFont( FontSelectPattern* i_pFont, float& o_rFontScale, HFONT& o_rOldFont )
{
    HFONT hNewFont = 0;

    HDC hdcScreen = 0;
    if( mbVirDev )
        // only required for virtual devices, see below for details
        hdcScreen = GetDC(0);

    LOGFONTW aLogFont;
    ImplGetLogFontFromFontSelect( getHDC(), i_pFont, aLogFont, true );

    // on the display we prefer Courier New when Courier is a
    // bitmap only font and we need to stretch or rotate it
    if( mbScreen
    &&  (i_pFont->mnWidth != 0
      || i_pFont->mnOrientation != 0
      || i_pFont->mpFontData == NULL
      || (i_pFont->mpFontData->GetHeight() != i_pFont->mnHeight))
    && !bImplSalCourierScalable
    && bImplSalCourierNew
    && (ImplSalWICompareAscii( aLogFont.lfFaceName, "Courier" ) == 0) )
        lstrcpynW( aLogFont.lfFaceName, L"Courier New", 11 );

    // #i47675# limit font requests to MAXFONTHEIGHT
    // TODO: share MAXFONTHEIGHT font instance
    if( (-aLogFont.lfHeight <= MAXFONTHEIGHT)
    &&  (+aLogFont.lfWidth <= MAXFONTHEIGHT) )
    {
        o_rFontScale = 1.0;
    }
    else if( -aLogFont.lfHeight >= +aLogFont.lfWidth )
    {
        o_rFontScale = -aLogFont.lfHeight / (float)MAXFONTHEIGHT;
        aLogFont.lfHeight = -MAXFONTHEIGHT;
        aLogFont.lfWidth = FRound( aLogFont.lfWidth / o_rFontScale );
    }
    else // #i95867# also limit font widths
    {
        o_rFontScale = +aLogFont.lfWidth / (float)MAXFONTHEIGHT;
        aLogFont.lfWidth = +MAXFONTHEIGHT;
        aLogFont.lfHeight = FRound( aLogFont.lfHeight / o_rFontScale );
    }

    hNewFont = ::CreateFontIndirectW( &aLogFont );
    if( hdcScreen )
    {
        // select font into screen hdc first to get an antialiased font
        // see knowledge base article 305290:
        // "PRB: Fonts Not Drawn Antialiased on Device Context for DirectDraw Surface"
        SelectFont( hdcScreen, SelectFont( hdcScreen , hNewFont ) );
    }
    o_rOldFont = ::SelectFont( getHDC(), hNewFont );

    TEXTMETRICW aTextMetricW;
    if( !::GetTextMetricsW( getHDC(), &aTextMetricW ) )
    {
        // the selected font doesn't work => try a replacement
        // TODO: use its font fallback instead
        lstrcpynW( aLogFont.lfFaceName, L"Courier New", 11 );
        aLogFont.lfPitchAndFamily = FIXED_PITCH;
        HFONT hNewFont2 = CreateFontIndirectW( &aLogFont );
        SelectFont( getHDC(), hNewFont2 );
        DeleteFont( hNewFont );
        hNewFont = hNewFont2;
    }

    if( hdcScreen )
        ::ReleaseDC( NULL, hdcScreen );

    return hNewFont;
}

sal_uInt16 WinSalGraphics::SetFont( FontSelectPattern* pFont, int nFallbackLevel )
{
    // return early if there is no new font
    if( !pFont )
    {
        // deselect still active font
        if( mhDefFont )
            ::SelectFont( getHDC(), mhDefFont );
        mfCurrentFontScale = mfFontScale[nFallbackLevel];
        // release no longer referenced font handles
        for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
        {
            if( mhFonts[i] )
                ::DeleteFont( mhFonts[i] );
            mhFonts[ i ] = 0;
        }
        mhDefFont = 0;
        return 0;
    }

    DBG_ASSERT( pFont->mpFontData, "WinSalGraphics mpFontData==NULL");
    mpWinFontEntry[ nFallbackLevel ] = reinterpret_cast<ImplWinFontEntry*>( pFont->mpFontEntry );
    mpWinFontData[ nFallbackLevel ] = static_cast<const ImplWinFontData*>( pFont->mpFontData );

    HFONT hOldFont = 0;
    HFONT hNewFont = ImplDoSetFont( pFont, mfFontScale[ nFallbackLevel ], hOldFont );
    mfCurrentFontScale = mfFontScale[nFallbackLevel];

    if( !mhDefFont )
    {
        // keep default font
        mhDefFont = hOldFont;
    }
    else
    {
        // release no longer referenced font handles
        for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
        {
            if( mhFonts[i] )
            {
                ::DeleteFont( mhFonts[i] );
                mhFonts[i] = 0;
            }
        }
    }

    // store new font in correct layer
    mhFonts[ nFallbackLevel ] = hNewFont;
    // now the font is live => update font face
    if( mpWinFontData[ nFallbackLevel ] )
        mpWinFontData[ nFallbackLevel ]->UpdateFromHDC( getHDC() );

    // some printers have higher internal resolution, so their
    // text output would be different from what we calculated
    // => suggest DrawTextArray to workaround this problem
    if ( mbPrinter )
        return SAL_SETFONT_USEDRAWTEXTARRAY;
    else
        return 0;
}

void WinSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel )
{
    // temporarily change the HDC to the font in the fallback level
    HFONT hOldFont = SelectFont( getHDC(), mhFonts[nFallbackLevel] );

    wchar_t aFaceName[LF_FACESIZE+60];
    if( ::GetTextFaceW( getHDC(), sizeof(aFaceName)/sizeof(wchar_t), aFaceName ) )
        pMetric->SetFamilyName(OUString(reinterpret_cast<const sal_Unicode*>(aFaceName)));

    // get the font metric
    TEXTMETRICA aWinMetric;
    const bool bOK = GetTextMetricsA( getHDC(), &aWinMetric );
    // restore the HDC to the font in the base level
    SelectFont( getHDC(), hOldFont );
    if( !bOK )
        return;

    // device independent font attributes
    pMetric->SetFamilyType(ImplFamilyToSal( aWinMetric.tmPitchAndFamily ));
    pMetric->SetSymbolFlag(aWinMetric.tmCharSet == SYMBOL_CHARSET);
    pMetric->SetWeight(ImplWeightToSal( aWinMetric.tmWeight ));
    pMetric->SetPitch(ImplMetricPitchToSal( aWinMetric.tmPitchAndFamily ));
    pMetric->SetItalic(aWinMetric.tmItalic ? ITALIC_NORMAL : ITALIC_NONE);
    pMetric->mnSlant        = 0;

    // device dependent font attributes
    pMetric->mbDevice       = (aWinMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;
    pMetric->mbScalableFont = (aWinMetric.tmPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE)) != 0;
    if( pMetric->mbScalableFont )
    {
        // check if there are kern pairs
        // TODO: does this work with GPOS kerning?
        DWORD nKernPairs = ::GetKerningPairsA( getHDC(), 0, NULL );
        pMetric->mbKernableFont = (nKernPairs > 0);
    }
    else
    {
        // bitmap fonts cannot be rotated directly
        pMetric->mnOrientation  = 0;
        // bitmap fonts have no kerning
        pMetric->mbKernableFont = false;
    }

    // transformation dependent font metrics
    pMetric->mnWidth        = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmAveCharWidth );
    pMetric->mnIntLeading   = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmInternalLeading );
    pMetric->mnExtLeading   = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmExternalLeading );
    pMetric->mnAscent       = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmAscent );
    pMetric->mnDescent      = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmDescent );

    // #107888# improved metric compatibility for Asian fonts...
    // TODO: assess workaround below for CWS >= extleading
    // TODO: evaluate use of aWinMetric.sTypo* members for CJK
    if( mpWinFontData[nFallbackLevel] && mpWinFontData[nFallbackLevel]->SupportsCJK() )
    {
        pMetric->mnIntLeading += pMetric->mnExtLeading;

        // #109280# The line height for Asian fonts is too small.
        // Therefore we add half of the external leading to the
        // ascent, the other half is added to the descent.
        const long nHalfTmpExtLeading = pMetric->mnExtLeading / 2;
        const long nOtherHalfTmpExtLeading = pMetric->mnExtLeading - nHalfTmpExtLeading;

        // #110641# external leading for Asian fonts.
        // The factor 0.3 has been confirmed with experiments.
        long nCJKExtLeading = static_cast<long>(0.30 * (pMetric->mnAscent + pMetric->mnDescent));
        nCJKExtLeading -= pMetric->mnExtLeading;
        pMetric->mnExtLeading = (nCJKExtLeading > 0) ? nCJKExtLeading : 0;

        pMetric->mnAscent   += nHalfTmpExtLeading;
        pMetric->mnDescent  += nOtherHalfTmpExtLeading;
    }

    pMetric->mnMinKashida = GetMinKashidaWidth();
}

const FontCharMapPtr WinSalGraphics::GetFontCharMap() const
{
    if( !mpWinFontData[0] )
    {
        FontCharMapPtr pDefFontCharMap( new FontCharMap() );
        return pDefFontCharMap;
    }
    return mpWinFontData[0]->GetFontCharMap();
}

bool WinSalGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if( !mpWinFontData[0] )
        return false;
    return mpWinFontData[0]->GetFontCapabilities(rFontCapabilities);
}

int CALLBACK SalEnumFontsProcExA( const ENUMLOGFONTEXA* pLogFont,
                                  const NEWTEXTMETRICEXA* pMetric,
                                  DWORD nFontType, LPARAM lParam )
{
    ImplEnumInfo* pInfo = (ImplEnumInfo*)(void*)lParam;
    if ( !pInfo->mpName )
    {
        // Ignore vertical fonts
        if ( pLogFont->elfLogFont.lfFaceName[0] != '@' )
        {
            if ( !pInfo->mbImplSalCourierNew )
                pInfo->mbImplSalCourierNew = stricmp( pLogFont->elfLogFont.lfFaceName, "Courier New" ) == 0;
            if ( !pInfo->mbImplSalCourierScalable )
                pInfo->mbCourier = stricmp( pLogFont->elfLogFont.lfFaceName, "Courier" ) == 0;
            else
                pInfo->mbCourier = FALSE;
            OUString aName( ImplSalGetUniString( pLogFont->elfLogFont.lfFaceName ) );
            pInfo->mpName = &aName;
            strncpy( pInfo->mpLogFontA->lfFaceName, pLogFont->elfLogFont.lfFaceName, LF_FACESIZE );
            pInfo->mpLogFontA->lfCharSet = pLogFont->elfLogFont.lfCharSet;
            EnumFontFamiliesExA( pInfo->mhDC, pInfo->mpLogFontA, (FONTENUMPROCA)SalEnumFontsProcExA,
                                 (LPARAM)(void*)pInfo, 0 );
            pInfo->mpLogFontA->lfFaceName[0] = '\0';
            pInfo->mpLogFontA->lfCharSet = DEFAULT_CHARSET;
            pInfo->mpName = NULL;
            pInfo->mbCourier = FALSE;
        }
    }
    else
    {
        // ignore non-scalable non-device font on printer
        if( pInfo->mbPrinter )
            if( (nFontType & RASTER_FONTTYPE) && !(nFontType & DEVICE_FONTTYPE) )
                return 1;

        ImplWinFontData* pData = ImplLogMetricToDevFontDataA( pLogFont, &(pMetric->ntmTm), nFontType );
        pData->SetFontId( sal_IntPtr( pInfo->mnFontCount++ ) );

        // prefer the system character set, so that we get as much as
        // possible important characters. In the other case we could only
        // display a limited set of characters (#87309#)
        if ( pInfo->mnPreferredCharSet == pLogFont->elfLogFont.lfCharSet )
            pData->mnQuality += 100;

        // knowing Courier to be scalable is nice
        if( pInfo->mbCourier )
            pInfo->mbImplSalCourierScalable |= pData->IsScalable();

        pInfo->mpList->Add( pData );
    }

    return 1;
}

int CALLBACK SalEnumFontsProcExW( const ENUMLOGFONTEXW* pLogFont,
                                  const NEWTEXTMETRICEXW* pMetric,
                                  DWORD nFontType, LPARAM lParam )
{
    ImplEnumInfo* pInfo = (ImplEnumInfo*)(void*)lParam;
    if ( !pInfo->mpName )
    {
        // Ignore vertical fonts
        if ( pLogFont->elfLogFont.lfFaceName[0] != '@' )
        {
            if ( !pInfo->mbImplSalCourierNew )
                pInfo->mbImplSalCourierNew = ImplSalWICompareAscii( pLogFont->elfLogFont.lfFaceName, "Courier New" ) == 0;
            if ( !pInfo->mbImplSalCourierScalable )
                pInfo->mbCourier = ImplSalWICompareAscii( pLogFont->elfLogFont.lfFaceName, "Courier" ) == 0;
            else
                pInfo->mbCourier = FALSE;
            OUString aName = OUString(reinterpret_cast<const sal_Unicode*>(pLogFont->elfLogFont.lfFaceName));
            pInfo->mpName = &aName;
            memcpy( pInfo->mpLogFontW->lfFaceName, pLogFont->elfLogFont.lfFaceName, (aName.getLength()+1)*sizeof( wchar_t ) );
            pInfo->mpLogFontW->lfCharSet = pLogFont->elfLogFont.lfCharSet;
            EnumFontFamiliesExW( pInfo->mhDC, pInfo->mpLogFontW, (FONTENUMPROCW)SalEnumFontsProcExW,
                                 (LPARAM)(void*)pInfo, 0 );
            pInfo->mpLogFontW->lfFaceName[0] = '\0';
            pInfo->mpLogFontW->lfCharSet = DEFAULT_CHARSET;
            pInfo->mpName = NULL;
            pInfo->mbCourier = FALSE;
        }
    }
    else
    {
        // ignore non-scalable non-device font on printer
        if( pInfo->mbPrinter )
            if( (nFontType & RASTER_FONTTYPE) && !(nFontType & DEVICE_FONTTYPE) )
                return 1;

        ImplWinFontData* pData = ImplLogMetricToDevFontDataW( pLogFont, &(pMetric->ntmTm), nFontType );
        pData->SetFontId( sal_IntPtr( pInfo->mnFontCount++ ) );

        // knowing Courier to be scalable is nice
        if( pInfo->mbCourier )
            pInfo->mbImplSalCourierScalable |= pData->IsScalable();

        pInfo->mpList->Add( pData );
    }

    return 1;
}

struct TempFontItem
{
    OUString maFontFilePath;
    OString maResourcePath;
    TempFontItem* mpNextItem;
};

#ifdef FR_PRIVATE
static int WINAPI __AddFontResourceExW( LPCWSTR lpszfileName, DWORD fl, PVOID pdv )
{
    typedef int (WINAPI *AddFontResourceExW_FUNC)(LPCWSTR, DWORD, PVOID );

    static AddFontResourceExW_FUNC  pFunc = NULL;
    static HMODULE                  hmGDI = NULL;

    if ( !pFunc && !hmGDI )
    {
        hmGDI = GetModuleHandleA( "GDI32" );
        if ( hmGDI )
            pFunc = reinterpret_cast<AddFontResourceExW_FUNC>( GetProcAddress( hmGDI, "AddFontResourceExW" ) );
    }

    if ( pFunc )
        return pFunc( lpszfileName, fl, pdv );
    else
    {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return 0;
    }
}
#endif

bool ImplAddTempFont( SalData& rSalData, const OUString& rFontFileURL )
{
    int nRet = 0;
    OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

#ifdef FR_PRIVATE
    nRet = __AddFontResourceExW( reinterpret_cast<LPCWSTR>(aUSytemPath.getStr()), FR_PRIVATE, NULL );
#endif

    if ( !nRet )
    {
        static int nCounter = 0;
        char aFileName[] = "soAA.fot";
        aFileName[2] = sal::static_int_cast<char>('A' + (15 & (nCounter>>4)));
        aFileName[3] = sal::static_int_cast<char>('A' + (15 & nCounter));
        char aResourceName[512];
        int nMaxLen = sizeof(aResourceName)/sizeof(*aResourceName) - 16;
        int nLen = ::GetTempPathA( nMaxLen, aResourceName );
        ::strncpy( aResourceName + nLen, aFileName, sizeof( aResourceName )- nLen );
        // security: end buffer in any case
        aResourceName[ (sizeof(aResourceName)/sizeof(*aResourceName))-1 ] = 0;
        ::DeleteFileA( aResourceName );

        rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
        OString aCFileName = OUStringToOString( aUSytemPath, theEncoding );
        // TODO: font should be private => need to investigate why it doesn't work then
        if( !::CreateScalableFontResourceA( 0, aResourceName, aCFileName.getStr(), NULL ) )
            return false;
        ++nCounter;

        nRet = ::AddFontResourceA( aResourceName );
        if( nRet > 0 )
        {
            TempFontItem* pNewItem = new TempFontItem;
            pNewItem->maResourcePath = OString( aResourceName );
            pNewItem->maFontFilePath = aUSytemPath.getStr();
            pNewItem->mpNextItem = rSalData.mpTempFontItem;
            rSalData.mpTempFontItem = pNewItem;
        }
    }

    return (nRet > 0);
}

void ImplReleaseTempFonts( SalData& rSalData )
{
    int nCount = 0;
    while( TempFontItem* p = rSalData.mpTempFontItem )
    {
        ++nCount;
        if( p->maResourcePath.getLength() )
        {
            const char* pResourcePath = p->maResourcePath.getStr();
            ::RemoveFontResourceA( pResourcePath );
            ::DeleteFileA( pResourcePath );
        }
        else
        {
            ::RemoveFontResourceW( reinterpret_cast<LPCWSTR>(p->maFontFilePath.getStr()) );
        }

        rSalData.mpTempFontItem = p->mpNextItem;
        delete p;
    }

#ifndef FR_PRIVATE
    // notify every other application
    // unless the temp fonts were installed as private fonts
    if( nCount > 0 )
        PostMessageW( HWND_BROADCAST, WM_FONTCHANGE, 0, NULL );
#endif // FR_PRIVATE
}

static bool ImplGetFontAttrFromFile( const OUString& rFontFileURL,
    ImplDevFontAttributes& rDFA )
{
    OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

    // get FontAttributes from a *fot file
    // TODO: use GetTTGlobalFontInfo() to access the font directly
    rDFA.mnQuality    = 1000;
    rDFA.mbDevice     = true;
    rDFA.SetFamilyType(FAMILY_DONTKNOW);
    rDFA.SetWidthType(WIDTH_DONTKNOW);
    rDFA.SetWeight(WEIGHT_DONTKNOW);
    rDFA.SetItalic(ITALIC_DONTKNOW);
    rDFA.SetPitch(PITCH_DONTKNOW);
    rDFA.mbSubsettable= true;
    rDFA.mbEmbeddable = false;

    // Create temporary file name
    char aFileName[] = "soAAT.fot";
    char aResourceName[512];
    int nMaxLen = sizeof(aResourceName)/sizeof(*aResourceName) - 16;
    int nLen = ::GetTempPathA( nMaxLen, aResourceName );
    ::strncpy( aResourceName + nLen, aFileName, std::max( 0, nMaxLen - nLen ));
    ::DeleteFileA( aResourceName );

    // Create font resource file (typically with a .fot file name extension).
    rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
    OString aCFileName = OUStringToOString( aUSytemPath, theEncoding );
    ::CreateScalableFontResourceA( 0, aResourceName, aCFileName.getStr(), NULL );

    // Open and read the font resource file
    OUString aFotFileName = OStringToOUString( aResourceName, osl_getThreadTextEncoding() );
    osl::FileBase::getFileURLFromSystemPath( aFotFileName, aFotFileName );
    osl::File aFotFile( aFotFileName );
    osl::FileBase::RC aError = aFotFile.open( osl_File_OpenFlag_Read );
    if( aError != osl::FileBase::E_None )
        return false;

    sal_uInt64  nBytesRead = 0;
    char        aBuffer[4096];
    aFotFile.read( aBuffer, sizeof( aBuffer ), nBytesRead );
    // clean up temporary resource file
    aFotFile.close();
    ::DeleteFileA( aResourceName );

    // retrieve font family name from byte offset 0x4F6
    sal_uInt64 i = 0x4F6;
    sal_uInt64 nNameOfs = i;
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    // skip full name
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    // retrieve font style name
    int nStyleOfs = i;
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    if( i >= nBytesRead )
        return false;

    // convert byte strings to unicode
    char *pName = aBuffer + nNameOfs;
    rDFA.SetFamilyName(OUString(pName, strlen(pName), osl_getThreadTextEncoding()));
    char *pStyle = aBuffer + nStyleOfs;
    rDFA.SetStyleName(OUString(pStyle, strlen(pStyle), osl_getThreadTextEncoding() ));

    // byte offset 0x4C7: OS2_fsSelection
    const char nFSS = aBuffer[ 0x4C7 ];
    if( nFSS & 0x01 )   // italic
        rDFA.SetItalic(ITALIC_NORMAL);
    //if( nFSS & 0x20 )   // bold
    //   rDFA.meWeight = WEIGHT_BOLD;
    if( nFSS & 0x40 )   // regular
    {
        rDFA.SetWeight(WEIGHT_NORMAL);
        rDFA.SetItalic(ITALIC_NONE);
    }

    // byte offsets 0x4D7/0x4D8: wingdi's FW_WEIGHT
    int nWinWeight = (aBuffer[0x4D7] & 0xFF) + ((aBuffer[0x4D8] & 0xFF) << 8);
    rDFA.SetWeight(ImplWeightToSal( nWinWeight ));

    rDFA.SetSymbolFlag(false);          // TODO
    rDFA.SetPitch(PITCH_DONTKNOW); // TODO

    // byte offset 0x4DE: pitch&family
    rDFA.SetFamilyType(ImplFamilyToSal( aBuffer[0x4DE] ));

    // byte offsets 0x4C8/0x4C9: emunits
    // byte offsets 0x4CE/0x4CF: winascent
    // byte offsets 0x4D0/0x4D1: winascent+windescent-emunits
    // byte offsets 0x4DF/0x4E0: avgwidth

    return true;
}

bool WinSalGraphics::AddTempDevFont( PhysicalFontCollection* pFontCollection,
    const OUString& rFontFileURL, const OUString& rFontName )
{
    SAL_INFO( "vcl.gdi", "WinSalGraphics::AddTempDevFont(): " << OUStringToOString( rFontFileURL, RTL_TEXTENCODING_UTF8 ).getStr() );

    ImplDevFontAttributes aDFA;
    aDFA.SetFamilyName(rFontName);
    aDFA.mnQuality    = 1000;
    aDFA.mbDevice     = true;

    // Search Font Name in Cache
    if( rFontName.isEmpty() && mpFontAttrCache )
        aDFA = mpFontAttrCache->GetFontAttr( rFontFileURL );

    // Retrieve font name from font resource
    if( aDFA.GetFamilyName().isEmpty() )
    {
        ImplGetFontAttrFromFile( rFontFileURL, aDFA );
        if( mpFontAttrCache && !aDFA.GetFamilyName().isEmpty() )
            mpFontAttrCache->AddFontAttr( rFontFileURL, aDFA );
    }

    if ( aDFA.GetFamilyName().isEmpty() )
        return false;

    // remember temp font for cleanup later
    if( !ImplAddTempFont( *GetSalData(), rFontFileURL ) )
        return false;

    UINT nPreferredCharSet = DEFAULT_CHARSET;

    // create matching FontData struct
    aDFA.SetSymbolFlag(false); // TODO: how to know it without accessing the font?
    aDFA.SetFamilyType(FAMILY_DONTKNOW);
    aDFA.SetWidthType(WIDTH_DONTKNOW);
    aDFA.SetWeight(WEIGHT_DONTKNOW);
    aDFA.SetItalic(ITALIC_DONTKNOW);
    aDFA.SetPitch(PITCH_DONTKNOW);
    aDFA.mbSubsettable= true;
    aDFA.mbEmbeddable = false;

    /*
    // TODO: improve ImplDevFontAttributes using the "font resource file"
    aDFS.maName = // using "FONTRES:" from file
    if( rFontName != aDFS.maName )
        aDFS.maMapName = aFontName;
    */

    ImplWinFontData* pFontData = new ImplWinFontData( aDFA, 0,
        sal::static_int_cast<BYTE>(nPreferredCharSet),
        sal::static_int_cast<BYTE>(TMPF_VECTOR|TMPF_TRUETYPE) );
    pFontData->SetFontId( reinterpret_cast<sal_IntPtr>(pFontData) );
    pFontCollection->Add( pFontData );
    return true;
}

void WinSalGraphics::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    // make sure all fonts are registered at least temporarily
    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;

        // determine font path
        // since we are only interested in fonts that could not be
        // registered before because of missing administration rights
        // only the font path of the user installation is needed
        OUString aPath;
        osl_getExecutableFile( &aPath.pData );
        aPath = aPath.copy( 0, aPath.lastIndexOf('/') );
        OUString aFontDirUrl = aPath.copy( 0, aPath.lastIndexOf('/') );
        aFontDirUrl += "/" LIBO_SHARE_FOLDER "/fonts/truetype";

        // collect fonts in font path that could not be registered
        osl::Directory aFontDir( aFontDirUrl );
        osl::FileBase::RC rcOSL = aFontDir.open();
        if( rcOSL == osl::FileBase::E_None )
        {
            osl::DirectoryItem aDirItem;
            OUString aEmptyString;

            OUString aBootStrap;
            rtl::Bootstrap::get( OUString("BRAND_BASE_DIR"), aBootStrap );
            aBootStrap += "/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap" );
            rtl::Bootstrap aBootstrap( aBootStrap );
            OUString aUserPath;
            aBootstrap.getFrom( OUString( "UserInstallation" ), aUserPath );
            aUserPath += "/user/config/fontnames.dat";
            OUString aBaseURL = aPath.copy( 0, aPath.lastIndexOf('/')+1 );
            mpFontAttrCache = new ImplFontAttrCache( aUserPath, aBaseURL );

            while( aFontDir.getNextItem( aDirItem, 10 ) == osl::FileBase::E_None )
            {
                osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileURL );
                rcOSL = aDirItem.getFileStatus( aFileStatus );
                if ( rcOSL == osl::FileBase::E_None )
                    AddTempDevFont( pFontCollection, aFileStatus.getFileURL(), aEmptyString );
            }

            delete mpFontAttrCache; // destructor rewrites the cache file if needed
            mpFontAttrCache = NULL;
        }
    }

    ImplEnumInfo aInfo;
    aInfo.mhDC          = getHDC();
    aInfo.mpList        = pFontCollection;
    aInfo.mpName        = NULL;
    aInfo.mpLogFontA    = NULL;
    aInfo.mpLogFontW    = NULL;
    aInfo.mbCourier     = false;
    aInfo.mbPrinter     = mbPrinter;
    aInfo.mnFontCount   = 0;
    if ( !mbPrinter )
    {
        aInfo.mbImplSalCourierScalable  = false;
        aInfo.mbImplSalCourierNew       = false;
    }
    else
    {
        aInfo.mbImplSalCourierScalable  = true;
        aInfo.mbImplSalCourierNew       = true;
    }

    aInfo.mnPreferredCharSet = DEFAULT_CHARSET;
    DWORD nCP = GetACP();
    CHARSETINFO aCharSetInfo;
    if ( TranslateCharsetInfo( (DWORD*)(sal_IntPtr)nCP, &aCharSetInfo, TCI_SRCCODEPAGE ) )
        aInfo.mnPreferredCharSet = aCharSetInfo.ciCharset;

    LOGFONTW aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );
    aLogFont.lfCharSet = DEFAULT_CHARSET;
    aInfo.mpLogFontW = &aLogFont;
    EnumFontFamiliesExW( getHDC(), &aLogFont,
        (FONTENUMPROCW)SalEnumFontsProcExW, (LPARAM)(void*)&aInfo, 0 );

    // check what Courier fonts are used on the screen, so to perhaps
    // map Courier to CourierNew in SetFont()
    if ( !mbPrinter )
    {
        bImplSalCourierScalable = aInfo.mbImplSalCourierScalable;
        bImplSalCourierNew      = aInfo.mbImplSalCourierNew;
    }

    // set glyph fallback hook
    static WinGlyphFallbackSubstititution aSubstFallback( getHDC() );
    pFontCollection->SetFallbackHook( &aSubstFallback );
}

void WinSalGraphics::ClearDevFontCache()
{
    //anything to do here ?
}

bool WinSalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    HDC hDC = getHDC();

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_METRICS;
    if( !(aGlyphId & GF_ISCHAR) )
        nGGOFlags |= GGO_GLYPH_INDEX;
    aGlyphId &= GF_IDXMASK;

    GLYPHMETRICS aGM;
    aGM.gmptGlyphOrigin.x = aGM.gmptGlyphOrigin.y = 0;
    aGM.gmBlackBoxX = aGM.gmBlackBoxY = 0;
    DWORD nSize = ::GetGlyphOutlineW( hDC, aGlyphId, nGGOFlags, &aGM, 0, NULL, &aMat );
    if( nSize == GDI_ERROR )
        return false;

    rRect = Rectangle( Point( +aGM.gmptGlyphOrigin.x, -aGM.gmptGlyphOrigin.y ),
        Size( aGM.gmBlackBoxX, aGM.gmBlackBoxY ) );
    rRect.Left()    = static_cast<int>( mfCurrentFontScale * rRect.Left() );
    rRect.Right()   = static_cast<int>( mfCurrentFontScale * rRect.Right() );
    rRect.Top()     = static_cast<int>( mfCurrentFontScale * rRect.Top() );
    rRect.Bottom()  = static_cast<int>( mfCurrentFontScale * rRect.Bottom() );
    return true;
}

bool WinSalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId,
    ::basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    rB2DPolyPoly.clear();

    HDC  hDC = getHDC();

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_NATIVE;
    if( !(aGlyphId & GF_ISCHAR) )
        nGGOFlags |= GGO_GLYPH_INDEX;
    aGlyphId &= GF_IDXMASK;

    GLYPHMETRICS aGlyphMetrics;
    const DWORD nSize1 = ::GetGlyphOutlineW( hDC, aGlyphId, nGGOFlags, &aGlyphMetrics, 0, NULL, &aMat );
    if( !nSize1 )       // blank glyphs are ok
        return true;
    else if( nSize1 == GDI_ERROR )
        return false;

    BYTE* pData = new BYTE[ nSize1 ];
    const DWORD nSize2 = ::GetGlyphOutlineW( hDC, aGlyphId, nGGOFlags,
              &aGlyphMetrics, nSize1, pData, &aMat );

    if( nSize1 != nSize2 )
        return false;

    // TODO: avoid tools polygon by creating B2DPolygon directly
    int     nPtSize = 512;
    Point*  pPoints = new Point[ nPtSize ];
    BYTE*   pFlags = new BYTE[ nPtSize ];

    TTPOLYGONHEADER* pHeader = (TTPOLYGONHEADER*)pData;
    while( (BYTE*)pHeader < pData+nSize2 )
    {
        // only outline data is interesting
        if( pHeader->dwType != TT_POLYGON_TYPE )
            break;

        // get start point; next start points are end points
        // of previous segment
        sal_uInt16 nPnt = 0;

        long nX = IntTimes256FromFixed( pHeader->pfxStart.x );
        long nY = IntTimes256FromFixed( pHeader->pfxStart.y );
        pPoints[ nPnt ] = Point( nX, nY );
        pFlags[ nPnt++ ] = POLY_NORMAL;

        bool bHasOfflinePoints = false;
        TTPOLYCURVE* pCurve = (TTPOLYCURVE*)( pHeader + 1 );
        pHeader = (TTPOLYGONHEADER*)( (BYTE*)pHeader + pHeader->cb );
        while( (BYTE*)pCurve < (BYTE*)pHeader )
        {
            int nNeededSize = nPnt + 16 + 3 * pCurve->cpfx;
            if( nPtSize < nNeededSize )
            {
                Point* pOldPoints = pPoints;
                BYTE* pOldFlags = pFlags;
                nPtSize = 2 * nNeededSize;
                pPoints = new Point[ nPtSize ];
                pFlags = new BYTE[ nPtSize ];
                for( sal_uInt16 i = 0; i < nPnt; ++i )
                {
                    pPoints[ i ] = pOldPoints[ i ];
                    pFlags[ i ] = pOldFlags[ i ];
                }
                delete[] pOldPoints;
                delete[] pOldFlags;
            }

            int i = 0;
            if( TT_PRIM_LINE == pCurve->wType )
            {
                while( i < pCurve->cpfx )
                {
                    nX = IntTimes256FromFixed( pCurve->apfx[ i ].x );
                    nY = IntTimes256FromFixed( pCurve->apfx[ i ].y );
                    ++i;
                    pPoints[ nPnt ] = Point( nX, nY );
                    pFlags[ nPnt ] = POLY_NORMAL;
                    ++nPnt;
                }
            }
            else if( TT_PRIM_QSPLINE == pCurve->wType )
            {
                bHasOfflinePoints = true;
                while( i < pCurve->cpfx )
                {
                    // get control point of quadratic bezier spline
                    nX = IntTimes256FromFixed( pCurve->apfx[ i ].x );
                    nY = IntTimes256FromFixed( pCurve->apfx[ i ].y );
                    ++i;
                    Point aControlP( nX, nY );

                    // calculate first cubic control point
                    // P0 = 1/3 * (PBeg + 2 * PQControl)
                    nX = pPoints[ nPnt-1 ].X() + 2 * aControlP.X();
                    nY = pPoints[ nPnt-1 ].Y() + 2 * aControlP.Y();
                    pPoints[ nPnt+0 ] = Point( (2*nX+3)/6, (2*nY+3)/6 );
                    pFlags[ nPnt+0 ] = POLY_CONTROL;

                    // calculate endpoint of segment
                    nX = IntTimes256FromFixed( pCurve->apfx[ i ].x );
                    nY = IntTimes256FromFixed( pCurve->apfx[ i ].y );

                    if ( i+1 >= pCurve->cpfx )
                    {
                        // endpoint is either last point in segment => advance
                        ++i;
                    }
                    else
                    {
                        // or endpoint is the middle of two control points
                        nX += IntTimes256FromFixed( pCurve->apfx[ i-1 ].x );
                        nY += IntTimes256FromFixed( pCurve->apfx[ i-1 ].y );
                        nX = (nX + 1) / 2;
                        nY = (nY + 1) / 2;
                        // no need to advance, because the current point
                        // is the control point in next bezier spline
                    }

                    pPoints[ nPnt+2 ] = Point( nX, nY );
                    pFlags[ nPnt+2 ] = POLY_NORMAL;

                    // calculate second cubic control point
                    // P1 = 1/3 * (PEnd + 2 * PQControl)
                    nX = pPoints[ nPnt+2 ].X() + 2 * aControlP.X();
                    nY = pPoints[ nPnt+2 ].Y() + 2 * aControlP.Y();
                    pPoints[ nPnt+1 ] = Point( (2*nX+3)/6, (2*nY+3)/6 );
                    pFlags[ nPnt+1 ] = POLY_CONTROL;

                    nPnt += 3;
                }
            }

            // next curve segment
            pCurve = (TTPOLYCURVE*)&pCurve->apfx[ i ];
        }

        // end point is start point for closed contour
        // disabled, because Polygon class closes the contour itself
        // pPoints[nPnt++] = pPoints[0];
        // #i35928#
        // Added again, but add only when not yet closed
        if(pPoints[nPnt - 1] != pPoints[0])
        {
            if( bHasOfflinePoints )
                pFlags[nPnt] = pFlags[0];

            pPoints[nPnt++] = pPoints[0];
        }

        // convert y-coordinates W32 -> VCL
        for( int i = 0; i < nPnt; ++i )
            pPoints[i].Y() = -pPoints[i].Y();

        // insert into polypolygon
        Polygon aPoly( nPnt, pPoints, (bHasOfflinePoints ? pFlags : NULL) );
        // convert to B2DPolyPolygon
        // TODO: get rid of the intermediate PolyPolygon
        rB2DPolyPoly.append( aPoly.getB2DPolygon() );
    }

    delete[] pPoints;
    delete[] pFlags;

    delete[] pData;

    // rescaling needed for the tools::PolyPolygon conversion
    if( rB2DPolyPoly.count() )
    {
        const double fFactor(mfCurrentFontScale/256);
        rB2DPolyPoly.transform(basegfx::tools::createScaleB2DHomMatrix(fFactor, fFactor));
    }

    return true;
}

class ScopedFont
{
public:
    explicit ScopedFont(WinSalGraphics & rData);

    ~ScopedFont();

private:
    WinSalGraphics & m_rData;
    HFONT m_hOrigFont;
};

ScopedFont::ScopedFont(WinSalGraphics & rData): m_rData(rData)
{
    m_hOrigFont = m_rData.mhFonts[0];
    m_rData.mhFonts[0] = 0; // avoid deletion of current font
}

ScopedFont::~ScopedFont()
{
    if( m_hOrigFont )
    {
        // restore original font, destroy temporary font
        HFONT hTempFont = m_rData.mhFonts[0];
        m_rData.mhFonts[0] = m_hOrigFont;
        SelectObject( m_rData.getHDC(), m_hOrigFont );
        DeleteObject( hTempFont );
    }
}

class ScopedTrueTypeFont
{
public:
    inline ScopedTrueTypeFont(): m_pFont(0) {}

    ~ScopedTrueTypeFont();

    int open(void * pBuffer, sal_uInt32 nLen, sal_uInt32 nFaceNum);

    inline TrueTypeFont * get() const { return m_pFont; }

private:
    TrueTypeFont * m_pFont;
};

ScopedTrueTypeFont::~ScopedTrueTypeFont()
{
    if (m_pFont != 0)
        CloseTTFont(m_pFont);
}

int ScopedTrueTypeFont::open(void * pBuffer, sal_uInt32 nLen,
                             sal_uInt32 nFaceNum)
{
    OSL_ENSURE(m_pFont == 0, "already open");
    return OpenTTFontBuffer(pBuffer, nLen, nFaceNum, &m_pFont);
}

bool WinSalGraphics::CreateFontSubset( const OUString& rToFile,
    const PhysicalFontFace* pFont, const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
    sal_Int32* pGlyphWidths, int nGlyphCount, FontSubsetInfo& rInfo )
{
    // TODO: use more of the central font-subsetting code, move stuff there if needed

    // create matching FontSelectPattern
    // we need just enough to get to the font file data
    // use height=1000 for easier debugging (to match psprint's font units)
    FontSelectPattern aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    // TODO: much better solution: move SetFont and restoration of old font to caller
    ScopedFont aOldFont(*this);
    float fScale = 1.0;
    HFONT hOldFont = 0;
    ImplDoSetFont( &aIFSD, fScale, hOldFont );

    ImplWinFontData* pWinFontData = (ImplWinFontData*)aIFSD.mpFontData;

#if OSL_DEBUG_LEVEL > 1
    // get font metrics
    TEXTMETRICA aWinMetric;
    if( !::GetTextMetricsA( getHDC(), &aWinMetric ) )
        return FALSE;

    DBG_ASSERT( !(aWinMetric.tmPitchAndFamily & TMPF_DEVICE), "cannot subset device font" );
    DBG_ASSERT( aWinMetric.tmPitchAndFamily & TMPF_TRUETYPE, "can only subset TT font" );
#endif

    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return FALSE;
    const rtl_TextEncoding aThreadEncoding = osl_getThreadTextEncoding();
    const OString aToFile(OUStringToOString(aSysPath, aThreadEncoding));

    // check if the font has a CFF-table
    const DWORD nCffTag = CalcTag( "CFF " );
    const RawFontData aRawCffData( getHDC(), nCffTag );
    if( aRawCffData.get() )
    {
        pWinFontData->UpdateFromHDC( getHDC() );
        FontCharMapPtr pCharMap = pWinFontData->GetFontCharMap();

        sal_GlyphId aRealGlyphIds[ 256 ];
        for( int i = 0; i < nGlyphCount; ++i )
        {
            // TODO: remap notdef glyph if needed
            // TODO: use GDI's GetGlyphIndices instead? Does it handle GSUB properly?
            sal_GlyphId aGlyphId = pGlyphIds[i] & GF_IDXMASK;
            if( pGlyphIds[i] & GF_ISCHAR ) // remaining pseudo-glyphs need to be translated
                aGlyphId = pCharMap->GetGlyphIndex( aGlyphId );
            if( (pGlyphIds[i] & (GF_ROTMASK|GF_GSUB)) != 0) // TODO: vertical substitution
                {/*####*/}

            aRealGlyphIds[i] = aGlyphId;
        }

        pCharMap = 0;

        // provide a font subset from the CFF-table
        FILE* pOutFile = fopen( aToFile.getStr(), "wb" );
        rInfo.LoadFont( FontSubsetInfo::CFF_FONT, aRawCffData.get(), aRawCffData.size() );
        bool bRC = rInfo.CreateFontSubset( FontSubsetInfo::TYPE1_PFB, pOutFile, NULL,
                aRealGlyphIds, pEncoding, nGlyphCount, pGlyphWidths );
        fclose( pOutFile );
        return bRC;
    }

    // get raw font file data
    const RawFontData xRawFontData( getHDC(), 0 );
    if( !xRawFontData.get() )
        return FALSE;

    // open font file
    sal_uInt32 nFaceNum = 0;
    if( !*xRawFontData.get() )  // TTC candidate
        nFaceNum = ~0U;  // indicate "TTC font extracts only"

    ScopedTrueTypeFont aSftTTF;
    int nRC = aSftTTF.open( (void*)xRawFontData.get(), xRawFontData.size(), nFaceNum );
    if( nRC != SF_OK )
        return FALSE;

    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( aSftTTF.get(), &aTTInfo );
    rInfo.m_nFontType   = FontSubsetInfo::SFNT_TTF;
    rInfo.m_aPSName     = ImplSalGetUniString( aTTInfo.psname );
    rInfo.m_nAscent     = aTTInfo.winAscent;
    rInfo.m_nDescent    = aTTInfo.winDescent;
    rInfo.m_aFontBBox   = Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
                                    Point( aTTInfo.xMax, aTTInfo.yMax ) );
    rInfo.m_nCapHeight  = aTTInfo.yMax; // Well ...

    // subset TTF-glyphs and get their properties
    // take care that subset fonts require the NotDef glyph in pos 0
    int nOrigCount = nGlyphCount;
    sal_uInt16    aShortIDs[ 256 ];
    sal_uInt8 aTempEncs[ 256 ];

    int nNotDef=-1, i;
    for( i = 0; i < nGlyphCount; ++i )
    {
        aTempEncs[i] = pEncoding[i];
        sal_GlyphId aGlyphId = pGlyphIds[i] & GF_IDXMASK;
        if( pGlyphIds[i] & GF_ISCHAR )
        {
            sal_Unicode cChar = static_cast<sal_Unicode>(aGlyphId); // TODO: sal_UCS4
            const bool bVertical = ((pGlyphIds[i] & (GF_ROTMASK|GF_GSUB)) != 0);
            aGlyphId = ::MapChar( aSftTTF.get(), cChar, bVertical );
            if( (aGlyphId == 0) && pFont->IsSymbolFont() )
            {
                // #i12824# emulate symbol aliasing U+FXXX <-> U+0XXX
                cChar = (cChar & 0xF000) ? (cChar & 0x00FF) : (cChar | 0xF000);
                aGlyphId = ::MapChar( aSftTTF.get(), cChar, bVertical );
            }
        }
        aShortIDs[i] = static_cast<sal_uInt16>( aGlyphId );
        if( !aGlyphId )
            if( nNotDef < 0 )
                nNotDef = i; // first NotDef glyph found
    }

    if( nNotDef != 0 )
    {
        // add fake NotDef glyph if needed
        if( nNotDef < 0 )
            nNotDef = nGlyphCount++;

        // NotDef glyph must be in pos 0 => swap glyphids
        aShortIDs[ nNotDef ] = aShortIDs[0];
        aTempEncs[ nNotDef ] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }
    DBG_ASSERT( nGlyphCount < 257, "too many glyphs for subsetting" );

    // fill pWidth array
    TTSimpleGlyphMetrics* pMetrics =
        ::GetTTSimpleGlyphMetrics( aSftTTF.get(), aShortIDs, nGlyphCount, aIFSD.mbVertical );
    if( !pMetrics )
        return FALSE;
    sal_uInt16 nNotDefAdv   = pMetrics[0].adv;
    pMetrics[0].adv         = pMetrics[nNotDef].adv;
    pMetrics[nNotDef].adv   = nNotDefAdv;
    for( i = 0; i < nOrigCount; ++i )
        pGlyphWidths[i] = pMetrics[i].adv;
    free( pMetrics );

    // write subset into destination file
    nRC = ::CreateTTFromTTGlyphs( aSftTTF.get(), aToFile.getStr(), aShortIDs,
            aTempEncs, nGlyphCount, 0, NULL, 0 );
    return (nRC == SF_OK);
}

const void* WinSalGraphics::GetEmbedFontData( const PhysicalFontFace* pFont,
    const sal_Unicode* pUnicodes, sal_Int32* pCharWidths, size_t nLen,
    FontSubsetInfo& rInfo, long* pDataLen )
{
    // create matching FontSelectPattern
    // we need just enough to get to the font file data
    FontSelectPattern aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    // TODO: much better solution: move SetFont and restoration of old font to caller
    ScopedFont aOldFont(*this);
    SetFont( &aIFSD, 0 );

    // get the raw font file data
    RawFontData aRawFontData( getHDC() );
    *pDataLen = aRawFontData.size();
    if( !aRawFontData.get() )
        return NULL;

    // get important font properties
    TEXTMETRICA aTm;
    if( !::GetTextMetricsA( getHDC(), &aTm ) )
        *pDataLen = 0;
    const bool bPFA = (*aRawFontData.get() < 0x80);
    rInfo.m_nFontType = bPFA ? FontSubsetInfo::TYPE1_PFA : FontSubsetInfo::TYPE1_PFB;
    WCHAR aFaceName[64];
    sal_Int32 nFNLen = ::GetTextFaceW( getHDC(), 64, aFaceName );
    // #i59854# strip eventual null byte
    while( nFNLen > 0 && aFaceName[nFNLen-1] == 0 )
        nFNLen--;
    if( nFNLen == 0 )
        *pDataLen = 0;
    rInfo.m_aPSName     = OUString(reinterpret_cast<const sal_Unicode*>(aFaceName), nFNLen);
    rInfo.m_nAscent     = +aTm.tmAscent;
    rInfo.m_nDescent    = -aTm.tmDescent;
    rInfo.m_aFontBBox   = Rectangle( Point( -aTm.tmOverhang, -aTm.tmDescent ),
              Point( aTm.tmMaxCharWidth, aTm.tmAscent+aTm.tmExternalLeading ) );
    rInfo.m_nCapHeight  = aTm.tmAscent; // Well ...

    // get individual character widths
    for (size_t i = 0; i < nLen; ++i)
    {
        int nCharWidth = 0;
        const sal_Unicode cChar = pUnicodes[i];
        if( !::GetCharWidth32W( getHDC(), cChar, cChar, &nCharWidth ) )
            *pDataLen = 0;
        pCharWidths[i] = nCharWidth;
    }

    if( !*pDataLen )
        return NULL;

    const unsigned char* pData = aRawFontData.steal();
    return (void*)pData;
}

void WinSalGraphics::FreeEmbedFontData( const void* pData, long /*nLen*/ )
{
    delete[] reinterpret_cast<char*>(const_cast<void*>(pData));
}

const Ucs2SIntMap* WinSalGraphics::GetFontEncodingVector( const PhysicalFontFace* pFont, const Ucs2OStrMap** pNonEncoded, std::set<sal_Unicode> const**)
{
    // TODO: even for builtin fonts we get here... why?
    if( !pFont->IsEmbeddable() )
        return NULL;

    // fill the encoding vector
    // currently no nonencoded vector
    if( pNonEncoded )
        *pNonEncoded = NULL;

    const ImplWinFontData* pWinFontData = static_cast<const ImplWinFontData*>(pFont);
    const Ucs2SIntMap* pEncoding = pWinFontData->GetEncodingVector();
    if( pEncoding == NULL )
    {
        Ucs2SIntMap* pNewEncoding = new Ucs2SIntMap;
        for( sal_Unicode i = 32; i < 256; ++i )
            (*pNewEncoding)[i] = i;
        pWinFontData->SetEncodingVector( pNewEncoding );
    pEncoding = pNewEncoding;
    }

    return pEncoding;
}

void WinSalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                     bool bVertical,
                                     Int32Vector& rWidths,
                                     Ucs2UIntMap& rUnicodeEnc )
{
    // create matching FontSelectPattern
    // we need just enough to get to the font file data
    FontSelectPattern aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    // TODO: much better solution: move SetFont and restoration of old font to caller
    ScopedFont aOldFont(*this);

    float fScale = 0.0;
    HFONT hOldFont = 0;
    ImplDoSetFont( &aIFSD, fScale, hOldFont );

    if( pFont->IsSubsettable() )
    {
        // get raw font file data
        const RawFontData xRawFontData( getHDC() );
        if( !xRawFontData.get() )
            return;

        // open font file
        sal_uInt32 nFaceNum = 0;
        if( !*xRawFontData.get() )  // TTC candidate
            nFaceNum = ~0U;  // indicate "TTC font extracts only"

        ScopedTrueTypeFont aSftTTF;
        int nRC = aSftTTF.open( (void*)xRawFontData.get(), xRawFontData.size(), nFaceNum );
        if( nRC != SF_OK )
            return;

        int nGlyphs = GetTTGlyphCount( aSftTTF.get() );
        if( nGlyphs > 0 )
        {
            rWidths.resize(nGlyphs);
            std::vector<sal_uInt16> aGlyphIds(nGlyphs);
            for( int i = 0; i < nGlyphs; i++ )
                aGlyphIds[i] = sal_uInt16(i);
            TTSimpleGlyphMetrics* pMetrics = ::GetTTSimpleGlyphMetrics( aSftTTF.get(),
                                                                        &aGlyphIds[0],
                                                                        nGlyphs,
                                                                        bVertical );
            if( pMetrics )
            {
                for( int i = 0; i< nGlyphs; i++ )
                    rWidths[i] = pMetrics[i].adv;
                free( pMetrics );
                rUnicodeEnc.clear();
            }
            const ImplWinFontData* pWinFont = static_cast<const ImplWinFontData*>(pFont);
            FontCharMapPtr pMap = pWinFont->GetFontCharMap();
            DBG_ASSERT( pMap && pMap->GetCharCount(), "no map" );

            int nCharCount = pMap->GetCharCount();
            sal_uInt32 nChar = pMap->GetFirstChar();
            for( int i = 0; i < nCharCount; i++ )
            {
                if( nChar < 0x00010000 )
                {
                    sal_uInt16 nGlyph = ::MapChar( aSftTTF.get(),
                                                   static_cast<sal_Ucs>(nChar),
                                                   bVertical );
                    if( nGlyph )
                        rUnicodeEnc[ static_cast<sal_Unicode>(nChar) ] = nGlyph;
                }
                nChar = pMap->GetNextChar( nChar );
            }

            pMap = 0;
        }
    }
    else if( pFont->IsEmbeddable() )
    {
        // get individual character widths
        rWidths.clear();
        rUnicodeEnc.clear();
        rWidths.reserve( 224 );
        for( sal_Unicode i = 32; i < 256; ++i )
        {
            int nCharWidth = 0;
            if( ::GetCharWidth32W( getHDC(), i, i, &nCharWidth ) )
            {
                rUnicodeEnc[ i ] = rWidths.size();
                rWidths.push_back( nCharWidth );
            }
        }
    }
}

void WinSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{}

SystemFontData WinSalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    SystemFontData aSysFontData;

    if (nFallbacklevel >= MAX_FALLBACK) nFallbacklevel = MAX_FALLBACK - 1;
    if (nFallbacklevel < 0 ) nFallbacklevel = 0;

    aSysFontData.hFont = mhFonts[nFallbacklevel];

    OSL_TRACE("\r\n:WinSalGraphics::GetSysFontData(): FontID: %p, Fallback level: %d",
              aSysFontData.hFont, nFallbacklevel);

    return aSysFontData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
