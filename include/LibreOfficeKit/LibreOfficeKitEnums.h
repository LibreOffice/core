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

#include <assert.h>

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

typedef enum
{
    LOK_WINDOW_CLOSE,
    LOK_WINDOW_PASTE
}
LibreOfficeKitWindowAction;

typedef enum
{
    LOK_SELTYPE_NONE,
    LOK_SELTYPE_TEXT,
    LOK_SELTYPE_LARGE_TEXT, // unused (same as LOK_SELTYPE_COMPLEX)
    LOK_SELTYPE_COMPLEX
}
LibreOfficeKitSelectionType;

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

    /**
     * Request to have the part number as an 5th value in the
     * LOK_CALLBACK_INVALIDATE_TILES payload.
     */
    LOK_FEATURE_PART_IN_INVALIDATION_CALLBACK = (1ULL << 2),

    /**
     * Turn off tile rendering for annotations
     */
    LOK_FEATURE_NO_TILED_ANNOTATIONS = (1ULL << 3),

    /**
     * Enable range based header data
     */
    LOK_FEATURE_RANGE_HEADERS = (1ULL << 4),

    /**
     * Request to have the active view's Id as the 1st value in the
     * LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR payload.
     */
    LOK_FEATURE_VIEWID_IN_VISCURSOR_INVALIDATION_CALLBACK = (1ULL << 5)
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
     *
     * @see LOK_FEATURE_PART_IN_INVALIDATION_CALLBACK.
     */
    LOK_CALLBACK_INVALIDATE_TILES = 0,
    /**
     * The size and/or the position of the visible cursor changed.
     *
     * Old format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     * New format is a JSON with 3 elements the 'viewId' element represented by
     * an integer value, a 'rectangle' element in the format "x, y, width, height",
     * and a 'misspelledWord' element represented by an integer value: '1' when
     * a misspelled word is at the cursor position, '0' when the word is
     * not misspelled.
     */
    LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR = 1,
    /**
     * The list of rectangles representing the current text selection changed.
     *
     * List format is "rectangle1[; rectangle2[; ...]]" (without quotes and
     * brackets), where rectangleN has the same format as
     * LOK_CALLBACK_INVALIDATE_TILES. When there is no selection, an empty
     * string is provided.
     */
    LOK_CALLBACK_TEXT_SELECTION = 2,
    /**
     * The position and size of the cursor rectangle at the text
     * selection start. It is used to draw the selection handles.
     *
     * This callback must be called prior to LOK_CALLBACK_TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_TEXT_SELECTION_START = 3,
    /**
     * The position and size of the cursor rectangle at the text
     * selection end. It is used to draw the selection handles.
     *
     * This callback must be called prior to LOK_CALLBACK_TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_TEXT_SELECTION_END = 4,
    /**
     * The blinking text cursor is now visible or not.
     *
     * Clients should assume that this is true initially and are expected to
     * hide the blinking cursor at the rectangle described by
     * LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR once it becomes false. Payload is
     * either the "true" or the "false" string.
     */
    LOK_CALLBACK_CURSOR_VISIBLE = 5,
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
    LOK_CALLBACK_GRAPHIC_SELECTION = 6,

    /**
     * User clicked on a hyperlink that should be handled by other
     * applications accordingly.
     */
    LOK_CALLBACK_HYPERLINK_CLICKED = 7,

    /**
     * Emit state update to the client.
     * For example, when cursor is on bold text, this callback is triggered
     * with payload: ".uno:Bold=true"
     */
    LOK_CALLBACK_STATE_CHANGED = 8,

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
    LOK_CALLBACK_STATUS_INDICATOR_START = 9,

    /**
     * Sets the numeric value of the status indicator.
     * The payload should be a percentage, an integer between 0 and 100.
     */
    LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE = 10,

    /**
     * Ends the status indicator.
     *
     * Not necessarily ever emitted.
     */
    LOK_CALLBACK_STATUS_INDICATOR_FINISH = 11,

    /**
     * No match was found for the search input
     */
    LOK_CALLBACK_SEARCH_NOT_FOUND = 12,

    /**
     * Size of the document changed.
     *
     * Payload format is "width, height", i.e. clients get the new size without
     * having to do an explicit lok::Document::getDocumentSize() call.
     *
     * A size change is always preceded by a series of
     * LOK_CALLBACK_INVALIDATE_TILES events invalidating any areas
     * need re-rendering to adapt.
     */
    LOK_CALLBACK_DOCUMENT_SIZE_CHANGED = 13,

    /**
     * The current part number is changed.
     *
     * Payload is a single 0-based integer.
     */
    LOK_CALLBACK_SET_PART = 14,

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
    LOK_CALLBACK_SEARCH_RESULT_SELECTION = 15,

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
    LOK_CALLBACK_UNO_COMMAND_RESULT = 16,

    /**
     * The size and/or the position of the cell cursor changed.
     *
     * Payload format: "x, y, width, height, column, row", where the first
     * 4 numbers are document coordinates, in twips, and the last 2 are table
     * coordinates starting from 0.
     * When the cursor is not shown the payload format is the "EMPTY" string.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_CELL_CURSOR = 17,

    /**
     * The current mouse pointer style.
     *
     * Payload is a css mouse pointer style.
     */
    LOK_CALLBACK_MOUSE_POINTER = 18,

    /**
     * The text content of the formula bar in Calc.
     */
    LOK_CALLBACK_CELL_FORMULA = 19,

    /**
     * Loading a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * lok::Office::setDocumentPassword().  The document cannot be loaded
     * without the password.
     */
    LOK_CALLBACK_DOCUMENT_PASSWORD = 20,

    /**
     * Editing a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * lok::Office::setDocumentPassword().
     */
    LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY = 21,

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
    LOK_CALLBACK_ERROR = 22,

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
    LOK_CALLBACK_CONTEXT_MENU = 23,

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
     * - viewId is a value returned earlier by lok::Document::createView()
     * - rectangle uses the format of LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR
     */
    LOK_CALLBACK_INVALIDATE_VIEW_CURSOR = 24,

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
    LOK_CALLBACK_TEXT_VIEW_SELECTION = 25,

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
    LOK_CALLBACK_CELL_VIEW_CURSOR = 26,

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
    LOK_CALLBACK_GRAPHIC_VIEW_SELECTION = 27,

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
    LOK_CALLBACK_VIEW_CURSOR_VISIBLE = 28,

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
    LOK_CALLBACK_VIEW_LOCK = 29,

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
     * lok::Document::getCommandValues('.uno:AcceptTrackedChanges'), extra
     * fields:
     *
     * - 'action' is either 'Add' or 'Remove', depending on if this is an
     *   insertion into the table or a removal.
     */
    LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED = 30,

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
     * lok::Document::getCommandValues('.uno:AcceptTrackedChanges'), extra
     * fields:
     *
     * - 'action' is 'Modify'.
     */
    LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED = 31,

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
     * lok::Document::getCommandValues('.uno:ViewAnnotations'), extra
     * fields:
     *
     * - 'action' can be 'Add', 'Remove' or 'Modify' depending on whether
     *    comment has been added, removed or modified.
     */
    LOK_CALLBACK_COMMENT = 32,

    /**
     * The column/row header is no more valid because of a column/row insertion
     * or a similar event. Clients must query a new column/row header set.
     *
     * The payload says if we are invalidating a row or column header. So,
     * payload values can be: "row", "column", "all".
     */
    LOK_CALLBACK_INVALIDATE_HEADER = 33,
    /**
     * The text content of the address field in Calc. Eg: "A7"
     */
    LOK_CALLBACK_CELL_ADDRESS = 34,
    /**
     * The key ruler related properties on change are reported by this.
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
    LOK_CALLBACK_RULER_UPDATE = 35,
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
    LOK_CALLBACK_WINDOW = 36,

    /**
     * When for the current cell is defined a validity list we need to show
     * a drop down button in the form of a marker.
     *
     * The payload format is: "x, y, visible" where x, y are the current
     * cell cursor coordinates and visible is set to 0 or 1.
     */
    LOK_CALLBACK_VALIDITY_LIST_BUTTON = 37,

    /**
     * Notification that the clipboard contents have changed.
     * Typically fired in response to copying to clipboard.
     *
     * Payload is optional. When payload is empty, Online gets string from selected text.
     * Payload format is JSON.
     * Example: { "mimeType": "text/plain", "content": "some content" }
     */
    LOK_CALLBACK_CLIPBOARD_CHANGED = 38,

    /**
     * When the (editing) context changes - like the user switches from
     * editing textbox in Impress to editing a shape there.
     *
     * Payload is the application ID and context, delimited by space.
     * Eg. com.sun.star.presentation.PresentationDocument TextObject
     */
    LOK_CALLBACK_CONTEXT_CHANGED = 39,

    /**
     * On-load notification of the document signature status.
     */
    LOK_CALLBACK_SIGNATURE_STATUS = 40,

    /**
     * Profiling tracing information single string of multiple lines
     * containing <pid> <timestamp> and zone start/stop information
     */
    LOK_CALLBACK_PROFILE_FRAME = 41,

    /**
     * The position and size of the cell selection area. It is used to
     * draw the selection handles for cells in Calc documents.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_CELL_SELECTION_AREA = 42,

    /**
     * The position and size of the cell auto fill area. It is used to
     * trigger auto fill functionality if that area is hit.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_CELL_AUTO_FILL_AREA = 43,

    /**
     * When the cursor is in a table or a table is selected in the
     * document, this sends the table's column and row border positions
     * to the client. If the payload is empty (empty JSON object), then
     * no table is currently selected or the cursor is not inside a table
     * cell.
     */
    LOK_CALLBACK_TABLE_SELECTED = 44,

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
    LOK_CALLBACK_REFERENCE_MARKS = 45,

    /**
     * Callback related to native dialogs generated in JavaScript from
     * the description.
     */
    LOK_CALLBACK_JSDIALOG = 46,

    /**
     * Send the list of functions whose name starts with the characters entered
     * by the user in the formula input bar.
     */
    LOK_CALLBACK_CALC_FUNCTION_LIST = 47,

    /**
     * Sends the tab stop list for the current of the current cursor position.
     */
    LOK_CALLBACK_TAB_STOP_LIST = 48,

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
    LOK_CALLBACK_FORM_FIELD_BUTTON = 49,

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
    LOK_CALLBACK_INVALIDATE_SHEET_GEOMETRY = 50,

    /**
     * When for the current cell is defined an input help text.
     *
     * The payload format is JSON: { "title": "title text", "content": "content text" }
     */
    LOK_CALLBACK_VALIDITY_INPUT_HELP = 51,

    /**
     * Indicates the document background color in the payload as a RGB hex string (RRGGBB).
     */
    LOK_CALLBACK_DOCUMENT_BACKGROUND_COLOR = 52,

    /**
     * When a user tries to use command which is restricted for that user
     */
    LOK_COMMAND_BLOCKED = 53,

    /**
     * The position of the cell cursor jumped to.
     *
     * Payload format: "x, y, width, height, column, row", where the first
     * 4 numbers are document coordinates, in twips, and the last 2 are table
     * coordinates starting from 0.
     * When the cursor is not shown the payload format is the "EMPTY" string.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_SC_FOLLOW_JUMP = 54,

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
    LOK_CALLBACK_CONTENT_CONTROL = 55,

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
    LOK_CALLBACK_PRINT_RANGES = 56,

    /**
     * Informs the LibreOfficeKit client that a font specified in the
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
    LOK_CALLBACK_FONTS_MISSING = 57,

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
    LOK_CALLBACK_MEDIA_SHAPE = 58,

    /**
     * The document is available to download by the client.
     *
     * Payload example:
     * "file:///tmp/hello-world.pdf"
     */
    LOK_CALLBACK_EXPORT_FILE = 59,

    /**
     * Some attribute of this view has changed, that will cause it
     * to completely re-render, eg. non-printing characters or
     * or dark mode was toggled, and then distinct from other views.
     *
     * Payload is an opaque string that matches this set of states.
     * this will be emitted after creating a new view.
     */
    LOK_CALLBACK_VIEW_RENDER_STATE = 60,

    /**
     * Informs the LibreOfficeKit client that the background color surrounding
     * the document has changed.
    */
   LOK_CALLBACK_APPLICATION_BACKGROUND_COLOR = 61,

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
    LOK_CALLBACK_A11Y_FOCUS_CHANGED = 62,

    /**
     * Accessibility event: text cursor position has changed.
     *
     *  {
     *      "position": N
     *  }
     *  where N is the position of the text cursor inside the focused paragraph.
     */
    LOK_CALLBACK_A11Y_CARET_CHANGED = 63,

    /**
     * Accessibility event: text selection has changed.
     *
     *  {
     *      "start": N1
     *      "end": N2
     *  }
     *  where [N1,N2] is the range of the text selection inside the focused paragraph.
     */
    LOK_CALLBACK_A11Y_TEXT_SELECTION_CHANGED = 64,

    /**
     * Informs the LibreOfficeKit client that the color palettes have changed.
    */
    LOK_CALLBACK_COLOR_PALETTES = 65,

    /**
     * Informs that the document password has been succesfully changed.
     * The payload contains the the new password and the type.
    */
    LOK_CALLBACK_DOCUMENT_PASSWORD_RESET = 66,

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
     *           <same structure as for LOK_CALLBACK_A11Y_FOCUS_CHANGED>
     *        }
     *   }
     *   where row/column indexes start from 0, inList is the list of tables
     *   the user got in from the outer to the inner; row/column span default
     *   value is 1; paragraph is the cell text content.
     */
    LOK_CALLBACK_A11Y_FOCUSED_CELL_CHANGED = 67,

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
    LOK_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE = 68,

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
    LOK_CALLBACK_A11Y_SELECTION_CHANGED = 69,

    /**
     * Forwarding logs from core to client can be useful
     * for keep track of the real core/client event sequence
     *
     * Payload is the log to be sent
     */
    LOK_CALLBACK_CORE_LOG = 70,

    /**
     * Tooltips shown in the documents, like redline author and date.
     *
     *  {
     *      "text": "text of tooltip",
     *      "rectangle": "x, y, width, height"
     *  }
     */
    LOK_CALLBACK_TOOLTIP = 71,

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
    /// cf. SalEvent::ExtTextInput
    LOK_EXT_TEXTINPUT,
    /// cf. SalEvent::ExtTextInputPos
    LOK_EXT_TEXTINPUT_POS,
    /// cf. SalEvent::EndExtTextInput
    LOK_EXT_TEXTINPUT_END
}
LibreOfficeKitExtTextInputType;

