/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <array>
#include <oox/export/ColorExportUtils.hxx>
#include <docmodel/color/ComplexColor.hxx>

namespace oox
{
double convertColorTransformsToTintOrShade(model::ComplexColor const& rComplexColor)
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

sal_Int32 convertThemeColorTypeToExcelThemeNumber(model::ThemeColorType eType)
{
    if (eType == model::ThemeColorType::Unknown)
        return -1;

    // Change position of text1 and text2 and background1 and background2 - needed because of an bug in excel, where
    // the text and background index positions are switched.
    // 0 -> 1, 1 -> 0
    // 2 -> 3, 3 -> 2
    // everything else stays the same
    static constexpr std::array<sal_Int32, 12> constThemeColorMapToXmlMap
        = { 1, 0, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11 };

    return constThemeColorMapToXmlMap[sal_Int32(eType)];
}
} // end oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
