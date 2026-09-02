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

#include <common/ContainerUtil.hpp>
#include <common/IpNetwork.hpp>

#include <cassert>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace RegexUtil
{

/// Return true if the subject matches in given set. It uses regex
/// Mainly used to match WOPI hosts patterns
bool matchRegex(const std::set<std::string>& set, const std::string& subject);

/// Return value from key:value pair if the subject matches in given map. It uses regex
/// Mainly used to match WOPI hosts patterns
std::string getValue(const std::map<std::string, std::string>& map, const std::string& subject);

std::string getValue(const std::set<std::string>& set, const std::string& subject);

bool isRegexValid(const std::string& regex);

/// Given one or more patterns to allow, and one or more to deny,
/// the match member will return true if, and only if, the subject
/// matches the allowed list, but not the deny.
/// A pattern is either a network in CIDR notation (10.0.0.0/8, fd00::/8),
/// which matches subjects that are IP addresses inside it, or a regular
/// expression that must match the whole subject.
/// By default, everything is denied.
class RegexListMatcher final
{
    static constexpr std::size_t MaxMemoizationSize = 16;

public:
    RegexListMatcher()
        : RegexListMatcher(false)
    {
    }

    explicit RegexListMatcher(const bool allowByDefault)
        : _allowByDefault(allowByDefault)
    {
        _matchedMemo.reserve(MaxMemoizationSize * 3);
    }

    void allow(const std::string& pattern)
    {
        _matchedMemo.clear();
        if (std::optional<Util::IpNetwork> network = Util::IpNetwork::parse(pattern))
            _allowedNetworks.push_back(std::move(*network));
        else
            _allowed.insert(pattern);
    }

    void deny(const std::string& pattern)
    {
        _matchedMemo.clear();
        if (std::optional<Util::IpNetwork> network = Util::IpNetwork::parse(pattern))
        {
            eraseNetwork(_allowedNetworks, pattern);
            _deniedNetworks.push_back(std::move(*network));
        }
        else
        {
            _allowed.erase(pattern);
            _denied.insert(pattern);
        }
    }

    void clear()
    {
        _matchedMemo.clear();
        _allowed.clear();
        _denied.clear();
        _allowedNetworks.clear();
        _deniedNetworks.clear();
    }

    /// Match the given value to the regex rules.
    bool match(const std::string& subject) const
    {
        // Check the memoized results; will match virtually always.
        if (const auto it = _matchedMemo.find(subject); it != _matchedMemo.end())
        {
            assert(matchImpl(subject) == it->second && "Inconsistent memoized match result");
            return it->second; // Return the memoized result.
        }

        const bool res = matchImpl(subject);
        if (_matchedMemo.size() < MaxMemoizationSize)
        {
            // Cap the memoization memory footprint.
            _matchedMemo[subject] = res;
        }

        return res;
    }

private:
    /// The matching logic, without memoization.
    bool matchImpl(const std::string& subject) const
    {
        return (_allowByDefault || matchRegex(_allowed, subject) ||
                matchNetwork(_allowedNetworks, subject)) &&
               !matchRegex(_denied, subject) && !matchNetwork(_deniedNetworks, subject);
    }

    static bool matchNetwork(const std::vector<Util::IpNetwork>& networks,
                             const std::string& subject)
    {
        for (const Util::IpNetwork& network : networks)
        {
            if (network.contains(subject))
                return true;
        }

        return false;
    }

    static void eraseNetwork(std::vector<Util::IpNetwork>& networks, const std::string& text)
    {
        for (std::vector<Util::IpNetwork>::iterator it = networks.begin(); it != networks.end();)
        {
            if (it->toString() == text)
                it = networks.erase(it);
            else
                ++it;
        }
    }

public:
    /// Whether a match exist in either the allowed or the denied rules.
    bool matchExist(const std::string& subject) const
    {
        return matchRegex(_allowed, subject) || matchRegex(_denied, subject) ||
               matchNetwork(_allowedNetworks, subject) || matchNetwork(_deniedNetworks, subject);
    }

    bool empty() const
    {
        return _allowed.empty() && _denied.empty() && _allowedNetworks.empty() &&
               _deniedNetworks.empty();
    }

private:
    std::set<std::string> _allowed;
    std::set<std::string> _denied;
    std::vector<Util::IpNetwork> _allowedNetworks;
    std::vector<Util::IpNetwork> _deniedNetworks;
    /// Memoizes the result of match().
    mutable Util::UnorderedStringMap<bool> _matchedMemo;
    const bool _allowByDefault;
};

} // namespace RegexUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
