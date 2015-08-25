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

#include "winlayout.hxx"

#include "osl/module.h"
#include "osl/file.h"

#include <comphelper/windowserrorstring.hxx>

#include <opengl/texture.hxx>
#include <opengl/win/gdiimpl.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <win/salgdi.h>
#include <win/saldata.hxx>

#include "sft.hxx"
#include "sallayout.hxx"

#include <cstdio>
#include <cstdlib>

#include <sal/alloca.h>

#include <algorithm>

#include <shlwapi.h>
#include <winver.h>

#include <unordered_map>

typedef std::unordered_map<int,int> IntMap;

// Graphite headers
#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <i18nlangtag/languagetag.hxx>
#include <graphite_features.hxx>
#endif

#define DROPPED_OUTGLYPH 0xFFFF

#include <config_mingw.h>

struct OpenGLGlyphCacheChunk
{
    WORD mnFirstGlyph;
    int mnGlyphCount;
    std::vector<Rectangle> maLocation;
    std::shared_ptr<OpenGLTexture> mpTexture;
    int mnAscentPlusIntLeading;
    bool mbVertical;
};

// win32 specific physical font instance
class ImplWinFontEntry : public ImplFontEntry
{
public:
    explicit                ImplWinFontEntry( FontSelectPattern& );
    virtual                 ~ImplWinFontEntry();

private:
    // TODO: also add HFONT??? Watch out for issues with too many active fonts...

public:
    SCRIPT_CACHE&           GetScriptCache() const
                            { return maScriptCache; }
private:
    mutable SCRIPT_CACHE    maScriptCache;
    std::vector<OpenGLGlyphCacheChunk> maOpenGLGlyphCache;

public:
    int                     GetCachedGlyphWidth( int nCharCode ) const;
    void                    CacheGlyphWidth( int nCharCode, int nCharWidth );

    bool                    InitKashidaHandling( HDC );
    int                     GetMinKashidaWidth() const { return mnMinKashidaWidth; }
    int                     GetMinKashidaGlyph() const { return mnMinKashidaGlyph; }

private:
    IntMap                  maWidthMap;
    mutable int             mnMinKashidaWidth;
    mutable int             mnMinKashidaGlyph;

public:
    bool                    GlyphIsCached(int nGlyphIndex) const;
    bool                    AddChunkOfGlyphs(int nGlyphIndex, const WinLayout& rLayout, SalGraphics& rGraphics);
    const OpenGLGlyphCacheChunk&  GetCachedGlyphChunkFor(int nGlyphIndex) const;
};

char ColorFor(COLORREF aColor)
{
    if (aColor == RGB(0xFF, 0xFF, 0xFF))
        return ' ';
    else if (aColor == RGB(0x00, 0x00, 0x00))
        return 'X';

    return '0' + (10*(GetRValue(aColor) + GetGValue(aColor) + GetBValue(aColor))) / (0xFF*3);
}

OUString DumpGlyphBitmap(OpenGLGlyphCacheChunk& rChunk, HDC hDC)
{
    HBITMAP hBitmap = static_cast<HBITMAP>(GetCurrentObject(hDC, OBJ_BITMAP));
    if (hBitmap == NULL)
    {
        SAL_WARN("vcl.gdi", "GetCurrentObject failed: " << WindowsErrorString(GetLastError()));
        return "";
    }

    BITMAP aBitmap;
    if (!GetObjectW(hBitmap, sizeof(aBitmap), &aBitmap))
    {
        SAL_WARN("vcl.gdi", "GetObjectW failed: " << WindowsErrorString(GetLastError()));
        return "";
    }

    std::cerr << "Bitmap " << hBitmap << ": " << aBitmap.bmWidth << "x" << aBitmap.bmHeight << ":" << std::endl;

    // Print out start pos of each glyph only in the horizontal font case
    int nPos = 0;
    if (rChunk.mnGlyphCount > 1 && rChunk.maLocation[1].Left() > rChunk.maLocation[0].Left())
    {
        for (int i = 1; i < rChunk.mnGlyphCount && nPos < 75; i++)
        {
            for (int j = nPos; j < rChunk.maLocation[i].Left(); j++)
                std::cerr << " ";
            std::cerr << "!";
            nPos = rChunk.maLocation[i].Left() + 1;
        }
    }
    std::cerr << std::endl;

    for (long y = 0; y < aBitmap.bmHeight; y++)
    {
        for (long x = 0; x < std::min(75l, aBitmap.bmWidth); x++)
            std::cerr << ColorFor(GetPixel(hDC, x, y));
        std::cerr << std::endl;
    }
    return "";
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const std::vector<OpenGLGlyphCacheChunk>& rCache )
{
    stream << "{";
    for (auto i = rCache.cbegin(); i != rCache.cend(); ++i)
    {
        stream << "[" << i->mnFirstGlyph;
        if (i->mnGlyphCount > 1)
            stream << ".." << (i->mnFirstGlyph + i->mnGlyphCount - 1);
        stream << "]";
        if (i+1 != rCache.cend())
        {
            stream << ",";
            assert(i->mnFirstGlyph + i->mnGlyphCount <= (i+1)->mnFirstGlyph);
        }
    }

    return stream << "}";
}

inline void ImplWinFontEntry::CacheGlyphWidth( int nCharCode, int nCharWidth )
{
    maWidthMap[ nCharCode ] = nCharWidth;
}

inline int ImplWinFontEntry::GetCachedGlyphWidth( int nCharCode ) const
{
    IntMap::const_iterator it = maWidthMap.find( nCharCode );
    if( it == maWidthMap.end() )
        return -1;
    return it->second;
}

bool ImplWinFontEntry::GlyphIsCached(int nGlyphIndex) const
{
    if (nGlyphIndex == DROPPED_OUTGLYPH)
        return true;

    for (size_t i = 0; i < maOpenGLGlyphCache.size(); i++)
        if (nGlyphIndex >= maOpenGLGlyphCache[i].mnFirstGlyph &&
            nGlyphIndex < maOpenGLGlyphCache[i].mnFirstGlyph + maOpenGLGlyphCache[i].mnGlyphCount)
            return true;

    return false;
}

