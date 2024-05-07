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
#include <utility>
#include <vcl/glyphitemcache.hxx>
#include <vcl/vcllayout.hxx>
#include <vcl/lazydelete.hxx>
#include <tools/stream.hxx>
#include <unotools/configmgr.hxx>
#include <TextLayoutCache.hxx>
#include <officecfg/Office/Common.hxx>
#include <o3tl/string_view.hxx>

#include <unicode/ubidi.h>
#include <unicode/uchar.h>

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
    bool rtl = front().IsRTLGlyph();
    // Avoid mixing LTR/RTL or layouts that do not have it set explicitly (BiDiStrong). Otherwise
    // the subset may not quite match what would a real layout call give (e.g. some characters with neutral
    // direction such as space might have different LTR/RTL flag). It seems bailing out here mostly
    // avoid relatively rare corner cases and doesn't matter for performance.
    // This is also checked in SalLayoutGlyphsCache::GetLayoutGlyphs() below.
    if (!(GetFlags() & SalLayoutFlags::BiDiStrong)
        || rtl != bool(GetFlags() & SalLayoutFlags::BiDiRtl))
        return nullptr;
    copy->reserve(std::min<size_t>(size(), length));
    sal_Int32 beginPos = index;
    sal_Int32 endPos = index + length;
    const_iterator pos;
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
    // For RTL make sure we're not cutting in the middle of a multi-character glyph,
    // or in the middle of a cluster
    // (for non-RTL charPos is always the start of a multi-character glyph).
    if (rtl && (pos->charPos() + pos->charCount() > beginPos + 1 || pos->IsInCluster()))
        return nullptr;
    if (!isSafeToBreak(pos, rtl))
        return nullptr;
    // LinearPos needs adjusting to start at xOffset/yOffset for the first item,
    // that's how it's computed in GenericSalLayout::LayoutText().
    basegfx::B2DPoint zeroPoint
        = pos->linearPos() - basegfx::B2DPoint(pos->xOffset(), pos->yOffset());
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
        if (!isSafeToBreak(pos, rtl))
            return nullptr;
    }
    return copy.release();
}

bool SalLayoutGlyphsImpl::isSafeToBreak(const_iterator pos, bool rtl) const
{
    if (rtl)
    {
        // RTL is more complicated, because HB_GLYPH_FLAG_UNSAFE_TO_BREAK talks about beginning
        // of a cluster, which refers to the text, not glyphs. This function is called
        // for the first glyph of the subset and the first glyph after the subset, but since
        // the glyphs are backwards, and we need the beginning of cluster at the start of the text
        // and beginning of the cluster after the text, we need to check glyphs before this position.
        if (pos == begin())
            return true;
        --pos;
    }
    // Don't create a subset if it's not safe to break at the beginning or end of the sequence
    // (https://harfbuzz.github.io/harfbuzz-hb-buffer.html#hb-glyph-flags-t).
    if (pos->IsUnsafeToBreak() || (pos->IsInCluster() && !pos->IsClusterStart()))
        return false;
    return true;
}

#ifdef DBG_UTIL
bool SalLayoutGlyphsImpl::isLayoutEquivalent(const SalLayoutGlyphsImpl* other) const
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
        if (!(*this)[pos].isLayoutEquivalent((*other)[pos]))
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

void SalLayoutGlyphsCache::clear() { mCachedGlyphs.clear(); }

SalLayoutGlyphsCache* SalLayoutGlyphsCache::self()
{
    static vcl::DeleteOnDeinit<SalLayoutGlyphsCache> cache(
        !comphelper::IsFuzzing() ? officecfg::Office::Common::Cache::Font::GlyphsCacheSize::get()
                                 : 20000000);
    return cache.get();
}

static UBiDiDirection getBiDiDirection(std::u16string_view text, sal_Int32 index, sal_Int32 len)
{
    // Return whether all character are LTR, RTL, neutral or whether it's mixed.
    // This is sort of ubidi_getBaseDirection() and ubidi_getDirection(),
    // but it's meant to be fast but also check all characters.
    sal_Int32 end = index + len;
    UBiDiDirection direction = UBIDI_NEUTRAL;
    while (index < end)
    {
        switch (u_charDirection(o3tl::iterateCodePoints(text, &index)))
        {
            // Only characters with strong direction.
            case U_LEFT_TO_RIGHT:
                if (direction == UBIDI_RTL)
                    return UBIDI_MIXED;
                direction = UBIDI_LTR;
                break;
            case U_RIGHT_TO_LEFT:
            case U_RIGHT_TO_LEFT_ARABIC:
                if (direction == UBIDI_LTR)
                    return UBIDI_MIXED;
                direction = UBIDI_RTL;
                break;
            default:
                break;
        }
    }
    return direction;
}

