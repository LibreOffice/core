/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/color/ComplexColor.hxx>
#include <sal/log.hxx>
#include <tools/color.hxx>
#include <docmodel/theme/ThemeColorType.hxx>
#include <docmodel/color/Transformation.hxx>
#include <o3tl/hash_combine.hxx>

namespace model
{
void ComplexColor::addTransformation(Transformation const& rTransform)
{
    maTransformations.push_back(rTransform);
}

void ComplexColor::removeTransformations(TransformationType eType)
{
    std::erase_if(maTransformations,
                  [eType](Transformation const& rTransform) { return rTransform.meType == eType; });
}

void ComplexColor::setCRGB(sal_Int32 nR, sal_Int32 nG, sal_Int32 nB)
{
    mnComponent1 = nR;
    mnComponent2 = nG;
    mnComponent3 = nB;
    meType = ColorType::CRGB;
}

void ComplexColor::setRGB(double r, double g, double b)
{
    mnComponent1 = r;
    mnComponent2 = g;
    mnComponent3 = b;
    maFinalColor = ::Color(sal_Int8(r), sal_Int8(g), sal_Int8(b));
    meType = ColorType::RGB;
}

void ComplexColor::getRGB(double& r, double& g, double& b)
{
    if (meType != ColorType::RGB)
        return;
    r = mnComponent1;
    g = mnComponent2;
    b = mnComponent3;
}

Color ComplexColor::getRGB() const { return Color(mnComponent1, mnComponent2, mnComponent3); }

void ComplexColor::setColor(Color const& rColor)
{
    mnComponent1 = rColor.GetRed();
    mnComponent2 = rColor.GetGreen();
    mnComponent3 = rColor.GetBlue();
    maFinalColor = rColor;
    meType = ColorType::RGB;
}

void ComplexColor::setRGB(sal_Int32 nRGB)
{
    ::Color aColor(ColorTransparency, nRGB);
    setColor(aColor);
}

void ComplexColor::setHSL(sal_Int32 nH, sal_Int32 nS, sal_Int32 nL)
{
    mnComponent1 = nH;
    mnComponent2 = nS;
    mnComponent3 = nL;
    meType = ColorType::HSL;
}

void ComplexColor::setSystemColor(SystemColorType eSystemColorType, sal_Int32 nRGB)
{
    maLastColor = ::Color(ColorTransparency, nRGB);
    meSystemColorType = eSystemColorType;
    meType = ColorType::System;
}

bool ComplexColor::isOpaque() const
{
    for (const Transformation& t : maTransformations)
    {
        if (t.meType == model::TransformationType::Alpha && t.mnValue != 10000)
        {
            return false;
        }
    }
    return true;
}

bool ComplexColor::isTransparent() const
{
    for (const Transformation& t : maTransformations)
    {
        if (t.meType == model::TransformationType::Alpha && t.mnValue == 0)
        {
            return true;
        }
    }
    return false;
}

void ComplexColor::setThemeColor(ThemeColorType eType)
{
    meThemeColorType = eType;
    meType = ColorType::Theme;
}

bool ComplexColor::operator==(const ComplexColor& rComplexColor) const
{
    return meType == rComplexColor.meType && mnComponent1 == rComplexColor.mnComponent1
           && mnComponent2 == rComplexColor.mnComponent2
           && mnComponent3 == rComplexColor.mnComponent3
           && meSystemColorType == rComplexColor.meSystemColorType
           && maLastColor == rComplexColor.maLastColor
           && meThemeColorType == rComplexColor.meThemeColorType
           && maTransformations.size() == rComplexColor.maTransformations.size()
           && std::equal(maTransformations.begin(), maTransformations.end(),
                         rComplexColor.maTransformations.begin());
}

Color ComplexColor::applyTransformations(Color const& rColor) const
{
    Color aColor(rColor);

    for (auto const& rTransform : maTransformations)
    {
        switch (rTransform.meType)
        {
            case TransformationType::Tint:
                aColor.ApplyTintOrShade(rTransform.mnValue);
                break;
            case TransformationType::Shade:
                aColor.ApplyTintOrShade(-rTransform.mnValue);
                break;
            case TransformationType::LumMod:
                aColor.ApplyLumModOff(rTransform.mnValue, 0);
                break;
            case TransformationType::LumOff:
                aColor.ApplyLumModOff(10000, rTransform.mnValue);
                break;
            default:
                break;
        }
    }
    return aColor;
}

std::size_t ComplexColor::getHash() const
{
    std::size_t seed = 0;
    o3tl::hash_combine(seed, meType);
    o3tl::hash_combine(seed, mnComponent1);
    o3tl::hash_combine(seed, mnComponent2);
    o3tl::hash_combine(seed, mnComponent3);
    o3tl::hash_combine(seed, meSystemColorType);
    o3tl::hash_combine(seed, sal_uInt32(maLastColor));
    for (auto const& rTransform : maTransformations)
        o3tl::hash_combine(seed, rTransform);
    o3tl::hash_combine(seed, sal_uInt32(maFinalColor));
    return seed;
}

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
