/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/theme/ThemeColorPaletteManager.hxx>

#include <basegfx/color/bcolortools.hxx>
#include <tools/color.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/color/ComplexColorJSON.hxx>
#include <tools/json_writer.hxx>

#include <array>

namespace
{
constexpr const std::array<const std::array<sal_Int16, 6>, 5> g_aLumMods = {
    std::array<sal_Int16, 6>{ 10'000, 5'000, 6'500, 7'500, 8'500, 9'500 },
    std::array<sal_Int16, 6>{ 10'000, 1'000, 2'500, 5'000, 7'500, 9'000 },
    std::array<sal_Int16, 6>{ 10'000, 2'000, 4'000, 6'000, 7'500, 5'000 },
    std::array<sal_Int16, 6>{ 10'000, 9'000, 7'500, 5'000, 2'500, 1'000 },
    std::array<sal_Int16, 6>{ 10'000, 9'500, 8'500, 7'500, 6'500, 5'000 },
};

constexpr const std::array<const std::array<sal_Int16, 6>, 5> g_aLumOffs = {
    std::array<sal_Int16, 6>{ 0, 5'000, 3'500, 2'500, 1'500, 0'500 },
    std::array<sal_Int16, 6>{ 0, 9'000, 7'500, 5'000, 2'500, 1'000 },
    std::array<sal_Int16, 6>{ 0, 8'000, 6'000, 4'000, 0, 0 },
    std::array<sal_Int16, 6>{ 0, 0, 0, 0, 0, 0 },
    std::array<sal_Int16, 6>{ 0, 0, 0, 0, 0, 0 },
};

} // end anonymous namespace

namespace svx
{
ThemeColorPaletteManager::ThemeColorPaletteManager(
    std::shared_ptr<model::ColorSet> const& pColorSet)
    : m_pColorSet(pColorSet)
{
}

svx::ThemePaletteCollection ThemeColorPaletteManager::generate()
{
    svx::ThemePaletteCollection aThemePaletteCollection;

    const std::array<OUString, 12> aColorNames = {
        SvxResId(RID_SVXSTR_THEME_COLOR1),  SvxResId(RID_SVXSTR_THEME_COLOR2),
        SvxResId(RID_SVXSTR_THEME_COLOR3),  SvxResId(RID_SVXSTR_THEME_COLOR4),
        SvxResId(RID_SVXSTR_THEME_COLOR5),  SvxResId(RID_SVXSTR_THEME_COLOR6),
        SvxResId(RID_SVXSTR_THEME_COLOR7),  SvxResId(RID_SVXSTR_THEME_COLOR8),
        SvxResId(RID_SVXSTR_THEME_COLOR9),  SvxResId(RID_SVXSTR_THEME_COLOR10),
        SvxResId(RID_SVXSTR_THEME_COLOR11), SvxResId(RID_SVXSTR_THEME_COLOR12),
    };

    for (size_t nColor = 0; nColor < aColorNames.size(); ++nColor)
    {
        auto eThemeType = model::convertToThemeColorType(nColor);
        if (eThemeType == model::ThemeColorType::Unknown)
            continue;

        auto& aThemeColorData = aThemePaletteCollection.maColors[nColor];
        aThemeColorData.meThemeColorType = eThemeType;

        Color aThemeColor = m_pColorSet->getColor(eThemeType);
        aThemeColorData.maBaseColor = aThemeColor;

        basegfx::BColor aHSLColor = basegfx::utils::rgb2hsl(aThemeColor.getBColor());
        double aLuminanceValue = aHSLColor.getBlue() * 255.0;

        for (size_t nEffect : { 0, 1, 2, 3, 4, 5 })
        {
            auto& rEffect = aThemeColorData.maEffects[nEffect];
            size_t nIndex = 0;

            if (aLuminanceValue < 0.5)
                nIndex = 0; // Black
            else if (aLuminanceValue > 254.5)
                nIndex = 4; // White
            else if (aLuminanceValue < 50.5)
                nIndex = 1; // Low
            else if (aLuminanceValue > 203.5)
                nIndex = 3; // High
            else
                nIndex = 2; // Middle

            rEffect.mnLumOff = g_aLumOffs[nIndex][nEffect];
            rEffect.mnLumMod = g_aLumMods[nIndex][nEffect];

            rEffect.maColor = aThemeColor;
            rEffect.maColor.ApplyLumModOff(rEffect.mnLumMod, rEffect.mnLumOff);

            OUString aColorName;
            sal_Int16 nPercent = rEffect.getPercentage();

            OUString aTemplate;
            if (nPercent > 0)
            {
                aTemplate = SvxResId(RID_SVXSTR_THEME_EFFECT_LIGHTER);
            }
            else if (nPercent < 0)
            {
                aTemplate = SvxResId(RID_SVXSTR_THEME_EFFECT_DARKER);
            }

            if (!aTemplate.isEmpty())
            {
                aColorName = aTemplate.replaceAll("$THEME_NAME", aColorNames[nColor]);
                aColorName
                    = aColorName.replaceAll("$PERCENTAGE", OUString::number(std::abs(nPercent)));
            }
            else
            {
                aColorName = aColorNames[nColor];
            }
            rEffect.maColorName = aColorName;
        }
    }
    return aThemePaletteCollection;
}

void ThemeColorPaletteManager::generateJSON(tools::JsonWriter& aTree)
{
    svx::ThemePaletteCollection aThemePaletteCollection = generate();

    auto aColorListTree = aTree.startArray("ThemeColors");

    for (size_t nEffect = 0; nEffect < 6; ++nEffect)
    {
        auto aColorRowTree = aTree.startAnonArray();
        for (size_t nIndex = 0; nIndex < 12; ++nIndex)
        {
            auto aColorTree = aTree.startStruct();

            auto const& rColorData = aThemePaletteCollection.maColors[nIndex];
            auto const& rEffectData = rColorData.maEffects[nEffect];

            aTree.put("Value", rEffectData.maColor.AsRGBHexString().toUtf8());
            aTree.put("Name", rEffectData.maColorName.toUtf8());

            model::ComplexColor aComplexColor;
            aComplexColor.setThemeColor(rColorData.meThemeColorType);
            aComplexColor.addTransformation(
                { model::TransformationType::LumMod, rEffectData.mnLumMod });
            aComplexColor.addTransformation(
                { model::TransformationType::LumOff, rEffectData.mnLumOff });
            auto aDataTree = aTree.startNode("Data");
            model::color::convertToJSONTree(aTree, aComplexColor);
        }
    }
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
