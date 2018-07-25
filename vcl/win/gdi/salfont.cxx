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
#include <map>
#include <memory>
#include <set>
#include <string.h>
#include <svsys.h>
#include <vector>

#include <o3tl/lru_map.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <rtl/tencinfo.h>
#include <o3tl/char16_t2wchar_t.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <unotools/fontcfg.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/metric.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>

#include <fontsubset.hxx>
#include <outdev.h>
#include <PhysicalFontCollection.hxx>
#include <PhysicalFontFace.hxx>
#include <sft.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>
#include <win/winlayout.hxx>
#include <impfontcharmap.hxx>
#include <impfontmetricdata.hxx>

using namespace vcl;

// GetGlyphOutlineW() seems to be a little slow, and doesn't seem to do its own caching (tested on Windows10).
// TODO include the font as part of the cache key, then we won't need to clear it on font change
// The cache limit is set by the rough number of characters needed to read your average Asian newspaper.
static o3tl::lru_map<sal_GlyphId, tools::Rectangle> g_BoundRectCache(3000);

static const int MAXFONTHEIGHT = 2048;

inline FIXED FixedFromDouble( double d )
{
    const long l = static_cast<long>( d * 65536. );
    return *reinterpret_cast<FIXED const *>(&l);
}

inline int IntTimes256FromFixed(FIXED f)
{
    int nFixedTimes256 = (f.value << 8) + ((f.fract+0x80) >> 8);
    return nFixedTimes256;
}

// raw font data with a scoped lifetime
class RawFontData
{
public:
    explicit    RawFontData( HDC, DWORD nTableTag=0 );
    const unsigned char*    get() const { return mpRawBytes.get(); }
    const unsigned char*    steal() { return mpRawBytes.release(); }
    int               size() const { return mnByteCount; }

private:
    std::unique_ptr<unsigned char[]> mpRawBytes;
    unsigned        mnByteCount;
};

RawFontData::RawFontData( HDC hDC, DWORD nTableTag )
:   mnByteCount( 0 )
{
    // get required size in bytes
    mnByteCount = ::GetFontData( hDC, nTableTag, 0, nullptr, 0 );
    if (mnByteCount == GDI_ERROR)
        mnByteCount = 0;
    if (!mnByteCount)
        return;

    // allocate the array
    mpRawBytes.reset(new unsigned char[ mnByteCount ]);

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
            mpRawBytes.get() + nRawDataOfs, nFDGet );
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
        mpRawBytes.reset();
        // mnByteCount must correspond to mpRawBytes length
        SAL_WARN( "vcl", "Raw data of font is incomplete: " << nRawDataOfs << " byte(s) found whereas " << mnByteCount << " byte(s) expected!" );
        mnByteCount = 0;
    }
}

// platform specific font substitution hooks for glyph fallback enhancement

class WinPreMatchFontSubstititution
:    public ImplPreMatchFontSubstitution
{
public:
    bool FindFontSubstitute(FontSelectPattern&) const override;
};

class WinGlyphFallbackSubstititution
:    public ImplGlyphFallbackFontSubstitution
{
public:
    explicit WinGlyphFallbackSubstititution()
        : mhDC(GetDC(nullptr))
    {
    };

    ~WinGlyphFallbackSubstititution() override
    {
        ReleaseDC(nullptr, mhDC);
    };

    bool FindFontSubstitute( FontSelectPattern&, OUString& rMissingChars ) const override;
private:
    HDC mhDC;
    bool HasMissingChars(PhysicalFontFace*, OUString& rMissingChars) const;
};

// does a font face hold the given missing characters?
bool WinGlyphFallbackSubstititution::HasMissingChars(PhysicalFontFace* pFace, OUString& rMissingChars) const
{
    WinFontFace* pWinFont = static_cast< WinFontFace* >(pFace);
    FontCharMapRef xFontCharMap = pWinFont->GetFontCharMap();
    if( !xFontCharMap.is() )
    {
        // construct a Size structure as the parameter of constructor of class FontSelectPattern
        const Size aSize( pFace->GetWidth(), pFace->GetHeight() );
        // create a FontSelectPattern object for getting s LOGFONT
        const FontSelectPattern aFSD( *pFace, aSize, static_cast<float>(aSize.Height()), 0, false );
        // construct log font
        LOGFONTW aLogFont;
        ImplGetLogFontFromFontSelect( mhDC, &aFSD, pFace, aLogFont );

        // create HFONT from log font
        HFONT hNewFont = ::CreateFontIndirectW( &aLogFont );
        // select the new font into device
        HFONT hOldFont = ::SelectFont( mhDC, hNewFont );

        // read CMAP table to update their xFontCharMap
        pWinFont->UpdateFromHDC( mhDC );

        // cleanup temporary font
        ::SelectFont( mhDC, hOldFont );
        ::DeleteFont( hNewFont );

        // get the new charmap
        xFontCharMap = pWinFont->GetFontCharMap();
    }

    // avoid fonts with unknown CMAP subtables for glyph fallback
    if( !xFontCharMap.is() || xFontCharMap->IsDefaultMap() )
        return false;

    int nMatchCount = 0;
    std::vector<sal_UCS4> rRemainingCodes;
    const sal_Int32 nStrLen = rMissingChars.getLength();
    sal_Int32 nStrIdx = 0;
    while (nStrIdx < nStrLen)
    {
        const sal_UCS4 uChar = rMissingChars.iterateCodePoints( &nStrIdx );
        if (xFontCharMap->HasChar(uChar))
            nMatchCount++;
        else
            rRemainingCodes.push_back(uChar);
    }

    xFontCharMap = nullptr;

    if (nMatchCount > 0)
        rMissingChars = OUString(rRemainingCodes.data(), rRemainingCodes.size());

    return nMatchCount > 0;
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
        return rFontCollection.FindFontFamilyByTokenNames(aDefault);
    }
}

