/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string.h>
#include <malloc.h>

#include <prewin.h>
#include <postwin.h>
#include <vcl/sysdata.hxx>
#include "tools/svwin.h"

#include "wincomp.hxx"
#include "saldata.hxx"
#include "salgdi.h"

#include "vcl/svapp.hxx"
#include "vcl/outfont.hxx"
#include "vcl/font.hxx"
#include "vcl/fontsubset.hxx"
#include "vcl/sallayout.hxx"

#include "vcl/outdev.h"         // for ImplGlyphFallbackFontSubstitution
#include "unotools/fontcfg.hxx" // for IMPL_FONT_ATTR_SYMBOL

#include "rtl/logfile.hxx"
#include "rtl/tencinfo.h"
#include "rtl/textcvt.h"
#include "rtl/bootstrap.hxx"

#include "i18npool/mslangid.hxx"

#include "osl/module.h"
#include "osl/file.hxx"
#include "osl/thread.hxx"
#include "osl/process.h"

#include "tools/poly.hxx"
#include "tools/debug.hxx"
#include "tools/stream.hxx"

#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolypolygon.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include "sft.hxx"

#ifdef GCP_KERN_HACK
#include <algorithm>
#endif

#ifdef ENABLE_GRAPHITE
#include <graphite/GrClient.h>
#include <graphite/WinFont.h>
#endif

#include <vector>
#include <set>
#include <map>

using namespace vcl;

static const int MAXFONTHEIGHT = 2048;

// -----------
// - Inlines -
// -----------

inline FIXED FixedFromDouble( double d )
{
    const long l = (long) ( d * 65536. );
    return *(FIXED*) &l;
}

// -----------------------------------------------------------------------

inline int IntTimes256FromFixed(FIXED f)
{
    int nFixedTimes256 = (f.value << 8) + ((f.fract+0x80) >> 8);
    return nFixedTimes256;
}

// =======================================================================

// these variables can be static because they store system wide settings
static bool bImplSalCourierScalable = false;
static bool bImplSalCourierNew = false;


// =======================================================================

// -----------------------------------------------------------------------

// TODO: also support temporary TTC font files
typedef std::map< String, ImplDevFontAttributes > FontAttrMap;

class ImplFontAttrCache
{
private:
    FontAttrMap     aFontAttributes;
    rtl::OUString   aCacheFileName;
    String          aBaseURL;
    BOOL            bModified;

protected:
    String  OptimizeURL( const String& rURL ) const;

    enum{ MAGIC = 0x12349876 }; // change if fontattrcache format changes

public:
            ImplFontAttrCache( const String& rCacheFileName, const String& rBaseURL );
            ~ImplFontAttrCache();

    ImplDevFontAttributes  GetFontAttr( const String& rFontFileName ) const;
    void                   AddFontAttr( const String& rFontFileName, const ImplDevFontAttributes& );
};

ImplFontAttrCache::ImplFontAttrCache( const String& rFileNameURL, const String& rBaseURL ) : aBaseURL( rBaseURL )
{
    bModified = FALSE;
    aBaseURL.ToLowerAscii();    // Windows only, no problem...

    // open the cache file
    osl::FileBase::getSystemPathFromFileURL( rFileNameURL, aCacheFileName );
    SvFileStream aCacheFile( aCacheFileName, STREAM_READ );
    if( !aCacheFile.IsOpen() )
        return;

    // check the cache version
    sal_uInt32 nCacheMagic;
    aCacheFile >> nCacheMagic;
    if( nCacheMagic != ImplFontAttrCache::MAGIC )
        return;  // ignore cache and rewrite if no match

    // read the cache entries from the file
    String aFontFileURL, aFontName;
    ImplDevFontAttributes aDFA;
    for(;;)
    {
        aCacheFile.ReadByteString( aFontFileURL, RTL_TEXTENCODING_UTF8 );
        if( !aFontFileURL.Len() )
            break;
        aCacheFile.ReadByteString( aDFA.maName, RTL_TEXTENCODING_UTF8 );

        short n;
        aCacheFile >> n; aDFA.meWeight     = static_cast<FontWeight>(n);
        aCacheFile >> n; aDFA.meItalic     = static_cast<FontItalic>(n);
        aCacheFile >> n; aDFA.mePitch      = static_cast<FontPitch>(n);
        aCacheFile >> n; aDFA.meWidthType  = static_cast<FontWidth>(n);
        aCacheFile >> n; aDFA.meFamily     = static_cast<FontFamily>(n);
        aCacheFile >> n; aDFA.mbSymbolFlag = (n != 0);

        aCacheFile.ReadByteStringLine( aDFA.maStyleName, RTL_TEXTENCODING_UTF8 );

        aFontAttributes[ aFontFileURL ] = aDFA;
    }
}

ImplFontAttrCache::~ImplFontAttrCache()
{
    if ( bModified )
    {
        SvFileStream aCacheFile( aCacheFileName, STREAM_WRITE|STREAM_TRUNC );
        if ( aCacheFile.IsWritable() )
        {
            sal_uInt32 nCacheMagic = ImplFontAttrCache::MAGIC;
            aCacheFile << nCacheMagic;

            // write the cache entries to the file
            FontAttrMap::const_iterator aIter = aFontAttributes.begin();
            while ( aIter != aFontAttributes.end() )
            {
                const String rFontFileURL( (*aIter).first );
                const ImplDevFontAttributes& rDFA( (*aIter).second );
                aCacheFile.WriteByteString( rFontFileURL, RTL_TEXTENCODING_UTF8 );
                aCacheFile.WriteByteString( rDFA.maName, RTL_TEXTENCODING_UTF8 );

                aCacheFile << static_cast<short>(rDFA.meWeight);
                aCacheFile << static_cast<short>(rDFA.meItalic);
                aCacheFile << static_cast<short>(rDFA.mePitch);
                aCacheFile << static_cast<short>(rDFA.meWidthType);
                aCacheFile << static_cast<short>(rDFA.meFamily);
                aCacheFile << static_cast<short>(rDFA.mbSymbolFlag != false);

                aCacheFile.WriteByteStringLine( rDFA.maStyleName, RTL_TEXTENCODING_UTF8 );

                ++aIter;
            }
            // EOF Marker
            String aEmptyStr;
            aCacheFile.WriteByteString( aEmptyStr, RTL_TEXTENCODING_UTF8 );
        }
    }
}

String ImplFontAttrCache::OptimizeURL( const String& rURL ) const
{
    String aOptimizedFontFileURL( rURL );
    aOptimizedFontFileURL.ToLowerAscii();   // Windows only, no problem...
    if ( aOptimizedFontFileURL.CompareTo( aBaseURL, aBaseURL.Len() ) == COMPARE_EQUAL )
        aOptimizedFontFileURL = aOptimizedFontFileURL.Copy( aBaseURL.Len() );
    return aOptimizedFontFileURL;
}

ImplDevFontAttributes ImplFontAttrCache::GetFontAttr( const String& rFontFileName ) const
{
    ImplDevFontAttributes aDFA;
    FontAttrMap::const_iterator it = aFontAttributes.find( OptimizeURL( rFontFileName ) );
    if( it != aFontAttributes.end() )
    {
        aDFA = it->second;
    }
    return aDFA;
}

void ImplFontAttrCache::AddFontAttr( const String& rFontFileName, const ImplDevFontAttributes& rDFA )
{
    DBG_ASSERT( rFontFileName.Len() && rDFA.maName.Len(), "ImplFontNameCache::AddFontName - invalid data!" );
    if ( rFontFileName.Len() && rDFA.maName.Len() )
    {
        aFontAttributes.insert( FontAttrMap::value_type( OptimizeURL( rFontFileName ), rDFA ) );
        bModified = TRUE;
    }
}

// =======================================================================

// raw font data with a scoped lifetime
class RawFontData
{
public:
    explicit    RawFontData( HDC, DWORD nTableTag=0 );
                ~RawFontData() { delete[] mpRawBytes; }
    const unsigned char*    get() const { return mpRawBytes; }
    const unsigned char*    steal() { unsigned char* p = mpRawBytes; mpRawBytes = NULL; return p; }
    const int               size() const { return mnByteCount; }

private:
    unsigned char*  mpRawBytes;
    int             mnByteCount;
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
    int nRawDataOfs = 0;
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

// ===========================================================================
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
    {0x0B00, 0x0B7F, LANGUAGE_ORIYA},               // Oriya
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
    {0x18A0, 0x18AF, LANGUAGE_MONGOLIAN},           // Mongolian
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
    // entries marked with default-CJK get replaced with the prefered CJK language
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
            nDefaultLang = Application::GetSettings().GetUILanguage();

