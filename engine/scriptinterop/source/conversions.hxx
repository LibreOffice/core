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

#include <cmath>

#include <cpo/uno/RuntimeException.hpp>
#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>

namespace scriptinterop::detail
{
// The API works in points; the UNO drawing and table layers work in 1/100 mm.  A value that is
// not a finite number, or that falls outside the layer's integer range after the conversion, is
// an error.  The comparison is written so that a NaN input fails it too.
inline sal_Int32 pointsToHundredthMm(double points)
{
    auto const hundredthMm
        = std::round(o3tl::convert(points, o3tl::Length::pt, o3tl::Length::mm100));
    if (!(hundredthMm >= SAL_MIN_INT32 && hundredthMm <= SAL_MAX_INT32))
    {
        throw cpo::uno::RuntimeException(
            OUString::Concat(
                "expected a length in points that fits the page coordinate range, got ")
            + OUString::number(points));
    }
    return static_cast<sal_Int32>(hundredthMm);
}

// A size in points, such as a shape's width or a column's width.  The value must not be
// negative; the comparison is written so that a NaN input fails it too.
inline sal_Int32 extentToHundredthMm(double points)
{
    if (!(points >= 0))
    {
        throw cpo::uno::RuntimeException(
            OUString::Concat("expected a non-negative size in points, got ")
            + OUString::number(points));
    }
    return pointsToHundredthMm(points);
}

// The double keeps the fraction of a point that a whole number of 1/100 mm falls on.
inline double hundredthMmToPoints(sal_Int32 hundredthMm)
{
    return o3tl::convert(double(hundredthMm), o3tl::Length::mm100, o3tl::Length::pt);
}

// A column width in pixels, which is the unit SpreadsheetApp states one in.  A negative width,
// or one that falls outside the table layer's integer range after the conversion, is an error.
inline sal_Int32 pixelsToHundredthMm(sal_Int32 pixels)
{
    if (pixels < 0)
    {
        throw cpo::uno::RuntimeException(
            OUString::Concat("expected a non-negative width in pixels, got ")
            + OUString::number(pixels));
    }
    auto const hundredthMm = o3tl::convert(pixels, o3tl::Length::px, o3tl::Length::mm100);
    if (hundredthMm > SAL_MAX_INT32)
    {
        throw cpo::uno::RuntimeException(
            OUString::Concat("expected a width in pixels that fits the page coordinate range, got ")
            + OUString::number(pixels));
    }
    return static_cast<sal_Int32>(hundredthMm);
}

inline sal_Int32 parseHexColor(OUString const& hexColor)
{
    bool valid = hexColor.getLength() == 7 && hexColor[0] == '#';
    if (valid)
    {
        for (sal_Int32 i = 1; i != 7; ++i)
        {
            auto const c = hexColor[i];
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            {
                valid = false;
                break;
            }
        }
    }
    if (!valid)
    {
        throw cpo::uno::RuntimeException(
            OUString::Concat("expected a color in \"#rrggbb\" form, got ") + hexColor);
    }
    return static_cast<sal_Int32>(o3tl::toUInt32(hexColor.subView(1), 16));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
