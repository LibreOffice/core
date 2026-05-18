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

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Util
{

/// Transparent hasher for std::string keys.
/// Hashes std::string, std::string_view, and const char* to the same value.
/// Enables heterogeneous lookup in associative containers when paired with
/// a transparent key-equal (e.g. std::equal_to<>).
struct TransparentStringHash
{
    using is_transparent = void;

    std::size_t operator()(std::string_view s) const noexcept
    {
        return std::hash<std::string_view>{}(s);
    }

    std::size_t operator()(const std::string& s) const noexcept
    {
        return std::hash<std::string_view>{}(s);
    }

    std::size_t operator()(const char* s) const noexcept
    {
        return std::hash<std::string_view>{}(s);
    }
};

/// std::unordered_map keyed on std::string with heterogeneous lookup.
/// find/count/contains/equal_range accept std::string_view and const char*
/// without constructing a temporary std::string.
template <typename Value>
using UnorderedStringMap =
    std::unordered_map<std::string, Value, TransparentStringHash, std::equal_to<>>;

} // namespace Util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