// These are Win 3.1 bitmap fonts using "FON" font format
// which is not supported with DirectWrite so let's substitute them
// with a font that is supported and always available.
// Based on:
// https://dxr.mozilla.org/mozilla-esr10/source/gfx/thebes/gfxDWriteFontList.cpp#1057
static const std::map<OUString, OUString> aBitmapFontSubs =
{
    { "MS Sans Serif", "Microsoft Sans Serif" },
    { "MS Serif",      "Times New Roman" },
    { "Small Fonts",   "Arial" },
    { "Courier",       "Courier New" },
    { "Roman",         "Times New Roman" },
    { "Script",        "Mistral" }
};

// TODO: See if Windows have API that we can use here to improve font fallback.
bool WinPreMatchFontSubstititution::FindFontSubstitute(FontSelectPattern& rFontSelData) const
{
    if (rFontSelData.IsSymbolFont() || IsStarSymbol(rFontSelData.maSearchName))
        return false;

    for (const auto& aSub : aBitmapFontSubs)
    {
        if (rFontSelData.maSearchName == GetEnglishSearchFontName(aSub.first))
        {
            rFontSelData.maSearchName = aSub.second;
            return true;
        }
    }

    return false;
}

// find a fallback font for missing characters
// TODO: should stylistic matches be searched and preferred?
bool WinGlyphFallbackSubstititution::FindFontSubstitute( FontSelectPattern& rFontSelData, OUString& rMissingChars ) const
{
    // guess a locale matching to the missing chars
    LanguageType eLang = rFontSelData.meLanguage;
    LanguageTag aLanguageTag( eLang);

    // fall back to default UI locale if the font language is inconclusive
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
    pFontFamily = pFontCollection->FindFontFamilyByAttributes( ImplFontAttrs::Symbol,
                                                     rFontSelData.GetWeight(),
                                                     rFontSelData.GetWidthType(),
                                                     rFontSelData.GetItalic(),
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
    std::unique_ptr<ImplDeviceFontList> pTestFontList = pFontCollection->GetDeviceFontList();
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

    return bFound;
}

struct ImplEnumInfo
{
    HDC                 mhDC;
    PhysicalFontCollection* mpList;
    OUString*           mpName;
    LOGFONTW*           mpLogFont;
    bool                mbPrinter;
    int                 mnFontCount;
};

static rtl_TextEncoding ImplCharSetToSal( BYTE nCharSet )
{
    rtl_TextEncoding eTextEncoding;

    if ( nCharSet == OEM_CHARSET )
    {
        UINT nCP = static_cast<sal_uInt16>(GetOEMCP());
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

static FontAttributes WinFont2DevFontAttributes( const ENUMLOGFONTEXW& rEnumFont,
    const NEWTEXTMETRICW& rMetric)
{
    FontAttributes aDFA;

    const LOGFONTW rLogFont = rEnumFont.elfLogFont;

    // get font face attributes
    aDFA.SetFamilyType(ImplFamilyToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetWidthType(WIDTH_DONTKNOW);
    aDFA.SetWeight(ImplWeightToSal( rLogFont.lfWeight ));
    aDFA.SetItalic((rLogFont.lfItalic) ? ITALIC_NORMAL : ITALIC_NONE);
    aDFA.SetPitch(ImplLogPitchToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetSymbolFlag(rLogFont.lfCharSet == SYMBOL_CHARSET);

    // get the font face name
    aDFA.SetFamilyName(o3tl::toU(rLogFont.lfFaceName));

    // use the face's style name only if it looks reasonable
    const wchar_t* pStyleName = rEnumFont.elfStyle;
    const wchar_t* pEnd = pStyleName + sizeof(rEnumFont.elfStyle)/sizeof(*rEnumFont.elfStyle);
    const wchar_t* p = pStyleName;
    for(; *p && (p < pEnd); ++p )
        if( *p < 0x0020 )
            break;
    if( p < pEnd )
        aDFA.SetStyleName(o3tl::toU(pStyleName));

    // heuristics for font quality
    // -   opentypeTT > truetype
    aDFA.SetQuality( 0 );
    if( rMetric.tmPitchAndFamily & TMPF_TRUETYPE )
        aDFA.IncreaseQualityBy( 50 );
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE)) )
        aDFA.IncreaseQualityBy( 10 );

    // TODO: add alias names
    return aDFA;
}


static rtl::Reference<WinFontFace> ImplLogMetricToDevFontDataW( const ENUMLOGFONTEXW* pLogFont,
                                         const NEWTEXTMETRICW* pMetric,
                                         DWORD nFontType )
{
    int nHeight = 0;
    if ( nFontType & RASTER_FONTTYPE )
        nHeight = pMetric->tmHeight - pMetric->tmInternalLeading;

    rtl::Reference<WinFontFace> pData = new WinFontFace(
        WinFont2DevFontAttributes(*pLogFont, *pMetric),
        nHeight,
        pLogFont->elfLogFont.lfCharSet,
        pMetric->tmPitchAndFamily );

    return pData;
}

void ImplSalLogFontToFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont )
{
    OUString aFontName( o3tl::toU(rLogFont.lfFaceName) );
    if (!aFontName.isEmpty())
    {
        rFont.SetFamilyName( aFontName );
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
        rFont.SetFontSize( Size( 0, nFontHeight ) );
        rFont.SetOrientation( static_cast<short>(rLogFont.lfEscapement) );
        if ( rLogFont.lfItalic )
            rFont.SetItalic( ITALIC_NORMAL );
        else
            rFont.SetItalic( ITALIC_NONE );
        if ( rLogFont.lfUnderline )
            rFont.SetUnderline( LINESTYLE_SINGLE );
        else
            rFont.SetUnderline( LINESTYLE_NONE );
        if ( rLogFont.lfStrikeOut )
            rFont.SetStrikeout( STRIKEOUT_SINGLE );
        else
            rFont.SetStrikeout( STRIKEOUT_NONE );
    }
}

WinFontFace::WinFontFace( const FontAttributes& rDFS,
    int nHeight, BYTE eWinCharSet, BYTE nPitchAndFamily )
:   PhysicalFontFace( rDFS ),
    mnId( 0 ),
    mbFontCapabilitiesRead( false ),
    mxUnicodeMap( nullptr ),
    meWinCharSet( eWinCharSet ),
    mnPitchAndFamily( nPitchAndFamily ),
    mbAliasSymbolsHigh( false ),
    mbAliasSymbolsLow( false )
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

WinFontFace::~WinFontFace()
{
    mxUnicodeMap.clear();
}

sal_IntPtr WinFontFace::GetFontId() const
{
    return mnId;
}

rtl::Reference<LogicalFontInstance> WinFontFace::CreateFontInstance(const FontSelectPattern& rFSD) const
{
    return new WinFontInstance(*this, rFSD);
}

static inline DWORD CalcTag( const char p[5]) { return (p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24)); }

