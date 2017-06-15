/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_ARRAY_VIEW_HXX
#define INCLUDED_O3TL_ARRAY_VIEW_HXX

#include <sal/config.h>

#include <algorithm>
#include <cstddef>
#include <ios>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <config_global.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

// A barebones approximation of C++17(?) <array_view>, haven't bothered with more than single-dimensional arrays

#if HAVE_CXX14_CONSTEXPR
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif

namespace o3tl {

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4814) // in C++14 'constexpr' will not imply 'const'
#endif

template<typename T>
class array_view {
public:
    using value_type = T;
    using pointer = value_type *;
    using const_pointer = value_type const *;
    using reference = value_type &;
    using const_reference = value_type const &;
    using const_iterator = const_pointer;
    using iterator = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    static constexpr size_type npos = size_type(-1);

    constexpr array_view() noexcept : data_(nullptr), size_(0) {}

    template<size_type N>
    CONSTEXPR array_view (T const (&a)[N]) noexcept : data_(a), size_(N) {}

    CONSTEXPR array_view (T const *a, size_type len) noexcept
        : data_(a), size_(len)
    {
#if HAVE_CXX14_CONSTEXPR
        // not terribly sure about this, might need to relax it
        assert((a == nullptr && len == 0) || (a != nullptr && len > 0));
#endif
    }

    constexpr bool            empty() const noexcept { return size_ == 0; }

    constexpr const_iterator begin() const noexcept { return data_; }
    constexpr const_iterator end() const noexcept { return begin() + size(); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }

    constexpr const_reverse_iterator rbegin() const noexcept
    { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator rend() const noexcept
    { return const_reverse_iterator(begin()); }

    constexpr const_reverse_iterator crbegin() const noexcept
    { return rbegin(); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type length() const noexcept { return size(); }

#if !defined __clang__ || HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    size_type max_size() const noexcept {
#if defined __clang__ // avoid constexpr issues with other, older compilers
        (void) this; // loplugin:staticmethods
#endif
        return npos - 1;
    }

    constexpr const_reference operator [](size_type pos) const {
#if HAVE_CXX14_CONSTEXPR
        assert(pos < size());
#endif
        return data_[pos];
    }

    CONSTEXPR
    const_reference at(size_type pos) const {
        if (pos >= size()) {
            throw std::out_of_range("o3tl::array_view::at");
        }
        return operator [](pos);
    }

    constexpr const_reference front() const {
#if HAVE_CXX14_CONSTEXPR
        assert(!empty());
#endif
        return operator [](0);
    }

    constexpr const_reference back() const {
#if HAVE_CXX14_CONSTEXPR
        assert(!empty());
#endif
        return operator [](size() - 1);
    }

    constexpr const_pointer data() const noexcept { return data_; }

    CONSTEXPR void swap(array_view & s) noexcept {
        std::swap(data_, s.data_);
        std::swap(size_, s.size_);
    }

private:
    const_pointer data_;
    size_type     size_;
};


#if defined _MSC_VER
#pragma warning(pop)
#endif

} // namespace o3tl

namespace std {

template<typename T>
struct hash<o3tl::array_view<T>> {
    std::size_t operator()(o3tl::array_view<T> s)
    { return hash<T[]>()(s.data(), s.size()); }
};

} // namespace std


#undef CONSTEXPR

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
