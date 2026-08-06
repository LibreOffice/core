/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <sddllapi.h>
#include <xmloff/autolayout.hxx>

#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

class SdDrawDocument;

namespace sd
{
/** The design templates available to apply to a deck, both the bundled set and
    any preset templates an integrator or user has supplied. Each pair is the
    template's display name (its file base name, in its original letter case) and
    the URL of its template document. The bundled set comes first; a later
    template whose name has already been seen is skipped, so a bundled template
    takes precedence over a preset that shares its name. */
std::vector<std::pair<OUString, OUString>> CollectDesignTemplates();

/** The part a master slide plays in a design template. A deck is themed by
    placing each generated slide on the master that fits its part: the opening
    on a Title master, a section break on a Divider, a body slide on Content, a
    final slide on Closing. Unknown means the master's name carries no part
    keyword, so the part is decided from the layout of its example slide
    instead. */
enum class DesignMasterRole
{
    Title,
    Divider,
    Content,
    Closing,
    Unknown
};

/** The part suggested by the role keywords in a master's name, or Unknown when
    the name carries none. The match is case-insensitive and looks for a keyword
    anywhere in the name as a substring, not as a whole word. The keyword groups
    are tested in a fixed order, so a name that carries more than one keyword
    takes the stronger part: first the divider words ("separator", "divider",
    "section", "topic", ...), then the closing words ("ending", "closing",
    "thank", ...), then the body words ("content", "outline", "body", ...), and
    "title" last. That order keeps a name like "Section Title" a divider rather
    than a title. */
SD_DLLPUBLIC DesignMasterRole DesignMasterRoleFromName(std::u16string_view rName);

/** The name a slide part travels under between the model and the engine:
    "opening" for Title, "divider" for Divider, "body" for Content, "closing"
    for Closing, and "other" for a master that plays no part. The words are a
    wire contract, so changing one changes the protocol, not just this
    mapping. */
SD_DLLPUBLIC OUString DesignRoleToWireName(DesignMasterRole eRole);

/** The slide part a wire name stands for, or no value when the word is not one
    of the four part names. The match is exact and case-sensitive. */
SD_DLLPUBLIC std::optional<DesignMasterRole> WireNameToDesignRole(std::u16string_view rName);

/** A design manifest a template declares in its document metadata, read from a
    JSON string stored in the standard ODF user-defined property named
    AIDesignManifest. The manifest states directly what the name-and-example
    heuristic would otherwise guess, and adds fields the heuristic cannot carry.
    Every field is optional: a template may declare only some of them, and a
    template with no manifest, an unparsable one, or one whose version is not
    recognised is treated as having no manifest at all, so the heuristic runs as
    before. */
struct AIDesignManifest
{
    /// The part each named master plays. Only masters that exist in the template
    /// and carry one of the four part wire names are kept.
    std::unordered_map<OUString, DesignMasterRole> maMasterRoles;
    /// A master to prefer for a slide whose intent matches the key. The key is
    /// an opaque lowercase word the model may send; the value names a master
    /// that exists in the template.
    std::unordered_map<OUString, OUString> maIntentMasters;
    /// One sentence describing the image style, used only to compose
    /// image-generation prompts.
    OUString maArtDirection;
    /// Upper limits the model should keep within. Each is present only when the
    /// manifest declared it. They may only tighten the server's own limits, so
    /// a manifest cannot raise a limit above what the server allows.
    std::optional<sal_Int32> moMaxSlides;
    std::optional<sal_Int32> moMaxItemsPerBullets;
    std::optional<sal_Int32> moMaxItemLength;
    std::optional<sal_Int32> moMaxTitleLength;
};

/** The design manifest a template declares, or no value when the template
    carries none, its manifest cannot be parsed, or its schema version is not
    recognised. In every one of those cases the caller falls back to the
    name-and-example heuristic. */
SD_DLLPUBLIC std::optional<AIDesignManifest> ReadAIDesignManifest(SdDrawDocument& rTemplate);

/** One standard master in a design template, with the part it plays. */
struct DesignTemplateMaster
{
    /// The master's name, as the template stores it.
    OUString maName;
    /// The part the master plays, taken from the template's declared manifest,
    /// else from its name when the name carries a keyword, else from the layout
    /// of an example slide that uses it.
    DesignMasterRole meRole;
    /// The layout of an example slide that uses the master, or AUTOLAYOUT_NONE
    /// when no example slide does.
    AutoLayout meExampleLayout;
    /// How many of the template's example slides use the master.
    sal_uInt16 mnExampleUses;
    /// True when the part came from the template's declared manifest rather than
    /// from the name-and-example heuristic.
    bool mbDeclared = false;
};

/** The standard masters of a design template, each classified by part. The
    classification is shared so the master list offered to the model and the
    fallback that places slides without a model choice agree on every part. */
SD_DLLPUBLIC std::vector<DesignTemplateMaster> CollectDesignTemplateMasters(SdDrawDocument& rTemplate);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