        LanguageType nDefaultCJK = LANGUAGE_CHINESE;
        switch( nDefaultLang )
        {
            case LANGUAGE_JAPANESE:
            case LANGUAGE_KOREAN:
            case LANGUAGE_KOREAN_JOHAB:
            case LANGUAGE_CHINESE_SIMPLIFIED:
            case LANGUAGE_CHINESE_TRADITIONAL:
            case LANGUAGE_CHINESE_SINGAPORE:
            case LANGUAGE_CHINESE_HONGKONG:
            case LANGUAGE_CHINESE_MACAU:
                nDefaultCJK = nDefaultLang;
                break;
            default:
                nDefaultCJK = LANGUAGE_CHINESE;
                break;
        }

        // change the marked entries to prefered language
        static const int nCount = (sizeof(aLangFromCodeChart) / sizeof(*aLangFromCodeChart));
        for( int i = 0; i < nCount; ++i )
        {
            if( aLangFromCodeChart[ i].mnLangID == LANGUAGE_DEFAULT_CJK )
                aLangFromCodeChart[ i].mnLangID = nDefaultCJK;
        }
    }

    // binary search
    int nLow = 0;
    int nHigh = (sizeof(aLangFromCodeChart) / sizeof(*aLangFromCodeChart)) - 1;
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

    bool FindFontSubstitute( ImplFontSelectData&, rtl::OUString& rMissingChars ) const;
private:
    HDC mhDC;
    bool HasMissingChars( const ImplFontData*, const rtl::OUString& rMissingChars ) const;
};

inline WinGlyphFallbackSubstititution::WinGlyphFallbackSubstititution( HDC hDC )
:   mhDC( hDC )
{}

void ImplGetLogFontFromFontSelect( HDC, const ImplFontSelectData*,
    LOGFONTW&, bool /*bTestVerticalAvail*/ );

// does a font face hold the given missing characters?
bool WinGlyphFallbackSubstititution::HasMissingChars( const ImplFontData* pFace, const rtl::OUString& rMissingChars ) const
{
    const ImplWinFontData* pWinFont = static_cast<const ImplWinFontData*>(pFace);
    const ImplFontCharMap* pCharMap = pWinFont->GetImplFontCharMap();
    if( !pCharMap )
    {
        // construct a Size structure as the parameter of constructor of class ImplFontSelectData
        const Size aSize( pFace->GetWidth(), pFace->GetHeight() );
        // create a ImplFontSelectData object for getting s LOGFONT
        const ImplFontSelectData aFSD( *pFace, aSize, (float)aSize.Height(), 0, false );
        // construct log font
        LOGFONTW aLogFont;
        ImplGetLogFontFromFontSelect( mhDC, &aFSD, aLogFont, true );

        // create HFONT from log font
        HFONT hNewFont = ::CreateFontIndirectW( &aLogFont );
        // select the new font into device
        HFONT hOldFont = ::SelectFont( mhDC, hNewFont );

        // read CMAP table to update their pCharMap
        pWinFont->UpdateFromHDC( mhDC );;

        // cleanup temporary font
        ::SelectFont( mhDC, hOldFont );
        ::DeleteFont( hNewFont );

        // get the new charmap
        pCharMap = pWinFont->GetImplFontCharMap();
    }

    // avoid fonts with unknown CMAP subtables for glyph fallback
    if( !pCharMap || pCharMap->IsDefaultMap() )
        return false;

    int nMatchCount = 0;
    // static const int nMaxMatchCount = 1; // TODO: check more missing characters?
    const sal_Int32 nStrLen = rMissingChars.getLength();
    for( sal_Int32 nStrIdx = 0; nStrIdx < nStrLen; ++nStrIdx )
    {
        const sal_UCS4 uChar = rMissingChars.iterateCodePoints( &nStrIdx );
        nMatchCount += pCharMap->HasChar( uChar );
        break; // for now
    }

    const bool bHasMatches = (nMatchCount > 0);
    return bHasMatches;
}

// find a fallback font for missing characters
// TODO: should stylistic matches be searched and prefered?
bool WinGlyphFallbackSubstititution::FindFontSubstitute( ImplFontSelectData& rFontSelData, rtl::OUString& rMissingChars ) const
{
    // guess a locale matching to the missing chars
    com::sun::star::lang::Locale aLocale;

    sal_Int32 nStrIdx = 0;
    const sal_Int32 nStrLen = rMissingChars.getLength();
    while( nStrIdx < nStrLen )
    {
        const sal_UCS4 uChar = rMissingChars.iterateCodePoints( &nStrIdx );
        const LanguageType eLang = MapCharToLanguage( uChar );
        if( eLang == LANGUAGE_DONTKNOW )
            continue;
        MsLangId::convertLanguageToLocale( eLang, aLocale );
        break;
    }

    // fall back to default UI locale if the missing characters are inconclusive
    if( nStrIdx >= nStrLen )
        aLocale = Application::GetSettings().GetUILocale();

    // first level fallback:
    // try use the locale specific default fonts defined in VCL.xcu
    const ImplDevFontList* pDevFontList = ImplGetSVData()->maGDIData.mpScreenFontList;
    /*const*/ ImplDevFontListData* pDevFont = pDevFontList->ImplFindByLocale( aLocale );
    if( pDevFont )
    {
        const ImplFontData* pFace = pDevFont->FindBestFontFace( rFontSelData );
        if( HasMissingChars( pFace, rMissingChars ) )
        {
            rFontSelData.maSearchName = pDevFont->GetSearchName();
            return true;
        }
    }

    // are the missing characters symbols?
    pDevFont = pDevFontList->ImplFindByAttributes( IMPL_FONT_ATTR_SYMBOL,
                    rFontSelData.meWeight, rFontSelData.meWidthType,
                    rFontSelData.meFamily, rFontSelData.meItalic, rFontSelData.maSearchName );
    if( pDevFont )
    {
        const ImplFontData* pFace = pDevFont->FindBestFontFace( rFontSelData );
        if( HasMissingChars( pFace, rMissingChars ) )
        {
            rFontSelData.maSearchName = pDevFont->GetSearchName();
            return true;
        }
    }

    // last level fallback, check each font type face one by one
    const ImplGetDevFontList* pTestFontList = pDevFontList->GetDevFontList();
    // limit the count of fonts to be checked to prevent hangs
    static const int MAX_GFBFONT_COUNT = 600;
    int nTestFontCount = pTestFontList->Count();
    if( nTestFontCount > MAX_GFBFONT_COUNT )
        nTestFontCount = MAX_GFBFONT_COUNT;

    for( int i = 0; i < nTestFontCount; ++i )
    {
        const ImplFontData* pFace = pTestFontList->Get( i );
        if( !HasMissingChars( pFace, rMissingChars ) )
            continue;
        rFontSelData.maSearchName = pFace->maName;
        return true;
    }

    return false;
}

// =======================================================================

struct ImplEnumInfo
{
    HDC                 mhDC;
    ImplDevFontList*    mpList;
    String*             mpName;
    LOGFONTA*           mpLogFontA;
    LOGFONTW*           mpLogFontW;
    UINT                mnPreferedCharSet;
    bool                mbCourier;
    bool                mbImplSalCourierScalable;
    bool                mbImplSalCourierNew;
    bool                mbPrinter;
    int                 mnFontCount;
};

// =======================================================================

