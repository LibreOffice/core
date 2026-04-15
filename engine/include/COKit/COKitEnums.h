/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COKIT_COKIT_ENUMS_H
#define INCLUDED_COKIT_COKIT_ENUMS_H

#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined KIT_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
typedef enum
{
  KIT_DOCTYPE_TEXT,
  KIT_DOCTYPE_SPREADSHEET,
  KIT_DOCTYPE_PRESENTATION,
  KIT_DOCTYPE_DRAWING,
  KIT_DOCTYPE_OTHER
}
COKitDocumentType;

typedef enum
{
    KIT_PARTMODE_SLIDES,
    KIT_PARTMODE_NOTES
}
COKitPartMode;

typedef enum
{
    KIT_TILEMODE_RGBA,
    KIT_TILEMODE_BGRA
}
COKitTileMode;

typedef enum
{
    KIT_WINDOW_CLOSE,
    KIT_WINDOW_PASTE
}
COKitWindowAction;

typedef enum
{
    KIT_SELTYPE_NONE,
    KIT_SELTYPE_TEXT,
    KIT_SELTYPE_LARGE_TEXT, // unused (same as KIT_SELTYPE_COMPLEX)
    KIT_SELTYPE_COMPLEX
}
COKitSelectionType;

/** Optional features of COKit, in particular callbacks that block
 *  COKit until the corresponding reply is received, which would
 *  deadlock if the client does not support the feature.
 *
 *  @see kit::Office::setOptionalFeatures().
 */
typedef enum
{
    /**
     * Handle KIT_CALLBACK_DOCUMENT_PASSWORD by prompting the user
     * for a password.
     *
     * @see kit::Office::setDocumentPassword().
     */
    KIT_FEATURE_DOCUMENT_PASSWORD = (1ULL << 0),

    /**
     * Handle KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY by prompting the user
     * for a password.
     *
     * @see kit::Office::setDocumentPassword().
     */
    KIT_FEATURE_DOCUMENT_PASSWORD_TO_MODIFY = (1ULL << 1),

    /**
     * Request to have the part number as an 5th value in the
     * KIT_CALLBACK_INVALIDATE_TILES payload.
     */
    KIT_FEATURE_PART_IN_INVALIDATION_CALLBACK = (1ULL << 2),

    /**
     * Turn off tile rendering for annotations
     */
    KIT_FEATURE_NO_TILED_ANNOTATIONS = (1ULL << 3),

    /**
     * Enable range based header data
     */
    KIT_FEATURE_RANGE_HEADERS = (1ULL << 4),

    /**
     * Request to have the active view's Id as the 1st value in the
     * KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR payload.
     */
    KIT_FEATURE_VIEWID_IN_VISCURSOR_INVALIDATION_CALLBACK = (1ULL << 5)
}
COKitOptionalFeatures;

