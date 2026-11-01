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

#include <common/Common.hpp>
#include <common/FileUtil.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// The thumbnails of a document that a host can ask for with each upload of it.
namespace Thumbnail
{
/// The least and the greatest length, in pixels, of a side of the box a thumbnail fits in.
constexpr int MinSide = 16;
constexpr int MaxSide = 2048;

/// The greatest number of thumbnails of one document.
constexpr std::size_t MaxCount = 4;

/// Returns the path of the thumbnail of the file at basePath, where size is the pixel size of
/// the thumbnail as "<W>x<H>".
inline std::string filePath(const std::string& basePath, const std::string& size)
{
    return basePath + THUMBNAIL_SUFFIX + size + ".png";
}

/// Returns the path of the thumbnail of the file at basePath that is width by height pixels.
inline std::string filePath(const std::string& basePath, int width, int height)
{
    return filePath(basePath, std::to_string(width) + 'x' + std::to_string(height));
}

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

/// Parses sizes written as "<W>x<H>" and separated by commas, as "1000x1000,256x256".
/// Returns the sizes that parseSize reads, in their order, at most MaxCount of them. An entry
/// it cannot read is left out.
inline std::vector<std::pair<int, int>> parseSizeList(std::string_view list)
{
    std::vector<std::pair<int, int>> sizes;
    while (!list.empty() && sizes.size() < MaxCount)
    {
        const std::size_t comma = list.find(',');
        const std::string_view entry = list.substr(0, comma);
        list = (comma == std::string_view::npos) ? std::string_view() : list.substr(comma + 1);

        if (const std::optional<std::pair<int, int>> size = parseSize(entry))
            sizes.push_back(*size);
    }

    return sizes;
}

/// Returns the sizes, each as "<W>x<H>", of the thumbnails of the file at basePath that are on
/// disk: the regular files whose names are what filePath gives for a size that parseSize reads.
/// The sizes come in the order of the names.
inline std::vector<std::string> listSizes(const std::string& basePath)
{
    const std::size_t slash = basePath.rfind('/');
    const std::string directory = slash == std::string::npos ? "." : basePath.substr(0, slash);
    const std::string prefix =
        (slash == std::string::npos ? basePath : basePath.substr(slash + 1)) + THUMBNAIL_SUFFIX;
    static constexpr std::string_view Extension = ".png";

    std::vector<std::string> sizes;
    for (const std::string& name : FileUtil::getDirEntries(directory))
    {
        if (name.size() <= prefix.size() + Extension.size() || !name.starts_with(prefix) ||
            !name.ends_with(Extension))
        {
            continue;
        }

        const std::string size =
            name.substr(prefix.size(), name.size() - prefix.size() - Extension.size());
        if (parseSize(size) && FileUtil::Stat(directory + '/' + name).isFile())
            sizes.push_back(size);
    }

    std::sort(sizes.begin(), sizes.end());
    return sizes;
}

} // namespace Thumbnail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
