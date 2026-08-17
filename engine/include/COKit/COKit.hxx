/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/**
 * A clipboard backend the app registers to do the raw platform clipboard
 * input and output, in both directions. The engine drives the format
 * decision: on a paste it asks for the list of available types and then for the
 * bytes of the single format it chose; on a copy it hands over the list of
 * formats it offers so the app can advertise them to the platform clipboard.
 *
 * Every callback runs synchronously on the thread that runs the document's UNO
 * commands. In the in-process native apps that is the app's main thread, so the
 * callbacks may touch the platform clipboard directly. A callback must not
 * re-enter the engine.
 *
 * @see COKitDocument::installClipboardProvider().
 */
struct COKitClipboardProvider
{
    /**
     * Copy: advertise the given nullptr-terminated list of mime types on the
     * platform clipboard without serializing any bytes. The app serves the
     * bytes later by pulling a single format from the engine through
     * getClipboard(). May be null.
     */
    void (*advertiseToPlatform)(const char** pMimeTypes);

    /**
     * Return true if the platform clipboard still holds the content the app
     * last advertised, false if some other source now owns it. When it still
     * holds ours, the engine pastes from its own in-memory copy (full
     * fidelity); when it does not, the engine reads the platform through the
     * calls below.
     */
    bool (*ownsClipboard)(void);

    /**
     * Paste: return a nullptr-terminated, malloc'd array of malloc'd mime-type
     * strings the platform clipboard currently offers. No bytes are read. The
     * engine takes ownership and frees each string and the array.
     */
    char** (*getMimeTypes)(void);

    /**
     * Paste: fetch the bytes for one mime type. On success set *pOutData to a
     * malloc'd buffer and *pOutSize to its length and return true; on failure
     * return false. The engine frees *pOutData.
     */
    bool (*getDataForMimeType)(const char* pMimeType, char** pOutData, size_t* pOutSize);
};

// getDocumentType is part of the API whether or not the unstable half is asked for, so the
// type it returns sits outside that guard.
enum class COKitDocumentType
{
  TEXT,
  SPREADSHEET,
  PRESENTATION,
  DRAWING,
  OTHER
};

enum class COKitPartMode
{
    SLIDES,
    NOTES
};

enum class COKitTileMode
{
    RGBA,
    BGRA
};

enum class COKitWindowAction
{
    CLOSE,
    PASTE
};

enum class COKitSelectionType
{
    NONE,
    TEXT,
    COMPLEX
};

/** Optional features of COKit, in particular callbacks that block
 *  COKit until the corresponding reply is received, which would
 *  deadlock if the client does not support the feature.
 *
 *  @see COKit::setOptionalFeatures().
 */
enum class COKitOptionalFeatures : unsigned long long
{
    NONE = 0,

    /**
     * Handle COKitCallbackType::DOCUMENT_PASSWORD by prompting the user
     * for a password.
     *
     * @see COKit::setDocumentPassword().
     */
    DOCUMENT_PASSWORD = (1ULL << 0),

    /**
     * Handle COKitCallbackType::DOCUMENT_PASSWORD_TO_MODIFY by prompting the user
     * for a password.
     *
     * @see COKit::setDocumentPassword().
     */
    DOCUMENT_PASSWORD_TO_MODIFY = (1ULL << 1),

    /**
     * Request to have the part number as an 5th value in the
     * COKitCallbackType::INVALIDATE_TILES payload.
     */
    PART_IN_INVALIDATION_CALLBACK = (1ULL << 2),

    /**
     * Enable range based header data
     */
    RANGE_HEADERS = (1ULL << 4),

    /**
     * Request to have the active view's Id as the 1st value in the
     * COKitCallbackType::INVALIDATE_VISIBLE_CURSOR payload.
     */
    VIEWID_IN_VISCURSOR_INVALIDATION_CALLBACK = (1ULL << 5)
};

/// The features are a set of flags, so they combine and are tested with the operators below.
inline COKitOptionalFeatures operator|(COKitOptionalFeatures a, COKitOptionalFeatures b)
{
    return static_cast<COKitOptionalFeatures>(static_cast<unsigned long long>(a)
                                              | static_cast<unsigned long long>(b));
}

inline COKitOptionalFeatures operator&(COKitOptionalFeatures a, COKitOptionalFeatures b)
{
    return static_cast<COKitOptionalFeatures>(static_cast<unsigned long long>(a)
                                              & static_cast<unsigned long long>(b));
}

inline COKitOptionalFeatures operator^(COKitOptionalFeatures a, COKitOptionalFeatures b)
{
    return static_cast<COKitOptionalFeatures>(static_cast<unsigned long long>(a)
                                              ^ static_cast<unsigned long long>(b));
}

inline COKitOptionalFeatures& operator|=(COKitOptionalFeatures& a, COKitOptionalFeatures b)
{
    a = a | b;
    return a;
}

// This enumerates the types of callbacks emitted to a COKit
// object's callback function or to a COKitDocument object's
// callback function. No callback type will be emitted to both. It is a
// bit unfortunate that the same enum contains both kinds of
// callbacks.

// TODO: We should really add some indication at the documentation for
// each enum value telling which type of callback it is.

enum class COKitCallbackType
{
    /**
     * Any tiles which are over the rectangle described in the payload are no
     * longer valid.
     *
     * Rectangle format: "x, y, width, height", where all numbers are document
     * coordinates, in twips. When all tiles are supposed to be dropped, the
     * format is the "EMPTY" string.
     *
     * @see COKitOptionalFeatures::PART_IN_INVALIDATION_CALLBACK.
     */
    INVALIDATE_TILES = 0,
    /**
     * The size and/or the position of the visible cursor changed.
     *
     * Old format is the same as COKitCallbackType::INVALIDATE_TILES.
     * New format is a JSON with 3 elements the 'viewId' element represented by
     * an integer value, a 'rectangle' element in the format "x, y, width, height",
     * and a 'misspelledWord' element represented by an integer value: '1' when
     * a misspelled word is at the cursor position, '0' when the word is
     * not misspelled.
     */
    INVALIDATE_VISIBLE_CURSOR = 1,
    /**
     * The list of rectangles representing the current text selection changed.
     *
     * List format is "rectangle1[; rectangle2[; ...]]" (without quotes and
     * brackets), where rectangleN has the same format as
     * COKitCallbackType::INVALIDATE_TILES. When there is no selection, an empty
     * string is provided.
     */
    TEXT_SELECTION = 2,
    /**
     * The position and size of the cursor rectangle at the text
     * selection start. It is used to draw the selection handles.
     *
     * This callback must be called prior to COKitCallbackType::TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as COKitCallbackType::INVALIDATE_TILES.
     */
    TEXT_SELECTION_START = 3,
    /**
     * The position and size of the cursor rectangle at the text
     * selection end. It is used to draw the selection handles.
     *
     * This callback must be called prior to COKitCallbackType::TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as COKitCallbackType::INVALIDATE_TILES.
     */
    TEXT_SELECTION_END = 4,
    /**
     * The blinking text cursor is now visible or not.
     *
     * Clients should assume that this is true initially and are expected to
     * hide the blinking cursor at the rectangle described by
     * COKitCallbackType::INVALIDATE_VISIBLE_CURSOR once it becomes false. Payload is
     * either the "true" or the "false" string.
     */
    CURSOR_VISIBLE = 5,
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
    GRAPHIC_SELECTION = 6,

    /**
     * User clicked on a hyperlink that should be handled by other
     * applications accordingly.
     */
    HYPERLINK_CLICKED = 7,

    /**
     * Emit state update to the client.
     * For example, when cursor is on bold text, this callback is triggered
     * with payload: ".uno:Bold=true"
     */
    STATE_CHANGED = 8,

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
    STATUS_INDICATOR_START = 9,

    /**
     * Sets the numeric value of the status indicator.
     * The payload should be a percentage, an integer between 0 and 100.
     */
    STATUS_INDICATOR_SET_VALUE = 10,

    /**
     * Ends the status indicator.
     *
     * Not necessarily ever emitted.
     */
    STATUS_INDICATOR_FINISH = 11,

    /**
     * No match was found for the search input
     */
    SEARCH_NOT_FOUND = 12,

    /**
     * Size of the document changed.
     *
     * Payload format is "width, height", i.e. clients get the new size without
     * having to do an explicit COKitDocument::getDocumentSize() call.
     *
     * A size change is always preceded by a series of
     * COKitCallbackType::INVALIDATE_TILES events invalidating any areas
     * need re-rendering to adapt.
     */
    DOCUMENT_SIZE_CHANGED = 13,

