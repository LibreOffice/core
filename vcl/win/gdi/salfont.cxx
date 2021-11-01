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

#include <sal/config.h>

#include <sal/types.h>
#include <config_folders.h>

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
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
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/fontcfg.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/metric.hxx>
#include <vcl/fontcharmap.hxx>
#include <comphelper/scopeguard.hxx>

#include <font/FontSelectPattern.hxx>
#include <fontsubset.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <font/PhysicalFontFaceCollection.hxx>
#include <font/PhysicalFontFace.hxx>
#include <font/fontsubstitution.hxx>
#include <sft.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>
#include <win/winlayout.hxx>
#include <win/wingdiimpl.hxx>
#include <impfontcharmap.hxx>
#include <impfontmetricdata.hxx>
#include <impglyphitem.hxx>

#if HAVE_FEATURE_SKIA
#include <vcl/skia/SkiaHelper.hxx>
#include <skia/win/font.hxx>
#endif

using namespace vcl;

static FIXED FixedFromDouble( double d )
{
    const tools::Long l = static_cast<tools::Long>( d * 65536. );
    return *reinterpret_cast<FIXED const *>(&l);
}

static int IntTimes256FromFixed(FIXED f)
{
    int nFixedTimes256 = (f.value << 8) + ((f.fract+0x80) >> 8);
    return nFixedTimes256;
}

namespace {

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

}

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

namespace {

class WinPreMatchFontSubstititution
:    public vcl::font::PreMatchFontSubstitution
{
public:
    bool FindFontSubstitute(vcl::font::FontSelectPattern&) const override;
};

class WinGlyphFallbackSubstititution
:    public vcl::font::GlyphFallbackFontSubstitution
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

    bool FindFontSubstitute(vcl::font::FontSelectPattern&, LogicalFontInstance* pLogicalFont, OUString& rMissingChars) const override;
private:
    HDC mhDC;
    bool HasMissingChars(vcl::font::PhysicalFontFace*, OUString& rMissingChars) const;
};

}