// This enumerates the types of callbacks emitted to a COKit
// object's callback function or to a COKitDocument object's
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
     *
     * @see KIT_FEATURE_PART_IN_INVALIDATION_CALLBACK.
     */
    KIT_CALLBACK_INVALIDATE_TILES = 0,
    /**
     * The size and/or the position of the visible cursor changed.
     *
     * Old format is the same as KIT_CALLBACK_INVALIDATE_TILES.
     * New format is a JSON with 3 elements the 'viewId' element represented by
     * an integer value, a 'rectangle' element in the format "x, y, width, height",
     * and a 'misspelledWord' element represented by an integer value: '1' when
     * a misspelled word is at the cursor position, '0' when the word is
     * not misspelled.
     */
    KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR = 1,
    /**
     * The list of rectangles representing the current text selection changed.
     *
     * List format is "rectangle1[; rectangle2[; ...]]" (without quotes and
     * brackets), where rectangleN has the same format as
     * KIT_CALLBACK_INVALIDATE_TILES. When there is no selection, an empty
     * string is provided.
     */
    KIT_CALLBACK_TEXT_SELECTION = 2,
    /**
     * The position and size of the cursor rectangle at the text
     * selection start. It is used to draw the selection handles.
     *
     * This callback must be called prior to KIT_CALLBACK_TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as KIT_CALLBACK_INVALIDATE_TILES.
     */
    KIT_CALLBACK_TEXT_SELECTION_START = 3,
    /**
     * The position and size of the cursor rectangle at the text
     * selection end. It is used to draw the selection handles.
     *
     * This callback must be called prior to KIT_CALLBACK_TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as KIT_CALLBACK_INVALIDATE_TILES.
     */
    KIT_CALLBACK_TEXT_SELECTION_END = 4,
    /**
     * The blinking text cursor is now visible or not.
     *
     * Clients should assume that this is true initially and are expected to
     * hide the blinking cursor at the rectangle described by
     * KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR once it becomes false. Payload is
     * either the "true" or the "false" string.
     */
    KIT_CALLBACK_CURSOR_VISIBLE = 5,
    /**
     * The size and/or the position of the graphic selection changed,
     * the rotation angle of the embedded graphic object, and a property list
     * which can be used for informing the client about several properties.
     *
     * Format is "x, y, width, height, angle, { list of properties }",
     * where angle is in 100th of degree, and the property list is optional.
     *
     * The "{ list of properties }" part is in JSON format.
     * Follow some examples of the property list part:
     *
     * 1) when the selected object is an image inserted in Writer:
     *
     *      { "isWriterGraphic": true }
     *
     * 2) when the selected object is a chart legend:
     *
     *      { "isDraggable": true, "isResizable": true, "isRotatable": false }
     *
     * 3) when the selected object is a pie segment in a chart:
     *
     *      {
     *          "isDraggable": true,
     *          "isResizable": false,
     *          "isRotatable": false,
     *          "dragInfo": {
     *              "dragMethod": "PieSegmentDragging",
     *              "initialOffset": 50,
     *              "dragDirection": [x, y],
     *              "svg": "<svg ..."
     *          }
     *      }
     *
     *      where the "svg" property is a string containing an svg document
     *      which is a representation of the pie segment.
     */
    KIT_CALLBACK_GRAPHIC_SELECTION = 6,

    /**
     * User clicked on a hyperlink that should be handled by other
     * applications accordingly.
     */
    KIT_CALLBACK_HYPERLINK_CLICKED = 7,

    /**
     * Emit state update to the client.
     * For example, when cursor is on bold text, this callback is triggered
     * with payload: ".uno:Bold=true"
     */
    KIT_CALLBACK_STATE_CHANGED = 8,

    /**
     * Start a "status indicator" (here restricted to a progress bar type
     * indicator). The payload is the descriptive text (or empty). Even if
     * there is no documentation that would promise so, we assume that de facto
     * for a document being viewed or edited, there will be at most one status
     * indicator, and its descriptive text will not change.
     *
     * Note that for the case of the progress indication during loading of a
     * document, the status indicator callbacks will arrive to the callback
     * registered for the COKit (singleton) object, not a
     * COKitDocument one, because we are in the very progress of
     * loading a document and then constructing a COKitDocument
     * object.
     */
    KIT_CALLBACK_STATUS_INDICATOR_START = 9,

    /**
     * Sets the numeric value of the status indicator.
     * The payload should be a percentage, an integer between 0 and 100.
     */
    KIT_CALLBACK_STATUS_INDICATOR_SET_VALUE = 10,

    /**
     * Ends the status indicator.
     *
     * Not necessarily ever emitted.
     */
    KIT_CALLBACK_STATUS_INDICATOR_FINISH = 11,

    /**
     * No match was found for the search input
     */
    KIT_CALLBACK_SEARCH_NOT_FOUND = 12,

    /**
     * Size of the document changed.
     *
     * Payload format is "width, height", i.e. clients get the new size without
     * having to do an explicit kit::Document::getDocumentSize() call.
     *
     * A size change is always preceded by a series of
     * KIT_CALLBACK_INVALIDATE_TILES events invalidating any areas
     * need re-rendering to adapt.
     */
    KIT_CALLBACK_DOCUMENT_SIZE_CHANGED = 13,

    /**
     * The current part number is changed.
     *
     * Payload is a single 0-based integer.
     */
    KIT_CALLBACK_SET_PART = 14,

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
     *   pairs, in KIT_CALLBACK_SET_PART / KIT_CALLBACK_TEXT_SELECTION format.
     */
    KIT_CALLBACK_SEARCH_RESULT_SELECTION = 15,

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
    KIT_CALLBACK_UNO_COMMAND_RESULT = 16,

    /**
     * The size and/or the position of the cell cursor changed.
     *
     * Payload format: "x, y, width, height, column, row", where the first
     * 4 numbers are document coordinates, in twips, and the last 2 are table
     * coordinates starting from 0.
     * When the cursor is not shown the payload format is the "EMPTY" string.
     *
     * Rectangle format is the same as KIT_CALLBACK_INVALIDATE_TILES.
     */
    KIT_CALLBACK_CELL_CURSOR = 17,

    /**
     * The current mouse pointer style.
     *
     * Payload is a css mouse pointer style.
     */
    KIT_CALLBACK_MOUSE_POINTER = 18,

    /**
     * The text content of the formula bar in Calc.
     */
    KIT_CALLBACK_CELL_FORMULA = 19,

    /**
     * Loading a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * kit::Office::setDocumentPassword().  The document cannot be loaded
     * without the password.
     */
    KIT_CALLBACK_DOCUMENT_PASSWORD = 20,

    /**
     * Editing a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * kit::Office::setDocumentPassword().
     */
    KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY = 21,

    /**
     * An error happened.
     *
     * The payload returns information further identifying the error, like:
     *
     * {
     *     "classification": "error" | "warning" | "info"
     *     "kind": "network" etc.
     *     "code": a structured 32-bit error code, the ErrCode from LibreOffice's <comphelper/errcode.hxx>
     *     "message": freeform description
     * }
     */
    KIT_CALLBACK_ERROR = 22,

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
    KIT_CALLBACK_CONTEXT_MENU = 23,

    /**
     * The size and/or the position of the view cursor changed. A view cursor
     * is a cursor of another view, the current view can't change it.
     *
     * The payload format:
     *
     * {
     *     "viewId": "..."
     *     "rectangle": "..."
     * }
     *
     * - viewId is a value returned earlier by kit::Document::createView()
     * - rectangle uses the format of KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR
     */
    KIT_CALLBACK_INVALIDATE_VIEW_CURSOR = 24,

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
     * - viewId is a value returned earlier by kit::Document::createView()
     * - selection uses the format of KIT_CALLBACK_TEXT_SELECTION.
     */
    KIT_CALLBACK_TEXT_VIEW_SELECTION = 25,

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
     * - viewId is a value returned earlier by kit::Document::createView()
     * - rectangle uses the format of KIT_CALLBACK_CELL_CURSOR.
     */
    KIT_CALLBACK_CELL_VIEW_CURSOR = 26,

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
     * - viewId is a value returned earlier by kit::Document::createView()
     * - selection uses the format of KIT_CALLBACK_INVALIDATE_TILES.
     */
    KIT_CALLBACK_GRAPHIC_VIEW_SELECTION = 27,

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
     * - viewId is a value returned earlier by kit::Document::createView()
     * - visible uses the format of KIT_CALLBACK_CURSOR_VISIBLE.
     */
    KIT_CALLBACK_VIEW_CURSOR_VISIBLE = 28,

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
     * - viewId is a value returned earlier by kit::Document::createView()
     * - rectangle uses the format of KIT_CALLBACK_INVALIDATE_TILES.
     */
    KIT_CALLBACK_VIEW_LOCK = 29,

    /**
     * The size of the change tracking table has changed.
     *
     * The payload example:
     * {
     *     "redline": {
     *         "action": "Remove",
     *         "index": "1",
     *         "author": "Unknown Author",
     *         "type": "Delete",
     *         "comment": "",
     *         "description": "Delete 'abc'",
     *         "dateTime": "2016-08-18T12:14:00"
     *     }
     * }
     *
     * The format is the same as an entry of
     * kit::Document::getCommandValues('.uno:AcceptTrackedChanges'), extra
     * fields:
     *
     * - 'action' is either 'Add' or 'Remove', depending on if this is an
     *   insertion into the table or a removal.
     */
    KIT_CALLBACK_REDLINE_TABLE_SIZE_CHANGED = 30,

    /**
     * An entry in the change tracking table has been modified.
     *
     * The payload example:
     * {
     *     "redline": {
     *         "action": "Modify",
     *         "index": "1",
     *         "author": "Unknown Author",
     *         "type": "Insert",
     *         "comment": "",
     *         "description": "Insert 'abcd'",
     *         "dateTime": "2016-08-18T13:13:00"
     *     }
     * }
     *
     * The format is the same as an entry of
     * kit::Document::getCommandValues('.uno:AcceptTrackedChanges'), extra
     * fields:
     *
     * - 'action' is 'Modify'.
     */
    KIT_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED = 31,

    /**
     * There is some change in comments in the document
     *
     * The payload example:
     * {
     *     "comment": {
     *         "action": "Add",
     *         "id": "11",
     *         "parent": "4",
     *         "author": "Unknown Author",
     *         "text": "",
     *         "dateTime": "2016-08-18T13:13:00",
     *         "anchorPos": "4529, 3906",
     *         "textRange": "1418, 3906, 3111, 919"
     *     }
     * }
     *
     * The format is the same as an entry of
     * kit::Document::getCommandValues('.uno:ViewAnnotations'), extra
     * fields:
     *
     * - 'action' can be 'Add', 'Remove' or 'Modify' depending on whether
     *    comment has been added, removed or modified.
     */
    KIT_CALLBACK_COMMENT = 32,

    /**
     * The column/row header is no more valid because of a column/row insertion
     * or a similar event. Clients must query a new column/row header set.
     *
     * The payload says if we are invalidating a row or column header. So,
     * payload values can be: "row", "column", "all".
     */
    KIT_CALLBACK_INVALIDATE_HEADER = 33,
    /**
     * The text content of the address field in Calc. Eg: "A7"
     */
    KIT_CALLBACK_CELL_ADDRESS = 34,
    /**
     * The key horizontal ruler related properties on change are reported by this.
     *
     * The payload format is:
     *
     * {
     *      "margin1": "...",
     *      "margin2": "...",
     *      "leftOffset": "...",
     *      "pageOffset": "...",
     *      "pageWidth": "...",
     *      "unit": "..."
     *  }
     *
     * Here all aproperties are same as described in svxruler.
     */
    KIT_CALLBACK_RULER_UPDATE = 35,
    /**
     * Window related callbacks are emitted under this category. It includes
     * external windows like dialogs, autopopups for now.
     *
     * The payload format is:
     *
     * {
     *    "id": "unique integer id of the dialog",
     *    "action": "<see below>",
     *    "type": "<see below>"
     *    "rectangle": "x, y, width, height"
     * }
     *
     * "type" tells the type of the window the action is associated with
     *  - "dialog" - window is a dialog
     *  - "child" - window is a floating window (combo boxes, etc.)
     *  - "deck" - window is a docked/floating deck (i.e. the sidebar)
     *  - "tooltip" - window is a tooltip popup
     *
     * "action" can take following values:
     * - "created" - window is created in the backend, client can render it now
     * - "title_changed" - window's title is changed
     * - "size_changed" - window's size is changed
     * - "invalidate" - the area as described by "rectangle" is invalidated
     *    Clients must request the new area
     * - "cursor_invalidate" - cursor is invalidated. New position is in "rectangle"
     * - "cursor_visible" - cursor visible status is changed. Status is available
     *    in "visible" field
     * - "close" - window is closed
     * - "show" - show the window
     * - "hide" - hide the window
     */
    KIT_CALLBACK_WINDOW = 36,

    /**
     * When for the current cell is defined a validity list we need to show
     * a drop down button in the form of a marker.
     *
     * The payload format is: "x, y, visible" where x, y are the current
     * cell cursor coordinates and visible is set to 0 or 1.
     */
    KIT_CALLBACK_VALIDITY_LIST_BUTTON = 37,

    /**
     * Notification that the clipboard contents have changed.
     * Typically fired in response to copying to clipboard.
     *
     * Payload is optional. When payload is empty, Online gets string from selected text.
     * Payload format is JSON.
     * Example: { "mimeType": "text/plain", "content": "some content" }
     */
    KIT_CALLBACK_CLIPBOARD_CHANGED = 38,

    /**
     * When the (editing) context changes - like the user switches from
     * editing textbox in Impress to editing a shape there.
     *
     * Payload is the application ID and context, delimited by space.
     * Eg. com.sun.star.presentation.PresentationDocument TextObject
     */
    KIT_CALLBACK_CONTEXT_CHANGED = 39,

    /**
     * On-load notification of the document signature status.
     */
    KIT_CALLBACK_SIGNATURE_STATUS = 40,

    /**
     * Profiling tracing information single string of multiple lines
     * containing <pid> <timestamp> and zone start/stop information
     */
    KIT_CALLBACK_PROFILE_FRAME = 41,

    /**
     * The position and size of the cell selection area. It is used to
     * draw the selection handles for cells in Calc documents.
     *
     * Rectangle format is the same as KIT_CALLBACK_INVALIDATE_TILES.
     */
    KIT_CALLBACK_CELL_SELECTION_AREA = 42,

    /**
     * The position and size of the cell auto fill area. It is used to
     * trigger auto fill functionality if that area is hit.
     *
     * Rectangle format is the same as KIT_CALLBACK_INVALIDATE_TILES.
     */
    KIT_CALLBACK_CELL_AUTO_FILL_AREA = 43,

    /**
     * When the cursor is in a table or a table is selected in the
     * document, this sends the table's column and row border positions
     * to the client. If the payload is empty (empty JSON object), then
     * no table is currently selected or the cursor is not inside a table
     * cell.
     */
    KIT_CALLBACK_TABLE_SELECTED = 44,

    /*
     * Show reference marks from payload.
     *
     * Example payload:
     * {
     *     "marks": [
     *         { "rectangle": "3825, 3315, 1245, 2010", "color": "0000ff", "part": "0" },
     *         { "rectangle": "8925, 4335, 2520, 735", "color": "ff0000", "part": "0" },
     *         ...
     *     ]
     * }
     */
    KIT_CALLBACK_REFERENCE_MARKS = 45,

    /**
     * Callback related to native dialogs generated in JavaScript from
     * the description.
     */
    KIT_CALLBACK_JSDIALOG = 46,

    /**
     * Send the list of functions whose name starts with the characters entered
     * by the user in the formula input bar.
     */
    KIT_CALLBACK_CALC_FUNCTION_LIST = 47,

    /**
     * Sends the tab stop list for the current of the current cursor position.
     */
    KIT_CALLBACK_TAB_STOP_LIST = 48,

    /**
     * Sends all information for displaying form field button for a text based field.
     *
     * It contains the position where the frame with the button should be displayed and
     * also contains all information that the popup window needs.
     *
     * The payload example:
     * {
     *      "action": "show",
     *      "type": "drop-down",
     *      "textArea": "1418, 3906, 3111, 919",
     *      "params": {
     *           "items": ["January", "February", "July"],
     *           "selected": "2",
     *           "placeholder": "No items specified"
     *      }
     * }
     *
     * or
     * {
     *      "action": "hide",
     *      "type": "drop-down"
     * }
     */
    KIT_CALLBACK_FORM_FIELD_BUTTON = 49,

    /**
     * This is Calc specific. Indicates that some or all of the current sheet's
     * geometry data has changed. Clients must request a full or partial sheet
     * geometry data set.
     *
     * The payload specifies what part of the sheet geometry data has changed.
     * The payload format is:
     * 'all|rows|columns [sizes [hidden [filtered [groups]]]]'
     *
     * For example, the payload 'rows sizes groups' indicates that the row heights
     * and row-groups data have changed.
     */
    KIT_CALLBACK_INVALIDATE_SHEET_GEOMETRY = 50,

    /**
     * When for the current cell is defined an input help text.
     *
     * The payload format is JSON: { "title": "title text", "content": "content text" }
     */
    KIT_CALLBACK_VALIDITY_INPUT_HELP = 51,

    /**
     * Indicates the document background color in the payload as a RGB hex string (RRGGBB).
     */
    KIT_CALLBACK_DOCUMENT_BACKGROUND_COLOR = 52,

    /**
     * When a user tries to use command which is restricted for that user
     */
    KIT_COMMAND_BLOCKED = 53,

    /**
     * The position of the cell cursor jumped to.
     *
     * Payload format: "x, y, width, height, column, row", where the first
     * 4 numbers are document coordinates, in twips, and the last 2 are table
     * coordinates starting from 0.
     * When the cursor is not shown the payload format is the "EMPTY" string.
     *
     * Rectangle format is the same as KIT_CALLBACK_INVALIDATE_TILES.
     */
    KIT_CALLBACK_SC_FOLLOW_JUMP = 54,

    /**
     * Sends all information for displaying metadata for a text based content control.
     *
     * Examples:
     * Entered a rich text content control:
     * {
     *     "action": "show",
     *     "alias": "my alias", // omitted if empty
     *     "rectangles": "1418, 1694, 720, 551; 10291, 1418, 1099, 275"
     * }
     *
     * Left a rich text content control:
     * {
     *     "action": "hide"
     * }
     *
     * Entered a dropdown content control:
     * {
     *     "action": "show",
     *     "rectangles": "...",
     *     "items": ["red", "green", "blue"]
     * }
     *
     * Clicked on a picture content control's placeholder:
     * {
     *     "action": "change-picture"
     * }
     *
     * Entered a date content control:
     * {
     *     "action": "show",
     *     "rectangles": "...",
     *     "date": "true"
     * }
     */
    KIT_CALLBACK_CONTENT_CONTROL = 55,

    /**
     * This is Calc specific. The payload contains print ranges of all
     * sheets in the document.
     *
     * Payload example:
     * {
     *     "printranges" : [
     *         {
     *             "sheet": 0,
     *             "ranges": [
     *                 [0, 0, 4, 5],
     *                 [5, 100, 8, 150]
     *             ]
     *         },
     *         {
     *             "sheet": 3,
     *             "ranges": [
     *                 [1, 0, 6, 10],
     *                 [3, 200, 6, 230]
     *             ]
     *         }
     *     ]
     * }
     *
     * The format of the inner "ranges" array for each sheet is
     * [<startColumn>, <startRow>, <endColumn>, <endRow>]
     */
    KIT_CALLBACK_PRINT_RANGES = 56,

    /**
     * Informs the COKit client that a font specified in the
     * document is missing.
     *
     * This callback is emitted right after the document has been loaded.
     *
     * Payload example:
     * {
     *     "fontsmissing": [
     *         "Some Random Font",
     *         "Another Font"
     *     ]
     * }
     *
     * The names are those of the font family. Sadly it is currently
     * not possible to know the name of the font style that is
     * missing.
     *
     */
    KIT_CALLBACK_FONTS_MISSING = 57,

    /**
     * Insertion, removal, movement, and selection of a media shape.
     * The payload is a json with the relevant details.
     *
     *      {
     *          "action": "insert",
     *          "id": 123456,
     *          "url": "file:// ..."
     *          "x": ...,
     *          "y": ...,
     *      }
     *
     *      where the "svg" property is a string containing an svg document
     *      which is a representation of the pie segment.
     */
    KIT_CALLBACK_MEDIA_SHAPE = 58,

    /**
     * The document is available to download by the client.
     *
     * Payload example:
     * "file:///tmp/hello-world.pdf"
     */
    KIT_CALLBACK_EXPORT_FILE = 59,

    /**
     * Some attribute of this view has changed, that will cause it
     * to completely re-render, eg. non-printing characters or
     * or dark mode was toggled, and then distinct from other views.
     *
     * Payload is an opaque string that matches this set of states.
     * this will be emitted after creating a new view.
     */
    KIT_CALLBACK_VIEW_RENDER_STATE = 60,

    /**
     * Informs the COKit client that the background color surrounding
     * the document has changed.
    */
   KIT_CALLBACK_APPLICATION_BACKGROUND_COLOR = 61,

    /**
     * Accessibility event: a paragraph got focus.
     * The payload is a json with the following structure.
     *
     *   {
     *       "content": "<paragraph text>"
     *       "position": N
     *       "start": N1
     *       "end": N2
     *       "listPrefixLength": L
     *   }
     *   where N is the position of the text cursor inside the focused paragraph,
     *   and [N1,N2] is the range of the text selection inside the focused paragraph.
     *   In case the paragraph is a list item, L is the length of the bullet/number prefix.
     */
    KIT_CALLBACK_A11Y_FOCUS_CHANGED = 62,

    /**
     * Accessibility event: text cursor position has changed.
     *
     *  {
     *      "position": N
     *  }
     *  where N is the position of the text cursor inside the focused paragraph.
     */
    KIT_CALLBACK_A11Y_CARET_CHANGED = 63,

    /**
     * Accessibility event: text selection has changed.
     *
     *  {
     *      "start": N1
     *      "end": N2
     *  }
     *  where [N1,N2] is the range of the text selection inside the focused paragraph.
     */
    KIT_CALLBACK_A11Y_TEXT_SELECTION_CHANGED = 64,

    /**
     * Informs the COKit client that the color palettes have changed.
    */
    KIT_CALLBACK_COLOR_PALETTES = 65,

    /**
     * Informs that the document password has been successfully changed.
     * The payload contains the new password and the type.
    */
    KIT_CALLBACK_DOCUMENT_PASSWORD_RESET = 66,

    /**
     * Accessibility event: a cell got focus.
     * The payload is a json with the following structure.
     *
     *   {
     *       "outCount": <number of tables user gets out of>
     *       "inList": [
     *           {
     *               "rowCount": <number of rows for outer table user got in>
     *               "colCount": <number of columns for outer table user got in>
     *           },
     *           ...
     *           {
     *               "rowCount": <number of rows for inner table user got in>
     *               "colCount": <number of columns for inner table user got in>
     *           }
     *       ]
     *       "row": <current row index>
     *       "col": <current column index>
     *       "rowSpan": <row span for current cell>
     *       "colSpan": <column span for current cell>
     *       "paragraph": {
     *           <same structure as for KIT_CALLBACK_A11Y_FOCUS_CHANGED>
     *        }
     *   }
     *   where row/column indexes start from 0, inList is the list of tables
     *   the user got in from the outer to the inner; row/column span default
     *   value is 1; paragraph is the cell text content.
     */
    KIT_CALLBACK_A11Y_FOCUSED_CELL_CHANGED = 67,

    /**
     * Accessibility event: text editing in a shape or cell has been enabled/disabled
     *
     *  {
     *      "cell": true/false (editing a cell ?)
     *      "enabled": true|false
     *      "selection": a selection description
     *      "paragraph": focused paragraph
     *  }
     */
    KIT_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE = 68,

    /**
     * Accessibility event: a selection (of a shape/graphic, etc.) has changed
     *
     *  {
     *      "cell": true/false (selected object is a cell ?)
     *      "action": "create"|"add"|"remove"
     *      "name": selected object name
     *      "text": text content if any
     *  }
     */
    KIT_CALLBACK_A11Y_SELECTION_CHANGED = 69,

    /**
     * Forwarding logs from core to client can be useful
     * for keep track of the real core/client event sequence
     *
     * Payload is the log to be sent
     */
    KIT_CALLBACK_CORE_LOG = 70,

    /**
     * Tooltips shown in the documents, like redline author and date.
     *
     *  {
     *      "text": "text of tooltip",
     *      "rectangle": "x, y, width, height"
     *  }
     */
    KIT_CALLBACK_TOOLTIP = 71,

    /**
     * Used for sending the rectangle for text inside a shape/textbox
     *
     *  Payload contains the rectangle details
     */
    KIT_CALLBACK_SHAPE_INNER_TEXT = 72,
        /**
     * The key vertical ruler related properties on change are reported by this.
     *
     * The payload format is:
     *
     * {
     *      "margin1": "...",
     *      "margin2": "...",
     *      "leftOffset": "...",
     *      "pageOffset": "...",
     *      "pageWidth": "...",
     *      "unit": "..."
     *  }
     *
     * Here all aproperties are same as described in svxruler.
     */
    KIT_CALLBACK_VERTICAL_RULER_UPDATE = 73

}
COKitCallbackType;