bool ImplWinFontEntry::AddChunkOfGlyphs(int nGlyphIndex, const WinLayout& rLayout, SalGraphics& rGraphics)
{
    const int DEFAULT_CHUNK_SIZE = 20;

    if (nGlyphIndex == DROPPED_OUTGLYPH)
        return true;

    SAL_INFO("vcl.gdi.opengl", "this=" << this << " " << nGlyphIndex << " old: " << maOpenGLGlyphCache);

    auto n = maOpenGLGlyphCache.begin();
    while (n != maOpenGLGlyphCache.end() &&
           nGlyphIndex > n->mnFirstGlyph)
        ++n;
    assert(n == maOpenGLGlyphCache.end() || nGlyphIndex < n->mnFirstGlyph);

    int nCount = DEFAULT_CHUNK_SIZE;
    if (n != maOpenGLGlyphCache.end() && nGlyphIndex + nCount >= n->mnFirstGlyph)
        nCount = n->mnFirstGlyph - nGlyphIndex;

    if (nCount < DEFAULT_CHUNK_SIZE)
    {
        if (n == maOpenGLGlyphCache.begin())
        {
            nGlyphIndex = std::max(0, n->mnFirstGlyph - DEFAULT_CHUNK_SIZE);
        }
        else
        {
            nGlyphIndex = std::max(n[-1].mnFirstGlyph + n[-1].mnGlyphCount,
                                   n->mnFirstGlyph - DEFAULT_CHUNK_SIZE);
        }
        nCount = n->mnFirstGlyph - nGlyphIndex;
    }

    OpenGLGlyphCacheChunk aChunk;
    aChunk.mnFirstGlyph = nGlyphIndex;
    aChunk.mnGlyphCount = nCount;

    std::vector<WORD> aGlyphIndices(nCount);
    for (int i = 0; i < nCount; i++)
        aGlyphIndices[i] = nGlyphIndex + i;

    HDC hDC = CreateCompatibleDC(rLayout.mhDC);
    if (hDC == NULL)
    {
        SAL_WARN("vcl.gdi", "CreateCompatibleDC failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    HFONT hOrigFont = static_cast<HFONT>(SelectObject(hDC, rLayout.mhFont));
    if (hOrigFont == NULL)
    {
        SAL_WARN("vcl.gdi", "SelectObject failed: " << WindowsErrorString(GetLastError()));
        DeleteDC(hDC);
        return false;
    }

    SIZE aSize;

    if (!GetTextExtentExPointI(hDC, aGlyphIndices.data(), nCount, 0, NULL, NULL, &aSize))
    {
        SAL_WARN("vcl.gdi", "GetTextExtentExPointI failed: " << WindowsErrorString(GetLastError()));
        SelectObject(hDC, hOrigFont);
        DeleteDC(hDC);
        return false;
    }

    std::vector<ABC> aABC(nCount);
    if (!GetCharABCWidthsI(hDC, 0, nCount, aGlyphIndices.data(), aABC.data()))
    {
        SAL_WARN("vcl.gdi", "GetCharABCWidthsI failed: " << WindowsErrorString(GetLastError()));
        SelectObject(hDC, hOrigFont);
        DeleteDC(hDC);
        return false;
    }

    for (int i = 0; i < nCount; i++)
        std::cerr << aABC[i].abcA << ":" << aABC[i].abcB << ":" << aABC[i].abcC << " ";
    std::cerr << std::endl;

    // Try hard to avoid overlap as we want to be able to use
    // individual rectangles for each glyph. The ABC widths don't
    // take anti-alising into consideration. Let's hope that leaving
    // four pixels of "extra" space inbetween glyphs will help.
    std::vector<int> aDX(nCount);
    int totWidth = 0;
    for (int i = 0; i < nCount; i++)
    {
        aDX[i] = aABC[i].abcB + std::abs(aABC[i].abcC);
        if (i == 0)
            aDX[0] += std::abs(aABC[0].abcA);
        if (i < nCount-1)
            aDX[i] += std::abs(aABC[i+1].abcA);
        aDX[i] += 4;
        totWidth += aDX[i];
    }

    TEXTMETRICW aTextMetric;
    if (!GetTextMetricsW(hDC, &aTextMetric))
    {
        SAL_WARN("vcl.gdi", "GetTextMetrics failed: " << WindowsErrorString(GetLastError()));
        SelectObject(hDC, hOrigFont);
        DeleteDC(hDC);
        return false;
    }
    aChunk.mnAscentPlusIntLeading = aTextMetric.tmAscent + aTextMetric.tmInternalLeading;

    LOGFONTW aLogfont;
    if (!GetObjectW(rLayout.mhFont, sizeof(aLogfont), &aLogfont))
    {
        SAL_WARN("vcl.gdi", "GetObject failed: " << WindowsErrorString(GetLastError()));
        SelectObject(hDC, hOrigFont);
        DeleteDC(hDC);
        return false;
    }

    wchar_t sFaceName[200];
    int nFaceNameLen = GetTextFaceW(hDC, SAL_N_ELEMENTS(sFaceName), sFaceName);
    if (!nFaceNameLen)
    {
        SAL_WARN("vcl.gdi", "GetTextFace failed: " << WindowsErrorString(GetLastError()));
        SelectObject(hDC, hOrigFont);
        DeleteDC(hDC);
        return false;
    }

    SAL_INFO("vcl.gdi.opengl", OUString(sFaceName, nFaceNameLen) <<
             ": Escapement=" << aLogfont.lfEscapement <<
             " Orientation=" << aLogfont.lfOrientation <<
             " Ascent=" << aTextMetric.tmAscent <<
             " InternalLeading=" << aTextMetric.tmInternalLeading <<
             " Size=(" << aSize.cx << "," << aSize.cy << ") totWidth=" << totWidth);

    if (SelectObject(hDC, hOrigFont) == NULL)
        SAL_WARN("vcl.gdi", "SelectObject failed: " << WindowsErrorString(GetLastError()));
    if (!DeleteDC(hDC))
        SAL_WARN("vcl.gdi", "DeleteDC failed: " << WindowsErrorString(GetLastError()));

    // Leave two pixels of extra space also at top and bottom
    int nBitmapWidth, nBitmapHeight;
    if (sFaceName[0] == '@')
    {
        nBitmapWidth = aSize.cy + 4;
        nBitmapHeight = totWidth;
        aChunk.mbVertical = true;
    }
    else
    {
        nBitmapWidth = totWidth;
        nBitmapHeight = aSize.cy + 4;
        aChunk.mbVertical = false;
    }

    if (aChunk.mbVertical && aLogfont.lfEscapement != 2700)
        return false;

    OpenGLCompatibleDC aDC(rGraphics, 0, 0, nBitmapWidth, nBitmapHeight);

    HFONT hNonAntialiasedFont = NULL;

#ifdef DBG_UTIL
    static bool bNoAntialias = (std::getenv("VCL_GLYPH_CACHING_HACK_NO_ANTIALIAS") != NULL);
    if (bNoAntialias)
    {
        aLogfont.lfQuality = NONANTIALIASED_QUALITY;
        hNonAntialiasedFont = CreateFontIndirectW(&aLogfont);
        if (hNonAntialiasedFont == NULL)
        {
            SAL_WARN("vcl.gdi", "CreateFontIndirect failed: " << WindowsErrorString(GetLastError()));
            return false;
        }
    }
#endif

    hOrigFont = SelectFont(aDC.getCompatibleHDC(), hNonAntialiasedFont != NULL ? hNonAntialiasedFont : rLayout.mhFont);
    if (hOrigFont == NULL)
    {
        SAL_WARN("vcl.gdi", "SelectObject failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
    SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

    aDC.fill(MAKE_SALCOLOR(0xff, 0xff, 0xff));

    // The 2,2 is for the extra space
    int nY = 2;
    int nX = 2;
    if (aChunk.mbVertical)
        nX += aDX[0];
    if (!ExtTextOutW(aDC.getCompatibleHDC(), nX, nY, ETO_GLYPH_INDEX, NULL, aGlyphIndices.data(), nCount, aDX.data()))
    {
        SAL_WARN("vcl.gdi", "ExtTextOutW failed: " << WindowsErrorString(GetLastError()));
        SelectFont(aDC.getCompatibleHDC(), hOrigFont);
        if (hNonAntialiasedFont != NULL)
            DeleteObject(hNonAntialiasedFont);
        return false;
    }

    aChunk.maLocation.resize(nCount);
    UINT nPos = 0;
    for (int i = 0; i < nCount; i++)
    {
        if (aChunk.mbVertical)
        {
            aChunk.maLocation[i].Left() = 0;
            aChunk.maLocation[i].Right() = nBitmapWidth;
            aChunk.maLocation[i].Top() = nPos;
            aChunk.maLocation[i].Bottom() = nPos + aDX[i];
            nPos = aChunk.maLocation[i].Bottom();
        }
        else
        {
            aChunk.maLocation[i].Left() = nPos;
            aChunk.maLocation[i].Right() = nPos + aDX[i];
            nPos = aChunk.maLocation[i].Right();
            aChunk.maLocation[i].Top() = 0;
            aChunk.maLocation[i].Bottom() = aSize.cy + 4;
        }
    }

    aChunk.mpTexture = std::unique_ptr<OpenGLTexture>(aDC.getTexture());

    maOpenGLGlyphCache.insert(n, aChunk);

    SelectFont(aDC.getCompatibleHDC(), hOrigFont);
    if (hNonAntialiasedFont != NULL)
        DeleteObject(hNonAntialiasedFont);

    SAL_INFO("vcl.gdi.opengl", "this=" << this << " now: " << maOpenGLGlyphCache << DumpGlyphBitmap(aChunk, aDC.getCompatibleHDC()));

    return true;
}

const OpenGLGlyphCacheChunk& ImplWinFontEntry::GetCachedGlyphChunkFor(int nGlyphIndex) const
{
    auto i = maOpenGLGlyphCache.cbegin();
    while (i != maOpenGLGlyphCache.cend() && nGlyphIndex >= i->mnFirstGlyph + i->mnGlyphCount)
        ++i;
    assert(i != maOpenGLGlyphCache.cend());
    assert(nGlyphIndex >= i->mnFirstGlyph && nGlyphIndex < i->mnFirstGlyph + i->mnGlyphCount);
    return *i;
}

WinLayout::WinLayout(HDC hDC, const ImplWinFontData& rWFD, ImplWinFontEntry& rWFE, bool bUseOpenGL)
:   mhDC( hDC ),
    mhFont( (HFONT)GetCurrentObject(hDC,OBJ_FONT) ),
    mnBaseAdv( 0 ),
    mfFontScale( 1.0 ),
    mrWinFontData( rWFD ),
    mrWinFontEntry(rWFE),
    mbUseOpenGL(bUseOpenGL)
{}

void WinLayout::InitFont() const
{
    SelectObject( mhDC, mhFont );
}

// Using reasonably sized fonts to emulate huge fonts works around
// a lot of problems in printer and display drivers. Huge fonts are
// mostly used by high resolution reference devices which are never
// painted to anyway. In the rare case that a huge font needs to be
// displayed somewhere then the workaround doesn't help anymore.
// If the drivers fail silently for huge fonts, so be it...
HFONT WinLayout::DisableFontScaling() const
{
    if( mfFontScale == 1.0 )
        return 0;

    LOGFONTW aLogFont;
    GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
    aLogFont.lfHeight = (LONG)(mfFontScale * aLogFont.lfHeight);
    aLogFont.lfWidth  = (LONG)(mfFontScale * aLogFont.lfWidth);
    HFONT hHugeFont = CreateFontIndirectW( &aLogFont);
    if( !hHugeFont )
        return 0;

    return SelectFont( mhDC, hHugeFont );
}

SCRIPT_CACHE& WinLayout::GetScriptCache() const
{
    return mrWinFontEntry.GetScriptCache();
}

void WinLayout::DrawText(SalGraphics& rGraphics) const
{
    WinSalGraphics& rWinGraphics = static_cast<WinSalGraphics&>(rGraphics);
    HDC hDC = rWinGraphics.getHDC();

    if (!mbUseOpenGL)
    {
        // no OpenGL, just classic rendering
        DrawTextImpl(hDC);
    }
    else if (CacheGlyphs(rGraphics) &&
             DrawCachedGlyphs(rGraphics))
    {
        // Nothing
    }
    else
    {
        // We have to render the text to a hidden texture, and draw it.
        //
        // Note that Windows GDI does not really support the alpha correctly
        // when drawing - ie. it draws nothing to the alpha channel when
        // rendering the text, even the antialiasing is done as 'real' pixels,
        // not alpha...
        //
        // Luckily, this does not really limit us:
        //
        // To blend properly, we draw the texture, but then use it as an alpha
        // channel for solid color (that will define the text color).  This
        // destroys the subpixel antialiasing - turns it into 'classic'
        // antialiasing - but that is the best we can do, because the subpixel
        // antialiasing needs to know what is in the background: When the
        // background is white, or white-ish, it does the subpixel, but when
        // there is a color, it just darkens the color (and does this even
        // when part of the character is on a colored background, and part on
        // white).  It has to work this way, the results would look strange
        // otherwise.
        //
        // For the GL rendering to work even with the subpixel antialiasing,
        // we would need to get the current texture from the screen, let GDI
        // draw the text to it (so that it can decide well where to use the
        // subpixel and where not), and draw the result - but in that case we
        // don't need alpha anyway.
        //
        // TODO: check the performance of this 2nd approach at some stage and
        // switch to that if it performs well.

        Rectangle aRect;
        GetBoundRect(rGraphics, aRect);

        OpenGLCompatibleDC aDC(rGraphics, aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight());

        // we are making changes to the DC, make sure we got a new one
        assert(aDC.getCompatibleHDC() != hDC);

        // setup the hidden DC with black color and white background, we will
        // use the result of the text drawing later as a mask only
        HFONT hOrigFont = SelectFont(aDC.getCompatibleHDC(), mhFont);

        SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
        SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

        UINT nTextAlign = GetTextAlign(hDC);
        SetTextAlign(aDC.getCompatibleHDC(), nTextAlign);

        // the actual drawing
        DrawTextImpl(aDC.getCompatibleHDC());

        COLORREF color = GetTextColor(hDC);
        SalColor salColor = MAKE_SALCOLOR(GetRValue(color), GetGValue(color), GetBValue(color));

        WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(rWinGraphics.mpImpl.get());
        if (pImpl)
        {
            pImpl->PreDraw();

            std::unique_ptr<OpenGLTexture> xTexture(aDC.getTexture());
            if (xTexture)
                pImpl->DrawMask(*xTexture, salColor, aDC.getTwoRect());

            pImpl->PostDraw();
        }

        SelectFont(aDC.getCompatibleHDC(), hOrigFont);
    }
}

struct VisualItem
{
public:
    SCRIPT_ITEM*    mpScriptItem;
    int             mnMinGlyphPos;
    int             mnEndGlyphPos;
    int             mnMinCharPos;
    int             mnEndCharPos;
    //long          mnPixelWidth;
    int             mnXOffset;
    ABC             maABCWidths;
    bool            mbHasKashidas;

public:
    bool            IsEmpty() const { return (mnEndGlyphPos <= 0); }
    bool            IsRTL() const { return mpScriptItem->a.fRTL; }
    bool            HasKashidas() const { return mbHasKashidas; }
};

static bool bUspInited = false;

static bool bManualCellAlign = true;

static void InitUSP()
{
    // get the usp10.dll version info
    HMODULE usp10 = GetModuleHandle("usp10.dll");
    void *pScriptIsComplex = reinterpret_cast< void* >( GetProcAddress(usp10, "ScriptIsComplex"));
    int nUspVersion = 0;
    rtl_uString* pModuleURL = NULL;
    osl_getModuleURLFromAddress( pScriptIsComplex, &pModuleURL );
    rtl_uString* pModuleFileName = NULL;
    if( pModuleURL )
        osl_getSystemPathFromFileURL( pModuleURL, &pModuleFileName );
    const sal_Unicode* pModuleFileCStr = NULL;
    if( pModuleFileName )
        pModuleFileCStr = rtl_uString_getStr( pModuleFileName );
    if( pModuleFileCStr )
    {
        DWORD nHandle;
        DWORD nBufSize = GetFileVersionInfoSizeW( const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(pModuleFileCStr)), &nHandle );
        char* pBuffer = (char*)alloca( nBufSize );
        BOOL bRC = GetFileVersionInfoW( const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(pModuleFileCStr)), nHandle, nBufSize, pBuffer );
        VS_FIXEDFILEINFO* pFixedFileInfo = NULL;
        UINT nFixedFileSize = 0;
        if( bRC )
            VerQueryValueW( pBuffer, const_cast<LPWSTR>(L"\\"), (void**)&pFixedFileInfo, &nFixedFileSize );
        if( pFixedFileInfo && pFixedFileInfo->dwSignature == 0xFEEF04BD )
            nUspVersion = HIWORD(pFixedFileInfo->dwProductVersionMS) * 10000
                        + LOWORD(pFixedFileInfo->dwProductVersionMS);
    }

    // #i77976# USP>=1.0600 changed the need to manually align glyphs in their cells
    if( nUspVersion >= 10600 )
        bManualCellAlign = false;

    bUspInited = true;
}

UniscribeLayout::UniscribeLayout(HDC hDC, const ImplWinFontData& rWinFontData,
        ImplWinFontEntry& rWinFontEntry, bool bUseOpenGL)
:   WinLayout(hDC, rWinFontData, rWinFontEntry, bUseOpenGL),
    mpScriptItems( NULL ),
    mpVisualItems( NULL ),
    mnItemCount( 0 ),
    mnCharCapacity( 0 ),
    mpLogClusters( NULL ),
    mpCharWidths( NULL ),
    mnSubStringMin( 0 ),
    mnGlyphCount( 0 ),
    mnGlyphCapacity( 0 ),
    mpGlyphAdvances( NULL ),
    mpJustifications( NULL ),
    mpOutGlyphs( NULL ),
    mpGlyphOffsets( NULL ),
    mpVisualAttrs( NULL ),
    mpGlyphs2Chars( NULL ),
    mnMinKashidaWidth( 0 ),
    mnMinKashidaGlyph( 0 ),
    mbDisableGlyphInjection( false )
{}

UniscribeLayout::~UniscribeLayout()
{
    delete[] mpScriptItems;
    delete[] mpVisualItems;
    delete[] mpLogClusters;
    delete[] mpCharWidths;
    delete[] mpOutGlyphs;
    delete[] mpGlyphAdvances;
    delete[] mpJustifications;
    delete[] mpGlyphOffsets;
    delete[] mpVisualAttrs;
    delete[] mpGlyphs2Chars;
}

bool UniscribeLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    // for a base layout only the context glyphs have to be dropped
    // => when the whole string is involved there is no extra context
    typedef std::vector<int> TIntVector;
    TIntVector aDropChars;
    if( rArgs.mnFlags & SalLayoutFlags::ForFallback )
    {
        // calculate superfluous context char positions
        aDropChars.push_back( 0 );
        aDropChars.push_back( rArgs.mnLength );
        int nMin, nEnd;
        bool bRTL;
        for( rArgs.ResetPos(); rArgs.GetNextRun( &nMin, &nEnd, &bRTL ); )
        {
            aDropChars.push_back( nMin );
            aDropChars.push_back( nEnd );
        }
        // prepare aDropChars for binary search which will allow to
        // not bother with visual items that will be dropped anyway
        std::sort( aDropChars.begin(), aDropChars.end() );
    }

    // prepare layout
    // TODO: fix case when recyclying old UniscribeLayout object
    mnMinCharPos = rArgs.mnMinCharPos;
    mnEndCharPos = rArgs.mnEndCharPos;

    // determine script items from string

    // prepare itemization
    // TODO: try to avoid itemization since it costs a lot of performance
    SCRIPT_STATE aScriptState = {0,false,false,false,false,false,false,false,false,0,0};
    aScriptState.uBidiLevel         = bool(rArgs.mnFlags & SalLayoutFlags::BiDiRtl);
    aScriptState.fOverrideDirection = bool(rArgs.mnFlags & SalLayoutFlags::BiDiStrong);
    aScriptState.fDigitSubstitute   = bool(rArgs.mnFlags & SalLayoutFlags::SubstituteDigits);
    aScriptState.fArabicNumContext  = aScriptState.fDigitSubstitute & aScriptState.uBidiLevel;
    DWORD nLangId = 0;  // TODO: get language from font
    SCRIPT_CONTROL aScriptControl = {nLangId,false,false,false,false,false,false,false,false,0};
    aScriptControl.fNeutralOverride = aScriptState.fOverrideDirection;
    aScriptControl.fContextDigits   = bool(rArgs.mnFlags & SalLayoutFlags::SubstituteDigits);
#if HAVE_FMERGENEUTRALITEMS
    aScriptControl.fMergeNeutralItems = true;
#endif
    // determine relevant substring and work only on it
    // when Bidi status is unknown we need to look at the whole string though
    mnSubStringMin = 0;
    int nSubStringEnd = rArgs.mnLength;
    if( aScriptState.fOverrideDirection )
    {
        // TODO: limit substring to portion limits
        mnSubStringMin = rArgs.mnMinCharPos - 8;
        if( mnSubStringMin < 0 )
            mnSubStringMin = 0;
        nSubStringEnd = rArgs.mnEndCharPos + 8;
        if( nSubStringEnd > rArgs.mnLength )
            nSubStringEnd = rArgs.mnLength;

    }

    // now itemize the substring with its context
    for( int nItemCapacity = 16;; nItemCapacity *= 8 )
    {
        mpScriptItems = new SCRIPT_ITEM[ nItemCapacity ];
        HRESULT nRC = ScriptItemize(
            reinterpret_cast<LPCWSTR>(rArgs.mpStr + mnSubStringMin), nSubStringEnd - mnSubStringMin,
            nItemCapacity - 1, &aScriptControl, &aScriptState,
            mpScriptItems, &mnItemCount );
        if( !nRC )  // break loop when everything is correctly itemized
            break;

        // prepare bigger buffers for another itemization round
        delete[] mpScriptItems;
        mpScriptItems = NULL;
        if( nRC != E_OUTOFMEMORY )
            return false;
        if( nItemCapacity > (nSubStringEnd - mnSubStringMin) + 16 )
            return false;
    }

    // calculate the order of visual items
    int nItem, i;

    // adjust char positions by substring offset
    for( nItem = 0; nItem <= mnItemCount; ++nItem )
        mpScriptItems[ nItem ].iCharPos += mnSubStringMin;
    // default visual item ordering
    mpVisualItems = new VisualItem[ mnItemCount ];
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        // initialize char specific item info
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        SCRIPT_ITEM* pScriptItem = &mpScriptItems[ nItem ];
        rVisualItem.mpScriptItem = pScriptItem;
        rVisualItem.mnMinCharPos = pScriptItem[0].iCharPos;
        rVisualItem.mnEndCharPos = pScriptItem[1].iCharPos;
    }

    // reorder visual item order if needed
    if( rArgs.mnFlags & SalLayoutFlags::BiDiStrong )
    {
        // force RTL item ordering if requested
        if( rArgs.mnFlags & SalLayoutFlags::BiDiRtl )
        {
            VisualItem* pVI0 = &mpVisualItems[ 0 ];
            VisualItem* pVI1 = &mpVisualItems[ mnItemCount ];
            while( pVI0 < --pVI1 )
            {
                VisualItem aVtmp = *pVI0;
                *(pVI0++) = *pVI1;
                *pVI1 = aVtmp;
            }
        }
    }
    else if( mnItemCount > 1 )
    {
        // apply bidi algorithm's rule L2 on item level
        // TODO: use faster L2 algorithm
        int nMaxBidiLevel = 0;
        VisualItem* pVI = &mpVisualItems[0];
        VisualItem* const pVIend = pVI + mnItemCount;
        for(; pVI < pVIend; ++pVI )
            if( nMaxBidiLevel < pVI->mpScriptItem->a.s.uBidiLevel )
                nMaxBidiLevel = pVI->mpScriptItem->a.s.uBidiLevel;

        while( --nMaxBidiLevel >= 0 )
        {
            for( pVI = &mpVisualItems[0]; pVI < pVIend; )
            {
                // find item range that needs reordering
                for(; pVI < pVIend; ++pVI )
                    if( nMaxBidiLevel < pVI->mpScriptItem->a.s.uBidiLevel )
                        break;
                VisualItem* pVImin = pVI++;
                for(; pVI < pVIend; ++pVI )
                    if( nMaxBidiLevel >= pVI->mpScriptItem->a.s.uBidiLevel )
                        break;
                VisualItem* pVImax = pVI++;

                // reverse order of items in this range
                while( pVImin < --pVImax )
                {
                    VisualItem aVtmp = *pVImin;
                    *(pVImin++) = *pVImax;
                    *pVImax = aVtmp;
                }
            }
        }
    }

    // allocate arrays
    // TODO: when reusing object reuse old allocations or delete them
    // TODO: use only [nSubStringMin..nSubStringEnd) instead of [0..nSubStringEnd)
    mnCharCapacity  = nSubStringEnd;
    mpLogClusters   = new WORD[ mnCharCapacity ];
    mpCharWidths    = new int[ mnCharCapacity ];

    mnGlyphCount    = 0;
    mnGlyphCapacity = 16 + 4 * (nSubStringEnd - mnSubStringMin); // worst case assumption
    mpGlyphAdvances = new int[ mnGlyphCapacity ];
    mpOutGlyphs     = new WORD[ mnGlyphCapacity ];
    mpGlyphOffsets  = new GOFFSET[ mnGlyphCapacity ];
    mpVisualAttrs   = new SCRIPT_VISATTR[ mnGlyphCapacity ];

    long nXOffset = 0;
    for( int j = mnSubStringMin; j < nSubStringEnd; ++j )
        mpCharWidths[j] = 0;

    // layout script items
    SCRIPT_CACHE& rScriptCache = GetScriptCache();
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // initialize glyph specific item info
        rVisualItem.mnMinGlyphPos = mnGlyphCount;
        rVisualItem.mnEndGlyphPos = 0;
        rVisualItem.mnXOffset     = nXOffset;
        //rVisualItem.mnPixelWidth  = 0;

        // shortcut ignorable items
        if( (rArgs.mnEndCharPos <= rVisualItem.mnMinCharPos)
         || (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos) )
        {
            for( int j = rVisualItem.mnMinCharPos; j < rVisualItem.mnEndCharPos; ++j )
                mpLogClusters[j] = sal::static_int_cast<WORD>(~0U);
            if (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos)
            {   // fdo#47553 adjust "guessed" min (maybe up to -8 off) to
                // actual min so it can be used properly in GetNextGlyphs
                assert(mnSubStringMin <= rVisualItem.mnEndCharPos);
                mnSubStringMin = rVisualItem.mnEndCharPos;
            }
            continue;
        }

        // override bidi analysis if requested
        if( rArgs.mnFlags & SalLayoutFlags::BiDiStrong )
        {
            // FIXME: is this intended ?
            rVisualItem.mpScriptItem->a.fRTL                 = (aScriptState.uBidiLevel & 1);
            rVisualItem.mpScriptItem->a.s.uBidiLevel         = aScriptState.uBidiLevel;
            rVisualItem.mpScriptItem->a.s.fOverrideDirection = aScriptState.fOverrideDirection;
        }

        // convert the unicodes to glyphs
        int nGlyphCount = 0;
        int nCharCount = rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos;
        HRESULT nRC = ScriptShape( mhDC, &rScriptCache,
            reinterpret_cast<LPCWSTR>(rArgs.mpStr + rVisualItem.mnMinCharPos),
            nCharCount,
            mnGlyphCapacity - rVisualItem.mnMinGlyphPos, // problem when >0xFFFF
            &rVisualItem.mpScriptItem->a,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &nGlyphCount );

        // find and handle problems in the unicode to glyph conversion
        if( nRC == USP_E_SCRIPT_NOT_IN_FONT )
        {
            // the whole visual item needs a fallback, but make sure that the next
            // fallback request is limited to the characters in the original request
            // => this is handled in ImplLayoutArgs::PrepareFallback()
            rArgs.NeedFallback( rVisualItem.mnMinCharPos, rVisualItem.mnEndCharPos,
                rVisualItem.IsRTL() );

            // don't bother to do a default layout in a fallback level
            if( rArgs.mnFlags & SalLayoutFlags::ForFallback )
                continue;

            // the primitive layout engine is good enough for the default layout
            rVisualItem.mpScriptItem->a.eScript = SCRIPT_UNDEFINED;
            nRC = ScriptShape( mhDC, &rScriptCache,
                reinterpret_cast<LPCWSTR>(rArgs.mpStr + rVisualItem.mnMinCharPos),
                nCharCount,
                mnGlyphCapacity - rVisualItem.mnMinGlyphPos,
                &rVisualItem.mpScriptItem->a,
                mpOutGlyphs + rVisualItem.mnMinGlyphPos,
                mpLogClusters + rVisualItem.mnMinCharPos,
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                &nGlyphCount );

            if( nRC != 0 )
                continue;

        }
        else if( nRC != 0 )
            // something undefined happened => give up for this visual item
            continue;
        else // if( nRC == 0 )
        {
            // check if there are any NotDef glyphs
            for( i = 0; i < nGlyphCount; ++i )
                if( 0 == mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] )
                    break;
            if( i < nGlyphCount )
            {
                // clip charpos limits to the layout string without context
                int nMinCharPos = rVisualItem.mnMinCharPos;
                if( nMinCharPos < rArgs.mnMinCharPos )
                    nMinCharPos = rArgs.mnMinCharPos;
                int nEndCharPos = rVisualItem.mnEndCharPos;
                if( nEndCharPos > rArgs.mnEndCharPos )
                    nEndCharPos = rArgs.mnEndCharPos;
                // request fallback for individual NotDef glyphs
                do
                {
                    // ignore non-NotDef glyphs
                    if( 0 != mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] )
                        continue;
                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = DROPPED_OUTGLYPH;
                    // request fallback for the whole cell that resulted in a NotDef glyph
                    // TODO: optimize algorithm
                    const bool bRTL = rVisualItem.IsRTL();
                    if( !bRTL )
                    {
                        // request fallback for the left-to-right cell
                        for( int c = nMinCharPos; c < nEndCharPos; ++c )
                        {
                            if( mpLogClusters[ c ] == i )
                            {
                                // #i55716# skip WORDJOINER
                                if( rArgs.mpStr[ c ] == 0x2060 )
                                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = 1;
                                else
                                    rArgs.NeedFallback( c, false );
                           }
                        }
                    }
                    else
                    {
                        // request fallback for the right to left cell
                        for( int c = nEndCharPos; --c >= nMinCharPos; )
                        {
                            if( mpLogClusters[ c ] == i )
                            {
                                // #i55716# skip WORDJOINER
                                if( rArgs.mpStr[ c ] == 0x2060 )
                                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = 1;
                                else
                                    rArgs.NeedFallback( c, true );
                            }
                        }
                    }
                } while( ++i < nGlyphCount );
            }
        }

        // now place the glyphs
        nRC = ScriptPlace( mhDC, &rScriptCache,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            nGlyphCount,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &rVisualItem.mpScriptItem->a,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpGlyphOffsets + rVisualItem.mnMinGlyphPos,
            &rVisualItem.maABCWidths );

        if( nRC != 0 )
            continue;

        // calculate the logical char widths from the glyph layout
        nRC = ScriptGetLogicalWidths(
            &rVisualItem.mpScriptItem->a,
            nCharCount, nGlyphCount,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            mpCharWidths + rVisualItem.mnMinCharPos );

        // update the glyph counters
        mnGlyphCount += nGlyphCount;
        rVisualItem.mnEndGlyphPos = mnGlyphCount;

        // update nXOffset
        int nEndGlyphPos;
        if( GetItemSubrange( rVisualItem, i, nEndGlyphPos ) )
            for(; i < nEndGlyphPos; ++i )
                nXOffset += mpGlyphAdvances[ i ];

        // TODO: shrink glyphpos limits to match charpos/fallback limits
        //pVI->mnMinGlyphPos = nMinGlyphPos;
        //pVI->mnEndGlyphPos = nEndGlyphPos;

        // drop the superfluous context glyphs
        TIntVector::const_iterator it = aDropChars.begin();
        while( it != aDropChars.end() )
        {
            // find matching "drop range"
            int nMinDropPos = *(it++); // begin of drop range
            if( nMinDropPos >= rVisualItem.mnEndCharPos )
                break;
            int nEndDropPos = *(it++); // end of drop range
            if( nEndDropPos <= rVisualItem.mnMinCharPos )
                continue;
            // clip "drop range" to visual item's char range
            if( nMinDropPos <= rVisualItem.mnMinCharPos )
            {
                nMinDropPos = rVisualItem.mnMinCharPos;
                // drop the whole visual item if possible
                if( nEndDropPos >= rVisualItem.mnEndCharPos )
                {
                    rVisualItem.mnEndGlyphPos = 0;
                    break;
                }
            }
            if( nEndDropPos > rVisualItem.mnEndCharPos )
                nEndDropPos = rVisualItem.mnEndCharPos;

            // drop the glyphs which correspond to the charpos range
            // drop the corresponding glyphs in the cluster
            for( int c = nMinDropPos; c < nEndDropPos; ++c )
            {
                int nGlyphPos = mpLogClusters[c] + rVisualItem.mnMinGlyphPos;
                // no need to bother when the cluster was already dropped
                if( mpOutGlyphs[ nGlyphPos ] != DROPPED_OUTGLYPH )
                {
                    for(;;)
                    {
                        mpOutGlyphs[ nGlyphPos ] = DROPPED_OUTGLYPH;
                        // until the end of visual item
                        if( ++nGlyphPos >= rVisualItem.mnEndGlyphPos )
                            break;
                        // until the next cluster start
                        if( mpVisualAttrs[ nGlyphPos ].fClusterStart )
                            break;
                    }
                }
            }
        }
    }

    // scale layout metrics if needed
    // TODO: does it make the code more simple if the metric scaling
    // is moved to the methods that need metric scaling (e.g. FillDXArray())?
    if( mfFontScale != 1.0 )
    {
        mnBaseAdv = (int)((double)mnBaseAdv*mfFontScale);

        for( i = 0; i < mnItemCount; ++i )
            mpVisualItems[i].mnXOffset = (int)((double)mpVisualItems[i].mnXOffset*mfFontScale);

        mnBaseAdv = (int)((double)mnBaseAdv*mfFontScale);
        for( i = 0; i < mnGlyphCount; ++i )
        {
            mpGlyphAdvances[i]   = (int)(mpGlyphAdvances[i] * mfFontScale);
            mpGlyphOffsets[i].du = (LONG)(mpGlyphOffsets[i].du * mfFontScale);
            mpGlyphOffsets[i].dv = (LONG)(mpGlyphOffsets[i].dv * mfFontScale);
            // mpJustifications are still NULL
        }

        for( i = mnSubStringMin; i < nSubStringEnd; ++i )
            mpCharWidths[i] = (int)(mpCharWidths[i] * mfFontScale);
    }

    return true;
}