// does a font face hold the given missing characters?
bool WinGlyphFallbackSubstititution::HasMissingChars(vcl::font::PhysicalFontFace* pFace, OUString& rMissingChars) const
{
    WinFontFace* pWinFont = static_cast< WinFontFace* >(pFace);
    FontCharMapRef xFontCharMap = pWinFont->GetFontCharMap();
    if( !xFontCharMap.is() )
    {
        // create a FontSelectPattern object for getting s LOGFONT
        const vcl::font::FontSelectPattern aFSD( *pFace, Size(), 0.0, 0, false );
        // construct log font
        LOGFONTW aLogFont;
        ImplGetLogFontFromFontSelect( aFSD, pFace, aLogFont );

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
    vcl::font::PhysicalFontFamily* findDevFontListByLocale(const vcl::font::PhysicalFontCollection &rFontCollection,
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
const std::map<OUString, OUString> aBitmapFontSubs =
{
    { "MS Sans Serif", "Microsoft Sans Serif" },
    { "MS Serif",      "Times New Roman" },
    { "Small Fonts",   "Arial" },
    { "Courier",       "Courier New" },
    { "Roman",         "Times New Roman" },
    { "Script",        "Mistral" }
};

// TODO: See if Windows have API that we can use here to improve font fallback.
bool WinPreMatchFontSubstititution::FindFontSubstitute(vcl::font::FontSelectPattern& rFontSelData) const
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
bool WinGlyphFallbackSubstititution::FindFontSubstitute(vcl::font::FontSelectPattern& rFontSelData, LogicalFontInstance* /*pLogicalFont*/, OUString& rMissingChars) const
{
    // guess a locale matching to the missing chars
    LanguageType eLang = rFontSelData.meLanguage;
    LanguageTag aLanguageTag( eLang);

    // fall back to default UI locale if the font language is inconclusive
    if( eLang == LANGUAGE_DONTKNOW )
        aLanguageTag = Application::GetSettings().GetUILanguageTag();

    // first level fallback:
    // try use the locale specific default fonts defined in VCL.xcu
    const vcl::font::PhysicalFontCollection* pFontCollection = ImplGetSVData()->maGDIData.mxScreenFontList.get();
    vcl::font::PhysicalFontFamily* pFontFamily = findDevFontListByLocale(*pFontCollection, aLanguageTag);
    if( pFontFamily )
    {
        vcl::font::PhysicalFontFace* pFace = pFontFamily->FindBestFontFace( rFontSelData );
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
        vcl::font::PhysicalFontFace* pFace = pFontFamily->FindBestFontFace( rFontSelData );
        if( HasMissingChars( pFace, rMissingChars ) )
        {
            rFontSelData.maSearchName = pFontFamily->GetSearchName();
            return true;
        }
    }

    // last level fallback, check each font type face one by one
    std::unique_ptr<vcl::font::PhysicalFontFaceCollection> pTestFontList = pFontCollection->GetFontFaceCollection();
    // limit the count of fonts to be checked to prevent hangs
    static const int MAX_GFBFONT_COUNT = 600;
    int nTestFontCount = pTestFontList->Count();
    if( nTestFontCount > MAX_GFBFONT_COUNT )
        nTestFontCount = MAX_GFBFONT_COUNT;

    bool bFound = false;
    for( int i = 0; i < nTestFontCount; ++i )
    {
        vcl::font::PhysicalFontFace* pFace = pTestFontList->Get( i );
        bFound = HasMissingChars( pFace, rMissingChars );
        if( !bFound )
            continue;
        rFontSelData.maSearchName = pFace->GetFamilyName();
        break;
    }

    return bFound;
}

namespace {

struct ImplEnumInfo
{
    HDC                 mhDC;
    vcl::font::PhysicalFontCollection* mpList;
    OUString*           mpName;
    LOGFONTW*           mpLogFont;
    bool                mbPrinter;
    int                 mnFontCount;
};

}

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

static FontPitch ImplLogPitchToSal( BYTE nPitch )
{
    if ( nPitch & FIXED_PITCH )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}

static FontPitch ImplMetricPitchToSal( BYTE nPitch )
{
    // Grrrr! See NT help
    if ( !(nPitch & TMPF_FIXED_PITCH) )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}

static BYTE ImplPitchToWin( FontPitch ePitch )
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
    aDFA.SetFamilyName(OUString(o3tl::toU(rLogFont.lfFaceName)));

    // use the face's style name only if it looks reasonable
    const wchar_t* pStyleName = rEnumFont.elfStyle;
    const wchar_t* pEnd = pStyleName + sizeof(rEnumFont.elfStyle)/sizeof(*rEnumFont.elfStyle);
    const wchar_t* p = pStyleName;
    for(; *p && (p < pEnd); ++p )
        if( *p < 0x0020 )
            break;
    if( p < pEnd )
        aDFA.SetStyleName(OUString(o3tl::toU(pStyleName)));

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
                                         const NEWTEXTMETRICW* pMetric)
{
    rtl::Reference<WinFontFace> pData = new WinFontFace(
        WinFont2DevFontAttributes(*pLogFont, *pMetric),
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

        tools::Long nFontHeight = rLogFont.lfHeight;
        if ( nFontHeight < 0 )
            nFontHeight = -nFontHeight;
        tools::Long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );
        if( !nDPIY )
            nDPIY = 600;
        nFontHeight *= 72;
        nFontHeight += nDPIY/2;
        nFontHeight /= nDPIY;
        rFont.SetFontSize( Size( 0, nFontHeight ) );
        rFont.SetOrientation( Degree10(static_cast<sal_Int16>(rLogFont.lfEscapement)) );
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
    BYTE eWinCharSet, BYTE nPitchAndFamily )
:   vcl::font::PhysicalFontFace( rDFS ),
    mnId( 0 ),
    mbFontCapabilitiesRead( false ),
    meWinCharSet( eWinCharSet ),
    mnPitchAndFamily( nPitchAndFamily ),
    mbAliasSymbolsHigh( false ),
    mbAliasSymbolsLow( false )
{
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

rtl::Reference<LogicalFontInstance> WinFontFace::CreateFontInstance(const vcl::font::FontSelectPattern& rFSD) const
{
#if HAVE_FEATURE_SKIA
    if (SkiaHelper::isVCLSkiaEnabled())
        return new SkiaWinFontInstance(*this, rFSD);
#endif
    return new WinFontInstance(*this, rFSD);
}

static DWORD CalcTag( const char p[5]) { return (p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24)); }

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
        unsigned char* pTable = aTable.data();
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

static int CALLBACK SalEnumQueryFontProcExW( const LOGFONTW*, const TEXTMETRICW*, DWORD, LPARAM lParam )
{
    *reinterpret_cast<bool*>(lParam) = true;
    return 0;
}

void ImplGetLogFontFromFontSelect( const vcl::font::FontSelectPattern& rFont,
                                   const vcl::font::PhysicalFontFace* pFontFace,
                                   LOGFONTW& rLogFont )
{
    OUString aName;
    if (pFontFace)
        aName = pFontFace->GetFamilyName();
    else
        aName = rFont.GetFamilyName().getToken( 0, ';' );

    UINT nNameLen = aName.getLength();
    if (nNameLen >= LF_FACESIZE)
        nNameLen = LF_FACESIZE - 1;
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
        rLogFont.lfCharSet = rFont.IsSymbolFont() ? SYMBOL_CHARSET : DEFAULT_CHARSET;
        rLogFont.lfPitchAndFamily = ImplPitchToWin( rFont.GetPitch() )
                                  | ImplFamilyToWin( rFont.GetFamilyType() );
    }

    rLogFont.lfWeight          = ImplWeightToWin( rFont.GetWeight() );
    rLogFont.lfHeight          = static_cast<LONG>(-rFont.mnHeight);
    rLogFont.lfWidth           = static_cast<LONG>(rFont.mnWidth);
    rLogFont.lfUnderline       = 0;
    rLogFont.lfStrikeOut       = 0;
    rLogFont.lfItalic          = BYTE(rFont.GetItalic() != ITALIC_NONE);
    rLogFont.lfEscapement      = rFont.mnOrientation.get();
    rLogFont.lfOrientation     = rLogFont.lfEscapement;
    rLogFont.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
    rLogFont.lfQuality         = DEFAULT_QUALITY;
    rLogFont.lfOutPrecision    = OUT_TT_PRECIS;
    if ( rFont.mnOrientation )
        rLogFont.lfClipPrecision |= CLIP_LH_ANGLES;

    // disable antialiasing if requested
    if ( rFont.mbNonAntialiased )
        rLogFont.lfQuality = NONANTIALIASED_QUALITY;

}

std::tuple<HFONT,bool,sal_Int32> WinSalGraphics::ImplDoSetFont(HDC hDC, vcl::font::FontSelectPattern const & i_rFont,
                                    const vcl::font::PhysicalFontFace * i_pFontFace,
                                    HFONT& o_rOldFont)
{
    HFONT hNewFont = nullptr;

    LOGFONTW aLogFont;
    ImplGetLogFontFromFontSelect( i_rFont, i_pFontFace, aLogFont );

    bool    bIsCJKVerticalFont = false;
    // select vertical mode for printing if requested and available
    if ( i_rFont.mbVertical && mbPrinter )
    {
        constexpr size_t nLen = sizeof(aLogFont.lfFaceName) - sizeof(aLogFont.lfFaceName[0]);
        // vertical fonts start with an '@'
        memmove( &aLogFont.lfFaceName[1], &aLogFont.lfFaceName[0], nLen );
        aLogFont.lfFaceName[0] = '@';
        aLogFont.lfFaceName[LF_FACESIZE - 1] = 0;

        // check availability of vertical mode for this font
        EnumFontFamiliesExW( getHDC(), &aLogFont, SalEnumQueryFontProcExW,
                reinterpret_cast<LPARAM>(&bIsCJKVerticalFont), 0 );
        if( !bIsCJKVerticalFont )
        {
            // restore non-vertical name if not vertical mode isn't available
            memcpy( &aLogFont.lfFaceName[0], &aLogFont.lfFaceName[1], nLen );
            aLogFont.lfFaceName[LF_FACESIZE - 1] = 0;
        }
    }

    hNewFont = ::CreateFontIndirectW( &aLogFont );

    HDC hdcScreen = nullptr;
    if( mbVirDev )
        // only required for virtual devices, see below for details
        hdcScreen = GetDC(nullptr);
    if( hdcScreen )
    {
        // select font into screen hdc first to get an antialiased font
        // and instantly restore the default font!
        // see knowledge base article 305290:
        // "PRB: Fonts Not Drawn Antialiased on Device Context for DirectDraw Surface"
        SelectFont( hdcScreen, SelectFont( hdcScreen , hNewFont ) );
    }
    o_rOldFont = ::SelectFont(hDC, hNewFont);

    TEXTMETRICW aTextMetricW;
    if (!::GetTextMetricsW(hDC, &aTextMetricW))
    {
        // the selected font doesn't work => try a replacement
        // TODO: use its font fallback instead
        lstrcpynW( aLogFont.lfFaceName, L"Courier New", 12 );
        aLogFont.lfPitchAndFamily = FIXED_PITCH;
        HFONT hNewFont2 = CreateFontIndirectW( &aLogFont );
        SelectFont(hDC, hNewFont2);
        DeleteFont( hNewFont );
        hNewFont = hNewFont2;
        bIsCJKVerticalFont = false;
    }

    if( hdcScreen )
        ::ReleaseDC( nullptr, hdcScreen );

    return std::make_tuple(hNewFont, bIsCJKVerticalFont, static_cast<sal_Int32>(aTextMetricW.tmDescent));
}

void WinSalGraphics::SetFont(LogicalFontInstance* pFont, int nFallbackLevel)
{
    assert(nFallbackLevel >= 0 && nFallbackLevel < MAX_FALLBACK);

    // return early if there is no new font
    if( !pFont )
    {
        if (!mpWinFontEntry[nFallbackLevel].is())
            return;

        // DeInitGraphics doesn't free the cached fonts, so mhDefFont might be nullptr
        if (mhDefFont)
        {
            ::SelectFont(getHDC(), mhDefFont);
            mhDefFont = nullptr;
        }

        // release no longer referenced font handles
        for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
            mpWinFontEntry[i] = nullptr;
        return;
    }

    WinFontInstance *pFontInstance = static_cast<WinFontInstance*>(pFont);
    mpWinFontEntry[ nFallbackLevel ] = pFontInstance;

    HFONT hOldFont = nullptr;
    HFONT hNewFont = pFontInstance->GetHFONT();
    if (!hNewFont)
    {
        pFontInstance->SetGraphics(this);
        hNewFont = pFontInstance->GetHFONT();
    }
    hOldFont = ::SelectFont(getHDC(), hNewFont);

    // keep default font
    if( !mhDefFont )
        mhDefFont = hOldFont;
    else
    {
        // release no longer referenced font handles
        for( int i = nFallbackLevel + 1; i < MAX_FALLBACK && mpWinFontEntry[i].is(); ++i )
            mpWinFontEntry[i] = nullptr;
    }

    // now the font is live => update font face
    const WinFontFace* pFontFace = pFontInstance->GetFontFace();
    pFontFace->UpdateFromHDC(getHDC());
}

void WinSalGraphics::GetFontMetric( ImplFontMetricDataRef& rxFontMetric, int nFallbackLevel )
{
    // temporarily change the HDC to the font in the fallback level
    rtl::Reference<WinFontInstance> pFontInstance = mpWinFontEntry[nFallbackLevel];
    const HFONT hOldFont = SelectFont(getHDC(), pFontInstance->GetHFONT());

    wchar_t aFaceName[LF_FACESIZE+60];
    if( GetTextFaceW( getHDC(), SAL_N_ELEMENTS(aFaceName), aFaceName ) )
        rxFontMetric->SetFamilyName(OUString(o3tl::toU(aFaceName)));

    rxFontMetric->SetMinKashida(pFontInstance->GetKashidaWidth());
    rxFontMetric->ImplCalcLineSpacing(pFontInstance.get());

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
    rxFontMetric->SetWidth(static_cast<int>(pFontInstance->GetScale() * aWinMetric.tmAveCharWidth));
}

FontCharMapRef WinSalGraphics::GetFontCharMap() const
{
    if (!mpWinFontEntry[0])
    {
        return FontCharMapRef( new FontCharMap() );
    }
    return mpWinFontEntry[0]->GetFontFace()->GetFontCharMap();
}

bool WinSalGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if (!mpWinFontEntry[0])
        return false;
    return mpWinFontEntry[0]->GetFontFace()->GetFontCapabilities(rFontCapabilities);
}

