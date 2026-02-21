/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

/// Utilities for numeric conversions.
/// See test/NumUtilWhiteBoxTests.cpp for test cases.
/// For parsing from a given offset and reporting the end, use
/// parseStrTo<T, StrToState>, it will report the status of the conversion.
/// When the state or indeed any failure details aren't needed, use strTo<T>.
/// For convenient wrappers see parseStrTo*() and *FromString() functions.
namespace NumUtil
{

enum class StrToState : std::uint8_t
{
    Complete, ///< Completely parsed the given string into a valid integer.
    Partial, ///< Partially parsed the given string, i.e. "123abc" -> 123.
    Empty, ///< Fail: input was empty, including whitespace, i.e. no digits found.
    Nan, ///< Fail: input is not an integer, i.e. starts with non-digit characters.
    Overflow ///< Fail: the parsed integer overflowed the integer range and clamped.
};

/// Convert from a string into an integer, like strto* family,
/// supporting std::string_view arguments.
/// The number must be base-10 and can start with
/// either '+' or '-', as well as optional whitespace.
/// Unlike strto*, the result is *not* coerced into the return type.
/// A negative value for an unsigned type will return Overflow.
/// This is typically called while parsing.
/// @offset is the position at which to start parsing
/// and will hold the position at which parsing terminated.
/// As a bonus, this is 2-4x faster than glibc across all lengths.
template <typename T>
std::pair<T, StrToState> parseStrTo(const std::string_view str, std::size_t& orig_offset)
    requires std::is_integral_v<T>
{
    using Unsigned = std::make_unsigned_t<T>;
    using Signed = std::make_signed_t<T>;

    const char* s = str.data();
    const std::size_t len = str.size();
    std::size_t offset = orig_offset;
    if (offset >= len)
    {
        return { T(), StrToState::Empty };
    }

    Signed c = s[offset] - '0';
    bool neg = false;
    if (static_cast<std::uint_fast64_t>(c) >= 10)
    {
        // Skip whitespace.
        while (offset < len && (s[offset] == ' ' || s[offset] == '\t'))
            offset++;

        if (offset >= len)
            return { T(), StrToState::Empty };

        c = s[offset++];
        neg = (c == '-');
        if (neg || c == '+')
        {
            if (offset >= len)
                return { T(), StrToState::Nan };

            if constexpr (!std::is_signed_v<T>)
            {
                if (neg)
                {
                    // -ve in an unsigned int is overflow.
                    while (offset < len && static_cast<std::uint_fast64_t>(s[offset] - '0') < 10)
                        offset++; // Eat all the digits.
                    orig_offset = offset;
                    return { std::numeric_limits<T>::max(), StrToState::Overflow };
                }
            }

            c = s[offset++];
        }

        c -= '0';
        if (static_cast<std::uint_fast64_t>(c) >= 10)
            return { T(), StrToState::Nan };
    }
    else
    {
        ++offset;
        if (offset == len)
        {
            orig_offset = len;
            return { c, StrToState::Complete };
        }
    }

    Unsigned res = c;

    // Up to 9 total digits (1 already parsed + 8 remaining). Can't overflow any 32+ bit type.
    // Use an unrolled switch to avoid per-digit overflow checks and loop overhead.
    const std::size_t remaining =
        std::min<std::size_t>(len - offset, std::numeric_limits<T>::digits10 - 1);
#define CASE(X)                                                                                    \
    case X:                                                                                        \
        c = s[offset] - '0';                                                                       \
        if (static_cast<std::uint_fast64_t>(c) >= 10)                                              \
            break;                                                                                 \
        res = 10 * res + c;                                                                        \
        ++offset;                                                                                  \
        [[fallthrough]]

    do
    {
        switch (remaining)
        {
            CASE(8);
            CASE(7);
            CASE(6);
            CASE(5);
            CASE(4);
            CASE(3);
            CASE(2);
            CASE(1);
            case 0:
                break;

            default:
                if constexpr (std::numeric_limits<T>::digits10 > 10)
                {
                    // Unchecked loop for digits that can't overflow (up to digits10 total).
                    const std::size_t end = offset + remaining;
                    for (; offset < end;)
                    {
                        c = s[offset] - '0';
                        if (static_cast<std::uint_fast64_t>(c) >= 10)
                            break;

                        ++offset;
                        res = 10 * res + c;
                    }
                }
        }
    } while (false);
#undef CASE

    if (offset < len)
    {
        // Overflow-checked loop for remaining digits beyond digits10.
        constexpr auto cutoff = std::numeric_limits<T>::max() / 10;
        const Signed cutlim =
            (neg ? -(std::numeric_limits<T>::min() % 10) : std::numeric_limits<T>::max() % 10);

        do
        {
            c = s[offset] - '0';
            if (static_cast<std::uint_fast64_t>(c) >= 10)
                break;

            ++offset; // Skip the consumed character.
            if (res >= cutoff)
            {
                if (res > cutoff || c > cutlim ||
                    (offset < len && static_cast<std::uint_fast64_t>(s[offset] - '0') < 10))
                {
                    // Overflow.
                    while (offset < len && static_cast<std::uint_fast64_t>(s[offset] - '0') < 10)
                        offset++; // Eat all the digits.
                    orig_offset = offset;
                    return { neg ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max(),
                             StrToState::Overflow };
                }

                if constexpr (std::is_signed_v<T>)
                {
                    orig_offset = offset;
                    if (neg)
                    {
                        T sres = res;
                        return { -10 * sres - c,
                                 offset == len ? StrToState::Complete : StrToState::Partial };
                    }
                    else
                    {
                        return { 10 * res + c,
                                 offset == len ? StrToState::Complete : StrToState::Partial };
                    }
                }
            }

            res = 10 * res + c;
        } while (offset < len);
    }

    orig_offset = offset;
    if constexpr (!std::is_signed_v<T>)
    {
        return { res, offset == len ? StrToState::Complete : StrToState::Partial };
    }

    const T sres = static_cast<T>(res);
    return { neg ? -sres : sres, offset == len ? StrToState::Complete : StrToState::Partial };
}

/// Convert from a string into an integer and default on failure.
/// The number must be base-10 and can start with
/// either '+' or '-', as well as optional whitespace.
/// Unlike strto*, the result is *not* coerced into the return type.
/// A negative value for an unsigned type will return the default.
/// On failure to parse, overflow, or underflow, the default is returned.
/// As a bonus, this is 2-4x faster than glibc across all lengths.
template <typename T>
T strTo(const std::string_view str, T def)
    requires std::is_integral_v<T>
{
    std::size_t offset = 0;
    const auto [result, state] = parseStrTo<T>(str, offset);
    return (state == StrToState::Complete || state == StrToState::Partial) ? result : def;
}

/// Parse a string to 32-bit signed int.
/// Returns the parsed value and a boolean indicating success or failure.
/// const auto [number, state] = NumUtil::parseStrToInt32(str, offset);
inline std::pair<std::int32_t, StrToState> parseStrToInt32(const std::string_view str, std::size_t& offset)
{
    return parseStrTo<std::int32_t>(str, offset);
}

/// Convert a string to 32-bit signed int.
/// Returns the parsed value and a boolean indicating success or failure.
/// const auto [number, success] = NumUtil::i32FromString(portString);
inline std::pair<std::int32_t, bool> i32FromString(const std::string_view input)
{
    std::size_t offset = 0;
    const auto [value, status] = parseStrTo<std::int32_t>(input, offset);
    return { value, status <= StrToState::Partial };
}

/// Convert a string to 32-bit signed int. On failure, returns the default value.
/// Used where there is no interest in knowing whether the input was valid or not.
inline std::int32_t i32FromString(const std::string_view input, const std::int32_t def)
{
    return strTo<std::int32_t>(input, def);
}

/// Convert a string to 32-bit unsigned int.
/// Returns the parsed value and a boolean indicating success or failure.
/// const auto [number, success] = NumUtil::u32FromString(portString);
inline std::pair<std::uint32_t, bool> u32FromString(const std::string_view input)
{
    std::size_t offset = 0;
    const auto [value, status] = parseStrTo<std::uint32_t>(input, offset);
    return { value, status <= StrToState::Partial };
}

/// Convert a string to 32-bit unsigned int. On failure, returns the default value.
/// Used where there is no interest in knowing whether the input was valid or not.
inline std::uint32_t u32FromString(const std::string_view input, const std::uint32_t def)
{
    return strTo<std::uint32_t>(input, def);
}

/// Parse a string to unsigned 32-bit int.
/// Returns the parsed value and a boolean indicating success or failure.
/// const auto [number, state] = NumUtil::parseStrToUint32(str, offset);
inline std::pair<std::uint32_t, StrToState> parseStrToUint32(const std::string_view str, std::size_t& offset)
{
    return parseStrTo<std::uint32_t>(str, offset);
}

/// Parse a string to signed 64-bit int.
/// Returns the parsed value and a boolean indicating success or failure.
/// const auto [number, state] = NumUtil::parseStrToInt64(str, offset);
inline std::pair<std::int64_t, StrToState> parseStrToInt64(const std::string_view str,
                                                             std::size_t& offset)
{
    return parseStrTo<std::int64_t>(str, offset);
}

/// Parse a string to unsigned 64-bit int.
/// Returns the parsed value and a boolean indicating success or failure.
/// const auto [number, state] = NumUtil::parseStrToUint64(str, offset);
inline std::pair<std::uint64_t, StrToState> parseStrToUint64(const std::string_view str,
                                                             std::size_t& offset)
{
    return parseStrTo<std::uint64_t>(str, offset);
}

/// Convert a string to 64-bit int.
/// Returns the parsed value and a boolean indicating success or failure.
inline std::pair<std::int64_t, bool> i64FromString(const std::string_view input)
{
    std::size_t offset = 0;
    const auto [value, status] = parseStrTo<std::int64_t>(input, offset);
    return { value, status <= StrToState::Partial };
}

/// Convert a string to 64-bit int. On failure, returns the default value.
/// Used where there is no interest in knowing whether the input was valid or not.
inline std::int64_t i64FromString(const std::string_view input, const std::int64_t def)
{
    return strTo<std::int64_t>(input, def);
}

/// Convert a string to 64-bit unsigned int.
/// Returns the parsed value and a boolean indicating success or failure.
inline std::pair<std::uint64_t, bool> u64FromString(const std::string_view input)
{
    std::size_t offset = 0;
    const auto [value, status] = parseStrTo<std::uint64_t>(input, offset);
    return { value, status <= StrToState::Partial };
}

/// Convert a string to 64-bit unsigned int. On failure, returns the default value.
/// Used where there is no interest in knowing whether the input was valid or not.
inline std::uint64_t u64FromString(const std::string_view input, const std::uint64_t def)
{
    return strTo<std::uint64_t>(input, def);
}

/**
* Similar to std::atoi() but does not require p to be null-terminated.
*
* Returns std::numeric_limits<int>::min/max() if the result would overflow.
*/
inline int safe_atoi(const char* p, int len)
{
    std::size_t offset = 0;
    const auto [value, res] = parseStrTo<std::int32_t>(std::string_view(p, len), offset);
    return value;
}

/// Fast string to 32-bit signed int conversion.
/// Optimized for performance with manual parsing and minimal branches.
/// Drop-in replacement to std::stoi() that accepts string_view.
inline std::int32_t stoi(const std::string_view str)
{
    std::size_t offset = 0;
    const auto [value, res] = parseStrToInt32(str, offset);
    if (offset == 0)
        throw std::invalid_argument("stoi");

    if (res == StrToState::Overflow)
        throw std::out_of_range("stoi");

    return value;
}

} // namespace NumUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
