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
 * Shared helpers for the AI assistant. Pure functions, free of session state,
 * so they can be unit-tested in isolation. Add further AI utilities here.
 */

#pragma once

#include <Poco/JSON/Object.h>

#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace AIUtil
{

/// Base URL for a built-in AI provider id (openai, groq, together, mistral),
/// or an empty view when the id is not one of them. Keep the list in sync with
/// AI_PROVIDERS in browser/admin/src/integrator/AdminIntegratorSettings.ts.
std::string_view preCannedAIProviderBaseUrl(std::string_view id);

/// True when the host belongs to one of the built-in AI providers above. These
/// are fixed public endpoints, so outbound AI requests to them are trusted
/// without needing an entry in the net.lok_allow host allowlist; a custom
/// provider's host still goes through that allowlist.
bool isPreCannedAIProviderHost(std::string_view host);

/// Return the base URL reduced to its bare origin: any trailing slashes and a
/// single trailing "/v1" segment removed (the "/v1" match is case-insensitive).
/// An empty input returns empty. This is the canonical form to which a version
/// path such as "/v1/chat/completions" is appended, so a stored value that
/// already ends in "/v1" does not produce a doubled "/v1/v1/..." path.
std::string normalizeAIBaseUrl(std::string_view baseUrl);

/// One command of the Impress SlideCommands vocabulary. The table behind
/// getSlideCommands() is the single source of truth for the vocabulary.
struct SlideCommandInfo
{
    /// Base command name, the part of a command key before any ".N" suffix.
    std::string_view name;
    /// True when the model may emit the command itself. False marks a
    /// command the server alone splices into a transform.
    bool allowedFromModel;
    /// Title of the documentation section the command is listed under.
    /// Commands sharing a title are grouped, in table order. Empty for a
    /// command with no documentation lines.
    std::string_view docSection;
    /// Documentation lines for the command, separated by newlines, without
    /// a trailing newline. Empty for an undocumented command.
    std::string_view docLines;
};

/// The Impress SlideCommands vocabulary, in documentation order.
std::span<const SlideCommandInfo> getSlideCommands();

/// True when the key's base name (before any ".N" suffix) is a SlideCommands
/// command the server alone may add to a transform.
bool isServerOnlySlideCommand(const std::string& key);

/// One slide layout in the set offered to the model.
struct SlideLayoutInfo
{
    /// The AUTOLAYOUT_* name.
    std::string_view name;
    /// The numeric AutoLayout id matching the name.
    int id;
    /// How many placeholder objects the layout puts on a slide, counting the
    /// title. Placeholders are addressed by index from 0, so the highest index
    /// a layout offers is one less than this. A layout with no placeholders at
    /// all has 0.
    int placeholderCount;
    /// One-line description for the model-facing documentation.
    std::string_view description;
};

/// The slide layouts offered to the model, in documentation order.
std::span<const SlideLayoutInfo> getSlideLayouts();

/// True when the name is one of the layouts in getSlideLayouts().
bool isKnownSlideLayout(const std::string& name);

/// The SlideCommands and layout portion of the Impress transform
/// documentation, built from getSlideCommands() and getSlideLayouts().
const std::string& getSlideCommandDocs();

/// Parse a tool's argument JSON. Most models emit a single object ({...}), but
/// some emit a JSON array of objects ([{...},{...}]); when that happens, merge
/// all element objects into one so downstream lookups by key continue to work.
bool parseLenientArgs(const std::string& argsJson, Poco::JSON::Object::Ptr& argsObj);

/// Lightweight structural validation of a transform_document_structure payload,
/// run before the user approval dialog so the model can self-correct silently.
/// Returns an actionable error message for the first structural problem, or
/// std::nullopt when the structure is plausible. Intentionally conservative: it
/// only flags clear shape errors (no Transforms/UnoCommand, non-array
/// SlideCommands, or an unrecognized Impress command), leaving deeper semantics
/// to the kit.
std::optional<std::string> validateTransformStructure(
    const Poco::JSON::Object::Ptr& transformObj);

} // namespace AIUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
