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

#include "DeckSpec.hpp"

#include <Poco/JSON/Array.h>

#include <array>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{

/// One slide part: the role a slide plays in the deck. A template offers a
/// distinct master design per part.
const std::array<std::string_view, 4>& partNames()
{
    static const std::array<std::string_view, 4> parts = { "opening", "divider", "body",
                                                            "closing" };
    return parts;
}

/// The layout and block-count rules for one slide intent. The bullets and text
/// counts are inclusive ranges; a slide's blocks must fall within them. The
/// layout name is one of AIUtil::getSlideLayouts(), and it offers a placeholder
/// for the title plus one for every block the counts allow.
struct IntentRule
{
    std::string_view intent;
    std::string_view layout;
    int minBullets;
    int maxBullets;
    int minText;
    int maxText;
    bool requiresImage;
};

/// The intent table. Each intent maps to one auto layout and a shape rule for
/// the blocks a slide of that intent may carry.
const std::vector<IntentRule>& intentRules()
{
    static const std::vector<IntentRule> rules = {
        { "title", "AUTOLAYOUT_TITLE", 0, 0, 0, 1, false },
        { "agenda", "AUTOLAYOUT_TITLE_CONTENT", 1, 1, 0, 0, false },
        { "bullets", "AUTOLAYOUT_TITLE_CONTENT", 1, 1, 0, 0, false },
        { "two-column", "AUTOLAYOUT_TITLE_2CONTENT", 2, 2, 0, 0, false },
        { "comparison", "AUTOLAYOUT_TITLE_2CONTENT", 2, 2, 0, 0, false },
        { "quote", "AUTOLAYOUT_TITLE_CONTENT", 0, 0, 1, 1, false },
        { "big-number", "AUTOLAYOUT_TITLE_CONTENT", 0, 0, 1, 1, false },
        { "image", "AUTOLAYOUT_TITLE_CONTENT", 0, 0, 0, 0, true },
        { "section", "AUTOLAYOUT_TITLE_ONLY", 0, 0, 0, 0, false },
        { "closing", "AUTOLAYOUT_TITLE", 0, 0, 0, 1, false },
    };
    return rules;
}

const IntentRule* findIntentRule(const std::string& intent)
{
    for (const auto& rule : intentRules())
        if (intent == rule.intent)
            return &rule;
    return nullptr;
}

bool isKnownPart(const std::string& part)
{
    for (const auto& name : partNames())
        if (part == name)
            return true;
    return false;
}

/// The comma-separated list of part names, for error messages.
std::string partList()
{
    std::string list;
    for (const auto& name : partNames())
    {
        if (!list.empty())
            list += ", ";
        list += name;
    }
    return list;
}

/// The comma-separated list of intent names, for error messages.
std::string intentList()
{
    std::string list;
    for (const auto& rule : intentRules())
    {
        if (!list.empty())
            list += ", ";
        list += rule.intent;
    }
    return list;
}

/// Read a string value by key, returning the empty string when it is missing or
/// not a string.
std::string getString(const Poco::JSON::Object::Ptr& obj, const std::string& key)
{
    if (!obj || !obj->has(key))
        return std::string();
    try
    {
        return obj->getValue<std::string>(key);
    }
    catch (const std::exception&)
    {
        return std::string();
    }
}

std::string slidePrefix(unsigned index, const std::string& intent)
{
    std::string prefix = "Slide " + std::to_string(index + 1);
    if (!intent.empty())
        prefix += " (intent \"" + intent + "\")";
    return prefix + ": ";
}