static SalLayoutGlyphs makeGlyphsSubset(const SalLayoutGlyphs& source,
                                        const OutputDevice* outputDevice, std::u16string_view text,
                                        sal_Int32 index, sal_Int32 len)
{
    // tdf#149264: We need to check if the text is LTR, RTL or mixed. Apparently
    // harfbuzz doesn't give reproducible results (or possibly HB_GLYPH_FLAG_UNSAFE_TO_BREAK
    // is not reliable?) when asked to lay out RTL text as LTR. So require that the whole
    // subset ir either LTR or RTL.
    UBiDiDirection direction = getBiDiDirection(text, index, len);
    if (direction == UBIDI_MIXED)
        return SalLayoutGlyphs();
    SalLayoutGlyphs ret;
    for (int level = 0;; ++level)
    {
        const SalLayoutGlyphsImpl* sourceLevel = source.Impl(level);
        if (sourceLevel == nullptr)
            break;
        bool sourceRtl = bool(sourceLevel->GetFlags() & SalLayoutFlags::BiDiRtl);
        if ((direction == UBIDI_LTR && sourceRtl) || (direction == UBIDI_RTL && !sourceRtl))
            return SalLayoutGlyphs();
        SalLayoutGlyphsImpl* cloned = sourceLevel->cloneCharRange(index, len);
        // If the glyphs range cannot be cloned, bail out.
        if (cloned == nullptr)
            return SalLayoutGlyphs();
        // If the entire string is mixed LTR/RTL but the subset is only LTR,
        // then make sure the flags match that, otherwise checkGlyphsEqual()
        // would assert on flags being different.
        cloned->SetFlags(cloned->GetFlags()
                         | outputDevice->GetBiDiLayoutFlags(text, index, index + len));
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
        assert(l1->isLayoutEquivalent(l2));
    }
}
#endif

const SalLayoutGlyphs* SalLayoutGlyphsCache::GetLayoutGlyphs(
    const VclPtr<const OutputDevice>& outputDevice, const OUString& text, sal_Int32 nIndex,
    sal_Int32 nLen, tools::Long nLogicWidth, const vcl::text::TextLayoutCache* layoutCache)
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
    bool resetLastSubstringKey = true;
    const sal_Unicode nbSpace = 0xa0; // non-breaking space
    // SalLayoutGlyphsImpl::cloneCharRange() requires BiDiStrong, so if not set, do not even try.
    bool skipGlyphSubsets
        = !(outputDevice->GetLayoutMode() & vcl::text::ComplexTextLayoutFlags::BiDiStrong);
    if ((nIndex != 0 || nLen != text.getLength()) && !skipGlyphSubsets)
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
            // Writer layouts tests enable SAL_NON_APPLICATION_FONT_USE=abort in order
            // to make PrintFontManager::Substitute() abort if font fallback happens. When
            // laying out the entire string the chance this happens increases (e.g. testAbi11870
            // normally calls this function only for a part of a string, but this optimization
            // lays out the entire string and causes a fallback). Since this optimization
            // does not change result of this function, simply disable it for those tests.
            static const bool bAbortOnFontSubstitute = [] {
                const char* pEnv = getenv("SAL_NON_APPLICATION_FONT_USE");
                return pEnv && strcmp(pEnv, "abort") == 0;
            }();
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
                mLastTemporaryKey = key;
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
                    = outputDevice->ImplLayout(text, nIndex, nLen, Point(0, 0), nLogicWidth, {}, {},
                                               SalLayoutFlags::GlyphItemsOnly, layoutCache);
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
    std::unique_ptr<SalLayout> layout
        = outputDevice->ImplLayout(text, nIndex, nLen, Point(0, 0), nLogicWidth, {}, {},
                                   SalLayoutFlags::GlyphItemsOnly, layoutCache);
    if (layout)
    {
        SalLayoutGlyphs glyphs = layout->GetGlyphs();
        if (glyphs.IsValid())
        {
            // TODO: Fallbacks do not work reliably (fallback font not included in the key),
            // so do not cache (but still return once, using the temporary without a key set).
            if (!mbCacheGlyphsWhenDoingFallbackFonts && glyphs.Impl(1) != nullptr)
            {
                mLastTemporaryGlyphs = std::move(glyphs);
                mLastTemporaryKey.reset();
                return &mLastTemporaryGlyphs;
            }
            mCachedGlyphs.insert(std::make_pair(key, std::move(glyphs)));
            assert(mCachedGlyphs.find(key)
                   == mCachedGlyphs.begin()); // newly inserted item is first
            return &mCachedGlyphs.begin()->second;
        }
    }
    // Failure, cache it too as invalid glyphs.
    mCachedGlyphs.insert(std::make_pair(key, SalLayoutGlyphs()));
    return nullptr;
}

