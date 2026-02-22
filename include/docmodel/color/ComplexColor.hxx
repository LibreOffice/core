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

#include <docmodel/dllapi.h>
#include <tools/color.hxx>
#include <docmodel/theme/ThemeColorType.hxx>
#include <docmodel/color/Transformation.hxx>
#include <o3tl/hash_combine.hxx>
#include <basegfx/color/bcolortools.hxx>

#include <vector>
#include <array>

namespace model
{
enum class ColorType
{
    Unused, /// Color is not used, or undefined.
    RGB, /// Absolute RGB (r/g/b: 0...255).
    CRGB, /// Relative RGB (r/g/b: 0...100000)
    HSL, /// HSL (hue: 0...21600000, sat/lum: 0...100000).
    Theme, /// Color from scheme.
    Palette, /// Color from application defined palette.
    System, /// Color from system palette.
    Placeholder, /// Placeholder color in theme style lists.
    Finalized /// Finalized RGB color.
};

/// Used when ColorType == System
enum class SystemColorType
{
    Unused,
    DarkShadow3D,
    Light3D,
    ActiveBorder,
    ActiveCaption,
    AppWorkspace,
    Background,
    ButtonFace,
    ButtonHighlight,
    ButtonShadow,
    ButtonText,
    CaptionText,
    GradientActiveCaption,
    GradientInactiveCaption,
    GrayText,
    Highlight,
    HighlightText,
    HotLight,
    InactiveBorder,
    InactiveCaption,
    InactiveCaptionText,
    InfoBack,
    InfoText,
    Menu,
    MenuBar,
    MenuHighlight,
    MenuText,
    ScrollBar,
    Window,
    WindowFrame,
    WindowText
};

/** Definition of a color with multiple representations
 *
 * A color that can be expresses as a RGB, CRGB or HSL representation or
 * a more abstract representation as for example system color, palette,
 * theme color or a placeholder. In these representations the
 * color needs to be additionally computed and/or looked up.
 *
 * The color can also have transformations defined, which in addition
 * manipulates the resulting color (i.e. tints, shades, alpha,...).
 */
class DOCMODEL_DLLPUBLIC ComplexColor
{
private:
    ColorType meType = ColorType::Unused;

    double mnComponent1 = 0.0; // Red, Hue
    double mnComponent2 = 0.0; // Green, Saturation
    double mnComponent3 = 0.0; // Blue, Luminance

    SystemColorType meSystemColorType
        = SystemColorType::Unused; /// Only used when ColorType == System
    ::Color
        maLastColor; /// Only used when ColorType == System, does not do anything useful right now.

    ThemeColorType meThemeColorType = ThemeColorType::Unknown;
    ThemeColorUsage meThemeColorUsage = ThemeColorUsage::Unknown;

    std::vector<Transformation> maTransformations;

    ::Color maFinalColor;

public:
    ColorType getType() const { return meType; }
    void setType(ColorType eType) { meType = eType; }

    ThemeColorType getThemeColorType() const { return meThemeColorType; }
    bool isValidThemeType() const
    {
        return meType == model::ColorType::Theme && meThemeColorType != ThemeColorType::Unknown;
    }

    bool isUsed() const
    {
        return !(meType == ColorType::Unused
                 || (meType == ColorType::System && meSystemColorType == SystemColorType::Unused));
    }

    void assignIfUsed(const ComplexColor& rColor)
    {
        if (rColor.isUsed())
            *this = rColor;
    }

    ThemeColorUsage getThemeColorUsage() const { return meThemeColorUsage; }
    void setThemeColorUsage(ThemeColorUsage eThemeColorUsage)
    {
        meThemeColorUsage = eThemeColorUsage;
    }

    SystemColorType getSystemColorType() const { return meSystemColorType; }

    void setSystemColorType(SystemColorType eSystemColorType)
    {
        meSystemColorType = eSystemColorType;
        meType = ColorType::System;
    }

    Color getRGBColor() const { return Color(mnComponent1, mnComponent2, mnComponent3); }

    std::vector<Transformation> const& getTransformations() const { return maTransformations; }

    void setTransformations(std::vector<Transformation> const& rTransformations)
    {
        maTransformations = rTransformations;
    }

    void addTransformation(Transformation const& rTransform);

    void removeTransformations(TransformationType eType);

    void clearTransformations() { maTransformations.clear(); }

    double getRed() const { return mnComponent1; }
    double getGreen() const { return mnComponent2; }
    double getBlue() const { return mnComponent3; }

