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
#include <string>

namespace AIUtil
{

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