/// Validate one slide's blocks. Returns an error string, or the empty string
/// when the blocks satisfy the intent rule and every budget.
std::string validateBlocks(const Poco::JSON::Object::Ptr& slide, unsigned index,
                           const std::string& intent, const IntentRule& rule)
{
    const std::string prefix = slidePrefix(index, intent);

    Poco::JSON::Array::Ptr blocks;
    if (slide->has("blocks"))
    {
        blocks = slide->getArray("blocks");
        if (!blocks)
            return prefix + "\"blocks\" must be an array.";
    }

    int bulletsBlocks = 0;
    int textBlocks = 0;
    const unsigned count = blocks ? blocks->size() : 0;
    for (unsigned b = 0; b < count; ++b)
    {
        Poco::JSON::Object::Ptr block = blocks->getObject(b);
        if (!block)
            return prefix + "block " + std::to_string(b + 1) + " must be an object.";

        const std::string kind = getString(block, "kind");
        if (kind == "bullets")
        {
            ++bulletsBlocks;
            Poco::JSON::Array::Ptr items = block->getArray("items");
            if (!items || items->size() == 0)
                return prefix + "a bullets block needs a non-empty \"items\" array.";
            if (static_cast<int>(items->size()) > DeckSpec::MaxItemsPerBullets)
                return prefix + "a bullets block may list at most " +
                       std::to_string(DeckSpec::MaxItemsPerBullets) + " items, found " +
                       std::to_string(items->size()) + ".";
            for (unsigned it = 0; it < items->size(); ++it)
            {
                std::string item;
                try
                {
                    item = items->getElement<std::string>(it);
                }
                catch (const std::exception&)
                {
                    return prefix + "every bullet item must be a string.";
                }
                if (item.empty())
                    return prefix + "a bullet item must not be empty.";
                if (static_cast<int>(item.size()) > DeckSpec::MaxItemLength)
                    return prefix + "a bullet item exceeds " +
                           std::to_string(DeckSpec::MaxItemLength) + " characters.";
            }
        }
        else if (kind == "text")
        {
            ++textBlocks;
            const std::string text = getString(block, "text");
            if (text.empty())
                return prefix + "a text block needs a non-empty \"text\" string.";
            if (static_cast<int>(text.size()) > DeckSpec::MaxItemLength)
                return prefix + "a text block exceeds " +
                       std::to_string(DeckSpec::MaxItemLength) + " characters.";
        }
        else
            return prefix + "block \"kind\" must be \"bullets\" or \"text\".";
    }

    if (bulletsBlocks < rule.minBullets || bulletsBlocks > rule.maxBullets)
    {
        if (rule.minBullets == rule.maxBullets)
            return prefix + "needs exactly " + std::to_string(rule.minBullets) +
                   " bullets block(s), found " + std::to_string(bulletsBlocks) + ".";
        return prefix + "allows " + std::to_string(rule.minBullets) + " to " +
               std::to_string(rule.maxBullets) + " bullets block(s), found " +
               std::to_string(bulletsBlocks) + ".";
    }
    if (textBlocks < rule.minText || textBlocks > rule.maxText)
    {
        if (rule.minText == rule.maxText)
            return prefix + "needs exactly " + std::to_string(rule.minText) +
                   " text block(s), found " + std::to_string(textBlocks) + ".";
        return prefix + "allows " + std::to_string(rule.minText) + " to " +
               std::to_string(rule.maxText) + " text block(s), found " +
               std::to_string(textBlocks) + ".";
    }

    return std::string();
}

/// Join a bullets block's items into one string separated by newlines, the form
/// SetText expects for a multi-item placeholder.
std::string joinItems(const Poco::JSON::Array::Ptr& items)
{
    std::string joined;
    for (unsigned i = 0; items && i < items->size(); ++i)
    {
        std::string item;
        try
        {
            item = items->getElement<std::string>(i);
        }
        catch (const std::exception&)
        {
            continue;
        }
        if (!joined.empty())
            joined += '\n';
        joined += item;
    }
    return joined;
}

void pushCommand(Poco::JSON::Array::Ptr cmds, const std::string& key,
                 const std::string& value)
{
    Poco::JSON::Object::Ptr cmd = new Poco::JSON::Object();
    cmd->set(key, value);
    cmds->add(cmd);
}

void pushCommand(Poco::JSON::Array::Ptr cmds, const std::string& key, int value)
{
    Poco::JSON::Object::Ptr cmd = new Poco::JSON::Object();
    cmd->set(key, value);
    cmds->add(cmd);
}

/// Push an EditTextObject command that selects all text in placeholder slot and
/// applies one UNO formatting command to it.
void pushEditTextObject(Poco::JSON::Array::Ptr cmds, int slot, const std::string& unoCommand)
{
    Poco::JSON::Array::Ptr sub = new Poco::JSON::Array();

    Poco::JSON::Object::Ptr select = new Poco::JSON::Object();
    Poco::JSON::Array::Ptr selectArgs = new Poco::JSON::Array();
    select->set("SelectText", selectArgs);
    sub->add(select);

    Poco::JSON::Object::Ptr uno = new Poco::JSON::Object();
    uno->set("UnoCommand", unoCommand);
    sub->add(uno);

    Poco::JSON::Object::Ptr cmd = new Poco::JSON::Object();
    cmd->set("EditTextObject." + std::to_string(slot), sub);
    cmds->add(cmd);
}

std::string transformString(const Poco::JSON::Array::Ptr& cmds)
{
    Poco::JSON::Object::Ptr transforms = new Poco::JSON::Object();
    transforms->set("SlideCommands", cmds);
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("Transforms", transforms);

    std::ostringstream oss;
    root->stringify(oss);
    return oss.str();
}

} // anonymous namespace