void WinFontFace::UpdateFromHDC( HDC hDC ) const
{
    // short circuit if already initialized
    if( mxUnicodeMap.is() )
        return;

    ReadCmapTable( hDC );
    GetFontCapabilities( hDC );
}

FontCharMapRef WinFontFace::GetFontCharMap() const
{
    return mxUnicodeMap;
}

bool WinFontFace::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    rFontCapabilities = maFontCapabilities;
    return rFontCapabilities.oUnicodeRange || rFontCapabilities.oCodePageRange;
}

void WinFontFace::ReadCmapTable( HDC hDC ) const
{
    if( mxUnicodeMap.is() )
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
            FontCharMapRef pUnicodeMap(new FontCharMap(aResult));
            mxUnicodeMap = pUnicodeMap;
        }
    }

    if( !mxUnicodeMap.is() )
    {
        mxUnicodeMap = FontCharMap::GetDefaultMap( bIsSymbolFont );
    }
}

void WinFontFace::GetFontCapabilities( HDC hDC ) const
{
    // read this only once per font
    if( mbFontCapabilitiesRead )
        return;

    mbFontCapabilitiesRead = true;

    // OS/2 table
    const DWORD OS2Tag = CalcTag( "OS/2" );
    DWORD nLength = ::GetFontData( hDC, OS2Tag, 0, nullptr, 0 );
    if( (nLength != GDI_ERROR) && nLength )
    {
        std::vector<unsigned char> aTable( nLength );
        unsigned char* pTable = &aTable[0];
        ::GetFontData( hDC, OS2Tag, 0, pTable, nLength );
        vcl::getTTCoverage(maFontCapabilities.oUnicodeRange, maFontCapabilities.oCodePageRange, pTable, nLength);
    }
}

void WinSalGraphics::SetTextColor( Color nColor )
{
    COLORREF aCol = PALETTERGB( nColor.GetRed(),
                                nColor.GetGreen(),
                                nColor.GetBlue() );

    if( !mbPrinter &&
        GetSalData()->mhDitherPal &&
        ImplIsSysColorEntry( nColor ) )
    {
        aCol = PALRGB_TO_RGB( aCol );
    }

    ::SetTextColor( getHDC(), aCol );
}

int CALLBACK SalEnumQueryFontProcExW( const LOGFONTW*,
                                      const TEXTMETRICW*,
                                      DWORD, LPARAM lParam )
{
    *reinterpret_cast<bool*>(lParam) = true;
    return 0;
}

