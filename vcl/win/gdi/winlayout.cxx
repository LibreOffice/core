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
#include "opengl/PackedTextureAtlas.hxx"

#include <vcl/opengl/OpenGLHelper.hxx>
#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <outdev.h>

#include "sft.hxx"
#include "sallayout.hxx"

#include <cstdio>
#include <cstdlib>

#include <sal/alloca.h>
#include <rtl/character.hxx>

#include <algorithm>

#include <shlwapi.h>
#include <winver.h>

#include <unordered_map>
#include <unordered_set>

typedef std::unordered_map<int,int> IntMap;

// Graphite headers
#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <i18nlangtag/languagetag.hxx>
#include <graphite_features.hxx>
#if ENABLE_GRAPHITE_DWRITE
#include <d2d1.h>
#include <dwrite.h>
#endif
#endif

// This needs to come after any includes for d2d1.h, otherwise we get lots of errors
#include "glyphy/demo.hxx"


#define DROPPED_OUTGLYPH 0xFFFF

namespace
{
// Extra space at the top and bottom of the glyph in total = tmHeight / GLYPH_SPACE_RATIO;
const int GLYPH_SPACE_RATIO = 8;
// Border size at the top of the glyph = tmHeight / GLYPH_OFFSET_RATIO;
const int GLYPH_OFFSET_RATIO = GLYPH_SPACE_RATIO * 2;
}

struct OpenGLGlyphDrawElement
{
    Rectangle maLocation;
    int maLeftOverhangs;
    OpenGLTexture maTexture;
    int mnBaselineOffset;
    int mnHeight;
    bool mbVertical;
    bool mbRealGlyphIndices;

    int getExtraSpace() const
    {
        return std::max(mnHeight / GLYPH_SPACE_RATIO, 4);
    }

    int getExtraOffset() const
    {
        return std::max(mnHeight / GLYPH_OFFSET_RATIO, 2);
    }
};

class GlyphCache;

struct GlobalGlyphCache
{
    GlobalGlyphCache()
        : maPackedTextureAtlas(2048, 2048)
    {}

    PackedTextureAtlasManager maPackedTextureAtlas;
    std::unordered_set<GlyphCache*> maGlyphCaches;
};

class GlyphCache
{
private:
    static std::unique_ptr<GlobalGlyphCache> gGlobalGlyphCache;
    std::unordered_map<int, OpenGLGlyphDrawElement> maOpenGLTextureCache;

public:
    GlyphCache()
    {
        gGlobalGlyphCache.get()->maGlyphCaches.insert(this);
    }

    ~GlyphCache()
    {
        gGlobalGlyphCache.get()->maGlyphCaches.erase(this);
    }

    bool ReserveTextureSpace(OpenGLGlyphDrawElement& rElement, int nWidth, int nHeight)
    {
        GlobalGlyphCache* pGlobalGlyphCache = gGlobalGlyphCache.get();
        rElement.maTexture = pGlobalGlyphCache->maPackedTextureAtlas.Reserve(nWidth, nHeight);
        if (!rElement.maTexture)
            return false;
        std::vector<GLuint> aTextureIDs = pGlobalGlyphCache->maPackedTextureAtlas.ReduceTextureNumber(8);
        if (!aTextureIDs.empty())
        {
            for (auto& pGlyphCache: pGlobalGlyphCache->maGlyphCaches)
            {
                pGlyphCache->RemoveTextures(aTextureIDs);
            }
        }
        return true;
    }