// calculate the range of relevant glyphs for this visual item
bool UniscribeLayout::GetItemSubrange( const VisualItem& rVisualItem,
    int& rMinGlyphPos, int& rEndGlyphPos ) const
{
    // return early when nothing of interest in this item
    if( rVisualItem.IsEmpty()
     || (rVisualItem.mnEndCharPos <= mnMinCharPos)
     || (mnEndCharPos <= rVisualItem.mnMinCharPos) )
        return false;

    // default: subrange is complete range
    rMinGlyphPos = rVisualItem.mnMinGlyphPos;
    rEndGlyphPos = rVisualItem.mnEndGlyphPos;

    // return early when the whole item is of interest
    if( (mnMinCharPos <= rVisualItem.mnMinCharPos)
     && (rVisualItem.mnEndCharPos <= mnEndCharPos ) )
        return true;

    // get glyph range from char range by looking at cluster boundries
    // TODO: optimize for case that LTR/RTL correspond to monotonous glyph indexes
    rMinGlyphPos = rVisualItem.mnEndGlyphPos;
    int nMaxGlyphPos = 0;

    int i = mnMinCharPos;
    if( i < rVisualItem.mnMinCharPos )
        i = rVisualItem.mnMinCharPos;
    int nCharPosLimit = rVisualItem.mnEndCharPos;
    if( nCharPosLimit > mnEndCharPos )
        nCharPosLimit = mnEndCharPos;
    for(; i < nCharPosLimit; ++i )
    {
        int n = mpLogClusters[ i ] + rVisualItem.mnMinGlyphPos;
        if( rMinGlyphPos > n )
            rMinGlyphPos = n;
        if( nMaxGlyphPos < n )
            nMaxGlyphPos = n;
    }
    if (nMaxGlyphPos > rVisualItem.mnEndGlyphPos)
        nMaxGlyphPos = rVisualItem.mnEndGlyphPos - 1;

    // extend the glyph range to account for all glyphs in referenced clusters
    if( !rVisualItem.IsRTL() ) // LTR-item
    {
        // extend to rightmost glyph of rightmost referenced cluster
        for( i = nMaxGlyphPos; ++i < rVisualItem.mnEndGlyphPos; nMaxGlyphPos = i )
            if( mpVisualAttrs[i].fClusterStart )
                break;
    }
    else // RTL-item
    {
        // extend to leftmost glyph of leftmost referenced cluster
        for( i = rMinGlyphPos; --i >= rVisualItem.mnMinGlyphPos; rMinGlyphPos = i )
            if( mpVisualAttrs[i].fClusterStart )
                break;
    }
    rEndGlyphPos = nMaxGlyphPos + 1;

    return true;
}

int UniscribeLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos,
                                    int& nStartx8, DeviceCoordinate* pGlyphAdvances, int* pCharPosAry,
                                    const PhysicalFontFace** /*pFallbackFonts*/ ) const
{
    // HACK to allow fake-glyph insertion (e.g. for kashidas)
    // TODO: use iterator idiom instead of GetNextGlyphs(...)
    // TODO: else make sure that the limit for glyph injection is sufficient (currently 256)
    int nSubIter = nStartx8 & 0xff;
    int nStart = nStartx8 >> 8;

    // check the glyph iterator
    if( nStart > mnGlyphCount )       // nStart>MAX means no more glyphs
        return 0;

    // find the visual item for the nStart glyph position
    int nItem = 0;
    const VisualItem* pVI = mpVisualItems;
    if( nStart <= 0 )                 // nStart<=0 requests the first visible glyph
    {
        // find first visible item
        for(; nItem < mnItemCount; ++nItem, ++pVI )
            if( !pVI->IsEmpty() )
                break;
        // it is possible that there are glyphs but no valid visual item
        // TODO: get rid of these visual items more early
        if( nItem < mnItemCount )
            nStart = pVI->mnMinGlyphPos;
    }
    else //if( nStart > 0 )           // nStart>0 means absolute glyph pos +1
    {
        --nStart;

        // find matching item
        for(; nItem < mnItemCount; ++nItem, ++pVI )
            if( (nStart >= pVI->mnMinGlyphPos)
            &&  (nStart < pVI->mnEndGlyphPos) )
                break;
    }

    // after the last visual item there are no more glyphs
    if( (nItem >= mnItemCount) || (nStart < 0) )
    {
        nStartx8 = (mnGlyphCount + 1) << 8;
        return 0;
    }

    // calculate the first glyph in the next visual item
    int nNextItemStart = mnGlyphCount;
    while( ++nItem < mnItemCount )
    {
        if( mpVisualItems[nItem].IsEmpty() )
            continue;
        nNextItemStart = mpVisualItems[nItem].mnMinGlyphPos;
        break;
    }

    // get the range of relevant glyphs in this visual item
    int nMinGlyphPos, nEndGlyphPos;
    bool bRC = GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos );
    DBG_ASSERT( bRC, "USPLayout::GNG GISR() returned false" );
    if( !bRC )
    {
        nStartx8 = (mnGlyphCount + 1) << 8;
        return 0;
    }

    // make sure nStart is inside the range of relevant glyphs
    if( nStart < nMinGlyphPos )
        nStart = nMinGlyphPos;

    // calculate the start glyph xoffset relative to layout's base position,
    // advance to next visual glyph position by using adjusted glyph widths
    // TODO: speed up the calculation for nStart!=0 case by using rPos as a cache
    long nXOffset = pVI->mnXOffset;
    const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
    for( int i = nMinGlyphPos; i < nStart; ++i )
        nXOffset += pGlyphWidths[ i ];

    // adjust the nXOffset relative to glyph cluster start
    int c = mnMinCharPos;
    if( !pVI->IsRTL() ) // LTR-case
    {
        // LTR case: subtract the remainder of the cell from xoffset
        int nTmpIndex = mpLogClusters[c];
        while( (--c >= pVI->mnMinCharPos)
            && (nTmpIndex == mpLogClusters[c]) )
            nXOffset -= mpCharWidths[c];
    }
    else // RTL-case
    {
        // RTL case: add the remainder of the cell from xoffset
        int nTmpIndex = mpLogClusters[ pVI->mnEndCharPos - 1 ];
        while( (--c >= pVI->mnMinCharPos)
            && (nTmpIndex == mpLogClusters[c]) )
            nXOffset += mpCharWidths[c];

        // adjust the xoffset if justified glyphs are not positioned at their justified positions yet
        if( mpJustifications && !bManualCellAlign )
           nXOffset += mpJustifications[ nStart ] - mpGlyphAdvances[ nStart ];
    }

    // create mpGlyphs2Chars[] if it is needed later
    if( pCharPosAry && !mpGlyphs2Chars )
    {
        // create and reset the new array
        mpGlyphs2Chars = new int[ mnGlyphCapacity ];
        static const int CHARPOS_NONE = -1;
        for( int i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[i] = CHARPOS_NONE;
        // calculate the char->glyph mapping
        for( nItem = 0; nItem < mnItemCount; ++nItem )
        {
            // ignore invisible visual items
            const VisualItem& rVI = mpVisualItems[ nItem ];
            if( rVI.IsEmpty() )
                continue;

            //Resolves: fdo#33090 Ensure that all glyph slots, even if 0-width
            //or empty due to combining chars etc, map back to a character
            //position so that iterating over glyph slots one at a time for
            //glyph fallback can keep context as to what characters are the
            //inputs that caused a missing glyph in a given font.

            //See: fdo#46923/fdo#46896/fdo#46750 for extra complexities
            {
                int dir = 1;
                int out = rVI.mnMinCharPos;
                if (rVI.IsRTL())
                {
                    dir = -1;
                    out = rVI.mnEndCharPos-1;
                }
                for(c = rVI.mnMinCharPos; c < rVI.mnEndCharPos; ++c)
                {
                    int i = out - mnSubStringMin;
                    mpGlyphs2Chars[i] = c;
                    out += dir;
                }
            }

            // calculate the mapping by using mpLogClusters[]
            // mpGlyphs2Chars[] should obey the logical order
            // => reversing the loop does this by overwriting higher logicals
            for( c = rVI.mnEndCharPos; --c >= rVI.mnMinCharPos; )
            {
                int i = mpLogClusters[c] + rVI.mnMinGlyphPos;
                mpGlyphs2Chars[i] = c;
            }
            // use a heuristic to fill the gaps in the glyphs2chars array
            c = !rVI.IsRTL() ? rVI.mnMinCharPos : rVI.mnEndCharPos - 1;
            for( int i = rVI.mnMinGlyphPos; i < rVI.mnEndGlyphPos; ++i ) {
                if( mpGlyphs2Chars[i] == CHARPOS_NONE )
                    mpGlyphs2Chars[i] = c;
                else
                    c = mpGlyphs2Chars[i];
            }
        }
    }

    // calculate the absolute position of the first result glyph in pixel units
    const GOFFSET aGOffset = mpGlyphOffsets[ nStart ];
    Point aRelativePos( nXOffset + aGOffset.du, -aGOffset.dv );
    rPos = GetDrawPosition( aRelativePos );

    // fill the result arrays
    int nCount = 0;
    while( nCount < nLen )
    {
        // prepare return values
        sal_GlyphId aGlyphId = mpOutGlyphs[ nStart ];
        int nGlyphWidth = pGlyphWidths[ nStart ];
        int nCharPos = -1;    // no need to determine charpos
        if( mpGlyphs2Chars )  // unless explicitly requested+provided
        {
            nCharPos = mpGlyphs2Chars[ nStart ];
            assert(-1 != nCharPos);
        }

        // inject kashida glyphs if needed
        if( !mbDisableGlyphInjection
        && mpJustifications
        && mnMinKashidaWidth
        && mpVisualAttrs[nStart].uJustification >= SCRIPT_JUSTIFY_ARABIC_NORMAL )
        {
            // prepare draw position adjustment
            int nExtraOfs = (nSubIter++) * mnMinKashidaWidth;
            // calculate space available for the injected glyphs
               nGlyphWidth = mpGlyphAdvances[ nStart ];
            const int nExtraWidth = mpJustifications[ nStart ] - nGlyphWidth;
            const int nToFillWidth = nExtraWidth - nExtraOfs;
            if( (4*nToFillWidth >= mnMinKashidaWidth)    // prevent glyph-injection if there is no room
            ||  ((nSubIter > 1) && (nToFillWidth > 0)) ) // unless they can overlap with others
            {
                // handle if there is not sufficient room for a full glyph
                if( nToFillWidth < mnMinKashidaWidth )
                {
                    // overlap it with the previously injected glyph if possible
                    int nOverlap = mnMinKashidaWidth - nToFillWidth;
                    // else overlap it with both neighboring glyphs
                    if( nSubIter <= 1 )
                        nOverlap /= 2;
                    nExtraOfs -= nOverlap;
                }
                nGlyphWidth = mnMinKashidaWidth;
                aGlyphId = mnMinKashidaGlyph;
                nCharPos = -1;
            }
            else
            {
                nExtraOfs += nToFillWidth;    // at right of cell
                nSubIter = 0;                 // done with glyph injection
            }
            if( !bManualCellAlign )
                nExtraOfs -= nExtraWidth;     // adjust for right-aligned cells

            // adjust the draw position for the injected-glyphs case
            if( nExtraOfs )
            {
                aRelativePos.X() += nExtraOfs;
                rPos = GetDrawPosition( aRelativePos );
            }
        }

        // update return values
        *(pGlyphs++) = aGlyphId;
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = nGlyphWidth;
        if( pCharPosAry )
            *(pCharPosAry++) = nCharPos;

        // increment counter of returned glyphs
        ++nCount;

        // reduce code complexity by returning early in glyph-injection case
           if( nSubIter != 0 )
               break;

        // stop after the last visible glyph in this visual item
        if( ++nStart >= nEndGlyphPos )
        {
            nStart = nNextItemStart;
            break;
        }

        // RTL-justified glyph positioning is not easy
        // simplify the code by just returning only one glyph at a time
        if( mpJustifications && pVI->IsRTL() )
            break;

        // stop when the x-position of the next glyph is unexpected
        if( !pGlyphAdvances  )
            if( (mpGlyphOffsets && (mpGlyphOffsets[nStart].du != aGOffset.du) )
             || (mpJustifications && (mpJustifications[nStart] != mpGlyphAdvances[nStart]) ) )
                break;

        // stop when the y-position of the next glyph is unexpected
        if( mpGlyphOffsets && (mpGlyphOffsets[nStart].dv != aGOffset.dv) )
            break;
    }

    ++nStart;
    nStartx8 = (nStart << 8) + nSubIter;
    return nCount;
}

