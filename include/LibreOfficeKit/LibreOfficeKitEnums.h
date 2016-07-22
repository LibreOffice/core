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

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
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
    LOK_PARTMODE_SLIDES,
    LOK_PARTMODE_NOTES
}
LibreOfficeKitPartMode;

typedef enum
{
    LOK_TILEMODE_RGBA,
    LOK_TILEMODE_BGRA
}
LibreOfficeKitTileMode;

/** Optional features of LibreOfficeKit, in particular callbacks that block
 *  LibreOfficeKit until the corresponding reply is received, which would
 *  deadlock if the client does not support the feature.
 *
 *  @see lok::Office::setOptionalFeatures().
 */
typedef enum
{
    /**
     * Handle LOK_CALLBACK_DOCUMENT_PASSWORD by prompting the user
     * for a password.
     *
     * @see lok::Office::setDocumentPassword().
     */
    LOK_FEATURE_DOCUMENT_PASSWORD = (1ULL << 0),

    /**
     * Handle LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY by prompting the user
     * for a password.
     *
     * @see lok::Office::setDocumentPassword().
     */
    LOK_FEATURE_DOCUMENT_PASSWORD_TO_MODIFY = (1ULL << 1),
}
LibreOfficeKitOptionalFeatures;

// This enumerates the types of callbacks emitted to a LibreOfficeKit
// object's callback function or to a LibreOfficeKitDocument object's
// callback function. No callback type will be emitted to both. It is a
// bit unfortunate that the same enum contains both kinds of
// callbacks.

// TODO: We should really add some indication at the documentation for
// each enum value telling which type of callback it is.

