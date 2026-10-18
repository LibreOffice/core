/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/theme/RectangleAlignment.hxx>
#include <basegfx/units/Length.hxx>
#include <rtl/string.hxx>
#include <tools/degree.hxx>
#include <tools/json_writer.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <sstream>
#include <string>

/* The members of the JSON of the effects that more than one effect has. Lengths are in EMU, and the
   other values are in the units of the effect class. The color is the "Color" object in the format
   of model::color::convertToJSON, with an "RGB" member that has the color as a hex string of 6
   digits. Its alpha is the "Alpha" member of the effect, and not a transformation of the color.
 */

namespace model::effect
{
void putColor(tools::JsonWriter& rTree, model::ComplexColor const& rColor);

template <class Effect> void putOffset(tools::JsonWriter& rTree, Effect const& rEffect)
{
    rTree.put("BlurRadius", rEffect.maBlurRadius.template as_emu<sal_Int64>());
    rTree.put("Distance", rEffect.maDistance.template as_emu<sal_Int64>());
    rTree.put("Direction", rEffect.mnDirection.get());
}

template <class Effect> void putTransform(tools::JsonWriter& rTree, Effect const& rEffect)
{
    rTree.put("ScaleX", rEffect.mnScaleX);
    rTree.put("ScaleY", rEffect.mnScaleY);
    rTree.put("SkewX", rEffect.mnSkewX.get());
    rTree.put("SkewY", rEffect.mnSkewY.get());
    rTree.put("Alignment", sal_Int32(rEffect.meAlignment));
    rTree.put("RotateWithShape", rEffect.mbRotateWithShape);
}

/** Reads the effect from the JSON with aRead, on a copy of rEffect. rEffect gets the copy only
    when the JSON parses and every value has the right type. */
template <class Effect, class Read>
bool readEffect(OString const& rJSON, Effect& rEffect, Read aRead)
{
    try
    {
        std::stringstream aStream((std::string(rJSON)));
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        Effect aEffect(rEffect);
        if (!aRead(aTree, aEffect))
            return false;
        rEffect = aEffect;
        return true;
    }
    catch (const boost::property_tree::ptree_error&)
    {
        return false;
    }
}

/** The value of the member pName, or aDefault when the JSON lacks the member. A value that does not
    convert to T throws boost::property_tree::ptree_bad_data. */
template <class T>
T readValue(boost::property_tree::ptree const& rTree, const char* pName, T aDefault)
{
    if (auto oChild = rTree.get_child_optional(pName))
        return oChild->get_value<T>();
    return aDefault;
}

void readLength(boost::property_tree::ptree const& rTree, const char* pName, gfx::Length& rLength);

void readAngle(boost::property_tree::ptree const& rTree, const char* pName, Degree60000& rAngle);

/// Gives false for a "Color" object that does not parse or has no valid "RGB" member.
bool readColor(boost::property_tree::ptree const& rTree, model::ComplexColor& rColor);

template <class Effect> void readOffset(boost::property_tree::ptree const& rTree, Effect& rEffect)
{
    readLength(rTree, "BlurRadius", rEffect.maBlurRadius);
    readLength(rTree, "Distance", rEffect.maDistance);
    readAngle(rTree, "Direction", rEffect.mnDirection);
}

template <class Effect>
void readTransform(boost::property_tree::ptree const& rTree, Effect& rEffect)
{
    rEffect.mnScaleX = readValue(rTree, "ScaleX", rEffect.mnScaleX);
    rEffect.mnScaleY = readValue(rTree, "ScaleY", rEffect.mnScaleY);
    readAngle(rTree, "SkewX", rEffect.mnSkewX);
    readAngle(rTree, "SkewY", rEffect.mnSkewY);
    rEffect.meAlignment = model::RectangleAlignment(
        readValue(rTree, "Alignment", sal_Int32(rEffect.meAlignment)));
    rEffect.mbRotateWithShape = readValue(rTree, "RotateWithShape", rEffect.mbRotateWithShape);
}

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
