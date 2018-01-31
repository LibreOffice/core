/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_STRING_VIEW_HXX
#define INCLUDED_O3TL_STRING_VIEW_HXX

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

// An approximation of C++17 <string_view>, including implicit conversion
// from rtl::OString and rtl::OUString.

namespace o3tl {

namespace detail {

template<typename T> struct CharPtrDetector {
    static constexpr bool ok = false;
};

template<> struct CharPtrDetector<char *> {
    static constexpr bool ok = true;
};

template<> struct CharPtrDetector<char const *> {
    static constexpr bool ok = true;
};

template<typename T> struct NonConstCharArrayDetector {
    static constexpr bool ok = false;
};

template<std::size_t N> struct NonConstCharArrayDetector<char [N]> {
    static constexpr bool ok = true;
};

template<typename T> struct ConstCharArrayDetector {
    static constexpr bool ok = false;
};

template<std::size_t N> struct ConstCharArrayDetector<char const[N]> {
    static constexpr bool ok = true;
    static constexpr std::size_t length = N - 1;
};

template<typename T> struct Char16PtrDetector {
    static constexpr bool ok = false;
};

template<> struct Char16PtrDetector<char16_t *> {
    static constexpr bool ok = true;
};

template<> struct Char16PtrDetector<char16_t const *> {
    static constexpr bool ok = true;
};

template<typename T> struct NonConstChar16ArrayDetector {
    static constexpr bool ok = false;
};

template<std::size_t N> struct NonConstChar16ArrayDetector<char16_t [N]> {
    static constexpr bool ok = true;
};

template<typename T> struct ConstChar16ArrayDetector {
    static constexpr bool ok = false;
};

template<std::size_t N> struct ConstChar16ArrayDetector<char16_t const[N]> {
    static constexpr bool ok = true;
    static constexpr std::size_t length = N - 1;
};

template<typename T> struct Char32PtrDetector {
    static constexpr bool ok = false;
};

template<> struct Char32PtrDetector<char32_t *> {
    static constexpr bool ok = true;
};

template<> struct Char32PtrDetector<char32_t const *> {
    static constexpr bool ok = true;
};

template<typename T> struct NonConstChar32ArrayDetector {
    static constexpr bool ok = false;
};

template<std::size_t N> struct NonConstChar32ArrayDetector<char32_t [N]> {
    static constexpr bool ok = true;
};

template<typename T> struct ConstChar32ArrayDetector {
    static constexpr bool ok = false;
};

template<std::size_t N> struct ConstChar32ArrayDetector<char32_t const[N]> {
    static constexpr bool ok = true;
    static constexpr std::size_t length = N - 1;
};

template<typename T> struct WcharPtrDetector {
    static constexpr bool ok = false;
};

template<> struct WcharPtrDetector<wchar_t *> {
    static constexpr bool ok = true;
};

template<> struct WcharPtrDetector<wchar_t const *> {
    static constexpr bool ok = true;
};

template<typename T> struct NonConstWcharArrayDetector {
    static constexpr bool ok = false;
};

template<std::size_t N> struct NonConstWcharArrayDetector<wchar_t [N]> {
    static constexpr bool ok = true;
};

template<typename T> struct ConstWcharArrayDetector {
    static constexpr bool ok = false;
};

template<std::size_t N> struct ConstWcharArrayDetector<wchar_t const[N]> {
    static constexpr bool ok = true;
    static constexpr std::size_t length = N - 1;
};

}

template<typename charT, typename traits = std::char_traits<charT>>
class basic_string_view {
public:
    using traits_type = traits;
    using value_type = charT;
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

    constexpr basic_string_view() noexcept: data_(nullptr), size_(0) {}

    constexpr basic_string_view(basic_string_view const &) noexcept = default;

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    basic_string_view & operator =(basic_string_view const & other) noexcept
#if defined _MSC_VER && _MSC_VER <= 1900 && !defined __clang__
    {
        data_ = other.data_;
        size_ = other.size_;
        return *this;
    }
#else
        = default;
#endif

