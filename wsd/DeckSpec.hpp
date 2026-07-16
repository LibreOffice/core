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

/// Check a deck spec against the schema and the budgets. Returns an actionable
/// error string that names the offending slide (counted from 1) and the rule it
/// broke, or std::nullopt when the spec satisfies every rule. The message is
/// meant to be fed back to the model so it can fix exactly what is wrong.
std::optional<std::string> validateDeckSpec(const Poco::JSON::Object::Ptr& deckObj);

/// Compile a deck spec into the SlideCommands transform JSON string
/// ({"Transforms":{"SlideCommands":[...]}}). The spec is expected to have passed
/// validateDeckSpec; a structural surprise yields an empty SlideCommands array
/// rather than a throw. When haveDesignTemplate is true the compiler emits no
/// house-style formatting commands, because the template's master slides own the
/// look; when false it emits the bold-title and default-bullet commands itself.
std::string compileDeckSpec(const Poco::JSON::Object::Ptr& deckObj, bool haveDesignTemplate);

} // namespace DeckSpec

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