void UniscribeLayout::MoveGlyph( int nStartx8, long nNewXPos )
{
    DBG_ASSERT( !(nStartx8 & 0xff), "USP::MoveGlyph(): glyph injection not disabled!" );
    int nStart = nStartx8 >> 8;
    if( nStart > mnGlyphCount )
        return;

    VisualItem* pVI = mpVisualItems;
    int nMinGlyphPos = 0, nEndGlyphPos;
    if( nStart == 0 )               // nStart==0 for first visible glyph
    {
        for( int i = mnItemCount; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos ) )
                break;
        nStart = nMinGlyphPos;
        DBG_ASSERT( nStart <= mnGlyphCount, "USPLayout::MoveG overflow" );
    }
    else //if( nStart > 0 )         // nStart>0 means absolute_glyphpos+1
    {
        --nStart;
        for( int i = mnItemCount; --i >= 0; ++pVI )
            if( (nStart >= pVI->mnMinGlyphPos) && (nStart < pVI->mnEndGlyphPos) )
                break;
        bool bRC = GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos );
    (void)bRC; // avoid var-not-used warning
        DBG_ASSERT( bRC, "USPLayout::MoveG GISR() returned false" );
    }

    long nDelta = nNewXPos - pVI->mnXOffset;
    if( nStart > nMinGlyphPos )
    {
        // move the glyph by expanding its left glyph but ignore dropped glyphs
        int i, nLastUndropped = nMinGlyphPos - 1;
        for( i = nMinGlyphPos; i < nStart; ++i )
        {
            if (mpOutGlyphs[i] != DROPPED_OUTGLYPH)
            {
                nDelta -= (mpJustifications)? mpJustifications[ i ] : mpGlyphAdvances[ i ];
                nLastUndropped = i;
            }
        }
        if (nLastUndropped >= nMinGlyphPos)
        {
            mpGlyphAdvances[ nLastUndropped ] += nDelta;
            if (mpJustifications) mpJustifications[ nLastUndropped ] += nDelta;
        }
        else
        {
            pVI->mnXOffset += nDelta;
        }
    }
    else
    {
        // move the visual item by having an offset
        pVI->mnXOffset += nDelta;
    }
    // move subsequent items - this often isn't necessary because subsequent
    // moves will correct subsequent items. However, if there is a contiguous
    // range not involving fallback which spans items, this will be needed
    while (++pVI - mpVisualItems < mnItemCount)
    {
        pVI->mnXOffset += nDelta;
    }
}

void UniscribeLayout::DropGlyph( int nStartx8 )
{
    DBG_ASSERT( !(nStartx8 & 0xff), "USP::DropGlyph(): glyph injection not disabled!" );
    int nStart = nStartx8 >> 8;
    DBG_ASSERT( nStart<=mnGlyphCount, "USPLayout::MoveG nStart overflow" );

    if( nStart > 0 )        // nStart>0 means absolute glyph pos + 1
        --nStart;
    else                    // nStart<=0 for first visible glyph
    {
        VisualItem* pVI = mpVisualItems;
        for( int i = mnItemCount, nDummy; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nStart, nDummy ) )
                break;
        DBG_ASSERT( nStart <= mnGlyphCount, "USPLayout::DropG overflow" );

        int j = pVI->mnMinGlyphPos;
        while (mpOutGlyphs[j] == DROPPED_OUTGLYPH) j++;
        if (j == nStart)
        {
            pVI->mnXOffset += ((mpJustifications)? mpJustifications[nStart] : mpGlyphAdvances[nStart]);
        }
    }

    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}