static CharSet ImplCharSetToSal( BYTE nCharSet )
{
    rtl_TextEncoding eTextEncoding;

    if ( nCharSet == OEM_CHARSET )
    {
        UINT nCP = (USHORT)GetOEMCP();
        switch ( nCP )
        {
            // It is unclear why these two (undefined?) code page numbers are
            // handled specially here:
            case 1004:  eTextEncoding = RTL_TEXTENCODING_MS_1252; break;
            case 65400: eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;
            default:
                eTextEncoding = rtl_getTextEncodingFromWindowsCodePage(nCP);
                break;
        };
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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

inline FontPitch ImplLogPitchToSal( BYTE nPitch )
{
    if ( nPitch & FIXED_PITCH )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}

// -----------------------------------------------------------------------

inline FontPitch ImplMetricPitchToSal( BYTE nPitch )
{
    // Sausaecke bei MS !! siehe NT Hilfe
    if ( !(nPitch & TMPF_FIXED_PITCH) )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}

// -----------------------------------------------------------------------

inline BYTE ImplPitchToWin( FontPitch ePitch )
{
    if ( ePitch == PITCH_FIXED )
        return FIXED_PITCH;
    else if ( ePitch == PITCH_VARIABLE )
        return VARIABLE_PITCH;
    else
        return DEFAULT_PITCH;
}

// -----------------------------------------------------------------------

static ImplDevFontAttributes WinFont2DevFontAttributes( const ENUMLOGFONTEXA& rEnumFont,
    const NEWTEXTMETRICA& rMetric, DWORD nFontType )
{
    ImplDevFontAttributes aDFA;

    const LOGFONTA rLogFont = rEnumFont.elfLogFont;

    // get font face attributes
    aDFA.meFamily       = ImplFamilyToSal( rLogFont.lfPitchAndFamily );
    aDFA.meWidthType    = WIDTH_DONTKNOW;
    aDFA.meWeight       = ImplWeightToSal( rLogFont.lfWeight );
    aDFA.meItalic       = (rLogFont.lfItalic) ? ITALIC_NORMAL : ITALIC_NONE;
    aDFA.mePitch        = ImplLogPitchToSal( rLogFont.lfPitchAndFamily );
    aDFA.mbSymbolFlag   = (rLogFont.lfCharSet == SYMBOL_CHARSET);

    // get the font face name
    aDFA.maName = ImplSalGetUniString( rLogFont.lfFaceName );

    // use the face's style name only if it looks reasonable
    const char* pStyleName = (const char*)rEnumFont.elfStyle;
    const char* pEnd = pStyleName + sizeof( rEnumFont.elfStyle );
    const char* p = pStyleName;
    for(; *p && (p < pEnd); ++p )
        if( (0x00 < *p) && (*p < 0x20) )
            break;
    if( p < pEnd )
        aDFA.maStyleName = ImplSalGetUniString( pStyleName );

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
        if( aDFA.maName.EqualsAscii( "AvantGarde" )
        ||  aDFA.maName.EqualsAscii( "Bookman" )
        ||  aDFA.maName.EqualsAscii( "Courier" )
        ||  aDFA.maName.EqualsAscii( "Helvetica" )
        ||  aDFA.maName.EqualsAscii( "NewCenturySchlbk" )
        ||  aDFA.maName.EqualsAscii( "Palatino" )
        ||  aDFA.maName.EqualsAscii( "Symbol" )
        ||  aDFA.maName.EqualsAscii( "Times" )
        ||  aDFA.maName.EqualsAscii( "ZapfChancery" )
        ||  aDFA.maName.EqualsAscii( "ZapfDingbats" ) )
            aDFA.mnQuality += 500;
    }

    // TODO: add alias names
    return aDFA;
}

// -----------------------------------------------------------------------

static ImplDevFontAttributes WinFont2DevFontAttributes( const ENUMLOGFONTEXW& rEnumFont,
    const NEWTEXTMETRICW& rMetric, DWORD nFontType )
{
    ImplDevFontAttributes aDFA;

    const LOGFONTW rLogFont = rEnumFont.elfLogFont;

    // get font face attributes
    aDFA.meFamily       = ImplFamilyToSal( rLogFont.lfPitchAndFamily );
    aDFA.meWidthType    = WIDTH_DONTKNOW;
    aDFA.meWeight       = ImplWeightToSal( rLogFont.lfWeight );
    aDFA.meItalic       = (rLogFont.lfItalic) ? ITALIC_NORMAL : ITALIC_NONE;
    aDFA.mePitch        = ImplLogPitchToSal( rLogFont.lfPitchAndFamily );
    aDFA.mbSymbolFlag   = (rLogFont.lfCharSet == SYMBOL_CHARSET);

    // get the font face name
    aDFA.maName = reinterpret_cast<const sal_Unicode*>(rLogFont.lfFaceName);

    // use the face's style name only if it looks reasonable
    const wchar_t* pStyleName = rEnumFont.elfStyle;
    const wchar_t* pEnd = pStyleName + sizeof(rEnumFont.elfStyle)/sizeof(*rEnumFont.elfStyle);
    const wchar_t* p = pStyleName;
    for(; *p && (p < pEnd); ++p )
        if( *p < 0x0020 )
            break;
    if( p < pEnd )
        aDFA.maStyleName = reinterpret_cast<const sal_Unicode*>(pStyleName);

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
        if( aDFA.maName.EqualsAscii( "AvantGarde" )
        ||  aDFA.maName.EqualsAscii( "Bookman" )
        ||  aDFA.maName.EqualsAscii( "Courier" )
        ||  aDFA.maName.EqualsAscii( "Helvetica" )
        ||  aDFA.maName.EqualsAscii( "NewCenturySchlbk" )
        ||  aDFA.maName.EqualsAscii( "Palatino" )
        ||  aDFA.maName.EqualsAscii( "Symbol" )
        ||  aDFA.maName.EqualsAscii( "Times" )
        ||  aDFA.maName.EqualsAscii( "ZapfChancery" )
        ||  aDFA.maName.EqualsAscii( "ZapfDingbats" ) )
            aDFA.mnQuality += 500;
    }

    // TODO: add alias names
    return aDFA;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void ImplSalLogFontToFontA( HDC hDC, const LOGFONTA& rLogFont, Font& rFont )
{
    String aFontName( ImplSalGetUniString( rLogFont.lfFaceName ) );
    if ( aFontName.Len() )
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

// -----------------------------------------------------------------------

void ImplSalLogFontToFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont )
{
    XubString aFontName( reinterpret_cast<const xub_Unicode*>(rLogFont.lfFaceName) );
    if ( aFontName.Len() )
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

// =======================================================================

ImplWinFontData::ImplWinFontData( const ImplDevFontAttributes& rDFS,
    int nHeight, WIN_BYTE eWinCharSet, WIN_BYTE nPitchAndFamily )
:   ImplFontData( rDFS, 0 ),
    meWinCharSet( eWinCharSet ),
    mnPitchAndFamily( nPitchAndFamily ),
    mpFontCharSets( NULL ),
    mpUnicodeMap( NULL ),
    mbGsubRead( false ),
    mbDisableGlyphApi( false ),
    mbHasKoreanRange( false ),
    mbHasCJKSupport( false ),
#ifdef ENABLE_GRAPHITE
    mbHasGraphiteSupport( false ),
#endif
    mbHasArabicSupport ( false ),
    mbFontCapabilitiesRead( false ),
    mbAliasSymbolsLow( false ),
    mbAliasSymbolsHigh( false ),
    mnId( 0 ),
    mpEncodingVector( NULL )
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
}

// -----------------------------------------------------------------------

ImplWinFontData::~ImplWinFontData()
{
    delete[] mpFontCharSets;

    if( mpUnicodeMap )
        mpUnicodeMap->DeReference();
    delete mpEncodingVector;
}

// -----------------------------------------------------------------------

sal_IntPtr ImplWinFontData::GetFontId() const
{
    return mnId;
}

// -----------------------------------------------------------------------

void ImplWinFontData::UpdateFromHDC( HDC hDC ) const
{
    // short circuit if already initialized
    if( mpUnicodeMap != NULL )
        return;

    ReadCmapTable( hDC );
    GetFontCapabilities( hDC );
#ifdef ENABLE_GRAPHITE
    static const char* pDisableGraphiteText = getenv( "SAL_DISABLE_GRAPHITE" );
    if( !pDisableGraphiteText || (pDisableGraphiteText[0] == '0') )
    {
        mbHasGraphiteSupport = gr::WinFont::FontHasGraphiteTables(hDC);
    }
#endif

    // even if the font works some fonts have problems with the glyph API
    // => the heuristic below tries to figure out which fonts have the problem
    TEXTMETRICA aTextMetric;
    if( ::GetTextMetricsA( hDC, &aTextMetric ) )
        if( !(aTextMetric.tmPitchAndFamily & TMPF_TRUETYPE)
        ||   (aTextMetric.tmPitchAndFamily & TMPF_DEVICE) )
            mbDisableGlyphApi = true;

}

// -----------------------------------------------------------------------

bool ImplWinFontData::HasGSUBstitutions( HDC hDC ) const
{
    if( !mbGsubRead )
        ReadGsubTable( hDC );
    return !maGsubTable.empty();
}

// -----------------------------------------------------------------------

bool ImplWinFontData::IsGSUBstituted( sal_UCS4 cChar ) const
{
    return( maGsubTable.find( cChar ) != maGsubTable.end() );
}

// -----------------------------------------------------------------------

ImplFontCharMap* ImplWinFontData::GetImplFontCharMap() const
{
    if( !mpUnicodeMap )
        return NULL;
    mpUnicodeMap->AddReference();
    return mpUnicodeMap;
}

bool ImplWinFontData::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    rFontCapabilities = maFontCapabilities;
    return !rFontCapabilities.maUnicodeRange.empty() || !rFontCapabilities.maCodePageRange.empty();
}

