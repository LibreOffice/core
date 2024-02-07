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

#include <compare>

#include <config_global.h>

namespace o3tl
{
// A poor approximation of C++20 <compare> std::strong_order, falling back to operator <=> (so e.g.
// not providing a strict weak ordering for floating-point types with NaN):
#if HAVE_CPP_STRONG_ORDER

inline constexpr auto strong_order = std::strong_order;

#else

namespace detail
{
struct strong_order
{
    auto operator()(auto x, auto y) const { return x <=> y; }
};
}

inline constexpr auto strong_order = detail::strong_order();

#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
