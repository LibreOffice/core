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

#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

#include <rtl/ustring.h>

namespace o3tl
{
// Like OUString::equalsIgnoreAsciiCase, but for two std::u16string_view:
inline bool equalsIgnoreAsciiCase(std::u16string_view s1, std::u16string_view s2)
{
    return rtl_ustr_compareIgnoreAsciiCase_WithLength(s1.data(), s1.size(), s2.data(), s2.size());
};

// Similar to OString::getToken, returning the first token of a std::string_view, starting at a
// given position (and if needed, it can be turned into a template to also cover std::u16string_view
// etc., or extended to return the n'th token instead of just the first, or support an initial
// position of npos):
inline std::string_view getToken(std::string_view sv, char delimiter, std::size_t& position)
{
    assert(position <= sv.size());
    size_t const n = sv.find(delimiter, position);
    std::string_view t;
    if (n == std::string_view::npos)
    {
        t = sv.substr(position);
        position = std::string_view::npos;
    }
    else
    {
        t = sv.substr(position, n - position);
        position = n + 1;
    }
    return t;
}

// Implementations of C++20 std::basic_string_view::starts_with and
// std::basic_string_view::ends_with, until we can use those directly on all platforms:
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool starts_with(std::basic_string_view<charT, traits> sv,
                           std::basic_string_view<charT, traits> x) noexcept
{
#if defined __cpp_lib_starts_ends_with
    return sv.starts_with(x);
#else
    return sv.substr(0, x.size()) == x;
#endif
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool starts_with(std::basic_string_view<charT, traits> sv, charT x) noexcept
{
#if defined __cpp_lib_starts_ends_with
    return sv.starts_with(x);
#else
    return !sv.empty() && traits::eq(sv.front(), x);
#endif
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool starts_with(std::basic_string_view<charT, traits> sv, charT const* x)
{
#if defined __cpp_lib_starts_ends_with
    return sv.starts_with(x);
#else
    return starts_with(sv, std::basic_string_view<charT, traits>(x));
#endif
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool ends_with(std::basic_string_view<charT, traits> sv,
                         std::basic_string_view<charT, traits> x) noexcept
{
#if defined __cpp_lib_ends_ends_with
    return sv.ends_with(x);
#else
    return sv.size() >= x.size()
           && sv.compare(sv.size() - x.size(), std::basic_string_view<charT, traits>::npos, x) == 0;
#endif
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool ends_with(std::basic_string_view<charT, traits> sv, charT x) noexcept
{
#if defined __cpp_lib_ends_ends_with
    return sv.ends_with(x);
#else
    return !sv.empty() && traits::eq(sv.back(), x);
#endif
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool ends_with(std::basic_string_view<charT, traits> sv, charT const* x)
{
#if defined __cpp_lib_ends_ends_with
    return sv.ends_with(x);
#else
    return ends_with(sv, std::basic_string_view<charT, traits>(x));
#endif
}
// The following overloads prevent deduction failures that would occur with their template
// counterparts, when x is of a type that is implicitly convertible to basic_string_view (like
// OString or OUString, and we only bother to provide overloads for the char and char16_t cases, not
// also for char32_t and wchar_t, nor for C++20 char8_t):
constexpr bool starts_with(std::string_view sv, std::string_view x) noexcept
{
    return starts_with<char>(sv, x);
}
constexpr bool starts_with(std::u16string_view sv, std::u16string_view x) noexcept
{
    return starts_with<char16_t>(sv, x);
}
constexpr bool ends_with(std::string_view sv, std::string_view x) noexcept
{
    return ends_with<char>(sv, x);
}
constexpr bool ends_with(std::u16string_view sv, std::u16string_view x) noexcept
{
    return ends_with<char16_t>(sv, x);
}

// Variants of C++20 std::basic_string_view::starts_with and
// std::basic_string_view::ends_with that have a rest out parameter, similar to our OString and
// OUString startsWith and endsWith member functions:
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool starts_with(std::basic_string_view<charT, traits> sv,
                           std::basic_string_view<charT, traits> x,
                           std::basic_string_view<charT, traits>* rest) noexcept
{
    assert(rest != nullptr);
    auto const found = starts_with(sv, x);
    if (found)
    {
        *rest = sv.substr(x.length());
    }
    return found;
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool starts_with(std::basic_string_view<charT, traits> sv, charT x,
                           std::basic_string_view<charT, traits>* rest) noexcept
{
    assert(rest != nullptr);
    auto const found = starts_with(sv, x);
    if (found)
    {
        *rest = sv.substr(1);
    }
    return found;
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool starts_with(std::basic_string_view<charT, traits> sv, charT const* x,
                           std::basic_string_view<charT, traits>* rest)
{
    assert(rest != nullptr);
    auto const found = starts_with(sv, x);
    if (found)
    {
        *rest = sv.substr(traits::length(x));
    }
    return found;
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool ends_with(std::basic_string_view<charT, traits> sv,
                         std::basic_string_view<charT, traits> x,
                         std::basic_string_view<charT, traits>* rest) noexcept
{
    assert(rest != nullptr);
    auto const found = ends_with(sv, x);
    if (found)
    {
        *rest = sv.substr(0, sv.length() - x.length());
    }
    return found;
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool ends_with(std::basic_string_view<charT, traits> sv, charT x,
                         std::basic_string_view<charT, traits>* rest) noexcept
{
    assert(rest != nullptr);
    auto const found = ends_with(sv, x);
    if (found)
    {
        *rest = sv.substr(0, sv.length() - 1);
    }
    return found;
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool ends_with(std::basic_string_view<charT, traits> sv, charT const* x,
                         std::basic_string_view<charT, traits>* rest)
{
    assert(rest != nullptr);
    auto const found = ends_with(sv, x);
    if (found)
    {
        *rest = sv.substr(0, sv.length() - traits::length(x));
    }
    return found;
}
// The following overloads prevent deduction failures that would occur with their template
// counterparts, when x is of a type that is implicitly convertible to basic_string_view (like
// OString or OUString, and we only bother to provide overloads for the char and char16_t cases, not
// also for char32_t and wchar_t, nor for C++20 char8_t):
constexpr bool starts_with(std::string_view sv, std::string_view x, std::string_view* rest) noexcept
{
    return starts_with<char>(sv, x, rest);
}
constexpr bool starts_with(std::u16string_view sv, std::u16string_view x,
                           std::u16string_view* rest) noexcept
{
    return starts_with<char16_t>(sv, x, rest);
}
constexpr bool ends_with(std::string_view sv, std::string_view x, std::string_view* rest) noexcept
{
    return ends_with<char>(sv, x, rest);
}
constexpr bool ends_with(std::u16string_view sv, std::u16string_view x,
                         std::u16string_view* rest) noexcept
{
    return ends_with<char16_t>(sv, x, rest);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
