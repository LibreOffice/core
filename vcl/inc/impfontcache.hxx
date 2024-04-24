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

#pragma once

#include <sal/config.h>

#include <basegfx/range/b2drectangle.hxx>
#include <rtl/ref.hxx>
#include <o3tl/lru_map.hxx>
#include <o3tl/hash_combine.hxx>

#include "font/FontSelectPattern.hxx"
#include "glyphid.hxx"

class Size;
namespace vcl { class Font; }
namespace vcl::font { class PhysicalFontCollection; }

// TODO: closely couple with PhysicalFontCollection

struct GlyphBoundRectCacheKey
{
    const LogicalFontInstance* m_pFont;
    const sal_GlyphId m_nId;

    GlyphBoundRectCacheKey(const LogicalFontInstance* pFont, sal_GlyphId nID)
        : m_pFont(pFont), m_nId(nID)
    {}

    bool operator==(GlyphBoundRectCacheKey const& aOther) const
    { return m_pFont == aOther.m_pFont && m_nId == aOther.m_nId; }
};

struct GlyphBoundRectCacheHash
{
    std::size_t operator()(GlyphBoundRectCacheKey const& aCache) const
    {
        std::size_t seed = 0;
        o3tl::hash_combine(seed, aCache.m_pFont);
        o3tl::hash_combine(seed, aCache.m_nId);
        return seed;
    }
};

typedef o3tl::lru_map<GlyphBoundRectCacheKey, basegfx::B2DRectangle,
                      GlyphBoundRectCacheHash> GlyphBoundRectCache;

class ImplFontCache
{
private:
    // cache of recently used font instances
    struct IFSD_Equal { bool operator()( const vcl::font::FontSelectPattern&, const vcl::font::FontSelectPattern& ) const; };
    struct IFSD_Hash { size_t operator()( const vcl::font::FontSelectPattern& ) const; };
    typedef o3tl::lru_map<vcl::font::FontSelectPattern, rtl::Reference<LogicalFontInstance>, IFSD_Hash, IFSD_Equal> FontInstanceList;

    LogicalFontInstance* mpLastHitCacheEntry; ///< keeps the last hit cache entry
    FontInstanceList maFontInstanceList;
    GlyphBoundRectCache m_aBoundRectCache;

    rtl::Reference<LogicalFontInstance> GetFontInstance(vcl::font::PhysicalFontCollection const*, vcl::font::FontSelectPattern&);

public:
    ImplFontCache();
    ~ImplFontCache();

    rtl::Reference<LogicalFontInstance> GetFontInstance(vcl::font::PhysicalFontCollection const *,
                             const vcl::Font&, const Size& rPixelSize, float fExactHeight, bool bNonAntialias = false);
    rtl::Reference<LogicalFontInstance> GetGlyphFallbackFont( vcl::font::PhysicalFontCollection const *, vcl::font::FontSelectPattern&,
                            LogicalFontInstance* pLogicalFont,
                            int nFallbackLevel, OUString& rMissingCodes );

    bool GetCachedGlyphBoundRect(const LogicalFontInstance*, sal_GlyphId, basegfx::B2DRectangle&);
    void CacheGlyphBoundRect(const LogicalFontInstance*, sal_GlyphId, basegfx::B2DRectangle&);

    void                Invalidate();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