typedef enum
{
    /// A key on the keyboard is pressed.
    KIT_KEYEVENT_KEYINPUT,
    /// A key on the keyboard is released.
    KIT_KEYEVENT_KEYUP
}
COKitKeyEventType;

typedef enum
{
    /// cf. SalEvent::ExtTextInput
    KIT_EXT_TEXTINPUT,
    /// cf. SalEvent::ExtTextInputPos
    KIT_EXT_TEXTINPUT_POS,
    /// cf. SalEvent::EndExtTextInput
    KIT_EXT_TEXTINPUT_END
}
COKitExtTextInputType;

/// Returns the string representation of a COKitCallbackType enumeration element.
static inline const char* kitCallbackTypeToString(int nType)
{
    switch (static_cast<COKitCallbackType>(nType))
    {
    case KIT_CALLBACK_INVALIDATE_TILES:
        return "KIT_CALLBACK_INVALIDATE_TILES";
    case KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        return "KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR";
    case KIT_CALLBACK_TEXT_SELECTION:
        return "KIT_CALLBACK_TEXT_SELECTION";
    case KIT_CALLBACK_TEXT_SELECTION_START:
        return "KIT_CALLBACK_TEXT_SELECTION_START";
    case KIT_CALLBACK_TEXT_SELECTION_END:
        return "KIT_CALLBACK_TEXT_SELECTION_END";
    case KIT_CALLBACK_CURSOR_VISIBLE:
        return "KIT_CALLBACK_CURSOR_VISIBLE";
    case KIT_CALLBACK_VIEW_CURSOR_VISIBLE:
        return "KIT_CALLBACK_VIEW_CURSOR_VISIBLE";
    case KIT_CALLBACK_GRAPHIC_SELECTION:
        return "KIT_CALLBACK_GRAPHIC_SELECTION";
    case KIT_CALLBACK_GRAPHIC_VIEW_SELECTION:
        return "KIT_CALLBACK_GRAPHIC_VIEW_SELECTION";
    case KIT_CALLBACK_CELL_CURSOR:
        return "KIT_CALLBACK_CELL_CURSOR";
    case KIT_CALLBACK_HYPERLINK_CLICKED:
        return "KIT_CALLBACK_HYPERLINK_CLICKED";
    case KIT_CALLBACK_MOUSE_POINTER:
        return "KIT_CALLBACK_MOUSE_POINTER";
    case KIT_CALLBACK_STATE_CHANGED:
        return "KIT_CALLBACK_STATE_CHANGED";
    case KIT_CALLBACK_STATUS_INDICATOR_START:
        return "KIT_CALLBACK_STATUS_INDICATOR_START";
    case KIT_CALLBACK_STATUS_INDICATOR_SET_VALUE:
        return "KIT_CALLBACK_STATUS_INDICATOR_SET_VALUE";
    case KIT_CALLBACK_STATUS_INDICATOR_FINISH:
        return "KIT_CALLBACK_STATUS_INDICATOR_FINISH";
    case KIT_CALLBACK_SEARCH_NOT_FOUND:
        return "KIT_CALLBACK_SEARCH_NOT_FOUND";
    case KIT_CALLBACK_DOCUMENT_SIZE_CHANGED:
        return "KIT_CALLBACK_DOCUMENT_SIZE_CHANGED";
    case KIT_CALLBACK_SET_PART:
        return "KIT_CALLBACK_SET_PART";
    case KIT_CALLBACK_SEARCH_RESULT_SELECTION:
        return "KIT_CALLBACK_SEARCH_RESULT_SELECTION";
    case KIT_CALLBACK_DOCUMENT_PASSWORD:
        return "KIT_CALLBACK_DOCUMENT_PASSWORD";
    case KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY:
        return "KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY";
    case KIT_CALLBACK_CONTEXT_MENU:
        return "KIT_CALLBACK_CONTEXT_MENU";
    case KIT_CALLBACK_INVALIDATE_VIEW_CURSOR:
        return "KIT_CALLBACK_INVALIDATE_VIEW_CURSOR";
    case KIT_CALLBACK_TEXT_VIEW_SELECTION:
        return "KIT_CALLBACK_TEXT_VIEW_SELECTION";
    case KIT_CALLBACK_CELL_VIEW_CURSOR:
        return "KIT_CALLBACK_CELL_VIEW_CURSOR";
    case KIT_CALLBACK_CELL_ADDRESS:
        return "KIT_CALLBACK_CELL_ADDRESS";
    case KIT_CALLBACK_CELL_FORMULA:
        return "KIT_CALLBACK_CELL_FORMULA";
    case KIT_CALLBACK_UNO_COMMAND_RESULT:
        return "KIT_CALLBACK_UNO_COMMAND_RESULT";
    case KIT_CALLBACK_ERROR:
        return "KIT_CALLBACK_ERROR";
    case KIT_CALLBACK_VIEW_LOCK:
        return "KIT_CALLBACK_VIEW_LOCK";
    case KIT_CALLBACK_REDLINE_TABLE_SIZE_CHANGED:
        return "KIT_CALLBACK_REDLINE_TABLE_SIZE_CHANGED";
    case KIT_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED:
        return "KIT_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED";
    case KIT_CALLBACK_INVALIDATE_HEADER:
        return "KIT_CALLBACK_INVALIDATE_HEADER";
    case KIT_CALLBACK_COMMENT:
        return "KIT_CALLBACK_COMMENT";
    case KIT_CALLBACK_RULER_UPDATE:
        return "KIT_CALLBACK_RULER_UPDATE";
    case KIT_CALLBACK_VERTICAL_RULER_UPDATE:
        return "KIT_CALLBACK_VERTICAL_RULER_UPDATE";
    case KIT_CALLBACK_WINDOW:
        return "KIT_CALLBACK_WINDOW";
    case KIT_CALLBACK_VALIDITY_LIST_BUTTON:
        return "KIT_CALLBACK_VALIDITY_LIST_BUTTON";
    case KIT_CALLBACK_VALIDITY_INPUT_HELP:
        return "KIT_CALLBACK_VALIDITY_INPUT_HELP";
    case KIT_CALLBACK_CLIPBOARD_CHANGED:
        return "KIT_CALLBACK_CLIPBOARD_CHANGED";
    case KIT_CALLBACK_CONTEXT_CHANGED:
        return "KIT_CALLBACK_CONTEXT_CHANGED";
    case KIT_CALLBACK_SIGNATURE_STATUS:
        return "KIT_CALLBACK_SIGNATURE_STATUS";
    case KIT_CALLBACK_PROFILE_FRAME:
        return "KIT_CALLBACK_PROFILE_FRAME";
    case KIT_CALLBACK_CELL_SELECTION_AREA:
        return "KIT_CALLBACK_CELL_SELECTION_AREA";
    case KIT_CALLBACK_CELL_AUTO_FILL_AREA:
        return "KIT_CALLBACK_CELL_AUTO_FILL_AREA";
    case KIT_CALLBACK_TABLE_SELECTED:
        return "KIT_CALLBACK_TABLE_SELECTED";
    case KIT_CALLBACK_REFERENCE_MARKS:
        return "KIT_CALLBACK_REFERENCE_MARKS";
    case KIT_CALLBACK_JSDIALOG:
        return "KIT_CALLBACK_JSDIALOG";
    case KIT_CALLBACK_CALC_FUNCTION_LIST:
        return "KIT_CALLBACK_CALC_FUNCTION_LIST";
    case KIT_CALLBACK_TAB_STOP_LIST:
        return "KIT_CALLBACK_TAB_STOP_LIST";
    case KIT_CALLBACK_FORM_FIELD_BUTTON:
        return "KIT_CALLBACK_FORM_FIELD_BUTTON";
    case KIT_CALLBACK_INVALIDATE_SHEET_GEOMETRY:
        return "KIT_CALLBACK_INVALIDATE_SHEET_GEOMETRY";
    case KIT_CALLBACK_DOCUMENT_BACKGROUND_COLOR:
        return "KIT_CALLBACK_DOCUMENT_BACKGROUND_COLOR";
    case KIT_COMMAND_BLOCKED:
        return "KIT_COMMAND_BLOCKED";
    case KIT_CALLBACK_SC_FOLLOW_JUMP:
        return "KIT_CALLBACK_SC_FOLLOW_JUMP";
    case KIT_CALLBACK_CONTENT_CONTROL:
        return "KIT_CALLBACK_CONTENT_CONTROL";
    case KIT_CALLBACK_PRINT_RANGES:
        return "KIT_CALLBACK_PRINT_RANGES";
    case KIT_CALLBACK_FONTS_MISSING:
        return "KIT_CALLBACK_FONTS_MISSING";
    case KIT_CALLBACK_MEDIA_SHAPE:
        return "KIT_CALLBACK_MEDIA_SHAPE";
    case KIT_CALLBACK_EXPORT_FILE:
        return "KIT_CALLBACK_EXPORT_FILE";
    case KIT_CALLBACK_VIEW_RENDER_STATE:
        return "KIT_CALLBACK_VIEW_RENDER_STATE";
    case KIT_CALLBACK_APPLICATION_BACKGROUND_COLOR:
        return "KIT_CALLBACK_APPLICATION_BACKGROUND_COLOR";
    case KIT_CALLBACK_A11Y_FOCUS_CHANGED:
        return "KIT_CALLBACK_A11Y_FOCUS_CHANGED";
    case KIT_CALLBACK_A11Y_CARET_CHANGED:
        return "KIT_CALLBACK_A11Y_CARET_CHANGED";
    case KIT_CALLBACK_A11Y_TEXT_SELECTION_CHANGED:
        return "KIT_CALLBACK_A11Y_TEXT_SELECTION_CHANGED";
    case KIT_CALLBACK_COLOR_PALETTES:
        return "KIT_CALLBACK_COLOR_PALETTES";
    case KIT_CALLBACK_DOCUMENT_PASSWORD_RESET:
        return "KIT_CALLBACK_DOCUMENT_PASSWORD_RESET";
    case KIT_CALLBACK_A11Y_FOCUSED_CELL_CHANGED:
        return "KIT_CALLBACK_A11Y_FOCUSED_CELL_CHANGED";
    case KIT_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE:
        return "KIT_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE";
    case KIT_CALLBACK_A11Y_SELECTION_CHANGED:
        return "KIT_CALLBACK_A11Y_SELECTION_CHANGED";
    case KIT_CALLBACK_CORE_LOG:
        return "KIT_CALLBACK_CORE_LOG";
    case KIT_CALLBACK_TOOLTIP:
        return "KIT_CALLBACK_TOOLTIP";
    case KIT_CALLBACK_SHAPE_INNER_TEXT:
        return "KIT_CALLBACK_SHAPE_INNER_TEXT";
    }

    assert(!"Unknown COKitCallbackType type.");
    return nullptr;
}

