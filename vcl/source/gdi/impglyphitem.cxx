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
#include <unotools/configmgr.hxx>
#include <TextLayoutCache.hxx>
#include <officecfg/Office/Common.hxx>

// These need being explicit because of SalLayoutGlyphsImpl being private in vcl.
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

// Clone, but only glyphs in the given range in the original text string.
// It is possible the given range may not be cloned, in which case this returns nullptr.
SalLayoutGlyphsImpl* SalLayoutGlyphsImpl::cloneCharRange(sal_Int32 index, sal_Int32 length) const
{
    std::unique_ptr<SalLayoutGlyphsImpl> copy(new SalLayoutGlyphsImpl(*GetFont()));
    copy->SetFlags(GetFlags());
    if (empty())
        return copy.release();
    copy->reserve(std::min<size_t>(size(), length));
    sal_Int32 beginPos = index;
    sal_Int32 endPos = index + length;
    const_iterator pos;
    bool rtl = front().IsRTLGlyph();
    if (rtl)
    {
        // Glyphs are in reverse order for RTL.
        beginPos = index + length - 1;
        endPos = index - 1;
        // Skip glyphs that are in the string after the given index, i.e. are before the glyphs
        // we want.
        pos = std::partition_point(
            begin(), end(), [beginPos](const GlyphItem& it) { return it.charPos() > beginPos; });
    }
    else
    {
        // Skip glyphs that are in the string before the given index (glyphs are sorted by charPos()).
        pos = std::partition_point(
            begin(), end(), [beginPos](const GlyphItem& it) { return it.charPos() < beginPos; });
    }
    if (pos == end())
        return nullptr;
    // Require a start at the exact position given, otherwise bail out.
    if (pos->charPos() != beginPos)
        return nullptr;
    // For RTL make sure we're not cutting in the middle of a multi-character glyph
    // (for non-RTL charPos is always the start of a multi-character glyph).
    if (rtl && pos->charPos() + pos->charCount() > beginPos + 1)
        return nullptr;
    // Don't create a subset if it's not safe to break at the beginning or end of the sequence
    // (https://harfbuzz.github.io/harfbuzz-hb-buffer.html#hb-glyph-flags-t).
    if (pos->IsUnsafeToBreak() || (pos->IsInCluster() && !pos->IsClusterStart()))
        return nullptr;
    // LinearPos needs adjusting to start at xOffset/yOffset for the first item,
    // that's how it's computed in GenericSalLayout::LayoutText().
    DevicePoint zeroPoint = pos->linearPos() - DevicePoint(pos->xOffset(), pos->yOffset());
    // Add and adjust all glyphs until the given length.
    // The check is written as 'charPos + charCount <= endPos' rather than 'charPos < endPos'
    // (or similarly for RTL) to make sure we include complete glyphs. If a glyph is composed
    // from several characters, we should not cut in the middle of those characters, so this
    // checks the glyph is entirely in the given character range. If it is not, this will end
    // the loop and the later 'pos->charPos() != endPos' check will fail and bail out.
    // CppunitTest_sw_layoutwriter's testCombiningCharacterCursorPosition would fail without this.
    while (pos != end()
           && (rtl ? pos->charPos() - pos->charCount() >= endPos
                   : pos->charPos() + pos->charCount() <= endPos))
    {
        if (pos->IsRTLGlyph() != rtl)
            return nullptr; // Don't mix RTL and non-RTL runs.
        // HACK: When running CppunitTest_sw_uiwriter3's testTdf104649 on Mac there's glyph
        // with id 1232 that has 0 charCount, 0 origWidth and inconsistent xOffset (sometimes 0,
        // but sometimes not). Possibly font or Harfbuzz bug? It's extremely rare, so simply bail out.
        if (pos->charCount() == 0 && pos->origWidth() == 0)
            return nullptr;
        copy->push_back(*pos);
        copy->back().setLinearPos(copy->back().linearPos() - zeroPoint);
        ++pos;
    }
    if (pos != end())
    {
        // Fail if the next character is not at the expected past-end position. For RTL check
        // that we're not cutting in the middle of a multi-character glyph.
        if (rtl ? pos->charPos() + pos->charCount() != endPos + 1 : pos->charPos() != endPos)
            return nullptr;
        if (pos->IsUnsafeToBreak() || (pos->IsInCluster() && !pos->IsClusterStart()))
            return nullptr;
    }
    // HACK: If mode is se to be RTL, but the last glyph is a non-RTL space,
    // then making a subset would give a different result than the actual layout,
    // because the weak BiDi mode code in ImplLayoutArgs ctor would interpret
    // the string subset ending with space as the space being RTL, but it would
    // treat it as non-RTL for the whole string if there would be more non-RTL
    // characters after the space. So bail out.
    if (GetFlags() & SalLayoutFlags::BiDiRtl && !rtl && !copy->empty() && copy->back().IsSpacing())
    {
        return nullptr;
    }
    return copy.release();
}

