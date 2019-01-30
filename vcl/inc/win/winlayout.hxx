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

#include <opengl/PackedTextureAtlas.hxx>

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

class OpenGLGlyphCache;

struct GlobalOpenGLGlyphCache
{
    GlobalOpenGLGlyphCache()
        : maPackedTextureAtlas(2048, 2048)
    {}

    PackedTextureAtlasManager maPackedTextureAtlas;
    std::unordered_set<OpenGLGlyphCache*> maOpenGLGlyphCaches;

    static GlobalOpenGLGlyphCache * get();
};

class OpenGLGlyphCache
{
private:
    std::unordered_map<int, OpenGLGlyphDrawElement> maOpenGLTextureCache;

public:
    OpenGLGlyphCache()
    {
        GlobalOpenGLGlyphCache::get()->maOpenGLGlyphCaches.insert(this);
    }

    ~OpenGLGlyphCache()
    {
        GlobalOpenGLGlyphCache::get()->maOpenGLGlyphCaches.erase(this);
    }

    static bool ReserveTextureSpace(OpenGLGlyphDrawElement& rElement, int nWidth, int nHeight)
    {
        GlobalOpenGLGlyphCache* pGlobalOpenGLGlyphCache = GlobalOpenGLGlyphCache::get();
        rElement.maTexture = pGlobalOpenGLGlyphCache->maPackedTextureAtlas.Reserve(nWidth, nHeight);
        if (!rElement.maTexture)
            return false;
        std::vector<GLuint> aTextureIDs = pGlobalOpenGLGlyphCache->maPackedTextureAtlas.ReduceTextureNumber(8);
        if (!aTextureIDs.empty())
        {
            for (auto& pOpenGLGlyphCache: pGlobalOpenGLGlyphCache->maOpenGLGlyphCaches)
            {
                pOpenGLGlyphCache->RemoveTextures(aTextureIDs);
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
        maOpenGLTextureCache[nGlyphIndex] = rElement;
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

// win32 specific logical font instance
class WinFontInstance : public LogicalFontInstance
{
    friend rtl::Reference<LogicalFontInstance> WinFontFace::CreateFontInstance(const FontSelectPattern&) const;

public:
    ~WinFontInstance() override;

    bool hasHScale() const;

    void SetGraphics(WinSalGraphics*);
    WinSalGraphics* GetGraphics() const { return m_pGraphics; }

    HFONT GetHFONT() const { return m_hFont; }
    float GetScale() const { return m_fScale; }

    // Prevent deletion of the HFONT in the WinFontInstance destructor
    // Used for the ScopedFont handling
    void SetHFONT(HFONT hFont) { m_hFont = hFont; }

    const WinFontFace * GetFontFace() const { return static_cast<const WinFontFace *>(LogicalFontInstance::GetFontFace()); }

    bool CacheGlyphToAtlas(HDC hDC, HFONT hFont, int nGlyphIndex, SalGraphics& rGraphics);
    OpenGLGlyphCache& GetOpenGLGlyphCache() { return maOpenGLGlyphCache; }

    bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override;

private:
    explicit WinFontInstance(const WinFontFace&, const FontSelectPattern&);

    hb_font_t* ImplInitHbFont() override;
    bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const override;

    WinSalGraphics *m_pGraphics;
    HFONT m_hFont;
    float m_fScale;
    OpenGLGlyphCache maOpenGLGlyphCache;
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

    virtual bool operator ()(GenericSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC) = 0;
};

class ExTextOutRenderer : public TextOutRenderer
{
    ExTextOutRenderer(const ExTextOutRenderer &) = delete;
    ExTextOutRenderer & operator = (const ExTextOutRenderer &) = delete;

public:
    explicit ExTextOutRenderer() = default;

    bool operator ()(GenericSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC) override;
};

#endif // INCLUDED_VCL_INC_WIN_WINLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