    void RemoveTextures(std::vector<GLuint>& rTextureIDs)
    {
        auto it = maOpenGLTextureCache.begin();

        while (it != maOpenGLTextureCache.end())
        {
            GLuint nTextureID = it->second.maTexture.Id();

            if (std::find(rTextureIDs.begin(), rTextureIDs.end(), nTextureID) != rTextureIDs.end())
            {
                it = maOpenGLTextureCache.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void PutDrawElementInCache(const OpenGLGlyphDrawElement& rElement, int nGlyphIndex)
    {
        assert(!IsGlyphCached(nGlyphIndex));
        maOpenGLTextureCache[nGlyphIndex] = OpenGLGlyphDrawElement(rElement);
    }

    OpenGLGlyphDrawElement& GetDrawElement(int nGlyphIndex)
    {
        assert(IsGlyphCached(nGlyphIndex));
        return maOpenGLTextureCache[nGlyphIndex];
    }

    bool IsGlyphCached(int nGlyphIndex) const
    {
        return maOpenGLTextureCache.find(nGlyphIndex) != maOpenGLTextureCache.end();
    }
};

// static initialization
std::unique_ptr<GlobalGlyphCache> GlyphCache::gGlobalGlyphCache(new GlobalGlyphCache);

// win32 specific physical font instance
class WinFontInstance : public LogicalFontInstance
{
public:
    explicit                WinFontInstance( FontSelectPattern& );
    virtual                 ~WinFontInstance();
    void                    setupGLyphy(HDC hDC);

private:
    // TODO: also add HFONT??? Watch out for issues with too many active fonts...

public:
    bool                    HasKernData() const;
    void                    SetKernData( int, const KERNINGPAIR* );
    int                     GetKerning( sal_Unicode, sal_Unicode ) const;

private:
    KERNINGPAIR*            mpKerningPairs;
    int                     mnKerningPairs;

public:
    SCRIPT_CACHE&           GetScriptCache() const
                            { return maScriptCache; }
private:
    mutable SCRIPT_CACHE    maScriptCache;

public:
    int                     GetCachedGlyphWidth( int nCharCode ) const;
    void                    CacheGlyphWidth( int nCharCode, int nCharWidth );

    bool                    InitKashidaHandling( HDC );
    int                     GetMinKashidaWidth() const { return mnMinKashidaWidth; }
    int                     GetMinKashidaGlyph() const { return mnMinKashidaGlyph; }

    static GLuint             mnGLyphyProgram;
    demo_atlas_t*             mpGLyphyAtlas;
    demo_font_t*              mpGLyphyFont;

private:
    GlyphCache maGlyphCache;
public:
    bool CacheGlyphToAtlas(bool bRealGlyphIndices, int nGlyphIndex, const WinLayout& rLayout, SalGraphics& rGraphics);

    GlyphCache& GetGlyphCache()
    {
        return maGlyphCache;
    }

private:
    IntMap                  maWidthMap;
    mutable int             mnMinKashidaWidth;
    mutable int             mnMinKashidaGlyph;
    bool                    mbGLyphySetupCalled;
};

GLuint WinFontInstance::mnGLyphyProgram = 0;

class TextOutRenderer
{
protected:
    explicit TextOutRenderer() = default;
    TextOutRenderer(const TextOutRenderer &) = delete;
    TextOutRenderer & operator = (const TextOutRenderer &) = delete;

public:
    static TextOutRenderer & get();

    virtual ~TextOutRenderer() = default;

    virtual bool operator ()(WinLayout const &rLayout, HDC hDC,
        const Rectangle* pRectToErase,
        Point* pPos, int* pGetNextGlypInfo) = 0;
};

class ExTextOutRenderer : public TextOutRenderer
{
    ExTextOutRenderer(const ExTextOutRenderer &) = delete;
    ExTextOutRenderer & operator = (const ExTextOutRenderer &) = delete;

public:
    explicit ExTextOutRenderer() = default;
    virtual ~ExTextOutRenderer() override = default;

    bool operator ()(WinLayout const &rLayout, HDC hDC,
        const Rectangle* pRectToErase,
        Point* pPos, int* pGetNextGlypInfo) override;
};

#if ENABLE_GRAPHITE_DWRITE

class D2DWriteTextOutRenderer : public TextOutRenderer
{
    typedef HRESULT(WINAPI *pD2D1CreateFactory_t)(D2D1_FACTORY_TYPE,
        REFIID, const D2D1_FACTORY_OPTIONS *, void **);

    typedef HRESULT(WINAPI *pDWriteCreateFactory_t)(DWRITE_FACTORY_TYPE,
        REFIID, IUnknown **);

    static HINSTANCE mmD2d1, mmDWrite;
    static pD2D1CreateFactory_t     D2D1CreateFactory;
    static pDWriteCreateFactory_t   DWriteCreateFactory;

public:
    static bool InitModules();

    explicit D2DWriteTextOutRenderer();
    virtual ~D2DWriteTextOutRenderer() override;

    bool operator ()(WinLayout const &rLayout, HDC hDC,
        const Rectangle* pRectToErase,
        Point* pPos, int* pGetNextGlypInfo) override;

    inline bool BindDC(HDC hDC, Rectangle const & rRect = Rectangle(0, 0, 0, 0)) {
        RECT const rc = { rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom() };
        return SUCCEEDED(mpRT->BindDC(hDC, &rc));
    }

    bool BindFont(HDC hDC) /*override*/;
    bool ReleaseFont() /*override*/;

    std::vector<Rectangle>  GetGlyphInkBoxes(uint16_t * pGid, uint16_t * pGidEnd) const /*override*/;
    ID2D1RenderTarget * GetRenderTarget() const { return mpRT; }
    IDWriteFontFace   * GetFontFace() const { return mpFontFace; }
    float               GetEmHeight() const { return mlfEmHeight; }

    inline HRESULT CreateRenderTarget() {
        if (mpRT) mpRT->Release(); mpRT = nullptr;
        return mpD2DFactory->CreateDCRenderTarget(&mRTProps, &mpRT);
    }

    inline bool Ready() const { return mpGdiInterop && mpRT; }

private:
    static void CleanupModules();

    // This is a singleton object disable copy ctor and assignemnt operator
    D2DWriteTextOutRenderer(const D2DWriteTextOutRenderer &) = delete;
    D2DWriteTextOutRenderer & operator = (const D2DWriteTextOutRenderer &) = delete;

    bool GetDWriteFaceFromHDC(HDC hDC, IDWriteFontFace ** ppFontFace, float * lfSize) const;
    bool GetDWriteInkBox(IDWriteFontFace & rFontFace, WinLayout const &rLayout, float const lfEmHeight, Rectangle &) const;
    bool DrawGlyphs(const Point & origin, uint16_t * pGid, uint16_t * pGidEnd,
        float * pAdvances, Point * pOffsets) /*override*/;

    ID2D1Factory        * mpD2DFactory;
    IDWriteFactory      * mpDWriteFactory;
    IDWriteGdiInterop   * mpGdiInterop;
    ID2D1DCRenderTarget * mpRT;
    const D2D1_RENDER_TARGET_PROPERTIES mRTProps;

    IDWriteFontFace * mpFontFace;
    float             mlfEmHeight;
    HDC               mhDC;
};

inline void WinFontInstance::CacheGlyphWidth( int nCharCode, int nCharWidth )
{
    maWidthMap[ nCharCode ] = nCharWidth;
}

inline int WinFontInstance::GetCachedGlyphWidth( int nCharCode ) const
{
    IntMap::const_iterator it = maWidthMap.find( nCharCode );
    if( it == maWidthMap.end() )
        return -1;
    return it->second;
}

bool WinFontInstance::CacheGlyphToAtlas(bool bRealGlyphIndices, int nGlyphIndex, const WinLayout& rLayout, SalGraphics& rGraphics)
{
    if (nGlyphIndex == DROPPED_OUTGLYPH)
        return true;

    OpenGLGlyphDrawElement aElement;
    aElement.mbRealGlyphIndices = bRealGlyphIndices;

    std::vector<uint32_t> aCodePointsOrGlyphIndices(1);
    aCodePointsOrGlyphIndices[0] = nGlyphIndex;

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

    // For now we assume DWrite is present and we won't bother with fallback paths.
    D2DWriteTextOutRenderer * pTxt = dynamic_cast<D2DWriteTextOutRenderer *>(&TextOutRenderer::get());
    if (!pTxt)
        return false;

    if (!pTxt->BindFont(hDC))
    {
        SAL_WARN("vcl.gdi", "Binding of font failed. The font might not be supported by Direct Write.");
        DeleteDC(hDC);
        return false;
    }

    // Bail for non-horizontal text.
    {
        wchar_t sFaceName[200];
        int nFaceNameLen = GetTextFaceW(hDC, SAL_N_ELEMENTS(sFaceName), sFaceName);

        if (!nFaceNameLen)
            SAL_WARN("vcl.gdi", "GetTextFace failed: " << WindowsErrorString(GetLastError()));

        LOGFONTW aLogFont;
        GetObjectW(rLayout.mhFont, sizeof(LOGFONTW), &aLogFont);

        SelectObject(hDC, hOrigFont);
        DeleteDC(hDC);

        if (sFaceName[0] == '@' || aLogFont.lfOrientation != 0 || aLogFont.lfEscapement != 0)
        {
            pTxt->ReleaseFont();
            return false;
        }
    }
    std::vector<WORD> aGlyphIndices(1);
    // Fetch the ink boxes and calculate the size of the atlas.
    if (!bRealGlyphIndices)
    {
        if (!pTxt->GetFontFace())
        {
            SAL_WARN("vcl.gdi", "Font face is not available.");
            return false;
        }
        if (!SUCCEEDED(pTxt->GetFontFace()->GetGlyphIndices(aCodePointsOrGlyphIndices.data(), aCodePointsOrGlyphIndices.size(), aGlyphIndices.data())))
        {
            pTxt->ReleaseFont();
            return false;
        }
    }
    else
    {
        aGlyphIndices[0] = aCodePointsOrGlyphIndices[0];
    }

    Rectangle bounds(0, 0, 0, 0);
    auto aInkBoxes = pTxt->GetGlyphInkBoxes(aGlyphIndices.data(), aGlyphIndices.data() + 1);
    for (auto &box : aInkBoxes)
        bounds.Union(box + Point(bounds.Right(), 0));

    // bounds.Top() is the offset from the baseline at (0,0) to the top of the
    // inkbox.
    aElement.mnBaselineOffset = -bounds.Top();
    aElement.mnHeight = bounds.getHeight();
    aElement.mbVertical = false;

    // Try hard to avoid overlap as we want to be able to use
    // individual rectangles for each glyph. The ABC widths don't
    // take anti-aliasing into consideration. Let's hope that leaving
    // "extra" space between glyphs will help.
    std::vector<float> aGlyphAdv(1);   // offsets between glyphs
    std::vector<DWRITE_GLYPH_OFFSET> aGlyphOffset(1, DWRITE_GLYPH_OFFSET{0.0f, 0.0f});
    std::vector<int> aEnds(1); // end of each glyph box
    float totWidth = 0;
    {
        int overhang = aInkBoxes[0].Left();
        int blackWidth = aInkBoxes[0].getWidth(); // width of non-AA pixels
        aElement.maLeftOverhangs = overhang;

        aGlyphAdv[0] = blackWidth + aElement.getExtraSpace();
        aGlyphOffset[0].advanceOffset = -overhang;

        totWidth += aGlyphAdv[0];
        aEnds[0] = totWidth;
    }
    // Leave extra space also at top and bottom
    int nBitmapWidth = totWidth;
    int nBitmapHeight = bounds.getHeight() + aElement.getExtraSpace();

    UINT nPos = 0;

    // FIXME: really I don't get why 'vertical' makes any difference [!] what does it mean !?
    if (aElement.mbVertical)
    {
        aElement.maLocation.Left() = 0;
        aElement.maLocation.Right() = nBitmapWidth;
        aElement.maLocation.Top() = nPos;
        aElement.maLocation.Bottom() = nPos + aGlyphAdv[0] + aElement.maLeftOverhangs;
    }
    else
    {
        aElement.maLocation.Left() = nPos;
        aElement.maLocation.Right() = aEnds[0];
        aElement.maLocation.Top() = 0;
        aElement.maLocation.Bottom() = bounds.getHeight() + aElement.getExtraSpace();
    }
    nPos = aEnds[0];

    OpenGLCompatibleDC aDC(rGraphics, 0, 0, nBitmapWidth, nBitmapHeight);

    HFONT hNonAntialiasedFont = NULL;

    SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
    SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

    aDC.fill(MAKE_SALCOLOR(0xff, 0xff, 0xff));

    pTxt->BindDC(aDC.getCompatibleHDC(), Rectangle(0, 0, nBitmapWidth, nBitmapHeight));
    auto pRT = pTxt->GetRenderTarget();

    ID2D1SolidColorBrush* pBrush = nullptr;
    if (!SUCCEEDED(pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBrush)))
    {
        pTxt->ReleaseFont();
        return false;
    }

    D2D1_POINT_2F baseline = {
        aElement.getExtraOffset(),
        aElement.getExtraOffset() + aElement.mnBaselineOffset
    };

    DWRITE_GLYPH_RUN glyphs = {
        pTxt->GetFontFace(),
        pTxt->GetEmHeight(),
        1,
        aGlyphIndices.data(),
        aGlyphAdv.data(),
        aGlyphOffset.data(),
        false,
        0
    };

    pRT->BeginDraw();
    pRT->DrawGlyphRun(baseline, &glyphs, pBrush);
    HRESULT hResult = pRT->EndDraw();

    pBrush->Release();

    switch (hResult)
    {
    case S_OK:
        break;
    case D2DERR_RECREATE_TARGET:
        pTxt->CreateRenderTarget();
        break;
    default:
        SAL_WARN("vcl.gdi", "DrawGlyphRun-EndDraw failed: " << WindowsErrorString(GetLastError()));
        SelectFont(aDC.getCompatibleHDC(), hOrigFont);
        if (hNonAntialiasedFont != NULL)
            DeleteObject(hNonAntialiasedFont);
        return false;
    }

    pTxt->ReleaseFont();

    if (!maGlyphCache.ReserveTextureSpace(aElement, nBitmapWidth, nBitmapHeight))
        return false;
    if (!aDC.copyToTexture(aElement.maTexture))
        return false;

    maGlyphCache.PutDrawElementInCache(aElement, nGlyphIndex);

    SelectFont(aDC.getCompatibleHDC(), hOrigFont);
    if (hNonAntialiasedFont != NULL)
        DeleteObject(hNonAntialiasedFont);

    return true;
}

SimpleWinLayout::SimpleWinLayout(HDC hDC, const WinFontFace& rWinFontData,
        WinFontInstance& rWinFontEntry, bool bUseOpenGL)
:   WinLayout(hDC, rWinFontData, rWinFontEntry, bUseOpenGL),
    mnGlyphCount( 0 ),
    mnCharCount( 0 ),
    mpOutGlyphs( NULL ),
    mpGlyphAdvances( NULL ),
    mpGlyphOrigAdvs( NULL ),
    mpCharWidths( NULL ),
    mpChars2Glyphs( NULL ),
    mpGlyphs2Chars( NULL ),
    mpGlyphRTLFlags( NULL ),
    mnWidth( 0 ),
    mnNotdefWidth( -1 )
{
}

SimpleWinLayout::~SimpleWinLayout()
{
    delete[] mpGlyphRTLFlags;
    delete[] mpGlyphs2Chars;
    delete[] mpChars2Glyphs;
    if( mpCharWidths != mpGlyphAdvances )
        delete[] mpCharWidths;
    delete[] mpGlyphOrigAdvs;
    delete[] mpGlyphAdvances;
    delete[] mpOutGlyphs;
}

bool SimpleWinLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    // prepare layout
    // TODO: fix case when recyclying old SimpleWinLayout object
    mnCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;

    // TODO: use a cached value for bDisableAsianKern from upper layers
    if( rArgs.mnFlags & SalLayoutFlags::KerningAsian )
    {
        TEXTMETRICA aTextMetricA;
        if( GetTextMetricsA( mhDC, &aTextMetricA )
        && !(aTextMetricA.tmPitchAndFamily & TMPF_FIXED_PITCH) && !(aTextMetricA.tmCharSet == 0x86) )
            rArgs.mnFlags &= ~SalLayoutFlags::KerningAsian;
    }

    // layout text
    int i, j;

    mnGlyphCount = 0;
    bool bVertical(rArgs.mnFlags & SalLayoutFlags::Vertical);

    // count the number of chars to process if no RTL run
    rArgs.ResetPos();
    bool bHasRTL = false;
    while( rArgs.GetNextRun( &i, &j, &bHasRTL ) && !bHasRTL )
        mnGlyphCount += j - i;

    // if there are RTL runs we need room to remember individual BiDi flags
    if( bHasRTL )
    {
        mpGlyphRTLFlags = new bool[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpGlyphRTLFlags[i] = false;
    }

    // rewrite the logical string if needed to prepare for the API calls
    const sal_Unicode* pBidiStr = rArgs.mrStr.pData->buffer + rArgs.mnMinCharPos;
    if( (mnGlyphCount != mnCharCount) || bVertical )
    {
        // we need to rewrite the pBidiStr when any of
        // - BiDirectional layout
        // - vertical layout
        // - partial runs (e.g. with control chars or for glyph fallback)
        // are involved
        sal_Unicode* pRewrittenStr = (sal_Unicode*)alloca( mnCharCount * sizeof(sal_Unicode) );
        pBidiStr = pRewrittenStr;

        // note: glyph to char mapping is relative to first character
        mpChars2Glyphs = new int[ mnCharCount ];
        mpGlyphs2Chars = new int[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpChars2Glyphs[i] = mpGlyphs2Chars[i] = -1;

        mnGlyphCount = 0;
        rArgs.ResetPos();
        bool bIsRTL = false;
        while( rArgs.GetNextRun( &i, &j, &bIsRTL ) )
        {
            do
            {
                // get the next leftmost character in this run
                int nCharPos = bIsRTL ? --j : i++;
                sal_UCS4 cChar = rArgs.mrStr[ nCharPos ];

                // in the RTL case mirror the character and remember its RTL status
                if( bIsRTL )
                {
                    cChar = GetMirroredChar( cChar );
                    mpGlyphRTLFlags[ mnGlyphCount ] = true;
                }

                // rewrite the original string
                // update the mappings between original and rewritten string
               // TODO: support surrogates in rewritten strings
                pRewrittenStr[ mnGlyphCount ] = static_cast<sal_Unicode>(cChar);
                mpGlyphs2Chars[ mnGlyphCount ] = nCharPos;
                mpChars2Glyphs[ nCharPos - rArgs.mnMinCharPos ] = mnGlyphCount;
                ++mnGlyphCount;
            } while( i < j );
        }
    }

    mpOutGlyphs     = new WCHAR[ mnGlyphCount ];
    mpGlyphAdvances = new int[ mnGlyphCount ];

    if( rArgs.mnFlags & (SalLayoutFlags::KerningPairs | SalLayoutFlags::KerningAsian) )
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];

    for( i = 0; i < mnGlyphCount; ++i )
        mpOutGlyphs[i] = pBidiStr[ i ];
    mnWidth = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        // get the current UCS-4 code point, check for surrogate pairs
        const WCHAR* pCodes = reinterpret_cast<LPCWSTR>(&pBidiStr[i]);
        unsigned nCharCode = pCodes[0];
        bool bSurrogate = ((nCharCode >= 0xD800) && (nCharCode <= 0xDFFF));
        if( bSurrogate )
        {
            // ignore high surrogates, they were already processed with their low surrogates
            if( nCharCode >= 0xDC00 )
                continue;
            // check the second half of the surrogate pair
            bSurrogate &= (0xDC00 <= pCodes[1]) && (pCodes[1] <= 0xDFFF);
            // calculate the UTF-32 code of valid surrogate pairs
            if( bSurrogate )
                nCharCode = 0x10000 + ((pCodes[0] - 0xD800) << 10) + (pCodes[1] - 0xDC00);
            else // or fall back to a replacement character
            {
                // FIXME: Surely this is an error situation that should not happen?
                nCharCode = '?';
            }
        }

        // get the advance width for the current UTF-32 code point
        int nGlyphWidth = mrWinFontEntry.GetCachedGlyphWidth( nCharCode );
        if( nGlyphWidth == -1 )
        {
            ABC aABC;
            SIZE aExtent;
            if( GetTextExtentPoint32W( mhDC, &pCodes[0], bSurrogate ? 2 : 1, &aExtent) )
                nGlyphWidth = aExtent.cx;
            else if( GetCharABCWidthsW( mhDC, nCharCode, nCharCode, &aABC ) )
                nGlyphWidth = aABC.abcA + aABC.abcB + aABC.abcC;
            else if( !GetCharWidth32W( mhDC, nCharCode, nCharCode, &nGlyphWidth )
                 &&  !GetCharWidthW( mhDC, nCharCode, nCharCode, &nGlyphWidth ) )
                    nGlyphWidth = 0;
            mrWinFontEntry.CacheGlyphWidth( nCharCode, nGlyphWidth );
        }
        mpGlyphAdvances[ i ] = nGlyphWidth;
        mnWidth += nGlyphWidth;

        // the second half of surrogate pair gets a zero width
        if( bSurrogate && ((i+1) < mnGlyphCount) )
            mpGlyphAdvances[ i+1 ] = 0;

        // check with the font face if glyph fallback is needed
        if( mrWinFontData.HasChar( nCharCode ) )
            continue;

        // request glyph fallback at this position in the string
        bool bRTL = mpGlyphRTLFlags ? mpGlyphRTLFlags[i] : false;
        int nCharPos = mpGlyphs2Chars ? mpGlyphs2Chars[i]: i + rArgs.mnMinCharPos;
        rArgs.NeedFallback( nCharPos, bRTL );
        if( bSurrogate && ((nCharPos+1) < rArgs.mrStr.getLength()) )
            rArgs.NeedFallback( nCharPos+1, bRTL );

        // replace the current glyph shape with the NotDef glyph shape
        if( rArgs.mnFlags & SalLayoutFlags::ForFallback )
        {
            // when we already are layouting for glyph fallback
            // then a new unresolved glyph is not interesting
            mnNotdefWidth = 0;
            mpOutGlyphs[i] = DROPPED_OUTGLYPH;
        }
        else
        {
            if( mnNotdefWidth < 0 )
            {
                // get the width of the NotDef glyph
                SIZE aExtent;
                WCHAR cNotDef = rArgs.mrStr[ nCharPos ];
                mnNotdefWidth = 0;
                if( GetTextExtentPoint32W( mhDC, &cNotDef, 1, &aExtent) )
                    mnNotdefWidth = aExtent.cx;
            }
        }
        if( bSurrogate && ((i+1) < mnGlyphCount) )
            mpOutGlyphs[i+1] = DROPPED_OUTGLYPH;

        // adjust the current glyph width to the NotDef glyph width
        mnWidth += mnNotdefWidth - mpGlyphAdvances[i];
        mpGlyphAdvances[i] = mnNotdefWidth;
        if( mpGlyphOrigAdvs )
            mpGlyphOrigAdvs[i] = mnNotdefWidth;
    }

    // apply kerning if the layout engine has not yet done it
    if( rArgs.mnFlags & (SalLayoutFlags::KerningAsian|SalLayoutFlags::KerningPairs) )
    {
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphOrigAdvs[i] = mpGlyphAdvances[i];

        // #99658# also apply asian kerning on the substring border
        int nLen = mnGlyphCount;
        if( rArgs.mnMinCharPos + nLen < rArgs.mrStr.getLength() )
            ++nLen;
        for( i = 1; i < nLen; ++i )
        {
            if( rArgs.mnFlags & SalLayoutFlags::KerningPairs )
            {
                int nKernAmount = mrWinFontEntry.GetKerning( pBidiStr[i-1], pBidiStr[i] );
                mpGlyphAdvances[ i-1 ] += nKernAmount;
                mnWidth += nKernAmount;
            }
            else if( rArgs.mnFlags & SalLayoutFlags::KerningAsian )

            if( ( (0x3000 == (0xFF00 & pBidiStr[i-1])) || (0x2010 == (0xFFF0 & pBidiStr[i-1])) || (0xFF00 == (0xFF00 & pBidiStr[i-1])))
            &&  ( (0x3000 == (0xFF00 & pBidiStr[i])) || (0x2010 == (0xFFF0 & pBidiStr[i])) || (0xFF00 == (0xFF00 & pBidiStr[i])) ) )
            {
                long nKernFirst = +CalcAsianKerning( pBidiStr[i-1], true, bVertical );
                long nKernNext  = -CalcAsianKerning( pBidiStr[i], false, bVertical );

                long nDelta = (nKernFirst < nKernNext) ? nKernFirst : nKernNext;
                if( nDelta<0 && nKernFirst!=0 && nKernNext!=0 )
                {
                    nDelta = (nDelta * mpGlyphAdvances[i-1] + 2) / 4;
                    mpGlyphAdvances[i-1] += nDelta;
                    mnWidth += nDelta;
                }
            }
        }
    }

    // calculate virtual char widths
    if( !mpGlyphs2Chars )
        mpCharWidths = mpGlyphAdvances;
    else
    {
        mpCharWidths = new int[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpCharWidths[ i ] = 0;
        for( i = 0; i < mnGlyphCount; ++i )
        {
            int k = mpGlyphs2Chars[ i ] - rArgs.mnMinCharPos;
            if( k >= 0 )
                mpCharWidths[ k ] += mpGlyphAdvances[ i ];
        }
    }

    // scale layout metrics if needed
    // TODO: does it make the code more simple if the metric scaling
    // is moved to the methods that need metric scaling (e.g. FillDXArray())?
    if( mfFontScale != 1.0 )
    {
        mnWidth   = (long)(mnWidth * mfFontScale);
        mnBaseAdv = (int)(mnBaseAdv * mfFontScale);
        for( i = 0; i < mnCharCount; ++i )
            mpCharWidths[i] = (int)(mpCharWidths[i] * mfFontScale);
        if( mpGlyphAdvances != mpCharWidths )
            for( i = 0; i < mnGlyphCount; ++i )
                mpGlyphAdvances[i] = (int)(mpGlyphAdvances[i] * mfFontScale);
        if( mpGlyphOrigAdvs && (mpGlyphOrigAdvs != mpGlyphAdvances) )
            for( i = 0; i < mnGlyphCount; ++i )
                mpGlyphOrigAdvs[i] = (int)(mpGlyphOrigAdvs[i] * mfFontScale);
    }

    return true;
}

int SimpleWinLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIds, Point& rPos, int& nStart,
                                    DeviceCoordinate* pGlyphAdvances, int* pCharIndexes,
                                    const PhysicalFontFace** /*pFallbackFonts*/ ) const
{
    // return zero if no more glyph found
    if( nStart >= mnGlyphCount )
        return 0;

    // calculate glyph position relative to layout base
    // TODO: avoid for nStart!=0 case by reusing rPos
    long nXOffset = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXOffset += mpGlyphAdvances[ i ];

    // calculate absolute position in pixel units
    Point aRelativePos( nXOffset, 0 );
    rPos = GetDrawPosition( aRelativePos );

    int nCount = 0;
    while( nCount < nLen )
    {
        // update return values {aGlyphId,nCharPos,nGlyphAdvance}
        sal_GlyphId aGlyphId = mpOutGlyphs[ nStart ];
        if( mnLayoutFlags & SalLayoutFlags::Vertical )
        {
            const sal_UCS4 cChar = static_cast<sal_UCS4>(aGlyphId & GF_IDXMASK);
            if( mrWinFontData.HasGSUBstitutions( mhDC )
            &&  mrWinFontData.IsGSUBstituted( cChar ) )
                aGlyphId |= GF_GSUB | GF_ROTL;
            else
            {
                aGlyphId |= GetVerticalFlags( cChar );
                if( (aGlyphId & GF_ROTMASK) == 0 )
                    aGlyphId |= GF_VERT;
            }
        }
        aGlyphId |= GF_ISCHAR;

        ++nCount;
        *(pGlyphIds++) = aGlyphId;
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = mpGlyphAdvances[ nStart ];
        if( pCharIndexes )
        {
            int nCharPos;
            if( !mpGlyphs2Chars )
                nCharPos = nStart + mnMinCharPos;
            else
                nCharPos = mpGlyphs2Chars[nStart];
            *(pCharIndexes++) = nCharPos;
        }

        // stop at last glyph
        if( ++nStart >= mnGlyphCount )
            break;

        // stop when next x-position is unexpected
        if( !pGlyphAdvances && mpGlyphOrigAdvs )
            if( mpGlyphAdvances[nStart-1] != mpGlyphOrigAdvs[nStart-1] )
                break;
    }

    return nCount;
}

bool SimpleWinLayout::DrawTextImpl(HDC hDC,
                                   const Rectangle* /* pRectToErase */,
                                   Point* /* pPos */,
                                   int* /* pGetNextGlypInfo */) const
{
    if( mnGlyphCount <= 0 )
        return false;

    HFONT hOrigFont = DisableFontScaling();
    Point aPos = GetDrawPosition( Point( mnBaseAdv, 0 ) );

    // #108267#, break up into glyph portions of a limited size required by Win32 API
    const unsigned int maxGlyphCount = 8192;
    UINT numGlyphPortions = mnGlyphCount / maxGlyphCount;
    UINT remainingGlyphs = mnGlyphCount % maxGlyphCount;

    if( numGlyphPortions )
    {
        // #108267#,#109387# break up string into smaller chunks
        // the output positions will be updated by windows (SetTextAlign)
        POINT oldPos;
        UINT oldTa = GetTextAlign(hDC);
        SetTextAlign(hDC, (oldTa & ~TA_NOUPDATECP) | TA_UPDATECP);
        MoveToEx(hDC, aPos.X(), aPos.Y(), &oldPos);
        unsigned int i = 0;
        for( unsigned int n = 0; n < numGlyphPortions; ++n, i+=maxGlyphCount )
        {
            ExtTextOutW(hDC, 0, 0, 0, NULL, mpOutGlyphs+i, maxGlyphCount, mpGlyphAdvances+i);
        }
        ExtTextOutW(hDC, 0, 0, 0, NULL, mpOutGlyphs+i, remainingGlyphs, mpGlyphAdvances+i);
        MoveToEx(hDC, oldPos.x, oldPos.y, (LPPOINT) NULL);
        SetTextAlign(hDC, oldTa);
    }
    else
        ExtTextOutW(hDC, aPos.X(), aPos.Y(), 0, NULL, mpOutGlyphs, mnGlyphCount, mpGlyphAdvances);

    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));

    return false;
}

