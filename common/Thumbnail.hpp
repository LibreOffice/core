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

#include <cstddef>
#include <optional>
#include <string_view>
#include <utility>

/// The thumbnails of a document that a host can ask for with each upload of it.
namespace Thumbnail
{
/// The least and the greatest length, in pixels, of a side of the box a thumbnail fits in.
constexpr int MinSide = 16;
constexpr int MaxSide = 2048;

/// The greatest number of thumbnails of one document.
constexpr std::size_t MaxCount = 4;

/// Parses the size of a box written as "<W>x<H>", with each side a decimal integer without
/// leading zeros from MinSide to MaxSide. Returns the width and the height, or nothing when
/// the text is not of that form.
inline std::optional<std::pair<int, int>> parseSize(std::string_view size)
{
    const auto side = [](std::string_view text) -> std::optional<int>
    {
        // Four digits hold MaxSide, so a longer text is out of range before it is read.
        if (text.empty() || text.size() > 4 || text[0] == '0')
            return std::nullopt;

        int value = 0;
        for (const char digit : text)
        {
            if (digit < '0' || digit > '9')
                return std::nullopt;
            value = value * 10 + (digit - '0');
        }

        if (value < MinSide || value > MaxSide)
            return std::nullopt;
        return value;
    };

    const std::size_t separator = size.find('x');
    if (separator == std::string_view::npos)
        return std::nullopt;

    const std::optional<int> width = side(size.substr(0, separator));
    const std::optional<int> height = side(size.substr(separator + 1));
    if (!width || !height)
        return std::nullopt;

    return std::make_pair(*width, *height);
}

} // namespace Thumbnail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