void ImplGetLogFontFromFontSelect( HDC hDC,
                                   const FontSelectPattern* pFont,
                                   const PhysicalFontFace* pFontFace,
                                   LOGFONTW& rLogFont )
{
    OUString aName;
    if (!pFontFace && pFont->mpFontInstance)
        pFontFace = pFont->mpFontInstance->GetFontFace();

    if (pFontFace)
        aName = pFontFace->GetFamilyName();
    else
        aName = pFont->GetFamilyName().getToken( 0, ';' );

    UINT nNameLen = aName.getLength();
    if ( nNameLen > (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
        nNameLen = (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1;
    memcpy( rLogFont.lfFaceName, aName.getStr(), nNameLen*sizeof( wchar_t ) );
    rLogFont.lfFaceName[nNameLen] = 0;

    if  (pFontFace)
    {
        const WinFontFace* pWinFontData = static_cast<const WinFontFace*>(pFontFace);
        rLogFont.lfCharSet = pWinFontData->GetCharSet();
        rLogFont.lfPitchAndFamily = pWinFontData->GetPitchAndFamily();
    }
    else
    {
        rLogFont.lfCharSet = pFont->IsSymbolFont() ? SYMBOL_CHARSET : DEFAULT_CHARSET;
        rLogFont.lfPitchAndFamily = ImplPitchToWin( pFont->GetPitch() )
                                  | ImplFamilyToWin( pFont->GetFamilyType() );
    }

    static BYTE nDefaultQuality = NONANTIALIASED_QUALITY;
    if (nDefaultQuality == NONANTIALIASED_QUALITY)
    {
        if (OpenGLWrapper::isVCLOpenGLEnabled())
            nDefaultQuality = ANTIALIASED_QUALITY;
        else
            nDefaultQuality = DEFAULT_QUALITY;
    }

    rLogFont.lfWeight          = ImplWeightToWin( pFont->GetWeight() );
    rLogFont.lfHeight          = static_cast<LONG>(-pFont->mnHeight);
    rLogFont.lfWidth           = static_cast<LONG>(pFont->mnWidth);
    rLogFont.lfUnderline       = 0;
    rLogFont.lfStrikeOut       = 0;
    rLogFont.lfItalic          = BYTE(pFont->GetItalic() != ITALIC_NONE);
    rLogFont.lfEscapement      = pFont->mnOrientation;
    rLogFont.lfOrientation     = rLogFont.lfEscapement;
    rLogFont.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
    rLogFont.lfQuality         = nDefaultQuality;
    rLogFont.lfOutPrecision    = OUT_TT_PRECIS;
    if ( pFont->mnOrientation )
        rLogFont.lfClipPrecision |= CLIP_LH_ANGLES;

    // disable antialiasing if requested
    if ( pFont->mbNonAntialiased )
        rLogFont.lfQuality = NONANTIALIASED_QUALITY;

    // select vertical mode if requested and available
    if ( pFont->mbVertical && nNameLen )
    {
        // vertical fonts start with an '@'
        memmove( &rLogFont.lfFaceName[1], &rLogFont.lfFaceName[0],
            sizeof(rLogFont.lfFaceName)-sizeof(rLogFont.lfFaceName[0]) );
        rLogFont.lfFaceName[0] = '@';

        // check availability of vertical mode for this font
        bool bAvailable = false;
        EnumFontFamiliesExW( hDC, &rLogFont, SalEnumQueryFontProcExW,
                         reinterpret_cast<LPARAM>(&bAvailable), 0 );

        if( !bAvailable )
        {
            // restore non-vertical name if not vertical mode isn't available
            memcpy( &rLogFont.lfFaceName[0], aName.getStr(), nNameLen*sizeof(wchar_t) );
            if( nNameLen < LF_FACESIZE )
                rLogFont.lfFaceName[nNameLen] = '\0';
            // keep it upright and create the font for sideway glyphs later.
            rLogFont.lfEscapement = rLogFont.lfEscapement - 2700;
            rLogFont.lfOrientation = rLogFont.lfEscapement;
        }
    }
}

HFONT WinSalGraphics::ImplDoSetFont(FontSelectPattern const * i_pFont,
                                    const PhysicalFontFace * i_pFontFace,
                                    float& o_rFontScale,
                                    HFONT& o_rOldFont)
{
    // clear the cache on font change
    g_BoundRectCache.clear();
    HFONT hNewFont = nullptr;

    HDC hdcScreen = nullptr;
    if( mbVirDev )
        // only required for virtual devices, see below for details
        hdcScreen = GetDC(nullptr);

    LOGFONTW aLogFont;
    ImplGetLogFontFromFontSelect( getHDC(), i_pFont, i_pFontFace, aLogFont );

    // #i47675# limit font requests to MAXFONTHEIGHT
    // TODO: share MAXFONTHEIGHT font instance
    if( (-aLogFont.lfHeight <= MAXFONTHEIGHT)
    &&  (+aLogFont.lfWidth <= MAXFONTHEIGHT) )
    {
        o_rFontScale = 1.0;
    }
    else if( -aLogFont.lfHeight >= +aLogFont.lfWidth )
    {
        o_rFontScale = -aLogFont.lfHeight / float(MAXFONTHEIGHT);
        aLogFont.lfHeight = -MAXFONTHEIGHT;
        aLogFont.lfWidth = FRound( aLogFont.lfWidth / o_rFontScale );
    }
    else // #i95867# also limit font widths
    {
        o_rFontScale = +aLogFont.lfWidth / float(MAXFONTHEIGHT);
        aLogFont.lfWidth = +MAXFONTHEIGHT;
        aLogFont.lfHeight = FRound( aLogFont.lfHeight / o_rFontScale );
    }

    hNewFont = ::CreateFontIndirectW( &aLogFont );
    if( hdcScreen )
    {
        // select font into screen hdc first to get an antialiased font
        // and instantly restore the default font!
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
        lstrcpynW( aLogFont.lfFaceName, L"Courier New", 12 );
        aLogFont.lfPitchAndFamily = FIXED_PITCH;
        HFONT hNewFont2 = CreateFontIndirectW( &aLogFont );
        SelectFont( getHDC(), hNewFont2 );
        DeleteFont( hNewFont );
        hNewFont = hNewFont2;
    }

    if( hdcScreen )
        ::ReleaseDC( nullptr, hdcScreen );

    return hNewFont;
}

void WinSalGraphics::SetFont( const FontSelectPattern* pFont, int nFallbackLevel )
{
    // return early if there is no new font
    if( !pFont )
    {
        // deselect still active font
        if (mhDefFont)
        {
            ::SelectFont(getHDC(), mhDefFont);
            mhDefFont = nullptr;
        }
        mfCurrentFontScale = mfFontScale[nFallbackLevel];
        // release no longer referenced font handles
        for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
        {
            if( mhFonts[i] )
            {
                ::DeleteFont( mhFonts[i] );
                mhFonts[ i ] = nullptr;
            }
            mpWinFontEntry[i] = nullptr;
        }
        return;
    }

    // WinSalGraphics::GetEmbedFontData does not set mpFontInstance
    // since it is interested in font file data only.
    mpWinFontEntry[ nFallbackLevel ] = reinterpret_cast<WinFontInstance*>( pFont->mpFontInstance.get() );

    HFONT hOldFont = nullptr;
    HFONT hNewFont = ImplDoSetFont(pFont, nullptr, mfFontScale[ nFallbackLevel ], hOldFont);
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
                mhFonts[i] = nullptr;
            }
            if (i > nFallbackLevel)
                mpWinFontEntry[i] = nullptr;
        }
    }

    // store new font in correct layer
    if (mpWinFontEntry[nFallbackLevel])
    {
        mpWinFontEntry[nFallbackLevel]->SetHFONT(hNewFont);
        // now the font is live => update font face
        const WinFontFace* pFontFace = static_cast<const WinFontFace*>(mpWinFontEntry[nFallbackLevel]->GetFontFace());
        pFontFace->UpdateFromHDC(getHDC());
    }
    else
        mhFonts[ nFallbackLevel ] = hNewFont;
}

