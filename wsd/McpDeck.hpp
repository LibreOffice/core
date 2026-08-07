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

} // namespace McpDeck

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
