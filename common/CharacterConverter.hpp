/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/Log.hpp>

#include <cstdint>
#include <iconv.h>
#include <string>
#include <vector>

namespace Util
{

/// Using iconv(3) API, converts strings between character encodings.
class CharacterConverter
{
public:
    /// Create an instance to convert @from encoding @to encoding.
    /// Note that the order of these two arguments are reversed,
    /// compared to iconv_open(3).
    CharacterConverter(const std::string& from, const std::string& to)
        : _iconvd(iconv_open(to.data(), from.data()))
    {
        if (reinterpret_cast<int64_t>(_iconvd) == -1)
        {
            LOG_SYS("Failed to initialize iconv to convert from ["
                    << from << "] to [" << to << "] and will return the source string unmodified");
        }
    }

    ~CharacterConverter()
    {
        if (reinterpret_cast<int64_t>(_iconvd) != -1)
        {
            iconv_close(_iconvd);
        }
    }

    /// Convert the given source string to the desired encoding.
    std::string convert(const std::string& source) const
    {
        if (reinterpret_cast<int64_t>(_iconvd) == -1)
        {
            LOG_WRN("Failed to initize iconv and cannot convert [" << source << ']');
            return source;
        }

        const char* in = source.data();
        std::size_t in_left = source.size();

        std::vector<char> buffer(8 * source.size());
        char* out = buffer.data();
        std::size_t out_left = buffer.size();

        // Convert.
        if (iconv(_iconvd, const_cast<char**>(&in), &in_left, &out, &out_left) == static_cast<size_t>(-1))
        {
            LOG_ERR("Failed to convert [" << source << ']');
            return source;
        }

        // Flush.
        if (iconv(_iconvd, nullptr, nullptr, &out, &out_left) == static_cast<size_t>(-1))
        {
            LOG_ERR("Failed to flush [" << source << ']');
            return source;
        }

        return std::string(buffer.data(), buffer.size() - out_left);
    }

private:
    /// The character-set conversion descriptor.
    iconv_t _iconvd;
};

} // end namespace Util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
