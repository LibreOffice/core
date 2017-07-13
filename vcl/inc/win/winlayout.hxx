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

#ifndef INCLUDED_VCL_INC_WIN_WINLAYOUT_HXX
#define INCLUDED_VCL_INC_WIN_WINLAYOUT_HXX

#include <rtl/ustring.hxx>

#include <sallayout.hxx>
#include <svsys.h>
#include <win/salgdi.h>

#include <usp10.h>
#include <d2d1.h>
#include <dwrite.h>

#include "opengl/PackedTextureAtlas.hxx"

typedef std::unordered_map<int,int> IntMap;

// Graphite headers
#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <graphite_layout.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <graphite_features.hxx>
#endif

class WinFontInstance;
struct VisualItem;

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

    static bool ReserveTextureSpace(OpenGLGlyphDrawElement& rElement, int nWidth, int nHeight)
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

// win32 specific physical font instance
class WinFontInstance : public LogicalFontInstance
{
public:
    explicit                WinFontInstance( FontSelectPattern& );
    virtual                 ~WinFontInstance() override;

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

private:
    GlyphCache maGlyphCache;
public:
    bool CacheGlyphToAtlas(bool bRealGlyphIndices, HDC hDC, HFONT hFont, int nGlyphIndex, SalGraphics& rGraphics);

    GlyphCache& GetGlyphCache()
    {
        return maGlyphCache;
    }

private:
    std::unordered_map<int, int>    maWidthMap;
    mutable int                     mnMinKashidaWidth;
    mutable int                     mnMinKashidaGlyph;
};

class WinLayout : public SalLayout
{
public:
                        WinLayout(HDC, const WinFontFace&, WinFontInstance&, bool bUseOpenGL);
    virtual             ~WinLayout() override;
    virtual void        InitFont() const override;
    void                SetFontScale( float f ) { mfFontScale = f; }
    HFONT               DisableFontScaling() const;

    SCRIPT_CACHE&       GetScriptCache() const;

    /// In the non-OpenGL case, call the DrawTextImpl directly, otherwise make
    /// sure we draw to an interim texture.
    virtual void        DrawText(SalGraphics&) const override;

    /// Draw to the provided HDC.
    virtual bool        DrawTextImpl(HDC hDC, const Rectangle* pRectToErase, Point* pPos, int* pGetNextGlypInfo) const = 0;

    virtual bool        CacheGlyphs(SalGraphics& rGraphics) const = 0;
    virtual bool        DrawCachedGlyphs(SalGraphics& rGraphics) const = 0;

    HDC                 mhDC;               // WIN32 device handle
    HFONT               mhFont;             // WIN32 font handle
    int                 mnBaseAdv;          // x-offset relative to Layout origin
    float               mfFontScale;        // allows metrics emulation of huge font sizes
    bool                mbUseOpenGL;        ///< We need to render via OpenGL

    const WinFontFace& mrWinFontData;
    WinFontInstance&   mrWinFontEntry;
};

class SimpleWinLayout : public WinLayout
{
public:
                    SimpleWinLayout(HDC, const WinFontFace&, WinFontInstance&, bool bUseOpenGL);
    virtual         ~SimpleWinLayout() override;

    virtual bool    LayoutText( ImplLayoutArgs& ) override;
    virtual void    AdjustLayout( ImplLayoutArgs& ) override;
    virtual bool    DrawTextImpl(HDC hDC, const Rectangle* pRectToErase, Point* pPos, int* pGetNextGlypInfo) const override;

    virtual bool    CacheGlyphs(SalGraphics& rGraphics) const override;
    virtual bool    DrawCachedGlyphs(SalGraphics& rGraphics) const override;
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                                   DeviceCoordinate* pGlyphAdvances = nullptr, int* pCharIndexes = nullptr,
                                   const PhysicalFontFace** pFallbackFonts = nullptr ) const override;

    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const override;
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const override;

    // for glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) override;
    virtual void    DropGlyph( int nStart ) override;
    virtual void    Simplify( bool bIsBase ) override;

protected:
    void            Justify( DeviceCoordinate nNewWidth );
    void            ApplyDXArray( const ImplLayoutArgs& );