// -----------------------------------------------------------------------

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8)+p[1]);}
//static signed GetSShort( const unsigned char* p ){ return((short)((p[0]<<8)+p[1]));}
static inline DWORD CalcTag( const char p[4]) { return (p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24)); }

// -----------------------------------------------------------------------

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
            if( ::MapChar( pTTFont, cChar, 0 ) != ::MapChar( pTTFont, cChar, 1 ) )
                maGsubTable.insert( cChar ); // insert GSUBbed unicodes

    CloseTTFont( pTTFont );
}

// -----------------------------------------------------------------------

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
        mbDisableGlyphApi |= aResult.mbRecoded;
        aResult.mbSymbolic = bIsSymbolFont;
        if( aResult.mnRangeCount > 0 )
            mpUnicodeMap = new ImplFontCharMap( aResult );
    }

    if( !mpUnicodeMap )
        mpUnicodeMap = ImplFontCharMap::GetDefaultMap( bIsSymbolFont );
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

// =======================================================================

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

    ::SetTextColor( mhDC, aCol );
}

// -----------------------------------------------------------------------

int CALLBACK SalEnumQueryFontProcExW( const ENUMLOGFONTEXW*,
                                      const NEWTEXTMETRICEXW*,
                                      DWORD, LPARAM lParam )
{
    *((bool*)(void*)lParam) = true;
    return 0;
}

// -----------------------------------------------------------------------