static int CALLBACK SalEnumFontsProcExW( const LOGFONTW* lpelfe,
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
            OUString aName(o3tl::toU(pLogFont->elfLogFont.lfFaceName));
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
                SAL_INFO("vcl.fonts", "Unsupported printer font ignored: " << OUString(o3tl::toU(pLogFont->elfLogFont.lfFaceName)));
                return 1;
            }
        }
        // Only SFNT fonts are supported, ignore anything else.
        else if (!(nFontType & TRUETYPE_FONTTYPE) &&
                 !(pMetric->ntmTm.ntmFlags & NTM_PS_OPENTYPE) &&
                 !(pMetric->ntmTm.ntmFlags & NTM_TT_OPENTYPE))
        {
            SAL_INFO("vcl.fonts", "Unsupported font ignored: " << OUString(o3tl::toU(pLogFont->elfLogFont.lfFaceName)));
            return 1;
        }

        rtl::Reference<WinFontFace> pData = ImplLogMetricToDevFontDataW(pLogFont, &(pMetric->ntmTm));
        pData->SetFontId( sal_IntPtr( pInfo->mnFontCount++ ) );

        pInfo->mpList->Add( pData.get() );
        SAL_INFO("vcl.fonts", "SalEnumFontsProcExW: font added: " << pData->GetFamilyName() << " " << pData->GetStyleName());
    }

    return 1;
}

