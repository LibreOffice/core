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
 * The design templates the Model Context Protocol deck tools offer.
 *
 * A design template is an .otp document installed next to the document engine. It may declare a
 * design manifest in its metadata, a small JSON object naming the part each of its master slides
 * plays, the image style its pictures should follow, and limits it wants a deck to stay within.
 * This reads that manifest straight from the template file, so listing the designs needs no
 * document engine process.
 */

#pragma once

#include <config.h>

#include <wsd/DeckSpec.hpp>

#include <Poco/JSON/Object.h>

#include <optional>
#include <string>
#include <vector>

namespace McpDeck
{

/// One design template a deck can be built with.
struct DesignTemplate
{
    /// The name the template is picked by, its file name without the extension.
    std::string name;
    /// The slide parts the template declares a master design for, in the order "opening",
    /// "divider", "body", "closing", leaving out any the template does not cover. Empty when the
    /// template declares no manifest.
    std::vector<std::string> parts;
    /// The one sentence describing the image style, already sanitized. Empty when the template
    /// declares none.
    std::string artDirection;
    /// The limits a deck built on this template is checked against: the configured budgets with
    /// every limit the manifest declares applied as a lower bound.
    DeckSpec::Budgets budgets;
};

/// The design templates installed alongside the document engine, in the order the engine itself
/// finds them: the bundled set first, then the per-config preset templates, and the first template
/// seen for a name wins. A template whose file name is not a safe design name is left out.
std::vector<DesignTemplate> installedDesignTemplates();

/// The design manifest a template declares, as the raw JSON string held in its AIDesignManifest
/// metadata property. Empty when the file cannot be read, is not a zip, holds no metadata part, or
/// declares no manifest.
std::string readDesignManifest(const std::string& templatePath);

/// The design template a name and a manifest describe. The manifest is the raw JSON string; an
/// empty, unparsable, or below-version-1 manifest gives a template with only its name filled in and
/// the configured budgets. Budget limits apply as lower bounds, so a manifest can only tighten what
/// the server allows.
DesignTemplate designTemplateFromManifest(const std::string& name,
                                          const std::string& manifestJson);

/// The design of the given name, or no value when no template goes by it. The name is matched
/// without regard to case, the way the document engine matches it.
std::optional<DesignTemplate> findDesignTemplate(const std::string& name);

/// A deck turned into the commands that build it.
struct CompiledDeck
{
    /// True when the deck is ready to run, false when the spec was refused.
    bool ok = false;
    /// When ok is false, a readable message naming the slide at fault and what is wrong with it,
    /// meant to be handed to the calling model so it can send a corrected spec.
    std::string errorText;
    /// When ok is true, the transform JSON that builds the whole deck.
    std::string transformJSON;
};

/// Turn a deck spec into the single transform that builds it.
///
/// The spec is {"slides": [ ... ]}, each entry a slide in the shape DeckSpec::validateSlideSpec
/// accepts. Every slide is checked against the design's budgets before any of them is compiled, so
/// a spec is either built whole or refused whole. The slides compile into one command sequence
/// applied in order, led by the command that puts the deck on the design when one is named.
///
/// A slide whose intent asks for a generated picture is refused: this path has no image generator
/// behind it.
CompiledDeck compileDeck(const Poco::JSON::Object::Ptr& spec, const DesignTemplate& design);

/// A presentation of one empty slide, as the bytes of a flat ODF document. It is the document a new
/// deck starts from, which is why a deck's first slide fills a slide that is already there.
std::string blankPresentation();

} // namespace McpDeck

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