bool ImplIsFontAvailable( HDC hDC, const UniString& rName )
{
    bool bAvailable = false;

    // Test, if Font available
    LOGFONTW aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );
    aLogFont.lfCharSet = DEFAULT_CHARSET;

    UINT nNameLen = rName.Len();
    if ( nNameLen > (sizeof( aLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
        nNameLen = (sizeof( aLogFont.lfFaceName )/sizeof( wchar_t ))-1;
    memcpy( aLogFont.lfFaceName, rName.GetBuffer(), nNameLen*sizeof( wchar_t ) );
    aLogFont.lfFaceName[nNameLen] = 0;

    EnumFontFamiliesExW( hDC, &aLogFont, (FONTENUMPROCW)SalEnumQueryFontProcExW,
                         (LPARAM)(void*)&bAvailable, 0 );

    return bAvailable;
}

// -----------------------------------------------------------------------

void ImplGetLogFontFromFontSelect( HDC hDC,
                                   const ImplFontSelectData* pFont,
                                   LOGFONTW& rLogFont,
                                   bool /*bTestVerticalAvail*/ )
{
    UniString   aName;
    if ( pFont->mpFontData )
        aName = pFont->mpFontData->maName;
    else
        aName = pFont->maName.GetToken( 0 );

    UINT nNameLen = aName.Len();
    if ( nNameLen > (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
        nNameLen = (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1;
    memcpy( rLogFont.lfFaceName, aName.GetBuffer(), nNameLen*sizeof( wchar_t ) );
    rLogFont.lfFaceName[nNameLen] = 0;

    if( !pFont->mpFontData )
    {
        rLogFont.lfCharSet = pFont->IsSymbolFont() ? SYMBOL_CHARSET : DEFAULT_CHARSET;
        rLogFont.lfPitchAndFamily = ImplPitchToWin( pFont->mePitch )
                                  | ImplFamilyToWin( pFont->meFamily );
    }
    else
    {
        const ImplWinFontData* pWinFontData = static_cast<const ImplWinFontData*>( pFont->mpFontData );
        rLogFont.lfCharSet        = pWinFontData->GetCharSet();
        rLogFont.lfPitchAndFamily = pWinFontData->GetPitchAndFamily();
    }

    rLogFont.lfWeight          = ImplWeightToWin( pFont->meWeight );
    rLogFont.lfHeight          = (LONG)-pFont->mnHeight;
    rLogFont.lfWidth           = (LONG)pFont->mnWidth;
    rLogFont.lfUnderline       = 0;
    rLogFont.lfStrikeOut       = 0;
    rLogFont.lfItalic          = (pFont->meItalic) != ITALIC_NONE;
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
            memcpy( &rLogFont.lfFaceName[0], aName.GetBuffer(), nNameLen*sizeof(wchar_t) );
            if( nNameLen < LF_FACESIZE )
                rLogFont.lfFaceName[nNameLen] = '\0';
        }
    }
}

// -----------------------------------------------------------------------

HFONT WinSalGraphics::ImplDoSetFont( ImplFontSelectData* i_pFont, float& o_rFontScale, HFONT& o_rOldFont )
{
    HFONT hNewFont = 0;

    HDC hdcScreen = 0;
    if( mbVirDev )
        // only required for virtual devices, see below for details
        hdcScreen = GetDC(0);

    LOGFONTW aLogFont;
    ImplGetLogFontFromFontSelect( mhDC, i_pFont, aLogFont, true );

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
    o_rOldFont = ::SelectFont( mhDC, hNewFont );

    TEXTMETRICW aTextMetricW;
    if( !::GetTextMetricsW( mhDC, &aTextMetricW ) )
    {
        // the selected font doesn't work => try a replacement
        // TODO: use its font fallback instead
        lstrcpynW( aLogFont.lfFaceName, L"Courier New", 11 );
        aLogFont.lfPitchAndFamily = FIXED_PITCH;
        HFONT hNewFont2 = CreateFontIndirectW( &aLogFont );
        SelectFont( mhDC, hNewFont2 );
        DeleteFont( hNewFont );
        hNewFont = hNewFont2;
    }

    if( hdcScreen )
        ::ReleaseDC( NULL, hdcScreen );

    return hNewFont;
}

USHORT WinSalGraphics::SetFont( ImplFontSelectData* pFont, int nFallbackLevel )
{
    // return early if there is no new font
    if( !pFont )
    {
        // deselect still active font
        if( mhDefFont )
            ::SelectFont( mhDC, mhDefFont );
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
    HFONT hNewFont = ImplDoSetFont( pFont, mfFontScale, hOldFont );

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
        mpWinFontData[ nFallbackLevel ]->UpdateFromHDC( mhDC );

    if( !nFallbackLevel )
    {
        mbFontKernInit = TRUE;
        if ( mpFontKernPairs )
        {
            delete[] mpFontKernPairs;
            mpFontKernPairs = NULL;
        }
        mnFontKernPairCount = 0;
    }

    mnFontCharSetCount = 0;

    // some printers have higher internal resolution, so their
    // text output would be different from what we calculated
    // => suggest DrawTextArray to workaround this problem
    if ( mbPrinter )
        return SAL_SETFONT_USEDRAWTEXTARRAY;
    else
        return 0;
}

// -----------------------------------------------------------------------

void WinSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel )
{
    // temporarily change the HDC to the font in the fallback level
    HFONT hOldFont = SelectFont( mhDC, mhFonts[nFallbackLevel] );

    wchar_t aFaceName[LF_FACESIZE+60];
    if( ::GetTextFaceW( mhDC, sizeof(aFaceName)/sizeof(wchar_t), aFaceName ) )
        pMetric->maName = reinterpret_cast<const sal_Unicode*>(aFaceName);

    // get the font metric
    TEXTMETRICA aWinMetric;
    const bool bOK = GetTextMetricsA( mhDC, &aWinMetric );
    // restore the HDC to the font in the base level
    SelectFont( mhDC, hOldFont );
    if( !bOK )
        return;

    // device independent font attributes
    pMetric->meFamily       = ImplFamilyToSal( aWinMetric.tmPitchAndFamily );;
    pMetric->mbSymbolFlag   = (aWinMetric.tmCharSet == SYMBOL_CHARSET);
    pMetric->meWeight       = ImplWeightToSal( aWinMetric.tmWeight );
    pMetric->mePitch        = ImplMetricPitchToSal( aWinMetric.tmPitchAndFamily );
    pMetric->meItalic       = aWinMetric.tmItalic ? ITALIC_NORMAL : ITALIC_NONE;
    pMetric->mnSlant        = 0;

    // device dependend font attributes
    pMetric->mbDevice       = (aWinMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;
    pMetric->mbScalableFont = (aWinMetric.tmPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE)) != 0;
    if( pMetric->mbScalableFont )
    {
        // check if there are kern pairs
        // TODO: does this work with GPOS kerning?
        DWORD nKernPairs = ::GetKerningPairsA( mhDC, 0, NULL );
        pMetric->mbKernableFont = (nKernPairs > 0);
    }
    else
    {
        // bitmap fonts cannot be rotated directly
        pMetric->mnOrientation  = 0;
        // bitmap fonts have no kerning
        pMetric->mbKernableFont = false;
    }

    // transformation dependend font metrics
    pMetric->mnWidth        = static_cast<int>( mfFontScale * aWinMetric.tmAveCharWidth );
    pMetric->mnIntLeading   = static_cast<int>( mfFontScale * aWinMetric.tmInternalLeading );
    pMetric->mnExtLeading   = static_cast<int>( mfFontScale * aWinMetric.tmExternalLeading );
    pMetric->mnAscent       = static_cast<int>( mfFontScale * aWinMetric.tmAscent );
    pMetric->mnDescent      = static_cast<int>( mfFontScale * aWinMetric.tmDescent );

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

// -----------------------------------------------------------------------

int CALLBACK SalEnumCharSetsProcExA( const ENUMLOGFONTEXA* pLogFont,
                                     const NEWTEXTMETRICEXA* /*pMetric*/,
                                     DWORD /*nFontType*/, LPARAM lParam )
{
    WinSalGraphics* pData = (WinSalGraphics*)lParam;
    // Charset already in the list?
    for ( BYTE i = 0; i < pData->mnFontCharSetCount; i++ )
    {
        if ( pData->mpFontCharSets[i] == pLogFont->elfLogFont.lfCharSet )
            return 1;
    }
    pData->mpFontCharSets[pData->mnFontCharSetCount] = pLogFont->elfLogFont.lfCharSet;
    pData->mnFontCharSetCount++;
    return 1;
}

// -----------------------------------------------------------------------

static void ImplGetAllFontCharSets( WinSalGraphics* pData )
{
    if ( !pData->mpFontCharSets )
        pData->mpFontCharSets = new BYTE[256];

    LOGFONTA aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );
    aLogFont.lfCharSet = DEFAULT_CHARSET;
    GetTextFaceA( pData->mhDC, sizeof( aLogFont.lfFaceName ), aLogFont.lfFaceName );
    EnumFontFamiliesExA( pData->mhDC, &aLogFont, (FONTENUMPROCA)SalEnumCharSetsProcExA,
                         (LPARAM)(void*)pData, 0 );
}

// -----------------------------------------------------------------------

static void ImplAddKerningPairs( WinSalGraphics* pData )
{
    ULONG nPairs = ::GetKerningPairsA( pData->mhDC, 0, NULL );
    if ( !nPairs )
        return;

    CHARSETINFO aInfo;
    if ( !TranslateCharsetInfo( (DWORD*)(ULONG)GetTextCharset( pData->mhDC ), &aInfo, TCI_SRCCHARSET ) )
        return;

    if ( !pData->mpFontKernPairs )
        pData->mpFontKernPairs = new KERNINGPAIR[nPairs];
    else
    {
        KERNINGPAIR* pOldPairs = pData->mpFontKernPairs;
        pData->mpFontKernPairs = new KERNINGPAIR[nPairs+pData->mnFontKernPairCount];
        memcpy( pData->mpFontKernPairs, pOldPairs,
                pData->mnFontKernPairCount*sizeof( KERNINGPAIR ) );
        delete[] pOldPairs;
    }

    UINT            nCP = aInfo.ciACP;
    ULONG           nOldPairs = pData->mnFontKernPairCount;
    KERNINGPAIR*    pTempPair = pData->mpFontKernPairs+pData->mnFontKernPairCount;
    nPairs = ::GetKerningPairsA( pData->mhDC, nPairs, pTempPair );
    for ( ULONG i = 0; i < nPairs; i++ )
    {
        unsigned char   aBuf[2];
        wchar_t         nChar;
        int             nLen;
        BOOL            bAdd = TRUE;

        // None-ASCII?, then we must convert the char
        if ( (pTempPair->wFirst > 125) || (pTempPair->wFirst == 92) )
        {
            if ( pTempPair->wFirst < 256 )
            {
                aBuf[0] = (unsigned char)pTempPair->wFirst;
                nLen = 1;
            }
            else
            {
                aBuf[0] = (unsigned char)(pTempPair->wFirst >> 8);
                aBuf[1] = (unsigned char)(pTempPair->wFirst & 0xFF);
                nLen = 2;
            }
            if ( MultiByteToWideChar( nCP, MB_PRECOMPOSED | MB_USEGLYPHCHARS,
                                      (const char*)aBuf, nLen, &nChar, 1 ) )
                pTempPair->wFirst = nChar;
            else
                bAdd = FALSE;
        }
        if ( (pTempPair->wSecond > 125) || (pTempPair->wSecond == 92) )
        {
            if ( pTempPair->wSecond < 256 )
            {
                aBuf[0] = (unsigned char)pTempPair->wSecond;
                nLen = 1;
            }
            else
            {
                aBuf[0] = (unsigned char)(pTempPair->wSecond >> 8);
                aBuf[1] = (unsigned char)(pTempPair->wSecond & 0xFF);
                nLen = 2;
            }
            if ( MultiByteToWideChar( nCP, MB_PRECOMPOSED | MB_USEGLYPHCHARS,
                                      (const char*)aBuf, nLen, &nChar, 1 ) )
                pTempPair->wSecond = nChar;
            else
                bAdd = FALSE;
        }

        // TODO: get rid of linear search!
        KERNINGPAIR* pTempPair2 = pData->mpFontKernPairs;
        for ( ULONG j = 0; j < nOldPairs; j++ )
        {
            if ( (pTempPair2->wFirst == pTempPair->wFirst) &&
                 (pTempPair2->wSecond == pTempPair->wSecond) )
            {
                bAdd = FALSE;
                break;
            }
            pTempPair2++;
        }

        if ( bAdd )
        {
            KERNINGPAIR* pDestPair = pData->mpFontKernPairs+pData->mnFontKernPairCount;
            if ( pDestPair != pTempPair )
                memcpy( pDestPair, pTempPair, sizeof( KERNINGPAIR ) );
            pData->mnFontKernPairCount++;
        }

        pTempPair++;
    }
}

// -----------------------------------------------------------------------

ULONG WinSalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
    DBG_ASSERT( sizeof( KERNINGPAIR ) == sizeof( ImplKernPairData ),
                "WinSalGraphics::GetKernPairs(): KERNINGPAIR != ImplKernPairData" );

    if ( mbFontKernInit )
    {
        if( mpFontKernPairs )
        {
            delete[] mpFontKernPairs;
            mpFontKernPairs = NULL;
        }
        mnFontKernPairCount = 0;

        KERNINGPAIR* pPairs = NULL;
        int nCount = ::GetKerningPairsW( mhDC, 0, NULL );
        if( nCount )
        {
#ifdef GCP_KERN_HACK
            pPairs = new KERNINGPAIR[ nCount+1 ];
            mpFontKernPairs = pPairs;
            mnFontKernPairCount = nCount;
            ::GetKerningPairsW( mhDC, nCount, pPairs );
#else // GCP_KERN_HACK
            pPairs = pKernPairs;
            nCount = (nCount < nPairs) : nCount : nPairs;
            ::GetKerningPairsW( mhDC, nCount, pPairs );
            return nCount;
#endif // GCP_KERN_HACK
        }

        mbFontKernInit = FALSE;

        std::sort( mpFontKernPairs, mpFontKernPairs + mnFontKernPairCount, ImplCmpKernData );
    }

    if( !pKernPairs )
        return mnFontKernPairCount;
    else if( mpFontKernPairs )
    {
        if ( nPairs < mnFontKernPairCount )
            nPairs = mnFontKernPairCount;
        memcpy( pKernPairs, mpFontKernPairs,
                nPairs*sizeof( ImplKernPairData ) );
        return nPairs;
    }

    return 0;
}

// -----------------------------------------------------------------------

ImplFontCharMap* WinSalGraphics::GetImplFontCharMap() const
{
    if( !mpWinFontData[0] )
        return ImplFontCharMap::GetDefaultMap();
    return mpWinFontData[0]->GetImplFontCharMap();
}

bool WinSalGraphics::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if( !mpWinFontData[0] )
        return false;
    return mpWinFontData[0]->GetImplFontCapabilities(rFontCapabilities);
}

// -----------------------------------------------------------------------

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
            String aName( ImplSalGetUniString( pLogFont->elfLogFont.lfFaceName ) );
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
        if ( pInfo->mnPreferedCharSet == pLogFont->elfLogFont.lfCharSet )
            pData->mnQuality += 100;

        // knowing Courier to be scalable is nice
        if( pInfo->mbCourier )
            pInfo->mbImplSalCourierScalable |= pData->IsScalable();

        pInfo->mpList->Add( pData );
    }

    return 1;
}