void WinSalGraphics::GetFontMetric( ImplFontMetricDataRef& rxFontMetric, int nFallbackLevel )
{
    // temporarily change the HDC to the font in the fallback level
    assert(!mhFonts[nFallbackLevel] && mpWinFontEntry[nFallbackLevel]);
    const HFONT hOldFont = SelectFont(getHDC(), mpWinFontEntry[nFallbackLevel]->GetHFONT());

    wchar_t aFaceName[LF_FACESIZE+60];
    if( GetTextFaceW( getHDC(), SAL_N_ELEMENTS(aFaceName), aFaceName ) )
        rxFontMetric->SetFamilyName(o3tl::toU(aFaceName));

    const DWORD nHheaTag = CalcTag("hhea");
    const DWORD nOS2Tag = CalcTag("OS/2");
    const RawFontData aHheaRawData(getHDC(), nHheaTag);
    const RawFontData aOS2RawData(getHDC(), nOS2Tag);

    rxFontMetric->SetMinKashida(mpWinFontEntry[nFallbackLevel]->GetKashidaWidth());

    // get the font metric
    OUTLINETEXTMETRICW aOutlineMetric;
    const bool bOK = GetOutlineTextMetricsW(getHDC(), sizeof(aOutlineMetric), &aOutlineMetric);
    // restore the HDC to the font in the base level
    SelectFont( getHDC(), hOldFont );
    if( !bOK )
        return;

    TEXTMETRICW aWinMetric = aOutlineMetric.otmTextMetrics;

    // device independent font attributes
    rxFontMetric->SetFamilyType(ImplFamilyToSal( aWinMetric.tmPitchAndFamily ));
    rxFontMetric->SetSymbolFlag(aWinMetric.tmCharSet == SYMBOL_CHARSET);
    rxFontMetric->SetWeight(ImplWeightToSal( aWinMetric.tmWeight ));
    rxFontMetric->SetPitch(ImplMetricPitchToSal( aWinMetric.tmPitchAndFamily ));
    rxFontMetric->SetItalic(aWinMetric.tmItalic ? ITALIC_NORMAL : ITALIC_NONE);
    rxFontMetric->SetSlant( 0 );

    // transformation dependent font metrics
    rxFontMetric->SetWidth(static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmAveCharWidth ));

    const std::vector<uint8_t> rHhea(aHheaRawData.get(), aHheaRawData.get() + aHheaRawData.size());
    const std::vector<uint8_t> rOS2(aOS2RawData.get(), aOS2RawData.get() + aOS2RawData.size());
    rxFontMetric->ImplCalcLineSpacing(rHhea, rOS2, aOutlineMetric.otmEMSquare);
}

const FontCharMapRef WinSalGraphics::GetFontCharMap() const
{
    if (!mpWinFontEntry[0])
    {
        FontCharMapRef xDefFontCharMap( new FontCharMap() );
        return xDefFontCharMap;
    }
    return static_cast<const WinFontFace*>(mpWinFontEntry[0]->GetFontFace())->GetFontCharMap();
}

bool WinSalGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if (!mpWinFontEntry[0])
        return false;
    return static_cast<const WinFontFace*>(mpWinFontEntry[0]->GetFontFace())->GetFontCapabilities(rFontCapabilities);
}

int CALLBACK SalEnumFontsProcExW( const LOGFONTW* lpelfe,
                                  const TEXTMETRICW* lpntme,
                                  DWORD nFontType, LPARAM lParam )
{
    ENUMLOGFONTEXW const * pLogFont
        = reinterpret_cast<ENUMLOGFONTEXW const *>(lpelfe);
    NEWTEXTMETRICEXW const * pMetric
        = reinterpret_cast<NEWTEXTMETRICEXW const *>(lpntme);
    ImplEnumInfo* pInfo = reinterpret_cast<ImplEnumInfo*>(lParam);
    if ( !pInfo->mpName )
    {
        // Ignore vertical fonts
        if ( pLogFont->elfLogFont.lfFaceName[0] != '@' )
        {
            OUString aName = o3tl::toU(pLogFont->elfLogFont.lfFaceName);
            pInfo->mpName = &aName;
            memcpy(pInfo->mpLogFont->lfFaceName, pLogFont->elfLogFont.lfFaceName, (aName.getLength()+1)*sizeof(wchar_t));
            pInfo->mpLogFont->lfCharSet = pLogFont->elfLogFont.lfCharSet;
            EnumFontFamiliesExW(pInfo->mhDC, pInfo->mpLogFont, SalEnumFontsProcExW,
                                reinterpret_cast<LPARAM>(pInfo), 0);
            pInfo->mpLogFont->lfFaceName[0] = '\0';
            pInfo->mpLogFont->lfCharSet = DEFAULT_CHARSET;
            pInfo->mpName = nullptr;
        }
    }
    else
    {
        // Ignore non-device fonts on printers.
        if (pInfo->mbPrinter)
        {
            if ((nFontType & RASTER_FONTTYPE) && !(nFontType & DEVICE_FONTTYPE))
            {
                SAL_WARN("vcl.gdi.font", "Unsupported printer font ignored: " << OUString(o3tl::toU(pLogFont->elfLogFont.lfFaceName)));
                return 1;
            }
        }
        // Only SFNT fonts are supported, ignore anything else.
        else if (!(nFontType & TRUETYPE_FONTTYPE) &&
                 !(pMetric->ntmTm.ntmFlags & NTM_PS_OPENTYPE) &&
                 !(pMetric->ntmTm.ntmFlags & NTM_TT_OPENTYPE))
        {
            SAL_WARN("vcl.gdi.font", "Unsupported font ignored: " << OUString(o3tl::toU(pLogFont->elfLogFont.lfFaceName)));
            return 1;
        }

        rtl::Reference<WinFontFace> pData = ImplLogMetricToDevFontDataW( pLogFont, &(pMetric->ntmTm), nFontType );
        pData->SetFontId( sal_IntPtr( pInfo->mnFontCount++ ) );

        pInfo->mpList->Add( pData.get() );
        SAL_WARN("vcl.gdi.font", "SalEnumFontsProcExW: font added: " << pData->GetFamilyName() << " " << pData->GetStyleName());
    }

    return 1;
}

struct TempFontItem
{
    OUString maFontFilePath;
    OUString maResourcePath;
    TempFontItem* mpNextItem;
};

