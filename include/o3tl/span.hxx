/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_SPAN_HXX
#define INCLUDED_O3TL_SPAN_HXX

#include <sal/config.h>

#if __has_include(<span>)

#include <span>

namespace o3tl { using std::span; }

#else

#include <cassert>
#include <cstddef>
#include <iterator>

namespace o3tl {

/** A barebones approximation of C++20 <span>.
*/
template<typename T>
class span {
public:
    using value_type = T;
    using pointer = value_type *;
    using const_pointer = value_type const *;
    using reference = value_type &;
    using const_reference = value_type const &;
    using const_iterator = const_pointer;
    using iterator = pointer;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using index_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr span() noexcept : data_(nullptr), size_(0) {}

    template<std::size_t N>
    constexpr span (T (&a)[N]) noexcept : data_(a), size_(N) {}

    constexpr span (T *a, index_type len) noexcept
        : data_(a), size_(len)
    {
        // not terribly sure about this, might need to strengthen it
        assert(a != nullptr || len == 0);
    }

    constexpr bool empty() const noexcept { return size_ == 0; }

    constexpr iterator begin() const noexcept { return data_; }
    constexpr iterator end() const noexcept { return begin() + size(); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }

    reverse_iterator rbegin() const noexcept
    { return reverse_iterator(end()); }
    reverse_iterator rend() const noexcept
    { return reverse_iterator(begin()); }

    constexpr const_reverse_iterator crbegin() const noexcept
    { return rbegin(); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    constexpr index_type size() const noexcept { return size_; }

    constexpr reference operator [](index_type pos) const {
        assert(pos < size());
        return data_[pos];
    }

    constexpr pointer data() const noexcept { return data_; }

private:
    pointer    data_;
    index_type size_;
};

} // namespace o3tl

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
