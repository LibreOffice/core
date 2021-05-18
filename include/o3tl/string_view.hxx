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
#include <string_view>

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
    auto const n = sv.find(delimiter, position);
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