struct TempFontItem
{
    OUString maFontResourcePath;
    TempFontItem* mpNextItem;
};

static int lcl_AddFontResource(SalData& rSalData, const OUString& rFontFileURL, bool bShared)
{
    OUString aFontSystemPath;
    OSL_VERIFY(!osl::FileBase::getSystemPathFromFileURL(rFontFileURL, aFontSystemPath));

    int nRet = AddFontResourceExW(o3tl::toW(aFontSystemPath.getStr()), FR_PRIVATE, nullptr);
    SAL_WARN_IF(nRet <= 0, "vcl.fonts", "AddFontResourceExW failed for " << rFontFileURL);
    if (nRet > 0)
    {
        TempFontItem* pNewItem = new TempFontItem;
        pNewItem->maFontResourcePath = aFontSystemPath;
        if (bShared)
        {
            pNewItem->mpNextItem = rSalData.mpSharedTempFontItem;
            rSalData.mpSharedTempFontItem = pNewItem;
        }
        else
        {
            pNewItem->mpNextItem = rSalData.mpOtherTempFontItem;
            rSalData.mpOtherTempFontItem = pNewItem;
        }
    }
    return nRet;
}

void ImplReleaseTempFonts(SalData& rSalData, bool bAll)
{
    while (TempFontItem* p = rSalData.mpOtherTempFontItem)
    {
        RemoveFontResourceExW(o3tl::toW(p->maFontResourcePath.getStr()), FR_PRIVATE, nullptr);
        rSalData.mpOtherTempFontItem = p->mpNextItem;
        delete p;
    }

    if (!bAll)
        return;

    while (TempFontItem* p = rSalData.mpSharedTempFontItem)
    {
        RemoveFontResourceExW(o3tl::toW(p->maFontResourcePath.getStr()), FR_PRIVATE, nullptr);
        rSalData.mpSharedTempFontItem = p->mpNextItem;
        delete p;
    }
}