// -----------------------------------------------------------------------

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
            String aName( reinterpret_cast<const sal_Unicode*>(pLogFont->elfLogFont.lfFaceName) );
            pInfo->mpName = &aName;
            memcpy( pInfo->mpLogFontW->lfFaceName, pLogFont->elfLogFont.lfFaceName, (aName.Len()+1)*sizeof( wchar_t ) );
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

// -----------------------------------------------------------------------

struct TempFontItem
{
    ::rtl::OUString maFontFilePath;
    ::rtl::OString maResourcePath;
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

bool ImplAddTempFont( SalData& rSalData, const String& rFontFileURL )
{
    int nRet = 0;
    ::rtl::OUString aUSytemPath;
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
        ::rtl::OString aCFileName = rtl::OUStringToOString( aUSytemPath, theEncoding );
        // TODO: font should be private => need to investigate why it doesn't work then
        if( !::CreateScalableFontResourceA( 0, aResourceName, aCFileName.getStr(), NULL ) )
            return false;
        ++nCounter;

        nRet = ::AddFontResourceA( aResourceName );
        if( nRet > 0 )
        {
            TempFontItem* pNewItem = new TempFontItem;
            pNewItem->maResourcePath = rtl::OString( aResourceName );
            pNewItem->maFontFilePath = aUSytemPath.getStr();
            pNewItem->mpNextItem = rSalData.mpTempFontItem;
            rSalData.mpTempFontItem = pNewItem;
        }
    }

    return (nRet > 0);
}

// -----------------------------------------------------------------------

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
        ::PostMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, NULL );
#endif // FR_PRIVATE
}

// -----------------------------------------------------------------------

static bool ImplGetFontAttrFromFile( const String& rFontFileURL,
    ImplDevFontAttributes& rDFA )
{
    ::rtl::OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

    // get FontAttributes from a *fot file
    // TODO: use GetTTGlobalFontInfo() to access the font directly
    rDFA.mnQuality    = 1000;
    rDFA.mbDevice     = true;
    rDFA.meFamily     = FAMILY_DONTKNOW;
    rDFA.meWidthType  = WIDTH_DONTKNOW;
    rDFA.meWeight     = WEIGHT_DONTKNOW;
    rDFA.meItalic     = ITALIC_DONTKNOW;
    rDFA.mePitch      = PITCH_DONTKNOW;;
    rDFA.mbSubsettable= true;
    rDFA.mbEmbeddable = false;

    // Create temporary file name
    char aFileName[] = "soAAT.fot";
    char aResourceName[512];
    int nMaxLen = sizeof(aResourceName)/sizeof(*aResourceName) - 16;
    int nLen = ::GetTempPathA( nMaxLen, aResourceName );
    ::strncpy( aResourceName + nLen, aFileName, Max( 0, nMaxLen - nLen ));
    ::DeleteFileA( aResourceName );

    // Create font resource file (typically with a .fot file name extension).
    rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
    ::rtl::OString aCFileName = rtl::OUStringToOString( aUSytemPath, theEncoding );
    ::CreateScalableFontResourceA( 0, aResourceName, aCFileName.getStr(), NULL );

    // Open and read the font resource file
    rtl::OUString aFotFileName = rtl::OStringToOUString( aResourceName, osl_getThreadTextEncoding() );
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
    int i = 0x4F6;
    int nNameOfs = i;
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    // skip full name
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    // retrieve font style name
    int nStyleOfs = i;
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    if( i >= nBytesRead )
        return false;

    // convert byte strings to unicode
    rDFA.maName      = String( aBuffer + nNameOfs, osl_getThreadTextEncoding() );
    rDFA.maStyleName = String( aBuffer + nStyleOfs, osl_getThreadTextEncoding() );

    // byte offset 0x4C7: OS2_fsSelection
    const char nFSS = aBuffer[ 0x4C7 ];
    if( nFSS & 0x01 )   // italic
        rDFA.meItalic = ITALIC_NORMAL;
    //if( nFSS & 0x20 )   // bold
    //   rDFA.meWeight = WEIGHT_BOLD;
    if( nFSS & 0x40 )   // regular
    {
        rDFA.meWeight = WEIGHT_NORMAL;
        rDFA.meItalic = ITALIC_NONE;
    }

    // byte offsets 0x4D7/0x4D8: wingdi's FW_WEIGHT
    int nWinWeight = (aBuffer[0x4D7] & 0xFF) + ((aBuffer[0x4D8] & 0xFF) << 8);
    rDFA.meWeight = ImplWeightToSal( nWinWeight );

    rDFA.mbSymbolFlag = false;          // TODO
    rDFA.mePitch      = PITCH_DONTKNOW; // TODO

    // byte offset 0x4DE: pitch&family
    rDFA.meFamily = ImplFamilyToSal( aBuffer[0x4DE] );

    // byte offsets 0x4C8/0x4C9: emunits
    // byte offsets 0x4CE/0x4CF: winascent
    // byte offsets 0x4D0/0x4D1: winascent+windescent-emunits
    // byte offsets 0x4DF/0x4E0: avgwidth
    //...

    return true;
}

// -----------------------------------------------------------------------

bool WinSalGraphics::AddTempDevFont( ImplDevFontList* pFontList,
    const String& rFontFileURL, const String& rFontName )
{
    RTL_LOGFILE_TRACE1( "WinSalGraphics::AddTempDevFont(): %s", rtl::OUStringToOString( rFontFileURL, RTL_TEXTENCODING_UTF8 ).getStr() );

    ImplDevFontAttributes aDFA;
    aDFA.maName = rFontName;
    aDFA.mnQuality    = 1000;
    aDFA.mbDevice     = true;

    // Search Font Name in Cache
    if( !rFontName.Len() && mpFontAttrCache )
        aDFA = mpFontAttrCache->GetFontAttr( rFontFileURL );

    // Retrieve font name from font resource
    if( !aDFA.maName.Len() )
    {
        ImplGetFontAttrFromFile( rFontFileURL, aDFA );
        if( mpFontAttrCache && aDFA.maName.Len() )
            mpFontAttrCache->AddFontAttr( rFontFileURL, aDFA );
    }

    if ( !aDFA.maName.Len() )
        return false;

    // remember temp font for cleanup later
    if( !ImplAddTempFont( *GetSalData(), rFontFileURL ) )
        return false;

    UINT nPreferedCharSet = DEFAULT_CHARSET;

    // create matching FontData struct
    aDFA.mbSymbolFlag = false; // TODO: how to know it without accessing the font?
    aDFA.meFamily     = FAMILY_DONTKNOW;
    aDFA.meWidthType  = WIDTH_DONTKNOW;
    aDFA.meWeight     = WEIGHT_DONTKNOW;
    aDFA.meItalic     = ITALIC_DONTKNOW;
    aDFA.mePitch      = PITCH_DONTKNOW;;
    aDFA.mbSubsettable= true;
    aDFA.mbEmbeddable = false;

    /*
    // TODO: improve ImplDevFontAttributes using the "font resource file"
    aDFS.maName = // using "FONTRES:" from file
    if( rFontName != aDFS.maName )
        aDFS.maMapName = aFontName;
    */

    ImplWinFontData* pFontData = new ImplWinFontData( aDFA, 0,
        sal::static_int_cast<WIN_BYTE>(nPreferedCharSet),
        sal::static_int_cast<WIN_BYTE>(TMPF_VECTOR|TMPF_TRUETYPE) );
    pFontData->SetFontId( reinterpret_cast<sal_IntPtr>(pFontData) );
    pFontList->Add( pFontData );
    return true;
}

// -----------------------------------------------------------------------

