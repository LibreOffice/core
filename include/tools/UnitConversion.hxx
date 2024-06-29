/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <o3tl/unit_conversion.hxx>
#include <sal/types.h>
#include <tools/fract.hxx>

inline Fraction conversionFract(o3tl::Length from, o3tl::Length to)
{
    const auto & [ mul, div ] = o3tl::getConversionMulDiv(from, to);
    return { mul, div };
}

template <typename N> constexpr auto convertTwipToMm100(N n)
{
    using namespace o3tl; // Allow ADL find overloads e.g. in <tools/gen.hxx>
    return convert(n, o3tl::Length::twip, o3tl::Length::mm100);
}

constexpr sal_Int64 sanitiseMm100ToTwip(sal_Int64 n)
{
    return o3tl::convertSaturate(n, o3tl::Length::mm100, o3tl::Length::twip);
}

template <typename N> constexpr auto convertPointToMm100(N n)
{
    return o3tl::convert(n, o3tl::Length::pt, o3tl::Length::mm100);
}
template <typename N> constexpr auto convertMm100ToPoint(N n)
{
    return o3tl::convert(n, o3tl::Length::mm100, o3tl::Length::pt);
}

// PPT's "master unit" (1/576 inch) <=> mm/100
template <typename N> constexpr auto convertMasterUnitToMm100(N n)
{
    return o3tl::convert(n, o3tl::Length::master, o3tl::Length::mm100);
}
template <typename N> constexpr auto convertMm100ToMasterUnit(N n)
{
    return o3tl::convert(n, o3tl::Length::mm100, o3tl::Length::master);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
