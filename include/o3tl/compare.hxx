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
#include <concepts>
#include <cstdint>
#include <cstring>
#include <limits>

#include <config_global.h>

namespace o3tl
{
// An approximation of C++20 <compare> std::strong_order that should work at least for IEC559 float
// and double on platforms that have correspondingly-sized std::int32_t and std::int64_t:
#if HAVE_CPP_STRONG_ORDER

inline constexpr auto strong_order = std::strong_order;

#else

namespace detail
{
template<typename To, typename From> To bit_cast(From val) requires (sizeof (To) == sizeof (From)) {
    char buf alignas(To)[sizeof (From)];
    std::memcpy(buf, &val, sizeof (From));
    return *reinterpret_cast<To const *>(buf);
}

struct strong_order
{
    template <typename T> auto operator ()(T x, T y) const
        requires std::same_as<T, float> && std::numeric_limits<T>::is_iec559
    {
        return bit_cast<std::int32_t>(x) <=> bit_cast<std::int32_t>(y);
    }

    template <typename T> auto operator ()(T x, T y) const
        requires std::same_as<T, double> && std::numeric_limits<T>::is_iec559
    {
        return bit_cast<std::int64_t>(x) <=> bit_cast<std::int64_t>(y);
    }
};
}

inline constexpr auto strong_order = detail::strong_order();

#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