private:
    int             mnGlyphCount;
    int             mnCharCount;
    WCHAR*          mpOutGlyphs;
    int*            mpGlyphAdvances;    // if possible this is shared with mpGlyphAdvances[]
    int*            mpGlyphOrigAdvs;
    int*            mpCharWidths;       // map rel char pos to char width
    int*            mpChars2Glyphs;     // map rel char pos to abs glyph pos
    int*            mpGlyphs2Chars;     // map abs glyph pos to abs char pos
    bool*           mpGlyphRTLFlags;    // BiDi status for glyphs: true=>RTL
    mutable long    mnWidth;

    int             mnNotdefWidth;
};

class UniscribeLayout : public WinLayout
{
public:
                    UniscribeLayout(HDC, const WinFontFace&, WinFontInstance&, bool bUseOpenGL);

    virtual bool    LayoutText( ImplLayoutArgs& ) override;
    virtual void    AdjustLayout( ImplLayoutArgs& ) override;
    virtual bool    DrawTextImpl(HDC hDC, const Rectangle* pRectToErase, Point* pPos, int* pGetNextGlypInfo) const override;
    virtual bool    CacheGlyphs(SalGraphics& rGraphics) const override;
    virtual bool    DrawCachedGlyphs(SalGraphics& rGraphics) const override;
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                                   DeviceCoordinate* pGlyphAdvances = nullptr, int* pCharPosAry = nullptr,
                                   const PhysicalFontFace** pFallbackFonts = nullptr ) const override;

    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const override;
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const override;
    virtual bool    IsKashidaPosValid ( int nCharPos ) const override;

    // for glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) override;
    virtual void    DropGlyph( int nStart ) override;
    virtual void    Simplify( bool bIsBase ) override;
    virtual void    DisableGlyphInjection( bool bDisable ) override { mbDisableGlyphInjection = bDisable; }

protected:
    virtual         ~UniscribeLayout() override;

    void            Justify( DeviceCoordinate nNewWidth );
    void            ApplyDXArray( const ImplLayoutArgs& );

    bool            GetItemSubrange( const VisualItem&,
                        int& rMinIndex, int& rEndIndex ) const;

private:
    // item specific info
    SCRIPT_ITEM*    mpScriptItems;      // in logical order
    VisualItem*     mpVisualItems;      // in visual order
    int             mnItemCount;        // number of visual items

    // string specific info
    // everything is in logical order
    OUString        msTheString;        // Sadly we need it in GetNextGlyphs(), to be able to call GetVerticalFlags()
    int             mnCharCapacity;
    WORD*           mpLogClusters;      // map from absolute_char_pos to relative_glyph_pos
    int*            mpCharWidths;       // map from absolute_char_pos to char_width
    int             mnSubStringMin;     // char_pos of first char in context

    // glyph specific info
    // everything is in visual order
    int             mnGlyphCount;
    int             mnGlyphCapacity;
    int*            mpGlyphAdvances;    // glyph advance width before justification
    int*            mpJustifications;   // glyph advance width after justification
    WORD*           mpOutGlyphs;        // glyphids in visual order
    GOFFSET*        mpGlyphOffsets;     // glyph offsets to the "naive" layout
    SCRIPT_VISATTR* mpVisualAttrs;      // glyph visual attributes
    mutable int*    mpGlyphs2Chars;     // map from absolute_glyph_pos to absolute_char_pos

    // kashida stuff
    void InitKashidaHandling();
    void KashidaItemFix( int nMinGlyphPos, int nEndGlyphPos );
    bool KashidaWordFix( int nMinGlyphPos, int nEndGlyphPos, int* pnCurrentPos );

    int            mnMinKashidaWidth;
    int            mnMinKashidaGlyph;
    bool           mbDisableGlyphInjection;
};

#if ENABLE_GRAPHITE

class GraphiteLayoutWinImpl : public GraphiteLayout
{
public:
    GraphiteLayoutWinImpl(const gr_face * pFace, WinFontInstance & rFont)
        throw()
    : GraphiteLayout(pFace), mrFont(rFont) {};
    virtual ~GraphiteLayoutWinImpl() throw() override {};
    virtual sal_GlyphId getKashidaGlyph(int & rWidth) override;
private:
    WinFontInstance & mrFont;
};

