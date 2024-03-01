/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/color/ComplexColorJSON.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <utility>
#include <sal/log.hxx>
#include <tools/json_writer.hxx>

namespace model::color
{
bool convertFromJSON(OString const& rJsonString, model::ComplexColor& rComplexColor)
{
    model::ComplexColor aComplexColor;

    try
    {
        std::stringstream aStream((std::string(rJsonString)));
        boost::property_tree::ptree aRootTree;
        boost::property_tree::read_json(aStream, aRootTree);

        sal_Int32 nThemeType = aRootTree.get<sal_Int32>("ThemeIndex", -1);
        aComplexColor.setThemeColor(model::convertToThemeColorType(nThemeType));
        boost::property_tree::ptree aTransformTree = aRootTree.get_child("Transformations");
        for (const auto& rEachTransformationNode :
             boost::make_iterator_range(aTransformTree.equal_range("")))
        {
            auto const& rTransformationTree = rEachTransformationNode.second;
            std::string sType = rTransformationTree.get<std::string>("Type", "");
            sal_Int16 nValue = rTransformationTree.get<sal_Int16>("Value", 0);

            auto eType = model::TransformationType::Undefined;
            if (sType == "LumOff")
                eType = model::TransformationType::LumOff;
            else if (sType == "LumMod")
                eType = model::TransformationType::LumMod;
            else if (sType == "Tint")
                eType = model::TransformationType::Tint;
            else if (sType == "Shade")
                eType = model::TransformationType::Shade;

            if (eType != model::TransformationType::Undefined)
                aComplexColor.addTransformation({ eType, nValue });
        }
    }
    catch (const boost::property_tree::json_parser_error& /*exception*/)
    {
        return false;
    }

    rComplexColor = aComplexColor;
    return true;
}

void convertToJSONTree(tools::JsonWriter& rTree, model::ComplexColor const& rComplexColor)
{
    rTree.put("ThemeIndex", sal_Int16(rComplexColor.getThemeColorType()));
    auto aTransformationsList = rTree.startArray("Transformations");

    for (auto const& rTransformation : rComplexColor.getTransformations())
    {
        std::string aType;
        switch (rTransformation.meType)
        {
            case model::TransformationType::LumMod:
                aType = "LumMod";
                break;
            case model::TransformationType::LumOff:
                aType = "LumOff";
                break;
            case model::TransformationType::Tint:
                aType = "Tint";
                break;
            case model::TransformationType::Shade:
                aType = "Shade";
                break;
            default:
                break;
        }
        if (!aType.empty())
        {
            auto aChild = rTree.startStruct();
            rTree.put("Type", aType);
            rTree.put("Value", rTransformation.mnValue);
        }
    }
}

OString convertToJSON(model::ComplexColor const& rComplexColor)
{
    tools::JsonWriter aTree;
    convertToJSONTree(aTree, rComplexColor);
    return aTree.finishAndGetAsOString();
}

} // end model::theme

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