typedef enum
{
    /**
     * Any tiles which are over the rectangle described in the payload are no
     * longer valid.
     *
     * Rectangle format: "x, y, width, height", where all numbers are document
     * coordinates, in twips. When all tiles are supposed to be dropped, the
     * format is the "EMPTY" string.
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
     * The position and size of the cursor rectangle at the text
     * selection start. It is used to draw the selection handles.
     *
     * This callback must be called prior to LOK_CALLBACK_TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_TEXT_SELECTION_START,
    /**
     * The position and size of the cursor rectangle at the text
     * selection end. It is used to draw the selection handles.
     *
     * This callback must be called prior to LOK_CALLBACK_TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_TEXT_SELECTION_END,
    /**
     * The blinking text cursor is now visible or not.
     *
     * Clients should assume that this is true initially and are expected to
     * hide the blinking cursor at the rectangle described by
     * LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR once it becomes false. Payload is
     * either the "true" or the "false" string.
     */
    LOK_CALLBACK_CURSOR_VISIBLE,
    /**
     * The size and/or the position of the graphic selection changed.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_GRAPHIC_SELECTION,

    /**
     * User clicked on an hyperlink that should be handled by other
     * applications accordingly.
     */
    LOK_CALLBACK_HYPERLINK_CLICKED,

    /**
     * Emit state update to the client.
     * For example, when cursor is on bold text, this callback is triggered
     * with payload: ".uno:Bold=true"
     */
    LOK_CALLBACK_STATE_CHANGED,

    /**
     * Start a "status indicator" (here restricted to a progress bar type
     * indicator). The payload is the descriptive text (or empty). Even if
     * there is no documentation that would promise so, we assume that de facto
     * for a document being viewed or edited, there will be at most one status
     * indicator, and its descriptive text will not change.
     *
     * Note that for the case of the progress indication during loading of a
     * document, the status indicator callbacks will arrive to the callback
     * registered for the LibreOfficeKit (singleton) object, not a
     * LibreOfficeKitDocument one, because we are in the very progress of
     * loading a document and then constructing a LibreOfficeKitDocument
     * object.
     */
    LOK_CALLBACK_STATUS_INDICATOR_START,

    /**
     * Sets the numeric value of the status indicator.
     * The payload should be a percentage, an integer between 0 and 100.
     */
    LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE,

    /**
     * Ends the status indicator.
     *
     * Not necessarily ever emitted.
     */
    LOK_CALLBACK_STATUS_INDICATOR_FINISH,

    /**
     * No match was found for the search input
     */
    LOK_CALLBACK_SEARCH_NOT_FOUND,

    /**
     * Size of the document changed.
     *
     * Payload format is "width, height", i.e. clients get the new size without
     * having to do an explicit lok::Document::getDocumentSize() call.
     */
    LOK_CALLBACK_DOCUMENT_SIZE_CHANGED,

    /**
     * The current part number is changed.
     *
     * Payload is a single 0-based integer.
     */
    LOK_CALLBACK_SET_PART,

    /**
     * Selection rectangles of the search result when find all is performed.
     *
     * Payload format example, in case of two matches:
     *
     * {
     *     "searchString": "...",
     *     "highlightAll": true|false, // this is a result of 'search all'
     *     "searchResultSelection": [
     *         {
     *             "part": "...",
     *             "rectangles": "..."
     *         },
     *         {
     *             "part": "...",
     *             "rectangles": "..."
     *         }
     *     ]
     * }
     *
     * - searchString is the search query
     * - searchResultSelection is an array of part-number and rectangle list
     *   pairs, in LOK_CALLBACK_SET_PART / LOK_CALLBACK_TEXT_SELECTION format.
     */
    LOK_CALLBACK_SEARCH_RESULT_SELECTION,

    /**
     * Result of the UNO command execution when bNotifyWhenFinished was set
     * to 'true' during the postUnoCommand() call.
     *
     * The result returns a success / failure state, and potentially
     * additional data:
     *
     * {
     *     "commandName": "...",    // the command for which this is the result
     *     "success": true/false,   // when the result is "don't know", this is missing
     *     // TODO "result": "..."  // UNO Any converted to JSON (not implemented yet)
     * }
     */
    LOK_CALLBACK_UNO_COMMAND_RESULT,

    /**
     * The size and/or the position of the cell cursor changed.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_CELL_CURSOR,

    /**
     * The current mouse pointer style.
     *
     * Payload is a css mouse pointer style.
     */
    LOK_CALLBACK_MOUSE_POINTER,

    /**
     * The text content of the formula bar in Calc.
     */
    LOK_CALLBACK_CELL_FORMULA,

    /**
     * Loading a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * lok::Office::setDocumentPassword().  The document cannot be loaded
     * without the password.
     */
    LOK_CALLBACK_DOCUMENT_PASSWORD,

    /**
     * Editing a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * lok::Office::setDocumentPassword().
     */
    LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY,

    /**
     * An error happened.
     *
     * The payload returns information further identifying the error, like:
     *
     * {
     *     "classification": "error" | "warning" | "info"
     *     "kind": "network" etc.
     *     "code": 403 | 404 | ...
     *     "message": freeform description
     * }
     */
    LOK_CALLBACK_ERROR,

    /**
     * Context menu structure
     *
     * Returns the structure of context menu.  Contains all the separators &
     * submenus, example of the returned structure:
     *
     * {
     *     "menu": [
     *         { "text": "label text1", "type": "command", "command": ".uno:Something1", "enabled": "true" },
     *         { "text": "label text2", "type": "command", "command": ".uno:Something2", "enabled": "false" },
     *         { "type": "separator" },
     *         { "text": "label text2", "type": "menu", "menu": [ { ... }, { ... }, ... ] },
     *         ...
     *     ]
     * }
     *
     * The 'command' can additionally have a checkable status, like:
     *
     *     {"text": "label text3", "type": "command", "command": ".uno:Something3", "checktype": "checkmark|radio|auto", "checked": "true|false"}
     */
    LOK_CALLBACK_CONTEXT_MENU,

    /**
     * The size and/or the position of the view cursor changed. A view cursor
     * is a cursor of an other view, the current view can't change it.
     *
     * The payload format:
     *
     * {
     *     "viewId": "..."
     *     "rectangle": "..."
     * }
     *
     * - viewId is a value returned earlier by lok::Document::createView()
     * - rectangle uses the format of LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR
     */
    LOK_CALLBACK_INVALIDATE_VIEW_CURSOR,

    /**
     * The text selection in one of the other views has changed.
     *
     * The payload format:
     *
     * {
     *     "viewId": "..."
     *     "selection": "..."
     * }
     *
     * - viewId is a value returned earlier by lok::Document::createView()
     * - selection uses the format of LOK_CALLBACK_TEXT_SELECTION.
     */
    LOK_CALLBACK_TEXT_VIEW_SELECTION,

    /**
     * The cell cursor in one of the other views has changed.
     *
     * The payload format:
     *
     * {
     *     "viewId": "..."
     *     "rectangle": "..."
     * }
     *
     * - viewId is a value returned earlier by lok::Document::createView()
     * - rectangle uses the format of LOK_CALLBACK_CELL_CURSOR.
     */
    LOK_CALLBACK_CELL_VIEW_CURSOR,

    /**
     * The size and/or the position of a graphic selection in one of the other
     * views has changed.
     *
     * The payload format:
     *
     * {
     *     "viewId": "..."
     *     "selection": "..."
     * }
     *
     * - viewId is a value returned earlier by lok::Document::createView()
     * - selection uses the format of LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_GRAPHIC_VIEW_SELECTION,

    /**
     * The blinking text cursor in one of the other views is now visible or
     * not.
     *
     * The payload format:
     *
     * {
     *     "viewId": "..."
     *     "visible": "..."
     * }
     *
     * - viewId is a value returned earlier by lok::Document::createView()
     * - visible uses the format of LOK_CALLBACK_CURSOR_VISIBLE.
     */
    LOK_CALLBACK_VIEW_CURSOR_VISIBLE,

    /**
     * The size and/or the position of a lock rectangle in one of the other
     * views has changed.
     *
     * The payload format:
     *
     * {
     *     "viewId": "..."
     *     "rectangle": "..."
     * }
     *
     * - viewId is a value returned earlier by lok::Document::createView()
     * - rectangle uses the format of LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_VIEW_LOCK,

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

typedef enum
{
    /**
     * A move or a resize action starts. It is assumed that there is a valid
     * graphic selection (see LOK_CALLBACK_GRAPHIC_SELECTION) and the supplied
     * coordinates are the ones the user tapped on.
     *
     * The type of the action is move by default, unless the coordinates are
     * the position of a handle (see below), in which case it's a resize.
     *
     * There are 8 handles for a graphic selection:
     * - top-left, top-center, top-right
     * - middle-left, middle-right
     * - bottom-left, bottom-center, bottom-right
     */
    LOK_SETGRAPHICSELECTION_START,
    /**
     * A move or resize action stops. It is assumed that this is always used
     * only after a LOK_SETTEXTSELECTION_START. The supplied coordinates are
     * the ones where the user released the screen.
     */
    LOK_SETGRAPHICSELECTION_END
}
LibreOfficeKitSetGraphicSelectionType;

#endif // defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITENUMS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
