/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "AIUtil.hpp"

#include <common/JsonUtil.hpp>
#include <common/Log.hpp>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>

#include <array>
#include <exception>
#include <string>
#include <string_view>
#include <vector>

namespace
{

/// Recognized top-level Impress SlideCommands base names (the part before any
/// ".N"/".X" suffix). Used only for conservative structural validation.
bool isKnownSlideCommand(const std::string& key)
{
    static constexpr std::array<std::string_view, 16> known = {
        "JumpToSlide", "JumpToSlideByName", "InsertMasterSlide",
        "InsertMasterSlideByName", "DeleteSlide", "DuplicateSlide", "MoveSlide",
        "RenameSlide", "ChangeLayoutByName", "ChangeLayout", "SetText",
        "GenerateImage", "MarkObject", "UnMarkObject", "EditTextObject",
        "UnoCommand"
    };
    const std::string base = key.substr(0, key.find('.'));
    for (const auto& k : known)
        if (base == k)
            return true;
    return false;
}

} // anonymous namespace

namespace AIUtil
{

bool parseLenientArgs(const std::string& argsJson, Poco::JSON::Object::Ptr& argsObj)
{
    if (JsonUtil::parseJSON(argsJson, argsObj))
        return true;

    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var v = parser.parse(argsJson);
        if (v.type() != typeid(Poco::JSON::Array::Ptr))
            return false;

        auto arr = v.extract<Poco::JSON::Array::Ptr>();
        Poco::JSON::Object::Ptr merged = new Poco::JSON::Object();
        for (unsigned i = 0; arr && i < arr->size(); ++i)
        {
            Poco::JSON::Object::Ptr el = arr->getObject(i);
            if (!el)
                continue;
            std::vector<std::string> keys;
            el->getNames(keys);
            for (const auto& k : keys)
                merged->set(k, el->get(k));
        }
        argsObj = merged;
        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERR("parseLenientArgs: JSON parser threw while parsing tool args: "
                << e.what() << "; argsSize=" << argsJson.size()
                << " argsHead=" << argsJson.substr(0, 200));
        return false;
    }
}

std::optional<std::string> validateTransformStructure(
    const Poco::JSON::Object::Ptr& transformObj)
{
    if (!transformObj)
        return std::string("Transform payload is not a JSON object.");

    const bool hasTransforms = transformObj->has("Transforms");
    const bool hasUno = transformObj->has("UnoCommand");
    if (!hasTransforms && !hasUno)
        return std::string(
            "Transform must contain a \"Transforms\" object and/or a top-level "
            "\"UnoCommand\".");

    if (!hasTransforms)
        return std::nullopt; // UnoCommand-only transform

    Poco::JSON::Object::Ptr transforms = transformObj->getObject("Transforms");
    if (!transforms)
        return std::string("\"Transforms\" must be a JSON object.");

    // SlideCommands is Impress-specific and optional - Writer/Calc use
    // ContentControls selectors instead - so validate it only when present.
    if (!transforms->has("SlideCommands"))
        return std::nullopt;

    Poco::JSON::Array::Ptr cmds = transforms->getArray("SlideCommands");
    if (!cmds)
        return std::string(
            "\"SlideCommands\" must be a JSON array. Put every slide operation in "
            "a single SlideCommands array.");

    for (unsigned i = 0; i < cmds->size(); ++i)
    {
        Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
        if (!cmd)
            return "SlideCommands[" + std::to_string(i) + "] must be a JSON object.";

        std::vector<std::string> keys;
        cmd->getNames(keys);
        if (keys.empty())
            return "SlideCommands[" + std::to_string(i) +
                   "] is empty; each entry needs exactly one command.";

        for (const std::string& key : keys)
        {
            if (!isKnownSlideCommand(key))
                return "SlideCommands[" + std::to_string(i) + "] uses unknown command \"" +
                       key + "\". Use only the documented SlideCommands operations.";
        }
    }

    return std::nullopt;
}

} // namespace AIUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