    /**
     * The current part number is changed.
     *
     * Payload is a single 0-based integer.
     */
    SET_PART = 14,

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
     *   pairs, in COKitCallbackType::SET_PART / COKitCallbackType::TEXT_SELECTION format.
     */
    SEARCH_RESULT_SELECTION = 15,

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
    UNO_COMMAND_RESULT = 16,

    /**
     * The size and/or the position of the cell cursor changed.
     *
     * Payload format: "x, y, width, height, column, row", where the first
     * 4 numbers are document coordinates, in twips, and the last 2 are table
     * coordinates starting from 0.
     * When the cursor is not shown the payload format is the "EMPTY" string.
     *
     * Rectangle format is the same as COKitCallbackType::INVALIDATE_TILES.
     */
    CELL_CURSOR = 17,

    /**
     * The current mouse pointer style.
     *
     * Payload is a css mouse pointer style.
     */
    MOUSE_POINTER = 18,

    /**
     * The text content of the formula bar in Calc.
     */
    CELL_FORMULA = 19,

    /**
     * Loading a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * COKit::setDocumentPassword().  The document cannot be loaded
     * without the password.
     */
    DOCUMENT_PASSWORD = 20,

    /**
     * Editing a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * COKit::setDocumentPassword().
     */
    DOCUMENT_PASSWORD_TO_MODIFY = 21,

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
    ERROR_REPORT = 22,

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
    CONTEXT_MENU = 23,

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
     * - viewId is a value returned earlier by COKitDocument::createView()
     * - rectangle uses the format of COKitCallbackType::INVALIDATE_VISIBLE_CURSOR
     */
    INVALIDATE_VIEW_CURSOR = 24,

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
     * - viewId is a value returned earlier by COKitDocument::createView()
     * - selection uses the format of COKitCallbackType::TEXT_SELECTION.
     */
    TEXT_VIEW_SELECTION = 25,

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
     * - viewId is a value returned earlier by COKitDocument::createView()
     * - rectangle uses the format of COKitCallbackType::CELL_CURSOR.
     */
    CELL_VIEW_CURSOR = 26,

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
     * - viewId is a value returned earlier by COKitDocument::createView()
     * - selection uses the format of COKitCallbackType::INVALIDATE_TILES.
     */
    GRAPHIC_VIEW_SELECTION = 27,

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
     * - viewId is a value returned earlier by COKitDocument::createView()
     * - visible uses the format of COKitCallbackType::CURSOR_VISIBLE.
     */
    VIEW_CURSOR_VISIBLE = 28,

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
     * - viewId is a value returned earlier by COKitDocument::createView()
     * - rectangle uses the format of COKitCallbackType::INVALIDATE_TILES.
     */
    VIEW_LOCK = 29,

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
     * COKitDocument::getCommandValues('.uno:AcceptTrackedChanges'), extra
     * fields:
     *
     * - 'action' is either 'Add' or 'Remove', depending on if this is an
     *   insertion into the table or a removal.
     */
    REDLINE_TABLE_SIZE_CHANGED = 30,

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
     * COKitDocument::getCommandValues('.uno:AcceptTrackedChanges'), extra
     * fields:
     *
     * - 'action' is 'Modify'.
     */
    REDLINE_TABLE_ENTRY_MODIFIED = 31,

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
     * COKitDocument::getCommandValues('.uno:ViewAnnotations'), extra
     * fields:
     *
     * - 'action' can be 'Add', 'Remove' or 'Modify' depending on whether
     *    comment has been added, removed or modified.
     */
    COMMENT = 32,

    /**
     * The column/row header is no more valid because of a column/row insertion
     * or a similar event. Clients must query a new column/row header set.
     *
     * The payload says if we are invalidating a row or column header. So,
     * payload values can be: "row", "column", "all".
     */
    INVALIDATE_HEADER = 33,
    /**
     * The text content of the address field in Calc. Eg: "A7"
     */
    CELL_ADDRESS = 34,
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
    RULER_UPDATE = 35,
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
    WINDOW = 36,

    /**
     * When for the current cell is defined a validity list we need to show
     * a drop down button in the form of a marker.
     *
     * The payload format is: "x, y, visible" where x, y are the current
     * cell cursor coordinates and visible is set to 0 or 1.
     */
    VALIDITY_LIST_BUTTON = 37,

    /**
     * Notification that the clipboard contents have changed.
     * Typically fired in response to copying to clipboard.
     *
     * Payload is optional. When payload is empty, Online gets string from selected text.
     * Payload format is JSON.
     * Example: { "mimeType": "text/plain", "content": "some content" }
     */
    CLIPBOARD_CHANGED = 38,

    /**
     * When the (editing) context changes - like the user switches from
     * editing textbox in Impress to editing a shape there.
     *
     * Payload is the application ID and context, delimited by space.
     * Eg. com.sun.star.presentation.PresentationDocument TextObject
     */
    CONTEXT_CHANGED = 39,

    /**
     * On-load notification of the document signature status.
     */
    SIGNATURE_STATUS = 40,

    /**
     * Profiling tracing information single string of multiple lines
     * containing <pid> <timestamp> and zone start/stop information
     */
    PROFILE_FRAME = 41,

    /**
     * The position and size of the cell selection area. It is used to
     * draw the selection handles for cells in Calc documents.
     *
     * Rectangle format is the same as COKitCallbackType::INVALIDATE_TILES.
     */
    CELL_SELECTION_AREA = 42,

    /**
     * The position and size of the cell auto fill area. It is used to
     * trigger auto fill functionality if that area is hit.
     *
     * Rectangle format is the same as COKitCallbackType::INVALIDATE_TILES.
     */
    CELL_AUTO_FILL_AREA = 43,

    /**
     * When the cursor is in a table or a table is selected in the
     * document, this sends the table's column and row border positions
     * to the client. If the payload is empty (empty JSON object), then
     * no table is currently selected or the cursor is not inside a table
     * cell.
     */
    TABLE_SELECTED = 44,

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
    REFERENCE_MARKS = 45,

    /**
     * Callback related to native dialogs generated in JavaScript from
     * the description.
     */
    JSDIALOG = 46,

    /**
     * Send the list of functions whose name starts with the characters entered
     * by the user in the formula input bar.
     */
    CALC_FUNCTION_LIST = 47,

    /**
     * Sends the tab stop list for the current of the current cursor position.
     */
    TAB_STOP_LIST = 48,

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
    FORM_FIELD_BUTTON = 49,

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
    INVALIDATE_SHEET_GEOMETRY = 50,

    /**
     * When for the current cell is defined an input help text.
     *
     * The payload format is JSON: { "title": "title text", "content": "content text" }
     */
    VALIDITY_INPUT_HELP = 51,

    /**
     * Indicates the document background color in the payload as a RGB hex string (RRGGBB).
     */
    DOCUMENT_BACKGROUND_COLOR = 52,

    /**
     * When a user tries to use command which is restricted for that user
     */
    COMMAND_BLOCKED = 53,

    /**
     * The position of the cell cursor jumped to.
     *
     * Payload format: "x, y, width, height, column, row", where the first
     * 4 numbers are document coordinates, in twips, and the last 2 are table
     * coordinates starting from 0.
     * When the cursor is not shown the payload format is the "EMPTY" string.
     *
     * Rectangle format is the same as COKitCallbackType::INVALIDATE_TILES.
     */
    SC_FOLLOW_JUMP = 54,

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
    CONTENT_CONTROL = 55,

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
    PRINT_RANGES = 56,

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
    FONTS_MISSING = 57,

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
    MEDIA_SHAPE = 58,

    /**
     * The document is available to download by the client.
     *
     * Payload example:
     * "file:///tmp/hello-world.pdf"
     */
    EXPORT_FILE = 59,

    /**
     * Some attribute of this view has changed, that will cause it
     * to completely re-render, eg. non-printing characters or
     * or dark mode was toggled, and then distinct from other views.
     *
     * Payload is an opaque string that matches this set of states.
     * this will be emitted after creating a new view.
     */
    VIEW_RENDER_STATE = 60,

    /**
     * Informs the COKit client that the background color surrounding
     * the document has changed.
    */
   APPLICATION_BACKGROUND_COLOR = 61,

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
    A11Y_FOCUS_CHANGED = 62,

    /**
     * Accessibility event: text cursor position has changed.
     *
     *  {
     *      "position": N
     *  }
     *  where N is the position of the text cursor inside the focused paragraph.
     */
    A11Y_CARET_CHANGED = 63,

    /**
     * Accessibility event: text selection has changed.
     *
     *  {
     *      "start": N1
     *      "end": N2
     *  }
     *  where [N1,N2] is the range of the text selection inside the focused paragraph.
     */
    A11Y_TEXT_SELECTION_CHANGED = 64,

    /**
     * Informs the COKit client that the color palettes have changed.
    */
    COLOR_PALETTES = 65,

