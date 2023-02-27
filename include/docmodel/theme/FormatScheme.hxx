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
#include <docmodel/theme/ThemeColor.hxx>

namespace model
{
enum class ColorType
{
    Unused,
    RGB,
    CRGB,
    HSL,
    Scheme,
    Palette,
    System,
    Placeholder
};

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

struct DOCMODEL_DLLPUBLIC ColorDefinition
{
    ColorType meType = ColorType::Unused;

    sal_Int32 mnComponent1 = 0; // Red, Hue
    sal_Int32 mnComponent2 = 0; // Green, Saturation
    sal_Int32 mnComponent3 = 0; // Blue, Luminance
    sal_Int32 mnAlpha = 0; // Percentage

    SystemColorType meSystemColorType = SystemColorType::Unused;
    ::Color maLastColor;

    ThemeColorType meSchemeType = ThemeColorType::Unknown;
    std::vector<Transformation> maTransformations;

    void setCRGB(sal_Int32 nR, sal_Int32 nG, sal_Int32 nB)
    {
        mnComponent1 = nR;
        mnComponent2 = nG;
        mnComponent3 = nB;
        meType = ColorType::CRGB;
    }

    void setRGB(sal_Int32 nRGB)
    {
        ::Color aColor(ColorTransparency, nRGB);
        mnComponent1 = aColor.GetRed();
        mnComponent2 = aColor.GetGreen();
        mnComponent3 = aColor.GetBlue();
        meType = ColorType::RGB;
    }

    void setHSL(sal_Int32 nH, sal_Int32 nS, sal_Int32 nL)
    {
        mnComponent1 = nH;
        mnComponent2 = nS;
        mnComponent3 = nL;
        meType = ColorType::HSL;
    }

    void setSystemColor(SystemColorType eSystemColorType, sal_Int32 nRGB)
    {
        maLastColor = ::Color(ColorTransparency, nRGB);
        meSystemColorType = eSystemColorType;
        meType = ColorType::System;
    }

    void setSchemePlaceholder() { meType = ColorType::Placeholder; }

    void setSchemeColor(ThemeColorType eType)
    {
        meSchemeType = eType;
        meType = ColorType::Scheme;
    }
};

enum class FillType
{
    None,
    Solid,
    Gradient,
    Pattern,
    Blip
};

class DOCMODEL_DLLPUBLIC Fill
{
public:
    Fill(FillType eType)
        : meType(eType)
    {
    }

    FillType meType;
};

class DOCMODEL_DLLPUBLIC NoFill : public Fill
{
public:
    NoFill()
        : Fill(FillType::None)
    {
    }
};

class DOCMODEL_DLLPUBLIC SolidFill : public Fill
{
public:
    ColorDefinition maColorDefinition;

    SolidFill()
        : Fill(FillType::Solid)
    {
    }
};

class DOCMODEL_DLLPUBLIC GradientStop
{
public:
    double mfPosition = 0.0; // 0.0 - 1.0
    ColorDefinition maColor;
};

enum class GradientType
{
    Undefined,
    Linear,
    Circle,
    Rectangle,
    Shape,
};

struct DOCMODEL_DLLPUBLIC LinearGradientProperties
{
    sal_Int32 mnAngle = 0;
    bool mbScaled = false;
};

struct DOCMODEL_DLLPUBLIC RelativeRectangle
{
    sal_Int32 mnLeft = 0;
    sal_Int32 mnTop = 0;
    sal_Int32 mnRight = 0;
    sal_Int32 mnBottom = 0;
};

class DOCMODEL_DLLPUBLIC GradientFill : public Fill
{
public:
    bool mbRotateWithShape = false;
    GradientType meGradientType = GradientType::Undefined;
    std::vector<GradientStop> maGradientStops;
    LinearGradientProperties maLinearGradient;
    RelativeRectangle maFillToRectangle;
    RelativeRectangle maTileRectangle;

    GradientFill()
        : Fill(FillType::Gradient)
    {
    }
};

enum class PatternPreset
{
    Unused,
    Percent_5,
    Percent_10,
    Percent_20,
    Percent_25,
    Percent_30,
    Percent_40,
    Percent_50,
    Percent_60,
    Percent_70,
    Percent_75,
    Percent_80,
    Percent_90,
    Horizontal,
    Vertical,
    LightHorizontal,
    LightVertical,
    DarkHorizontal,
    DarkVertical,
    NarrowHorizontal,
    NarrowVertical,
    DashedHorizontal,
    DashedVertical,
    Cross,
    DownwardDiagonal,
    UpwardDiagonal,
    LightDownwardDiagonal,
    LightUpwardDiagonal,
    DarkDownwardDiagonal,
    DarkUpwardDiagonal,
    WideDownwardDiagonal,
    WideUpwardDiagonal,
    DashedDownwardDiagonal,
    DashedUpwardDiagonal,
    DiagonalCross,
    SmallCheckerBoard,
    LargeCheckerBoard,
    SmallGrid,
    LargeGrid,
    DottedGrid,
    SmallConfetti,
    LargeConfetti,
    HorizontalBrick,
    DiagonalBrick,
    SolidDiamond,
    OpenDiamond,
    DottedDiamond,
    Plaid,
    Sphere,
    Weave,
    Divot,
    Shingle,
    Wave,
    Trellis,
    ZigZag
};

class DOCMODEL_DLLPUBLIC PatternFill : public Fill
{
public:
    PatternPreset mePatternPreset = PatternPreset::Unused;
    ColorDefinition maForegroundColor;
    ColorDefinition maBackgroundColor;

    PatternFill()
        : Fill(FillType::Pattern)
    {
    }
};

// Format Scheme

class DOCMODEL_DLLPUBLIC FillStyle
{
public:
    std::shared_ptr<Fill> mpFill;
};

class DOCMODEL_DLLPUBLIC FormatScheme
{
private:
    OUString maName;
    std::vector<FillStyle> maFillStyleList;
    std::vector<FillStyle> maBackgroundFillStyleList;

public:
    FormatScheme() = default;

    FormatScheme(OUString const& rName)
        : maName(rName)
    {
    }

    const OUString& getName() const { return maName; }

    std::vector<FillStyle> const& getFillStyleList() const { return maFillStyleList; }

    FillStyle* addFillStyle()
    {
        if (maFillStyleList.size() > 3)
            return nullptr;
        auto& rFillStyle = maFillStyleList.emplace_back();
        return &rFillStyle;
    }

    std::vector<FillStyle> const& getBackgroundFillStyleList() const
    {
        return maBackgroundFillStyleList;
    }

    FillStyle* addBackgroundFillStyle()
    {
        if (maBackgroundFillStyleList.size() > 3)
            return nullptr;
        auto& rBackgroundFillStyle = maBackgroundFillStyleList.emplace_back();
        return &rBackgroundFillStyle;
    }
};

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