const SalLayoutGlyphs* SalLayoutGlyphsCache::GetLayoutGlyphs(
    const VclPtr<const OutputDevice>& outputDevice, const OUString& text, sal_Int32 nIndex,
    sal_Int32 nLen, sal_Int32 nDrawMinCharPos, sal_Int32 nDrawEndCharPos, tools::Long nLogicWidth,
    const vcl::text::TextLayoutCache* layoutCache)
{
    // This version is used by callers that need to draw a subset of a layout. In all ordinary uses
    // this function will be called for successive glyph subsets, so should optimize for that case.
    auto* pWholeGlyphs
        = GetLayoutGlyphs(outputDevice, text, nIndex, nLen, nLogicWidth, layoutCache);
    if (nDrawMinCharPos <= nIndex && nDrawEndCharPos >= (nIndex + nLen))
    {
        return pWholeGlyphs;
    }

    if (pWholeGlyphs && pWholeGlyphs->IsValid())
    {
        mLastTemporaryKey.reset();
        mLastTemporaryGlyphs = makeGlyphsSubset(*pWholeGlyphs, outputDevice, text, nDrawMinCharPos,
                                                nDrawEndCharPos - nDrawMinCharPos);
        if (mLastTemporaryGlyphs.IsValid())
        {
            return &mLastTemporaryGlyphs;
        }
    }

    return nullptr;
}

void SalLayoutGlyphsCache::SetCacheGlyphsWhenDoingFallbackFonts(bool bOK)
{
    mbCacheGlyphsWhenDoingFallbackFonts = bOK;
    if (!bOK)
        clear();
}

SalLayoutGlyphsCache::CachedGlyphsKey::CachedGlyphsKey(
    const VclPtr<const OutputDevice>& outputDevice, OUString t, sal_Int32 i, sal_Int32 l,
    tools::Long w)
    : text(std::move(t))
    , index(i)
    , len(l)
    , logicWidth(w)
    // we also need to save things used in OutputDevice::ImplPrepareLayoutArgs(), in case they
    // change in the output device, plus mapMode affects the sizes.
    , fontMetric(outputDevice->GetFontMetric())
    // TODO It would be possible to get a better hit ratio if mapMode wasn't part of the key
    // and results that differ only in mapmode would have coordinates adjusted based on that.
    // That would occasionally lead to rounding errors (at least differences that would
    // make checkGlyphsEqual() fail).
    , mapMode(outputDevice->GetMapMode())
    , digitLanguage(outputDevice->GetDigitLanguage())
    , layoutMode(outputDevice->GetLayoutMode())
    , rtl(outputDevice->IsRTLEnabled())
{
    const LogicalFontInstance* fi = outputDevice->GetFontInstance();
    fi->GetScale(&fontScaleX, &fontScaleY);

    const vcl::font::FontSelectPattern& rFSD = fi->GetFontSelectPattern();
    disabledLigatures = rFSD.GetPitch() == PITCH_FIXED;
    artificialItalic = fi->NeedsArtificialItalic();
    artificialBold = fi->NeedsArtificialBold();

    hashValue = 0;
    o3tl::hash_combine(hashValue, vcl::text::FirstCharsStringHash()(text));
    o3tl::hash_combine(hashValue, index);
    o3tl::hash_combine(hashValue, len);
    o3tl::hash_combine(hashValue, logicWidth);
    o3tl::hash_combine(hashValue, outputDevice.get());
    // Need to use IgnoreColor, because sometimes the color changes, but it's irrelevant
    // for text layout (and also obsolete in vcl::Font).
    o3tl::hash_combine(hashValue, fontMetric.GetHashValueIgnoreColor());
    // For some reason font scale may differ even if vcl::Font is the same,
    // so explicitly check it too.
    o3tl::hash_combine(hashValue, fontScaleX);
    o3tl::hash_combine(hashValue, fontScaleY);
    o3tl::hash_combine(hashValue, mapMode.GetHashValue());
    o3tl::hash_combine(hashValue, rtl);
    o3tl::hash_combine(hashValue, disabledLigatures);
    o3tl::hash_combine(hashValue, artificialItalic);
    o3tl::hash_combine(hashValue, artificialBold);
    o3tl::hash_combine(hashValue, layoutMode);
    o3tl::hash_combine(hashValue, digitLanguage.get());
}

inline bool SalLayoutGlyphsCache::CachedGlyphsKey::operator==(const CachedGlyphsKey& other) const
{
    return hashValue == other.hashValue && index == other.index && len == other.len
           && logicWidth == other.logicWidth && mapMode == other.mapMode && rtl == other.rtl
           && disabledLigatures == other.disabledLigatures
           && artificialItalic == other.artificialItalic && artificialBold == other.artificialBold
           && layoutMode == other.layoutMode && digitLanguage == other.digitLanguage
           && fontScaleX == other.fontScaleX && fontScaleY == other.fontScaleY
           && fontMetric.EqualIgnoreColor(other.fontMetric)
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
