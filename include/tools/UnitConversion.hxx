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
#include <tools/fldunit.hxx>
#include <tools/fract.hxx>
#include <tools/mapunit.hxx>

constexpr o3tl::Length FieldToO3tlLength(FieldUnit eU, o3tl::Length ePixelValue = o3tl::Length::px)
{
    switch (eU)
    {
        case FieldUnit::MM:
            return o3tl::Length::mm;
        case FieldUnit::CM:
            return o3tl::Length::cm;
        case FieldUnit::M:
            return o3tl::Length::m;
        case FieldUnit::KM:
            return o3tl::Length::km;
        case FieldUnit::TWIP:
            return o3tl::Length::twip;
        case FieldUnit::POINT:
            return o3tl::Length::pt;
        case FieldUnit::PICA:
            return o3tl::Length::pc;
        case FieldUnit::INCH:
            return o3tl::Length::in;
        case FieldUnit::FOOT:
            return o3tl::Length::ft;
        case FieldUnit::MILE:
            return o3tl::Length::mi;
        case FieldUnit::CHAR:
            return o3tl::Length::ch;
        case FieldUnit::LINE:
            return o3tl::Length::line;
        case FieldUnit::MM_100TH:
            return o3tl::Length::mm100;
        case FieldUnit::PIXEL:
            return ePixelValue;
        default:
            return o3tl::Length::invalid;
    }
}

constexpr o3tl::Length MapToO3tlLength(MapUnit eU, o3tl::Length ePixelValue = o3tl::Length::px)
{
    switch (eU)
    {
        case MapUnit::Map100thMM:
            return o3tl::Length::mm100;
        case MapUnit::Map10thMM:
            return o3tl::Length::mm10;
        case MapUnit::MapMM:
            return o3tl::Length::mm;
        case MapUnit::MapCM:
            return o3tl::Length::cm;
        case MapUnit::Map1000thInch:
            return o3tl::Length::in1000;
        case MapUnit::Map100thInch:
            return o3tl::Length::in100;
        case MapUnit::Map10thInch:
            return o3tl::Length::in10;
        case MapUnit::MapInch:
            return o3tl::Length::in;
        case MapUnit::MapPoint:
            return o3tl::Length::pt;
        case MapUnit::MapTwip:
            return o3tl::Length::twip;
        case MapUnit::MapPixel:
            return ePixelValue;
        default:
            return o3tl::Length::invalid;
    }
}

inline Fraction conversionFract(o3tl::Length from, o3tl::Length to)
{
    const auto & [ mul, div ] = o3tl::getConversionMulDiv(from, to);
    return { mul, div };
}

template <typename N> constexpr auto convertTwipToMm100(N n)
{
    return o3tl::convert(n, o3tl::Length::twip, o3tl::Length::mm100);
}
template <typename N> constexpr auto convertMm100ToTwip(N n)
{
    return o3tl::convert(n, o3tl::Length::mm100, o3tl::Length::twip);
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