#ifdef DBG_UTIL
bool SalLayoutGlyphsImpl::isEqual(const SalLayoutGlyphsImpl* other) const
{
    if (!GetFont()->mxFontMetric->CompareDeviceIndependentFontAttributes(
            *other->GetFont()->mxFontMetric))
        return false;
    if (GetFlags() != other->GetFlags())
        return false;
    if (empty() || other->empty())
        return empty() == other->empty();
    if (size() != other->size())
        return false;
    for (size_t pos = 0; pos < size(); ++pos)
    {
        if ((*this)[pos] != (*other)[pos])
            return false;
    }
    return true;
}
#endif

bool SalLayoutGlyphsImpl::IsValid() const
{
    if (!m_rFontInstance.is())
        return false;
    return true;
}

SalLayoutGlyphsCache* SalLayoutGlyphsCache::self()
{
    static vcl::DeleteOnDeinit<SalLayoutGlyphsCache> cache(
        !utl::ConfigManager::IsFuzzing()
            ? officecfg::Office::Common::Cache::Font::GlyphsCacheSize::get()
            : 20000);
    return cache.get();
}

static SalLayoutGlyphs makeGlyphsSubset(const SalLayoutGlyphs& source,
                                        const OutputDevice* outputDevice, std::u16string_view text,
                                        sal_Int32 index, sal_Int32 len)
{
    SalLayoutGlyphs ret;
    for (int level = 0;; ++level)
    {
        const SalLayoutGlyphsImpl* sourceLevel = source.Impl(level);
        if (sourceLevel == nullptr)
            break;
        SalLayoutGlyphsImpl* cloned = sourceLevel->cloneCharRange(index, len);
        // If the glyphs range cannot be cloned, bail out.
        if (cloned == nullptr)
            return SalLayoutGlyphs();
        // If the entire string is mixed LTR/RTL but the subset is only LTR,
        // then make sure the flags match that, otherwise checkGlyphsEqual()
        // would assert on flags being different.
        cloned->SetFlags(cloned->GetFlags()
                         | outputDevice->GetBiDiLayoutFlags(text, index, index + len));
        // SalLayoutFlags::KashidaJustification is set only if any glyph
        // in the range has GlyphItemFlags::ALLOW_KASHIDA (otherwise unset it).
        if (cloned->GetFlags() & SalLayoutFlags::KashidaJustification)
        {
            bool hasKashida = false;
            for (const GlyphItem& item : *cloned)
            {
                if (item.AllowKashida())
                {
                    hasKashida = true;
                    break;
                }
            }
            if (!hasKashida)
                cloned->SetFlags(cloned->GetFlags() & ~SalLayoutFlags::KashidaJustification);
        }
#ifdef DBG_UTIL
        else
            for (const GlyphItem& item : *cloned)
                assert(!item.AllowKashida());
#endif
        ret.AppendImpl(cloned);
    }
    return ret;
}

#ifdef DBG_UTIL
static void checkGlyphsEqual(const SalLayoutGlyphs& g1, const SalLayoutGlyphs& g2)
{
    for (int level = 0;; ++level)
    {
        const SalLayoutGlyphsImpl* l1 = g1.Impl(level);
        const SalLayoutGlyphsImpl* l2 = g2.Impl(level);
        if (l1 == nullptr || l2 == nullptr)
        {
            assert(l1 == l2);
            break;
        }
        assert(l1->isEqual(l2));
    }
}
#endif

