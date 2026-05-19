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

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace HexUtil
{

/// Convert unsigned char data to hex.
/// @buffer can be either std::vector<char> or std::string.
/// @offset the offset within the buffer to start from.
/// @length is the number of bytes to convert.
template <typename T>
inline std::string dataToHexString(const T& buffer, const std::size_t offset,
                                   const std::size_t length)
{
    char scratch[64];
    std::stringstream os;

    for (unsigned int i = 0; i < length; i++)
    {
        if ((offset + i) >= buffer.size())
            break;

        snprintf(scratch, sizeof(scratch), "%.2x", static_cast<unsigned char>(buffer[offset + i]));
        os << scratch;
    }

    return os.str();
}

/// Hex to unsigned char
template <typename T> bool dataFromHexString(const std::string_view hexString, T& data)
{
    if (hexString.length() % 2 != 0)
    {
        return false;
    }

    data.clear();
    std::stringstream stream;
    unsigned value;
    for (unsigned long offset = 0; offset < hexString.size(); offset += 2)
    {
        stream.clear();
        stream << std::hex << hexString.substr(offset, 2);
        stream >> value;
        data.push_back(static_cast<typename T::value_type>(value));
    }

    return true;
}

constexpr std::array<char, 2> hexFromByte(unsigned char byte)
{
    constexpr auto hex = "0123456789ABCDEF";
    return { hex[byte >> 4], hex[byte & 0xf] };
}

inline std::string bytesToHexString(const uint8_t* data, size_t size)
{
    std::string s;
    s.resize(size * 2); // Each byte is two hex digits.
    for (size_t i = 0; i < size; ++i)
    {
        const std::array<char, 2> hex = hexFromByte(data[i]);
        const size_t off = i * 2;
        s[off] = hex[0];
        s[off + 1] = hex[1];
    }

    return s;
}

inline std::string bytesToHexString(const char* data, size_t size)
{
    return bytesToHexString(reinterpret_cast<const uint8_t*>(data), size);
}

inline std::string bytesToHexString(const std::string_view str)
{
    return bytesToHexString(str.data(), str.size());
}

/// Maps a char to its hex value (0..15) or -1 for any non-hex char.
constexpr int hexDigitFromChar(char c)
{
    constexpr auto table = []
    {
        std::array<signed char, 256> t{};
        t.fill(-1);
        for (int i = 0; i <= 9; ++i)
            t['0' + i] = static_cast<signed char>(i);
        for (int i = 0; i < 6; ++i)
        {
            t['a' + i] = static_cast<signed char>(10 + i);
            t['A' + i] = static_cast<signed char>(10 + i);
        }
        return t;
    }();
    return table[static_cast<unsigned char>(c)];
}

inline std::string hexStringToBytes(const uint8_t* data, size_t size)
{
    assert(data && (size % 2 == 0) && "Invalid hex digits to convert.");

    std::string s;
    s.resize(size / 2); // Each pair of hex digits is a single byte.
    for (size_t i = 0; i < size; i += 2)
    {
        const int high = hexDigitFromChar(data[i]);
        assert(high >= 0 && high <= 16);
        const int low = hexDigitFromChar(data[i + 1]);
        assert(low >= 0 && low <= 16);
        const size_t off = i / 2;
        s[off] = ((high << 4) | low) & 0xff;
    }

    return s;
}

inline std::string hexStringToBytes(const char* data, size_t size)
{
    return hexStringToBytes(reinterpret_cast<const uint8_t*>(data), size);
}

inline std::string hexStringToBytes(const std::string_view str)
{
    return hexStringToBytes(str.data(), str.size());
}

/// Dump a line of data as hex.
/// @buffer can be either std::vector<char> or std::string.
/// @offset, the offset within the buffer to start from.
/// @width is the number of bytes to dump.
template <typename T>
inline std::string stringifyHexLine(const T& buffer, std::size_t offset,
                                    const std::size_t width = 32)
{
    std::string str;
    str.reserve(width * 4 + width / 8 + 3 + 1);

    for (unsigned int i = 0; i < width; i++)
    {
        if (i && (i % 8) == 0)
            str.push_back(' ');
        if ((offset + i) < buffer.size())
        {
            const std::array<char, 2> hex = hexFromByte(buffer[offset + i]);
            str.push_back(hex[0]);
            str.push_back(hex[1]);
            str.push_back(' ');
        }
        else
            str.append(3, ' ');
    }
    str.append(" | ");

    for (unsigned int i = 0; i < width; i++)
    {
        if ((offset + i) < buffer.size())
            str.push_back(::isprint(buffer[offset + i]) ? buffer[offset + i] : '.');
        else
            str.push_back(' '); // Leave blank if we are out of data.
    }

    return str;
}

/// Dump data as hex and chars to stream.
/// @buffer can be either std::vector<char> or std::string.
/// @legend is streamed into @os before the hex data once.
/// @prefix is streamed into @os for each line.
/// @skipDup, when true,  will avoid writing identical lines.
/// @width is the number of bytes to dump per line.
template <typename T>
inline void dumpHex(std::ostream& os, const T& buffer, const char* legend = "",
                    const char* prefix = "", bool skipDup = true, const unsigned int width = 32)
{
    unsigned int j;
    char scratch[64];
    int skip = 0;
    std::string lastLine;

    os << legend;
    for (j = 0; j < buffer.size() + width - 1; j += width)
    {
        snprintf(scratch, sizeof(scratch), "%s0x%.4x  ", prefix, j);
        os << scratch;

        std::string line = stringifyHexLine(buffer, j, width);
        if (skipDup && lastLine == line)
            skip++;
        else
        {
            if (skip > 0)
            {
                os << "... dup " << skip - 1 << "...";
                skip = 0;
            }
            else
                os << line;
        }
        lastLine.swap(line);

        os << '\n';
    }
    os.flush();
}

/// Dump data as hex and chars into a string.
/// Primarily used for logging.
template <typename T>
inline std::string dumpHex(const T& buffer, const char* legend = "", const char* prefix = "",
                           bool skipDup = true, const unsigned int width = 32)
{
    std::ostringstream oss;
    dumpHex(oss, buffer, legend, prefix, skipDup, width);
    return oss.str();
}

inline std::string dumpHex(const char* legend, const char* prefix,
                           const std::vector<char>::iterator& startIt,
                           const std::vector<char>::iterator& endIt, bool skipDup = true,
                           const unsigned int width = 32)
{
    std::ostringstream oss;
    std::vector<char> data(startIt, endIt);
    dumpHex(oss, data, legend, prefix, skipDup, width);
    return oss.str();
}

/// Hex-encode an integral ID into a buffer, with padding support.
/// If @size is smaller than the encoded form needs, high-order hex digits are
/// silently truncated to fit. A @size of 0 returns an empty view.
inline std::string_view encodeId(char* buffer, std::size_t size, const std::uint64_t number,
                                 int width, char pad = '0')
{
    if (buffer == nullptr || size == 0)
    {
        return std::string_view();
    }

    // Skip leading (high-order) zeros, if any. For number == 0 we stop at
    // highNibble == 0 so we still emit a single '0' digit below.
    int highNibble = (2 * sizeof(number) - 1) * 4;
    while ((number & (std::uint64_t(0xf) << highNibble)) == 0)
    {
        highNibble -= 4;
        if (highNibble <= 0)
            break;
    }

    // Pad, if necessary.
    highNibble = std::min<int>(size - 1, highNibble / 4) * 4;
    width = std::min<int>(size, width);
    int outIndex = 0;
    const int hexDigits = (highNibble / 4) + 1;
    for (; width > hexDigits; --width)
    {
        buffer[outIndex++] = pad;
    }

    // Hexify the remaining digits. We emit two chars per iteration when the
    // remaining bit count is byte-aligned, and a single leading char otherwise.
    constexpr const char* const Hex = "0123456789abcdef";
    int bits = highNibble + 4; // total bits of @number still to emit
    if ((bits & 7) != 0) // odd hex digits remaining: emit the leading nibble
    {
        bits -= 4;
        const auto nibble = static_cast<unsigned char>((number >> bits) & 0xf);
        buffer[outIndex++] = Hex[nibble];
    }

    while (bits > 0)
    {
        bits -= 8;
        const auto byte = static_cast<unsigned char>((number >> bits) & 0xff);
        buffer[outIndex++] = Hex[byte >> 4];
        buffer[outIndex++] = Hex[byte & 0xf];
    }

    // Return a view over the given buffer.
    return std::string_view(buffer, outIndex);
}

/// Hex-encode an integral ID into a string, with padding support.
inline std::string encodeId(const std::uint64_t number, int width = 5, char pad = '0')
{
    char buffer[32];
    return std::string(encodeId(buffer, sizeof(buffer), number, width, pad));
}

/// Hex-encode an integral ID into a stream, with padding support.
inline std::ostringstream& encodeId(std::ostringstream& oss, const std::uint64_t number,
                                    int width = 5, char pad = '0')
{
    char buffer[32];
    oss << encodeId(buffer, sizeof(buffer), number, width, pad);
    return oss;
}

/// Decode the hex-string into an ID. The reverse of encodeId().
/// Returns 0 if @str is empty or has no parseable hex prefix.
inline std::uint64_t decodeId(const std::string_view str)
{
    std::uint64_t id = 0;
    std::from_chars(str.data(), str.data() + str.size(), id, 16);
    return id;
}

} // namespace HexUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
