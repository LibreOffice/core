/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <config_options.h>
#include <svx/svxdllapi.h>
#include <rtl/ustring.hxx>
#include <array>
#include <memory>
#include <tools/color.hxx>
#include <docmodel/theme/ThemeColorType.hxx>

namespace model
{
class ColorSet;
}
namespace tools
{
class JsonWriter;
}

namespace svx
{
struct SVXCORE_DLLPUBLIC ThemePaletteEffect
{
    Color maColor;
    OUString maColorName;
    sal_Int16 mnLumOff = 0;
    sal_Int16 mnLumMod = 10'000;

    sal_Int16 getPercentage()
    {
        if (mnLumOff > 0)
            return mnLumOff / 100;
        else
            return (-10'000 + mnLumMod) / 100;
    }
};

struct SVXCORE_DLLPUBLIC ThemePaletteColorData
{
    Color maBaseColor;
    model::ThemeColorType meThemeColorType = model::ThemeColorType::Unknown;
    std::array<ThemePaletteEffect, 6> maEffects;

    sal_Int16 getLumMod(sal_uInt16 nEffect) const { return maEffects[nEffect].mnLumMod; }
    sal_Int16 getLumOff(sal_uInt16 nEffect) const { return maEffects[nEffect].mnLumOff; }
};

struct SVXCORE_DLLPUBLIC ThemePaletteCollection
{
    std::array<ThemePaletteColorData, 12> maColors;
};

class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) ThemeColorPaletteManager final
{
    std::shared_ptr<model::ColorSet> m_pColorSet;

public:
    ThemeColorPaletteManager(std::shared_ptr<model::ColorSet> const& pColorSet);
    ThemePaletteCollection generate();
    void generateJSON(tools::JsonWriter& aTree);
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
