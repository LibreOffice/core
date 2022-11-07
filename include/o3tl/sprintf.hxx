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
#include <cstdio>
#include <utility>

#include <o3tl/safeint.hxx>

namespace o3tl
{
// A drop-in replacement for safe uses of std::sprintf where it is statically known that the
// provided buffer is large enough.  Compared to a plain use of std::sprintf, using o3tl::sprintf
// for one makes it explicit that the call is considered safe and for another avoids deprecation
// warnings on platforms like the macOS 13 SDK that mark std::sprintf as deprecated.  Many simple
// uses of std::sprintf across the code base can be replaced with alternative code using e.g.
// OString::number.  This is for the remaining formatting-rich cases for which there is no easy
// replacement yet in our C++17 baseline.  Ultimately, it should be removed again once alternatives
// for those remaining cases, like C++20 std::format, are available.
template <std::size_t N, typename... T>
int sprintf(char (&s)[N], char const* format, T&&... arguments)
{
    auto const n = std::snprintf(s, N, format, std::forward<T>(arguments)...);
    assert(n < 0 || o3tl::make_unsigned(n) < N);
    (void)n;
    return n;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
