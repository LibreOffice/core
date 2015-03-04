/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITENUMS_H
#define INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITENUMS_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef LOK_USE_UNSTABLE_API
typedef enum
{
  LOK_DOCTYPE_TEXT,
  LOK_DOCTYPE_SPREADSHEET,
  LOK_DOCTYPE_PRESENTATION,
  LOK_DOCTYPE_DRAWING,
  LOK_DOCTYPE_OTHER
}
LibreOfficeKitDocumentType;

typedef enum
{
    LOK_PARTMODE_DEFAULT,
    LOK_PARTMODE_SLIDE,
    LOK_PARTMODE_NOTES,
    LOK_PARTMODE_SLIDENOTES,
    LOK_PARTMODE_EMBEDDEDOBJ
}
LibreOfficeKitPartMode;

typedef enum
{
    /**
     * Any tiles which are over the rectangle described in the payload are no
     * longer valid.
     *
     * Rectangle format: "width, height, x, y", where all numbers are document
     * coordinates, in twips.
     */
    LOK_CALLBACK_INVALIDATE_TILES,
    /**
     * The size and/or the position of the visible cursor changed.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR,
    /**
     * The list of rectangles representing the current text selection changed.
     *
     * List format is "rectangle1[; rectangle2[; ...]]" (without quotes and
     * brackets), where rectangleN has the same format as
     * LOK_CALLBACK_INVALIDATE_TILES. When there is no selection, an empty
     * string is provided.
     */
    LOK_CALLBACK_TEXT_SELECTION,
    /**
     * The size and/or the position of the cursor rectangle at the text
     * selection start changed.
     *
     * If this callback is emitted, it's always followed by a
     * LOK_CALLBACK_TEXT_SELECTION one. Rectangle format is the same as
     * LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_TEXT_SELECTION_START,
    /**
     * The size and/or the position of the cursor rectangle at the text
     * selection end changed.
     *
     * If this callback is emitted, it's always followed by a
     * LOK_CALLBACK_TEXT_SELECTION one. Rectangle format is the same as
     * LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_TEXT_SELECTION_END,
    /**
     * The blinking text cursor is now visible or not.
     *
     * Clients should assume that this is false initially and are expected to
     * show a blinking cursor at the rectangle described by
     * LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR once it becomes true. Payload is
     * either the "true" or the "false" string.
     */
    LOK_CALLBACK_CURSOR_VISIBLE
}
LibreOfficeKitCallbackType;

typedef enum
{
    /// A key on the keyboard is pressed.
    LOK_KEYEVENT_KEYINPUT,
    /// A key on the keyboard is released.
    LOK_KEYEVENT_KEYUP
}
LibreOfficeKitKeyEventType;

typedef enum
{
    /// A pressed gesture has started.
    LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
    /// A pressed gesture has finished.
    LOK_MOUSEEVENT_MOUSEBUTTONUP,
    /// A change has happened during a press gesture.
    LOK_MOUSEEVENT_MOUSEMOVE
}
LibreOfficeKitMouseEventType;

typedef enum
{
    /// The start of selection is to be adjusted.
    LOK_SETTEXTSELECTION_START,
    /// The end of selection is to be adjusted.
    LOK_SETTEXTSELECTION_END,
    /// Both the start and the end of selection is to be adjusted.
    LOK_SETTEXTSELECTION_RESET
}
LibreOfficeKitSetTextSelectionType;

#endif // LOK_USE_UNSTABLE_API

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITENUMS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
