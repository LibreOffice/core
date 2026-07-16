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
 * Shared tool descriptions for document operations.
 * Used by both the MCP endpoint and the AI sidebar tool definitions.
 *
 * The transform_document_structure description is split into per-document-type
 * fragments so callers can advertise only the operations relevant to the open
 * document. The caller composes the intro with one document-type fragment.
 */

#pragma once

namespace DocumentToolDescriptions
{

/// Description for the extract_link_targets tool.
inline constexpr const char* EXTRACT_LINK_TARGETS_DESCRIPTION =
    "Extract all link targets from a document. Returns a JSON object with "
    "categories: Headings, Bookmarks, Tables, Frames, Images, Sections, "
    "OLE objects, Drawing objects. Each entry maps a name to a target string "
    "(e.g. \"Table1\": \"Table1|table\"). These targets can be used to open the "
    "document at a specific position.";

/// extract_document_structure descriptions, split per document type so each
/// open document advertises only the filters that work for it. The caller
/// composes the intro with the matching document-type fragment(s).
inline constexpr const char* EXTRACT_INTRO = "Inspect the open document as JSON.";

inline constexpr const char* EXTRACT_WRITER =
    " With filter=\"text\" it returns the document body as markdown for "
    "summarizing or answering questions about the content. To answer about a "
    "specific part without reading the whole body, first call "
    "extract_link_targets to get the heading or section target, then call this "
    "with filter=\"text\" and the target argument set to that target string "
    "(e.g. \"Introduction|outline\" for a heading's section, \"Summary|region\" "
    "for a named section). If the document is too large to read in full, "
    "filter=\"text\" with no target returns no body text and instead supplies "
    "link_targets and an instruction: show the listed sections to the user, "
    "wait for their pick, then call this tool again with the chosen target. If "
    "link_targets is empty the document has no navigable structure - the "
    "instruction will tell you to ask the user to select text in the document. "
    "It can also return content controls (filter=\"contentcontrol\"), charts, "
    "document properties, and tracked changes. To list headings, bookmarks, "
    "tables, frames or images for navigation, use extract_link_targets "
    "instead.";

inline constexpr const char* EXTRACT_CALC =
    " For a spreadsheet, filter=\"text\" returns the active sheet as markdown, "
    "optionally limited to a cell range, for summarizing or answering questions "
    "about the data.";

inline constexpr const char* EXTRACT_IMPRESS =
    " For a presentation, filter=\"text\" returns the text of every slide as "
    "markdown, one section per slide, for summarizing or answering questions "
    "about the content. With filter=\"slides\" it returns each slide's name, "
    "layout, and master, the master slides with their theme, and each object "
    "with its placeholder kind (Title, Outline, Graphic, ...), an Empty flag "
    "for placeholders that still await content, and its text.";

/// Shared intro for the transform parameter, valid for every document type.
inline constexpr const char* TRANSFORM_INTRO =
    R"(JSON transformation commands. The top-level object can contain "Transforms" and/or "UnoCommand" objects in any order.)";

/// Impress/ODP-specific transform documentation, the part before the
/// generated SlideCommands vocabulary (see AIUtil::getSlideCommandDocs()).
inline constexpr const char* TRANSFORM_IMPRESS_INTRO =
    R"(

--- Impress/ODP Presentations ---

For presentations, use {"Transforms": {"SlideCommands": [...]}} where SlideCommands is an array of operations applied in order. There is always a "current slide" (default: index 0) that most commands act on. All slides must go in a single SlideCommands array - use InsertMasterSlide to add new slides within the same array. Never send multiple JSON objects.

Do NOT prefix text lines with "- " (any bullet marker is added automatically). Do NOT put sub-headings or blank lines inside content placeholders - only the items to be listed. Choose the layout that fits the content (see Available layouts below). The instructions for this request say how much formatting to apply yourself; when a design template is in use, its master slides handle the look, so leave styling to them.
)";

/// Impress/ODP-specific transform documentation, the part after the
/// generated SlideCommands vocabulary.
inline constexpr const char* TRANSFORM_IMPRESS_DETAILS =
    R"(