    /**
     * Informs that the document password has been successfully changed.
     * The payload contains the new password and the type.
    */
    DOCUMENT_PASSWORD_RESET = 66,

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
     *           <same structure as for COKitCallbackType::A11Y_FOCUS_CHANGED>
     *        }
     *   }
     *   where row/column indexes start from 0, inList is the list of tables
     *   the user got in from the outer to the inner; row/column span default
     *   value is 1; paragraph is the cell text content.
     */
    A11Y_FOCUSED_CELL_CHANGED = 67,

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
    A11Y_EDITING_IN_SELECTION_STATE = 68,

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
    A11Y_SELECTION_CHANGED = 69,

    /**
     * Forwarding logs from engine to client can be useful
     * for keep track of the real core/client event sequence
     *
     * Payload is the log to be sent
     */
    CORE_LOG = 70,

    /**
     * Tooltips shown in the documents, like redline author and date.
     *
     *  {
     *      "text": "text of tooltip",
     *      "rectangle": "x, y, width, height"
     *  }
     */
    TOOLTIP = 71,

    /**
     * Used for sending the rectangle for text inside a shape/textbox
     *
     *  Payload contains the rectangle details
     */
    SHAPE_INNER_TEXT = 72,
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
    VERTICAL_RULER_UPDATE = 73,

    /**
     * Advertises the MIME types currently held by the kit clipboard,
     * without any serialised content.
     *
     * Payload format is JSON.
     * Example: { "mimeTypes": ["text/plain;charset=utf-8", "image/png"] }
     */
    CLIPBOARD_MIMETYPES = 74,

    /**
     * Preview geometry while a shape handle is dragged.
     *
     * Sent in response to a MoveShapeHandle command in preview mode. The
     * document is not modified. The payload describes the geometry the
     * shape would get if the handle was dropped at the requested position.
     *
     * The payload format is:
     *
     * {
     *     "handle": "<handle number>",
     *     "polygons": ["x1,y1 x2,y2 ...", ...]
     * }
     *
     * The points are in twips, in document coordinates. An empty
     * "polygons" array means no preview is available for this handle.
     */
    SHAPE_DRAG_PREVIEW = 75,

    /**
     * A vector-primitives delta for a slide that changed.
     *
     * Pushed to a vector-rendering view so it does not have to request the
     * delta after an invalidation. The payload is the same JSON the
     * .uno:VectorPrimitives command returns for a delta: a vectorprimitivesdelta
     * with the part, the version, the object order, the changed objects and,
     * when the master page changed, its content.
     */
    VECTOR_PRIMITIVES_DELTA = 76,

    /**
     * The presentation info of an Impress or Draw document changed and
     * should be re-sent.
     *
     * Pushed to every view of the document when a slide's content changes
     * (a shape is added, moved, resized or deleted) or the set or order of
     * pages changes. The payload is a JSON object naming the reason and the
     * changed part, for example { "reason": "gifupdate", "part": 3 }.
     */
    PRESENTATION_INFO = 77
};

enum class COKitKeyEventType
{
    /// A key on the keyboard is pressed.
    DOWN,
    /// A key on the keyboard is released.
    UP
};

enum class COKitExtTextInputType
{
    /// cf. SalEvent::ExtTextInput
    TEXTINPUT,
    /// cf. SalEvent::ExtTextInputPos
    TEXTINPUT_POS,
    /// cf. SalEvent::EndExtTextInput
    TEXTINPUT_END
};