void WinSalGraphics::GetDevFontList( ImplDevFontList* pFontList )
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
        ::rtl::OUString aPath;
        osl_getExecutableFile( &aPath.pData );
        ::rtl::OUString aExecutableFile( aPath );
        aPath = aPath.copy( 0, aPath.lastIndexOf('/') );
        String aFontDirUrl = aPath.copy( 0, aPath.lastIndexOf('/') );
        aFontDirUrl += String( RTL_CONSTASCII_USTRINGPARAM("/Basis/share/fonts/truetype") );

        // collect fonts in font path that could not be registered
        osl::Directory aFontDir( aFontDirUrl );
        osl::FileBase::RC rcOSL = aFontDir.open();
        if( rcOSL == osl::FileBase::E_None )
        {
            osl::DirectoryItem aDirItem;
            String aEmptyString;

            ::rtl::OUString aBootStrap;
            rtl::Bootstrap::get( String( RTL_CONSTASCII_USTRINGPARAM( "BRAND_BASE_DIR" ) ), aBootStrap );
            aBootStrap += String( RTL_CONSTASCII_USTRINGPARAM( "/program/" SAL_CONFIGFILE( "bootstrap" ) ) );
            rtl::Bootstrap aBootstrap( aBootStrap );
            ::rtl::OUString aUserPath;
            aBootstrap.getFrom( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UserInstallation" ) ), aUserPath );
            aUserPath += String( RTL_CONSTASCII_USTRINGPARAM("/user/config/fontnames.dat") );
            String aBaseURL = aPath.copy( 0, aPath.lastIndexOf('/')+1 );
            mpFontAttrCache = new ImplFontAttrCache( aUserPath, aBaseURL );

            while( aFontDir.getNextItem( aDirItem, 10 ) == osl::FileBase::E_None )
            {
                osl::FileStatus aFileStatus( FileStatusMask_FileURL );
                rcOSL = aDirItem.getFileStatus( aFileStatus );
                if ( rcOSL == osl::FileBase::E_None )
                    AddTempDevFont( pFontList, aFileStatus.getFileURL(), aEmptyString );
            }

            delete mpFontAttrCache; // destructor rewrites the cache file if needed
            mpFontAttrCache = NULL;
        }
    }

    ImplEnumInfo aInfo;
    aInfo.mhDC          = mhDC;
    aInfo.mpList        = pFontList;
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

    aInfo.mnPreferedCharSet = DEFAULT_CHARSET;
    DWORD nCP = GetACP();
    CHARSETINFO aCharSetInfo;
    if ( TranslateCharsetInfo( (DWORD*)nCP, &aCharSetInfo, TCI_SRCCODEPAGE ) )
        aInfo.mnPreferedCharSet = aCharSetInfo.ciCharset;

    LOGFONTW aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );
    aLogFont.lfCharSet = DEFAULT_CHARSET;
    aInfo.mpLogFontW = &aLogFont;
    EnumFontFamiliesExW( mhDC, &aLogFont,
        (FONTENUMPROCW)SalEnumFontsProcExW, (LPARAM)(void*)&aInfo, 0 );

    // Feststellen, was es fuer Courier-Schriften auf dem Bildschirm gibt,
    // um in SetFont() evt. Courier auf Courier New zu mappen
    if ( !mbPrinter )
    {
        bImplSalCourierScalable = aInfo.mbImplSalCourierScalable;
        bImplSalCourierNew      = aInfo.mbImplSalCourierNew;
    }

    // set glyph fallback hook
    static WinGlyphFallbackSubstititution aSubstFallback( mhDC );
    pFontList->SetFallbackHook( &aSubstFallback );
}

// ----------------------------------------------------------------------------

void WinSalGraphics::GetDevFontSubstList( OutputDevice* )
{}

// -----------------------------------------------------------------------

BOOL WinSalGraphics::GetGlyphBoundRect( long nIndex, Rectangle& rRect )
{
    HDC hDC = mhDC;

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_METRICS;
    if( !(nIndex & GF_ISCHAR) )
        nGGOFlags |= GGO_GLYPH_INDEX;
    nIndex &= GF_IDXMASK;

    GLYPHMETRICS aGM;
    aGM.gmptGlyphOrigin.x = aGM.gmptGlyphOrigin.y = 0;
    aGM.gmBlackBoxX = aGM.gmBlackBoxY = 0;
    DWORD nSize = GDI_ERROR;
    nSize = ::GetGlyphOutlineW( hDC, nIndex, nGGOFlags, &aGM, 0, NULL, &aMat );

    if( nSize == GDI_ERROR )
        return false;

    rRect = Rectangle( Point( +aGM.gmptGlyphOrigin.x, -aGM.gmptGlyphOrigin.y ),
        Size( aGM.gmBlackBoxX, aGM.gmBlackBoxY ) );
    rRect.Left()    = static_cast<int>( mfFontScale * rRect.Left() );
    rRect.Right()   = static_cast<int>( mfFontScale * rRect.Right() );
    rRect.Top()     = static_cast<int>( mfFontScale * rRect.Top() );
    rRect.Bottom()  = static_cast<int>( mfFontScale * rRect.Bottom() );
    return true;
}

// -----------------------------------------------------------------------

BOOL WinSalGraphics::GetGlyphOutline( long nIndex,
    ::basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    rB2DPolyPoly.clear();

    BOOL bRet = FALSE;
    HDC  hDC = mhDC;

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_NATIVE;
    if( !(nIndex & GF_ISCHAR) )
        nGGOFlags |= GGO_GLYPH_INDEX;
    nIndex &= GF_IDXMASK;

    GLYPHMETRICS aGlyphMetrics;
    DWORD nSize1 = GDI_ERROR;
    nSize1 = ::GetGlyphOutlineW( hDC, nIndex, nGGOFlags, &aGlyphMetrics, 0, NULL, &aMat );

    if( !nSize1 )       // blank glyphs are ok
        bRet = TRUE;
    else if( nSize1 != GDI_ERROR )
    {
        BYTE*   pData = new BYTE[ nSize1 ];
        DWORD   nSize2;
        nSize2 = ::GetGlyphOutlineW( hDC, nIndex, nGGOFlags,
                                     &aGlyphMetrics, nSize1, pData, &aMat );

        if( nSize1 == nSize2 )
        {
            bRet = TRUE;

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
                USHORT nPnt = 0;

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
                        for( USHORT i = 0; i < nPnt; ++i )
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
        }

        delete[] pData;
    }

    // rescaling needed for the PolyPolygon conversion
    if( rB2DPolyPoly.count() )
    {
        const double fFactor(mfFontScale/256);
        rB2DPolyPoly.transform(basegfx::tools::createScaleB2DHomMatrix(fFactor, fFactor));
    }

    return bRet;
}

// -----------------------------------------------------------------------

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
        SelectObject( m_rData.mhDC, m_hOrigFont );
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

BOOL WinSalGraphics::CreateFontSubset( const rtl::OUString& rToFile,
    const ImplFontData* pFont, long* pGlyphIDs, sal_uInt8* pEncoding,
    sal_Int32* pGlyphWidths, int nGlyphCount, FontSubsetInfo& rInfo )
{
    // TODO: use more of the central font-subsetting code, move stuff there if needed

    // create matching ImplFontSelectData
    // we need just enough to get to the font file data
    // use height=1000 for easier debugging (to match psprint's font units)
    ImplFontSelectData aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    // TODO: much better solution: move SetFont and restoration of old font to caller
    ScopedFont aOldFont(*this);
    float fScale = 1.0;
    HFONT hOldFont = 0;
    ImplDoSetFont( &aIFSD, fScale, hOldFont );

    ImplWinFontData* pWinFontData = (ImplWinFontData*)aIFSD.mpFontData;
    pWinFontData->UpdateFromHDC( mhDC );
/*const*/ ImplFontCharMap* pImplFontCharMap = pWinFontData->GetImplFontCharMap();

#if OSL_DEBUG_LEVEL > 1
    // get font metrics
    TEXTMETRICA aWinMetric;
    if( !::GetTextMetricsA( mhDC, &aWinMetric ) )
        return FALSE;

    DBG_ASSERT( !(aWinMetric.tmPitchAndFamily & TMPF_DEVICE), "cannot subset device font" );
    DBG_ASSERT( aWinMetric.tmPitchAndFamily & TMPF_TRUETYPE, "can only subset TT font" );
#endif

    rtl::OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return FALSE;
    const rtl_TextEncoding aThreadEncoding = osl_getThreadTextEncoding();
    const ByteString aToFile( aSysPath.getStr(), (xub_StrLen)aSysPath.getLength(), aThreadEncoding );

    // check if the font has a CFF-table
    const DWORD nCffTag = CalcTag( "CFF " );
    const RawFontData aRawCffData( mhDC, nCffTag );
    if( aRawCffData.get() )
    {
        long nRealGlyphIds[ 256 ];
        for( int i = 0; i < nGlyphCount; ++i )
        {
            // TODO: remap notdef glyph if needed
            // TODO: use GDI's GetGlyphIndices instead? Does it handle GSUB properly?
            sal_uInt32 nGlyphIdx = pGlyphIDs[i] & GF_IDXMASK;
            if( pGlyphIDs[i] & GF_ISCHAR ) // remaining pseudo-glyphs need to be translated
                nGlyphIdx = pImplFontCharMap->GetGlyphIndex( nGlyphIdx );
            if( (pGlyphIDs[i] & (GF_ROTMASK|GF_GSUB)) != 0) // TODO: vertical substitution
                {/*####*/}

            nRealGlyphIds[i] = nGlyphIdx;
        }

        // provide a font subset from the CFF-table
        FILE* pOutFile = fopen( aToFile.GetBuffer(), "wb" );
        rInfo.LoadFont( FontSubsetInfo::CFF_FONT, aRawCffData.get(), aRawCffData.size() );
        bool bRC = rInfo.CreateFontSubset( FontSubsetInfo::TYPE1_PFB, pOutFile, NULL,
                nRealGlyphIds, pEncoding, nGlyphCount, pGlyphWidths );
        fclose( pOutFile );
        return bRC;
    }

    // get raw font file data
    const RawFontData xRawFontData( mhDC, NULL );
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
    rInfo.m_nAscent     = +aTTInfo.winAscent;
    rInfo.m_nDescent    = -aTTInfo.winDescent;
    rInfo.m_aFontBBox   = Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
                                    Point( aTTInfo.xMax, aTTInfo.yMax ) );
    rInfo.m_nCapHeight  = aTTInfo.yMax; // Well ...

    // subset TTF-glyphs and get their properties
    // take care that subset fonts require the NotDef glyph in pos 0
    int nOrigCount = nGlyphCount;
    USHORT    aShortIDs[ 256 ];
    sal_uInt8 aTempEncs[ 256 ];

    int nNotDef=-1, i;
    for( i = 0; i < nGlyphCount; ++i )
    {
        aTempEncs[i] = pEncoding[i];
        sal_uInt32 nGlyphIdx = pGlyphIDs[i] & GF_IDXMASK;
        if( pGlyphIDs[i] & GF_ISCHAR )
        {
            sal_Unicode cChar = static_cast<sal_Unicode>(nGlyphIdx); // TODO: sal_UCS4
            const bool bVertical = ((pGlyphIDs[i] & (GF_ROTMASK|GF_GSUB)) != 0);
            nGlyphIdx = ::MapChar( aSftTTF.get(), cChar, bVertical );
            if( (nGlyphIdx == 0) && pFont->IsSymbolFont() )
            {
                // #i12824# emulate symbol aliasing U+FXXX <-> U+0XXX
                cChar = (cChar & 0xF000) ? (cChar & 0x00FF) : (cChar | 0xF000);
                nGlyphIdx = ::MapChar( aSftTTF.get(), cChar, bVertical );
            }
        }
        aShortIDs[i] = static_cast<USHORT>( nGlyphIdx );
        if( !nGlyphIdx )
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
    nRC = ::CreateTTFromTTGlyphs( aSftTTF.get(), aToFile.GetBuffer(), aShortIDs,
            aTempEncs, nGlyphCount, 0, NULL, 0 );
    return (nRC == SF_OK);
}

