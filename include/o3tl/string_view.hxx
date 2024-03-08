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

#include <o3tl/intcmp.hxx>
#include <rtl/character.hxx>
#include <rtl/ustring.h>
#include <rtl/math.h>
#include <sal/types.h>

namespace o3tl
{
// Like OUString::equalsAscii/OUString::equalsAsciiL, but for std::u16string_view:
inline bool equalsAscii(std::u16string_view s1, std::string_view s2)
{
    return s1.size() == s2.size()
           && rtl_ustr_ascii_shortenedCompare_WithLength(s1.data(), s1.size(), s2.data(), s2.size())
                  == 0;
}

// Like OUString::compareToAscii, but for std::u16string_view and std::string_view:
inline int compareToAscii(std::u16string_view s1, std::string_view s2)
{
    return rtl_ustr_asciil_reverseCompare_WithLength(s1.data(), s1.size(), s2.data(), s2.size());
};

// Like OUString::equalsIgnoreAsciiCase, but for two std::u16string_view:
inline bool equalsIgnoreAsciiCase(std::u16string_view s1, std::u16string_view s2)
{
    if (s1.size() != s2.size())
        return false;
    if (s1.data() == s2.data())
        return true;
    return rtl_ustr_compareIgnoreAsciiCase_WithLength(s1.data(), s1.size(), s2.data(), s2.size())
           == 0;
};

inline bool equalsIgnoreAsciiCase(std::u16string_view s1, std::string_view s2)
{
    return s1.size() == s2.size()
           && (rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength(s1.data(), s1.size(),
                                                                         s2.data(), s2.size())
               == 0);
}

inline bool equalsIgnoreAsciiCase(std::string_view s1, std::string_view s2)
{
    if (s1.size() != s2.size())
        return false;
    if (s1.data() == s2.data())
        return true;
    return rtl_str_compareIgnoreAsciiCase_WithLength(s1.data(), s1.size(), s2.data(), s2.size())
           == 0;
};

// Like OUString::compareToIgnoreAsciiCase, but for two std::u16string_view:
inline int compareToIgnoreAsciiCase(std::u16string_view s1, std::u16string_view s2)
{
    return rtl_ustr_compareIgnoreAsciiCase_WithLength(s1.data(), s1.size(), s2.data(), s2.size());
};

// Like OUString::matchIgnoreAsciiCase, but for two std::u16string_view:
inline bool matchIgnoreAsciiCase(std::u16string_view s1, std::u16string_view s2,
                                 sal_Int32 fromIndex = 0)
{
    return rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength(
               s1.data() + fromIndex, s1.size() - fromIndex, s2.data(), s2.size(), s2.size())
           == 0;
}

// Like OUString::matchIgnoreAsciiCase, but for std::u16string_view and std::string_view:
inline bool matchIgnoreAsciiCase(std::u16string_view s1, std::string_view s2,
                                 sal_Int32 fromIndex = 0)
{
    return rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength(
               s1.data() + fromIndex, s1.size() - fromIndex, s2.data(), s2.size())
           == 0;
}

// Like OUString::endsWithIgnoreAsciiCase, but for std::u16string_view
inline bool endsWithIgnoreAsciiCase(std::u16string_view s1, std::u16string_view s2,
                                    std::u16string_view* rest = nullptr)
{
    auto const b = s2.size() <= s1.size() && matchIgnoreAsciiCase(s1, s2, s1.size() - s2.size());
    if (b && rest != nullptr)
    {
        *rest = s1.substr(0, s1.size() - s2.size());
    }
    return b;
}

inline bool endsWithIgnoreAsciiCase(std::u16string_view s1, std::string_view s2,
                                    std::u16string_view* rest = nullptr)
{
    auto const b = s2.size() <= s1.size()
                   && rtl_ustr_ascii_compareIgnoreAsciiCase_WithLengths(
                          s1.data() + s1.size() - s2.size(), s2.size(), s2.data(), s2.size())
                          == 0;
    if (b && rest != nullptr)
    {
        *rest = s1.substr(0, s1.size() - s2.size());
    }
    return b;
}

// Similar to O[U]String::getToken, returning the first token of a std::[u16]string_view starting
// at a given position.
//
// Attention:  There are two sets of o3tl::getToken overloads here.  This first set has an interface
// based on std::size_t length parameters, and its semantics don't match those of
// O[U]String::getToken exactly (buf if needed, it can be extended to return the n'th token instead
// of just the first, and/or support an initial position of npos, to make the semantics match).
template <typename charT, typename traits = std::char_traits<charT>>
inline std::basic_string_view<charT, traits> getToken(std::basic_string_view<charT, traits> sv,
                                                      charT delimiter, std::size_t& position)
{
    assert(position <= sv.size());
    auto const n = sv.find(delimiter, position);
    std::basic_string_view<charT, traits> t;
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
// The following two overloads prevent overload resolution mistakes that would occur with their
// template counterpart, when sv is of a type that is implicitly convertible to basic_string_view
// (like OString or OUString), in which case overload resolution would erroneously choose the
// three-argument overloads (taking sv, nToken, cTok) from the second set of
// o3tl::getToken overloads below:
inline std::string_view getToken(std::string_view sv, char delimiter, std::size_t& position)
{
    return getToken<char>(sv, delimiter, position);
}
inline std::u16string_view getToken(std::u16string_view sv, char16_t delimiter,
                                    std::size_t& position)
{
    return getToken<char16_t>(sv, delimiter, position);
}

// Similar to O[U]String::getToken.
//
// Attention:  There are two sets of o3tl::getToken overloads here.  This second set has an
// interface based on sal_Int32 length parameters, and is meant to be a drop-in replacement for
// O[U]String::getToken.
template <typename charT, typename traits = std::char_traits<charT>>
inline std::basic_string_view<charT, traits> getToken(std::basic_string_view<charT, traits> pStr,
                                                      sal_Int32 nToken, charT cTok,
                                                      sal_Int32& rnIndex)
{
    assert(o3tl::IntCmp(rnIndex) <= o3tl::IntCmp(pStr.size()));

    // Return an empty string and set rnIndex to -1 if either nToken or rnIndex is
    // negative:
    if (rnIndex >= 0 && nToken >= 0)
    {
        const charT* pOrgCharStr = pStr.data();
        const charT* pCharStr = pOrgCharStr + rnIndex;
        sal_Int32 nLen = pStr.size() - rnIndex;
        sal_Int32 nTokCount = 0;
        const charT* pCharStrStart = pCharStr;
        while (nLen > 0)
        {
            if (*pCharStr == cTok)
            {
                nTokCount++;

                if (nTokCount > nToken)
                    break;
                if (nTokCount == nToken)
                    pCharStrStart = pCharStr + 1;
            }

            pCharStr++;
            nLen--;
        }
        if (nTokCount >= nToken)
        {
            if (nLen > 0)
                rnIndex = pCharStr - pOrgCharStr + 1;
            else
                rnIndex = -1;
            return std::basic_string_view<charT, traits>(pCharStrStart, pCharStr - pCharStrStart);
        }
    }

    rnIndex = -1;
    return std::basic_string_view<charT, traits>();
}
// The following two overloads prevent deduction failures that would occur with their template
// counterpart, when sv is of a type that is implicitly convertible to basic_string_view (like
// OString or OUString):
inline std::string_view getToken(std::string_view sv, sal_Int32 nToken, char cTok,
                                 sal_Int32& rnIndex)
{
    return getToken<char>(sv, nToken, cTok, rnIndex);
}
inline std::u16string_view getToken(std::u16string_view sv, sal_Int32 nToken, char16_t cTok,
                                    sal_Int32& rnIndex)
{
    return getToken<char16_t>(sv, nToken, cTok, rnIndex);
}
inline std::string_view getToken(std::string_view sv, sal_Int32 nToken, char cTok)
{
    sal_Int32 nIndex = 0;
    return getToken<char>(sv, nToken, cTok, nIndex);
}
inline std::u16string_view getToken(std::u16string_view sv, sal_Int32 nToken, char16_t cTok)
{
    sal_Int32 nIndex = 0;
    return getToken<char16_t>(sv, nToken, cTok, nIndex);
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
#if defined __cpp_lib_starts_ends_with
    return sv.ends_with(x);
#else
    return sv.size() >= x.size()
           && sv.compare(sv.size() - x.size(), std::basic_string_view<charT, traits>::npos, x) == 0;
#endif
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool ends_with(std::basic_string_view<charT, traits> sv, charT x) noexcept
{
#if defined __cpp_lib_starts_ends_with
    return sv.ends_with(x);
#else
    return !sv.empty() && traits::eq(sv.back(), x);
#endif
}
template <typename charT, typename traits = std::char_traits<charT>>
constexpr bool ends_with(std::basic_string_view<charT, traits> sv, charT const* x)
{
#if defined __cpp_lib_starts_ends_with
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

namespace internal
{
inline bool implIsWhitespace(sal_Unicode c)
{
    /* Space or Control character? */
    if ((c <= 32) && c)
        return true;

    /* Only in the General Punctuation area Space or Control characters are included? */
    if ((c < 0x2000) || (c > 0x2029))
        return false;

    if ((c <= 0x200B) || /* U+2000 - U+200B All Spaces */
        (c >= 0x2028)) /* U+2028 LINE SEPARATOR, U+2029 PARAGRAPH SEPARATOR */
        return true;

    return false;
}
} // namespace internal

// Like OUString::trim, but for std::[u16]string_view:
template <typename charT, typename traits = std::char_traits<charT>>
std::basic_string_view<charT, traits> trim(std::basic_string_view<charT, traits> str)
{
    auto pFirst = str.data();
    auto pLast = pFirst + str.size();

    while ((pFirst < pLast) && internal::implIsWhitespace(*pFirst))
        ++pFirst;

    if (pFirst == pLast)
        return {};

    do
        --pLast;
    while (internal::implIsWhitespace(*pLast));

    return std::basic_string_view<charT, traits>(pFirst, pLast - pFirst + 1);
}

// "deduction guides"

inline auto trim(std::string_view str) { return trim<>(str); }
inline auto trim(std::u16string_view str) { return trim<>(str); }

// Like OString::toInt32, but for std::string_view:
inline sal_Int32 toInt32(std::u16string_view str, sal_Int16 radix = 10)
{
    sal_Int64 n = rtl_ustr_toInt64_WithLength(str.data(), radix, str.size());
    if (n < SAL_MIN_INT32 || n > SAL_MAX_INT32)
        n = 0;
    return n;
}
inline sal_Int32 toInt32(std::string_view str, sal_Int16 radix = 10)
{
    sal_Int64 n = rtl_str_toInt64_WithLength(str.data(), radix, str.size());
    if (n < SAL_MIN_INT32 || n > SAL_MAX_INT32)
        n = 0;
    return n;
}

// Like OString::toUInt32, but for std::string_view:
inline sal_uInt32 toUInt32(std::u16string_view str, sal_Int16 radix = 10)
{
    sal_Int64 n = rtl_ustr_toInt64_WithLength(str.data(), radix, str.size());
    if (n < 0 || n > SAL_MAX_UINT32)
        n = 0;
    return n;
}
inline sal_uInt32 toUInt32(std::string_view str, sal_Int16 radix = 10)
{
    sal_Int64 n = rtl_str_toInt64_WithLength(str.data(), radix, str.size());
    if (n < 0 || n > SAL_MAX_UINT32)
        n = 0;
    return n;
}

// Like OString::toInt64, but for std::string_view:
inline sal_Int64 toInt64(std::u16string_view str, sal_Int16 radix = 10)
{
    return rtl_ustr_toInt64_WithLength(str.data(), radix, str.size());
}
inline sal_Int64 toInt64(std::string_view str, sal_Int16 radix = 10)
{
    return rtl_str_toInt64_WithLength(str.data(), radix, str.size());
}

// Like OString::toDouble, but for std::string_view:
inline double toDouble(std::u16string_view str)
{
    return rtl_math_uStringToDouble(str.data(), str.data() + str.size(), '.', 0, nullptr, nullptr);
}
inline double toDouble(std::string_view str)
{
    return rtl_math_stringToDouble(str.data(), str.data() + str.size(), '.', 0, nullptr, nullptr);
}

// Like OUString::iterateCodePoints, but for std::string_view:
template <typename T>
requires(std::is_same_v<T, sal_Int32> || std::is_same_v<T, std::size_t>) sal_uInt32
    iterateCodePoints(std::u16string_view string, T* indexUtf16, sal_Int32 incrementCodePoints = 1)
{
    std::size_t n;
    char16_t cu;
    sal_uInt32 cp;
    assert(indexUtf16 != nullptr);
    n = *indexUtf16;
    assert(n <= string.length());
    while (incrementCodePoints < 0)
    {
        assert(n > 0);
        cu = string[--n];
        if (rtl::isLowSurrogate(cu) && n != 0 && rtl::isHighSurrogate(string[n - 1]))
        {
            --n;
        }
        ++incrementCodePoints;
    }
    assert(n < string.length());
    cu = string[n];
    if (rtl::isHighSurrogate(cu) && string.length() - n >= 2 && rtl::isLowSurrogate(string[n + 1]))
    {
        cp = rtl::combineSurrogates(cu, string[n + 1]);
    }
    else
    {
        cp = cu;
    }
    while (incrementCodePoints > 0)
    {
        assert(n < string.length());
        cu = string[n++];
        if (rtl::isHighSurrogate(cu) && n != string.length() && rtl::isLowSurrogate(string[n]))
        {
            ++n;
        }
        --incrementCodePoints;
    }
    assert(n <= string.length());
    *indexUtf16 = n;
    return cp;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