    void setCRGB(sal_Int32 nR, sal_Int32 nG, sal_Int32 nB);

    void setRGB(double r, double g, double b);

    void getRGB(double& r, double& g, double& b);

    Color getRGB() const;

    void setColor(Color const& rColor);

    void setRGB(sal_Int32 nRGB);

    void setHSL(sal_Int32 nH, sal_Int32 nS, sal_Int32 nL);

    void setSystemColor(SystemColorType eSystemColorType, sal_Int32 nRGB);

    bool isOpaque() const;

    bool isTransparent() const;

    void setThemePlaceholder() { meType = ColorType::Placeholder; }

    void setThemeColor(ThemeColorType eType);

    bool operator==(const ComplexColor& rComplexColor) const;

    /** Applies the defined transformations to the input color */
    Color applyTransformations(Color const& rColor) const;

    void setFinalColor(Color const& rColor) { maFinalColor = rColor; }

    Color const& getFinalColor() const { return maFinalColor; }

    std::size_t getHash() const;

    static model::ComplexColor createRGB(Color const& rColor)
    {
        model::ComplexColor aComplexColor;
        aComplexColor.setColor(rColor);
        return aComplexColor;
    }

    static model::ComplexColor Theme(ThemeColorType eThemeColorType)
    {
        model::ComplexColor aComplexColor;
        aComplexColor.setThemeColor(eThemeColorType);
        return aComplexColor;
    }

    static sal_Int32 getLumMod(size_t nIndex, size_t nEffect)
    {
        static constexpr std::array<const std::array<sal_Int32, 6>, 5> g_aLumMods = {
            std::array<sal_Int32, 6>{ 10'000, 5'000, 6'500, 7'500, 8'500, 9'500 },
            std::array<sal_Int32, 6>{ 10'000, 1'000, 2'500, 5'000, 7'500, 9'000 },
            std::array<sal_Int32, 6>{ 10'000, 2'000, 4'000, 6'000, 7'500, 5'000 },
            std::array<sal_Int32, 6>{ 10'000, 9'000, 7'500, 5'000, 2'500, 1'000 },
            std::array<sal_Int32, 6>{ 10'000, 9'500, 8'500, 7'500, 6'500, 5'000 },
        };
        return g_aLumMods[nIndex][nEffect];
    }

    static sal_Int32 getLumOff(size_t nIndex, size_t nEffect)
    {
        static constexpr std::array<const std::array<sal_Int32, 6>, 5> g_aLumOffs = {
            std::array<sal_Int32, 6>{ 0, 5'000, 3'500, 2'500, 1'500, 0'500 },
            std::array<sal_Int32, 6>{ 0, 9'000, 7'500, 5'000, 2'500, 1'000 },
            std::array<sal_Int32, 6>{ 0, 8'000, 6'000, 4'000, 0, 0 },
            std::array<sal_Int32, 6>{ 0, 0, 0, 0, 0, 0 },
            std::array<sal_Int32, 6>{ 0, 0, 0, 0, 0, 0 },
        };
        return g_aLumOffs[nIndex][nEffect];
    }

    static size_t getIndexForLuminance(Color const& rColor)
    {
        basegfx::BColor aHSLColor = basegfx::utils::rgb2hsl(rColor.getBColor());
        double aLuminanceValue = aHSLColor.getBlue() * 255.0;

        if (aLuminanceValue < 0.5)
            return 0; // Black
        else if (aLuminanceValue > 254.5)
            return 4; // White
        else if (aLuminanceValue < 50.5)
            return 1; // Low
        else if (aLuminanceValue > 203.5)
            return 3; // High
        else
            return 2; // Middle
    }

    static model::ComplexColor create(model::ThemeColorType eThemeType, Color const& rThemeColor,
                                      sal_Int32 nEffect)
    {
        size_t nIndex = getIndexForLuminance(rThemeColor);

        auto aComplexColor = model::ComplexColor::Theme(eThemeType);

        aComplexColor.addTransformation(
            { model::TransformationType::LumMod, getLumMod(nIndex, nEffect) });

        aComplexColor.addTransformation(
            { model::TransformationType::LumOff, getLumOff(nIndex, nEffect) });

        return aComplexColor;
    }
};

} // end of namespace model

namespace std
{
template <> struct hash<model::ComplexColor>
{
    std::size_t operator()(model::ComplexColor const& rColor) const { return rColor.getHash(); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
