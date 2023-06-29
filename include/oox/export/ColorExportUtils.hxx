/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <docmodel/color/ComplexColor.hxx>

namespace oox
{
static double convertColorTransformsToTintOrShade(model::ComplexColor const& rComplexColor)
{
    sal_Int16 nLumMod = 10'000;
    sal_Int16 nLumOff = 0;

    for (auto const& rTransform : rComplexColor.getTransformations())
    {
        if (rTransform.meType == model::TransformationType::LumMod)
            nLumMod = rTransform.mnValue;
        if (rTransform.meType == model::TransformationType::LumOff)
            nLumOff = rTransform.mnValue;
    }

    if (nLumMod == 10'000 && nLumOff == 0)
        return 0.0;

    double fTint = 0.0;

    if (nLumOff > 0) // tint
        fTint = double(nLumOff) / 10'000.0;
    else
        fTint = -double(10'000 - nLumMod) / 10'000.0;

    return fTint;
}

static sal_Int32 convertThemeColorTypeToExcelThemeNumber(model::ThemeColorType eType)
{
    if (eType == model::ThemeColorType::Unknown)
        return -1;

    constexpr std::array<sal_Int32, 12> constMap = { 1, 0, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11 };

    return constMap[sal_Int32(eType)];
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