bool ImplAddTempFont( SalData& rSalData, const OUString& rFontFileURL )
{
    int nRet = 0;
    OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

    nRet = AddFontResourceExW( o3tl::toW(aUSytemPath.getStr()), FR_PRIVATE, nullptr );

    if ( !nRet )
    {
        static int nCounter = 0;
        wchar_t aFileName[] = L"soAA.fot";
        aFileName[2] = sal::static_int_cast<wchar_t>(L'A' + (15 & (nCounter>>4)));
        aFileName[3] = sal::static_int_cast<wchar_t>(L'A' + (15 & nCounter));
        wchar_t aResourceName[512];
        int const nMaxLen = SAL_N_ELEMENTS(aResourceName) - 16;
        int nLen = GetTempPathW( nMaxLen, aResourceName );
        wcsncpy( aResourceName + nLen, aFileName, SAL_N_ELEMENTS( aResourceName ) - nLen );
        // security: end buffer in any case
        aResourceName[ SAL_N_ELEMENTS(aResourceName)-1 ] = 0;
        DeleteFileW( aResourceName );

        // TODO: font should be private => need to investigate why it doesn't work then
        if( !CreateScalableFontResourceW( 0, aResourceName, o3tl::toW(aUSytemPath.getStr()), nullptr ) )
            return false;
        ++nCounter;

        nRet = AddFontResourceW( aResourceName );
        if( nRet > 0 )
        {
            TempFontItem* pNewItem = new TempFontItem;
            pNewItem->maResourcePath = o3tl::toU( aResourceName );
            pNewItem->maFontFilePath = aUSytemPath;
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
            const wchar_t* pResourcePath = o3tl::toW(p->maResourcePath.getStr());
            RemoveFontResourceW( pResourcePath );
            DeleteFileW( pResourcePath );
        }
        else
        {
            RemoveFontResourceW( o3tl::toW(p->maFontFilePath.getStr()) );
        }

        rSalData.mpTempFontItem = p->mpNextItem;
        delete p;
    }
}

static bool ImplGetFontAttrFromFile( const OUString& rFontFileURL,
    FontAttributes& rDFA )
{
    OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

    // get FontAttributes from a *fot file
    // TODO: use GetTTGlobalFontInfo() to access the font directly
    rDFA.SetQuality( 1000 );
    rDFA.SetFamilyType(FAMILY_DONTKNOW);
    rDFA.SetWidthType(WIDTH_DONTKNOW);
    rDFA.SetWeight(WEIGHT_DONTKNOW);
    rDFA.SetItalic(ITALIC_DONTKNOW);
    rDFA.SetPitch(PITCH_DONTKNOW);

    // Create temporary file name
    wchar_t aResourceName[512];
    int nMaxLen = SAL_N_ELEMENTS(aResourceName) - 16;
    int nLen = GetTempPathW( nMaxLen, aResourceName );
    wcsncpy( aResourceName + nLen, L"soAAT.fot", std::max( 0, nMaxLen - nLen ));
    DeleteFileW( aResourceName );

    // Create font resource file (typically with a .fot file name extension).
    CreateScalableFontResourceW( 0, aResourceName, o3tl::toW(aUSytemPath.getStr()), nullptr );

    // Open and read the font resource file
    OUString aFotFileName = o3tl::toU( aResourceName );
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
    DeleteFileW( aResourceName );

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
    SAL_WARN( "vcl.gdi.font", "WinSalGraphics::AddTempDevFont(): " << rFontFileURL );

    FontAttributes aDFA;
    aDFA.SetFamilyName(rFontName);
    aDFA.SetQuality( 1000 );

    // Retrieve font name from font resource
    if( aDFA.GetFamilyName().isEmpty() )
    {
        ImplGetFontAttrFromFile( rFontFileURL, aDFA );
    }

    if ( aDFA.GetFamilyName().isEmpty() )
        return false;

    // remember temp font for cleanup later
    if( !ImplAddTempFont( *GetSalData(), rFontFileURL ) )
        return false;

    // create matching FontData struct
    aDFA.SetSymbolFlag(false); // TODO: how to know it without accessing the font?
    aDFA.SetFamilyType(FAMILY_DONTKNOW);
    aDFA.SetWidthType(WIDTH_DONTKNOW);
    aDFA.SetWeight(WEIGHT_DONTKNOW);
    aDFA.SetItalic(ITALIC_DONTKNOW);
    aDFA.SetPitch(PITCH_DONTKNOW);

    /*
    // TODO: improve FontAttributes using the "font resource file"
    aDFS.maName = // using "FONTRES:" from file
    if( rFontName != aDFS.maName )
        aDFS.maMapName = aFontName;
    */

    rtl::Reference<WinFontFace> pFontData = new WinFontFace( aDFA, 0,
        sal::static_int_cast<BYTE>(DEFAULT_CHARSET),
        sal::static_int_cast<BYTE>(TMPF_VECTOR|TMPF_TRUETYPE) );
    pFontData->SetFontId( reinterpret_cast<sal_IntPtr>(pFontData.get()) );
    pFontCollection->Add( pFontData.get() );
    return true;
}

void WinSalGraphics::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    SAL_WARN( "vcl.gdi.font", "WinSalGraphics::GetDevFontList(): enter" );

    // make sure all fonts are registered at least temporarily
    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;

        // determine font path
        // since we are only interested in fonts that could not be
        // registered before because of missing administration rights
        // only the font path of the user installation is needed
        OUString aPath("$BRAND_BASE_DIR");
        rtl_bootstrap_expandMacros(&aPath.pData);

        // collect fonts in font path that could not be registered
        osl::Directory aFontDir(aPath + "/" LIBO_SHARE_FOLDER "/fonts/truetype");
        osl::FileBase::RC rcOSL = aFontDir.open();
        if( rcOSL == osl::FileBase::E_None )
        {
            osl::DirectoryItem aDirItem;

            while( aFontDir.getNextItem( aDirItem, 10 ) == osl::FileBase::E_None )
            {
                osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileURL );
                rcOSL = aDirItem.getFileStatus( aFileStatus );
                if ( rcOSL == osl::FileBase::E_None )
                    AddTempDevFont( pFontCollection, aFileStatus.getFileURL(), "" );
            }
        }
    }

    ImplEnumInfo aInfo;
    aInfo.mhDC          = getHDC();
    aInfo.mpList        = pFontCollection;
    aInfo.mpName        = nullptr;
    aInfo.mbPrinter     = mbPrinter;
    aInfo.mnFontCount   = 0;

    LOGFONTW aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );
    aLogFont.lfCharSet = DEFAULT_CHARSET;
    aInfo.mpLogFont = &aLogFont;
    EnumFontFamiliesExW( getHDC(), &aLogFont,
        SalEnumFontsProcExW, reinterpret_cast<LPARAM>(&aInfo), 0 );

    // set glyph fallback hook
    static WinGlyphFallbackSubstititution aSubstFallback;
    static WinPreMatchFontSubstititution aPreMatchFont;
    pFontCollection->SetFallbackHook( &aSubstFallback );
    pFontCollection->SetPreMatchHook(&aPreMatchFont);

    SAL_WARN( "vcl.gdi.font", "WinSalGraphics::GetDevFontList(): leave" );
}

