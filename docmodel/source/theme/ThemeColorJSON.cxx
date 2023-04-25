/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/theme/ThemeColorJSON.hxx>
#include <sstream>
#include <utility>
#include <sal/log.hxx>
#include <boost/property_tree/json_parser.hpp>

namespace model::theme
{
bool convertFromJSON(OString const& rJsonString, model::ThemeColor& rThemeColor)
{
    model::ThemeColor aThemeColor;
    std::stringstream aStream(rJsonString.getStr());
    boost::property_tree::ptree aRootTree;
    try
    {
        boost::property_tree::read_json(aStream, aRootTree);
    }
    catch (const boost::property_tree::json_parser_error& /*exception*/)
    {
        return false;
    }

    sal_Int32 nThemeType = aRootTree.get<sal_Int32>("ThemeIndex", -1);
    aThemeColor.setType(model::convertToThemeColorType(nThemeType));
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
            aThemeColor.addTransformation({ eType, nValue });
    }
    rThemeColor = aThemeColor;
    return true;
}

OString convertToJSON(model::ThemeColor const& rThemeColor)
{
    boost::property_tree::ptree aTree;
    aTree.put("ThemeIndex", sal_Int16(rThemeColor.getType()));

    boost::property_tree::ptree aTransformationsList;
    for (auto const& rTransformation : rThemeColor.getTransformations())
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
            boost::property_tree::ptree aChild;
            aChild.put("Type", aType);
            aChild.put("Value", rTransformation.mnValue);
            aTransformationsList.push_back(std::make_pair("", aChild));
        }
    }
    aTree.add_child("Transformations", aTransformationsList);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    return OString(aStream.str().c_str());
}

} // end model::theme

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
