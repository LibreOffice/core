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

class WinFontInstance;

namespace
{
// Extra space at the top and bottom of the glyph in total = tmHeight / GLYPH_SPACE_RATIO;
const int GLYPH_SPACE_RATIO = 8;
// Border size at the top of the glyph = tmHeight / GLYPH_OFFSET_RATIO;
const int GLYPH_OFFSET_RATIO = GLYPH_SPACE_RATIO * 2;
}

struct WinGlyphDrawElement
{
    tools::Rectangle maLocation;
    int maLeftOverhangs;
    std::unique_ptr<CompatibleDC::Texture> maTexture;
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

class WinGlyphCache;

struct GlobalWinGlyphCache
{
    std::unordered_set<WinGlyphCache*> maWinGlyphCaches;

    static GlobalWinGlyphCache * get();

    virtual bool AllocateTexture(WinGlyphDrawElement& rElement, int nWidth, int nHeight) = 0;
};

class WinGlyphCache
{
protected:
    std::unordered_map<int, WinGlyphDrawElement> maWinTextureCache;

public:
    WinGlyphCache()
    {
        GlobalWinGlyphCache::get()->maWinGlyphCaches.insert(this);
    }

    virtual ~WinGlyphCache()
    {
        GlobalWinGlyphCache::get()->maWinGlyphCaches.erase(this);
    }

    void PutDrawElementInCache(WinGlyphDrawElement&& rElement, int nGlyphIndex)
    {
        assert(!IsGlyphCached(nGlyphIndex));
        maWinTextureCache[nGlyphIndex] = std::move( rElement );
    }

    WinGlyphDrawElement& GetDrawElement(int nGlyphIndex)
    {
        assert(IsGlyphCached(nGlyphIndex));
        return maWinTextureCache[nGlyphIndex];
    }

    bool IsGlyphCached(int nGlyphIndex) const
    {
        return maWinTextureCache.find(nGlyphIndex) != maWinTextureCache.end();
    }
};

// win32 specific logical font instance
class WinFontInstance : public LogicalFontInstance
{
    friend rtl::Reference<LogicalFontInstance> WinFontFace::CreateFontInstance(const FontSelectPattern&) const;

public:
    ~WinFontInstance() override;

    bool hasHScale() const;
    float getHScale() const;

    void SetGraphics(WinSalGraphics*);
    WinSalGraphics* GetGraphics() const { return m_pGraphics; }

    HFONT GetHFONT() const { return m_hFont; }
    float GetScale() const { return m_fScale; }

    // Prevent deletion of the HFONT in the WinFontInstance destructor
    // Used for the ScopedFont handling
    void SetHFONT(HFONT hFont) { m_hFont = hFont; }

    const WinFontFace * GetFontFace() const { return static_cast<const WinFontFace *>(LogicalFontInstance::GetFontFace()); }

    bool CacheGlyphToAtlas(HDC hDC, HFONT hFont, int nGlyphIndex, SalGraphics& rGraphics, const GenericSalLayout& rLayout);
    WinGlyphCache& GetWinGlyphCache() { return maWinGlyphCache; }

    bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override;

private:
    explicit WinFontInstance(const WinFontFace&, const FontSelectPattern&);

    hb_font_t* ImplInitHbFont() override;
    bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const override;

    WinSalGraphics *m_pGraphics;
    HFONT m_hFont;
    float m_fScale;
    WinGlyphCache maWinGlyphCache;
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