void WinSalGraphics::ClearDevFontCache()
{
    //anything to do here ?
}

bool WinSalGraphics::GetGlyphBoundRect(const GlyphItem& rGlyph, tools::Rectangle& rRect)
{
    auto it = g_BoundRectCache.find(rGlyph.maGlyphId);
    if (it != g_BoundRectCache.end())
    {
        rRect = it->second;
        return true;
    }

    HDC hDC = getHDC();

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_METRICS;
    nGGOFlags |= GGO_GLYPH_INDEX;

    GLYPHMETRICS aGM;
    aGM.gmptGlyphOrigin.x = aGM.gmptGlyphOrigin.y = 0;
    aGM.gmBlackBoxX = aGM.gmBlackBoxY = 0;
    DWORD nSize = ::GetGlyphOutlineW(hDC, rGlyph.maGlyphId, nGGOFlags, &aGM, 0, nullptr, &aMat);
    if( nSize == GDI_ERROR )
        return false;

    rRect = tools::Rectangle( Point( +aGM.gmptGlyphOrigin.x, -aGM.gmptGlyphOrigin.y ),
        Size( aGM.gmBlackBoxX, aGM.gmBlackBoxY ) );
    rRect.SetLeft(static_cast<int>( mfCurrentFontScale * rRect.Left() ));
    rRect.SetRight(static_cast<int>( mfCurrentFontScale * rRect.Right() ) + 1);
    rRect.SetTop(static_cast<int>( mfCurrentFontScale * rRect.Top() ));
    rRect.SetBottom(static_cast<int>( mfCurrentFontScale * rRect.Bottom() ) + 1);

    g_BoundRectCache.insert({rGlyph.maGlyphId, rRect});

    return true;
}

bool WinSalGraphics::GetGlyphOutline(const GlyphItem& rGlyph,
    basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    rB2DPolyPoly.clear();

    HDC  hDC = getHDC();

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_NATIVE;
    nGGOFlags |= GGO_GLYPH_INDEX;

    GLYPHMETRICS aGlyphMetrics;
    const DWORD nSize1 = ::GetGlyphOutlineW(hDC, rGlyph.maGlyphId, nGGOFlags, &aGlyphMetrics, 0, nullptr, &aMat);
    if( !nSize1 )       // blank glyphs are ok
        return true;
    else if( nSize1 == GDI_ERROR )
        return false;

    BYTE* pData = new BYTE[ nSize1 ];
    const DWORD nSize2 = ::GetGlyphOutlineW(hDC, rGlyph.maGlyphId, nGGOFlags,
              &aGlyphMetrics, nSize1, pData, &aMat );

    if( nSize1 != nSize2 )
        return false;

    // TODO: avoid tools polygon by creating B2DPolygon directly
    int     nPtSize = 512;
    Point*  pPoints = new Point[ nPtSize ];
    PolyFlags* pFlags = new PolyFlags[ nPtSize ];

    TTPOLYGONHEADER* pHeader = reinterpret_cast<TTPOLYGONHEADER*>(pData);
    while( reinterpret_cast<BYTE*>(pHeader) < pData+nSize2 )
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
        pFlags[ nPnt++ ] = PolyFlags::Normal;

        bool bHasOfflinePoints = false;
        TTPOLYCURVE* pCurve = reinterpret_cast<TTPOLYCURVE*>( pHeader + 1 );
        pHeader = reinterpret_cast<TTPOLYGONHEADER*>( reinterpret_cast<BYTE*>(pHeader) + pHeader->cb );
        while( reinterpret_cast<BYTE*>(pCurve) < reinterpret_cast<BYTE*>(pHeader) )
        {
            int nNeededSize = nPnt + 16 + 3 * pCurve->cpfx;
            if( nPtSize < nNeededSize )
            {
                Point* pOldPoints = pPoints;
                PolyFlags* pOldFlags = pFlags;
                nPtSize = 2 * nNeededSize;
                pPoints = new Point[ nPtSize ];
                pFlags = new PolyFlags[ nPtSize ];
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
                    pFlags[ nPnt ] = PolyFlags::Normal;
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
                    pFlags[ nPnt+0 ] = PolyFlags::Control;

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
                    pFlags[ nPnt+2 ] = PolyFlags::Normal;

                    // calculate second cubic control point
                    // P1 = 1/3 * (PEnd + 2 * PQControl)
                    nX = pPoints[ nPnt+2 ].X() + 2 * aControlP.X();
                    nY = pPoints[ nPnt+2 ].Y() + 2 * aControlP.Y();
                    pPoints[ nPnt+1 ] = Point( (2*nX+3)/6, (2*nY+3)/6 );
                    pFlags[ nPnt+1 ] = PolyFlags::Control;

                    nPnt += 3;
                }
            }

            // next curve segment
            pCurve = reinterpret_cast<TTPOLYCURVE*>(&pCurve->apfx[ i ]);
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
            pPoints[i].setY(-pPoints[i].Y());

        // insert into polypolygon
        tools::Polygon aPoly( nPnt, pPoints, (bHasOfflinePoints ? pFlags : nullptr) );
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
        rB2DPolyPoly.transform(basegfx::utils::createScaleB2DHomMatrix(fFactor, fFactor));
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
    if (m_rData.mpWinFontEntry[0])
    {
        m_hOrigFont = m_rData.mpWinFontEntry[0]->GetHFONT();
        m_rData.mpWinFontEntry[0]->UnsetHFONT();
    }
    else
    {
        m_hOrigFont = m_rData.mhFonts[0];
        m_rData.mhFonts[0] = nullptr; // avoid deletion of current font
    }
}

