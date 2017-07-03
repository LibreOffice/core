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

class WinFontInstance;

namespace
{
// Extra space at the top and bottom of the glyph in total = tmHeight / GLYPH_SPACE_RATIO;
const int GLYPH_SPACE_RATIO = 8;
// Border size at the top of the glyph = tmHeight / GLYPH_OFFSET_RATIO;
const int GLYPH_OFFSET_RATIO = GLYPH_SPACE_RATIO * 2;
}

struct OpenGLGlyphDrawElement
{
    tools::Rectangle maLocation;
    int maLeftOverhangs;
    OpenGLTexture maTexture;
    int mnBaselineOffset;
    int mnHeight;
    bool mbVertical;

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

    static GlobalGlyphCache * get();
};

class GlyphCache
{
private:
    std::unordered_map<int, OpenGLGlyphDrawElement> maOpenGLTextureCache;

public:
    GlyphCache()
    {
        GlobalGlyphCache::get()->maGlyphCaches.insert(this);
    }

    ~GlyphCache()
    {
        GlobalGlyphCache::get()->maGlyphCaches.erase(this);
    }

    static bool ReserveTextureSpace(OpenGLGlyphDrawElement& rElement, int nWidth, int nHeight)
    {
        GlobalGlyphCache* pGlobalGlyphCache = GlobalGlyphCache::get();
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

    GlyphCache maGlyphCache;
public:
    bool CacheGlyphToAtlas(HDC hDC, HFONT hFont, int nGlyphIndex, SalGraphics& rGraphics);

    GlyphCache& GetGlyphCache()
    {
        return maGlyphCache;
    }
};

class TextOutRenderer
{
protected:
    explicit TextOutRenderer() = default;
    TextOutRenderer(const TextOutRenderer &) = delete;
    TextOutRenderer & operator = (const TextOutRenderer &) = delete;

public:
    static TextOutRenderer & get(bool bUseDWrite);

    virtual ~TextOutRenderer() = default;

    virtual bool operator ()(CommonSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC) = 0;
};

class ExTextOutRenderer : public TextOutRenderer
{
    ExTextOutRenderer(const ExTextOutRenderer &) = delete;
    ExTextOutRenderer & operator = (const ExTextOutRenderer &) = delete;

public:
    explicit ExTextOutRenderer() = default;

    bool operator ()(CommonSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC) override;
};

enum class D2DTextAntiAliasMode
{
    Default,
    ClearType,
    AntiAliased,
};

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

    bool operator ()(CommonSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC) override;

    bool BindDC(HDC hDC, tools::Rectangle const & rRect = tools::Rectangle(0, 0, 0, 0)) {
        RECT const rc = { rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom() };
        return SUCCEEDED(mpRT->BindDC(hDC, &rc));
    }

    bool BindFont(HDC hDC) /*override*/;
    bool ReleaseFont() /*override*/;

    std::vector<tools::Rectangle>  GetGlyphInkBoxes(uint16_t * pGid, uint16_t * pGidEnd) const /*override*/;
    ID2D1RenderTarget * GetRenderTarget() const { return mpRT; }
    IDWriteFontFace   * GetFontFace() const { return mpFontFace; }
    float               GetEmHeight() const { return mlfEmHeight; }

    HRESULT CreateRenderTarget() {
        if (mpRT) mpRT->Release(); mpRT = nullptr;
        return mpD2DFactory->CreateDCRenderTarget(&mRTProps, &mpRT);
    }

    bool Ready() const { return mpGdiInterop && mpRT; }

    void applyTextAntiAliasMode();
    void setTextAntiAliasMode(D2DTextAntiAliasMode eMode)
    {
        meTextAntiAliasMode = eMode;
    }
private:
    static void CleanupModules();

    // This is a singleton object disable copy ctor and assignment operator
    D2DWriteTextOutRenderer(const D2DWriteTextOutRenderer &) = delete;
    D2DWriteTextOutRenderer & operator = (const D2DWriteTextOutRenderer &) = delete;

    bool GetDWriteFaceFromHDC(HDC hDC, IDWriteFontFace ** ppFontFace, float * lfSize) const;

    ID2D1Factory        * mpD2DFactory;
    IDWriteFactory      * mpDWriteFactory;
    IDWriteGdiInterop   * mpGdiInterop;
    ID2D1DCRenderTarget * mpRT;
    const D2D1_RENDER_TARGET_PROPERTIES mRTProps;

    IDWriteFontFace * mpFontFace;
    float             mlfEmHeight;
    HDC               mhDC;
    D2DTextAntiAliasMode meTextAntiAliasMode;
    IDWriteRenderingParams* mpRenderingParameters;
};

#endif // INCLUDED_VCL_INC_WIN_WINLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