/// Returns the string representation of a COKitCallbackType enumeration element.
static inline const char* kitCallbackTypeToString(COKitCallbackType eType)
{
    switch (eType)
    {
    case COKitCallbackType::INVALIDATE_TILES:
        return "KIT_CALLBACK_INVALIDATE_TILES";
    case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
        return "KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR";
    case COKitCallbackType::TEXT_SELECTION:
        return "KIT_CALLBACK_TEXT_SELECTION";
    case COKitCallbackType::TEXT_SELECTION_START:
        return "KIT_CALLBACK_TEXT_SELECTION_START";
    case COKitCallbackType::TEXT_SELECTION_END:
        return "KIT_CALLBACK_TEXT_SELECTION_END";
    case COKitCallbackType::CURSOR_VISIBLE:
        return "KIT_CALLBACK_CURSOR_VISIBLE";
    case COKitCallbackType::VIEW_CURSOR_VISIBLE:
        return "KIT_CALLBACK_VIEW_CURSOR_VISIBLE";
    case COKitCallbackType::GRAPHIC_SELECTION:
        return "KIT_CALLBACK_GRAPHIC_SELECTION";
    case COKitCallbackType::GRAPHIC_VIEW_SELECTION:
        return "KIT_CALLBACK_GRAPHIC_VIEW_SELECTION";
    case COKitCallbackType::CELL_CURSOR:
        return "KIT_CALLBACK_CELL_CURSOR";
    case COKitCallbackType::HYPERLINK_CLICKED:
        return "KIT_CALLBACK_HYPERLINK_CLICKED";
    case COKitCallbackType::MOUSE_POINTER:
        return "KIT_CALLBACK_MOUSE_POINTER";
    case COKitCallbackType::STATE_CHANGED:
        return "KIT_CALLBACK_STATE_CHANGED";
    case COKitCallbackType::STATUS_INDICATOR_START:
        return "KIT_CALLBACK_STATUS_INDICATOR_START";
    case COKitCallbackType::STATUS_INDICATOR_SET_VALUE:
        return "KIT_CALLBACK_STATUS_INDICATOR_SET_VALUE";
    case COKitCallbackType::STATUS_INDICATOR_FINISH:
        return "KIT_CALLBACK_STATUS_INDICATOR_FINISH";
    case COKitCallbackType::SEARCH_NOT_FOUND:
        return "KIT_CALLBACK_SEARCH_NOT_FOUND";
    case COKitCallbackType::DOCUMENT_SIZE_CHANGED:
        return "KIT_CALLBACK_DOCUMENT_SIZE_CHANGED";
    case COKitCallbackType::SET_PART:
        return "KIT_CALLBACK_SET_PART";
    case COKitCallbackType::SEARCH_RESULT_SELECTION:
        return "KIT_CALLBACK_SEARCH_RESULT_SELECTION";
    case COKitCallbackType::DOCUMENT_PASSWORD:
        return "KIT_CALLBACK_DOCUMENT_PASSWORD";
    case COKitCallbackType::DOCUMENT_PASSWORD_TO_MODIFY:
        return "KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY";
    case COKitCallbackType::CONTEXT_MENU:
        return "KIT_CALLBACK_CONTEXT_MENU";
    case COKitCallbackType::INVALIDATE_VIEW_CURSOR:
        return "KIT_CALLBACK_INVALIDATE_VIEW_CURSOR";
    case COKitCallbackType::TEXT_VIEW_SELECTION:
        return "KIT_CALLBACK_TEXT_VIEW_SELECTION";
    case COKitCallbackType::CELL_VIEW_CURSOR:
        return "KIT_CALLBACK_CELL_VIEW_CURSOR";
    case COKitCallbackType::CELL_ADDRESS:
        return "KIT_CALLBACK_CELL_ADDRESS";
    case COKitCallbackType::CELL_FORMULA:
        return "KIT_CALLBACK_CELL_FORMULA";
    case COKitCallbackType::UNO_COMMAND_RESULT:
        return "KIT_CALLBACK_UNO_COMMAND_RESULT";
    case COKitCallbackType::ERROR_REPORT:
        return "KIT_CALLBACK_ERROR";
    case COKitCallbackType::VIEW_LOCK:
        return "KIT_CALLBACK_VIEW_LOCK";
    case COKitCallbackType::REDLINE_TABLE_SIZE_CHANGED:
        return "KIT_CALLBACK_REDLINE_TABLE_SIZE_CHANGED";
    case COKitCallbackType::REDLINE_TABLE_ENTRY_MODIFIED:
        return "KIT_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED";
    case COKitCallbackType::INVALIDATE_HEADER:
        return "KIT_CALLBACK_INVALIDATE_HEADER";
    case COKitCallbackType::COMMENT:
        return "KIT_CALLBACK_COMMENT";
    case COKitCallbackType::RULER_UPDATE:
        return "KIT_CALLBACK_RULER_UPDATE";
    case COKitCallbackType::VERTICAL_RULER_UPDATE:
        return "KIT_CALLBACK_VERTICAL_RULER_UPDATE";
    case COKitCallbackType::WINDOW:
        return "KIT_CALLBACK_WINDOW";
    case COKitCallbackType::VALIDITY_LIST_BUTTON:
        return "KIT_CALLBACK_VALIDITY_LIST_BUTTON";
    case COKitCallbackType::VALIDITY_INPUT_HELP:
        return "KIT_CALLBACK_VALIDITY_INPUT_HELP";
    case COKitCallbackType::CLIPBOARD_CHANGED:
        return "KIT_CALLBACK_CLIPBOARD_CHANGED";
    case COKitCallbackType::CONTEXT_CHANGED:
        return "KIT_CALLBACK_CONTEXT_CHANGED";
    case COKitCallbackType::SIGNATURE_STATUS:
        return "KIT_CALLBACK_SIGNATURE_STATUS";
    case COKitCallbackType::PROFILE_FRAME:
        return "KIT_CALLBACK_PROFILE_FRAME";
    case COKitCallbackType::CELL_SELECTION_AREA:
        return "KIT_CALLBACK_CELL_SELECTION_AREA";
    case COKitCallbackType::CELL_AUTO_FILL_AREA:
        return "KIT_CALLBACK_CELL_AUTO_FILL_AREA";
    case COKitCallbackType::TABLE_SELECTED:
        return "KIT_CALLBACK_TABLE_SELECTED";
    case COKitCallbackType::REFERENCE_MARKS:
        return "KIT_CALLBACK_REFERENCE_MARKS";
    case COKitCallbackType::JSDIALOG:
        return "KIT_CALLBACK_JSDIALOG";
    case COKitCallbackType::CALC_FUNCTION_LIST:
        return "KIT_CALLBACK_CALC_FUNCTION_LIST";
    case COKitCallbackType::TAB_STOP_LIST:
        return "KIT_CALLBACK_TAB_STOP_LIST";
    case COKitCallbackType::FORM_FIELD_BUTTON:
        return "KIT_CALLBACK_FORM_FIELD_BUTTON";
    case COKitCallbackType::INVALIDATE_SHEET_GEOMETRY:
        return "KIT_CALLBACK_INVALIDATE_SHEET_GEOMETRY";
    case COKitCallbackType::DOCUMENT_BACKGROUND_COLOR:
        return "KIT_CALLBACK_DOCUMENT_BACKGROUND_COLOR";
    case COKitCallbackType::COMMAND_BLOCKED:
        return "KIT_COMMAND_BLOCKED";
    case COKitCallbackType::SC_FOLLOW_JUMP:
        return "KIT_CALLBACK_SC_FOLLOW_JUMP";
    case COKitCallbackType::CONTENT_CONTROL:
        return "KIT_CALLBACK_CONTENT_CONTROL";
    case COKitCallbackType::PRINT_RANGES:
        return "KIT_CALLBACK_PRINT_RANGES";
    case COKitCallbackType::FONTS_MISSING:
        return "KIT_CALLBACK_FONTS_MISSING";
    case COKitCallbackType::MEDIA_SHAPE:
        return "KIT_CALLBACK_MEDIA_SHAPE";
    case COKitCallbackType::EXPORT_FILE:
        return "KIT_CALLBACK_EXPORT_FILE";
    case COKitCallbackType::VIEW_RENDER_STATE:
        return "KIT_CALLBACK_VIEW_RENDER_STATE";
    case COKitCallbackType::APPLICATION_BACKGROUND_COLOR:
        return "KIT_CALLBACK_APPLICATION_BACKGROUND_COLOR";
    case COKitCallbackType::A11Y_FOCUS_CHANGED:
        return "KIT_CALLBACK_A11Y_FOCUS_CHANGED";
    case COKitCallbackType::A11Y_CARET_CHANGED:
        return "KIT_CALLBACK_A11Y_CARET_CHANGED";
    case COKitCallbackType::A11Y_TEXT_SELECTION_CHANGED:
        return "KIT_CALLBACK_A11Y_TEXT_SELECTION_CHANGED";
    case COKitCallbackType::COLOR_PALETTES:
        return "KIT_CALLBACK_COLOR_PALETTES";
    case COKitCallbackType::DOCUMENT_PASSWORD_RESET:
        return "KIT_CALLBACK_DOCUMENT_PASSWORD_RESET";
    case COKitCallbackType::A11Y_FOCUSED_CELL_CHANGED:
        return "KIT_CALLBACK_A11Y_FOCUSED_CELL_CHANGED";
    case COKitCallbackType::A11Y_EDITING_IN_SELECTION_STATE:
        return "KIT_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE";
    case COKitCallbackType::A11Y_SELECTION_CHANGED:
        return "KIT_CALLBACK_A11Y_SELECTION_CHANGED";
    case COKitCallbackType::CORE_LOG:
        return "KIT_CALLBACK_CORE_LOG";
    case COKitCallbackType::TOOLTIP:
        return "KIT_CALLBACK_TOOLTIP";
    case COKitCallbackType::SHAPE_INNER_TEXT:
        return "KIT_CALLBACK_SHAPE_INNER_TEXT";
    case COKitCallbackType::CLIPBOARD_MIMETYPES:
        return "KIT_CALLBACK_CLIPBOARD_MIMETYPES";
    case COKitCallbackType::SHAPE_DRAG_PREVIEW:
        return "KIT_CALLBACK_SHAPE_DRAG_PREVIEW";
    case COKitCallbackType::VECTOR_PRIMITIVES_DELTA:
        return "KIT_CALLBACK_VECTOR_PRIMITIVES_DELTA";
    case COKitCallbackType::PRESENTATION_INFO:
        return "KIT_CALLBACK_PRESENTATION_INFO";
    }

    assert(!"Unknown COKitCallbackType type.");
    return nullptr;
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& rStream, COKitCallbackType eType)
{
    if (const char* pName = kitCallbackTypeToString(eType))
        return rStream << pName;
    return rStream << static_cast<int>(eType);
}

enum class COKitMouseEventType
{
    /// A mouse button has been pressed down.
    BUTTONDOWN,
    /// A mouse button has been let go.
    BUTTONUP,
    /// The mouse has moved while a button is pressed.
    MOVE
};

enum class COKitSetTextSelectionType
{
    /// The start of selection is to be adjusted.
    START,
    /// The end of selection is to be adjusted.
    END,
    /// Both the start and the end of selection is to be adjusted.
    RESET
};

enum class COKitSetGraphicSelectionType
{
    /**
     * A move or a resize action starts. It is assumed that there is a valid
     * graphic selection (see COKitCallbackType::GRAPHIC_SELECTION) and the supplied
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
    START,
    /**
     * A move or resize action stops. It is assumed that this is always used
     * only after a START. The supplied coordinates are
     * the ones where the user released the screen.
     */
    END
};

/** @see COKit::registerCallback(). */
typedef void (*COKitCallback)(COKitCallbackType eType, const char* pPayload, void* pData);

/** @see COKit::runLoop(). */
typedef int (*COKitPollCallback)(void* pData, int timeoutUs);

typedef void (*COKitWakeCallback)(void* pData);

/// @see COKit::registerAnyInputCallback()
typedef bool (*COKitAnyInputCallback)(void* pData, int nMostUrgentPriority);

/// @see COKit::registerFileSaveDialogCallback()
typedef void (*COKitFileSaveDialogCallback)(const char* pSuggestedUri, char* pResultUri,
                                            size_t nResultUri);

/// @see COKit::registerRevealInFileManagerCallback()
typedef void (*COKitRevealInFileManagerCallback)(const char* pUri);

struct COKitDocument;

struct COKit
{
    virtual ~COKit() = default;

    /**
     * Loads a document from a URL.
     *
     * @param pURL the URL of the document to load
     */
    virtual COKitDocument* documentLoad(const char* pURL) = 0;

    /// Returns the last error as a string.
    virtual std::string getError() = 0;

    virtual COKitDocument* documentLoadWithOptions(const char* pURL, const char* pOptions) = 0;

    /**
     * Registers a callback. COKit will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    virtual void registerCallback(COKitCallback pCallback, void* pData) = 0;

    /**
     * Set bitmask of optional features supported by the client.
     *
     * @see COKitOptionalFeatures
     */
    virtual void setOptionalFeatures(COKitOptionalFeatures features) = 0;

    /**
     * Set password required for loading or editing a document.
     *
     * Loading the document is blocked until the password is provided.
     *
     * @param pURL      the URL of the document, as sent to the callback
     * @param pPassword the password, nullptr indicates no password
     *
     * In response to COKitCallbackType::DOCUMENT_PASSWORD, a valid password
     * will continue loading the document, an invalid password will
     * result in another COKitCallbackType::DOCUMENT_PASSWORD request,
     * and a NULL password will abort loading the document.
     *
     * In response to COKitCallbackType::DOCUMENT_PASSWORD_TO_MODIFY, a valid
     * password will continue loading the document, an invalid password will
     * result in another COKitCallbackType::DOCUMENT_PASSWORD_TO_MODIFY request,
     * and a NULL password will continue loading the document in read-only
     * mode.
     */
    virtual void setDocumentPassword(char const* pURL, char const* pPassword) = 0;