UNO commands for text formatting (use inside EditTextObject):
Toggle: .uno:Bold, .uno:Italic, .uno:Underline, .uno:Strikeout, .uno:Shadowed, .uno:OutlineFont, .uno:SuperScript, .uno:SubScript
Lists: .uno:DefaultBullet, .uno:DefaultNumbering (affect whole paragraphs)
Alignment: .uno:LeftPara, .uno:CenterPara, .uno:RightPara, .uno:JustifyPara
Spacing: .uno:SpacePara1 (single), .uno:SpacePara15 (1.5x), .uno:SpacePara2 (double)
Font: .uno:CharFontName {"CharFontName.FamilyName":{"type":"string","value":"Arial"}}
Size: .uno:FontHeight {"FontHeight.Height":{"type":"float","value":24}}
Color: .uno:Color {"Color.Color":{"type":"long","value":RGB_INT}} (RGB_INT = R*65536 + G*256 + B, e.g. 255 = blue, 16711680 = red)
Background: .uno:CharBackColor {"CharBackColor.Color":{"type":"long","value":RGB_INT}}

UnoCommand levels - there are three places to use UnoCommand, each for a different scope:

1. Inside EditTextObject.N [...] - formats the currently selected text within that object. Use for: bold, italic, color, font size, bullet lists, alignment.
   Format: {"UnoCommand": ".uno:Bold"} or with args: {"UnoCommand": ".uno:Color {\"Color.Color\":{\"type\":\"long\",\"value\":255}}"}

2. Inside SlideCommands but outside EditTextObject - applies a command in the slide context without entering text edit mode. Use for: operations on the slide or selected objects that do not require text selection.
   Format: {"UnoCommand": ".uno:CommandName"}

3. Top-level (outside SlideCommands, works for all doc types) - dispatches a command at the document level. Use for: document-wide settings like change tracking, spell checking, or other global toggles.
   Format: {"UnoCommand": {"name": ".uno:CommandName", "arguments": {"ArgName": {"type": "string|long|boolean", "value": "..."}}}}
   Example - enable change tracking:
   {"UnoCommand": {"name": ".uno:TrackChanges", "arguments": {"TrackChanges": {"type": "boolean", "value": "true"}}}}

Prefer SlideCommands operations (SetText, ChangeLayout, EditTextObject) over raw UnoCommand when a dedicated command exists. Use UnoCommand only for formatting and features not covered by SlideCommands.

Example - create a 5-slide presentation from a blank ODP:
{"Transforms":{"SlideCommands":[{"ChangeLayoutByName":"AUTOLAYOUT_TITLE"},{"SetText.0":"Quarterly Report"},{"SetText.1":"Q1 2026"},{"RenameSlide":"Title"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"},{"UnoCommand":".uno:CenterPara"}]},{"InsertMasterSlide":0},{"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},{"SetText.0":"Revenue"},{"SetText.1":"Revenue grew 15% year over year\nNew markets contributed 30% of growth\nCustomer retention at 95%"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},{"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},{"InsertMasterSlide":0},{"ChangeLayoutByName":"AUTOLAYOUT_TITLE_2CONTENT"},{"SetText.0":"Strengths & Risks"},{"SetText.1":"Strong brand recognition\nGrowing user base\nHigh retention rate"},{"SetText.2":"Supply chain delays\nRegulatory changes\nCompetitor pricing"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},{"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},{"EditTextObject.2":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},{"InsertMasterSlide":0},{"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},{"SetText.0":"Roadmap"},{"SetText.1":"Phase 1: Research\nPhase 2: Development\nPhase 3: Launch"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},{"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultNumbering"},{"SelectParagraph":0},{"UnoCommand":".uno:Bold"}]},{"InsertMasterSlide":0},{"ChangeLayoutByName":"AUTOLAYOUT_TITLE_ONLY"},{"SetText.0":"Thank You"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"},{"UnoCommand":".uno:CenterPara"}]},{"JumpToSlide":1},{"EditTextObject.1":[{"SelectParagraph":0},{"InsertText":"Revenue grew 15% YoY"},{"UnoCommand":".uno:Bold"},{"UnoCommand":".uno:Italic"}]}]}})";

/// The write_slides description in fragments so the caller composes it at run
/// time. The "Limits:" sentence is not a fragment here: it is built from the
/// live budgets by DeckSpec::limitsSentence and inserted between DECK_SLIDE_SHAPE
/// and WRITE_SLIDES_SUMMARY_NOTE, so the numbers the model is told match the
/// numbers the validator enforces. With default budgets that composition is
/// byte-identical to the previous single description. DECK_SLIDE_SHAPE is also
/// reused by the per-slide write_slide expansion tool.
inline constexpr const char* WRITE_SLIDES_INTRO =
    R"(Create presentation slides from a declarative deck description. The server lays out and styles the slides for you, so describe what each slide is about, not how to format it.

