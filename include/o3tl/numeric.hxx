/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <stdexcept>
#include <o3tl/concepts.hxx>

namespace o3tl
{

struct divide_by_zero final : public std::runtime_error
{
    explicit divide_by_zero()
        : std::runtime_error("divide by zero")
    {
    }
};

template <o3tl::integral T, o3tl::integral U>
constexpr T convertToHex(U aChar)
{
    if (aChar >= '0' && aChar <= '9')
        return T(aChar - '0');
    else if (aChar >= 'a' && aChar <= 'f')
        return T(aChar - 'a' + 10);
    else if (aChar >= 'A' && aChar <= 'F')
        return T(aChar - 'A' + 10);
    return T(-1);
}

template <o3tl::integral T, o3tl::integral U>
constexpr T convertToHex(U cHigh, U cLow)
{
    return (o3tl::convertToHex<T>(cHigh) << 4) | o3tl::convertToHex<T>(cLow);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