DeviceCoordinate SimpleWinLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
{
    if( !mnWidth )
    {
        mnWidth = mnBaseAdv;
        for( int i = 0; i < mnGlyphCount; ++i )
            mnWidth += mpGlyphAdvances[ i ];
    }

    if( pDXArray != NULL )
    {
        for( int i = 0; i < mnCharCount; ++i )
             pDXArray[ i ] = mpCharWidths[ i ];
    }

    return mnWidth;
}

sal_Int32 SimpleWinLayout::GetTextBreak( DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor ) const
// NOTE: the nFactor is used to prevent rounding errors for small nCharExtra values
{
    if( mnWidth )
        if( (mnWidth * nFactor + mnCharCount * nCharExtra) <= nMaxWidth )
            return -1;

    long nExtraWidth = mnBaseAdv * nFactor;
    for( int n = 0; n < mnCharCount; ++n )
    {
        // skip unused characters
        if( mpChars2Glyphs && (mpChars2Glyphs[n] < 0) )
            continue;
        // add char widths until max
        nExtraWidth += mpCharWidths[ n ] * nFactor;
        if( nExtraWidth > nMaxWidth )
            return (mnMinCharPos + n);
        nExtraWidth += nCharExtra;
    }

    return -1;
}

void SimpleWinLayout::GetCaretPositions( int nMaxIdx, long* pCaretXArray ) const
{
    long nXPos = mnBaseAdv;

    if( !mpGlyphs2Chars )
    {
        for( int i = 0; i < nMaxIdx; i += 2 )
        {
            pCaretXArray[ i ] = nXPos;
            nXPos += mpGlyphAdvances[ i>>1 ];
            pCaretXArray[ i+1 ] = nXPos;
        }
    }
    else
    {
        int  i;
        for( i = 0; i < nMaxIdx; ++i )
            pCaretXArray[ i ] = -1;

        // assign glyph positions to character positions
        for( i = 0; i < mnGlyphCount; ++i )
        {
            int nCurrIdx = mpGlyphs2Chars[ i ] - mnMinCharPos;
            long nXRight = nXPos + mpCharWidths[ nCurrIdx ];
            nCurrIdx *= 2;
            if( !(mpGlyphRTLFlags && mpGlyphRTLFlags[i]) )
            {
                // normal positions for LTR case
                pCaretXArray[ nCurrIdx ]   = nXPos;
                pCaretXArray[ nCurrIdx+1 ] = nXRight;
            }
            else
            {
                // reverse positions for RTL case
                pCaretXArray[ nCurrIdx ]   = nXRight;
                pCaretXArray[ nCurrIdx+1 ] = nXPos;
            }
            nXPos += mpGlyphAdvances[ i ];
        }
    }
}