static OUString lcl_GetFontFamilyName(const OUString& rFontFileURL)
{
    // Create temporary file name
    OUString aTempFileURL;
    if (osl::File::E_None != osl::File::createTempFile(nullptr, nullptr, &aTempFileURL))
        return OUString();
    osl::File::remove(aTempFileURL);
    OUString aResSystemPath;
    osl::FileBase::getSystemPathFromFileURL(aTempFileURL, aResSystemPath);

    // Create font resource file (.fot)
    // There is a limit of 127 characters for the full path passed via lpszFile, so we have to
    // split the font URL and pass it as two parameters. As a result we can't use
    // CreateScalableFontResource for renaming, as it now expects the font in the system path.
    // But it's still good to use it for family name extraction, we're currently after.
    // BTW: it doesn't help to prefix the lpszFile with \\?\ to support larger paths.
    // TODO: use TTLoadEmbeddedFont (needs an EOT as input, so we have to add a header to the TTF)
    // TODO: forward the EOT from the AddTempDevFont call side, if VCL supports it
    INetURLObject aTTFUrl(rFontFileURL);
    // GetBase() strips the extension
    OUString aFilename = aTTFUrl.GetLastName(INetURLObject::DecodeMechanism::WithCharset);
    if (!CreateScalableFontResourceW(0, o3tl::toW(aResSystemPath.getStr()),
            o3tl::toW(aFilename.getStr()), o3tl::toW(aTTFUrl.GetPath().getStr())))
    {
        sal_uInt32 nError = GetLastError();
        SAL_WARN("vcl.fonts", "CreateScalableFontResource failed for " << aResSystemPath << " "
                              << aFilename << " " << aTTFUrl.GetPath() << " " << nError);
        return OUString();
    }

    // Open and read the font resource file
    osl::File aFotFile(aTempFileURL);
    if (osl::FileBase::E_None != aFotFile.open(osl_File_OpenFlag_Read))
        return OUString();

    sal_uInt64  nBytesRead = 0;
    char        aBuffer[4096];
    aFotFile.read( aBuffer, sizeof( aBuffer ), nBytesRead );
    // clean up temporary resource file
    aFotFile.close();
    osl::File::remove(aTempFileURL);

    // retrieve font family name from byte offset 0x4F6
    static const sal_uInt64 nNameOfs = 0x4F6;
    sal_uInt64 nPos = nNameOfs;
    for (; (nPos < nBytesRead) && (aBuffer[nPos] != 0); nPos++);
    if (nPos >= nBytesRead || (nPos == nNameOfs))
        return OUString();

    return OUString(aBuffer + nNameOfs, nPos - nNameOfs, osl_getThreadTextEncoding());
}

bool WinSalGraphics::AddTempDevFont(vcl::font::PhysicalFontCollection* pFontCollection,
                                    const OUString& rFontFileURL, const OUString& rFontName)
{
    OUString aFontFamily = lcl_GetFontFamilyName(rFontFileURL);
    if (aFontFamily.isEmpty())
    {
        SAL_WARN("vcl.fonts", "error extracting font family from " << rFontFileURL);
        return false;
    }

    if (rFontName != aFontFamily)
    {
        SAL_WARN("vcl.fonts", "font family renaming not implemented; skipping embedded " << rFontName);
        return false;
    }

    int nFonts = lcl_AddFontResource(*GetSalData(), rFontFileURL, false);
    if (nFonts <= 0)
        return false;

    ImplEnumInfo aInfo;
    aInfo.mhDC = getHDC();
    aInfo.mpList = pFontCollection;
    aInfo.mpName = &aFontFamily;
    aInfo.mbPrinter = mbPrinter;
    aInfo.mnFontCount = pFontCollection->Count();
    const int nExpectedFontCount = aInfo.mnFontCount + nFonts;

    LOGFONTW aLogFont = {};
    aLogFont.lfCharSet = DEFAULT_CHARSET;
    aInfo.mpLogFont = &aLogFont;

    // add the font to the PhysicalFontCollection
    EnumFontFamiliesExW(getHDC(), &aLogFont,
        SalEnumFontsProcExW, reinterpret_cast<LPARAM>(&aInfo), 0);

    SAL_WARN_IF(nExpectedFontCount != pFontCollection->Count(), "vcl.fonts",
        "temp font was registered but is not in enumeration: " << rFontFileURL);

    return true;
}

