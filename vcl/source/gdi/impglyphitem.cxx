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

#include <impglyphitem.hxx>
#include <vcl/glyphitemcache.hxx>
#include <vcl/vcllayout.hxx>
#include <vcl/lazydelete.hxx>
#include <tools/stream.hxx>
#include <TextLayoutCache.hxx>
#include <config_fuzzers.h>

SalLayoutGlyphs::SalLayoutGlyphs() {}

SalLayoutGlyphs::~SalLayoutGlyphs() {}

SalLayoutGlyphs::SalLayoutGlyphs(SalLayoutGlyphs&& rOther) noexcept
{
    std::swap(m_pImpl, rOther.m_pImpl);
    std::swap(m_pExtraImpls, rOther.m_pExtraImpls);
}

SalLayoutGlyphs& SalLayoutGlyphs::operator=(SalLayoutGlyphs&& rOther) noexcept
{
    if (this != &rOther)
    {
        std::swap(m_pImpl, rOther.m_pImpl);
        std::swap(m_pExtraImpls, rOther.m_pExtraImpls);
    }
    return *this;
}

bool SalLayoutGlyphs::IsValid() const
{
    if (m_pImpl == nullptr)
        return false;
    if (!m_pImpl->IsValid())
        return false;
    if (m_pExtraImpls)
        for (std::unique_ptr<SalLayoutGlyphsImpl> const& impl : *m_pExtraImpls)
            if (!impl->IsValid())
                return false;
    return true;
}

void SalLayoutGlyphs::Invalidate()
{
    // Invalidating is in fact simply clearing.
    m_pImpl.reset();
    m_pExtraImpls.reset();
}

SalLayoutGlyphsImpl* SalLayoutGlyphs::Impl(unsigned int nLevel) const
{
    if (nLevel == 0)
        return m_pImpl.get();
    if (m_pExtraImpls != nullptr && nLevel - 1 < m_pExtraImpls->size())
        return (*m_pExtraImpls)[nLevel - 1].get();
    return nullptr;
}

void SalLayoutGlyphs::AppendImpl(SalLayoutGlyphsImpl* pImpl)
{
    if (!m_pImpl)
        m_pImpl.reset(pImpl);
    else
    {
        if (!m_pExtraImpls)
            m_pExtraImpls.reset(new std::vector<std::unique_ptr<SalLayoutGlyphsImpl>>);
        m_pExtraImpls->emplace_back(pImpl);
    }
}

SalLayoutGlyphsImpl* SalLayoutGlyphsImpl::clone() const { return new SalLayoutGlyphsImpl(*this); }

bool SalLayoutGlyphsImpl::IsValid() const
{
    if (!m_rFontInstance.is())
        return false;
    if (empty())
        return false;
    return true;
}

SalLayoutGlyphsCache* SalLayoutGlyphsCache::self()
{
    static vcl::DeleteOnDeinit<SalLayoutGlyphsCache> cache(1000);
    return cache.get();
}

const SalLayoutGlyphs*
SalLayoutGlyphsCache::GetLayoutGlyphs(VclPtr<const OutputDevice> outputDevice, const OUString& text,
                                      sal_Int32 nIndex, sal_Int32 nLen, tools::Long nLogicWidth,
                                      const vcl::text::TextLayoutCache* layoutCache) const
{
    if (nLen == 0)
        return nullptr;
    const CachedGlyphsKey key(outputDevice, text, nIndex, nLen, nLogicWidth);
    auto it = mCachedGlyphs.find(key);
    if (it != mCachedGlyphs.end())
    {
        if (it->second.IsValid())
            return &it->second;
        // Do not try to create the layout here. If a cache item exists, it's already
        // been attempted and the layout was invalid (this happens with MultiSalLayout).
        // So in that case this is a cached failure.
        return nullptr;
    }
    std::shared_ptr<const vcl::text::TextLayoutCache> tmpLayoutCache;
    if (layoutCache == nullptr)
    {
        tmpLayoutCache = OutputDevice::CreateTextLayoutCache(text);
        layoutCache = tmpLayoutCache.get();
    }
#if !ENABLE_FUZZERS
    const SalLayoutFlags glyphItemsOnlyLayout = SalLayoutFlags::GlyphItemsOnly;
#else
    // ofz#39150 skip detecting bidi directions
    const SalLayoutFlags glyphItemsOnlyLayout
        = SalLayoutFlags::GlyphItemsOnly | SalLayoutFlags::BiDiStrong;
#endif
    std::unique_ptr<SalLayout> layout = outputDevice->ImplLayout(
        text, nIndex, nLen, Point(0, 0), nLogicWidth, {}, glyphItemsOnlyLayout, layoutCache);
    if (layout)
    {
        mCachedGlyphs.insert(std::make_pair(key, layout->GetGlyphs()));
        assert(mCachedGlyphs.find(key) == mCachedGlyphs.begin()); // newly inserted item is first
        return &mCachedGlyphs.begin()->second;
    }
    else
    {
        // Failure, cache it too as invalid glyphs.
        mCachedGlyphs.insert(std::make_pair(key, SalLayoutGlyphs()));
        return nullptr;
    }
}

SalLayoutGlyphsCache::CachedGlyphsKey::CachedGlyphsKey(
    const VclPtr<const OutputDevice>& outputDevice, const OUString& t, sal_Int32 i, sal_Int32 l,
    tools::Long w)
    : text(t)
    , index(i)
    , len(l)
    , logicWidth(w)
    // we also need to save things used in OutputDevice::ImplPrepareLayoutArgs(), in case they
    // change in the output device, plus mapMode affects the sizes.
    , font(outputDevice->GetFont())
    , mapMode(outputDevice->GetMapMode())
    , rtl(outputDevice->IsRTLEnabled())
    , layoutMode(outputDevice->GetLayoutMode())
    , digitLanguage(outputDevice->GetDigitLanguage())
{
    hashValue = 0;
    o3tl::hash_combine(hashValue, vcl::text::FirstCharsStringHash()(text));
    o3tl::hash_combine(hashValue, index);
    o3tl::hash_combine(hashValue, len);
    o3tl::hash_combine(hashValue, logicWidth);

    SvMemoryStream stream;
    WriteFont(stream, font);
    o3tl::hash_combine(hashValue, static_cast<const char*>(stream.GetData()), stream.GetSize());
    o3tl::hash_combine(hashValue, mapMode.GetHashValue());
    o3tl::hash_combine(hashValue, rtl);
    o3tl::hash_combine(hashValue, layoutMode);
    o3tl::hash_combine(hashValue, digitLanguage.get());
}

inline bool SalLayoutGlyphsCache::CachedGlyphsKey::operator==(const CachedGlyphsKey& other) const
{
    return hashValue == other.hashValue && index == other.index && len == other.len
           && logicWidth == other.logicWidth && mapMode == other.mapMode && rtl == other.rtl
           && layoutMode == other.layoutMode
           && digitLanguage == other.digitLanguage
           // Need to use EqualIgnoreColor, because sometimes the color changes, but it's irrelevant
           // for text layout (and also obsolete in vcl::Font).
           && font.EqualIgnoreColor(other.font)
           && vcl::text::FastStringCompareEqual()(text, other.text);
    // Slower things last in the comparison.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