void SimpleWinLayout::Justify( DeviceCoordinate nNewWidth )
{
    DeviceCoordinate nOldWidth = mnWidth;
    mnWidth = nNewWidth;

    if( mnGlyphCount <= 0 )
        return;

    if( nNewWidth == nOldWidth )
        return;

    // the rightmost glyph cannot be stretched
    const int nRight = mnGlyphCount - 1;
    nOldWidth -= mpGlyphAdvances[ nRight ];
    nNewWidth -= mpGlyphAdvances[ nRight ];

    // count stretchable glyphs
    int nStretchable = 0, i;
    for( i = 0; i < nRight; ++i )
        if( mpGlyphAdvances[i] >= 0 )
            ++nStretchable;

    // stretch these glyphs
    DeviceCoordinate nDiffWidth = nNewWidth - nOldWidth;
    for( i = 0; (i < nRight) && (nStretchable > 0); ++i )
    {
        if( mpGlyphAdvances[i] <= 0 )
            continue;
        DeviceCoordinate nDeltaWidth = nDiffWidth / nStretchable;
        mpGlyphAdvances[i] += nDeltaWidth;
        --nStretchable;
        nDiffWidth -= nDeltaWidth;
    }
}

void SimpleWinLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
    else
        return;

    // recalculate virtual char widths if they were changed
    if( mpCharWidths != mpGlyphAdvances )
    {
        int i;
        if( !mpGlyphs2Chars )
        {
            // standard LTR case
            for( i = 0; i < mnGlyphCount; ++i )
                 mpCharWidths[ i ] = mpGlyphAdvances[ i ];
        }
        else
        {
            // BiDi or complex case
            for( i = 0; i < mnCharCount; ++i )
                mpCharWidths[ i ] = 0;
            for( i = 0; i < mnGlyphCount; ++i )
            {
                int j = mpGlyphs2Chars[ i ] - rArgs.mnMinCharPos;
                if( j >= 0 )
                    mpCharWidths[ j ] += mpGlyphAdvances[ i ];
            }
        }
    }
}

void SimpleWinLayout::ApplyDXArray( const ImplLayoutArgs& rArgs )
{
    // try to avoid disturbance of text flow for LSB rounding case;
    const long* pDXArray = rArgs.mpDXArray;

    int i = 0;
    long nOldWidth = mnBaseAdv;
    for(; i < mnCharCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
        {
            nOldWidth += mpGlyphAdvances[ j ];
            long nDiff = nOldWidth - pDXArray[ i ];

            // disabled because of #104768#
            // works great for static text, but problems when typing
            // if( nDiff>+1 || nDiff<-1 )
            // only bother with changing anything when something moved
            if( nDiff != 0 )
                break;
        }
    }
    if( i >= mnCharCount )
        return;

    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphOrigAdvs[ i ] = mpGlyphAdvances[ i ];
    }

    mnWidth = mnBaseAdv;
    for( i = 0; i < mnCharCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
            mpGlyphAdvances[j] = pDXArray[i] - mnWidth;
        mnWidth = pDXArray[i];
    }
}

void SimpleWinLayout::MoveGlyph( int nStart, long nNewXPos )
{
   if( nStart > mnGlyphCount )
        return;

    // calculate the current x-position of the requested glyph
    // TODO: cache absolute positions
    int nXPos = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXPos += mpGlyphAdvances[i];

    // calculate the difference to the current glyph position
    int nDelta = nNewXPos - nXPos;

    // adjust the width of the layout if it was already cached
    if( mnWidth )
        mnWidth += nDelta;

    // depending on whether the requested glyph is leftmost in the layout
    // adjust either the layout's or the requested glyph's relative position
    if( nStart > 0 )
        mpGlyphAdvances[ nStart-1 ] += nDelta;
    else
        mnBaseAdv += nDelta;
}

void SimpleWinLayout::DropGlyph( int nStart )
{
    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}

void SimpleWinLayout::Simplify( bool /*bIsBase*/ )
{
    // return early if no glyph has been dropped
    int i = mnGlyphCount;
    while( (--i >= 0) && (mpOutGlyphs[ i ] != DROPPED_OUTGLYPH) );
    if( i < 0 )
        return;

    // convert the layout to a sparse layout if it is not already
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCount ];
        mpCharWidths = new int[ mnCharCount ];
        // assertion: mnGlyphCount == mnCharCount
        for( int k = 0; k < mnGlyphCount; ++k )
        {
            mpGlyphs2Chars[ k ] = mnMinCharPos + k;
            mpCharWidths[ k ] = mpGlyphAdvances[ k ];
        }
    }

    // remove dropped glyphs that are rightmost in the layout
    for( i = mnGlyphCount; --i >= 0; )
    {
        if( mpOutGlyphs[ i ] != DROPPED_OUTGLYPH )
            break;
        if( mnWidth )
            mnWidth -= mpGlyphAdvances[ i ];
        int nRelCharPos = mpGlyphs2Chars[ i ] - mnMinCharPos;
        if( nRelCharPos >= 0 )
            mpCharWidths[ nRelCharPos ] = 0;
    }
    mnGlyphCount = i + 1;

    // keep original glyph widths around
    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( int k = 0; k < mnGlyphCount; ++k )
            mpGlyphOrigAdvs[ k ] = mpGlyphAdvances[ k ];
    }

    // remove dropped glyphs inside the layout
    int nNewGC = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
        {
            // adjust relative position to last valid glyph
            int nDroppedWidth = mpGlyphAdvances[ i ];
            mpGlyphAdvances[ i ] = 0;
            if( nNewGC > 0 )
                mpGlyphAdvances[ nNewGC-1 ] += nDroppedWidth;
            else
                mnBaseAdv += nDroppedWidth;

            // zero the virtual char width for the char that has a fallback
            int nRelCharPos = mpGlyphs2Chars[ i ] - mnMinCharPos;
            if( nRelCharPos >= 0 )
                mpCharWidths[ nRelCharPos ] = 0;
        }
        else
        {
            if( nNewGC != i )
            {
                // rearrange the glyph array to get rid of the dropped glyph
                mpOutGlyphs[ nNewGC ]     = mpOutGlyphs[ i ];
                mpGlyphAdvances[ nNewGC ] = mpGlyphAdvances[ i ];
                mpGlyphOrigAdvs[ nNewGC ] = mpGlyphOrigAdvs[ i ];
                mpGlyphs2Chars[ nNewGC ]  = mpGlyphs2Chars[ i ];
            }
            ++nNewGC;
        }
    }

    mnGlyphCount = nNewGC;
    if( mnGlyphCount <= 0 )
        mnWidth = mnBaseAdv = 0;
}

void WinFontInstance::setupGLyphy(HDC hDC)
{
    if (mbGLyphySetupCalled)
        return;

    mbGLyphySetupCalled = true;

    // First get the OUTLINETEXTMETRIC to find the font's em unit. Then, to get an unmodified (not
    // grid-fitted) glyph outline, create the font anew at that size. That is as the doc for
    // GetGlyphOutline() suggests.
    OUTLINETEXTMETRICW aOutlineTextMetric;
    if (!GetOutlineTextMetricsW (hDC, sizeof (OUTLINETEXTMETRICW), &aOutlineTextMetric))
    {
        SAL_WARN("vcl.gdi.opengl", "GetOutlineTextMetricsW failed: " << WindowsErrorString(GetLastError()));
        return;
    }

    HFONT hFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);
    LOGFONTW aLogFont;
    GetObjectW(hFont, sizeof(LOGFONTW), &aLogFont);

    HDC hNewDC = GetDC(NULL);
    if (hNewDC == NULL)
    {
        SAL_WARN("vcl.gdi.opengl", "GetDC failed: " << WindowsErrorString(GetLastError()));
        return;
    }

    hNewDC = CreateCompatibleDC(hNewDC);
    if (hNewDC == NULL)
    {
        SAL_WARN("vcl.gdi.opengl", "CreateCompatibleDC failed: " << WindowsErrorString(GetLastError()));
        return;
    }

    aLogFont.lfHeight = aOutlineTextMetric.otmEMSquare;
    hFont = CreateFontIndirectW(&aLogFont);
    if (hFont == NULL)
    {
        SAL_WARN("vcl.gdi.opengl", "CreateFontIndirectW failed: " << WindowsErrorString(GetLastError()));
        return;
    }
    if (SelectObject(hNewDC, hFont) == NULL)
    {
        SAL_WARN("vcl.gdi.opengl", "SelectObject failed: " << WindowsErrorString(GetLastError()));
        return;
    }

    if (mnGLyphyProgram == 0)
        mnGLyphyProgram = demo_shader_create_program();

    mpGLyphyAtlas = demo_atlas_create(2048, 1024, 64, 8);
    mpGLyphyFont = demo_font_create(hNewDC, mpGLyphyAtlas);
}

