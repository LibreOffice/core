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

/*
 * Declarative deck description for AI slide generation. A deck spec is a small,
 * schema-validated JSON shape in which the model describes what each slide is
 * about (its part and intent, a title, and content blocks) rather than driving
 * the slide-command editor directly. The compiler turns a valid deck spec into
 * the existing Impress SlideCommands transform, the same shape a model-written
 * transform takes.
 *
 * Pure functions, free of session state, so they can be unit-tested in
 * isolation.
 */

#pragma once

#include <Poco/JSON/Object.h>

#include <optional>
#include <string>
#include <vector>

namespace DeckSpec
{

/// The largest deck the compiler accepts, in slides.
constexpr int MaxSlides = 30;
/// The largest number of items one bullets block may list.
constexpr int MaxItemsPerBullets = 6;
/// The largest length, in characters, of a single bullet item or text block.
constexpr int MaxItemLength = 200;
/// The largest length, in characters, of a slide title.
constexpr int MaxTitleLength = 200;
/// The largest length, in characters, of a slide's one or two sentence outline
/// gist.
constexpr int MaxGistLength = 300;
/// The largest length, in characters, of a slide's speaker notes.
constexpr int MaxNotesLength = 500;

/// The limits a deck spec is checked against. Every field defaults to the
/// matching constexpr limit above, so a default-constructed Budgets reproduces
/// the built-in limits and stays the single source of the default values. A
/// template manifest may lower these; the alt-text length reuses maxItemLength
/// rather than adding a separate knob. The length limits count UTF-16 code
/// units, the unit the engine addresses text in, so a non-ASCII character
/// still counts as one (or, outside the BMP, two).
struct Budgets
{
    int maxSlides = MaxSlides;
    int maxItemsPerBullets = MaxItemsPerBullets;
    int maxItemLength = MaxItemLength;
    int maxTitleLength = MaxTitleLength;
    int maxGistLength = MaxGistLength;
    int maxNotesLength = MaxNotesLength;
};

/// The settings that shape a compiled deck. haveDesignTemplate is true when the
/// user picked a design template, whose masters then style the slides, so the
/// compiled commands carry no house-style formatting of their own. artDirection
/// is the image style appended to every generated-image prompt; it is empty only
/// when no style applies, in which case the image prompt is the brief alone.
struct CompileOptions
{
    bool haveDesignTemplate = false;
    std::string artDirection;
};

/// The one-line "Limits: ..." sentence for the deck tool descriptions, built
/// from the budgets so the numbers the model is told match the numbers the
/// validator enforces. It begins with a blank line so it reads as its own
/// paragraph when appended after the slide-shape text. With default budgets it
/// reproduces today's wording exactly.
std::string limitsSentence(const Budgets& budgets);

/// True when the name is one of the deck's slide-part names.
bool isKnownPart(const std::string& part);
/// True when the name is one of the known slide intents.
bool isKnownIntent(const std::string& intent);
/// The comma-separated list of valid part names, for error messages.
std::string partList();
/// The comma-separated list of valid intent names, for error messages.
std::string intentList();

/// Check one slide of a deck spec against the schema and budgets: its part,
/// intent, title, content blocks, and image. Returns an actionable error string
/// or std::nullopt when the slide satisfies every rule. The index is zero-based
/// and only used to name the slide (counted from 1) in the message.
std::optional<std::string> validateSlideSpec(const Poco::JSON::Object::Ptr& slideObj,
                                             unsigned index, const Budgets& budgets);

/// Check an outline against the schema and budgets. An outline names each
/// slide's part, intent, title and an optional gist, without the content blocks
/// a full deck spec carries; it is what the user reviews and edits before the
/// slides are built. The object is {title?, slides:[{part,intent,title,gist?}]}.
/// Returns an actionable error string or std::nullopt when it satisfies every
/// rule. The browser may edit an outline before sending it back, so this is run
/// again on the returned outline before it is used.
std::optional<std::string> validateOutline(const Poco::JSON::Object::Ptr& outlineObj,
                                           const Budgets& budgets);

/// A fresh outline object holding only the fields validateOutline checks: the
/// deck title, and per slide the part, intent, title and gist. Any other key is
/// left out, and every carriage return, line feed and NUL in a title or a gist
/// becomes a space, so each stays one line. A null outline gives an outline with
/// an empty title and no slides.
Poco::JSON::Object::Ptr sanitizeOutline(const Poco::JSON::Object::Ptr& outlineObj);

/// Compile one slide spec into a SlideCommands transform JSON string. Each such
/// transform is applied to the deck on its own, so docSlideIndex says where this
/// slide sits among the slides already built: index 0 fills the deck's starting
/// slide, a later index first moves to the end of the deck and inserts a new
/// slide before filling it. The options decide whether house-style formatting is
/// emitted and the image style appended to generated-image prompts.
std::string compileSlideSpec(const Poco::JSON::Object::Ptr& slideObj, int docSlideIndex,
                             const CompileOptions& options);

/// One generated image to insert, with the slide it belongs on. slideIndex is
/// the absolute zero-based page. objId is the placeholder object index (the N
/// from GenerateImage.N). prompt is the image description; alt is the text
/// alternative, empty when none was given.
struct ImageInsertion
{
    int slideIndex;
    int objId;
    std::string prompt;
    std::string alt;
};

/// Scan a compiled transform for GenerateImage.N commands and rewrite each into
/// an InsertImage.N that points at placeholderUrl, returning one ImageInsertion
/// per rewritten command with the slide it targets. The transform object is
/// modified in place. nExistingSlides is the number of slides already in the
/// document when this transform runs, the same base index compileSlideSpec was
/// given, so a JumpToSlide "last" resolves to the real last page and a following
/// InsertMasterSlide lands the new slide at the right absolute index. The
/// GenerateImage value is either an object with prompt and alt or a bare prompt
/// string; both are read.
std::vector<ImageInsertion>
rewriteGenerateImageCommands(const Poco::JSON::Object::Ptr& transformObj, int nExistingSlides,
                             const std::string& placeholderUrl);

/// Compose the user message that asks the model to write one slide during
/// outline expansion. slideNumber and slideCount are 1-based for display. The
/// slide object carries the approved outline entry's part, intent, title and
/// gist. When retryError is non-empty the message ends by asking the model to
/// fix exactly the problem the previous attempt hit.
std::string buildExpansionUserMessage(const Poco::JSON::Object::Ptr& slideObj,
                                      unsigned slideNumber, unsigned slideCount,
                                      const std::string& retryError);

} // namespace DeckSpec

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