//--------------------------------------------------------------------------

const void* WinSalGraphics::GetEmbedFontData( const ImplFontData* pFont,
    const sal_Unicode* pUnicodes, sal_Int32* pCharWidths,
    FontSubsetInfo& rInfo, long* pDataLen )
{
    // create matching ImplFontSelectData
    // we need just enough to get to the font file data
    ImplFontSelectData aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    // TODO: much better solution: move SetFont and restoration of old font to caller
    ScopedFont aOldFont(*this);
    SetFont( &aIFSD, 0 );

    // get the raw font file data
    RawFontData aRawFontData( mhDC );
    *pDataLen = aRawFontData.size();
    if( !aRawFontData.get() )
        return NULL;

    // get important font properties
    TEXTMETRICA aTm;
    if( !::GetTextMetricsA( mhDC, &aTm ) )
        *pDataLen = 0;
    const bool bPFA = (*aRawFontData.get() < 0x80);
    rInfo.m_nFontType = bPFA ? FontSubsetInfo::TYPE1_PFA : FontSubsetInfo::TYPE1_PFB;
    WCHAR aFaceName[64];
    int nFNLen = ::GetTextFaceW( mhDC, 64, aFaceName );
    // #i59854# strip eventual null byte
    while( nFNLen > 0 && aFaceName[nFNLen-1] == 0 )
        nFNLen--;
    if( nFNLen == 0 )
        *pDataLen = 0;
    rInfo.m_aPSName     = String( reinterpret_cast<const sal_Unicode*>(aFaceName), sal::static_int_cast<USHORT>(nFNLen) );
    rInfo.m_nAscent     = +aTm.tmAscent;
    rInfo.m_nDescent    = -aTm.tmDescent;
    rInfo.m_aFontBBox   = Rectangle( Point( -aTm.tmOverhang, -aTm.tmDescent ),
              Point( aTm.tmMaxCharWidth, aTm.tmAscent+aTm.tmExternalLeading ) );
    rInfo.m_nCapHeight  = aTm.tmAscent; // Well ...

    // get individual character widths
    for( int i = 0; i < 256; ++i )
    {
        int nCharWidth = 0;
        const sal_Unicode cChar = pUnicodes[i];
        if( !::GetCharWidth32W( mhDC, cChar, cChar, &nCharWidth ) )
            *pDataLen = 0;
        pCharWidths[i] = nCharWidth;
    }

    if( !*pDataLen )
        return NULL;

    const unsigned char* pData = aRawFontData.steal();
    return (void*)pData;
}

//--------------------------------------------------------------------------

void WinSalGraphics::FreeEmbedFontData( const void* pData, long /*nLen*/ )
{
    delete[] reinterpret_cast<char*>(const_cast<void*>(pData));
}

//--------------------------------------------------------------------------

const Ucs2SIntMap* WinSalGraphics::GetFontEncodingVector( const ImplFontData* pFont, const Ucs2OStrMap** pNonEncoded )
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

//--------------------------------------------------------------------------

void WinSalGraphics::GetGlyphWidths( const ImplFontData* pFont,
                                     bool bVertical,
                                     Int32Vector& rWidths,
                                     Ucs2UIntMap& rUnicodeEnc )
{
    // create matching ImplFontSelectData
    // we need just enough to get to the font file data
    ImplFontSelectData aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    // TODO: much better solution: move SetFont and restoration of old font to caller
    ScopedFont aOldFont(*this);

    float fScale = 0.0;
    HFONT hOldFont = 0;
    ImplDoSetFont( &aIFSD, fScale, hOldFont );

    if( pFont->IsSubsettable() )
    {
        // get raw font file data
        const RawFontData xRawFontData( mhDC );
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
                                                                        bVertical ? 1 : 0 );
            if( pMetrics )
            {
                for( int i = 0; i< nGlyphs; i++ )
                    rWidths[i] = pMetrics[i].adv;
                free( pMetrics );
                rUnicodeEnc.clear();
            }
            const ImplWinFontData* pWinFont = static_cast<const ImplWinFontData*>(pFont);
            ImplFontCharMap* pMap = pWinFont->GetImplFontCharMap();
            DBG_ASSERT( pMap && pMap->GetCharCount(), "no map" );

            int nCharCount = pMap->GetCharCount();
            sal_uInt32 nChar = pMap->GetFirstChar();
            for( int i = 0; i < nCharCount; i++ )
            {
                if( nChar < 0x00010000 )
                {
                    sal_uInt16 nGlyph = ::MapChar( aSftTTF.get(),
                                                   static_cast<sal_Ucs>(nChar),
                                                   bVertical ? 1 : 0 );
                    if( nGlyph )
                        rUnicodeEnc[ static_cast<sal_Unicode>(nChar) ] = nGlyph;
                }
                nChar = pMap->GetNextChar( nChar );
            }
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
            if( ::GetCharWidth32W( mhDC, i, i, &nCharWidth ) )
            {
                rUnicodeEnc[ i ] = rWidths.size();
                rWidths.push_back( nCharWidth );
            }
        }
    }
}

//--------------------------------------------------------------------------

void WinSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{}

//--------------------------------------------------------------------------

SystemFontData WinSalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    SystemFontData aSysFontData;

    if (nFallbacklevel >= MAX_FALLBACK) nFallbacklevel = MAX_FALLBACK - 1;
    if (nFallbacklevel < 0 ) nFallbacklevel = 0;

    aSysFontData.nSize = sizeof( SystemFontData );
    aSysFontData.hFont = mhFonts[nFallbacklevel];
    aSysFontData.bFakeBold = false;
    aSysFontData.bFakeItalic = false;
    aSysFontData.bAntialias = true;
    aSysFontData.bVerticalCharacterType = false;

    OSL_TRACE("\r\n:WinSalGraphics::GetSysFontData(): FontID: %p, Fallback level: %d",
              aSysFontData.hFont,
              nFallbacklevel);

    return aSysFontData;
}

//--------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
