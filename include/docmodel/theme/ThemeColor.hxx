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
#include <vector>
#include <docmodel/theme/ThemeColorType.hxx>
#include <tools/color.hxx>

namespace model
{
/** Color transfomation type */
enum class TransformationType
{
    Undefined,
    Red,
    RedMod,
    RedOff,
    Green,
    GreenMod,
    GreenOff,
    Blue,
    BlueMod,
    BlueOff,
    Alpha,
    AlphaMod,
    AlphaOff,
    Hue,
    HueMod,
    HueOff,
    Sat,
    SatMod,
    SatOff,
    Lum,
    LumMod,
    LumOff,
    Shade,
    Tint,
    Gray,
    Comp,
    Inv,
    Gamma,
    InvGamma
};

/** Definition of a color transformation.
 *
 * This just defines how a color should be transformed (changed). The
 * type defines what kind of transformation should occur and the value
 * defines by how much.
 */
struct DOCMODEL_DLLPUBLIC Transformation
{
    TransformationType meType = TransformationType::Undefined;
    sal_Int16 mnValue = 0; /// percentage value -10000 to +10000

    bool operator==(const Transformation& rTransformation) const
    {
        return meType == rTransformation.meType && mnValue == rTransformation.mnValue;
    }
};

/** Definition of a theme color
 *
 * A theme color is defined by the type of theme color and a set of
 * transformations that in addition manipulate the resulting color
 * (i.e. tints, shades).
 */
class DOCMODEL_DLLPUBLIC ThemeColor
{
    ThemeColorType meType = ThemeColorType::Unknown;
    std::vector<Transformation> maTransformations;

public:
    ThemeColor() = default;

    ThemeColorType getType() const { return meType; }

    void setType(ThemeColorType eType) { meType = eType; }

    void clearTransformations() { maTransformations.clear(); }

    void addTransformation(Transformation const& rTransform)
    {
        maTransformations.push_back(rTransform);
    }

    void removeTransformations(TransformationType eType)
    {
        maTransformations.erase(std::remove_if(maTransformations.begin(), maTransformations.end(),
                                               [eType](Transformation const& rTransform) {
                                                   return rTransform.meType == eType;
                                               }),
                                maTransformations.end());
    }

    std::vector<Transformation> const& getTransformations() const { return maTransformations; }

    /** Applies the defined trasformations to the input color */
    Color applyTransformations(Color const& rColor) const
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

    bool operator==(const ThemeColor& rThemeColor) const
    {
        return meType == rThemeColor.meType
               && maTransformations.size() == rThemeColor.maTransformations.size()
               && std::equal(maTransformations.begin(), maTransformations.end(),
                             rThemeColor.maTransformations.begin());
    }
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