WinLayout::WinLayout(HDC hDC, const WinFontFace& rWFD, WinFontInstance& rWFE, bool bUseOpenGL)
:   mhDC( hDC ),
    mhFont( (HFONT)GetCurrentObject(hDC,OBJ_FONT) ),
    mnBaseAdv( 0 ),
    mfFontScale( 1.0 ),
    mbUseOpenGL(bUseOpenGL),
    mrWinFontData( rWFD ),
    mrWinFontEntry(rWFE)
{
    assert(mrWinFontEntry.mnRefCount > 0);
    // keep mrWinFontEntry alive
    mrWinFontEntry.mpFontCache->Acquire(&mrWinFontEntry);
}

WinLayout::~WinLayout()
{
    mrWinFontEntry.mpFontCache->Release(&mrWinFontEntry);
}

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
        Point aPos(0, 0);
        int nGetNextGlypInfo(0);
        bool bContinue = DrawTextImpl(hDC, NULL, &aPos, &nGetNextGlypInfo);
        assert(!bContinue);
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

        WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(rWinGraphics.mpImpl.get());

        if (pImpl)
        {
            pImpl->PreDraw();

            Point aPos(0, 0);
            int nGetNextGlypInfo(0);
            while (true)
            {
                OpenGLCompatibleDC aDC(rGraphics, aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight());

                // we are making changes to the DC, make sure we got a new one
                assert(aDC.getCompatibleHDC() != hDC);

                RECT aWinRect = { aRect.Left(), aRect.Top(), aRect.Left() + aRect.GetWidth(), aRect.Top() + aRect.GetHeight() };
                FillRect(aDC.getCompatibleHDC(), &aWinRect, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

                // setup the hidden DC with black color and white background, we will
                // use the result of the text drawing later as a mask only
                HFONT hOrigFont = SelectFont(aDC.getCompatibleHDC(), mhFont);

                SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
                SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

                UINT nTextAlign = GetTextAlign(hDC);
                SetTextAlign(aDC.getCompatibleHDC(), nTextAlign);

                COLORREF color = GetTextColor(hDC);
                SalColor salColor = MAKE_SALCOLOR(GetRValue(color), GetGValue(color), GetBValue(color));

                // the actual drawing
                bool bContinue = DrawTextImpl(aDC.getCompatibleHDC(), &aRect, &aPos, &nGetNextGlypInfo);

                std::unique_ptr<OpenGLTexture> xTexture(aDC.getTexture());
                if (xTexture)
                    pImpl->DrawMask(*xTexture, salColor, aDC.getTwoRect());

                SelectFont(aDC.getCompatibleHDC(), hOrigFont);

                if (!bContinue)
                    break;
            }
            pImpl->PostDraw();
        }

    }
}

bool SimpleWinLayout::CacheGlyphs(SalGraphics& rGraphics) const
{
    static bool bDoGlyphCaching = (std::getenv("SAL_DISABLE_GLYPH_CACHING") == NULL);

    if (!bDoGlyphCaching)
        return false;

    for (int i = 0; i < mnGlyphCount; i++)
    {
        int nCodePoint;
        if (i < mnGlyphCount-1 && rtl::isHighSurrogate(mpOutGlyphs[i]) && rtl::isLowSurrogate(mpOutGlyphs[i+1]))
        {
#if 1 // Don't remove the #else branch in case somebody wants to
      // continue trying to figure out why sequential non-BMP glyphs
      // get scribbled on top of each others if caching is used.
            return false;
#else
            nCodePoint = rtl::combineSurrogates(mpOutGlyphs[i], mpOutGlyphs[i+1]);
            i++;
#endif
        }
        else
        {
            nCodePoint = mpOutGlyphs[i];
        }

        if (!mrWinFontEntry.GetGlyphCache().IsGlyphCached(nCodePoint))
        {
            if (!mrWinFontEntry.CacheGlyphToAtlas(false, nCodePoint, *this, rGraphics))
                return false;
        }
    }

    return true;
}

bool SimpleWinLayout::DrawCachedGlyphs(SalGraphics& rGraphics) const
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

    HFONT hOrigFont = DisableFontScaling();
    Point aPos = GetDrawPosition( Point( mnBaseAdv, 0 ) );

    int nAdvance = 0;

    for (int i = 0; i < mnGlyphCount; i++)
    {
        if (mpOutGlyphs[i] == DROPPED_OUTGLYPH)
            continue;

        int nCodePoint;
        if (i < mnGlyphCount-1 && rtl::isHighSurrogate(mpOutGlyphs[i]) && rtl::isLowSurrogate(mpOutGlyphs[i+1]))
        {
            nCodePoint = rtl::combineSurrogates(mpOutGlyphs[i], mpOutGlyphs[i+1]);
            i++;
        }
        else
        {
            nCodePoint = mpOutGlyphs[i];
        }

        OpenGLGlyphDrawElement& rElement(mrWinFontEntry.GetGlyphCache().GetDrawElement(nCodePoint));
        OpenGLTexture& rTexture = rElement.maTexture;

        if (!rTexture)
            return false;

        SalTwoRect a2Rects(0, 0,
                           rTexture.GetWidth(), rTexture.GetHeight(),
                           nAdvance + aPos.X() - rElement.getExtraOffset() + rElement.maLeftOverhangs,
                           aPos.Y() - rElement.mnBaselineOffset - rElement.getExtraOffset(),
                           rTexture.GetWidth(), rTexture.GetHeight());

        pImpl->DeferredTextDraw(rTexture, salColor, a2Rects);

        nAdvance += mpGlyphAdvances[i];
    }

    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));

    return true;
}

struct VisualItem
{
public:
    SCRIPT_ITEM*    mpScriptItem;
    int             mnMinGlyphPos;
    int             mnEndGlyphPos;
    int             mnMinCharPos;
    int             mnEndCharPos;
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
#if _WIN32_WINNT < _WIN32_WINNT_VISTA
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
#endif
    {
        bManualCellAlign = false;
    }

    bUspInited = true;
}

UniscribeLayout::UniscribeLayout(HDC hDC, const WinFontFace& rWinFontData,
        WinFontInstance& rWinFontEntry, bool bUseOpenGL)
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
    mbDisableGlyphInjection( false ),
    mbUseGLyphy( false )
{
    static bool bUseGLyphy = std::getenv("SAL_USE_GLYPHY") != NULL;
    mbUseGLyphy = bUseGLyphy;
}

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

#if 0 // Don't remove -- useful for temporary SAL_ DEBUG when hacking on this

namespace {

template<typename IntegerType>
OUString IntegerArrayToString(IntegerType *pWords, int n)
{
    OUString result = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            result += ",";
        if (i > 0 && i % 10 == 0)
            result += OUString::number(i) + ":";
        result += OUString::number(pWords[i]);
    }
    result += "}";

    return result;
}

OUString GoffsetArrayToString(GOFFSET *pGoffsets, int n)
{
    OUString result = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            result += ",";
        if (i > 0 && i % 10 == 0)
            result += OUString::number(i) + ":";
        result += "(" + OUString::number(pGoffsets[i].du) + "," + OUString::number(pGoffsets[i].dv) + ")";
    }
    result += "}";

    return result;
}

OUString VisAttrArrayToString(SCRIPT_VISATTR *pVisAttrs, int n)
{
    static const OUString JUSTIFICATION_NAME[] = {
        "NONE",
        "ARABIC_BLANK",
        "CHARACTER",
        "RESERVED1",
        "BLANK",
        "RESERVED2",
        "RESERVED3",
        "ARABIC_NORMAL",
        "ARABIC_KASHIDA",
        "ARABIC_ALEF",
        "ARABIC_HA",
        "ARABIC_RA",
        "ARABIC_BA",
        "ARABIC_BARA",
        "ARABIC_SEEN",
        "ARABIC_SEEN_M"
    };

    OUString result = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            result += ",";
        if (i > 0 && i % 10 == 0)
            result += OUString::number(i) + ":";
        result += OUString("{") + JUSTIFICATION_NAME[pVisAttrs[i].uJustification] + (pVisAttrs[i].fClusterStart ? OUString(",ClusterStart") : OUString()) + (pVisAttrs[i].fDiacritic ? OUString(",Diacritic") : OUString()) + OUString(pVisAttrs[i].fZeroWidth ? OUString(",ZeroWidth") : OUString()) + OUString("}");
    }
    result += "}";

    return result;
}

} // anonymous namespace

#endif // 0