ScopedFont::~ScopedFont()
{
    if( m_hOrigFont )
    {
        // restore original font, destroy temporary font
        HFONT hTempFont = m_rData.mhFonts[0];
        if (m_rData.mpWinFontEntry[0])
            m_rData.mpWinFontEntry[0]->SetHFONT(m_hOrigFont);
        else
            m_rData.mhFonts[0] = m_hOrigFont;
        SelectObject( m_rData.getHDC(), m_hOrigFont );
        DeleteObject( hTempFont );
    }
}

class ScopedTrueTypeFont
{
public:
    ScopedTrueTypeFont(): m_pFont(nullptr) {}

    ~ScopedTrueTypeFont();

    SFErrCodes open(void const * pBuffer, sal_uInt32 nLen, sal_uInt32 nFaceNum);

    TrueTypeFont * get() const { return m_pFont; }

private:
    TrueTypeFont * m_pFont;
};

ScopedTrueTypeFont::~ScopedTrueTypeFont()
{
    if (m_pFont != nullptr)
        CloseTTFont(m_pFont);
}

SFErrCodes ScopedTrueTypeFont::open(void const * pBuffer, sal_uInt32 nLen,
                             sal_uInt32 nFaceNum)
{
    OSL_ENSURE(m_pFont == nullptr, "already open");
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
    HFONT hOldFont = nullptr;
    ImplDoSetFont(&aIFSD, pFont, fScale, hOldFont);

    WinFontFace const * pWinFontData = static_cast<WinFontFace const *>(pFont);

#if OSL_DEBUG_LEVEL > 1
    // get font metrics
    TEXTMETRICW aWinMetric;
    if( !::GetTextMetricsW( getHDC(), &aWinMetric ) )
        return FALSE;

    SAL_WARN_IF( (aWinMetric.tmPitchAndFamily & TMPF_DEVICE), "vcl", "cannot subset device font" );
    SAL_WARN_IF( !(aWinMetric.tmPitchAndFamily & TMPF_TRUETYPE), "vcl", "can only subset TT font" );
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
        FontCharMapRef xFontCharMap = pWinFontData->GetFontCharMap();
        xFontCharMap = nullptr;

        // provide a font subset from the CFF-table
        FILE* pOutFile = fopen( aToFile.getStr(), "wb" );
        rInfo.LoadFont( FontType::CFF_FONT, aRawCffData.get(), aRawCffData.size() );
        bool bRC = rInfo.CreateFontSubset( FontType::TYPE1_PFB, pOutFile, nullptr,
                pGlyphIds, pEncoding, nGlyphCount, pGlyphWidths );
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
    SFErrCodes nRC = aSftTTF.open( xRawFontData.get(), xRawFontData.size(), nFaceNum );
    if( nRC != SFErrCodes::Ok )
        return FALSE;

    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( aSftTTF.get(), &aTTInfo );
    rInfo.m_nFontType   = FontType::SFNT_TTF;
    rInfo.m_aPSName     = ImplSalGetUniString( aTTInfo.psname );
    rInfo.m_nAscent     = aTTInfo.winAscent;
    rInfo.m_nDescent    = aTTInfo.winDescent;
    rInfo.m_aFontBBox   = tools::Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
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
        aShortIDs[i] = static_cast<sal_uInt16>(pGlyphIds[i]);
        if (!aShortIDs[i])
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
    SAL_WARN_IF( nGlyphCount >= 257, "vcl", "too many glyphs for subsetting" );

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
            aTempEncs, nGlyphCount );
    return (nRC == SFErrCodes::Ok);
}

const void* WinSalGraphics::GetEmbedFontData(const PhysicalFontFace* pFont, long* pDataLen)
{
    // create matching FontSelectPattern
    // we need just enough to get to the font file data
    FontSelectPattern aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    ScopedFont aOldFont(*this);
    SetFont( &aIFSD, 0 );

    // get the raw font file data
    RawFontData aRawFontData( getHDC() );
    *pDataLen = aRawFontData.size();
    if( !aRawFontData.get() )
        return nullptr;

    const unsigned char* pData = aRawFontData.steal();
    return pData;
}

void WinSalGraphics::FreeEmbedFontData( const void* pData, long /*nLen*/ )
{
    delete[] static_cast<char const *>(pData);
}

void WinSalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                     bool bVertical,
                                     std::vector< sal_Int32 >& rWidths,
                                     Ucs2UIntMap& rUnicodeEnc )
{
    // create matching FontSelectPattern
    // we need just enough to get to the font file data
    FontSelectPattern aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    // TODO: much better solution: move SetFont and restoration of old font to caller
    ScopedFont aOldFont(*this);

    float fScale = 0.0;
    HFONT hOldFont = nullptr;
    ImplDoSetFont(&aIFSD, pFont, fScale, hOldFont);

    // get raw font file data
    const RawFontData xRawFontData( getHDC() );
    if( !xRawFontData.get() )
        return;

    // open font file
    sal_uInt32 nFaceNum = 0;
    if( !*xRawFontData.get() )  // TTC candidate
        nFaceNum = ~0U;  // indicate "TTC font extracts only"

    ScopedTrueTypeFont aSftTTF;
    SFErrCodes nRC = aSftTTF.open( xRawFontData.get(), xRawFontData.size(), nFaceNum );
    if( nRC != SFErrCodes::Ok )
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
        const WinFontFace* pWinFont = static_cast<const WinFontFace*>(pFont);
        FontCharMapRef xFCMap = pWinFont->GetFontCharMap();
        SAL_WARN_IF( !xFCMap.is() || !xFCMap->GetCharCount(), "vcl", "no map" );

        int nCharCount = xFCMap->GetCharCount();
        sal_uInt32 nChar = xFCMap->GetFirstChar();
        for( int i = 0; i < nCharCount; i++ )
        {
            if( nChar < 0x00010000 )
            {
                sal_uInt16 nGlyph = ::MapChar( aSftTTF.get(),
                                               static_cast<sal_Ucs>(nChar));
                if( nGlyph )
                    rUnicodeEnc[ static_cast<sal_Unicode>(nChar) ] = nGlyph;
            }
            nChar = xFCMap->GetNextChar( nChar );
        }

        xFCMap = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