    /**
     * Get version information of the COKit process
     *
     * @returns JSON string containing version information in format:
     * {ProductName: <>, ProductVersion: <>, ProductExtension: <>, BuildId: <>}
     *
     * Eg: {"ProductName": "CollaboraOffice",
     * "ProductVersion": "26.4",
     * "ProductExtension": ".0.0.alpha0",
     * "BuildId": "<full 40 char git hash>"}
     */
    virtual char* getVersionInfo() = 0;

    /**
     * Run a macro.
     *
     * Same syntax as on command line is permissible (ie. the macro:// URI forms)
     *
     * @param pURL macro url to run
     * @returns true when the macro ran.
     */
    virtual bool runMacro(const char* pURL) = 0;

    /**
     * Exports the document and signs its content.
     */
    virtual bool signDocument(const char* pUrl, const unsigned char* pCertificateBinary,
                               const int nCertificateBinarySize,
                               const unsigned char* pPrivateKeyBinary,
                               const int nPrivateKeyBinarySize) = 0;

    /**
     * Runs the main-loop in the current thread. To trigger this
     * mode you need to putenv a SAL_KIT_OPTIONS containing 'unipoll'.
     * The @pPollCallback is called to poll for events from the Kit client
     * and the @pWakeCallback can be called by internal COKit threads
     * to wake the caller of 'runLoop' ie. the main thread.
     *
     * it is expected that runLoop does not return until Kit exit.
     *
     * @pData is a context/closure passed to both methods.
     */
    virtual void runLoop(COKitPollCallback pPollCallback, COKitWakeCallback pWakeCallback,
                         void* pData) = 0;

    /**
     * Posts a dialog event for the window with given id
     *
     * @param nWindowId id of the window to notify
     * @param pArguments arguments of the event.
     */
    virtual void sendDialogEvent(unsigned long long int nWindowId, const char* pArguments) = 0;

    /**
     * Generic function to toggle and tweak various things in engine
     *
     * The currently available option names and their allowed values are:
     *
     * "profilezonerecording": "start" or "stop"
     * Start or stop recording profile zone trace data in the process.
     *
     * "sallogoverride": "<string>"
     * Override the SAL_LOG environment variable
     *
     * For the syntax of the string see the documentation for "Basic
     * logging functionality" in the internal API documentation
     * (include/sal/log.hxx). If the logging selector has been set by this
     * function to a non-empty value, that is used instead of the environment
     * variable SAL_LOG.
     *
     * The parameter is not copied so you should pass a value that
     * points to memory that will stay valid until you call setOption
     * with this option name the next time.
     *
     * If you pass nullptr or an empty string as value, the
     * environment variable SAL_LOG is again used as by default. You
     * can switch back and forth as you like.
     *
     * "addfont": "<string>"
     *
     * Adds the font at the URL given.
     *
     * @param pOption the option name
     * @param pValue its value
     */
    virtual void setOption(const char* pOption, const char* pValue) = 0;

    /**
     * Debugging tool for triggering a dump of internal state.
     *
     * COKit can get into an unhelpful state at run-time when
     * in heavy use. This provides a critical tool for inspecting
     * relevant internal state.
     *
     * @param pOptions future expansion - string options.
     * @param pState - heap allocated, C string containing the state dump.
     */
    virtual void dumpState(const char* pOptions, char** pState) = 0;

    virtual char* extractRequest(const char* pFilePath) = 0;

    /**
     * Trim memory usage.
     *
     * COKit caches lots of information from large pixmaps
     * to view and calculation results. When a view has not been
     * used for some time, depending on the load on memory it can
     * be useful to free up memory.
     *
     * @param nTarget - a negative number means the app is back
     * in active use, and to re-fill caches, a large positive
     * number (>=1000) encourages immediate maximum memory saving.
     */
    virtual void trimMemory(int nTarget) = 0;

    /**
     * Start a UNO acceptor using the function pointers provides to read and write data to/from the acceptor.
     *
     * @param pReceiveURPFromEngineContext A pointer that will be passed to your fnRecieveURPFromEngine function
     * @param pSendURPToEngineContext A pointer that will be passed to your fnSendURPToEngine function
     * @param fnReceiveURPFromEngine A function pointer that engine ushould use to pass URP back to the caller
     * @param fnSendURPToEngine A function pointer pointer that the caller should use to pass URP to engine
     */
    virtual void* startURP(
        void* pReceiveURPFromEngineContext, void* pSendURPToEngineContext,
        int (*fnReceiveURPFromEngine)(void* pContext, const signed char* pBuffer, int nLen),
        int (*fnSendURPToEngine)(void* pContext, signed char* pBuffer, int nLen)) = 0;

    /**
     * Stop a function based URP connection you previously started with startURP
     *
     * @param pSendURPToEngineContext the context returned by startURP  when starting the connection
     */
    virtual void stopURP(void* pSendURPToEngineContext) = 0;

    /**
     * Joins all threads if possible to get down to a single process
     * which can be forked from safely.
     *
     * @returns true when the join succeeded.
     */
    virtual bool joinThreads() = 0;

    /**
     * Starts all threads that are necessary to continue working
     * after a joinThreads().
     */
    virtual void startThreads() = 0;

    /**
     * Informs that this process is either a parent, or a child
     * process post-fork, allowing improved resource sharing.
     */
    virtual void setForkedChild(bool bIsChild) = 0;

    virtual char* extractDocumentStructureRequest(const char* pFilePath, const char* pFilter) = 0;

    /**
     * Registers a callback that can determine if there are any pending input events.
     */
    virtual void registerAnyInputCallback(COKitAnyInputCallback pCallback, void* pData) = 0;

    /**
     * Get number of documents of this COKit.
     */
    virtual int getDocsCount() = 0;

    /**
     * Registers a callback that can display an interactive file save dialog.
     */
    virtual void registerFileSaveDialogCallback(COKitFileSaveDialogCallback pCallback) = 0;

    /**
     * Execute a JavaScript snippet via the embedded JS UNO support.
     *
     * On success, @c *result is set to the script's last expression result, JSON-stringified (or
     * null if it stringifies to nothing, e.g. `undefined`), and @c *error is set to null.  On
     * error, @c *result is null and @c *error holds the JS exception message.
     *
     * The caller takes ownership of @c *result and @c *error and must @c free() them.
     *
     * The @c script, @c *result and @c *error strings are NUL-terminated C strings, thus cannot
     * contain embedded NUL characters.
     *
     * @c proxyCallback, if non-null, is captured by every JS-UNO proxy listener stub created
     * during this call, and fires when the stub later receives a UNO call.  It is called
     * with the @c proxyCallbackData pointer and a NUL-terminated JSON payload describing the
     * call (see jsuno::execute).  The callback may fire synchronously while @c script runs,
     * or later from any thread for as long as the proxy is registered.  Each proxy keeps the
     * callback it captured at creation time, so a later executeScript with a different
     * callback only affects proxies created by that later call.
     *
     * @param script the script source.
     * @param result out-param for the result.
     * @param error out-param for the error message.
     * @param proxyCallback hook for proxy listener fires; may be null.
     * @param proxyCallbackData opaque pointer passed to @c proxyCallback on each call.
     * @param usedLegacyUnoApi must be non-null; set to true if the script touched the legacy
     *        com.sun.star UNO API, not modified otherwise.
     */
    virtual void executeScript(char const * script, char ** result, char ** error,
                               void (*proxyCallback) (void * data, char const * payload),
                               void * proxyCallbackData, bool * usedLegacyUnoApi) = 0;

    /**
     * Deliver the iframe-side response value back to a JS-UNO proxy listener whose
     * `invoke` is currently waiting (synchronous return-value path).  @c callId matches a callId
     * that was previously sent in the proxyCallback payload.  @c jsonValue is the JSON
     * encoding of the JS-side return value, which the proxy will decode to the listener
     * method's declared return type.
     *
     * Both strings are NUL-terminated C strings and must not contain embedded U+0000.
     *
     * Spurious callIds (no matching pending invoke) are silently ignored.
     *
     * @param callId opaque token matching one previously delivered to proxyCallback.
     * @param jsonValue JSON-encoded return value for the listener method.
     */
    virtual void deliverProxyResult(char const * callId, char const * jsonValue) = 0;

    /**
     * Cancel all in-flight JS-UNO proxy listener calls by unblocking any
     * ProxyInvocation::invoke currently waiting in Application::Yield, treating each pending
     * call as if the iframe had returned an empty value.  Intended to be called from
     * ChildSession destruction so the kit's main thread can't end up spinning on a
     * synchronous proxy result that will never come.
     */
    virtual void cancelProxyCalls() = 0;