typedef enum
{
    /// A mouse button has been pressed down.
    KIT_MOUSEEVENT_MOUSEBUTTONDOWN,
    /// A mouse button has been let go.
    KIT_MOUSEEVENT_MOUSEBUTTONUP,
    /// The mouse has moved while a button is pressed.
    KIT_MOUSEEVENT_MOUSEMOVE
}
COKitMouseEventType;

typedef enum
{
    /// The start of selection is to be adjusted.
    KIT_SETTEXTSELECTION_START,
    /// The end of selection is to be adjusted.
    KIT_SETTEXTSELECTION_END,
    /// Both the start and the end of selection is to be adjusted.
    KIT_SETTEXTSELECTION_RESET
}
COKitSetTextSelectionType;

typedef enum
{
    /**
     * A move or a resize action starts. It is assumed that there is a valid
     * graphic selection (see KIT_CALLBACK_GRAPHIC_SELECTION) and the supplied
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
    KIT_SETGRAPHICSELECTION_START,
    /**
     * A move or resize action stops. It is assumed that this is always used
     * only after a KIT_SETTEXTSELECTION_START. The supplied coordinates are
     * the ones where the user released the screen.
     */
    KIT_SETGRAPHICSELECTION_END
}
COKitSetGraphicSelectionType;

#endif // defined KIT_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_COKIT_COKIT_ENUMS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