void UniscribeLayout::Simplify( bool /*bIsBase*/ )
{
    static const WCHAR cDroppedGlyph = DROPPED_OUTGLYPH;
    int i;
    // if there are no dropped glyphs don't bother
    for( i = 0; i < mnGlyphCount; ++i )
        if( mpOutGlyphs[ i ] == cDroppedGlyph )
            break;
    if( i >= mnGlyphCount )
        return;

    // prepare for sparse layout
    // => make sure mpGlyphs2Chars[] exists
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCapacity ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[ i ] = -1;
        for( int nItem = 0; nItem < mnItemCount; ++nItem )
        {
            // skip invisible items
            VisualItem& rVI = mpVisualItems[ nItem ];
            if( rVI.IsEmpty() )
                continue;
            for( i = rVI.mnEndCharPos; --i >= rVI.mnMinCharPos; )
            {
                int j = mpLogClusters[ i ] + rVI.mnMinGlyphPos;
                mpGlyphs2Chars[ j ] = i;
            }
        }
    }

    // remove the dropped glyphs
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVI = mpVisualItems[ nItem ];
        if( rVI.IsEmpty() )
            continue;

        // mark replaced character widths
        for( i = rVI.mnMinCharPos; i < rVI.mnEndCharPos; ++i )
        {
            int j = mpLogClusters[ i ] + rVI.mnMinGlyphPos;
            if( mpOutGlyphs[ j ] == cDroppedGlyph )
                mpCharWidths[ i ] = 0;
        }

        // handle dropped glyphs at start of visual item
        int nMinGlyphPos, nEndGlyphPos, nOrigMinGlyphPos = rVI.mnMinGlyphPos;
        GetItemSubrange( rVI, nMinGlyphPos, nEndGlyphPos );
        i = nMinGlyphPos;
        while( (i < nEndGlyphPos) && (mpOutGlyphs[i] == cDroppedGlyph) )
        {
            rVI.mnMinGlyphPos = ++i;
        }

        // when all glyphs in item got dropped mark it as empty
        if( i >= nEndGlyphPos )
        {
            rVI.mnEndGlyphPos = 0;
            continue;
        }
        // If there are still glyphs in the cluster and mnMinGlyphPos
        // has changed then we need to remove the dropped glyphs at start
        // to correct logClusters, which is unsigned and relative to the
        // item start.
        if (rVI.mnMinGlyphPos != nOrigMinGlyphPos)
        {
            // drop any glyphs in the visual item outside the range
            for (i = nOrigMinGlyphPos; i < nMinGlyphPos; i++)
                mpOutGlyphs[ i ] = cDroppedGlyph;
            rVI.mnMinGlyphPos = i = nOrigMinGlyphPos;
        }

        // handle dropped glyphs in the middle of visual item
        for(; i < nEndGlyphPos; ++i )
            if( mpOutGlyphs[ i ] == cDroppedGlyph )
                break;
        int j = i;
        while( ++i < nEndGlyphPos )
        {
            if( mpOutGlyphs[ i ] == cDroppedGlyph )
                continue;
            mpOutGlyphs[ j ]      = mpOutGlyphs[ i ];
            mpGlyphOffsets[ j ]   = mpGlyphOffsets[ i ];
            mpVisualAttrs[ j ]    = mpVisualAttrs[ i ];
            mpGlyphAdvances[ j ]  = mpGlyphAdvances[ i ];
            if( mpJustifications )
                mpJustifications[ j ] = mpJustifications[ i ];
            const int k = mpGlyphs2Chars[ i ];
            mpGlyphs2Chars[ j ]   = k;
            const int nRelGlyphPos = (j++) - rVI.mnMinGlyphPos;
            if( k < 0) // extra glyphs are already mapped
                continue;
            mpLogClusters[ k ] = static_cast<WORD>(nRelGlyphPos);
        }

        rVI.mnEndGlyphPos = j;
    }
}

void UniscribeLayout::DrawTextImpl(HDC hDC) const
{
    HFONT hOrigFont = DisableFontScaling();

    int nBaseClusterOffset = 0;
    int nBaseGlyphPos = -1;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        if( nBaseGlyphPos < 0 )
        {
            // adjust draw position relative to cluster start
            if( rVisualItem.IsRTL() )
                nBaseGlyphPos = nEndGlyphPos - 1;
            else
                nBaseGlyphPos = nMinGlyphPos;

            int i = mnMinCharPos;
            while( (--i >= rVisualItem.mnMinCharPos)
                && (nBaseGlyphPos == mpLogClusters[i]) )
                 nBaseClusterOffset += mpCharWidths[i];

            if( !rVisualItem.IsRTL() )
                nBaseClusterOffset = -nBaseClusterOffset;
        }

        // now draw the matching glyphs in this item
        Point aRelPos( rVisualItem.mnXOffset + nBaseClusterOffset, 0 );
        Point aPos = GetDrawPosition( aRelPos );
        SCRIPT_CACHE& rScriptCache = GetScriptCache();
        ScriptTextOut(hDC, &rScriptCache,
            aPos.X(), aPos.Y(), 0, NULL,
            &rVisualItem.mpScriptItem->a, NULL, 0,
            mpOutGlyphs + nMinGlyphPos,
            nEndGlyphPos - nMinGlyphPos,
            mpGlyphAdvances + nMinGlyphPos,
            mpJustifications ? mpJustifications + nMinGlyphPos : NULL,
            mpGlyphOffsets + nMinGlyphPos);
    }

    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));
}

bool UniscribeLayout::CacheGlyphs(SalGraphics& rGraphics) const
{
    static bool bDoGlyphCaching = (std::getenv("SAL_DISABLE_GLYPH_CACHING") == NULL);

    if (!bDoGlyphCaching)
        return false;

    for (int i = 0; i < mnGlyphCount; i++)
    {
        if (mrWinFontEntry.GlyphIsCached(mpOutGlyphs[i]))
            continue;

        if (!mrWinFontEntry.AddChunkOfGlyphs(mpOutGlyphs[i], *this, rGraphics))
            return false;
    }

    return true;
}

bool UniscribeLayout::DrawCachedGlyphs(SalGraphics& rGraphics) const
{
    WinSalGraphics& rWinGraphics = static_cast<WinSalGraphics&>(rGraphics);
    HDC hDC = rWinGraphics.getHDC();

    Rectangle aRect;
    GetBoundRect(rGraphics, aRect);

    COLORREF color = GetTextColor(hDC);
    SalColor salColor = MAKE_SALCOLOR(GetRValue(color), GetGValue(color), GetBValue(color));

    WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(rWinGraphics.mpImpl.get());
    if (!pImpl)
        return false;

    pImpl->PreDraw();

    // FIXME: This code snippet is mostly copied from the one in
    // UniscribeLayout::DrawTextImpl. Should be factored out.
    int nBaseClusterOffset = 0;
    int nBaseGlyphPos = -1;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        if( nBaseGlyphPos < 0 )
        {
            // adjust draw position relative to cluster start
            if( rVisualItem.IsRTL() )
                nBaseGlyphPos = nEndGlyphPos - 1;
            else
                nBaseGlyphPos = nMinGlyphPos;

            int i = mnMinCharPos;
            while( (--i >= rVisualItem.mnMinCharPos)
                && (nBaseGlyphPos == mpLogClusters[i]) )
                 nBaseClusterOffset += mpCharWidths[i];

            if( !rVisualItem.IsRTL() )
                nBaseClusterOffset = -nBaseClusterOffset;
        }

        // now draw the matching glyphs in this item
        Point aRelPos( rVisualItem.mnXOffset + nBaseClusterOffset, 0 );
        Point aPos = GetDrawPosition( aRelPos );

        int nAdvance = 0;
        for (int i = nMinGlyphPos; i < nEndGlyphPos; i++)
        {
            assert(mrWinFontEntry.GlyphIsCached(mpOutGlyphs[i]));

            const OpenGLGlyphCacheChunk& rChunk = mrWinFontEntry.GetCachedGlyphChunkFor(mpOutGlyphs[i]);
            const int n = mpOutGlyphs[i] - rChunk.mnFirstGlyph;

            if (rChunk.mbVertical)
            {
                SalTwoRect a2Rects(rChunk.maLocation[n].Left(), rChunk.maLocation[n].Top(),
                                   rChunk.maLocation[n].getWidth(), rChunk.maLocation[n].getHeight(),
                                   aPos.X(), nAdvance + aPos.Y(),
                                   rChunk.maLocation[n].getWidth(), rChunk.maLocation[n].getHeight()); // ???
                pImpl->DrawMask(*rChunk.mpTexture, salColor, a2Rects);
            }
            else
            {
                SalTwoRect a2Rects(rChunk.maLocation[n].Left(), rChunk.maLocation[n].Top(),
                                   rChunk.maLocation[n].getWidth(), rChunk.maLocation[n].getHeight(),
                                   nAdvance + aPos.X() + mpGlyphOffsets[i].du, aPos.Y() + mpGlyphOffsets[i].dv - rChunk.mnAscentPlusIntLeading,
                                   rChunk.maLocation[n].getWidth(), rChunk.maLocation[n].getHeight()); // ???
                pImpl->DrawMask(*rChunk.mpTexture, salColor, a2Rects);
            }
            nAdvance += mpGlyphAdvances[i];
        }
    }
    pImpl->PostDraw();

    return true;
}

DeviceCoordinate UniscribeLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
{
    // calculate width of the complete layout
    long nWidth = mnBaseAdv;
    for( int nItem = mnItemCount; --nItem >= 0; )
    {
        const VisualItem& rVI = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVI, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        // width = xoffset + width of last item
        nWidth = rVI.mnXOffset;
        const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
        for( int i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            nWidth += pGlyphWidths[i];
        break;
    }

    // copy the virtual char widths into pDXArray[]
    if( pDXArray )
        for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
            pDXArray[ i - mnMinCharPos ] = mpCharWidths[ i ];

    return nWidth;
}

sal_Int32 UniscribeLayout::GetTextBreak( DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor ) const
{
    long nWidth = 0;
    for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
    {
        nWidth += mpCharWidths[ i ] * nFactor;

        // check if the nMaxWidth still fits the current sub-layout
        if( nWidth >= nMaxWidth )
        {
            // go back to cluster start
            // we have to find the visual item first since the mpLogClusters[]
            // needed to find the cluster start is relative to the visual item
            int nMinGlyphIndex = 0;
            for( int nItem = 0; nItem < mnItemCount; ++nItem )
            {
                const VisualItem& rVisualItem = mpVisualItems[ nItem ];
                nMinGlyphIndex = rVisualItem.mnMinGlyphPos;
                if( (i >= rVisualItem.mnMinCharPos)
                &&  (i < rVisualItem.mnEndCharPos) )
                    break;
            }
            // now go back to the matching cluster start
            do
            {
                int nGlyphPos = mpLogClusters[i] + nMinGlyphIndex;
                if( 0 != mpVisualAttrs[ nGlyphPos ].fClusterStart )
                    return i;
            } while( --i >= mnMinCharPos );

            // if the cluster starts before the start of the visual item
            // then set the visual breakpoint before this item
            return mnMinCharPos;
        }

        // the visual break also depends on the nCharExtra between the characters
        nWidth += nCharExtra;
    }

    // the whole layout did fit inside the nMaxWidth
    return -1;
}

void UniscribeLayout::GetCaretPositions( int nMaxIdx, long* pCaretXArray ) const
{
    int i;
    for( i = 0; i < nMaxIdx; ++i )
        pCaretXArray[ i ] = -1;
    long* const pGlyphPos = (long*)alloca( (mnGlyphCount+1) * sizeof(long) );
    for( i = 0; i <= mnGlyphCount; ++i )
        pGlyphPos[ i ] = -1;

    long nXPos = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.IsEmpty() )
            continue;

        if (mnLayoutFlags & SalLayoutFlags::ForFallback)
        {
            nXPos = rVisualItem.mnXOffset;
        }
        // get glyph positions
        // TODO: handle when rVisualItem's glyph range is only partially used
        for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
        {
            pGlyphPos[ i ] = nXPos;
            nXPos += mpGlyphAdvances[ i ];
        }
        // rightmost position of this visualitem
        pGlyphPos[ i ] = nXPos;

        // convert glyph positions to character positions
        i = rVisualItem.mnMinCharPos;
        if( i < mnMinCharPos )
            i = mnMinCharPos;
        for(; (i < rVisualItem.mnEndCharPos) && (i < mnEndCharPos); ++i )
        {
            int j = mpLogClusters[ i ] + rVisualItem.mnMinGlyphPos;
            int nCurrIdx = i * 2;
            if( !rVisualItem.IsRTL() )
            {
                // normal positions for LTR case
                pCaretXArray[ nCurrIdx ]   = pGlyphPos[ j ];
                pCaretXArray[ nCurrIdx+1 ] = pGlyphPos[ j+1 ];
            }
            else
            {
                // reverse positions for RTL case
                pCaretXArray[ nCurrIdx ]   = pGlyphPos[ j+1 ];
                pCaretXArray[ nCurrIdx+1 ] = pGlyphPos[ j ];
            }
        }
    }

    if (!(mnLayoutFlags & SalLayoutFlags::ForFallback))
    {
        nXPos = 0;
        // fixup unknown character positions to neighbor
        for( i = 0; i < nMaxIdx; ++i )
        {
            if( pCaretXArray[ i ] >= 0 )
                nXPos = pCaretXArray[ i ];
            else
                pCaretXArray[ i ] = nXPos;
        }
    }
}

void UniscribeLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
}

void UniscribeLayout::ApplyDXArray( const ImplLayoutArgs& rArgs )
{
    const long* pDXArray = rArgs.mpDXArray;

    // increase char widths in string range to desired values
    bool bModified = false;
    int nOldWidth = 0;
    DBG_ASSERT( mnUnitsPerPixel==1, "UniscribeLayout.mnUnitsPerPixel != 1" );
    int i,j;
    for( i = mnMinCharPos, j = 0; i < mnEndCharPos; ++i, ++j )
    {
        int nNewCharWidth = (pDXArray[j] - nOldWidth);
        // TODO: nNewCharWidth *= mnUnitsPerPixel;
        if( mpCharWidths[i] != nNewCharWidth )
        {
            mpCharWidths[i] = nNewCharWidth;
            bModified = true;
        }
        nOldWidth = pDXArray[j];
    }

    if( !bModified )
        return;

    // initialize justifications array
    mpJustifications = new int[ mnGlyphCapacity ];
    for( i = 0; i < mnGlyphCount; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    // apply new widths to script items
    long nXOffset = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // set the position of this visual item
        rVisualItem.mnXOffset = nXOffset;

        // ignore empty visual items
        if( rVisualItem.IsEmpty() )
        {
            for (i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; i++)
              nXOffset += mpCharWidths[i];
            continue;
        }
        // ignore irrelevant visual items
        if( (rVisualItem.mnMinCharPos >= mnEndCharPos)
         || (rVisualItem.mnEndCharPos <= mnMinCharPos) )
            continue;

        // if needed prepare special handling for arabic justification
        rVisualItem.mbHasKashidas = false;
        if( rVisualItem.IsRTL() )
        {
            for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
                if ( (1U << mpVisualAttrs[i].uJustification) & 0xFF82 )  //  any Arabic justification
                {                                                        //  excluding SCRIPT_JUSTIFY_NONE
                    // yes
                    rVisualItem.mbHasKashidas = true;
                    // so prepare for kashida handling
                    InitKashidaHandling();
                    break;
                }

            if( rVisualItem.HasKashidas() )
                for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
                {
                    // TODO: check if we still need this hack after correction of kashida placing?
                    // (i87688): apparently yes, we still need it!
                    if ( mpVisualAttrs[i].uJustification == SCRIPT_JUSTIFY_NONE )
                        // usp decided that justification can't be applied here
                        // but maybe our Kashida algorithm thinks differently.
                        // To avoid trouble (gaps within words, last character of
                        // a word gets a Kashida appended) override this.

                        // I chose SCRIPT_JUSTIFY_ARABIC_KASHIDA to replace SCRIPT_JUSTIFY_NONE
                        // just because this previous hack (which I haven't understand, sorry) used
                        // the same value to replace. Don't know if this is really the best
                        // thing to do, but it seems to fix things
                        mpVisualAttrs[i].uJustification = SCRIPT_JUSTIFY_ARABIC_KASHIDA;
                }
        }

        // convert virtual charwidths to glyph justification values
        HRESULT nRC = ScriptApplyLogicalWidth(
            mpCharWidths + rVisualItem.mnMinCharPos,
            rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos,
            rVisualItem.mnEndGlyphPos - rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            &rVisualItem.mpScriptItem->a,
            &rVisualItem.maABCWidths,
            mpJustifications + rVisualItem.mnMinGlyphPos );

        if( nRC != 0 )
        {
            delete[] mpJustifications;
            mpJustifications = NULL;
            break;
        }

        // to prepare for the next visual item
        // update nXOffset to the next items position
        // before the mpJustifications[] array gets modified
        int nMinGlyphPos, nEndGlyphPos;
        if( GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
        {
            for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
                nXOffset += mpJustifications[ i ];

            if( rVisualItem.mbHasKashidas )
                KashidaItemFix( nMinGlyphPos, nEndGlyphPos );
        }

        // workaround needed for older USP versions:
        // right align the justification-adjusted glyphs in their cells for RTL-items
        // unless the right alignment is done by inserting kashidas
        if( bManualCellAlign && rVisualItem.IsRTL() && !rVisualItem.HasKashidas() )
        {
            for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            {
                const int nXOffsetAdjust = mpJustifications[i] - mpGlyphAdvances[i];
                // #i99862# skip diacritics, we mustn't add extra justification to diacritics
                int nIdxAdd = i - 1;
                while( (nIdxAdd >= nMinGlyphPos) && !mpGlyphAdvances[nIdxAdd] )
                    --nIdxAdd;
                if( nIdxAdd < nMinGlyphPos )
                    rVisualItem.mnXOffset += nXOffsetAdjust;
                else
                    mpJustifications[nIdxAdd] += nXOffsetAdjust;
                mpJustifications[i] -= nXOffsetAdjust;
            }
        }
    }
}

void UniscribeLayout::InitKashidaHandling()
{
    if( mnMinKashidaGlyph != 0 )    // already initialized
        return;

    mrWinFontEntry.InitKashidaHandling( mhDC );
    mnMinKashidaWidth = static_cast<int>(mfFontScale * mrWinFontEntry.GetMinKashidaWidth());
    mnMinKashidaGlyph = mrWinFontEntry.GetMinKashidaGlyph();
}

// adjust the kashida placement matching to the WriterEngine
void UniscribeLayout::KashidaItemFix( int nMinGlyphPos, int nEndGlyphPos )
{
    // workaround needed for all known USP versions:
    // ApplyLogicalWidth does not match ScriptJustify behaviour
    for( int i = nMinGlyphPos; i < nEndGlyphPos; ++i )
    {
        // check for vowels
        if( (i > nMinGlyphPos && !mpGlyphAdvances[ i-1 ])
        &&  (1U << mpVisualAttrs[i].uJustification) & 0xFF83 )    // all Arabic justifiction types
        {                                                        // including SCRIPT_JUSTIFY_NONE
            // vowel, we do it like ScriptJustify does
            // the vowel gets the extra width
            long nSpaceAdded =  mpJustifications[ i ] - mpGlyphAdvances[ i ];
            mpJustifications [ i ] = mpGlyphAdvances [ i ];
            mpJustifications [ i - 1 ] += nSpaceAdded;
        }
    }

    // redistribute the widths for kashidas
    for( int i = nMinGlyphPos; i < nEndGlyphPos; )
        KashidaWordFix ( nMinGlyphPos, nEndGlyphPos, &i );
}

bool UniscribeLayout::KashidaWordFix ( int nMinGlyphPos, int nEndGlyphPos, int* pnCurrentPos )
{
    // doing pixel work within a word.
    // sometimes we have extra pixels and sometimes we miss some pixels to get to mnMinKashidaWidth

    // find the next kashida
    int nMinPos = *pnCurrentPos;
    int nMaxPos = *pnCurrentPos;
    for( int i = nMaxPos; i < nEndGlyphPos; ++i )
    {
        if( (mpVisualAttrs[ i ].uJustification >= SCRIPT_JUSTIFY_ARABIC_BLANK)
        &&  (mpVisualAttrs[ i ].uJustification < SCRIPT_JUSTIFY_ARABIC_NORMAL) )
            break;
        nMaxPos = i;
    }
    *pnCurrentPos = nMaxPos + 1;
    if( nMinPos == nMaxPos )
        return false;

    // calculate the available space for an extra kashida
    long nMaxAdded = 0;
    int nKashPos = -1;
    for( int i = nMaxPos; i >= nMinPos; --i )
    {
        long nSpaceAdded = mpJustifications[ i ] - mpGlyphAdvances[ i ];
        if( nSpaceAdded > nMaxAdded )
        {
            nKashPos = i;
            nMaxAdded = nSpaceAdded;
        }
    }

    // return early if there is no need for an extra kashida
    if ( nMaxAdded <= 0 )
        return false;
    // return early if there is not enough space for an extra kashida
    if( 2*nMaxAdded < mnMinKashidaWidth )
        return false;

    // redistribute the extra spacing to the kashida position
    for( int i = nMinPos; i <= nMaxPos; ++i )
    {
        if( i == nKashPos )
            continue;
        // everything else should not have extra spacing
        long nSpaceAdded = mpJustifications[ i ] - mpGlyphAdvances[ i ];
        if( nSpaceAdded > 0 )
        {
            mpJustifications[ i ] -= nSpaceAdded;
            mpJustifications[ nKashPos ] += nSpaceAdded;
        }
    }

    // check if we fulfill minimal kashida width
    long nSpaceAdded = mpJustifications[ nKashPos ] - mpGlyphAdvances[ nKashPos ];
    if( nSpaceAdded < mnMinKashidaWidth )
    {
        // ugly: steal some pixels
        long nSteal = 1;
        if ( nMaxPos - nMinPos > 0 && ((mnMinKashidaWidth - nSpaceAdded) > (nMaxPos - nMinPos)))
            nSteal = (mnMinKashidaWidth - nSpaceAdded) / (nMaxPos - nMinPos);
        for( int i = nMinPos; i <= nMaxPos; ++i )
        {
            if( i == nKashPos )
                continue;
            nSteal = std::min( mnMinKashidaWidth - nSpaceAdded, nSteal );
            if ( nSteal > 0 )
            {
                mpJustifications [ i ] -= nSteal;
                mpJustifications [ nKashPos ] += nSteal;
                nSpaceAdded += nSteal;
            }
            if( nSpaceAdded >= mnMinKashidaWidth )
                return true;
        }
    }

    // blank padding
    long nSpaceMissing = mnMinKashidaWidth - nSpaceAdded;
    if( nSpaceMissing > 0 )
    {
        // inner glyph: distribute extra space evenly
        if( (nMinPos > nMinGlyphPos) && (nMaxPos < nEndGlyphPos - 1) )
        {
            mpJustifications [ nKashPos ] += nSpaceMissing;
            long nHalfSpace = nSpaceMissing / 2;
            mpJustifications [ nMinPos - 1 ] -= nHalfSpace;
            mpJustifications [ nMaxPos + 1 ] -= nSpaceMissing - nHalfSpace;
        }
        // rightmost: left glyph gets extra space
        else if( nMinPos > nMinGlyphPos )
        {
            mpJustifications [ nMinPos - 1 ] -= nSpaceMissing;
            mpJustifications [ nKashPos ] += nSpaceMissing;
        }
        // leftmost: right glyph gets extra space
        else if( nMaxPos < nEndGlyphPos - 1 )
        {
            mpJustifications [ nKashPos ] += nSpaceMissing;
            mpJustifications [ nMaxPos + 1 ] -= nSpaceMissing;
        }
        else
            return false;
    }

    return true;
}

void UniscribeLayout::Justify( DeviceCoordinate nNewWidth )
{
    DeviceCoordinate nOldWidth = 0;
    int i;
    for( i = mnMinCharPos; i < mnEndCharPos; ++i )
        nOldWidth += mpCharWidths[ i ];
    if( nOldWidth <= 0 )
        return;

    nNewWidth *= mnUnitsPerPixel;    // convert into font units
    if( nNewWidth == nOldWidth )
        return;
    // prepare to distribute the extra width evenly among the visual items
    const double fStretch = (double)nNewWidth / nOldWidth;

    // initialize justifications array
    mpJustifications = new int[ mnGlyphCapacity ];
    for( i = 0; i < mnGlyphCapacity; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    // justify stretched script items
    long nXOffset = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.IsEmpty() )
            continue;

        if( (rVisualItem.mnMinCharPos < mnEndCharPos)
         && (rVisualItem.mnEndCharPos > mnMinCharPos) )
        {
            long nItemWidth = 0;
            for( i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; ++i )
                nItemWidth += mpCharWidths[ i ];
            nItemWidth = (int)((fStretch - 1.0) * nItemWidth + 0.5);

            ScriptJustify(
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
                rVisualItem.mnEndGlyphPos - rVisualItem.mnMinGlyphPos,
                nItemWidth,
                mnMinKashidaWidth,
                mpJustifications + rVisualItem.mnMinGlyphPos );

            rVisualItem.mnXOffset = nXOffset;
            nXOffset += nItemWidth;
        }
    }
}

