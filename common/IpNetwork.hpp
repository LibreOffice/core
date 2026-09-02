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

/*
 * IP network (CIDR) parsing and address matching.
 * Classes: Util::IpNetwork
 *
 * Self-contained on purpose: this header is reached from RegexUtil.hpp on
 * every platform, including the desktop app on Windows, which fakes its
 * sockets and cannot include winsock or the Poco socket headers.
 */

#pragma once

#include <array>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace Util
{

/// An IP network given in CIDR notation, such as 100.64.0.0/10 or fd00::/8.
/// IPv4 networks also contain the IPv4-mapped IPv6 form of their addresses
/// (::ffff:100.64.1.2), which is how a dual-stack listening socket reports
/// IPv4 peers.
class IpNetwork
{
public:
    /// Parses CIDR notation. Returns an empty optional unless the text is an
    /// IP address, a slash, and a prefix length valid for that address family.
    static std::optional<IpNetwork> parse(const std::string& cidr)
    {
        const std::size_t slash = cidr.find('/');
        if (slash == std::string::npos || slash == 0 || slash + 1 >= cidr.size())
            return std::nullopt;

        const std::string_view prefix = std::string_view(cidr).substr(slash + 1);
        const char* const prefixEnd = prefix.data() + prefix.size();
        unsigned prefixLength = 0;
        const std::from_chars_result parsed =
            std::from_chars(prefix.data(), prefixEnd, prefixLength);
        if (parsed.ec != std::errc() || parsed.ptr != prefixEnd)
            return std::nullopt;

        std::array<unsigned char, 16> bytes{};
        const std::size_t length = parseAddress(std::string_view(cidr).substr(0, slash), bytes);
        if (length == 0 || prefixLength > length * 8)
            return std::nullopt;

        return IpNetwork(bytes, length, prefixLength, cidr);
    }

    /// Returns true if the given textual IP address belongs to this network.
    /// Non-address input (host names, garbage) is never contained.
    bool contains(const std::string& address) const
    {
        std::array<unsigned char, 16> bytes{};
        const std::size_t length = parseAddress(address, bytes);
        if (length == 0 || length != _length)
            return false;

        return prefixEqual(_address, bytes, _prefixLength);
    }

    const std::string& toString() const { return _text; }

private:
    IpNetwork(const std::array<unsigned char, 16>& address, std::size_t length,
              unsigned prefixLength, std::string text)
        : _address(address)
        , _length(length)
        , _prefixLength(prefixLength)
        , _text(std::move(text))
    {
    }

    /// Strict dotted-decimal IPv4: four parts of one to three digits, each at most 255.
    static bool parseIPv4(std::string_view text, unsigned char* out)
    {
        const char* pos = text.data();
        const char* const end = text.data() + text.size();
        for (int part = 0; part < 4; ++part)
        {
            unsigned value = 0;
            const std::from_chars_result parsed = std::from_chars(pos, end, value);
            const std::ptrdiff_t digits = parsed.ptr - pos;
            if (parsed.ec != std::errc() || digits > 3 || value > 255)
                return false;
            out[part] = static_cast<unsigned char>(value);
            pos = parsed.ptr;
            if (part < 3)
            {
                if (pos == end || *pos != '.')
                    return false;
                ++pos;
            }
        }
        return pos == end;
    }

    /// Colon-separated 16-bit hex groups. When allowIPv4Tail is set the last
    /// group may instead be a dotted-decimal IPv4 address, counting as two groups.
    static bool parseHexGroups(std::string_view text, std::vector<unsigned>& groups,
                               bool allowIPv4Tail)
    {
        if (text.empty())
            return true;

        std::size_t start = 0;
        while (true)
        {
            const std::size_t colon = text.find(':', start);
            const std::string_view token = text.substr(
                start, colon == std::string_view::npos ? std::string_view::npos : colon - start);
            if (token.empty())
                return false;

            if (colon == std::string_view::npos && allowIPv4Tail &&
                token.find('.') != std::string_view::npos)
            {
                unsigned char v4[4];
                if (!parseIPv4(token, v4))
                    return false;
                groups.push_back((v4[0] << 8) | v4[1]);
                groups.push_back((v4[2] << 8) | v4[3]);
                return true;
            }

            if (token.size() > 4)
                return false;
            unsigned value = 0;
            const char* const tokenEnd = token.data() + token.size();
            const std::from_chars_result parsed =
                std::from_chars(token.data(), tokenEnd, value, 16);
            if (parsed.ec != std::errc() || parsed.ptr != tokenEnd)
                return false;
            groups.push_back(value);

            if (colon == std::string_view::npos)
                return true;
            start = colon + 1;
        }
    }

    /// IPv6 text form: eight hex groups, optionally with one "::" run of zero
    /// groups and optionally with an embedded IPv4 tail.
    static bool parseIPv6(std::string_view text, unsigned char* out)
    {
        std::vector<unsigned> left;
        std::vector<unsigned> right;
        const std::size_t twoColons = text.find("::");
        if (twoColons != std::string_view::npos)
        {
            if (text.find("::", twoColons + 1) != std::string_view::npos)
                return false;
            if (!parseHexGroups(text.substr(0, twoColons), left, false))
                return false;
            if (!parseHexGroups(text.substr(twoColons + 2), right, true))
                return false;
            if (left.size() + right.size() > 7)
                return false;
        }
        else if (!parseHexGroups(text, left, true) || left.size() != 8)
        {
            return false;
        }

        unsigned groups[8] = {};
        for (std::size_t i = 0; i < left.size(); ++i)
            groups[i] = left[i];
        for (std::size_t i = 0; i < right.size(); ++i)
            groups[8 - right.size() + i] = right[i];
        for (int i = 0; i < 8; ++i)
        {
            out[2 * i] = static_cast<unsigned char>(groups[i] >> 8);
            out[2 * i + 1] = static_cast<unsigned char>(groups[i] & 0xff);
        }
        return true;
    }

    /// Parses an IP address of either family and returns its length in bytes,
    /// or 0 if the text is not an address. A host in URL brackets is accepted.
    /// IPv4-mapped IPv6 addresses are folded to IPv4 so that both spellings of
    /// one peer compare equal.
    static std::size_t parseAddress(std::string_view text, std::array<unsigned char, 16>& out)
    {
        if (text.size() >= 2 && text.front() == '[' && text.back() == ']')
            text = text.substr(1, text.size() - 2);

        if (text.find(':') == std::string_view::npos)
            return parseIPv4(text, out.data()) ? 4 : 0;

        if (!parseIPv6(text, out.data()))
            return 0;

        static const unsigned char mappedPrefix[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };
        if (std::memcmp(out.data(), mappedPrefix, sizeof(mappedPrefix)) == 0)
        {
            std::memmove(out.data(), out.data() + 12, 4);
            return 4;
        }
        return 16;
    }

    /// Returns true if the first prefixLength bits of the two byte arrays are equal.
    static bool prefixEqual(const std::array<unsigned char, 16>& a,
                            const std::array<unsigned char, 16>& b, unsigned prefixLength)
    {
        const unsigned fullBytes = prefixLength / 8;
        if (std::memcmp(a.data(), b.data(), fullBytes) != 0)
            return false;

        const unsigned remainingBits = prefixLength % 8;
        if (remainingBits == 0)
            return true;

        const unsigned char mask = static_cast<unsigned char>(0xff << (8 - remainingBits));
        return (a[fullBytes] & mask) == (b[fullBytes] & mask);
    }

    /// Network address, IPv4 in the first 4 bytes, IPv6 in all 16.
    std::array<unsigned char, 16> _address;
    /// Address length in bytes: 4 for IPv4, 16 for IPv6.
    std::size_t _length;
    /// Number of leading bits that identify the network.
    unsigned _prefixLength;
    /// The canonical CIDR text, for logging.
    std::string _text;
};

} // namespace Util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
