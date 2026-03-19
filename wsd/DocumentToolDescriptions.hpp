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

/// Description for the extract_document_structure tool.
inline constexpr const char* EXTRACT_DOC_STRUCTURE_DESCRIPTION =
    "Extract the structural outline of a document as JSON. "
    "For Writer: headings, sections, tables, frames, images, bookmarks, content controls. "
    "For Calc: sheet names. "
    "For Impress: slide names, object names per slide. "
    "Useful for understanding document layout before applying transformations.";

/// Description for the transform parameter of transform_document_structure.
inline constexpr const char* TRANSFORM_PARAM_DESCRIPTION =
    R"(JSON transformation commands. The top-level object can contain "Transforms" and/or "UnoCommand" objects in any order.

--- Impress/ODP Presentations ---

For presentations, use {"Transforms": {"SlideCommands": [...]}} where SlideCommands is an array of operations applied in order. There is always a "current slide" (default: index 0) that most commands act on. All slides must go in a single SlideCommands array - use InsertMasterSlide to add new slides within the same array. Never send multiple JSON objects.

REQUIRED for every slide: use EditTextObject to bold the title (.uno:Bold), and apply .uno:DefaultBullet to content placeholders that list items. Do NOT prefix text lines with "- " when using DefaultBullet (the bullet is automatic). Do NOT put sub-headings or blank lines inside content placeholders - only the items to be bulleted. Choose the layout that fits the content (see Available layouts below).

Navigation:
- {"JumpToSlide": N} - jump to 0-based slide index; use "last" for last slide
- {"JumpToSlideByName": "name"} - jump to named slide

Slide management (inserts after current slide and jumps to new slide):
- {"InsertMasterSlide": N} - insert slide based on master slide at index N
- {"InsertMasterSlideByName": "name"} - insert slide by master slide name
- {"DeleteSlide": N} - delete slide at index; use "" for current slide
- {"DuplicateSlide": N} - duplicate slide at index; use "" for current
- {"MoveSlide": N} - move current slide to position N
- {"MoveSlide.X": N} - move slide at index X to position N
- {"RenameSlide": "name"} - rename current slide (must be unique)

Layout (applied to current slide):
- {"ChangeLayoutByName": "name"} - set layout by name
- {"ChangeLayout": N} - set layout by numeric ID
Available layouts (use ChangeLayoutByName with these names):
- AUTOLAYOUT_TITLE (id=0) - title + subtitle. Use for opening or closing slides.
- AUTOLAYOUT_TITLE_CONTENT (id=1) - title + one content area below. The default layout for bullet points, text, or a single diagram.
- AUTOLAYOUT_TITLE_2CONTENT (id=3) - title + two content areas side by side. Use for comparisons, pros/cons, or before/after.
- AUTOLAYOUT_TITLE_CONTENT_2CONTENT (id=12) - title + one large content left + two smaller content areas stacked right.
- AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT (id=14) - title + two content areas stacked vertically. Use when two blocks need equal width.
- AUTOLAYOUT_TITLE_2CONTENT_CONTENT (id=15) - title + two smaller content areas stacked left + one large content right.
- AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT (id=16) - title + two content areas side by side over one full-width content area.
- AUTOLAYOUT_TITLE_4CONTENT (id=18) - title + four content areas in a 2x2 grid. Use for dashboards or quadrant layouts.
- AUTOLAYOUT_TITLE_ONLY (id=19) - title only, no content placeholders. Use for section dividers or title cards.
- AUTOLAYOUT_NONE (id=20) - blank slide, no placeholders at all. Use for fully custom or image-only slides.
- AUTOLAYOUT_ONLY_TEXT (id=32) - one centered text area, no title. Use for quotes, key statements, or transition text.
- AUTOLAYOUT_TITLE_6CONTENT (id=34) - title + six content areas in a 3x2 grid.
- AUTOLAYOUT_VTITLE_VCONTENT (id=28) - vertical title + vertical content. For vertical/CJK text.
- AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT (id=27) - vertical title + two vertical content areas.
- AUTOLAYOUT_TITLE_VCONTENT (id=29) - horizontal title + vertical content below.
- AUTOLAYOUT_TITLE_2VTEXT (id=30) - title + two vertical text columns.

Text content:
- {"SetText.N": "text"} - set text of placeholder N on current slide (0=title, 1=first content, 2=second content, etc.). Use \n for paragraph breaks.

Object selection:
- {"MarkObject": N} - select object at index on current slide
- {"UnMarkObject": N} - deselect object at index

Rich text editing:
- {"EditTextObject.N": [...]} - edit text object N with sub-commands:
  - {"SelectText": []} - select all text; [para] selects paragraph; [para,startChar,endPara,endChar] selects range; [para,char] positions cursor
  - {"SelectParagraph": N} - select paragraph N
  - {"InsertText": "text"} - insert/replace text at selection
  - {"UnoCommand": "cmd"} - apply UNO command to selection

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

--- Writer/Calc Content Controls ---

For Writer/Calc, address content control items by selector:
{"Transforms": {"ContentControls.ByIndex.0": {"content": "new value"}}}
Selectors: ContentControls.ByIndex.N, ContentControls.ByTag.tagname, ContentControls.ByAlias.aliasname. Use extract_document_structure with filter="contentcontrol" first to discover available controls.

Example - create a 5-slide presentation from a blank ODP:
{"Transforms":{"SlideCommands":[{"ChangeLayoutByName":"AUTOLAYOUT_TITLE"},{"SetText.0":"Quarterly Report"},{"SetText.1":"Q1 2026"},{"RenameSlide":"Title"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"},{"UnoCommand":".uno:CenterPara"}]},{"InsertMasterSlide":0},{"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},{"SetText.0":"Revenue"},{"SetText.1":"Revenue grew 15% year over year\nNew markets contributed 30% of growth\nCustomer retention at 95%"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},{"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},{"InsertMasterSlide":0},{"ChangeLayoutByName":"AUTOLAYOUT_TITLE_2CONTENT"},{"SetText.0":"Strengths & Risks"},{"SetText.1":"Strong brand recognition\nGrowing user base\nHigh retention rate"},{"SetText.2":"Supply chain delays\nRegulatory changes\nCompetitor pricing"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},{"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},{"EditTextObject.2":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},{"InsertMasterSlide":0},{"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},{"SetText.0":"Roadmap"},{"SetText.1":"Phase 1: Research\nPhase 2: Development\nPhase 3: Launch"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},{"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultNumbering"},{"SelectParagraph":0},{"UnoCommand":".uno:Bold"}]},{"InsertMasterSlide":0},{"ChangeLayoutByName":"AUTOLAYOUT_TITLE_ONLY"},{"SetText.0":"Thank You"},{"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"},{"UnoCommand":".uno:CenterPara"}]},{"JumpToSlide":1},{"EditTextObject.1":[{"SelectParagraph":0},{"InsertText":"Revenue grew 15% YoY"},{"UnoCommand":".uno:Bold"},{"UnoCommand":".uno:Italic"}]}]}})";

} // namespace DocumentToolDescriptions

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