bool UniscribeLayout::IsKashidaPosValid ( int nCharPos ) const
{
    // we have to find the visual item first since the mpLogClusters[]
    // needed to find the cluster start is relative to to the visual item
    int nMinGlyphIndex = -1;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( (nCharPos >= rVisualItem.mnMinCharPos)
        &&  (nCharPos < rVisualItem.mnEndCharPos) )
        {
            nMinGlyphIndex = rVisualItem.mnMinGlyphPos;
            break;
        }
    }
    // Invalid char pos or leftmost glyph in visual item
    if ( nMinGlyphIndex == -1 || !mpLogClusters[ nCharPos ] )
        return false;

//    This test didn't give the expected results
/*    if( mpLogClusters[ nCharPos+1 ] == mpLogClusters[ nCharPos ])
    // two chars, one glyph
        return false;*/

    const int nGlyphPos = mpLogClusters[ nCharPos ] + nMinGlyphIndex;
    if( nGlyphPos <= 0 )
        return true;
    // justification is only allowed if the glyph to the left has not SCRIPT_JUSTIFY_NONE
    // and not SCRIPT_JUSTIFY_ARABIC_BLANK
    // special case: glyph to the left is vowel (no advance width)
    if ( mpVisualAttrs[ nGlyphPos-1 ].uJustification == SCRIPT_JUSTIFY_ARABIC_BLANK
        || ( mpVisualAttrs[ nGlyphPos-1 ].uJustification == SCRIPT_JUSTIFY_NONE
            && mpGlyphAdvances [ nGlyphPos-1 ] ))
        return false;
    return true;
}

#if ENABLE_GRAPHITE

sal_GlyphId GraphiteLayoutWinImpl::getKashidaGlyph(int & rWidth)
{
    rWidth = mrFont.GetMinKashidaWidth();
    return mrFont.GetMinKashidaGlyph();
}

float gr_fontAdvance(const void* appFontHandle, gr_uint16 glyphId)
{
    HDC hDC = reinterpret_cast<HDC>(const_cast<void*>(appFontHandle));
    GLYPHMETRICS gm;
    const MAT2 mat2 = {{0,1}, {0,0}, {0,0}, {0,1}};
    if (GDI_ERROR == GetGlyphOutlineW(hDC, glyphId, GGO_GLYPH_INDEX | GGO_METRICS,
        &gm, 0, NULL, &mat2))
    {
        return .0f;
    }
    return gm.gmCellIncX;
}

GraphiteWinLayout::GraphiteWinLayout(HDC hDC, const ImplWinFontData& rWFD, ImplWinFontEntry& rWFE, bool bUseOpenGL) throw()
  : WinLayout(hDC, rWFD, rWFE, bUseOpenGL), mpFont(NULL),
    maImpl(rWFD.GraphiteFace(), rWFE)
{
    // the log font size may differ from the font entry size if scaling is used for large fonts
    LOGFONTW aLogFont;
    GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
    mpFont = gr_make_font_with_advance_fn(static_cast<float>(-aLogFont.lfHeight),
        hDC, gr_fontAdvance, rWFD.GraphiteFace());
    maImpl.SetFont(mpFont);
    const OString aLang = OUStringToOString( LanguageTag::convertToBcp47( rWFE.maFontSelData.meLanguage ),
            RTL_TEXTENCODING_ASCII_US);
    OString name = OUStringToOString(
        rWFE.maFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    sal_Int32 nFeat = name.indexOf(grutils::GrFeatureParser::FEAT_PREFIX) + 1;
    if (nFeat > 0)
    {
        OString aFeat = name.copy(nFeat, name.getLength() - nFeat);
        mpFeatures = new grutils::GrFeatureParser(rWFD.GraphiteFace(), aFeat.getStr(), aLang.getStr());
    }
    else
    {
        mpFeatures = new grutils::GrFeatureParser(rWFD.GraphiteFace(), aLang.getStr());
    }
    maImpl.SetFeatures(mpFeatures);
}

GraphiteWinLayout::~GraphiteWinLayout()
{
    delete mpFeatures;
    gr_font_destroy(maImpl.GetFont());
}

bool GraphiteWinLayout::LayoutText( ImplLayoutArgs & args)
{
    if (args.mnMinCharPos >= args.mnEndCharPos)
    {
        maImpl.clear();
        return true;
    }
    HFONT hUnRotatedFont = 0;
    if (args.mnOrientation)
    {
        // Graphite gets very confused if the font is rotated
        LOGFONTW aLogFont;
        GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
        aLogFont.lfEscapement = 0;
        aLogFont.lfOrientation = 0;
        hUnRotatedFont = CreateFontIndirectW( &aLogFont);
        SelectFont(mhDC, hUnRotatedFont);
    }
    WinLayout::AdjustLayout(args);
    maImpl.SetFontScale(WinLayout::mfFontScale);
    gr_segment * pSegment = maImpl.CreateSegment(args);
    bool bSucceeded = false;
    if (pSegment)
    {
        // replace the DC on the font within the segment
        // create glyph vectors
        bSucceeded = maImpl.LayoutGlyphs(args, pSegment);
        gr_seg_destroy(pSegment);
    }
    if (args.mnOrientation)
    {
        // restore the rotated font
        SelectFont(mhDC, mhFont);
        DeleteObject(hUnRotatedFont);
    }
    return bSucceeded;
}

void  GraphiteWinLayout::AdjustLayout(ImplLayoutArgs& rArgs)
{
    WinLayout::AdjustLayout(rArgs);
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;
    if ( (rArgs.mnFlags & SalLayoutFlags::BiDiRtl) && rArgs.mpDXArray)
    {
        mrWinFontEntry.InitKashidaHandling(mhDC);
    }
    maImpl.AdjustLayout(rArgs);
}

void GraphiteWinLayout::DrawTextImpl(HDC hDC) const
{
    HFONT hOrigFont = DisableFontScaling();
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;
    const int MAX_GLYPHS = 2;
    sal_GlyphId glyphIntStr[MAX_GLYPHS];
    WORD glyphWStr[MAX_GLYPHS];
    int glyphIndex = 0;
    Point aPos(0,0);
    int nGlyphs = 0;
    do
    {
        nGlyphs = maImpl.GetNextGlyphs(1, glyphIntStr, aPos, glyphIndex);
        if (nGlyphs < 1)
            break;
        std::copy(glyphIntStr, glyphIntStr + nGlyphs, glyphWStr);
        ExtTextOutW(hDC, aPos.X(), aPos.Y(), ETO_GLYPH_INDEX, NULL, (LPCWSTR)&(glyphWStr), nGlyphs, NULL);
    } while (nGlyphs);
    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));
}

bool GraphiteWinLayout::CacheGlyphs(SalGraphics& /*rGraphics*/) const
{
    return false;
}

bool GraphiteWinLayout::DrawCachedGlyphs(SalGraphics& /*rGraphics*/) const
{
    return false;
}

sal_Int32 GraphiteWinLayout::GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const
{
    sal_Int32 nBreak = maImpl.GetTextBreak(nMaxWidth, nCharExtra, nFactor);
    return nBreak;
}

DeviceCoordinate GraphiteWinLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
{
    return maImpl.FillDXArray(pDXArray);
}

void GraphiteWinLayout::GetCaretPositions( int nArraySize, long* pCaretXArray ) const
{
    maImpl.GetCaretPositions(nArraySize, pCaretXArray);
}

int GraphiteWinLayout::GetNextGlyphs( int length, sal_GlyphId* glyph_out,
                                      Point& pos_out, int& glyph_slot, DeviceCoordinate* glyph_adv, int* char_index,
                                      const PhysicalFontFace** pFallbackFonts ) const
{
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;
    return maImpl.GetNextGlyphs(length, glyph_out, pos_out, glyph_slot, glyph_adv, char_index, pFallbackFonts);
}

void GraphiteWinLayout::MoveGlyph( int glyph_idx, long new_x_pos )
{
    maImpl.MoveGlyph(glyph_idx, new_x_pos);
}

void GraphiteWinLayout::DropGlyph( int glyph_idx )
{
    maImpl.DropGlyph(glyph_idx);
}

void GraphiteWinLayout::Simplify( bool is_base )
{
    maImpl.Simplify(is_base);
}
#endif // ENABLE_GRAPHITE

SalLayout* WinSalGraphics::GetTextLayout( ImplLayoutArgs& /*rArgs*/, int nFallbackLevel )
{
    if (!mpWinFontEntry[nFallbackLevel]) return nullptr;

    assert(mpWinFontData[nFallbackLevel]);

    WinLayout* pWinLayout = NULL;

    const ImplWinFontData& rFontFace = *mpWinFontData[ nFallbackLevel ];
    ImplWinFontEntry& rFontInstance = *mpWinFontEntry[ nFallbackLevel ];

    bool bUseOpenGL = OpenGLHelper::isVCLOpenGLEnabled();

    if (!bUspInited)
        InitUSP();
#if ENABLE_GRAPHITE
    if (rFontFace.SupportsGraphite())
    {
        pWinLayout = new GraphiteWinLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
    }
    else
#endif // ENABLE_GRAPHITE
    {
        pWinLayout = new UniscribeLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
        // NOTE: it must be guaranteed that the WinSalGraphics lives longer than
        // the created UniscribeLayout, otherwise the data passed into the
        // constructor might become invalid too early
    }

    if( mfFontScale[nFallbackLevel] != 1.0 )
        pWinLayout->SetFontScale( mfFontScale[nFallbackLevel] );

    return pWinLayout;
}

int    WinSalGraphics::GetMinKashidaWidth()
{
    if( !mpWinFontEntry[0] )
        return 0;
    mpWinFontEntry[0]->InitKashidaHandling( getHDC() );
    int nMinKashida = static_cast<int>(mfFontScale[0] * mpWinFontEntry[0]->GetMinKashidaWidth());
    return nMinKashida;
}

ImplWinFontEntry::ImplWinFontEntry( FontSelectPattern& rFSD )
:   ImplFontEntry( rFSD )
,   maWidthMap( 512 )
,    mnMinKashidaWidth( -1 )
,    mnMinKashidaGlyph( -1 )
{
    maScriptCache = NULL;
}

ImplWinFontEntry::~ImplWinFontEntry()
{
    if( maScriptCache != NULL )
        ScriptFreeCache( &maScriptCache );
}

bool ImplWinFontEntry::InitKashidaHandling( HDC hDC )
{
    if( mnMinKashidaWidth >= 0 )    // already cached?
        return mnMinKashidaWidth;

    // initialize the kashida width
    mnMinKashidaWidth = 0;
    mnMinKashidaGlyph = 0;
    if (!bUspInited)
        InitUSP();

    SCRIPT_FONTPROPERTIES aFontProperties;
    aFontProperties.cBytes = sizeof (aFontProperties);
    SCRIPT_CACHE& rScriptCache = GetScriptCache();
    HRESULT nRC = ScriptGetFontProperties( hDC, &rScriptCache, &aFontProperties );
    if( nRC != 0 )
        return false;
    mnMinKashidaWidth = aFontProperties.iKashidaWidth;
    mnMinKashidaGlyph = aFontProperties.wgKashida;

    return true;
}

PhysicalFontFace* ImplWinFontData::Clone() const
{
#if ENABLE_GRAPHITE
    if ( mpGraphiteData )
        mpGraphiteData->AddReference();
#endif
    PhysicalFontFace* pClone = new ImplWinFontData( *this );
    return pClone;
}

ImplFontEntry* ImplWinFontData::CreateFontInstance( FontSelectPattern& rFSD ) const
{
    ImplFontEntry* pEntry = new ImplWinFontEntry( rFSD );
    return pEntry;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