Pass a "deck" object of the form {"slides": [ ... ]}. )";

/// The shape of one slide in a deck description, without the budget limits (see
/// DeckSpec::limitsSentence). Shared by the write_slides tool and the per-slide
/// write_slide expansion tool.
inline constexpr const char* DECK_SLIDE_SHAPE =
    R"(Each slide is an object with:
- "part": one of "opening", "divider", "body", "closing" - the slide's role in the deck. Use opening for the first slide, divider for a section break, closing for the last slide, and body for the rest.
- "intent": one of "title", "agenda", "bullets", "two-column", "comparison", "quote", "big-number", "image", "section", "closing" - what the slide is for. The intent chooses the layout.
- "title": the slide title (required).
- "blocks": the content as an array of blocks. A bullets block is {"kind": "bullets", "items": ["...", "..."]}. A text block is {"kind": "text", "text": "..."}.
- "image": only for the "image" intent, {"brief": "a description to generate an image from", "alt": "short alt text"}.
- "notes": optional speaker notes for the slide. Put depth in the notes and keep the slide text scannable.

Inside bullet items and text blocks you may emphasise words with **bold** and *italic*. No other markdown is supported.

Which blocks each intent expects:
- title, closing: no blocks, or a single text block used as a subtitle.
- agenda, bullets: exactly one bullets block.
- two-column, comparison: exactly two bullets blocks (left and right).
- quote, big-number: exactly one text block.
- image: no blocks; supply an "image" instead.
- section: a title only, no blocks.)";

/// The trailing "summary" note appended after the budget limits in the
/// write_slides description.
inline constexpr const char* WRITE_SLIDES_SUMMARY_NOTE =
    R"(

Also pass a "summary": a short markdown preview of the slides being created, shown to the user for approval.)";

/// The propose_outline description without its trailing slide-count sentence.
/// The caller appends "Give at most N slides. ..." with N from the live budgets,
/// so the number matches the validator; with default budgets the composition is
/// byte-identical to the previous single description.
inline constexpr const char* PROPOSE_OUTLINE_HEAD =
    R"(Propose a slide-by-slide outline for a new deck for the user to review and edit before the slides are built. Call this instead of write_slides for a deck of more than about three slides. After the user approves the outline, the server builds the slides itself, so do not also call write_slides for a deck you have outlined.

Pass an "outline" object of the form {"title": "deck title", "slides": [ ... ]}. Each slide entry is an object with:
- "part": one of "opening", "divider", "body", "closing" - the slide's role in the deck.
- "intent": one of "title", "agenda", "bullets", "two-column", "comparison", "quote", "big-number", "image", "section", "closing" - what the slide is for. The intent chooses the layout.
- "title": a short slide title (required).
- "gist": one or two sentences saying specifically what this slide should cover. This drives the content the server writes for the slide, so make it concrete.

)";

/// The propose_outline slide-count sentence, closing the description. The "N"
/// is substituted from the live budgets at composition time.
inline constexpr const char* PROPOSE_OUTLINE_TAIL_OPEN_CLOSE =
    " slides. Open with an opening title slide and end with a closing slide.";

/// Writer content-control transform documentation.
inline constexpr const char* TRANSFORM_WRITER =
    R"(

--- Writer Content Controls ---

For Writer, address content control items by selector:
{"Transforms": {"ContentControls.ByIndex.0": {"content": "new value"}}}
Selectors: ContentControls.ByIndex.N, ContentControls.ByTag.tagname, ContentControls.ByAlias.aliasname. Use extract_document_structure with filter="contentcontrol" first to discover available controls.

Top-level UnoCommand (document-wide) dispatches a command at the document level. Use for global toggles like change tracking.
Format: {"UnoCommand": {"name": ".uno:CommandName", "arguments": {"ArgName": {"type": "string|long|boolean", "value": "..."}}}}
Example: {"UnoCommand": {"name": ".uno:TrackChanges", "arguments": {"TrackChanges": {"type": "boolean", "value": "true"}}}})";

} // namespace DocumentToolDescriptions

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
