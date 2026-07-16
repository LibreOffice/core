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

namespace DeckSpec
{

/// The largest deck the compiler accepts, in slides.
inline constexpr int MaxSlides = 30;
/// The largest number of items one bullets block may list.
inline constexpr int MaxItemsPerBullets = 6;
/// The largest length, in characters, of a single bullet item or text block.
inline constexpr int MaxItemLength = 200;
/// The largest length, in characters, of a slide title.
inline constexpr int MaxTitleLength = 200;
/// The largest length, in characters, of a slide's one or two sentence outline
/// gist.
inline constexpr int MaxGistLength = 300;

/// True when the name is one of the deck's slide-part names.
bool isKnownPart(const std::string& part);
/// True when the name is one of the known slide intents.
bool isKnownIntent(const std::string& intent);
/// The comma-separated list of valid part names, for error messages.
std::string partList();
/// The comma-separated list of valid intent names, for error messages.
std::string intentList();

/// Check a deck spec against the schema and the budgets. Returns an actionable
/// error string that names the offending slide (counted from 1) and the rule it
/// broke, or std::nullopt when the spec satisfies every rule. The message is
/// meant to be fed back to the model so it can fix exactly what is wrong.
std::optional<std::string> validateDeckSpec(const Poco::JSON::Object::Ptr& deckObj);

/// Check one slide of a deck spec against the schema and budgets: its part,
/// intent, title, content blocks, and image. Returns an actionable error string
/// or std::nullopt when the slide satisfies every rule. The index is zero-based
/// and only used to name the slide (counted from 1) in the message.
std::optional<std::string> validateSlideSpec(const Poco::JSON::Object::Ptr& slideObj,
                                             unsigned index);

/// Check an outline against the schema and budgets. An outline names each
/// slide's part, intent, title and an optional gist, without the content blocks
/// a full deck spec carries; it is what the user reviews and edits before the
/// slides are built. The object is {title?, slides:[{part,intent,title,gist?}]}.
/// Returns an actionable error string or std::nullopt when it satisfies every
/// rule. The browser may edit an outline before sending it back, so this is run
/// again on the returned outline before it is used.
std::optional<std::string> validateOutline(const Poco::JSON::Object::Ptr& outlineObj);

/// Compile a deck spec into the SlideCommands transform JSON string
/// ({"Transforms":{"SlideCommands":[...]}}). The spec is expected to have passed
/// validateDeckSpec; a structural surprise yields an empty SlideCommands array
/// rather than a throw. When haveDesignTemplate is true the compiler emits no
/// house-style formatting commands, because the template's master slides own the
/// look; when false it emits the bold-title and default-bullet commands itself.
std::string compileDeckSpec(const Poco::JSON::Object::Ptr& deckObj, bool haveDesignTemplate);

/// Compile one slide spec into a SlideCommands transform JSON string. Each such
/// transform is applied to the deck on its own, so docSlideIndex says where this
/// slide sits among the slides already built: index 0 fills the deck's starting
/// slide, a later index first moves to the end of the deck and inserts a new
/// slide before filling it. haveDesignTemplate is true when the user picked a
/// design template, whose masters then style the slide, so the compiled
/// commands carry no formatting of their own.
std::string compileSlideSpec(const Poco::JSON::Object::Ptr& slideObj, int docSlideIndex,
                             bool haveDesignTemplate);

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
