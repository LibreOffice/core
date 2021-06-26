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
#include <memory>
#include <svl/svldllapi.h>

typedef std::pair<sal_uInt16, sal_uInt16> WhichPair;

template <std::size_t N> struct WhichRangesLiteral
{
    WhichRangesLiteral(WhichPair const (&wids)[N])
    {
        for (std::size_t i = 0; i != N; ++i)
        {
            ranges[i] = wids[i];
        }
    }
    WhichPair ranges[N];
};

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

    WhichRangesContainer() {}

    template <std::size_t N>
    WhichRangesContainer(WhichRangesLiteral<N> const& literal)
        : m_pairs(literal.ranges)
        , m_size(static_cast<sal_Int32>(N))
        , m_bOwnRanges(false)
    {
    }
    WhichRangesContainer(std::unique_ptr<WhichPair[]> wids, sal_Int32 nSize)
        : m_pairs(wids.release())
        , m_size(nSize)
        , m_bOwnRanges(true)
    {
    }
    WhichRangesContainer(const WhichPair* wids, sal_Int32 nSize);
    WhichRangesContainer(sal_uInt16 nWhichStart, sal_uInt16 nWhichEnd);
    WhichRangesContainer(WhichRangesContainer const& other) { operator=(other); }
    WhichRangesContainer(WhichRangesContainer&& other);
    ~WhichRangesContainer();

    WhichRangesContainer& operator=(WhichRangesContainer&& other);
    WhichRangesContainer& operator=(WhichRangesContainer const& other);

    bool operator==(WhichRangesContainer const& other) const;
    const_iterator begin() const noexcept { return m_pairs; }
    const_iterator end() const noexcept { return begin() + size(); }
    bool empty() const noexcept { return m_size == 0; }
    sal_Int32 size() const noexcept { return m_size; }
    WhichPair const& operator[](sal_Int32 idx) const noexcept { return m_pairs[idx]; }
    void reset();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
