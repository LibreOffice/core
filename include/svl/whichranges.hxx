/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include <sal/types.h>
#include <svl/svldllapi.h>
#include <o3tl/sorted_vector.hxx>
#include <array>
#include <memory>
#include <cassert>

typedef std::pair<sal_uInt16, sal_uInt16> WhichPair;

namespace svl
{
namespace detail
{
constexpr bool validRange(sal_uInt16 wid1, sal_uInt16 wid2) { return wid1 != 0 && wid1 <= wid2; }

constexpr bool validGap(sal_uInt16 wid1, sal_uInt16 wid2) { return wid2 > wid1; }

template <sal_uInt16 WID1, sal_uInt16 WID2> constexpr bool validRanges()
{
    return validRange(WID1, WID2);
}

template <sal_uInt16 WID1, sal_uInt16 WID2, sal_uInt16 WID3, sal_uInt16... WIDs>
constexpr bool validRanges()
{
    return validRange(WID1, WID2) && validGap(WID2, WID3) && validRanges<WID3, WIDs...>();
}

// The calculations in rangeSize cannot overflow, assuming
// std::size_t is no smaller than sal_uInt16:
constexpr std::size_t rangeSize(sal_uInt16 wid1, sal_uInt16 wid2)
{
    assert(validRange(wid1, wid2));
    return wid2 - wid1 + 1;
}
}

template <sal_uInt16... WIDs> struct Items_t
{
    using Array = std::array<WhichPair, sizeof...(WIDs) / 2>;
    template <sal_uInt16 WID1, sal_uInt16 WID2, sal_uInt16... Rest>
    static constexpr void fill(typename Array::iterator it)
    {
        it->first = WID1;
        it->second = WID2;
        if constexpr (sizeof...(Rest) > 0)
            fill<Rest...>(++it);
    }
    static constexpr Array make()
    {
        assert(svl::detail::validRanges<WIDs...>());
        Array a{};
        fill<WIDs...>(a.begin());
        return a;
    }
    // This is passed to WhichRangesContainer so we can avoid needing to malloc()
    // for compile-time data.
    static constexpr Array value = make();
};

template <sal_uInt16... WIDs> inline static constexpr auto Items = Items_t<WIDs...>{};
}

/**
 * Most of the time, the which ranges we point at are a compile-time literal.
 * So we take advantage of that, and avoid the cost of allocating our own array and copying into it.
 */
struct SVL_DLLPUBLIC WhichRangesContainer
{
    using const_iterator = WhichPair const*;

    WhichPair const* m_pairs = nullptr;
    sal_Int32 m_size = 0;
    /** if true, we allocated and need to delete the pairs, if not, we are pointing
      * at a global const literal */
    bool m_bOwnRanges = false;

    WhichRangesContainer() = default;

    WhichRangesContainer(std::unique_ptr<WhichPair[]> wids, sal_Int32 nSize)
        : m_pairs(wids.release())
        , m_size(nSize)
        , m_bOwnRanges(true)
    {
    }
    template <sal_uInt16... WIDs>
    WhichRangesContainer(svl::Items_t<WIDs...>)
        : m_pairs(svl::Items_t<WIDs...>::value.data())
        , m_size(svl::Items_t<WIDs...>::value.size())
        , m_bOwnRanges(false)
    {
    }
    WhichRangesContainer(const WhichPair* wids, sal_Int32 nSize);
    WhichRangesContainer(sal_uInt16 nWhichStart, sal_uInt16 nWhichEnd);
    WhichRangesContainer(WhichRangesContainer const& other) { operator=(other); }
    WhichRangesContainer(WhichRangesContainer&& other);
    /// For when we need to build the container on the fly
    WhichRangesContainer(const o3tl::sorted_vector<sal_uInt16>&);
    ~WhichRangesContainer();

    WhichRangesContainer& operator=(WhichRangesContainer&& other);
    WhichRangesContainer& operator=(WhichRangesContainer const& other);

    bool operator==(WhichRangesContainer const& other) const;
    const_iterator begin() const noexcept { return m_pairs; }
    const_iterator end() const noexcept { return begin() + size(); }
    bool empty() const noexcept { return m_size == 0; }
    sal_Int32 size() const noexcept { return m_size; }
    WhichPair const& operator[](sal_Int32 idx) const noexcept
    {
        assert(idx >= 0 && idx < size() && "index out of range");
        return m_pairs[idx];
    }
    void reset();

    // Adds a range to which ranges, keeping the ranges in valid state (sorted, non-overlapping)
    SAL_WARN_UNUSED_RESULT WhichRangesContainer MergeRange(sal_uInt16 nFrom, sal_uInt16 nTo) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
