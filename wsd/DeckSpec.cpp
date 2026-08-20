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

#include <common/JsonUtil.hpp>
#include <common/Log.hpp>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Array.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{

/// The slide parts: the role a slide plays in the deck. A template offers a
/// distinct master design per part.
constexpr std::string_view PartNames[] = { "opening", "divider", "body", "closing" };

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
constexpr IntentRule IntentRules[] = {
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

const IntentRule* findIntentRule(const std::string& intent)
{
    for (const auto& rule : IntentRules)
        if (intent == rule.intent)
            return &rule;
    return nullptr;
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

/// The text with every carriage return, line feed and NUL turned into a single
/// space. A slide title and a gist are each written into the middle of a
/// one-line sentence in a prompt, and this keeps them on that one line.
std::string flattenToOneLine(std::string text)
{
    for (char& c : text)
    {
        if (c == '\n' || c == '\r' || c == '\0')
            c = ' ';
    }
    return text;
}

/// The UTF-16 code-unit length of a UTF-8 string. A code point at or above
/// U+10000 needs a surrogate pair and counts as two units; every other code
/// point counts as one. The engine addresses text positions in UTF-16 units, so
/// emphasis ranges are measured the same way.
size_t utf16Length(std::string_view text)
{
    size_t units = 0;
    for (size_t i = 0; i < text.size();)
    {
        const unsigned char lead = static_cast<unsigned char>(text[i]);
        int bytes;
        unsigned int codePoint;
        if (lead < 0x80)
        {
            bytes = 1;
            codePoint = lead;
        }
        else if ((lead >> 5) == 0x6)
        {
            bytes = 2;
            codePoint = lead & 0x1F;
        }
        else if ((lead >> 4) == 0xE)
        {
            bytes = 3;
            codePoint = lead & 0x0F;
        }
        else if ((lead >> 3) == 0x1E)
        {
            bytes = 4;
            codePoint = lead & 0x07;
        }
        else
        {
            // A stray continuation or invalid lead byte counts as one unit.
            bytes = 1;
            codePoint = lead;
        }
        for (int k = 1; k < bytes && i + k < text.size(); ++k)
            codePoint = (codePoint << 6) | (static_cast<unsigned char>(text[i + k]) & 0x3F);
        units += (codePoint >= 0x10000) ? 2 : 1;
        i += bytes;
    }
    return units;
}

/// One run of emphasised text within a single paragraph, its bounds measured in
/// UTF-16 code units from the start of the paragraph's visible text.
struct EmphasisRange
{
    size_t startU16;
    size_t endU16;
    bool bold;
    bool italic;
};

/// The visible text of one paragraph with the emphasis markers removed, and the
/// runs that carried emphasis.
struct EmphasisText
{
    std::string plain;
    std::vector<EmphasisRange> ranges;
};

/// Parse the small emphasis subset from one paragraph. Runs of '*' toggle
/// emphasis: one '*' toggles italic, two toggle bold, three toggle both, and
/// four or more are literal asterisks. A marker left open at the end of the
/// paragraph reverts to literal asterisks. This is a toggle scan, not CommonMark;
/// applying the same emphasis twice turns it off again. Returns the visible text
/// with the effective markers removed and the emphasis ranges.
EmphasisText parseEmphasis(const std::string& utf8)
{
    // Split into runs of ordinary text and runs of '*'.
    struct Token
    {
        bool mark;
        size_t begin;
        size_t len;
        int runLen;
    };
    std::vector<Token> tokens;
    for (size_t i = 0; i < utf8.size();)
    {
        size_t j = i;
        if (utf8[i] == '*')
        {
            while (j < utf8.size() && utf8[j] == '*')
                ++j;
            tokens.push_back({ true, i, j - i, static_cast<int>(j - i) });
        }
        else
        {
            while (j < utf8.size() && utf8[j] != '*')
                ++j;
            tokens.push_back({ false, i, j - i, 0 });
        }
        i = j;
    }

    // Pair the toggle markers for each emphasis type in order. Italic markers are
    // runs of one or three; bold markers are runs of two or three. A trailing
    // unpaired marker of a type has no effect on that type and stays literal.
    std::vector<bool> effectiveItalic(tokens.size(), false);
    std::vector<bool> effectiveBold(tokens.size(), false);
    auto pairMarkers = [&tokens](bool (*affects)(int), std::vector<bool>& effective)
    {
        int openIndex = -1;
        for (size_t t = 0; t < tokens.size(); ++t)
        {
            if (!tokens[t].mark || tokens[t].runLen >= 4 || !affects(tokens[t].runLen))
                continue;
            if (openIndex < 0)
                openIndex = static_cast<int>(t);
            else
            {
                effective[openIndex] = true;
                effective[t] = true;
                openIndex = -1;
            }
        }
    };
    pairMarkers([](int r) { return r == 1 || r == 3; }, effectiveItalic);
    pairMarkers([](int r) { return r == 2 || r == 3; }, effectiveBold);

    // Build the visible text and record a toggle event at the position of each
    // effective marker. Any other marker's asterisks are literal text.
    struct Event
    {
        size_t pos;
        bool italic;
        bool bold;
    };
    std::vector<Event> events;
    EmphasisText out;
    size_t u16 = 0;
    for (size_t t = 0; t < tokens.size(); ++t)
    {
        const Token& tok = tokens[t];
        const std::string_view piece(utf8.data() + tok.begin, tok.len);
        if (tok.mark && (effectiveItalic[t] || effectiveBold[t]))
            events.push_back({ u16, effectiveItalic[t], effectiveBold[t] });
        else
        {
            out.plain.append(piece);
            u16 += utf16Length(piece);
        }
    }

    // Emit a range for each maximal span with bold or italic on. Every effective
    // marker is paired, so the emphasis returns to plain by the paragraph's end.
    bool currentBold = false;
    bool currentItalic = false;
    size_t spanStart = 0;
    for (const Event& e : events)
    {
        if ((currentBold || currentItalic) && e.pos > spanStart)
            out.ranges.push_back({ spanStart, e.pos, currentBold, currentItalic });
        if (e.italic)
            currentItalic = !currentItalic;
        if (e.bold)
            currentBold = !currentBold;
        spanStart = e.pos;
    }
    return out;
}

/// One emphasis run within a placeholder, carrying the paragraph it sits in.
struct SlotRange
{
    int para;
    size_t startU16;
    size_t endU16;
    bool bold;
    bool italic;
};

/// The visible text of a placeholder and its emphasis runs. The paragraphs are
/// joined with newlines, the form SetText expects, and each run keeps the
/// paragraph index it belongs to.
struct SlotText
{
    std::string plain;
    std::vector<SlotRange> ranges;
};

SlotText buildSlotText(const std::vector<std::string>& paragraphs)
{
    SlotText out;
    for (size_t p = 0; p < paragraphs.size(); ++p)
    {
        const EmphasisText parsed = parseEmphasis(paragraphs[p]);
        if (p > 0)
            out.plain += '\n';
        out.plain += parsed.plain;
        for (const EmphasisRange& r : parsed.ranges)
            out.ranges.push_back(
                { static_cast<int>(p), r.startU16, r.endU16, r.bold, r.italic });
    }
    return out;
}

/// Collect the string items of a bullets block, skipping any non-string entry.
std::vector<std::string> collectItems(const Poco::JSON::Array::Ptr& items)
{
    std::vector<std::string> out;
    for (std::size_t i = 0; items && i < items->size(); ++i)
    {
        try
        {
            out.push_back(items->getElement<std::string>(i));
        }
        catch (const std::exception&)
        {
        }
    }
    return out;
}

/// Split a text block into paragraphs on newlines.
std::vector<std::string> splitLines(const std::string& text)
{
    std::vector<std::string> out;
    size_t start = 0;
    while (true)
    {
        const size_t newline = text.find('\n', start);
        if (newline == std::string::npos)
        {
            out.push_back(text.substr(start));
            break;
        }
        out.push_back(text.substr(start, newline - start));
        start = newline + 1;
    }
    return out;
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
                           const std::string& intent, const IntentRule& rule,
                           const DeckSpec::Budgets& budgets)
{
    const std::string prefix = slidePrefix(index, intent);

    Poco::JSON::Array::Ptr blocks;
    if (slide->has("blocks"))
    {
        blocks = slide->getArray("blocks");
        if (!blocks)
            return prefix + "\"blocks\" must be an array.";
    }

    // The block count is checked before the blocks themselves, so an
    // oversized array is rejected without walking every element.
    const int maxBlocks = rule.maxBullets + rule.maxText;
    if (blocks && static_cast<int>(blocks->size()) > maxBlocks)
        return prefix + "allows at most " + std::to_string(maxBlocks) + " block(s), found " +
               std::to_string(blocks->size()) + ".";

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
            if (static_cast<int>(items->size()) > budgets.maxItemsPerBullets)
                return prefix + "a bullets block may list at most " +
                       std::to_string(budgets.maxItemsPerBullets) + " items, found " +
                       std::to_string(items->size()) + ".";
            for (std::size_t it = 0; it < items->size(); ++it)
            {
                // Extraction converts a number or object element to its text,
                // so the element type is checked first: only a real JSON
                // string is a bullet item.
                const Poco::Dynamic::Var itemVar = items->get(it);
                if (!itemVar.isString())
                    return prefix + "every bullet item must be a string.";
                const std::string item = itemVar.extract<std::string>();
                if (item.empty())
                    return prefix + "a bullet item must not be empty.";
                // The emphasis markers are stripped before display, so the
                // length rule counts only the visible text.
                if (static_cast<int>(utf16Length(parseEmphasis(item).plain)) >
                    budgets.maxItemLength)
                    return prefix + "a bullet item exceeds " +
                           std::to_string(budgets.maxItemLength) + " characters.";
            }
        }
        else if (kind == "text")
        {
            ++textBlocks;
            const std::string text = getString(block, "text");
            if (text.empty())
                return prefix + "a text block needs a non-empty \"text\" string.";
            if (static_cast<int>(utf16Length(parseEmphasis(text).plain)) > budgets.maxItemLength)
                return prefix + "a text block exceeds " +
                       std::to_string(budgets.maxItemLength) + " characters.";
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

/// Push an EditTextObject command that applies the paragraph-level emphasis runs
/// to placeholder slot: each run selects its text and toggles bold and/or italic.
/// Emits nothing when there are no runs.
void pushEmphasis(Poco::JSON::Array::Ptr cmds, int slot, const std::vector<SlotRange>& ranges)
{
    if (ranges.empty())
        return;

    Poco::JSON::Array::Ptr sub = new Poco::JSON::Array();
    for (const SlotRange& r : ranges)
    {
        Poco::JSON::Object::Ptr select = new Poco::JSON::Object();
        Poco::JSON::Array::Ptr selectArgs = new Poco::JSON::Array();
        selectArgs->add(r.para);
        selectArgs->add(static_cast<int>(r.startU16));
        selectArgs->add(r.para);
        selectArgs->add(static_cast<int>(r.endU16));
        select->set("SelectText", selectArgs);
        sub->add(select);

        if (r.bold)
        {
            Poco::JSON::Object::Ptr uno = new Poco::JSON::Object();
            uno->set("UnoCommand", std::string(".uno:Bold"));
            sub->add(uno);
        }
        if (r.italic)
        {
            Poco::JSON::Object::Ptr uno = new Poco::JSON::Object();
            uno->set("UnoCommand", std::string(".uno:Italic"));
            sub->add(uno);
        }
    }

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

/// Emit the slide-command sequence for one slide onto cmds. When reuseCurrentSlide
/// is false a new slide is added first with InsertMasterSlide; when true the
/// commands fill the deck's current slide. The rest is the same for both the
/// whole-deck and the per-slide compiler: set the layout, the title, one SetText
/// per content block, a GenerateImage for an image slide, the house-style
/// formatting when no template is in use, and the slide's part and intent.
void emitSlideCommands(Poco::JSON::Array::Ptr cmds, const Poco::JSON::Object::Ptr& slide,
                       bool reuseCurrentSlide, const DeckSpec::CompileOptions& options)
{
    const std::string intent = getString(slide, "intent");
    const IntentRule* rule = findIntentRule(intent);
    if (!rule)
        return;

    const std::string part = getString(slide, "part");
    const std::string title = getString(slide, "title");

    if (!reuseCurrentSlide)
        pushCommand(cmds, "InsertMasterSlide", 0);

    pushCommand(cmds, "ChangeLayoutByName", std::string(rule->layout));
    // Titles carry no emphasis of their own; any markers are stripped.
    pushCommand(cmds, "SetText.0", parseEmphasis(title).plain);

    // Blocks fill the content placeholders in order, starting at slot 1. Bullet
    // items and text-block paragraphs may carry bold and italic emphasis, so each
    // slot's SetText gets the stripped text followed by the emphasis runs.
    std::vector<int> bulletSlots;
    int slot = 1;
    Poco::JSON::Array::Ptr blocks = slide->getArray("blocks");
    for (std::size_t b = 0; blocks && b < blocks->size(); ++b)
    {
        Poco::JSON::Object::Ptr block = blocks->getObject(b);
        if (!block)
            continue;
        const std::string kind = getString(block, "kind");
        if (kind == "bullets")
        {
            const SlotText slotText = buildSlotText(collectItems(block->getArray("items")));
            pushCommand(cmds, "SetText." + std::to_string(slot), slotText.plain);
            pushEmphasis(cmds, slot, slotText.ranges);
            bulletSlots.push_back(slot);
            ++slot;
        }
        else if (kind == "text")
        {
            const SlotText slotText = buildSlotText(splitLines(getString(block, "text")));
            pushCommand(cmds, "SetText." + std::to_string(slot), slotText.plain);
            pushEmphasis(cmds, slot, slotText.ranges);
            ++slot;
        }
    }

    // An image slide fills its content placeholder with a generated image. The
    // prompt puts the slide's brief first and appends the deck's image style, and
    // the alt text rides along so the inserted image gets an accessibility label.
    if (rule->requiresImage)
    {
        Poco::JSON::Object::Ptr image = slide->getObject("image");
        if (image)
        {
            std::string prompt = getString(image, "brief");
            if (!options.artDirection.empty())
                prompt += ". Style: " + options.artDirection;

            Poco::JSON::Object::Ptr generate = new Poco::JSON::Object();
            generate->set("prompt", prompt);
            generate->set("alt", getString(image, "alt"));

            Poco::JSON::Object::Ptr cmd = new Poco::JSON::Object();
            cmd->set("GenerateImage.1", generate);
            cmds->add(cmd);
        }
    }

    // Speaker notes are slide content, not house style, so they ride along in
    // both template modes.
    const std::string notes = getString(slide, "notes");
    if (!notes.empty())
        pushCommand(cmds, "SetNotes", parseEmphasis(notes).plain);

    if (!options.haveDesignTemplate)
        pushEditTextObject(cmds, 0, ".uno:Bold");

    // A plain SetText fills the placeholder with unmarked paragraphs, so every
    // deck turns bullet markers on for its bullet slots.
    for (int bulletSlot : bulletSlots)
        pushEditTextObject(cmds, bulletSlot, ".uno:DefaultBullet");

    pushCommand(cmds, "SetSlidePart", part);
    // The intent is a finer label than the part, and a template manifest may map
    // it to a master of its own.
    pushCommand(cmds, "SetSlideIntent", intent);
}

} // anonymous namespace

namespace DeckSpec
{

std::string limitsSentence(const Budgets& budgets)
{
    return "\n\nLimits: at most " + std::to_string(budgets.maxSlides) +
           " slides, at most " + std::to_string(budgets.maxItemsPerBullets) +
           " items per bullets block, and keep each item short. Do not prefix items with"
           " \"- \"; bullet markers are added for you, so put only the items themselves in"
           " each block.";
}

bool isKnownPart(const std::string& part)
{
    for (const auto& name : PartNames)
        if (part == name)
            return true;
    return false;
}

bool isKnownIntent(const std::string& intent)
{
    return findIntentRule(intent) != nullptr;
}

std::string partList()
{
    std::string list;
    for (const auto& name : PartNames)
    {
        if (!list.empty())
            list += ", ";
        list += name;
    }
    return list;
}

std::string intentList()
{
    std::string list;
    for (const auto& rule : IntentRules)
    {
        if (!list.empty())
            list += ", ";
        list += rule.intent;
    }
    return list;
}

std::optional<std::string> validateSlideSpec(const Poco::JSON::Object::Ptr& slideObj,
                                             unsigned index, const Budgets& budgets)
{
    if (!slideObj)
        return "Slide " + std::to_string(index + 1) + " must be a JSON object.";

    const std::string part = getString(slideObj, "part");
    if (!isKnownPart(part))
        return slidePrefix(index, std::string()) + "\"part\" must be one of " + partList() + ".";

    const std::string intent = getString(slideObj, "intent");
    const IntentRule* rule = findIntentRule(intent);
    if (!rule)
        return slidePrefix(index, std::string()) + "\"intent\" must be one of " + intentList() +
               ".";

    const std::string title = getString(slideObj, "title");
    if (title.empty())
        return slidePrefix(index, intent) +
               "\"title\" is required and must be a non-empty string.";
    if (static_cast<int>(utf16Length(title)) > budgets.maxTitleLength)
        return slidePrefix(index, intent) + "title exceeds " +
               std::to_string(budgets.maxTitleLength) + " characters.";

    const std::string blocksError = validateBlocks(slideObj, index, intent, *rule, budgets);
    if (!blocksError.empty())
        return blocksError;

    if (rule->requiresImage)
    {
        Poco::JSON::Object::Ptr image = slideObj->getObject("image");
        if (!image || getString(image, "brief").empty() || getString(image, "alt").empty())
            return slidePrefix(index, intent) + "an image slide needs an \"image\" object with a"
                                                " non-empty \"brief\" and \"alt\".";
        const std::string alt = getString(image, "alt");
        if (static_cast<int>(utf16Length(alt)) > budgets.maxItemLength)
            return slidePrefix(index, intent) + "image alt text exceeds " +
                   std::to_string(budgets.maxItemLength) + " characters.";
        // The brief becomes an image-generation prompt, so it gets the roomiest
        // text budget, the one speaker notes use.
        const std::string brief = getString(image, "brief");
        if (static_cast<int>(utf16Length(brief)) > budgets.maxNotesLength)
            return slidePrefix(index, intent) + "image brief exceeds " +
                   std::to_string(budgets.maxNotesLength) + " characters.";
    }

    const std::string notes = getString(slideObj, "notes");
    if (static_cast<int>(utf16Length(notes)) > budgets.maxNotesLength)
        return slidePrefix(index, intent) + "notes exceed " +
               std::to_string(budgets.maxNotesLength) + " characters.";

    return std::nullopt;
}

std::optional<std::string> validateOutline(const Poco::JSON::Object::Ptr& outlineObj,
                                           const Budgets& budgets)
{
    if (!outlineObj)
        return std::string("Outline must be a JSON object with a \"slides\" array.");

    const std::string deckTitle = getString(outlineObj, "title");
    if (static_cast<int>(utf16Length(deckTitle)) > budgets.maxTitleLength)
        return "The deck title exceeds " + std::to_string(budgets.maxTitleLength) + " characters.";

    Poco::JSON::Array::Ptr slides = outlineObj->getArray("slides");
    if (!slides)
        return std::string("Outline must have a \"slides\" array.");
    if (slides->size() == 0)
        return std::string("An outline needs at least one slide.");
    if (static_cast<int>(slides->size()) > budgets.maxSlides)
        return "An outline may have at most " + std::to_string(budgets.maxSlides) + " slides, found " +
               std::to_string(slides->size()) + ".";

    for (std::size_t i = 0; i < slides->size(); ++i)
    {
        Poco::JSON::Object::Ptr slide = slides->getObject(i);
        if (!slide)
            return "Slide " + std::to_string(i + 1) + " must be a JSON object.";

        const std::string part = getString(slide, "part");
        if (!isKnownPart(part))
            return slidePrefix(i, std::string()) + "\"part\" must be one of " + partList() + ".";

        const std::string intent = getString(slide, "intent");
        if (!isKnownIntent(intent))
            return slidePrefix(i, std::string()) + "\"intent\" must be one of " + intentList() +
                   ".";

        const std::string title = getString(slide, "title");
        if (title.empty())
            return slidePrefix(i, intent) +
                   "\"title\" is required and must be a non-empty string.";
        if (static_cast<int>(utf16Length(title)) > budgets.maxTitleLength)
            return slidePrefix(i, intent) + "title exceeds " +
                   std::to_string(budgets.maxTitleLength) + " characters.";

        const std::string gist = getString(slide, "gist");
        if (static_cast<int>(utf16Length(gist)) > budgets.maxGistLength)
            return slidePrefix(i, intent) + "gist exceeds " +
                   std::to_string(budgets.maxGistLength) + " characters.";
    }

    return std::nullopt;
}

Poco::JSON::Object::Ptr sanitizeOutline(const Poco::JSON::Object::Ptr& outlineObj)
{
    Poco::JSON::Object::Ptr clean = new Poco::JSON::Object();
    Poco::JSON::Array::Ptr cleanSlides = new Poco::JSON::Array();
    clean->set("title", flattenToOneLine(getString(outlineObj, "title")));
    clean->set("slides", cleanSlides);
    if (!outlineObj)
        return clean;

    Poco::JSON::Array::Ptr slides = outlineObj->getArray("slides");
    for (std::size_t i = 0; slides && i < slides->size(); ++i)
    {
        Poco::JSON::Object::Ptr slide = slides->getObject(i);
        if (!slide)
            continue;
        Poco::JSON::Object::Ptr cleanSlide = new Poco::JSON::Object();
        cleanSlide->set("part", getString(slide, "part"));
        cleanSlide->set("intent", getString(slide, "intent"));
        cleanSlide->set("title", flattenToOneLine(getString(slide, "title")));
        cleanSlide->set("gist", flattenToOneLine(getString(slide, "gist")));
        cleanSlides->add(cleanSlide);
    }

    return clean;
}

std::string compileSlideSpec(const Poco::JSON::Object::Ptr& slideObj, int docSlideIndex,
                             const CompileOptions& options)
{
    Poco::JSON::Array::Ptr cmds = new Poco::JSON::Array();
    if (!slideObj)
        return transformString(cmds);

    // Each per-slide transform runs in a fresh engine context whose current
    // slide is index 0. The first built slide reuses the deck's single starting
    // slide; a later slide first moves to the end of the deck so the new slide
    // is inserted after the ones already built.
    const bool reuseCurrentSlide = (docSlideIndex == 0);
    if (!reuseCurrentSlide)
        pushCommand(cmds, "JumpToSlide", std::string("last"));

    emitSlideCommands(cmds, slideObj, reuseCurrentSlide, options);

    return transformString(cmds);
}

std::vector<ImageInsertion>
rewriteGenerateImageCommands(const Poco::JSON::Object::Ptr& transformObj, int nExistingSlides,
                             const std::string& placeholderUrl)
{
    std::vector<ImageInsertion> insertions;

    Poco::JSON::Object::Ptr transforms =
        transformObj ? transformObj->getObject("Transforms") : nullptr;
    Poco::JSON::Array::Ptr cmds = transforms ? transforms->getArray("SlideCommands") : nullptr;
    if (!cmds)
        return insertions;

    // Track the current slide as the commands are scanned. It starts at the
    // number of slides already in the document so a JumpToSlide "last" resolves
    // to the real last page and a following InsertMasterSlide lands the new
    // slide at the same absolute index the engine gives it.
    int currentSlide = 0;
    int pageCount = nExistingSlides;

    for (std::size_t i = 0; i < cmds->size(); ++i)
    {
        Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
        if (!cmd)
            continue;

        if (cmd->has("JumpToSlide"))
        {
            std::string val = cmd->getValue<std::string>("JumpToSlide");
            if (val == "last")
                currentSlide = pageCount - 1;
            else
            {
                try
                {
                    currentSlide = std::stoi(val);
                }
                catch (const std::exception&)
                {
                    LOG_WRN("TransformImageGen: invalid JumpToSlide value: " << val);
                }
            }
        }
        else if (cmd->has("InsertMasterSlide") || cmd->has("InsertMasterSlideByName"))
        {
            currentSlide++;
            pageCount++;
        }
        else if (cmd->has("DeleteSlide"))
        {
            if (pageCount > 1)
                pageCount--;
        }

        static constexpr std::string_view kGenerateImagePrefix = "GenerateImage.";
        for (const auto& key : cmd->getNames())
        {
            if (key.substr(0, kGenerateImagePrefix.size()) != kGenerateImagePrefix)
                continue;

            int objId;
            try
            {
                objId = std::stoi(key.substr(kGenerateImagePrefix.size()));
            }
            catch (const std::exception&)
            {
                LOG_WRN("TransformImageGen: invalid GenerateImage key: " << key);
                continue;
            }

            // GenerateImage carries either an object {"prompt","alt"} from the
            // deck compiler or a bare prompt string from the imperative tool.
            // Read both; the alt is empty for the string form.
            std::string prompt;
            std::string alt;
            const Poco::Dynamic::Var value = cmd->get(key);
            if (value.type() == typeid(Poco::JSON::Object::Ptr))
            {
                Poco::JSON::Object::Ptr generate = value.extract<Poco::JSON::Object::Ptr>();
                JsonUtil::findJSONValue(generate, "prompt", prompt);
                JsonUtil::findJSONValue(generate, "alt", alt);
            }
            else if (!value.isEmpty())
                prompt = value.toString();

            insertions.push_back({ currentSlide, objId, std::move(prompt), std::move(alt) });

            // Replace GenerateImage.N with an InsertImage.N pointing at the
            // loading placeholder; the real image is filled in later.
            cmd->remove(key);
            cmd->set("InsertImage." + std::to_string(objId), placeholderUrl);
        }
    }

    return insertions;
}

std::string buildExpansionUserMessage(const Poco::JSON::Object::Ptr& slideObj,
                                      unsigned slideNumber, unsigned slideCount,
                                      const std::string& retryError)
{
    std::string message = "Write slide " + std::to_string(slideNumber) + " of " +
                          std::to_string(slideCount) + ". part: " + getString(slideObj, "part") +
                          "; intent: " + getString(slideObj, "intent") +
                          "; title: " + getString(slideObj, "title") +
                          "; gist: " + getString(slideObj, "gist") +
                          " . Provide the blocks this intent expects.";
    if (!retryError.empty())
        message += " Your previous attempt was rejected: " + retryError + ". Fix exactly that.";
    return message;
}

} // namespace DeckSpec

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