bool UniscribeLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    msTheString = rArgs.mrStr;

    // for a base layout only the context glyphs have to be dropped
    // => when the whole string is involved there is no extra context
    std::vector<int> aDropChars;
    if( rArgs.mnFlags & SalLayoutFlags::ForFallback )
    {
        // calculate superfluous context char positions
        aDropChars.push_back(0);
        aDropChars.push_back(rArgs.mrStr.getLength());
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
    // TODO: fix case when recycling old UniscribeLayout object
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
    SCRIPT_CONTROL aScriptControl;
    memset(&aScriptControl, 0, sizeof(aScriptControl));
    aScriptControl.uDefaultLanguage = nLangId;
    aScriptControl.fNeutralOverride = aScriptState.fOverrideDirection;
    aScriptControl.fContextDigits   = bool(rArgs.mnFlags & SalLayoutFlags::SubstituteDigits);
    aScriptControl.fMergeNeutralItems = true;

    // determine relevant substring and work only on it
    // when Bidi status is unknown we need to look at the whole string though
    mnSubStringMin = 0;
    const int nLength = rArgs.mrStr.getLength();
    const sal_Unicode *pStr = rArgs.mrStr.getStr();
    int nSubStringEnd = nLength;
    if( aScriptState.fOverrideDirection )
    {
        // TODO: limit substring to portion limits
        mnSubStringMin = rArgs.mnMinCharPos - 8;
        if( mnSubStringMin < 0 )
            mnSubStringMin = 0;
        nSubStringEnd = rArgs.mnEndCharPos + 8;
        if( nSubStringEnd > nLength )
            nSubStringEnd = nLength;

    }
    // now itemize the substring with its context
    for( int nItemCapacity = 16;; nItemCapacity *= 8 )
    {
        mpScriptItems = new SCRIPT_ITEM[ nItemCapacity ];
        HRESULT nRC = ScriptItemize(
            reinterpret_cast<LPCWSTR>(pStr + mnSubStringMin), nSubStringEnd - mnSubStringMin,
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

        // shortcut ignorable items
        if( (rArgs.mnEndCharPos <= rVisualItem.mnMinCharPos)
         || (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos) )
        {
            for( int j = rVisualItem.mnMinCharPos; j < rVisualItem.mnEndCharPos; ++j )
                mpLogClusters[j] = sal::static_int_cast<WORD>(~0U);
            if (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos)
            {   // fdo#47553 adjust "guessed" min (maybe up to -8 off) to
                // actual min so it can be used properly in GetNextGlyphs
                if (mnSubStringMin < rVisualItem.mnEndCharPos)
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
            reinterpret_cast<LPCWSTR>(pStr + rVisualItem.mnMinCharPos),
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
                reinterpret_cast<LPCWSTR>(pStr + rVisualItem.mnMinCharPos),
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
                                if( pStr[ c ] == 0x2060 )
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
                                if( pStr[ c ] == 0x2060 )
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
        auto it = aDropChars.cbegin();
        while( it != aDropChars.cend() )
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
        for( int i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[i] = -1;
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
                if( mpGlyphs2Chars[i] == -1 )
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
        if( (mnLayoutFlags & SalLayoutFlags::Vertical) &&
            nCharPos != -1 )
            aGlyphId |= GetVerticalFlags( msTheString[nCharPos] );
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
    assert(nStart <= mnGlyphCount);

    if( nStart > 0 )        // nStart>0 means absolute glyph pos + 1
        --nStart;
    else                    // nStart<=0 for first visible glyph
    {
        VisualItem* pVI = mpVisualItems;
        for( int i = mnItemCount, nDummy; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nStart, nDummy ) )
                break;
        assert(nStart <= mnGlyphCount);

        int j = pVI->mnMinGlyphPos;
        while (j < mnGlyphCount && mpOutGlyphs[j] == DROPPED_OUTGLYPH) j++;
        if (j == nStart)
        {
            pVI->mnXOffset += ((mpJustifications)? mpJustifications[nStart] : mpGlyphAdvances[nStart]);
        }
    }

    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}

void UniscribeLayout::Simplify( bool /*bIsBase*/ )
{
    int i;
    // if there are no dropped glyphs don't bother
    for( i = 0; i < mnGlyphCount; ++i )
        if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
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
            if( mpOutGlyphs[ j ] == DROPPED_OUTGLYPH )
                mpCharWidths[ i ] = 0;
        }

        // handle dropped glyphs at start of visual item
        int nMinGlyphPos, nEndGlyphPos, nOrigMinGlyphPos = rVI.mnMinGlyphPos;
        GetItemSubrange( rVI, nMinGlyphPos, nEndGlyphPos );
        i = nMinGlyphPos;
        while( (i < nEndGlyphPos) && (mpOutGlyphs[i] == DROPPED_OUTGLYPH) )
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
                mpOutGlyphs[ i ] = DROPPED_OUTGLYPH;
            rVI.mnMinGlyphPos = i = nOrigMinGlyphPos;
        }

        // handle dropped glyphs in the middle of visual item
        for(; i < nEndGlyphPos; ++i )
            if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
                break;
        int j = i;
        while( ++i < nEndGlyphPos )
        {
            if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
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

bool UniscribeLayout::DrawTextImpl(HDC hDC,
                                   const Rectangle* /* pRectToErase */,
                                   Point* /* pPos */,
                                   int* /* pGetNextGlypInfo */) const
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

    return false;
}

bool UniscribeLayout::CacheGlyphs(SalGraphics& rGraphics) const
{
    static bool bDoGlyphCaching = (std::getenv("SAL_DISABLE_GLYPH_CACHING") == NULL);

    if (!bDoGlyphCaching)
        return false;

    if (mbUseGLyphy)
    {
        if (!mrWinFontEntry.mxFontMetric->IsTrueTypeFont())
            return false;

        mrWinFontEntry.setupGLyphy(mhDC);

        for (int i = 0; i < mnGlyphCount; i++)
        {
            if (mpOutGlyphs[i] == DROPPED_OUTGLYPH)
                continue;

            glyph_info_t aGI;
            VCL_GL_INFO("Calling demo_font_lookup_glyph");
            demo_font_lookup_glyph( mrWinFontEntry.mpGLyphyFont, mpOutGlyphs[i], &aGI );
        }
    }
    else
    {
        for (int i = 0; i < mnGlyphCount; i++)
        {
            int nCodePoint = mpOutGlyphs[i];
            if (!mrWinFontEntry.GetGlyphCache().IsGlyphCached(nCodePoint))
            {
                if (!mrWinFontEntry.CacheGlyphToAtlas(true, nCodePoint, *this, rGraphics))
                    return false;
            }
        }
    }

    return true;
}

bool UniscribeLayout::DrawCachedGlyphsUsingGLyphy(SalGraphics& rGraphics) const
{
    WinSalGraphics& rWinGraphics = static_cast<WinSalGraphics&>(rGraphics);

    Rectangle aRect;
    GetBoundRect(rGraphics, aRect);

    WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(rWinGraphics.mpImpl.get());
    if (!pImpl)
        return false;

    pImpl->PreDraw();

    rGraphics.GetOpenGLContext()->UseNoProgram();

    glUseProgram( mrWinFontEntry.mnGLyphyProgram );
    CHECK_GL_ERROR();
    demo_atlas_set_uniforms( mrWinFontEntry.mpGLyphyAtlas );

    GLint nLoc;

    nLoc = glGetUniformLocation( mrWinFontEntry.mnGLyphyProgram, "u_debug" );
    CHECK_GL_ERROR();
    glUniform1f( nLoc, 0 );
    CHECK_GL_ERROR();

    nLoc = glGetUniformLocation( mrWinFontEntry.mnGLyphyProgram, "u_contrast" );
    CHECK_GL_ERROR();
    glUniform1f( nLoc, 1 );
    CHECK_GL_ERROR();

    nLoc = glGetUniformLocation( mrWinFontEntry.mnGLyphyProgram, "u_gamma_adjust" );
    CHECK_GL_ERROR();
    glUniform1f( nLoc, 1 );
    CHECK_GL_ERROR();

    nLoc = glGetUniformLocation( mrWinFontEntry.mnGLyphyProgram, "u_outline" );
    CHECK_GL_ERROR();
    glUniform1f( nLoc, false );
    CHECK_GL_ERROR();

    nLoc = glGetUniformLocation( mrWinFontEntry.mnGLyphyProgram, "u_outline_thickness" );
    CHECK_GL_ERROR();
    glUniform1f( nLoc, 1 );
    CHECK_GL_ERROR();

    nLoc = glGetUniformLocation( mrWinFontEntry.mnGLyphyProgram, "u_boldness" );
    CHECK_GL_ERROR();
    glUniform1f( nLoc, 0 );
    CHECK_GL_ERROR();

    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERROR();

#if 0
    // glyphy-demo sets sRGB on initially, and there it has a perhaps beneficial effect,
    // but doesn't help very much here, if at all
    GLboolean available = false;
    if ((glewIsSupported("GL_ARB_framebuffer_sRGB") || glewIsSupported("GL_EXT_framebuffer_sRGB")) &&
        (glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &available), available))
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
        CHECK_GL_ERROR();
    }
#endif

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

        // This has to be in sync with UniscribeLayout::FillDXArray(), so that
        // actual and reported glyph positions (used for e.g. cursor caret
        // positioning) match.
        const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;

        double font_size = mrWinFontEntry.maFontSelData.mfExactHeight; // ???

        // font_size = 1; // ???

        std::vector<glyph_vertex_t> vertices;
        for (int i = nMinGlyphPos; i < nEndGlyphPos; i++)
        {
            // Ignore dropped glyphs.
            if (mpOutGlyphs[i] == DROPPED_OUTGLYPH)
                continue;

            // Total crack
            glyphy_point_t pt;
            pt.x = nAdvance + aPos.X() + mpGlyphOffsets[i].du;
            pt.y = aPos.Y() + mpGlyphOffsets[i].dv;
            glyph_info_t gi;
            demo_font_lookup_glyph( mrWinFontEntry.mpGLyphyFont, mpOutGlyphs[i], &gi );
            demo_shader_add_glyph_vertices( pt, font_size, &gi, &vertices, NULL );

            nAdvance += pGlyphWidths[i];
        }

        GLfloat mat[16];
        m4LoadIdentity( mat );

        GLint viewport[4];
        glGetIntegerv( GL_VIEWPORT, viewport );
        CHECK_GL_ERROR();

        double width = viewport[2];
        double height = viewport[3];

#if 0
        // Based on demo_view_apply_transform(). I don't really understand it.

        // No scaling, no translation needed here

        // Perspective (but why would we want that in LO; it's needed in glyphy-demo because there
        // you can rotate the "sheet" the text is drawn on)
        {
            double d = std::max( width, height );
            double near = d / 4;
            double far = near + d;
            double factor = near / (2 * near + d);
            m4Frustum( mat, -width * factor, width * factor, -height * factor, height * factor, near, far );
            m4Translate( mat, 0, 0, -(near + d * 0.5) );
        }

        // No rotation here

        // Fix 'up'
        m4Scale( mat, 1, -1, 1 );

        // Foo
        // m4Scale( mat, 0.5, 0.5, 1 );

        // The "Center buffer" part in demo_view_display()
        m4Translate( mat, width/2, height/2, 0 );

#else
        // Crack that just happens to show something (even if not completely in correct location) in
        // some cases, but not all. I don't understand why.
        double scale = std::max( 2/width, 2/height );
        m4Scale( mat, scale, -scale, 1);
        m4Translate( mat, -width/2, -height/2, 0 );
#endif

        GLuint u_matViewProjection_loc = glGetUniformLocation( mrWinFontEntry.mnGLyphyProgram, "u_matViewProjection" );
        CHECK_GL_ERROR();
        glUniformMatrix4fv( u_matViewProjection_loc, 1, GL_FALSE, mat );
        CHECK_GL_ERROR();

        GLuint a_glyph_vertex_loc = glGetAttribLocation( mrWinFontEntry.mnGLyphyProgram, "a_glyph_vertex" );
        GLuint buf_name;
        glGenBuffers( 1, &buf_name );
        CHECK_GL_ERROR();
        glBindBuffer( GL_ARRAY_BUFFER, buf_name );
        CHECK_GL_ERROR();

        glBufferData( GL_ARRAY_BUFFER,  sizeof(glyph_vertex_t) * vertices.size(), (const char *) &vertices[0], GL_STATIC_DRAW );
        CHECK_GL_ERROR();
        glEnableVertexAttribArray( a_glyph_vertex_loc );
        CHECK_GL_ERROR();
        glVertexAttribPointer( a_glyph_vertex_loc, 4, GL_FLOAT, GL_FALSE, sizeof(glyph_vertex_t), 0 );
        CHECK_GL_ERROR();
        glDrawArrays( GL_TRIANGLES, 0, vertices.size() );
        CHECK_GL_ERROR();
        glDisableVertexAttribArray( a_glyph_vertex_loc );
        CHECK_GL_ERROR();
        glDeleteBuffers( 1, &buf_name );
        CHECK_GL_ERROR();
    }
    pImpl->PostDraw();

    return true;
}