void WinSalGraphics::GetDevFontList( vcl::font::PhysicalFontCollection* pFontCollection )
{
    // make sure all LO shared fonts are registered temporarily
    static std::once_flag init;
    std::call_once(init, []()
    {
        auto registerFontsIn = [](const OUString& dir) {
            // collect fonts in font path that could not be registered
            osl::Directory aFontDir(dir);
            osl::FileBase::RC rcOSL = aFontDir.open();
            if (rcOSL == osl::FileBase::E_None)
            {
                osl::DirectoryItem aDirItem;
                SalData* pSalData = GetSalData();
                assert(pSalData);

                while (aFontDir.getNextItem(aDirItem, 10) == osl::FileBase::E_None)
                {
                    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL);
                    rcOSL = aDirItem.getFileStatus(aFileStatus);
                    if (rcOSL == osl::FileBase::E_None)
                        lcl_AddFontResource(*pSalData, aFileStatus.getFileURL(), true);
                }
            }
        };

        // determine font path
        // since we are only interested in fonts that could not be
        // registered before because of missing administration rights
        // only the font path of the user installation is needed
        OUString aPath("$BRAND_BASE_DIR");
        rtl_bootstrap_expandMacros(&aPath.pData);

        // internal font resources, required for normal operation, like OpenSymbol
        registerFontsIn(aPath + "/" LIBO_SHARE_RESOURCE_FOLDER "/common/fonts");

        // collect fonts in font path that could not be registered
        registerFontsIn(aPath + "/" LIBO_SHARE_FOLDER "/fonts/truetype");

        return true;
    });

    ImplEnumInfo aInfo;
    aInfo.mhDC          = getHDC();
    aInfo.mpList        = pFontCollection;
    aInfo.mpName        = nullptr;
    aInfo.mbPrinter     = mbPrinter;
    aInfo.mnFontCount   = 0;

    LOGFONTW aLogFont = {};
    aLogFont.lfCharSet = DEFAULT_CHARSET;
    aInfo.mpLogFont = &aLogFont;

    // fill the PhysicalFontCollection
    EnumFontFamiliesExW( getHDC(), &aLogFont,
        SalEnumFontsProcExW, reinterpret_cast<LPARAM>(&aInfo), 0 );

    // set glyph fallback hook
    static WinGlyphFallbackSubstititution aSubstFallback;
    static WinPreMatchFontSubstititution aPreMatchFont;
    pFontCollection->SetFallbackHook( &aSubstFallback );
    pFontCollection->SetPreMatchHook(&aPreMatchFont);
}

void WinSalGraphics::ClearDevFontCache()
{
    WinSalGraphicsImplBase* pImpl = dynamic_cast<WinSalGraphicsImplBase*>(GetImpl());
    assert(pImpl != nullptr);
    pImpl->ClearDevFontCache();
    ImplReleaseTempFonts(*GetSalData(), false);
}

bool WinFontInstance::ImplGetGlyphBoundRect(sal_GlyphId nId, tools::Rectangle& rRect, bool bIsVertical) const
{
    assert(m_pGraphics);
    HDC hDC = m_pGraphics->getHDC();
    const HFONT hOrigFont = static_cast<HFONT>(GetCurrentObject(hDC, OBJ_FONT));
    const HFONT hFont = GetHFONT();
    if (hFont != hOrigFont)
        SelectObject(hDC, hFont);

    const ::comphelper::ScopeGuard aFontRestoreScopeGuard([hFont, hOrigFont, hDC]()
        { if (hFont != hOrigFont) SelectObject(hDC, hOrigFont); });
    const float fFontScale = GetScale();

    // use unity matrix
    MAT2 aMat;
    const vcl::font::FontSelectPattern& rFSD = GetFontSelectPattern();

    // Use identity matrix for fonts requested in horizontal
    // writing (LTR or RTL), or rotated glyphs in vertical writing.
    if (!rFSD.mbVertical || !bIsVertical)
    {
        aMat.eM11 = aMat.eM22 = FixedFromDouble(1.0);
        aMat.eM12 = aMat.eM21 = FixedFromDouble(0.0);
    }
    else
    {
        constexpr double nCos = 0.0;
        constexpr double nSin = 1.0;
        aMat.eM11 = FixedFromDouble(nCos);
        aMat.eM12 = FixedFromDouble(nSin);
        aMat.eM21 = FixedFromDouble(-nSin);
        aMat.eM22 = FixedFromDouble(nCos);
    }

    UINT nGGOFlags = GGO_METRICS;
    nGGOFlags |= GGO_GLYPH_INDEX;

    GLYPHMETRICS aGM;
    aGM.gmptGlyphOrigin.x = aGM.gmptGlyphOrigin.y = 0;
    aGM.gmBlackBoxX = aGM.gmBlackBoxY = 0;
    DWORD nSize = ::GetGlyphOutlineW(hDC, nId, nGGOFlags, &aGM, 0, nullptr, &aMat);
    if (nSize == GDI_ERROR)
        return false;

    rRect = tools::Rectangle( Point( +aGM.gmptGlyphOrigin.x, -aGM.gmptGlyphOrigin.y ),
        Size( aGM.gmBlackBoxX, aGM.gmBlackBoxY ) );
    rRect.SetLeft(static_cast<int>( fFontScale * rRect.Left() ));
    rRect.SetRight(static_cast<int>( fFontScale * rRect.Right() ) + 1);
    rRect.SetTop(static_cast<int>( fFontScale * rRect.Top() ));
    rRect.SetBottom(static_cast<int>( fFontScale * rRect.Bottom() ) + 1);
    return true;
}