/// This class uses the SIL Graphite engine to provide complex text layout services to the VCL
class GraphiteWinLayout : public WinLayout
{
private:
    gr_font * mpFont;
    grutils::GrFeatureParser * mpFeatures;
    mutable GraphiteLayoutWinImpl maImpl;
public:
    GraphiteWinLayout(HDC hDC, const WinFontFace& rWFD, WinFontInstance& rWFE, bool bUseOpenGL) throw();
    virtual ~GraphiteWinLayout() override;

    // used by upper layers
    virtual bool  LayoutText( ImplLayoutArgs& ) override;    // first step of layout
    virtual void  AdjustLayout( ImplLayoutArgs& ) override;  // adjusting after fallback etc.
    virtual bool  DrawTextImpl(HDC hDC, const Rectangle* pRectToErase, Point* pPos, int* pGetNextGlypInfo) const override;
    virtual bool  CacheGlyphs(SalGraphics& rGraphics) const override;
    virtual bool  DrawCachedGlyphs(SalGraphics& rGraphics) const override;

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const override;

    virtual void  GetCaretPositions( int nArraySize, long* pCaretXArray ) const override;

    // methods using glyph indexing
    virtual int   GetNextGlyphs(int nLen, sal_GlyphId* pGlyphIdxAry, ::Point & rPos, int&,
                                DeviceCoordinate* pGlyphAdvAry = nullptr, int* pCharPosAry = nullptr,
                                const PhysicalFontFace** pFallbackFonts = nullptr ) const override;

    // used by glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) override;
    virtual void    DropGlyph( int nStart ) override;
    virtual void    Simplify( bool bIsBase ) override;
};

#endif // ENABLE_GRAPHITE

class TextOutRenderer
{
protected:
    explicit TextOutRenderer() = default;
    TextOutRenderer(const TextOutRenderer &) = delete;
    TextOutRenderer & operator = (const TextOutRenderer &) = delete;

public:
    static TextOutRenderer & get(bool bUseDWrite);

    virtual ~TextOutRenderer() = default;

    virtual bool operator ()(SalLayout const &rLayout, HDC hDC,
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

    bool operator ()(SalLayout const &rLayout, HDC hDC,
        const Rectangle* pRectToErase,
        Point* pPos, int* pGetNextGlypInfo) override;
};

class D2DWriteTextOutRenderer : public TextOutRenderer
{
    typedef HRESULT(WINAPI *pD2D1CreateFactory_t)(D2D1_FACTORY_TYPE,
        REFIID, const D2D1_FACTORY_OPTIONS *, void **);

    typedef HRESULT(WINAPI *pD2D1MakeRotateMatrix_t)(float, D2D1_POINT_2F,
        D2D1_MATRIX_3X2_F*);

    typedef HRESULT(WINAPI *pDWriteCreateFactory_t)(DWRITE_FACTORY_TYPE,
        REFIID, IUnknown **);

    static HINSTANCE mmD2d1, mmDWrite;
    static pD2D1CreateFactory_t     D2D1CreateFactory;
    static pD2D1MakeRotateMatrix_t  D2D1MakeRotateMatrix;
    static pDWriteCreateFactory_t   DWriteCreateFactory;

public:
    static bool InitModules();

    explicit D2DWriteTextOutRenderer();
    virtual ~D2DWriteTextOutRenderer() override;

    bool operator ()(SalLayout const &rLayout, HDC hDC,
        const Rectangle* pRectToErase,
        Point* pPos, int* pGetNextGlypInfo) override;

    inline bool BindDC(HDC hDC, Rectangle const & rRect = Rectangle(0, 0, 1, 1))
    {
        if (rRect.GetWidth() == 0 || rRect.GetHeight() == 0)
            return false;
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
    bool GetDWriteInkBox(SalLayout const &rLayout, Rectangle &) const;

    ID2D1Factory        * mpD2DFactory;
    IDWriteFactory      * mpDWriteFactory;
    IDWriteGdiInterop   * mpGdiInterop;
    ID2D1DCRenderTarget * mpRT;
    const D2D1_RENDER_TARGET_PROPERTIES mRTProps;

    IDWriteFontFace * mpFontFace;
    float             mlfEmHeight;
    HDC               mhDC;
};

#endif // INCLUDED_VCL_INC_WIN_WINLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