bool UniscribeLayout::DrawCachedGlyphsUsingTextures(SalGraphics& rGraphics) const
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

        // This has to be in sync with UniscribeLayout::FillDXArray(), so that
        // actual and reported glyph positions (used for e.g. cursor caret
        // positioning) match.
        const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;

        for (int i = nMinGlyphPos; i < nEndGlyphPos; i++)
        {
            // Ignore dropped glyphs.
            if (mpOutGlyphs[i] == DROPPED_OUTGLYPH)
                continue;

            OpenGLGlyphDrawElement& rElement = mrWinFontEntry.GetGlyphCache().GetDrawElement(mpOutGlyphs[i]);
            OpenGLTexture& rTexture = rElement.maTexture;

            if (!rTexture)
                return false;

            if (rElement.mbVertical)
            {
                SalTwoRect a2Rects(0, 0,
                                   rTexture.GetWidth(), rTexture.GetHeight(),
                                   aPos.X() + rElement.maLeftOverhangs,
                                   nAdvance + aPos.Y(),
                                   rTexture.GetWidth(), rTexture.GetHeight());

                pImpl->DeferredTextDraw(rTexture, salColor, a2Rects);
            }
            else
            {
                SalTwoRect a2Rects(0, 0,
                                   rTexture.GetWidth(), rTexture.GetHeight(),
                                   nAdvance + aPos.X() + mpGlyphOffsets[i].du - rElement.getExtraOffset() + rElement.maLeftOverhangs,
                                   aPos.Y() + mpGlyphOffsets[i].dv - rElement.mnBaselineOffset - rElement.getExtraOffset(),
                                   rTexture.GetWidth(), rTexture.GetHeight());

                pImpl->DeferredTextDraw(rTexture, salColor, a2Rects);
            }

            nAdvance += pGlyphWidths[i];
        }
    }

    return true;
}

bool UniscribeLayout::DrawCachedGlyphs(SalGraphics& rGraphics) const
{
    if (mbUseGLyphy)
        return DrawCachedGlyphsUsingGLyphy(rGraphics);
    else
        return DrawCachedGlyphsUsingTextures( rGraphics );
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
    std::unique_ptr<long[]> const pGlyphPos(new long[mnGlyphCount + 1]);
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
            int nCurrIdx = (i - mnMinCharPos) * 2;
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

        // tdf#94897: Don't add extra justification to chars with diacritics when the diacritic is a
        // separate glyph, followed by blank, in LTR
        if( !rVisualItem.IsRTL() )
        {
            for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            {
                const int nXOffsetAdjust = mpJustifications[i] - mpGlyphAdvances[i];
                if( nXOffsetAdjust == 0 )
                    continue;
                int nIdxAdd = i + 1;
                while( (nIdxAdd < nEndGlyphPos) && mpVisualAttrs[nIdxAdd].fDiacritic )
                    ++nIdxAdd;
                if( nIdxAdd == i + 1 )
                    continue;
                if( nIdxAdd >= nEndGlyphPos || mpVisualAttrs[nIdxAdd].uJustification != SCRIPT_JUSTIFY_BLANK )
                    continue;
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
    // needed to find the cluster start is relative to the visual item
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

HINSTANCE D2DWriteTextOutRenderer::mmD2d1 = nullptr,
          D2DWriteTextOutRenderer::mmDWrite = nullptr;
D2DWriteTextOutRenderer::pD2D1CreateFactory_t D2DWriteTextOutRenderer::D2D1CreateFactory = nullptr;
D2DWriteTextOutRenderer::pDWriteCreateFactory_t D2DWriteTextOutRenderer::DWriteCreateFactory = nullptr;

bool D2DWriteTextOutRenderer::InitModules()
{
    mmD2d1 = LoadLibrary("D2d1.dll");
    mmDWrite = LoadLibrary("dwrite.dll");
    if (mmD2d1 && mmDWrite)
    {
        D2D1CreateFactory = pD2D1CreateFactory_t(GetProcAddress(mmD2d1, "D2D1CreateFactory"));
        DWriteCreateFactory = pDWriteCreateFactory_t(GetProcAddress(mmDWrite, "DWriteCreateFactory"));
    }

    if (!D2D1CreateFactory || !DWriteCreateFactory)
    {
        CleanupModules();
        return false;
    }

    return true;
}

void D2DWriteTextOutRenderer::CleanupModules()
{
    if (mmD2d1)
        FreeLibrary(mmD2d1);
    if (mmDWrite)
        FreeLibrary(mmDWrite);

    mmD2d1 = nullptr;
    mmDWrite = nullptr;
    D2D1CreateFactory = nullptr;
    DWriteCreateFactory = nullptr;
}
#endif // ENABLE_GRAPHITE_DWRITE

TextOutRenderer & TextOutRenderer::get()
{
#if ENABLE_GRAPHITE_DWRITE
    static std::unique_ptr<TextOutRenderer> _impl(D2DWriteTextOutRenderer::InitModules()
        ? static_cast<TextOutRenderer*>(new D2DWriteTextOutRenderer())
        : static_cast<TextOutRenderer*>(new ExTextOutRenderer()));
#else // ENABLE_GRAPHITE_DWRITE
    static std::unique_ptr<TextOutRenderer> _impl(static_cast<TextOutRenderer*>(new ExTextOutRenderer()));
#endif // ENABLE_GRAPHITE_DWRITE

    return *_impl;
}


bool ExTextOutRenderer::operator ()(WinLayout const &rLayout, HDC hDC,
    const Rectangle* pRectToErase,
    Point* pPos, int* pGetNextGlypInfo)
{
    const int MAX_GLYPHS = 2;
    sal_GlyphId glyphIntStr[MAX_GLYPHS];
    int nGlyphs = 0;
    WORD glyphWStr[MAX_GLYPHS];
    do
    {
        nGlyphs = rLayout.GetNextGlyphs(1, glyphIntStr, *pPos, *pGetNextGlypInfo);
        if (nGlyphs < 1)
            break;

        std::copy_n(glyphIntStr, nGlyphs, glyphWStr);
        ExtTextOutW(hDC, pPos->X(), pPos->Y(), ETO_GLYPH_INDEX, NULL, LPCWSTR(&glyphWStr), nGlyphs, NULL);
    } while (!pRectToErase);

    return (pRectToErase && nGlyphs >= 1);
}

#if ENABLE_GRAPHITE_DWRITE

D2DWriteTextOutRenderer::D2DWriteTextOutRenderer()
    : mpD2DFactory(nullptr),
    mpDWriteFactory(nullptr),
    mpGdiInterop(nullptr),
    mpRT(nullptr),
    mRTProps(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
    0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT)),
    mpFontFace(nullptr),
    mlfEmHeight(0.0f),
    mhDC(0)
{
    HRESULT hr = S_OK;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), NULL, reinterpret_cast<void **>(&mpD2DFactory));
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&mpDWriteFactory));
    if (SUCCEEDED(hr))
    {
        hr = mpDWriteFactory->GetGdiInterop(&mpGdiInterop);
        hr = CreateRenderTarget();
    }
}

D2DWriteTextOutRenderer::~D2DWriteTextOutRenderer()
{
    if (mpRT)
        mpRT->Release();
    if (mpGdiInterop)
        mpGdiInterop->Release();
    if (mpDWriteFactory)
        mpDWriteFactory->Release();
    if (mpD2DFactory)
        mpD2DFactory->Release();

    CleanupModules();
}

bool D2DWriteTextOutRenderer::operator ()(WinLayout const &rLayout, HDC hDC,
    const Rectangle* pRectToErase,
    Point* pPos, int* pGetNextGlypInfo)
{
    if (!Ready())
        return false;

    if (!BindFont(hDC))
    {
        // If for any reason we can't bind fallback to legacy APIs.
        return ExTextOutRenderer()(rLayout, hDC, pRectToErase, pPos, pGetNextGlypInfo);
    }

    Rectangle bounds;
    bool succeeded = GetDWriteInkBox(*mpFontFace, rLayout, mlfEmHeight, bounds);
    succeeded &= BindDC(hDC, bounds);   // Update the bounding rect.

    ID2D1SolidColorBrush* pBrush = NULL;
    COLORREF bgrTextColor = GetTextColor(mhDC);
    succeeded &= SUCCEEDED(mpRT->CreateSolidColorBrush(D2D1::ColorF(GetRValue(bgrTextColor) / 255.0f, GetGValue(bgrTextColor) / 255.0f, GetBValue(bgrTextColor) / 255.0f), &pBrush));

    HRESULT hr = S_OK;
    int nGlyphs = 0;
    if (succeeded)
    {
        const int MAX_GLYPHS = 2;
        sal_GlyphId glyphIntStr[MAX_GLYPHS];
        UINT16 glyphIndices[MAX_GLYPHS];
        long   glyphIntAdv[MAX_GLYPHS];
        FLOAT  glyphAdvances[MAX_GLYPHS];
        DWRITE_GLYPH_OFFSET glyphOffsets[MAX_GLYPHS] = { { 0.0f, 0.0f }, };

        mpRT->BeginDraw();
        do
        {
            nGlyphs = rLayout.GetNextGlyphs(1, glyphIntStr, *pPos, *pGetNextGlypInfo, glyphIntAdv);
            if (nGlyphs < 1)
                break;

            std::copy_n(glyphIntStr, nGlyphs, glyphIndices);
            std::copy_n(glyphIntAdv, nGlyphs, glyphAdvances);

            D2D1_POINT_2F baseline = { pPos->X() - bounds.Left(), pPos->Y() - bounds.Top() };
            DWRITE_GLYPH_RUN glyphs = {
                mpFontFace,
                mlfEmHeight,
                nGlyphs,
                glyphIndices,
                glyphAdvances,
                glyphOffsets,
                false,
                0
            };

            mpRT->DrawGlyphRun(baseline, &glyphs, pBrush);
        } while (!pRectToErase);

        hr = mpRT->EndDraw();
    }

    if (pBrush)
        pBrush->Release();

    ReleaseFont();

    if (hr == D2DERR_RECREATE_TARGET)
        CreateRenderTarget();

    return (succeeded && nGlyphs >= 1 && pRectToErase);
}

bool D2DWriteTextOutRenderer::BindFont(HDC hDC)
{
    // A TextOutRender can only be bound to one font at a time, so the
    assert(mpFontFace == nullptr);
    if (mpFontFace)
    {
        ReleaseFont();
        return false;
    }

    // Initially bind to an empty rectangle to get access to the font face,
    //  we'll update it once we've calculated a bounding rect in DrawGlyphs
    if (!BindDC(mhDC = hDC))
        return false;

    mlfEmHeight = 0;
    return GetDWriteFaceFromHDC(hDC, &mpFontFace, &mlfEmHeight);
}

bool D2DWriteTextOutRenderer::ReleaseFont()
{
    mpFontFace->Release();
    mpFontFace = nullptr;
    mhDC = 0;

    return true;
}

// GetGlyphInkBoxes
// The inkboxes returned have their origin on the baseline, to a -ve value
// of Top() means the glyph extends abs(Top()) many pixels above the
// baseline, and +ve means the ink starts that many pixels below.
std::vector<Rectangle> D2DWriteTextOutRenderer::GetGlyphInkBoxes(uint16_t * pGid, uint16_t * pGidEnd) const
{
    ptrdiff_t nGlyphs = pGidEnd - pGid;
    if (nGlyphs < 0) return std::vector<Rectangle>();

    DWRITE_FONT_METRICS aFontMetrics;
    mpFontFace->GetMetrics(&aFontMetrics);

    std::vector<DWRITE_GLYPH_METRICS> metrics(nGlyphs);
    if (!SUCCEEDED(mpFontFace->GetDesignGlyphMetrics(pGid, nGlyphs, metrics.data(), false)))
        return std::vector<Rectangle>();

    std::vector<Rectangle> aOut(nGlyphs);
    auto pOut = aOut.begin();
    for (auto &m : metrics)
    {
        const long left  = m.leftSideBearing,
                   top   = m.topSideBearing - m.verticalOriginY,
                   right = m.advanceWidth - m.rightSideBearing,
                   bottom = INT32(m.advanceHeight) - m.verticalOriginY - m.bottomSideBearing;

        // Scale to screen space.
        pOut->Left()   = std::lround(left * mlfEmHeight / aFontMetrics.designUnitsPerEm);
        pOut->Top()    = std::lround(top * mlfEmHeight / aFontMetrics.designUnitsPerEm);
        pOut->Right()  = std::lround(right * mlfEmHeight / aFontMetrics.designUnitsPerEm);
        pOut->Bottom() = std::lround(bottom * mlfEmHeight / aFontMetrics.designUnitsPerEm);

        ++pOut;
    }

    return aOut;
}