bool WinFontInstance::GetGlyphOutline(sal_GlyphId nId, basegfx::B2DPolyPolygon& rB2DPolyPoly, bool) const
{
    rB2DPolyPoly.clear();

    assert(m_pGraphics);
    HDC hDC = m_pGraphics->getHDC();
    const HFONT hOrigFont = static_cast<HFONT>(GetCurrentObject(hDC, OBJ_FONT));
    const HFONT hFont = GetHFONT();
    if (hFont != hOrigFont)
        SelectObject(hDC, hFont);

    const ::comphelper::ScopeGuard aFontRestoreScopeGuard([hFont, hOrigFont, hDC]()
        { if (hFont != hOrigFont) SelectObject(hDC, hOrigFont); });

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_NATIVE;
    nGGOFlags |= GGO_GLYPH_INDEX;

    GLYPHMETRICS aGlyphMetrics;
    const DWORD nSize1 = ::GetGlyphOutlineW(hDC, nId, nGGOFlags, &aGlyphMetrics, 0, nullptr, &aMat);
    if( !nSize1 )       // blank glyphs are ok
        return true;
    else if( nSize1 == GDI_ERROR )
        return false;

    BYTE* pData = new BYTE[ nSize1 ];
    const DWORD nSize2 = ::GetGlyphOutlineW(hDC, nId, nGGOFlags,
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

        tools::Long nX = IntTimes256FromFixed( pHeader->pfxStart.x );
        tools::Long nY = IntTimes256FromFixed( pHeader->pfxStart.y );
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
        const double fFactor(GetScale()/256);
        rB2DPolyPoly.transform(basegfx::utils::createScaleB2DHomMatrix(fFactor, fFactor));
    }

    return true;
}

namespace {

class ScopedFontHDC final
{
public:
    explicit ScopedFontHDC(WinSalGraphics& rGraphics, const vcl::font::PhysicalFontFace& rFontFace)
        // use height=1000 for easier debugging (to match psprint's font units)
        : m_aFSP(rFontFace, Size(0,1000), 1000.0, 0, false)
        , m_hDC(nullptr)
        , m_hOrigFont(nullptr)
    {
        m_hDC = CreateCompatibleDC(rGraphics.getHDC());
        if (!m_hDC)
            return;

        rGraphics.ImplDoSetFont(m_hDC, m_aFSP, &rFontFace, m_hOrigFont);
    }

    ~ScopedFontHDC()
    {
        if (m_hOrigFont)
            ::DeleteFont(SelectFont(m_hDC, m_hOrigFont));
        if (m_hDC)
            DeleteDC(m_hDC);
    }

    HDC hdc() const { return m_hDC; }
    const vcl::font::FontSelectPattern& fsp() const { return m_aFSP; }

private:
    vcl::font::FontSelectPattern m_aFSP;
    HDC m_hDC;
    HFONT m_hOrigFont;
};

class ScopedTrueTypeFont
{
public:
    ScopedTrueTypeFont(): m_pFont(nullptr) {}

    ~ScopedTrueTypeFont();

    SFErrCodes open(void const * pBuffer, sal_uInt32 nLen, sal_uInt32 nFaceNum, const FontCharMapRef xCharMap = nullptr);

    TrueTypeFont * get() const { return m_pFont; }

private:
    TrueTypeFont * m_pFont;
};

}

ScopedTrueTypeFont::~ScopedTrueTypeFont()
{
    if (m_pFont != nullptr)
        CloseTTFont(m_pFont);
}