/// Returns the string representation of a LibreOfficeKitCallbackType enumeration element.
static inline const char* lokCallbackTypeToString(int nType)
{
    switch (static_cast<LibreOfficeKitCallbackType>(nType))
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
        return "LOK_CALLBACK_INVALIDATE_TILES";
    case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        return "LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR";
    case LOK_CALLBACK_TEXT_SELECTION:
        return "LOK_CALLBACK_TEXT_SELECTION";
    case LOK_CALLBACK_TEXT_SELECTION_START:
        return "LOK_CALLBACK_TEXT_SELECTION_START";
    case LOK_CALLBACK_TEXT_SELECTION_END:
        return "LOK_CALLBACK_TEXT_SELECTION_END";
    case LOK_CALLBACK_CURSOR_VISIBLE:
        return "LOK_CALLBACK_CURSOR_VISIBLE";
    case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
        return "LOK_CALLBACK_VIEW_CURSOR_VISIBLE";
    case LOK_CALLBACK_GRAPHIC_SELECTION:
        return "LOK_CALLBACK_GRAPHIC_SELECTION";
    case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
        return "LOK_CALLBACK_GRAPHIC_VIEW_SELECTION";
    case LOK_CALLBACK_CELL_CURSOR:
        return "LOK_CALLBACK_CELL_CURSOR";
    case LOK_CALLBACK_HYPERLINK_CLICKED:
        return "LOK_CALLBACK_HYPERLINK_CLICKED";
    case LOK_CALLBACK_MOUSE_POINTER:
        return "LOK_CALLBACK_MOUSE_POINTER";
    case LOK_CALLBACK_STATE_CHANGED:
        return "LOK_CALLBACK_STATE_CHANGED";
    case LOK_CALLBACK_STATUS_INDICATOR_START:
        return "LOK_CALLBACK_STATUS_INDICATOR_START";
    case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
        return "LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE";
    case LOK_CALLBACK_STATUS_INDICATOR_FINISH:
        return "LOK_CALLBACK_STATUS_INDICATOR_FINISH";
    case LOK_CALLBACK_SEARCH_NOT_FOUND:
        return "LOK_CALLBACK_SEARCH_NOT_FOUND";
    case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
        return "LOK_CALLBACK_DOCUMENT_SIZE_CHANGED";
    case LOK_CALLBACK_SET_PART:
        return "LOK_CALLBACK_SET_PART";
    case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
        return "LOK_CALLBACK_SEARCH_RESULT_SELECTION";
    case LOK_CALLBACK_DOCUMENT_PASSWORD:
        return "LOK_CALLBACK_DOCUMENT_PASSWORD";
    case LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY:
        return "LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY";
    case LOK_CALLBACK_CONTEXT_MENU:
        return "LOK_CALLBACK_CONTEXT_MENU";
    case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
        return "LOK_CALLBACK_INVALIDATE_VIEW_CURSOR";
    case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        return "LOK_CALLBACK_TEXT_VIEW_SELECTION";
    case LOK_CALLBACK_CELL_VIEW_CURSOR:
        return "LOK_CALLBACK_CELL_VIEW_CURSOR";
    case LOK_CALLBACK_CELL_ADDRESS:
        return "LOK_CALLBACK_CELL_ADDRESS";
    case LOK_CALLBACK_CELL_FORMULA:
        return "LOK_CALLBACK_CELL_FORMULA";
    case LOK_CALLBACK_UNO_COMMAND_RESULT:
        return "LOK_CALLBACK_UNO_COMMAND_RESULT";
    case LOK_CALLBACK_ERROR:
        return "LOK_CALLBACK_ERROR";
    case LOK_CALLBACK_VIEW_LOCK:
        return "LOK_CALLBACK_VIEW_LOCK";
    case LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED:
        return "LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED";
    case LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED:
        return "LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED";
    case LOK_CALLBACK_INVALIDATE_HEADER:
        return "LOK_CALLBACK_INVALIDATE_HEADER";
    case LOK_CALLBACK_COMMENT:
        return "LOK_CALLBACK_COMMENT";
    case LOK_CALLBACK_RULER_UPDATE:
        return "LOK_CALLBACK_RULER_UPDATE";
    case LOK_CALLBACK_WINDOW:
        return "LOK_CALLBACK_WINDOW";
    case LOK_CALLBACK_VALIDITY_LIST_BUTTON:
        return "LOK_CALLBACK_VALIDITY_LIST_BUTTON";
    case LOK_CALLBACK_VALIDITY_INPUT_HELP:
        return "LOK_CALLBACK_VALIDITY_INPUT_HELP";
    case LOK_CALLBACK_CLIPBOARD_CHANGED:
        return "LOK_CALLBACK_CLIPBOARD_CHANGED";
    case LOK_CALLBACK_CONTEXT_CHANGED:
        return "LOK_CALLBACK_CONTEXT_CHANGED";
    case LOK_CALLBACK_SIGNATURE_STATUS:
        return "LOK_CALLBACK_SIGNATURE_STATUS";
    case LOK_CALLBACK_PROFILE_FRAME:
        return "LOK_CALLBACK_PROFILE_FRAME";
    case LOK_CALLBACK_CELL_SELECTION_AREA:
        return "LOK_CALLBACK_CELL_SELECTION_AREA";
    case LOK_CALLBACK_CELL_AUTO_FILL_AREA:
        return "LOK_CALLBACK_CELL_AUTO_FILL_AREA";
    case LOK_CALLBACK_TABLE_SELECTED:
        return "LOK_CALLBACK_TABLE_SELECTED";
    case LOK_CALLBACK_REFERENCE_MARKS:
        return "LOK_CALLBACK_REFERENCE_MARKS";
    case LOK_CALLBACK_JSDIALOG:
        return "LOK_CALLBACK_JSDIALOG";
    case LOK_CALLBACK_CALC_FUNCTION_LIST:
        return "LOK_CALLBACK_CALC_FUNCTION_LIST";
    case LOK_CALLBACK_TAB_STOP_LIST:
        return "LOK_CALLBACK_TAB_STOP_LIST";
    case LOK_CALLBACK_FORM_FIELD_BUTTON:
        return "LOK_CALLBACK_FORM_FIELD_BUTTON";
    case LOK_CALLBACK_INVALIDATE_SHEET_GEOMETRY:
        return "LOK_CALLBACK_INVALIDATE_SHEET_GEOMETRY";
    case LOK_CALLBACK_DOCUMENT_BACKGROUND_COLOR:
        return "LOK_CALLBACK_DOCUMENT_BACKGROUND_COLOR";
    case LOK_COMMAND_BLOCKED:
        return "LOK_COMMAND_BLOCKED";
    case LOK_CALLBACK_SC_FOLLOW_JUMP:
        return "LOK_CALLBACK_SC_FOLLOW_JUMP";
    case LOK_CALLBACK_CONTENT_CONTROL:
        return "LOK_CALLBACK_CONTENT_CONTROL";
    case LOK_CALLBACK_PRINT_RANGES:
        return "LOK_CALLBACK_PRINT_RANGES";
    case LOK_CALLBACK_FONTS_MISSING:
        return "LOK_CALLBACK_FONTS_MISSING";
    case LOK_CALLBACK_MEDIA_SHAPE:
        return "LOK_CALLBACK_MEDIA_SHAPE";
    case LOK_CALLBACK_EXPORT_FILE:
        return "LOK_CALLBACK_EXPORT_FILE";
    case LOK_CALLBACK_VIEW_RENDER_STATE:
        return "LOK_CALLBACK_VIEW_RENDER_STATE";
    case LOK_CALLBACK_APPLICATION_BACKGROUND_COLOR:
        return "LOK_CALLBACK_APPLICATION_BACKGROUND_COLOR";
    case LOK_CALLBACK_A11Y_FOCUS_CHANGED:
        return "LOK_CALLBACK_A11Y_FOCUS_CHANGED";
    case LOK_CALLBACK_A11Y_CARET_CHANGED:
        return "LOK_CALLBACK_A11Y_CARET_CHANGED";
    case LOK_CALLBACK_A11Y_TEXT_SELECTION_CHANGED:
        return "LOK_CALLBACK_A11Y_TEXT_SELECTION_CHANGED";
    case LOK_CALLBACK_COLOR_PALETTES:
        return "LOK_CALLBACK_COLOR_PALETTES";
    case LOK_CALLBACK_DOCUMENT_PASSWORD_RESET:
        return "LOK_CALLBACK_DOCUMENT_PASSWORD_RESET";
    case LOK_CALLBACK_A11Y_FOCUSED_CELL_CHANGED:
        return "LOK_CALLBACK_A11Y_FOCUSED_CELL_CHANGED";
    case LOK_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE:
        return "LOK_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE";
    case LOK_CALLBACK_A11Y_SELECTION_CHANGED:
        return "LOK_CALLBACK_A11Y_SELECTION_CHANGED";
    case LOK_CALLBACK_CORE_LOG:
        return "LOK_CALLBACK_CORE_LOG";
    case LOK_CALLBACK_TOOLTIP:
        return "LOK_CALLBACK_TOOLTIP";
    }

    assert(!"Unknown LibreOfficeKitCallbackType type.");
    return nullptr;
}

typedef enum
{
    /// A mouse button has been pressed down.
    LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
    /// A mouse button has been let go.
    LOK_MOUSEEVENT_MOUSEBUTTONUP,
    /// The mouse has moved while a button is pressed.
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