bool D2DWriteTextOutRenderer::DrawGlyphs(const Point & origin, uint16_t * pGid, uint16_t * pGidEnd,
    float * pAdvances, Point * pOffsets)
{
    Rectangle bounds;
    //bool succeeded = GetDWriteInkBox(*mpFontFace, rLayout, mlfEmHeight, bounds);

    Point aPos = origin;
    auto aInks = GetGlyphInkBoxes(pGid, pGidEnd);
    if (aInks.empty()) return false;

    // Calculate the bounding rectangle.
    auto adv = pAdvances;
    auto ofs = pOffsets;
    for (auto &b:aInks)
    {
        aPos += *ofs++;
        b += aPos;
        aPos.X() += *adv++;
        bounds.Union(b);
    }
    bool succeeded = BindDC(mhDC, bounds);   // Update the bounding rect.

    ID2D1SolidColorBrush* pBrush = NULL;
    COLORREF bgrTextColor = GetTextColor(mhDC);
    succeeded &= SUCCEEDED(mpRT->CreateSolidColorBrush(D2D1::ColorF(GetRValue(bgrTextColor) / 255.0f, GetGValue(bgrTextColor) / 255.0f, GetBValue(bgrTextColor) / 255.0f), &pBrush));

    HRESULT hr = S_OK;
    if (succeeded)
    {
        const int MAX_GLYPHS = 20;
        UINT16 glyphIndices[MAX_GLYPHS];
        FLOAT  glyphAdvances[MAX_GLYPHS];
        DWRITE_GLYPH_OFFSET glyphOffsets[MAX_GLYPHS] = { { 0.0f, 0.0f }, };

        mpRT->BeginDraw();
        for (size_t nGlyphs = std::min(static_cast<int>(pGidEnd - pGid), MAX_GLYPHS); pGid != pGidEnd;
                    pGid += nGlyphs, nGlyphs = std::min(static_cast<int>(pGidEnd - pGid), MAX_GLYPHS))
        {
            std::copy(pGid, pGidEnd, glyphIndices);
            std::copy_n(pAdvances, nGlyphs, glyphAdvances);
            std::transform(pOffsets, pOffsets + nGlyphs,
                glyphOffsets,
                [](Point &o) { return DWRITE_GLYPH_OFFSET{static_cast<FLOAT>(o.X()), static_cast<FLOAT>(o.Y()) }; });
            D2D1_POINT_2F baseline = { origin.X() - bounds.Left(), origin.Y() - bounds.Top() };
            DWRITE_GLYPH_RUN glyphs = {
                mpFontFace,
                mlfEmHeight,
                nGlyphs,
                glyphIndices,
                glyphAdvances,
                glyphOffsets,
                false,
                0
            };

            mpRT->DrawGlyphRun(baseline, &glyphs, pBrush);
        }

        hr = mpRT->EndDraw();
    }

    if (pBrush)
        pBrush->Release();

    if (hr == D2DERR_RECREATE_TARGET)
    {
        ReleaseFont();
        hr = CreateRenderTarget();
    }

    return (SUCCEEDED(hr) && succeeded);
}


bool D2DWriteTextOutRenderer::GetDWriteFaceFromHDC(HDC hDC, IDWriteFontFace ** ppFontFace, float * lfSize) const
{
    bool succeeded = false;
    try
    {
        succeeded = SUCCEEDED(mpGdiInterop->CreateFontFaceFromHdc(hDC, ppFontFace));
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.gdi.opengl", "Error in dwrite while creating font face: " << e.what());
        return false;
    }

    if (succeeded)
    {
        LOGFONTW aLogFont;
        HFONT hFont = (HFONT)::GetCurrentObject(hDC, OBJ_FONT);

        GetObjectW(hFont, sizeof(LOGFONTW), &aLogFont);
        float dpix, dpiy;
        mpRT->GetDpi(&dpix, &dpiy);
        *lfSize = aLogFont.lfHeight * 96.0f / dpiy;

        assert(*lfSize < 0);
        *lfSize *= -1;
    }

    return succeeded;
}

bool D2DWriteTextOutRenderer::GetDWriteInkBox(IDWriteFontFace & rFontFace, WinLayout const &rLayout, float const /*lfEmHeight*/, Rectangle & rOut) const
{
    rOut.SetEmpty();

    DWRITE_FONT_METRICS aFontMetrics;
    rFontFace.GetMetrics(&aFontMetrics);

    Point aPos;
    sal_GlyphId nLGlyph;
    std::vector<uint16_t> indices;
    std::vector<Point>  positions;
    int nStart = 0;
    while (rLayout.GetNextGlyphs(1, &nLGlyph, aPos, nStart) == 1)
    {
        positions.push_back(aPos);
        indices.push_back(nLGlyph);
    }

    auto aBoxes = GetGlyphInkBoxes(indices.data(), indices.data() + indices.size());
    if (aBoxes.empty())
        return false;

    auto p = positions.begin();
    for (auto &b:aBoxes)
    {
        b += *p++;
        rOut.Union(b);
    }

    return true;
}

#endif // ENABLE_GRAPHITE_DWRITE

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

GraphiteWinLayout::GraphiteWinLayout(HDC hDC, const WinFontFace& rWFD, WinFontInstance& rWFE, bool bUseOpenGL) throw()
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

bool GraphiteWinLayout::LayoutText(ImplLayoutArgs & args)
{
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
    bool bSucceeded = maImpl.LayoutText(args);
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

bool GraphiteWinLayout::DrawTextImpl(HDC hDC,
                                     const Rectangle* pRectToErase,
                                     Point* pPos,
                                     int* pGetNextGlypInfo) const
{
    HFONT hOrigFont = DisableFontScaling();
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;

    TextOutRenderer & render = TextOutRenderer::get();
    bool const ok = render(*this, hDC, pRectToErase, pPos, pGetNextGlypInfo);
    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));
    return ok;
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

SalLayout* WinSalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    if (!mpWinFontEntry[nFallbackLevel]) return nullptr;

    assert(mpWinFontData[nFallbackLevel]);

    WinLayout* pWinLayout = NULL;

    const WinFontFace& rFontFace = *mpWinFontData[ nFallbackLevel ];
    WinFontInstance& rFontInstance = *mpWinFontEntry[ nFallbackLevel ];

    bool bUseOpenGL = OpenGLHelper::isVCLOpenGLEnabled() && !mbPrinter;

    if (!bUspInited)
        InitUSP();

    if( !(rArgs.mnFlags & SalLayoutFlags::ComplexDisabled) )
    {
#if ENABLE_GRAPHITE
        if (rFontFace.SupportsGraphite())
        {
            pWinLayout = new GraphiteWinLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
        }
        else
#endif // ENABLE_GRAPHITE
        {
            // script complexity is determined in upper layers
            pWinLayout = new UniscribeLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
            // NOTE: it must be guaranteed that the WinSalGraphics lives longer than
            // the created UniscribeLayout, otherwise the data passed into the
            // constructor might become invalid too early
        }
    }
    else
    {
#if ENABLE_GRAPHITE
        if (rFontFace.SupportsGraphite())
        {
            pWinLayout = new GraphiteWinLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
        }
        else
#endif // ENABLE_GRAPHITE
        {
            static bool bAvoidSimpleWinLayout = (std::getenv("VCL_NO_SIMPLEWINLAYOUT") != NULL);

            if (!bAvoidSimpleWinLayout)
            {
                if( (rArgs.mnFlags & SalLayoutFlags::KerningPairs) && !rFontInstance.HasKernData() )
                {
                    // TODO: directly cache kerning info in the rFontInstance
                    // TODO: get rid of kerning methods+data in WinSalGraphics object
                    GetKernPairs();
                    rFontInstance.SetKernData( mnFontKernPairCount, mpFontKernPairs );
                }

                pWinLayout = new SimpleWinLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
            }
            else
            {
                pWinLayout = new UniscribeLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
                // NOTE: it must be guaranteed that the WinSalGraphics lives longer than
                // the created UniscribeLayout, otherwise the data passed into the
                // constructor might become invalid too early
            }
        }
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

LogicalFontInstance * WinSalGraphics::GetWinFontEntry(int const nFallbackLevel)
{
    return mpWinFontEntry[nFallbackLevel];
}

WinFontInstance::WinFontInstance( FontSelectPattern& rFSD )
:   LogicalFontInstance( rFSD )
,    mpKerningPairs( NULL )
,    mnKerningPairs( -1 )
,    mpGLyphyAtlas( nullptr )
,    mpGLyphyFont( nullptr )
,    maWidthMap( 512 )
,    mnMinKashidaWidth( -1 )
,    mnMinKashidaGlyph( -1 )
{
    maScriptCache = NULL;
    mpGLyphyFont = NULL;
    mbGLyphySetupCalled = false;
}

WinFontInstance::~WinFontInstance()
{
    if( maScriptCache != NULL )
        ScriptFreeCache( &maScriptCache );
    delete[] mpKerningPairs;
}

bool WinFontInstance::HasKernData() const
{
    return (mnKerningPairs >= 0);
}

void WinFontInstance::SetKernData( int nPairCount, const KERNINGPAIR* pPairData )
{
    mnKerningPairs = nPairCount;
    mpKerningPairs = new KERNINGPAIR[ mnKerningPairs ];
    memcpy( mpKerningPairs, (const void*)pPairData, nPairCount*sizeof(KERNINGPAIR) );
}

int WinFontInstance::GetKerning( sal_Unicode cLeft, sal_Unicode cRight ) const
{
    int nKernAmount = 0;
    if( mpKerningPairs )
    {
        const KERNINGPAIR aRefPair = { cLeft, cRight, 0 };
        const KERNINGPAIR* pFirstPair = mpKerningPairs;
        const KERNINGPAIR* pEndPair = mpKerningPairs + mnKerningPairs;
        const KERNINGPAIR* pPair = std::lower_bound( pFirstPair,
            pEndPair, aRefPair, ImplCmpKernData );
        if( (pPair != pEndPair)
        &&  (pPair->wFirst == aRefPair.wFirst)
        &&  (pPair->wSecond == aRefPair.wSecond) )
            nKernAmount = pPair->iKernAmount;
    }

    return nKernAmount;
}

bool WinFontInstance::InitKashidaHandling( HDC hDC )
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

PhysicalFontFace* WinFontFace::Clone() const
{
#if ENABLE_GRAPHITE
    if ( mpGraphiteData )
        mpGraphiteData->AddReference();
#endif
    PhysicalFontFace* pClone = new WinFontFace( *this );
    return pClone;
}

LogicalFontInstance* WinFontFace::CreateFontInstance( FontSelectPattern& rFSD ) const
{
    LogicalFontInstance* pFontInstance = new WinFontInstance( rFSD );
    return pFontInstance;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
