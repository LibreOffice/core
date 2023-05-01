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
#include <docmodel/color/Transformation.hxx>
#include <tools/color.hxx>

namespace model
{
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

    void setTransformations(std::vector<Transformation> const& rTransformations)
    {
        maTransformations = rTransformations;
    }

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
