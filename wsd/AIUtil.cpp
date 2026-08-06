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

#include <cstddef>
#include <exception>
#include <string>
#include <string_view>
#include <vector>

namespace
{

/// Finds a command by the key's base name (the part before any ".N"/".X"
/// suffix). Returns nullptr for a name outside the vocabulary.
const AIUtil::SlideCommandInfo* findSlideCommand(const std::string& key)
{
    const std::string base = key.substr(0, key.find('.'));
    for (const auto& cmd : AIUtil::getSlideCommands())
        if (base == cmd.name)
            return &cmd;
    return nullptr;
}

/// True when the key's base name is in the SlideCommands vocabulary. Used
/// only for conservative structural validation.
bool isKnownSlideCommand(const std::string& key) { return findSlideCommand(key) != nullptr; }

} // anonymous namespace

namespace AIUtil
{

namespace
{
struct PreCannedAIProvider
{
    std::string_view id;
    std::string_view baseUrl;
    std::string_view host; // must be the host part of baseUrl
};

// The built-in AI providers. Keep in sync with AI_PROVIDERS in
// browser/admin/src/integrator/AdminIntegratorSettings.ts.
constexpr PreCannedAIProvider PreCannedAIProviders[] = {
    { "openai", "https://api.openai.com", "api.openai.com" },
    { "groq", "https://api.groq.com/openai", "api.groq.com" },
    { "together", "https://api.together.xyz", "api.together.xyz" },
    { "mistral", "https://api.mistral.ai", "api.mistral.ai" },
};
}

std::string_view preCannedAIProviderBaseUrl(std::string_view id)
{
    for (const auto& provider : PreCannedAIProviders)
        if (provider.id == id)
            return provider.baseUrl;
    return {};
}

bool isPreCannedAIProviderHost(std::string_view host)
{
    for (const auto& provider : PreCannedAIProviders)
        if (provider.host == host)
            return true;
    return false;
}

std::string normalizeAIBaseUrl(std::string_view baseUrl)
{
    std::string url(baseUrl);

    const auto stripTrailingSlashes = [&url]()
    {
        while (!url.empty() && url.back() == '/')
            url.pop_back();
    };

    stripTrailingSlashes();

    // Drop a single trailing "/v1" (any case). The version path is appended
    // afterwards, so leaving one here would double it into "/v1/v1/...".
    if (url.size() >= 3)
    {
        const std::size_t tail = url.size() - 3;
        if (url[tail] == '/' && (url[tail + 1] == 'v' || url[tail + 1] == 'V') &&
            url[tail + 2] == '1')
        {
            url.erase(tail);
            stripTrailingSlashes();
        }
    }

    return url;
}

namespace
{
constexpr SlideCommandInfo SlideCommands[] = {
    { "JumpToSlide", true, "Navigation:",
      R"(- {"JumpToSlide": N} - jump to 0-based slide index; use "last" for last slide)" },
    { "JumpToSlideByName", true, "Navigation:",
      R"(- {"JumpToSlideByName": "name"} - jump to named slide)" },

    { "InsertMasterSlide", true,
      "Slide management (inserts after current slide and jumps to new slide):",
      R"(- {"InsertMasterSlide": N} - insert slide based on master slide at index N)" },
    { "InsertMasterSlideByName", true,
      "Slide management (inserts after current slide and jumps to new slide):",
      R"(- {"InsertMasterSlideByName": "name"} - insert slide by master slide name)" },
    { "DeleteSlide", true,
      "Slide management (inserts after current slide and jumps to new slide):",
      R"(- {"DeleteSlide": N} - delete slide at index; use "" for current slide)" },
    { "DuplicateSlide", true,
      "Slide management (inserts after current slide and jumps to new slide):",
      R"(- {"DuplicateSlide": N} - duplicate slide at index; use "" for current)" },
    { "MoveSlide", true,
      "Slide management (inserts after current slide and jumps to new slide):",
      R"(- {"MoveSlide": N} - move current slide to position N
- {"MoveSlide.X": N} - move slide at index X to position N)" },
    { "RenameSlide", true,
      "Slide management (inserts after current slide and jumps to new slide):",
      R"(- {"RenameSlide": "name"} - rename current slide (must be unique))" },

    { "ChangeLayoutByName", true, "Layout (applied to current slide):",
      R"(- {"ChangeLayoutByName": "name"} - set layout by name)" },
    { "ChangeLayout", true, "Layout (applied to current slide):",
      R"(- {"ChangeLayout": N} - set layout by numeric ID)" },

    { "SetText", true, "Text content:",
      R"(- {"SetText.N": "text"} - set text of placeholder N on current slide (0=title, 1=first content, 2=second content, etc.). Use \n for paragraph breaks.)" },

    { "SetNotes", true, "Text content:",
      R"(- {"SetNotes": "text"} - set the speaker notes of the current slide)" },

    { "GenerateImage", true,
      "Image generation (inserts AI-generated image into a placeholder):",
      R"(- {"GenerateImage.N": "prompt"} - generate an image from the text prompt using the AI image provider and insert it into placeholder N on the current slide, replacing the placeholder content. N is the 0-based object index (same as SetText.N). A loading placeholder is inserted immediately when the transform is applied, then the real image progressively replaces it after generation completes. Use descriptive prompts for best results. Example: {"GenerateImage.1": "A modern office building with glass facade at sunset, professional photography"})" },

    { "MarkObject", true, "Object selection:",
      R"(- {"MarkObject": N} - select object at index on current slide)" },
    { "UnMarkObject", true, "Object selection:",
      R"(- {"UnMarkObject": N} - deselect object at index)" },

    { "EditTextObject", true, "Rich text editing:",
      R"(- {"EditTextObject.N": [...]} - edit text object N with sub-commands:
  - {"SelectText": []} - select all text; [para] selects paragraph; [para,startChar,endPara,endChar] selects range; [para,char] positions cursor
  - {"SelectParagraph": N} - select paragraph N
  - {"InsertText": "text"} - insert/replace text at selection
  - {"UnoCommand": "cmd"} - apply UNO command to selection)" },

    // Documented in the static UnoCommand-levels narrative, not here.
    { "UnoCommand", true, "", "" },

    // The design template is the user's choice. The server alone
    // splices an ApplyTemplate into a transform.
    { "ApplyTemplate", false, "", "" },

    // Labels a slide's part (opening, divider, body, closing). The parts a
    // template offers and the instruction to use them are given per request,
    // when a template is in use, so the command carries no entry in the
    // static documentation.
    { "SetSlidePart", true, "", "" },

    // Labels a slide's intent (the kind of slide it is). A template manifest
    // may map an intent to a specific master. Like SetSlidePart, it carries
    // no static documentation because the intent list and its use are given
    // per request.
    { "SetSlideIntent", true, "", "" },
};
}

std::span<const SlideCommandInfo> getSlideCommands() { return SlideCommands; }

bool isServerOnlySlideCommand(const std::string& key)
{
    const SlideCommandInfo* cmd = findSlideCommand(key);
    return cmd && !cmd->allowedFromModel;
}

namespace
{
constexpr SlideLayoutInfo SlideLayouts[] = {
    { "AUTOLAYOUT_TITLE", 0, 2, "title + subtitle. Use for opening or closing slides." },
    { "AUTOLAYOUT_TITLE_CONTENT", 1, 2,
      "title + one content area below. The default layout for bullet points, text, or a "
      "single diagram." },
    { "AUTOLAYOUT_TITLE_2CONTENT", 3, 3,
      "title + two content areas side by side. Use for comparisons, pros/cons, or "
      "before/after." },
    { "AUTOLAYOUT_TITLE_CONTENT_2CONTENT", 12, 4,
      "title + one large content left + two smaller content areas stacked right." },
    { "AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT", 14, 3,
      "title + two content areas stacked vertically. Use when two blocks need equal width." },
    { "AUTOLAYOUT_TITLE_2CONTENT_CONTENT", 15, 4,
      "title + two smaller content areas stacked left + one large content right." },
    { "AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT", 16, 4,
      "title + two content areas side by side over one full-width content area." },
    { "AUTOLAYOUT_TITLE_4CONTENT", 18, 5,
      "title + four content areas in a 2x2 grid. Use for dashboards or quadrant layouts." },
    { "AUTOLAYOUT_TITLE_ONLY", 19, 1,
      "title only, no content placeholders. Use for section dividers or title cards." },
    { "AUTOLAYOUT_NONE", 20, 0,
      "blank slide, no placeholders at all. Use for fully custom or image-only slides." },
    { "AUTOLAYOUT_ONLY_TEXT", 32, 1,
      "one centered text area, no title. Use for quotes, key statements, or transition "
      "text." },
    { "AUTOLAYOUT_TITLE_6CONTENT", 34, 7, "title + six content areas in a 3x2 grid." },
    { "AUTOLAYOUT_VTITLE_VCONTENT", 28, 2,
      "vertical title + vertical content. For vertical/CJK text." },
    { "AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT", 27, 3,
      "vertical title + two vertical content areas." },
    { "AUTOLAYOUT_TITLE_VCONTENT", 29, 2, "horizontal title + vertical content below." },
    { "AUTOLAYOUT_TITLE_2VTEXT", 30, 3, "title + two vertical text columns." },
};
}

std::span<const SlideLayoutInfo> getSlideLayouts() { return SlideLayouts; }

bool isKnownSlideLayout(const std::string& name)
{
    for (const auto& layout : getSlideLayouts())
        if (name == layout.name)
            return true;
    return false;
}

const std::string& getSlideCommandDocs()
{
    static const std::string docs = []
    {
        std::string text;
        std::string_view currentSection;
        for (const auto& cmd : getSlideCommands())
        {
            if (cmd.docLines.empty())
                continue;
            if (cmd.docSection != currentSection)
            {
                currentSection = cmd.docSection;
                text += '\n';
                text += currentSection;
                text += '\n';
            }
            text += cmd.docLines;
            text += '\n';

            // The layout names belong right under the two layout-changing
            // commands, so the model reads them as the values those commands
            // accept.
            if (cmd.name == "ChangeLayout")
            {
                text += "Available layouts (use ChangeLayoutByName with these names):\n";
                for (const auto& layout : getSlideLayouts())
                {
                    text += "- ";
                    text += layout.name;
                    text += " (id=" + std::to_string(layout.id) + ") - ";
                    text += layout.description;
                    text += '\n';
                }
            }
        }
        return text;
    }();
    return docs;
}

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
        for (std::size_t i = 0; arr && i < arr->size(); ++i)
        {
            Poco::JSON::Object::Ptr el = arr->getObject(i);
            if (!el)
                continue;
            std::vector<std::string> keys;
            el->getNames(keys);
            for (const auto& k : keys)
                merged->set(k, el->get(k));
        }
        argsObj = std::move(merged);
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

    for (std::size_t i = 0; i < cmds->size(); ++i)
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