    /**
     * Whether the current thread is inside a window where it has explicitly opted into a kitPoll
     * re-entry (via vcl::kit::pushExpectedReentry).  The host poll loop should suppress its
     * non-async-dialog warning while this is true.
     *
     * @return true if a re-entry is expected.
     */
    virtual bool isExpectedReentry() = 0;

    /**
     * Returns and clears the process-wide "legacy UNO API use" flag set by the engine's UNO bridges
     * (Basic, Python, ...) when at runtime a script resolves an identifer in the legacy UNO API.
     *
     * @return true if at least one legacy identifier was resolved since the last call; false
     *         otherwise.
     */
    virtual bool takeLegacyUnoApiUseFlag() = 0;

    /**
     * Registers a callback that reveals (and selects) a file in the native file manager.
     */
    virtual void
    registerRevealInFileManagerCallback(COKitRevealInFileManagerCallback pCallback) = 0;

    /**
     * Installs a process-global clipboard provider and switches the kit to a
     * single shared clipboard for every view and document. Use this in the
     * in-process desktop app, where there is one local user and one platform
     * clipboard, so the clipboard survives closing an individual document. Pass
     * nullptr to remove the provider and return to the default per-view
     * clipboards (as used by the collaborative server).
     */
    virtual void installClipboardProvider(const COKitClipboardProvider* pProvider) = 0;

    /**
     * Read the desktop app's single process-wide clipboard. See
     * Document::getClipboard() for the parameters; this needs no document
     * because the shared clipboard is process-global. The distinct name marks
     * that it reads one global clipboard, not a per-view one.
     */
    virtual bool getGlobalClipboard(const char **pMimeTypes, size_t      *pOutCount,
                                    char      ***pOutMimeTypes, size_t     **pOutSizes,
                                    char      ***pOutStreams) = 0;
};

struct COKitDocument
{
    virtual ~COKitDocument() = default;

    /**
     * Stores the document's persistent data to a URL and
     * continues to be a representation of the old URL.
     *
     * @param pUrl the location where to store the document
     * @param pFormat the format to use while exporting, when omitted, then deducted from pURL's extension
     * @param pFilterOptions options for the export filter, e.g. SkipImages.
     *        Another useful FilterOption is "TakeOwnership".  It is consumed
     *        by the saveAs() itself, and when provided, the document identity
     *        changes to the provided pUrl - meaning that '.uno:ModifiedStatus'
     *        is triggered as with the "Save As..." in the UI.
     *        "TakeOwnership" mode must not be used when saving to PNG or PDF.
     */
    virtual bool saveAs(const char* pUrl, const char* pFormat, const char* pFilterOptions) = 0;

    /**
     * Get document type.
     *
     * @return an element of the COKitDocumentType enum.
     */
    virtual COKitDocumentType getDocumentType() = 0;

    /**
     * Get number of part that the document contains.
     *
     * Part refers to either individual sheets in a Calc, or slides in Impress,
     * and has no relevance for Writer.
     */
    virtual int getParts() = 0;

    /**
     * Get the extent of each page in the document.
     *
     * This function is relevant for Writer documents only. It is a
     * mistake that the API has "part" in its name as Writer documents
     * don't have parts.
     *
     * @return a rectangle list, using the same format as
     * COKitCallbackType::TEXT_SELECTION.
     */
    virtual std::string getWriterPageRectangles() = 0;

    /// Get the current part number of the document. For a presentation or
    /// drawing document a part number is the page's stable unique identifier;
    /// for other document types it is the part's index.
    virtual int getPart() = 0;

    /// Set the current part of the document by its part number. The part
    /// number of a page that is gone selects nothing.
    virtual void setPart(int nPart) = 0;

    /// Get the current part's name.
    virtual char* getPartName(int nPart) = 0;

    virtual void setPartMode(COKitPartMode eMode) = 0;

    /**
     * Renders a subset of the document to a pre-allocated buffer.
     *
     * Note that the buffer size and the tile size implicitly supports
     * rendering at different zoom levels, as the number of rendered pixels and
     * the rendered rectangle of the document are independent.
     *
     * @param pBuffer pointer to the buffer, its size is determined by nCanvasWidth and nCanvasHeight.
     * @param nCanvasWidth number of pixels in a row of pBuffer.
     * @param nCanvasHeight number of pixels in a column of pBuffer.
     * @param nTilePosX logical X position of the top left corner of the rendered rectangle, in TWIPs.
     * @param nTilePosY logical Y position of the top left corner of the rendered rectangle, in TWIPs.
     * @param nTileWidth logical width of the rendered rectangle, in TWIPs.
     * @param nTileHeight logical height of the rendered rectangle, in TWIPs.
     */
    virtual void paintTile(unsigned char* pBuffer, const int nCanvasWidth, const int nCanvasHeight,
                           const int nTilePosX, const int nTilePosY, const int nTileWidth,
                           const int nTileHeight) = 0;

    /**
     * Gets the tile mode: the pixel format used for the pBuffer of paintTile().
     *
     * @return the pixel order the document's tiles use.
     */
    virtual COKitTileMode getTileMode() = 0;

    /// Get the document sizes in TWIPs.
    virtual void getDocumentSize(long* pWidth, long* pHeight) = 0;

    /**
     * Initialize document for rendering.
     *
     * Sets the rendering and document parameters to default values that are
     * needed to render the document correctly using tiled rendering. This
     * method has to be called right after documentLoad() in case any of the
     * tiled rendering methods are to be used later.
     *
     * Example argument string for text documents:
     *
     * {
     *     ".uno:HideWhitespace":
     *     {
     *         "type": "boolean",
     *         "value": "true"
     *     }
     * }
     *
     * @param pArguments arguments of the rendering
     */
    virtual void initializeForRendering(const char* pArguments) = 0;

    /**
     * Registers a callback. COKit will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    virtual void registerCallback(COKitCallback pCallback, void* pData) = 0;

    /**
     * Posts a keyboard event to the focused frame.
     *
     * @param eType Event type, like press or release.
     * @param nCharCode contains the Unicode character generated by this event or 0
     * @param nKeyCode contains the integer code representing the key of the event (non-zero for control keys)
     */
    virtual void postKeyEvent(COKitKeyEventType eType, int nCharCode, int nKeyCode) = 0;