    // The various character types are handled below in the "LO specifics, to
    // make up for traits::length not necessarily being constexpr yet for
    // literal arguments" section:
    template<typename T = charT> constexpr basic_string_view(
        charT const * str,
        typename std::enable_if<
            !(std::is_same<T, char>::value || std::is_same<T, char16_t>::value
              || std::is_same<T, char32_t>::value
              || std::is_same<T, wchar_t>::value),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(str), size_(traits::length(str)) {}

    constexpr basic_string_view(charT const * str, size_type len):
        data_(str), size_(len) {}

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

    constexpr bool empty() const noexcept { return size_ == 0; }

    constexpr const_reference operator [](size_type pos) const {
#if HAVE_CXX14_CONSTEXPR
        assert(pos < size());
#endif
        return data_[pos];
    }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    const_reference at(size_type pos) const {
        if (pos >= size()) {
            throw std::out_of_range("o3tl::basic_string_view::at");
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

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    void remove_prefix(size_type n) {
        assert(n <= size());
        data_ += n;
        size_ -= n;
    }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    void remove_suffix(size_type n) {
        assert(n <= size());
        size_ -= n;
    }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    void swap(basic_string_view & s) noexcept {
        std::swap(data_, s.data_);
        std::swap(size_, s.size_);
    }

    size_type copy(charT * s, size_type n, size_type pos = 0) const {
        if (pos > size()) {
            throw std::out_of_range("o3tl::basic_string_view::copy");
        }
        auto rlen = std::min(n, size_type(size() - pos));
        traits::copy(s, data() + pos, rlen);
        return rlen;
    }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    basic_string_view substr(size_type pos = 0, size_type n = npos) const {
        if (pos > size()) {
            throw std::out_of_range("o3tl::basic_string_view::copy");
        }
        return basic_string_view(
            data() + pos, std::min(n, size_type(size() - pos)));
    }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    int compare(basic_string_view s) const noexcept {
        auto n = traits::compare(data(), s.data(), std::min(size(), s.size()));
        return n == 0
            ? (size() < s.size() ? -1 : size() == s.size() ? 0 : 1) : n;
    }

    constexpr int compare(size_type pos1, size_type n1, basic_string_view s)
        const
    { return substr(pos1, n1).compare(s); }

    constexpr int compare(
        size_type pos1, size_type n1, basic_string_view s, size_type pos2,
        size_type n2) const
    { return substr(pos1, n1).compare(s.substr(pos2, n2)); }

    constexpr int compare(charT const * s) const
    { return compare(basic_string_view(s)); }

    constexpr int compare(size_type pos1, size_type n1, charT const * s) const
    { return substr(pos1, n1).compare(s); }

    constexpr int compare(
        size_type pos1, size_type n1, charT const * s, size_type n2) const
    { return substr(pos1, n1).compare(basic_string_view(s, n2)); }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    size_type find(basic_string_view s, size_type pos = 0) const noexcept {
        if (s.size() <= size()) {
            for (auto xpos = pos; xpos <= size() - s.size(); ++xpos) {
                bool match = true;
                for (size_type i = 0; i != s.size(); ++i) {
                    if (!traits::eq(data_[xpos + i], s.data_[i])) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    return xpos;
                }
            }
        }
        return npos;
    }

    constexpr size_type find(charT c, size_type pos = 0) const noexcept
    { return find(basic_string_view(&c, 1), pos); }

    constexpr size_type find(charT const * s, size_type pos, size_type n) const
    { return find(basic_string_view(s, n), pos); }

    constexpr size_type find(charT const * s, size_type pos = 0) const
    { return find(basic_string_view(s), pos); }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    size_type rfind(basic_string_view s, size_type pos = npos) const noexcept {
        if (s.size() <= size()) {
            for (auto xpos = std::min<size_type>(size() - s.size(), pos);;
                 --xpos)
            {
                bool match = true;
                for (size_type i = 0; i != s.size(); ++i) {
                    if (!traits::eq(data_[xpos + i], s.data_[i])) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    return xpos;
                }
                if (xpos == 0) {
                    break;
                }
            }
        }
        return npos;
    }

    constexpr size_type rfind(charT c, size_type pos = npos) const noexcept
    { return rfind(basic_string_view(&c, 1), pos); }

    constexpr size_type rfind(charT const * s, size_type pos, size_type n) const
    { return rfind(basic_string_view(s, n), pos); }

    constexpr size_type rfind(charT const * s, size_type pos = npos) const
    { return rfind(basic_string_view(s), pos); }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    size_type find_first_of(basic_string_view s, size_type pos = 0) const
        noexcept
    {
        for (auto xpos = pos; xpos < size(); ++xpos) {
            for (size_type i = 0; i != s.size(); ++i) {
                if (traits::eq(data_[xpos], s.data_[i])) {
                    return xpos;
                }
            }
        }
        return npos;
    }

    constexpr size_type find_first_of(charT c, size_type pos = 0) const noexcept
    { return find_first_of(basic_string_view(&c, 1), pos); }

    constexpr size_type find_first_of(
        charT const * s, size_type pos, size_type n) const
    { return find_first_of(basic_string_view(s, n), pos); }

    constexpr size_type find_first_of(charT const * s, size_type pos = 0) const
    { return find_first_of(basic_string_view(s), pos); }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    size_type find_last_of(basic_string_view s, size_type pos = npos) const
        noexcept
    {
        if (!empty()) {
            for (auto xpos = std::min<size_type>(size() - 1, pos);; --xpos) {
                for (size_type i = 0; i != s.size(); ++i) {
                    if (traits::eq(data_[xpos], s.data_[i])) {
                        return xpos;
                    }
                }
                if (xpos == 0) {
                    break;
                }
            }
        }
        return npos;
    }

    constexpr size_type find_last_of(charT c, size_type pos = npos) const
        noexcept
    { return find_last_of(basic_string_view(&c, 1), pos); }

    constexpr size_type find_last_of(
        charT const * s, size_type pos, size_type n) const
    { return find_last_of(basic_string_view(s, n), pos); }

    constexpr size_type find_last_of(charT const * s, size_type pos = npos)
        const
    { return find_last_of(basic_string_view(s), pos); }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    size_type find_first_not_of(basic_string_view s, size_type pos = 0) const
        noexcept
    {
        for (auto xpos = pos; xpos < size(); ++xpos) {
            bool match = true;
            for (size_type i = 0; i != s.size(); ++i) {
                if (traits::eq(data_[xpos], s.data_[i])) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return xpos;
            }
        }
        return npos;
    }

    constexpr size_type find_first_not_of(charT c, size_type pos = 0) const
        noexcept
    { return find_first_not_of(basic_string_view(&c, 1), pos); }

    constexpr size_type find_first_not_of(
        charT const * s, size_type pos, size_type n) const
    { return find_first_not_of(basic_string_view(s, n), pos); }

    constexpr size_type find_first_not_of(charT const * s, size_type pos = 0)
        const
    { return find_first_not_of(basic_string_view(s), pos); }

#if HAVE_CXX14_CONSTEXPR
    constexpr
#endif
    size_type find_last_not_of(basic_string_view s, size_type pos = npos) const
        noexcept
    {
        if (!empty()) {
            for (auto xpos = std::min<size_type>(size() - 1, pos);; --xpos) {
                bool match = true;
                for (size_type i = 0; i != s.size(); ++i) {
                    if (traits::eq(data_[xpos], s.data_[i])) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    return xpos;
                }
                if (xpos == 0) {
                    break;
                }
            }
        }
        return npos;
    }

    constexpr size_type find_last_not_of(charT c, size_type pos = npos) const
        noexcept
    { return find_last_not_of(basic_string_view(&c, 1), pos); }

    constexpr size_type find_last_not_of(
        charT const * s, size_type pos, size_type n) const
    { return find_last_not_of(basic_string_view(s, n), pos); }

    constexpr size_type find_last_not_of(charT const * s, size_type pos = npos)
        const
    { return find_last_not_of(basic_string_view(s), pos); }

    // LO specifics:

    // For std::basic_string_view, this is provided via a non-explicit
    // conversion operator from std::basic_string:
    constexpr basic_string_view(std::basic_string<charT, traits> const & s):
        data_(s.data()), size_(s.size()) {}

    // For std::string_view, this will be provided by a (LIBO_INTERNAL_ONLY)
    // non-explicit conversion operator from rtl::OString:
    template<typename T = charT> basic_string_view(
        OString const & s,
        typename std::enable_if<
            std::is_same<T, char>::value,
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(s.getStr()), size_(s.getLength()) {}

    // For std::u16string_view, this will be provided by a (LIBO_INTERNAL_ONLY)
    // non-explicit conversion operator from rtl::OUString:
    template<typename T = charT> basic_string_view(
        OUString const & s,
        typename std::enable_if<
            std::is_same<T, sal_Unicode>::value,
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(s.getStr()), size_(s.getLength()) {}

    // For std::u16string_view, this would either be provided by a
    // (LIBO_INTERNAL_ONLY) non-explicit conversion operator from
    // rtl::OUStringLiteral, or rtl::OUStringLiteral would be given up in favor
    // of std::u16string_view anyway (but this constructor also serves to reject
    // as ambiguous construction of a o3tl::u16string_view from a narrow string
    // literal, which would otherwise go via the above rtl::OUString
    // constructor):
    template<typename T = charT> constexpr basic_string_view(
        OUStringLiteral literal,
        typename std::enable_if<
            std::is_same<T, sal_Unicode>::value,
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(literal.data), size_(literal.size) {}

    // LO specifics, to make up for traits::length not necessarily being
    // constexpr yet for literal arguments:

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 const & value,
        typename std::enable_if<
            std::is_same<T2, char>::value && detail::CharPtrDetector<T1>::ok,
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(value), size_(traits::length(value)) {}

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 & value,
        typename std::enable_if<
            (std::is_same<T2, char>::value
             && detail::NonConstCharArrayDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(value), size_(traits::length(value)) {}

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 & literal,
        typename std::enable_if<
            (std::is_same<T2, char>::value
             && detail::ConstCharArrayDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(literal), size_(detail::ConstCharArrayDetector<T1>::length)
    { /*assert(size_ == traits::length(literal);*/ }

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 const & value,
        typename std::enable_if<
            (std::is_same<T2, char16_t>::value
             && detail::Char16PtrDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(value), size_(traits::length(value)) {}

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 & value,
        typename std::enable_if<
            (std::is_same<T2, char16_t>::value
             && detail::NonConstChar16ArrayDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(value), size_(traits::length(value)) {}

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 & literal,
        typename std::enable_if<
            (std::is_same<T2, char16_t>::value
             && detail::ConstChar16ArrayDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(literal), size_(detail::ConstChar16ArrayDetector<T1>::length)
    { /*assert(size_ == traits::length(literal);*/ }

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 const & value,
        typename std::enable_if<
            (std::is_same<T2, char32_t>::value
             && detail::Char32PtrDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(value), size_(traits::length(value)) {}

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 & value,
        typename std::enable_if<
            (std::is_same<T2, char32_t>::value
             && detail::NonConstChar32ArrayDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(value), size_(traits::length(value)) {}

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 & literal,
        typename std::enable_if<
            (std::is_same<T2, char32_t>::value
             && detail::ConstChar32ArrayDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(literal), size_(detail::ConstChar32ArrayDetector<T1>::length)
    { /*assert(size_ == traits::length(literal);*/ }

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 const & value,
        typename std::enable_if<
            (std::is_same<T2, wchar_t>::value
             && detail::WcharPtrDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(value), size_(traits::length(value)) {}

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 & value,
        typename std::enable_if<
            (std::is_same<T2, wchar_t>::value
             && detail::NonConstWcharArrayDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(value), size_(traits::length(value)) {}

    template<typename T1, typename T2 = charT> constexpr basic_string_view(
        T1 & literal,
        typename std::enable_if<
            (std::is_same<T2, wchar_t>::value
             && detail::ConstWcharArrayDetector<T1>::ok),
            rtl::libreoffice_internal::Dummy>::type = {}):
        data_(literal), size_(detail::ConstWcharArrayDetector<T1>::length)
    { /*assert(size_ == traits::length(literal);*/ }

private:
    const_pointer data_;
    size_type size_;
};

template<class charT, class traits> constexpr bool operator ==(
    basic_string_view<charT, traits> x, basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) == 0; }

template<class charT, class traits> constexpr bool operator ==(
    basic_string_view<charT, traits> x,
    typename std::decay<basic_string_view<charT, traits>>::type y)
    noexcept
{ return x.compare(y) == 0; }

template<class charT, class traits> constexpr bool operator ==(
    typename std::decay<basic_string_view<charT, traits>>::type x,
    basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) == 0; }

template<class charT, class traits> constexpr bool operator !=(
    basic_string_view<charT, traits> x, basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) != 0; }

template<class charT, class traits> constexpr bool operator !=(
    basic_string_view<charT, traits> x,
    typename std::decay<basic_string_view<charT, traits>>::type y)
    noexcept
{ return x.compare(y) != 0; }

template<class charT, class traits> constexpr bool operator !=(
    typename std::decay<basic_string_view<charT, traits>>::type x,
    basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) != 0; }

template<class charT, class traits> constexpr bool operator <(
    basic_string_view<charT, traits> x, basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) < 0; }

template<class charT, class traits> constexpr bool operator <(
    basic_string_view<charT, traits> x,
    typename std::decay<basic_string_view<charT, traits>>::type y)
    noexcept
{ return x.compare(y) < 0; }

template<class charT, class traits> constexpr bool operator <(
    typename std::decay<basic_string_view<charT, traits>>::type x,
    basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) < 0; }

template<class charT, class traits> constexpr bool operator >(
    basic_string_view<charT, traits> x, basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) > 0; }

template<class charT, class traits> constexpr bool operator >(
    basic_string_view<charT, traits> x,
    typename std::decay<basic_string_view<charT, traits>>::type y)
    noexcept
{ return x.compare(y) > 0; }

template<class charT, class traits> constexpr bool operator >(
    typename std::decay<basic_string_view<charT, traits>>::type x,
    basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) > 0; }

template<class charT, class traits> constexpr bool operator <=(
    basic_string_view<charT, traits> x, basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) <= 0; }

template<class charT, class traits> constexpr bool operator <=(
    basic_string_view<charT, traits> x,
    typename std::decay<basic_string_view<charT, traits>>::type y)
    noexcept
{ return x.compare(y) <= 0; }

template<class charT, class traits> constexpr bool operator <=(
    typename std::decay<basic_string_view<charT, traits>>::type x,
    basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) <= 0; }

template<class charT, class traits> constexpr bool operator >=(
    basic_string_view<charT, traits> x, basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) >= 0; }

template<class charT, class traits> constexpr bool operator >=(
    basic_string_view<charT, traits> x,
    typename std::decay<basic_string_view<charT, traits>>::type y)
    noexcept
{ return x.compare(y) >= 0; }

template<class charT, class traits> constexpr bool operator >=(
    typename std::decay<basic_string_view<charT, traits>>::type x,
    basic_string_view<charT, traits> y)
    noexcept
{ return x.compare(y) >= 0; }

template<class charT, class traits> std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & os,
    basic_string_view<charT, traits> str)
{
    typename std::basic_ostream<charT, traits>::sentry sentry;
    if (sentry) {
        auto const w = os.width();
        auto const pad
            = std::max<std::make_unsigned<decltype(w + str.size())>::type>(
                w < 0 ? 0 : w, str.size());
        auto const after = (os.flags() & std::ios_base::adjustfield)
            == std::ios_base::left;
        if (pad != 0 && !after) {
            auto const c = os.fill();
            for (; pad != 0; --pad) {
                os.rdbuf()->sputc(c);
            }
        }
        os.rdbuf()->sputn(str.data(), str.size());
        if (pad != 0 && after) {
            auto const c = os.fill();
            for (; pad != 0; --pad) {
                os.rdbuf()->sputc(c);
            }
        }
        os.width(0);
    } else {
        os.setstate(std::ios_base::failbit);
    }
    return os;
}

using string_view = basic_string_view<char>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;
using wstring_view = basic_string_view<wchar_t>;

// no literals::string_view_literals::operator "" sv

}

namespace std {

template<> struct hash<o3tl::string_view> {
    std::size_t operator ()(o3tl::string_view s)
    { return hash<string>()(string(s.data(), s.size())); }
};

template<> struct hash<o3tl::u16string_view> {
    std::size_t operator ()(o3tl::u16string_view s)
    { return hash<u16string>()(u16string(s.data(), s.size())); }
};

template<> struct hash<o3tl::u32string_view> {
    std::size_t operator ()(o3tl::u32string_view s)
    { return hash<u32string>()(u32string(s.data(), s.size())); }
};

template<> struct hash<o3tl::wstring_view> {
    std::size_t operator ()(o3tl::wstring_view s)
    { return hash<wstring>()(wstring(s.data(), s.size())); }
};

}

namespace o3tl {

// LO-specific convenience functions:

// For std::u16string_view, this will be provided by a (LIBO_INTERNAL_ONLY)
// rtl::OUString constructor:
inline OUString toOUString(u16string_view s)
{ return OUString(s.data(), s.size()); }

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
