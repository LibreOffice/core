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

#include <EffectJSONTools.hxx>
#include <ComplexColorJSONTree.hxx>
#include <docmodel/color/ComplexColorJSON.hxx>
#include <rtl/character.hxx>

#include <algorithm>
#include <optional>

namespace model::effect
{
namespace
{
/// The color of a hex string of 6 digits, or none for any other string.
std::optional<Color> readRGB(std::string const& rRGB)
{
    if (rRGB.size() != 6
        || !std::all_of(rRGB.begin(), rRGB.end(),
                        [](unsigned char c) { return rtl::isAsciiHexDigit(c); }))
        return std::nullopt;
    return Color(ColorTransparency, OUString::fromUtf8(rRGB).toUInt32(16));
}
}

void putColor(tools::JsonWriter& rTree, model::ComplexColor const& rColor)
{
    // The alpha is a member of the effect, and not a transformation of the color.
    rTree.put("Alpha", rColor.getAlpha());

    model::ComplexColor aColor(rColor);
    aColor.setAlpha(10000);
    auto aNode = rTree.startNode("Color");
    model::color::convertToJSONTree(rTree, aColor);
    const Color aRGB(aColor.getType() == model::ColorType::RGB ? aColor.getRGBColor()
                                                               : aColor.getFinalColor());
    rTree.put("RGB", aRGB.AsRGBHexString());
}

void readLength(boost::property_tree::ptree const& rTree, const char* pName, gfx::Length& rLength)
{
    rLength = gfx::Length::emu(readValue(rTree, pName, rLength.as_emu<sal_Int64>()));
}

void readAngle(boost::property_tree::ptree const& rTree, const char* pName, Degree60000& rAngle)
{
    rAngle = Degree60000(readValue(rTree, pName, rAngle.get()));
}

bool readColor(boost::property_tree::ptree const& rTree, model::ComplexColor& rColor)
{
    const sal_Int32 nAlpha = readValue(rTree, "Alpha", rColor.getAlpha());

    model::ComplexColor aColor(rColor);
    if (auto oColorTree = rTree.get_child_optional("Color"))
    {
        const std::optional<Color> oRGB = readRGB(oColorTree->get<std::string>("RGB", ""));
        if (!oRGB)
            return false;
        if (readValue(*oColorTree, "ThemeIndex", sal_Int32(-1)) >= 0)
        {
            aColor = model::color::convertFromJSONTree(*oColorTree);
            // The RGB is the color that the theme color resolves to.
            aColor.setFinalColor(*oRGB);
        }
        else
        {
            aColor = model::ComplexColor::createRGB(*oRGB);
        }
    }

    aColor.setAlpha(nAlpha);
    rColor = aColor;
    return true;
}

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
