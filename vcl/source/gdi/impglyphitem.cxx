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
#include <tools/stream.hxx>
#include <TextLayoutCache.hxx>

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

const SalLayoutGlyphs*
SalLayoutGlyphsCache::GetLayoutGlyphs(VclPtr<const OutputDevice> outputDevice, const OUString& text,
                                      sal_Int32 nIndex, sal_Int32 nLen, const Point& rLogicPos,
                                      tools::Long nLogicWidth,
                                      const vcl::text::TextLayoutCache* layoutCache) const
{
    if (nLen == 0)
        return nullptr;
    const CachedGlyphsKey key(outputDevice, text, nIndex, nLen, rLogicPos, nLogicWidth);
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
    std::unique_ptr<SalLayout> layout
        = outputDevice->ImplLayout(text, nIndex, nLen, rLogicPos, nLogicWidth, {},
                                   SalLayoutFlags::GlyphItemsOnly, layoutCache);
    if (layout)
    {
        mCachedGlyphs.insert(std::make_pair(key, layout->GetGlyphs()));
        assert(mCachedGlyphs.find(key) == mCachedGlyphs.begin()); // newly inserted item is first
        return &mCachedGlyphs.begin()->second;
    }
    return nullptr;
}

SalLayoutGlyphsCache::CachedGlyphsKey::CachedGlyphsKey(const VclPtr<const OutputDevice>& d,
                                                       const OUString& t, sal_Int32 i, sal_Int32 l,
                                                       const Point& p, tools::Long w)
    : outputDevice(d)
    , text(t)
    , index(i)
    , len(l)
    , logicPos(p)
    , logicWidth(w)
{
    hashValue = 0;
    o3tl::hash_combine(hashValue, outputDevice.get());
    SvMemoryStream stream;
    WriteFont(stream, outputDevice->GetFont());
    o3tl::hash_combine(hashValue, static_cast<const char*>(stream.GetData()), stream.GetSize());
    o3tl::hash_combine(hashValue, vcl::text::FirstCharsStringHash()(text));
    o3tl::hash_combine(hashValue, index);
    o3tl::hash_combine(hashValue, len);
    o3tl::hash_combine(hashValue, logicPos.X());
    o3tl::hash_combine(hashValue, logicPos.Y());
    o3tl::hash_combine(hashValue, logicWidth);
}

inline bool SalLayoutGlyphsCache::CachedGlyphsKey::operator==(const CachedGlyphsKey& other) const
{
    return hashValue == other.hashValue && outputDevice == other.outputDevice
           && index == other.index && len == other.len && logicPos == other.logicPos
           && logicWidth == other.logicWidth
           && vcl::text::FastStringCompareEqual()(text, other.text);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