const SalLayoutGlyphs*
SalLayoutGlyphsCache::GetLayoutGlyphs(VclPtr<const OutputDevice> outputDevice, const OUString& text,
                                      sal_Int32 nIndex, sal_Int32 nLen, tools::Long nLogicWidth,
                                      const vcl::text::TextLayoutCache* layoutCache)
{
    if (nLen == 0)
        return nullptr;
    const CachedGlyphsKey key(outputDevice, text, nIndex, nLen, nLogicWidth);
    GlyphsCache::const_iterator it = mCachedGlyphs.find(key);
    if (it != mCachedGlyphs.end())
    {
        if (it->second.IsValid())
            return &it->second;
        // Do not try to create the layout here. If a cache item exists, it's already
        // been attempted and the layout was invalid (this happens with MultiSalLayout).
        // So in that case this is a cached failure.
        return nullptr;
    }
    const SalLayoutFlags glyphItemsOnlyLayout = SalLayoutFlags::GlyphItemsOnly;
    bool resetLastSubstringKey = true;
    const sal_Unicode nbSpace = 0xa0; // non-breaking space
    if (nIndex != 0 || nLen != text.getLength())
    {
        // The glyphs functions are often called first for an entire string
        // and then with an increasing starting index until the end of the string.
        // Which means it's possible to get the glyphs faster by just copying
        // a subset of the full glyphs and adjusting as necessary.
        if (mLastTemporaryKey.has_value() && mLastTemporaryKey == key)
            return &mLastTemporaryGlyphs;
        const CachedGlyphsKey keyWhole(outputDevice, text, 0, text.getLength(), nLogicWidth);
        GlyphsCache::const_iterator itWhole = mCachedGlyphs.find(keyWhole);
        if (itWhole == mCachedGlyphs.end())
        {
            // This function may often be called repeatedly for segments of the same string,
            // in which case it is more efficient to cache glyphs for the entire string
            // and then return subsets of them. So if a second call either starts at the same
            // position or starts at the end of the previous call, cache the entire string.
            // This used to do this only for the first two segments of the string,
            // but that missed the case when the font slightly changed e.g. because of the first
            // part being underlined. Doing this for any two segments allows this optimization
            // even when the prefix of the string would use a different font.
            // TODO: Can those font differences be ignored?
            // Writer layouts tests enable SAL_ABORT_ON_NON_APPLICATION_FONT_USE in order
            // to make PrintFontManager::Substitute() abort if font fallback happens. When
            // laying out the entire string the chance this happens increases (e.g. testAbi11870
            // normally calls this function only for a part of a string, but this optimization
            // lays out the entire string and causes a fallback). Since this optimization
            // does not change result of this function, simply disable it for those tests.
            static bool bAbortOnFontSubstitute
                = getenv("SAL_ABORT_ON_NON_APPLICATION_FONT_USE") != nullptr;
            if (mLastSubstringKey.has_value() && !bAbortOnFontSubstitute)
            {
                sal_Int32 pos = nIndex;
                if (mLastSubstringKey->len < pos && text[pos - 1] == nbSpace)
                    --pos; // Writer skips a non-breaking space, so skip that character too.
                if ((mLastSubstringKey->len == pos || mLastSubstringKey->index == nIndex)
                    && mLastSubstringKey
                           == CachedGlyphsKey(outputDevice, text, mLastSubstringKey->index,
                                              mLastSubstringKey->len, nLogicWidth))
                {
                    GetLayoutGlyphs(outputDevice, text, 0, text.getLength(), nLogicWidth,
                                    layoutCache);
                    itWhole = mCachedGlyphs.find(keyWhole);
                }
                else
                    mLastSubstringKey.reset();
            }
            if (!mLastSubstringKey.has_value())
            {
                mLastSubstringKey = key;
                resetLastSubstringKey = false;
            }
        }
        if (itWhole != mCachedGlyphs.end() && itWhole->second.IsValid())
        {
            mLastSubstringKey.reset();
            mLastTemporaryGlyphs
                = makeGlyphsSubset(itWhole->second, outputDevice, text, nIndex, nLen);
            if (mLastTemporaryGlyphs.IsValid())
            {
                mLastTemporaryKey = std::move(key);
#ifdef DBG_UTIL
                std::shared_ptr<const vcl::text::TextLayoutCache> tmpLayoutCache;
                if (layoutCache == nullptr)
                {
                    tmpLayoutCache = vcl::text::TextLayoutCache::Create(text);
                    layoutCache = tmpLayoutCache.get();
                }
                // Check if the subset result really matches what we would get normally,
                // to make sure corner cases are handled well (see SalLayoutGlyphsImpl::cloneCharRange()).
                std::unique_ptr<SalLayout> layout
                    = outputDevice->ImplLayout(text, nIndex, nLen, Point(0, 0), nLogicWidth, {},
                                               glyphItemsOnlyLayout, layoutCache);
                assert(layout);
                checkGlyphsEqual(mLastTemporaryGlyphs, layout->GetGlyphs());
#endif
                return &mLastTemporaryGlyphs;
            }
        }
    }
    if (resetLastSubstringKey)
    {
        // Writer does non-breaking space differently (not as part of the string), so in that
        // case ignore that call and still allow finding two adjacent substrings that have
        // the non-breaking space between them.
        if (nLen != 1 || text[nIndex] != nbSpace)
            mLastSubstringKey.reset();
    }

    std::shared_ptr<const vcl::text::TextLayoutCache> tmpLayoutCache;
    if (layoutCache == nullptr)
    {
        tmpLayoutCache = vcl::text::TextLayoutCache::Create(text);
        layoutCache = tmpLayoutCache.get();
    }
    std::unique_ptr<SalLayout> layout = outputDevice->ImplLayout(
        text, nIndex, nLen, Point(0, 0), nLogicWidth, {}, glyphItemsOnlyLayout, layoutCache);
    if (layout)
    {
        SalLayoutGlyphs glyphs = layout->GetGlyphs();
        if (glyphs.IsValid())
        {
            // TODO: Fallbacks do not work reliably (fallback font not included in the key),
            // so do not cache (but still return once, using the temporary without a key set).
            if (glyphs.Impl(1) != nullptr)
            {
                mLastTemporaryGlyphs = std::move(glyphs);
                mLastTemporaryKey.reset();
                return &mLastTemporaryGlyphs;
            }
            mCachedGlyphs.insert(std::make_pair(key, layout->GetGlyphs()));
            assert(mCachedGlyphs.find(key)
                   == mCachedGlyphs.begin()); // newly inserted item is first
            return &mCachedGlyphs.begin()->second;
        }
    }
    // Failure, cache it too as invalid glyphs.
    mCachedGlyphs.insert(std::make_pair(key, SalLayoutGlyphs()));
    return nullptr;
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
    // TODO It would be possible to get a better hit ratio if mapMode wasn't part of the key
    // and results that differ only in mapmode would have coordinates adjusted based on that.
    // That would occasionally lead to rounding errors (at least differences that would
    // make checkGlyphsEqual() fail).
    , mapMode(outputDevice->GetMapMode())
    , rtl(outputDevice->IsRTLEnabled())
    , layoutMode(outputDevice->GetLayoutMode())
    , digitLanguage(outputDevice->GetDigitLanguage())
{
    const LogicalFontInstance* fi = outputDevice->GetFontInstance();
    fi->GetScale(&fontScaleX, &fontScaleY);

    hashValue = 0;
    o3tl::hash_combine(hashValue, vcl::text::FirstCharsStringHash()(text));
    o3tl::hash_combine(hashValue, index);
    o3tl::hash_combine(hashValue, len);
    o3tl::hash_combine(hashValue, logicWidth);
    o3tl::hash_combine(hashValue, outputDevice.get());
    // Need to use IgnoreColor, because sometimes the color changes, but it's irrelevant
    // for text layout (and also obsolete in vcl::Font).
    o3tl::hash_combine(hashValue, font.GetHashValueIgnoreColor());
    // For some reason font scale may differ even if vcl::Font is the same,
    // so explicitly check it too.
    o3tl::hash_combine(hashValue, fontScaleX);
    o3tl::hash_combine(hashValue, fontScaleY);
    o3tl::hash_combine(hashValue, mapMode.GetHashValue());
    o3tl::hash_combine(hashValue, rtl);
    o3tl::hash_combine(hashValue, layoutMode);
    o3tl::hash_combine(hashValue, digitLanguage.get());
}

inline bool SalLayoutGlyphsCache::CachedGlyphsKey::operator==(const CachedGlyphsKey& other) const
{
    return hashValue == other.hashValue && index == other.index && len == other.len
           && logicWidth == other.logicWidth && mapMode == other.mapMode && rtl == other.rtl
           && layoutMode == other.layoutMode && digitLanguage == other.digitLanguage
           && fontScaleX == other.fontScaleX && fontScaleY == other.fontScaleY
           && font.EqualIgnoreColor(other.font)
           && vcl::text::FastStringCompareEqual()(text, other.text);
    // Slower things last in the comparison.
}

size_t SalLayoutGlyphsCache::GlyphsCost::operator()(const SalLayoutGlyphs& glyphs) const
{
    size_t cost = 0;
    for (int level = 0;; ++level)
    {
        const SalLayoutGlyphsImpl* impl = glyphs.Impl(level);
        if (impl == nullptr)
            break;
        // Count size in bytes, both the SalLayoutGlyphsImpl instance and contained GlyphItem's.
        cost += sizeof(*impl);
        cost += impl->size() * sizeof(impl->front());
    }
    return cost;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