namespace DeckSpec
{

std::optional<std::string> validateDeckSpec(const Poco::JSON::Object::Ptr& deckObj)
{
    if (!deckObj)
        return std::string("Deck spec must be a JSON object with a \"slides\" array.");

    Poco::JSON::Array::Ptr slides = deckObj->getArray("slides");
    if (!slides)
        return std::string("Deck spec must have a \"slides\" array.");
    if (slides->size() == 0)
        return std::string("A deck needs at least one slide.");
    if (static_cast<int>(slides->size()) > MaxSlides)
        return "A deck may have at most " + std::to_string(MaxSlides) + " slides, found " +
               std::to_string(slides->size()) + ".";

    for (unsigned i = 0; i < slides->size(); ++i)
    {
        Poco::JSON::Object::Ptr slide = slides->getObject(i);
        if (!slide)
            return "Slide " + std::to_string(i + 1) + " must be a JSON object.";

        const std::string part = getString(slide, "part");
        if (!isKnownPart(part))
            return slidePrefix(i, std::string()) + "\"part\" must be one of " + partList() + ".";

        const std::string intent = getString(slide, "intent");
        const IntentRule* rule = findIntentRule(intent);
        if (!rule)
            return slidePrefix(i, std::string()) + "\"intent\" must be one of " + intentList() +
                   ".";

        const std::string title = getString(slide, "title");
        if (title.empty())
            return slidePrefix(i, intent) + "\"title\" is required and must be a non-empty string.";
        if (static_cast<int>(title.size()) > MaxTitleLength)
            return slidePrefix(i, intent) + "title exceeds " + std::to_string(MaxTitleLength) +
                   " characters.";

        const std::string blocksError = validateBlocks(slide, i, intent, *rule);
        if (!blocksError.empty())
            return blocksError;

        if (rule->requiresImage)
        {
            Poco::JSON::Object::Ptr image = slide->getObject("image");
            if (!image || getString(image, "brief").empty())
                return slidePrefix(i, intent) +
                       "an image slide needs an \"image\" object with a non-empty \"brief\".";
        }
    }

    return std::nullopt;
}

std::string compileDeckSpec(const Poco::JSON::Object::Ptr& deckObj, bool haveDesignTemplate)
{
    Poco::JSON::Array::Ptr cmds = new Poco::JSON::Array();

    Poco::JSON::Array::Ptr slides = deckObj ? deckObj->getArray("slides") : nullptr;
    if (!slides)
        return transformString(cmds);

    for (unsigned i = 0; i < slides->size(); ++i)
    {
        Poco::JSON::Object::Ptr slide = slides->getObject(i);
        if (!slide)
            continue;

        const std::string intent = getString(slide, "intent");
        const IntentRule* rule = findIntentRule(intent);
        if (!rule)
            continue;

        const std::string part = getString(slide, "part");
        const std::string title = getString(slide, "title");

        // The first slide reuses the current slide of the deck; every later
        // slide is added after it.
        if (i != 0)
            pushCommand(cmds, "InsertMasterSlide", 0);

        pushCommand(cmds, "ChangeLayoutByName", std::string(rule->layout));
        pushCommand(cmds, "SetText.0", title);

        // Blocks fill the content placeholders in order, starting at slot 1.
        std::vector<int> bulletSlots;
        int slot = 1;
        Poco::JSON::Array::Ptr blocks = slide->getArray("blocks");
        for (unsigned b = 0; blocks && b < blocks->size(); ++b)
        {
            Poco::JSON::Object::Ptr block = blocks->getObject(b);
            if (!block)
                continue;
            const std::string kind = getString(block, "kind");
            if (kind == "bullets")
            {
                pushCommand(cmds, "SetText." + std::to_string(slot), joinItems(block->getArray("items")));
                bulletSlots.push_back(slot);
                ++slot;
            }
            else if (kind == "text")
            {
                pushCommand(cmds, "SetText." + std::to_string(slot), getString(block, "text"));
                ++slot;
            }
        }

        // An image slide fills its content placeholder with a generated image.
        if (rule->requiresImage)
        {
            Poco::JSON::Object::Ptr image = slide->getObject("image");
            if (image)
                pushCommand(cmds, "GenerateImage.1", getString(image, "brief"));
        }

        // Without a template the compiler supplies the house style itself:
        // bold titles and bulleted content. A template's masters own the look,
        // so with one the compiler emits no formatting.
        if (!haveDesignTemplate)
        {
            pushEditTextObject(cmds, 0, ".uno:Bold");
            for (int bulletSlot : bulletSlots)
                pushEditTextObject(cmds, bulletSlot, ".uno:DefaultBullet");
        }

        pushCommand(cmds, "SetSlidePart", part);
    }

    return transformString(cmds);
}

} // namespace DeckSpec

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