SFErrCodes ScopedTrueTypeFont::open(void const * pBuffer, sal_uInt32 nLen,
                             sal_uInt32 nFaceNum, const FontCharMapRef xCharMap)
{
    OSL_ENSURE(m_pFont == nullptr, "already open");
    return OpenTTFontBuffer(pBuffer, nLen, nFaceNum, &m_pFont, xCharMap);
}

bool WinSalGraphics::CreateFontSubset( const OUString& rToFile,
    const vcl::font::PhysicalFontFace* pFont, const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
    sal_Int32* pGlyphWidths, int nGlyphCount, FontSubsetInfo& rInfo )
{
    ScopedFontHDC aScopedFontHDC(*this, *pFont);
    HDC hDC = aScopedFontHDC.hdc();
    if (!hDC)
        return false;

#if OSL_DEBUG_LEVEL > 1
    // get font metrics
    TEXTMETRICW aWinMetric;
    if (!::GetTextMetricsW(hDC, &aWinMetric))
        return false;

    SAL_WARN_IF( (aWinMetric.tmPitchAndFamily & TMPF_DEVICE), "vcl", "cannot subset device font" );
    SAL_WARN_IF( !(aWinMetric.tmPitchAndFamily & TMPF_TRUETYPE), "vcl", "can only subset TT font" );
#endif

    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return false;
    const rtl_TextEncoding aThreadEncoding = osl_getThreadTextEncoding();
    const OString aToFile(OUStringToOString(aSysPath, aThreadEncoding));

    // check if the font has a CFF-table
    const DWORD nCffTag = CalcTag( "CFF " );
    const RawFontData aRawCffData(hDC, nCffTag);
    if (aRawCffData.get())
        return SalGraphics::CreateCFFfontSubset(aRawCffData.get(), aRawCffData.size(), aToFile,
                                                pGlyphIds, pEncoding, pGlyphWidths, nGlyphCount,
                                                rInfo);

    // get raw font file data
    const RawFontData xRawFontData(hDC, 0);
    if( !xRawFontData.get() )
        return false;

    // open font file
    sal_uInt32 nFaceNum = 0;
    if( !*xRawFontData.get() )  // TTC candidate
        nFaceNum = ~0U;  // indicate "TTC font extracts only"

    ScopedTrueTypeFont aSftTTF;
    SFErrCodes nRC = aSftTTF.open( xRawFontData.get(), xRawFontData.size(), nFaceNum, pFont->GetFontCharMap());
    if( nRC != SFErrCodes::Ok )
        return false;

    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( aSftTTF.get(), &aTTInfo );
    OUString aPSName = ImplSalGetUniString(aTTInfo.psname);
    FillFontSubsetInfo(aTTInfo, aPSName, rInfo);

    // write subset into destination file
    return SalGraphics::CreateTTFfontSubset(*aSftTTF.get(), aToFile, aScopedFontHDC.fsp().mbVertical,
                                            pGlyphIds, pEncoding, pGlyphWidths, nGlyphCount);
}

const void* WinSalGraphics::GetEmbedFontData(const vcl::font::PhysicalFontFace* pFont, tools::Long* pDataLen)
{
    ScopedFontHDC aScopedFontHDC(*this, *pFont);
    HDC hDC = aScopedFontHDC.hdc();
    if (!hDC)
        return nullptr;

    // get the raw font file data
    RawFontData aRawFontData(hDC);
    *pDataLen = aRawFontData.size();
    return aRawFontData.get() ? aRawFontData.steal() : nullptr;
}

void WinSalGraphics::FreeEmbedFontData( const void* pData, tools::Long /*nLen*/ )
{
    delete[] static_cast<char const *>(pData);
}

void WinSalGraphics::GetGlyphWidths( const vcl::font::PhysicalFontFace* pFont,
                                     bool bVertical,
                                     std::vector< sal_Int32 >& rWidths,
                                     Ucs2UIntMap& rUnicodeEnc )
{
    ScopedFontHDC aScopedFontHDC(*this, *pFont);
    HDC hDC = aScopedFontHDC.hdc();
    if (!hDC)
        return;

    // get raw font file data
    const RawFontData xRawFontData(hDC);
    if( !xRawFontData.get() )
        return;

    // open font file
    sal_uInt32 nFaceNum = 0;
    if( !*xRawFontData.get() )  // TTC candidate
        nFaceNum = ~0U;  // indicate "TTC font extracts only"

    ScopedTrueTypeFont aSftTTF;
    SFErrCodes nRC = aSftTTF.open(xRawFontData.get(), xRawFontData.size(), nFaceNum, pFont->GetFontCharMap());
    if( nRC != SFErrCodes::Ok )
        return;

    SalGraphics::GetGlyphWidths(*aSftTTF.get(), *pFont, bVertical, rWidths, rUnicodeEnc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