    /**
     * Posts a mouse event to the document.
     *
     * @param eType Event type, like down, move or up.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nCount number of clicks: 1 for single click, 2 for double click
     * @param nButtons which mouse buttons: 1 for left, 2 for middle, 4 right
     * @param nModifier which keyboard modifier: (see include/vcl/vclenum.hxx for possible values)
     */
    virtual void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount,
                                int nButtons, int nModifier) = 0;

    /**
     * Posts a UNO command to the document.
     *
     * Example argument string:
     *
     * {
     *     "SearchItem.SearchString":
     *     {
     *         "type": "string",
     *         "value": "foobar"
     *     },
     *     "SearchItem.Backward":
     *     {
     *         "type": "boolean",
     *         "value": "false"
     *     }
     * }
     *
     * @param pCommand uno command to be posted to the document, like ".uno:Bold"
     * @param pArguments arguments of the uno command.
     */
    virtual void postUnoCommand(const char* pCommand, const char* pArguments,
                                bool bNotifyWhenFinished) = 0;

    /**
     * Sets the start or end of a text selection.
     *
     * @param eType @see COKitSetTextSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    virtual void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) = 0;

    /**
     * Gets the currently selected text.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     * @param pUsedMimeType output parameter to inform about the determined format (suggested one or plain text).
     */
    virtual std::string getTextSelection(std::string_view pMimeType, std::string* pUsedMimeType) = 0;

    /**
     * Pastes content at the current cursor position.
     *
     * @param pMimeType format of pData, for example text/plain;charset=utf-8.
     * @param pData the actual data to be pasted.
     * @return if the supplied data was pasted successfully.
     */
    virtual bool paste(const char* pMimeType, const char* pData, size_t nSize) = 0;

    /**
     * Adjusts the graphic selection.
     *
     * @param eType @see COKitSetGraphicSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    virtual void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) = 0;

    /**
     * Gets rid of any text or graphic selection.
     */
    virtual void resetSelection() = 0;

    /**
     * Returns a json mapping of the possible values for the given command
     * e.g. {commandName: ".uno:StyleApply", commandValues: {"familyName1" : ["list of style names in the family1"], etc.}}
     * @param pCommand a UNO command for which the possible values are requested
     * @return {commandName: unoCmd, commandValues: {possible_values}}
     *
     * The caller owns the returned string and frees it.
     */
    virtual char* getCommandValues(const char* pCommand) = 0;

    /**
     * Save the client's view so that we can compute the right zoom level
     * for the mouse events. This only affects CALC.
     * @param nTilePixelWidth - tile width in pixels
     * @param nTilePixelHeight - tile height in pixels
     * @param nTileTwipWidth - tile width in twips
     * @param nTileTwipHeight - tile height in twips
     */
    virtual void setClientZoom(int nTilePixelWidth, int nTilePixelHeight, int nTileTwipWidth,
                               int nTileTwipHeight) = 0;

    /**
     * Inform engine about the currently visible area of the document on the
     * client, so that it can perform e.g. page down (which depends on the
     * visible height) in a sane way.
     *
     * @param nX - top left corner horizontal position
     * @param nY - top left corner vertical position
     * @param nWidth - area width
     * @param nHeight - area height
     */
    virtual void setClientVisibleArea(int nX, int nY, int nWidth, int nHeight) = 0;

    /**
     * Create a new view for an existing document. A loaded document has one view.
     * @return the ID of the new view.
     */
    virtual int createView() = 0;

    /**
     * Destroy a view of an existing document.
     * @param nId a view ID, returned by createView().
     */
    virtual void destroyView(int nId) = 0;
    /**
     * Set an existing view of an existing document as current.
     * @param nId a view ID, returned by createView().
     */
    virtual void setView(int nId) = 0;
    /**
     * Get the current view.
     * @return a view ID, previously returned by createView().
     */
    virtual int getView() = 0;
    /**
     * Get number of views of this document.
     */
    virtual int getViewsCount() = 0;

    /// Get the current part's hash.
    virtual char* getPartHash(int nPart) = 0;

    /**
     * Renders a subset of the document's part to a pre-allocated buffer.
     *
     * @param nPart the part number of the document of which the tile is painted. For a
     * presentation or drawing document that is the page's stable unique identifier, resolved to
     * the index the page holds when it paints; the part number of a page that is gone paints
     * nothing.
     * @see paintTile.
     */
    virtual void paintPartTile(unsigned char* pBuffer, const int nPart, const int nMode,
                               const int nCanvasWidth, const int nCanvasHeight,
                               const int nTilePosX, const int nTilePosY, const int nTileWidth,
                               const int nTileHeight) = 0;

    /**
     * Returns the viewID for each existing view. Since viewIDs are not reused,
     * viewIDs are not the same as the index of the view in the view array over
     * time. Use getViewsCount() to know the minimal nSize that's large enough.
     *
     * @param pArray the array to write the viewIDs into
     * @param nSize the size of pArray
     * @returns true if pArray was large enough and result is written, false
     * otherwise.
     */
    virtual bool getViewIds(int* pArray, size_t nSize) = 0;

    /**
     * Show/Hide a single row/column header outline for Calc documents.
     *
     * @param bColumn - if we are dealing with a column or row group
     * @param nLevel - the level to which the group belongs
     * @param nIndex - the group entry index
     * @param bHidden - the new group state (collapsed/expanded)
     */
    virtual void setOutlineState(bool bColumn, int nLevel, int nIndex, bool bHidden) = 0;

    /// Paints window with given id to the buffer
    virtual void paintWindow(unsigned nWindowId, unsigned char* pBuffer, const int x, const int y,
                             const int width, const int height) = 0;

    /**
     * Posts a command to the window (dialog, popup, etc.) with given id
     *
     * @param nWindowid
     */
    virtual void postWindow(unsigned nWindowId, COKitWindowAction eAction, const char* pData) = 0;

    /**
     * Posts a keyboard event to the dialog
     *
     * @param nWindowId id of window
     * @param eType Event type, like press or release.
     * @param nCharCode contains the Unicode character generated by this event or 0
     * @param nKeyCode contains the integer code representing the key of the event (non-zero for control keys)
     */
    virtual void postWindowKeyEvent(unsigned nWindowId, COKitKeyEventType eType, int nCharCode,
                                    int nKeyCode) = 0;

    /**
     * Posts a mouse event to the window with given id.
     *
     * @param nWindowId id of window
     * @param eType Event type, like down, move or up.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nCount number of clicks: 1 for single click, 2 for double click
     * @param nButtons which mouse buttons: 1 for left, 2 for middle, 4 right
     * @param nModifier which keyboard modifier: (see include/vcl/vclenum.hxx for possible values)
     */
    virtual void postWindowMouseEvent(unsigned nWindowId, COKitMouseEventType eType, int nX,
                                      int nY, int nCount, int nButtons, int nModifier) = 0;

    /**
     * Set the language tag of the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param language Bcp47 languageTag, like en-US or so.
     */
    virtual void setViewLanguage(int nId, const char* language) = 0;

    /**
     * Post the text input from external input window, like IME, to given windowId
     *
     * @param nWindowId Specify the window id to post the input event to. If
     * nWindow is 0, the event is posted into the document
     * @param eType which stage of the input method's composition this is
     * @param pText Text for COKitExtTextInputType::TEXTINPUT
     */
    virtual void postWindowExtTextInputEvent(unsigned nWindowId, COKitExtTextInputType eType,
                                             const char* pText) = 0;

    virtual char* getPartInfo(int nPart) = 0;

    /// Paints window with given id to the buffer with the give DPI scale
    /// (every pixel is dpiscale-times larger).
    /// @see COKitDocument::paintWindow().
    virtual void paintWindowDPI(unsigned nWindowId, unsigned char* pBuffer, const int x,
                                const int y, const int width, const int height,
                                const double dpiscale) = 0;

    /**
     *  Insert certificate (in binary form) to the certificate store.
     */
    virtual bool insertCertificate(const unsigned char* pCertificateBinary,
                                   const int nCertificateBinarySize,
                                   const unsigned char* pPrivateKeyBinary,
                                   const int nPrivateKeyBinarySize) = 0;

    /**
     *  Add the certificate (in binary form) to the certificate store.
     *
     */
    virtual bool addCertificate(const unsigned char* pCertificateBinary,
                                const int nCertificateBinarySize) = 0;

    /**
     *  Verify signature of the document.
     *
     *  Check possible values in include/sfx2/signaturestate.hxx
     */
    virtual int getSignatureState() = 0;

    /**
     * Gets an image of the selected shapes.
     * @param pOutput contains the result; use free to deallocate.
     * @return the size of *pOutput in bytes.
     */
    virtual size_t renderShapeSelection(char** pOutput) = 0;

    /**
     * Posts a gesture event to the window with given id.
     *
     * @param nWindowId id of window
     * @param pType Event type, like panStart, panEnd, panUpdate.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nOffset difference value from when the gesture started to current value
     */
    virtual void postWindowGestureEvent(unsigned nWindowId, const char* pType, int nX, int nY,
                                        int nOffset) = 0;

    /**
     * Create a new view for an existing document, with options in the same form
     * as documentLoadWithOptions() takes. A loaded document has one view.
     * @return the ID of the new view.
     */
    virtual int createViewWithOptions(const char* pOptions) = 0;

    /// Set a part's selection mode, naming the part by its part number.
    /// nSelect is 0 to deselect, 1 to select, and 2 to toggle.
    virtual void selectPart(int nPart, int nSelect) = 0;

    /// Moves the selected pages/slides to a new position.
    /// nPosition is the new position where the selection
    /// should go. bDuplicate when true will copy instead of move.
    /// nIntoSection: when >= 0, re-anchor that section to the first moved
    /// slide (slide becomes the section's new first slide).  Pass -1
    /// to keep the existing section anchoring.
    virtual void moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection) = 0;

    /**
     * Resize a window (dialog, popup, etc.) with give id.
     *
     * @param nWindowId id of window
     * @param width The width of the window.
     * @param height The height of the window.
     */
    virtual void resizeWindow(unsigned nWindowId, const int width, const int height) = 0;

    /**
     * Gets the content on the clipboard for the current view as a series of binary streams.
     *
     * NB. returns a complete set of possible selection types if nullptr is passed for pMimeTypes.
     *
     * @param pMimeTypes passes in a nullptr terminated list of mime types to fetch
     * @param pOutCount     returns the size of the other @pOut arrays
     * @param pOutMimeTypes returns an array of mime types
     * @param pOutSizes     returns the size of each pOutStream
     * @param pOutStreams   the content of each mime-type, of length in @pOutSizes
     *
     * @returns: true on success, false on error.
     */
    virtual bool getClipboard(const char **pMimeTypes, size_t      *pOutCount,
                              char      ***pOutMimeTypes, size_t     **pOutSizes,
                              char      ***pOutStreams) = 0;

    /**
     * Populates the clipboard for this view with multiple types of content.
     *
     * @param nInCount the number of types to paste
     * @param pInMimeTypes array of mime type strings
     * @param pInSizes array of sizes of the data to paste
     * @param pInStreams array containing the data of the various types
     *
     * @return if the supplied data was populated successfully.
     */
    virtual bool setClipboard(const size_t   nInCount, const char   **pInMimeTypes,
                              const size_t  *pInSizes, const char   **pInStreams) = 0;

    /**
     * Gets the type of the selected content.
     *
     * In most cases it is more efficient to use getSelectionTypeAndText().
     *
     * @return what kind of selection the document holds.
     */
    virtual COKitSelectionType getSelectionType() = 0;

    /**
     * For deleting many characters all at once
     *
     * @param nWindowId Specify the window id to post the input event to. If
     * nWindow is 0, the event is posted into the document
     * @param nBefore The characters to be deleted before the cursor position
     * @param nAfter The characters to be deleted after the cursor position
     */
    virtual void removeTextContext(unsigned nWindowId, int nBefore, int nAfter) = 0;

    /**
     * Posts a dialog event for the window with given id
     *
     * @param nWindowId id of the window to notify
     * @param pArguments arguments of the event.
     */
    virtual void sendDialogEvent(unsigned long long int nWindowId, const char* pArguments) = 0;

    /**
     * Paints a font name or character if provided to be displayed in the font list
     * @param pFontName the font to be painted
     */
    virtual unsigned char* renderFontOrientation(const char* pFontName, const char* pChar,
                                                 int* pFontWidth, int* pFontHeight,
                                                 int pOrientation) = 0;

    /**
     * Renders a window (dialog, popup, etc.) with the given id, switching to
     * viewId first when that is >= 0.
     *
     * @param pBuffer Buffer with enough memory allocated to render any dialog
     * @param x x-coordinate from where the dialog should start painting
     * @param y y-coordinate from where the dialog should start painting
     * @param width The width of the dialog image to be painted
     * @param height The height of the dialog image to be painted
     * @param dpiscale The dpi scale value used by the client. Please note
     *                 that the x, y, width, height are supposed to be the
     *                 values with dpiscale applied (ie. dialog covering
     *                 100x100 "normal" pixels with dpiscale '2' will have
     *                 200x200 width x height), so that it is easy to compute
     *                 the buffer sizes etc.
     */
    virtual void paintWindowForView(unsigned nWindowId, unsigned char* pBuffer, const int x,
                                    const int y, const int width, const int height,
                                    const double dpiscale, int viewId) = 0;

    /**
     * Select the Calc function to be pasted into the formula input box
     *
     * @param pFunctionName name of function
     */
    virtual void completeFunction(const char* pFunctionName) = 0;

    /**
     * Sets the start or end of a text selection for a dialog.
     *
     * @param nWindowId id of window
     * @param bSwap swap anchor and cursor position of current selection
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    virtual void setWindowTextSelection(unsigned nWindowId, bool bSwap, int nX, int nY) = 0;

    /**
     * Posts an event for the form field at the cursor position.
     *
     * @param pArguments arguments of the event.
     */
    virtual void sendFormFieldEvent(const char* pArguments) = 0;

    virtual void setBlockedCommandList(int nViewId, const char* blockedCommandList) = 0;

    /**
     * Render input search result to a bitmap buffer.
     *
     * @param pSearchResult payload containing the search result data
     * @param pBitmapBuffer contains the bitmap; use free to deallocate.
     * @param pWidth output bitmap width
     * @param pHeight output bitmap height
     * @param pByteSize output bitmap byte size
     * @return true if successful
     */
    virtual bool renderSearchResult(const char* pSearchResult, unsigned char** pBitmapBuffer,
                                    int* pWidth, int* pHeight, size_t* pByteSize) = 0;

    /**
     * Posts an event for the content control at the cursor position.
     *
     * @param pArguments arguments of the event.
     *
     * Examples:
     * To select the 3rd list item of the drop-down:
     * {
     *     "type": "drop-down",
     *     "selected": "2"
     * }
     *
     * To change a picture place-holder:
     * {
     *     "type": "picture",
     *     "changed": "file:///path/to/test.png"
     * }
     *
     * To select a date of the current date content control:
     * {
     *     "type": "date",
     *     "selected": "2022-05-29T00:00:00Z"
     * }
     */
    virtual void sendContentControlEvent(const char* pArguments) = 0;

    /**
     * Gets the type of the selected content and possibly its text.
     *
     * This function is a more efficient combination of getSelectionType() and getTextSelection().
     * It returns the same as getSelectionType(), and additionally if the return value is
     * COKitSelectionType::TEXT then it also returns the same as getTextSelection(), otherwise
     * pText and pUsedMimeType are unchanged.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     * @param pText the currently selected text
     * @param pUsedMimeType output parameter to inform about the determined format (suggested one or plain text).
     * @return what kind of selection the document holds.
     */
    virtual COKitSelectionType getSelectionTypeAndText(const char* pMimeType, char** pText,
                                                       char** pUsedMimeType) = 0;

    /// Get the data area (in Calc last row and column).
    virtual void getDataArea(long nPart, long* pCol, long* pRow) = 0;

    virtual int getEditMode() = 0;

    /**
     * Set the timezone of the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param pTimezone a timezone in the tzfile(5) format (e.g. Pacific/Auckland).
     */
    virtual void setViewTimezone(int nId, const char* pTimezone) = 0;

    /**
     * Enable/Disable accessibility support for the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param nEnabled true/false
     */
    virtual void setAccessibilityState(int nId, bool nEnabled) = 0;

    /**
     *  Get the current focused paragraph info:
     *  {
     *      "content": paragraph content
     *      "start": selection start
     *      "end": selection end
     *  }
     */
    virtual char* getA11yFocusedParagraph() = 0;

    /// Get the current text cursor position.
    virtual int getA11yCaretPosition() = 0;

    /** Set if the view should be treated as readonly or not.
     *
     * @param nId view ID
     * @param readOnly true if view readonly
    */
    virtual void setViewReadOnly(int nId, const bool readOnly) = 0;

    /** Set if the view can edit comments on readonly mode or not.
     *
     * @param nId view ID
     * @param allow true if comments allowed
    */
    virtual void setAllowChangeComments(int nId, const bool allow) = 0;

    /// Get the information about the current presentation (Impress only).
    virtual char* getPresentationInfo() = 0;

    /// Create a slide renderer in engine for the input slide.
    virtual bool createSlideRenderer(const char* pSlideHash, int nSlideNumber,
                                     unsigned* nViewWidth, unsigned* nViewHeight,
                                     bool bRenderBackground, bool bRenderMasterPage) = 0;

    /// Clean-up the slideshow (slide renderer)
    virtual void postSlideshowCleanup() = 0;

    /// Render the slide layer
    virtual bool renderNextSlideLayer(unsigned char* pBuffer, bool* bIsBitmapLayer, double* pScale,
                                      char** pJsonMessage) = 0;

    /// Set named view options
    virtual void setViewOption(const char* pOption, const char* pValue) = 0;

    /**
     * Set color preview state for the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param nEnabled true/false
     */
    virtual void setColorPreviewState(int nId, bool nEnabled) = 0;

    /** Set if the view can manage redlines in readonly mode or not.
     *
     * @param nId view ID
     * @param allow true for redlines allowed
    */
    virtual void setAllowManageRedlines(int nId, bool allow) = 0;

    /**
     * Shares another view's live clipboard transferable into the current view's
     * clipboard by reference, without serializing (same-process only). The caller
     * must have made the destination the current view first.
     *
     * @param nSourceViewId the view whose clipboard contents to share.
     */
    virtual void transferClipboardFromView(int nSourceViewId) = 0;

    /**
     * Renders every advertised clipboard format now, so the clipboard's
     * contents stay readable after this document is closed. Call it while the
     * document is still alive, when it produced the current clipboard content
     * and other documents remain open. A lazy transferable (Writer, Impress)
     * builds its own clip document; a self-contained one (Calc) is unaffected.
     */
    virtual void flushClipboard() = 0;

    /**
     * Get the stable unique identifier of one part: a nonzero integer assigned
     * to the part for the whole document session, kept over part moves,
     * insertions and deletions of other parts. nMode selects the part list the
     * index addresses: 0 for the standard parts, 1 for the master pages, 2 for
     * the notes pages. Zero when there is no such part or the document has no
     * part identifiers.
     */
    virtual unsigned long long getPartUniqueId(int nPart, int nMode) = 0;

    /**
     * Get the index the part with the given part number holds now. For a
     * presentation or drawing document a part number is the page's stable
     * unique identifier, and the result is the position of that page in the
     * part list nMode selects; -1 when no page carries that number any more.
     * For other document types the part number is the index itself.
     */
    virtual int getPartIndex(int nPart, int nMode) = 0;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
