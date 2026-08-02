/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stddef.h>
#include <assert.h>

#include <ostream>

// the API needs C99's bool
# ifndef _WIN32
#  include <stdbool.h>
# endif
# include <stdint.h>

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
 * @see kit::Document::installClipboardProvider().
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
     * Return 1 if the platform clipboard still holds the content the app last
     * advertised, 0 if some other source now owns it. When it still holds ours,
     * the engine pastes from its own in-memory copy (full fidelity); when it
     * does not, the engine reads the platform through the calls below.
     */
    int (*ownsClipboard)(void);

    /**
     * Paste: return a nullptr-terminated, malloc'd array of malloc'd mime-type
     * strings the platform clipboard currently offers. No bytes are read. The
     * engine takes ownership and frees each string and the array.
     */
    char** (*getMimeTypes)(void);

    /**
     * Paste: fetch the bytes for one mime type. On success set *pOutData to a
     * malloc'd buffer and *pOutSize to its length and return 1; on failure
     * return 0. The engine frees *pOutData.
     */
    int (*getDataForMimeType)(const char* pMimeType, char** pOutData, size_t* pOutSize);
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
 *  @see kit::Office::setOptionalFeatures().
 */
enum class COKitOptionalFeatures : unsigned long long
{
    NONE = 0,

    /**
     * Handle COKitCallbackType::DOCUMENT_PASSWORD by prompting the user
     * for a password.
     *
     * @see kit::Office::setDocumentPassword().
     */
    DOCUMENT_PASSWORD = (1ULL << 0),

    /**
     * Handle COKitCallbackType::DOCUMENT_PASSWORD_TO_MODIFY by prompting the user
     * for a password.
     *
     * @see kit::Office::setDocumentPassword().
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
     * having to do an explicit kit::Document::getDocumentSize() call.
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
     * kit::Office::setDocumentPassword().  The document cannot be loaded
     * without the password.
     */
    DOCUMENT_PASSWORD = 20,

    /**
     * Editing a document requires a password.
     *
     * Loading the document is blocked until the password is provided via
     * kit::Office::setDocumentPassword().
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
     * - viewId is a value returned earlier by kit::Document::createView()
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
     * - viewId is a value returned earlier by kit::Document::createView()
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
     * - viewId is a value returned earlier by kit::Document::createView()
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
     * - viewId is a value returned earlier by kit::Document::createView()
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
     * - viewId is a value returned earlier by kit::Document::createView()
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
     * - viewId is a value returned earlier by kit::Document::createView()
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
     * kit::Document::getCommandValues('.uno:AcceptTrackedChanges'), extra
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
     * kit::Document::getCommandValues('.uno:AcceptTrackedChanges'), extra
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
     * kit::Document::getCommandValues('.uno:ViewAnnotations'), extra
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
     * Forwarding logs from core to client can be useful
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
    VECTOR_PRIMITIVES_DELTA = 76
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

/** @see kit::Office::registerCallback(). */
typedef void (*COKitCallback)(COKitCallbackType eType, const char* pPayload, void* pData);

/** @see kit::Office::runLoop(). */
typedef int (*COKitPollCallback)(void* pData, int timeoutUs);

typedef void (*COKitWakeCallback)(void* pData);

/// @see kit::Office::registerAnyInputCallback()
typedef bool (*COKitAnyInputCallback)(void* pData, int nMostUrgentPriority);

/// @see kit::Office::registerFileSaveDialogCallback()
typedef void (*COKitFileSaveDialogCallback)(const char* pSuggestedUri, char* pResultUri,
                                            size_t nResultUri);

/// @see kit::Office::registerRevealInFileManagerCallback()
typedef void (*COKitRevealInFileManagerCallback)(const char* pUri);

struct COKitDocument;

struct COKit
{
    virtual ~COKit() = default;

    virtual void destroy() = 0;

    virtual COKitDocument* documentLoad(const char* pURL) = 0;

    virtual char* getError() = 0;

    virtual COKitDocument* documentLoadWithOptions(const char* pURL, const char* pOptions) = 0;

    /// The name "freeError" is a historical accident, actually this
    /// is a generic deallocation function for dynamically allocated
    /// memory returned by other COKit functions.

    /// Especially on Windows it is important to not call free() in
    /// your own code on a pointer returned from some random other
    /// dynamic library (like the one this code goes into) where it
    /// might have been allocated by calling malloc() (etc) in a C
    /// runtime library that is different from the one used by your
    /// code. That will lead to a crash. Alays call the free() in the
    /// same C runtime where the malloc() that allocated the pointer
    /// is.

    virtual void freeError(char* pFree) = 0;

    virtual void registerCallback(COKitCallback pCallback, void* pData) = 0;

    /** @see kit::Office::getFilterTypes(). */
    virtual char* getFilterTypes() = 0;

    /** @see kit::Office::setOptionalFeatures(). */
    virtual void setOptionalFeatures(COKitOptionalFeatures features) = 0;

    /** @see kit::Office::setDocumentPassword(). */
    virtual void setDocumentPassword(char const* pURL, char const* pPassword) = 0;

    /** @see kit::Office::getVersionInfo(). */
    virtual char* getVersionInfo() = 0;

    /** @see kit::Office::runMacro(). */
    virtual int runMacro(const char* pURL) = 0;

    /** @see kit::Office::signDocument(). */
    virtual bool signDocument(const char* pUrl, const unsigned char* pCertificateBinary,
                               const int nCertificateBinarySize,
                               const unsigned char* pPrivateKeyBinary,
                               const int nPrivateKeyBinarySize) = 0;

    /// @see kit::Office::runLoop()
    virtual void runLoop(COKitPollCallback pPollCallback, COKitWakeCallback pWakeCallback,
                         void* pData) = 0;

    /// @see kit::Office::sendDialogEvent
    virtual void sendDialogEvent(unsigned long long int nKitWindowId, const char* pArguments) = 0;

    /// @see kit::Office::setOption
    virtual void setOption(const char* pOption, const char* pValue) = 0;

    /// @see kit::Office::dumpState
    virtual void dumpState(const char* pOptions, char** pState) = 0;

    /** @see kit::Office::extractRequest.
     */
    virtual char* extractRequest(const char* pFilePath) = 0;

    /// @see kit::Office::trimMemory
    virtual void trimMemory(int nTarget) = 0;

    /// @see kit::Office::startURP
    virtual void* startURP(
        void* pReceiveURPFromLOContext, void* pSendURPToLOContext,
        int (*fnReceiveURPFromLO)(void* pContext, const signed char* pBuffer, int nLen),
        int (*fnSendURPToLO)(void* pContext, signed char* pBuffer, int nLen)) = 0;

    /// @see kit::Office::stopURP
    virtual void stopURP(void* pSendURPToLOContext) = 0;

    /// @see kit::Office::joinThreads
    virtual int joinThreads() = 0;

    /// @see kit::Office::startThreads
    virtual void startThreads() = 0;

    /// @see kit::Office::setForkedChild
    virtual void setForkedChild(bool bIsChild) = 0;

    /** @see kit::Office::extractDocumentStructureRequest.
     */
    virtual char* extractDocumentStructureRequest(const char* pFilePath, const char* pFilter) = 0;

    /// @see kit::Office::registerAnyInputCallback()
    virtual void registerAnyInputCallback(COKitAnyInputCallback pCallback, void* pData) = 0;

    /// @see kit::Office::getDocsCount().
    virtual int getDocsCount() = 0;

    /// @see kit::Office::registerFileSaveDialogCallback()
    virtual void registerFileSaveDialogCallback(COKitFileSaveDialogCallback pCallback) = 0;

    /// @see kit::Office::executeScript().
    virtual void executeScript(char const * script, char ** result, char ** error,
                               void (*proxyCallback) (void * data, char const * payload),
                               void * proxyCallbackData, bool * usedLegacyUnoApi) = 0;

    /// @see kit::Office::deliverProxyResult().
    virtual void deliverProxyResult(char const * callId, char const * jsonValue) = 0;

    /// @see kit::Office::cancelProxyCalls().
    virtual void cancelProxyCalls() = 0;

    /// @see kit::Office::isExpectedReentry().
    virtual int isExpectedReentry() = 0;

    /// @see kit::Office::takeLegacyUnoApiUseFlag().
    virtual bool takeLegacyUnoApiUseFlag() = 0;

    /// @see kit::Office::registerRevealInFileManagerCallback()
    virtual void
    registerRevealInFileManagerCallback(COKitRevealInFileManagerCallback pCallback) = 0;

    /** @see kit::Office::installClipboardProvider(). */
    virtual void installClipboardProvider(const COKitClipboardProvider* pProvider) = 0;

    /** @see kit::Office::getGlobalClipboard(). */
    virtual int getGlobalClipboard(const char **pMimeTypes, size_t      *pOutCount,
                                   char      ***pOutMimeTypes, size_t     **pOutSizes,
                                   char      ***pOutStreams) = 0;
};

struct COKitDocument
{
    virtual ~COKitDocument() = default;

    virtual void destroy() = 0;

    virtual int saveAs(const char* pUrl, const char* pFormat, const char* pFilterOptions) = 0;

    /** @see kit::Document::getDocumentType(). */
    virtual COKitDocumentType getDocumentType() = 0;

    /// @see kit::Document::getParts().
    virtual int getParts() = 0;

    /// @see kit::Document::getPartPageRectangles().
    virtual char* getPartPageRectangles() = 0;

    /// @see kit::Document::getPart().
    virtual int getPart() = 0;

    /// @see kit::Document::setPart().
    virtual void setPart(int nPart) = 0;

    /// @see kit::Document::getPartName().
    virtual char* getPartName(int nPart) = 0;

    /// @see kit::Document::setPartMode().
    virtual void setPartMode(COKitPartMode eMode) = 0;

    /// @see kit::Document::paintTile().
    virtual void paintTile(unsigned char* pBuffer, const int nCanvasWidth, const int nCanvasHeight,
                           const int nTilePosX, const int nTilePosY, const int nTileWidth,
                           const int nTileHeight) = 0;

    /// @see kit::Document::getTileMode().
    virtual COKitTileMode getTileMode() = 0;

    /// @see kit::Document::getDocumentSize().
    virtual void getDocumentSize(long* pWidth, long* pHeight) = 0;

    /// @see kit::Document::initializeForRendering().
    virtual void initializeForRendering(const char* pArguments) = 0;

    /// @see kit::Document::registerCallback().
    virtual void registerCallback(COKitCallback pCallback, void* pData) = 0;

    /// @see kit::Document::postKeyEvent
    virtual void postKeyEvent(COKitKeyEventType eType, int nCharCode, int nKeyCode) = 0;

    /// @see kit::Document::postMouseEvent
    virtual void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount,
                                int nButtons, int nModifier) = 0;

    /// @see kit::Document::postUnoCommand
    virtual void postUnoCommand(const char* pCommand, const char* pArguments,
                                bool bNotifyWhenFinished) = 0;

    /// @see kit::Document::setTextSelection
    virtual void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) = 0;

    /// @see kit::Document::getTextSelection
    virtual char* getTextSelection(const char* pMimeType, char** pUsedMimeType) = 0;

    /// @see kit::Document::paste().
    virtual bool paste(const char* pMimeType, const char* pData, size_t nSize) = 0;

    /// @see kit::Document::setGraphicSelection
    virtual void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) = 0;

    /// @see kit::Document::resetSelection
    virtual void resetSelection() = 0;

    /// @see kit::Document::getCommandValues().
    virtual char* getCommandValues(const char* pCommand) = 0;

    /// @see kit::Document::setClientZoom().
    virtual void setClientZoom(int nTilePixelWidth, int nTilePixelHeight, int nTileTwipWidth,
                               int nTileTwipHeight) = 0;

    /// @see kit::Document::setVisibleArea).
    virtual void setClientVisibleArea(int nX, int nY, int nWidth, int nHeight) = 0;

    /// @see kit::Document::createView().
    virtual int createView() = 0;
    /// @see kit::Document::destroyView().
    virtual void destroyView(int nId) = 0;
    /// @see kit::Document::setView().
    virtual void setView(int nId) = 0;
    /// @see kit::Document::getView().
    virtual int getView() = 0;
    /// @see kit::Document::getViewsCount().
    virtual int getViewsCount() = 0;

    /// @see kit::Document::getPartHash().
    virtual char* getPartHash(int nPart) = 0;

    /// Paints a tile from a specific part.
    /// @see kit::Document::paintTile().
    virtual void paintPartTile(unsigned char* pBuffer, const int nPart, const int nMode,
                               const int nCanvasWidth, const int nCanvasHeight,
                               const int nTilePosX, const int nTilePosY, const int nTileWidth,
                               const int nTileHeight) = 0;

    /// @see kit::Document::getViewIds().
    virtual bool getViewIds(int* pArray, size_t nSize) = 0;

    /// @see kit::Document::setOutlineState).
    virtual void setOutlineState(bool bColumn, int nLevel, int nIndex, bool bHidden) = 0;

    /// Paints window with given id to the buffer
    /// @see kit::Document::paintWindow().
    virtual void paintWindow(unsigned nWindowId, unsigned char* pBuffer, const int x, const int y,
                             const int width, const int height) = 0;

    /// @see kit::Document::postWindow().
    virtual void postWindow(unsigned nWindowId, COKitWindowAction eAction, const char* pData) = 0;

    /// @see kit::Document::postWindowKeyEvent().
    virtual void postWindowKeyEvent(unsigned nWindowId, COKitKeyEventType eType, int nCharCode,
                                    int nKeyCode) = 0;

    /// @see kit::Document::postWindowMouseEvent().
    virtual void postWindowMouseEvent(unsigned nWindowId, COKitMouseEventType eType, int nX,
                                      int nY, int nCount, int nButtons, int nModifier) = 0;

    /// @see kit::Document::setViewLanguage().
    virtual void setViewLanguage(int nId, const char* language) = 0;

    /// @see kit::Document::postWindowExtTextInputEvent
    virtual void postWindowExtTextInputEvent(unsigned nWindowId, COKitExtTextInputType eType,
                                             const char* pText) = 0;

    /// @see kit::Document::getPartInfo().
    virtual char* getPartInfo(int nPart) = 0;

    /// Paints window with given id to the buffer with the give DPI scale
    /// (every pixel is dpiscale-times larger).
    /// @see kit::Document::paintWindow().
    virtual void paintWindowDPI(unsigned nWindowId, unsigned char* pBuffer, const int x,
                                const int y, const int width, const int height,
                                const double dpiscale) = 0;

    /// @see kit::Document::insertCertificate().
    virtual bool insertCertificate(const unsigned char* pCertificateBinary,
                                   const int nCertificateBinarySize,
                                   const unsigned char* pPrivateKeyBinary,
                                   const int nPrivateKeyBinarySize) = 0;

    /// @see kit::Document::addCertificate().
    virtual bool addCertificate(const unsigned char* pCertificateBinary,
                                const int nCertificateBinarySize) = 0;

    /// @see kit::Document::getSignatureState().
    virtual int getSignatureState() = 0;

    /// @see kit::Document::renderShapeSelection
    virtual size_t renderShapeSelection(char** pOutput) = 0;

    /// @see kit::Document::postWindowGestureEvent().
    virtual void postWindowGestureEvent(unsigned nWindowId, const char* pType, int nX, int nY,
                                        int nOffset) = 0;

    /// @see kit::Document::createViewWithOptions().
    virtual int createViewWithOptions(const char* pOptions) = 0;

    /// @see kit::Document::selectPart().
    virtual void selectPart(int nPart, int nSelect) = 0;

    /// @see kit::Document::moveSelectedParts().
    /// nIntoSection: when >= 0, the section at that index will be re-anchored
    /// to the first moved slide (i.e. the slide becomes the new section start).
    /// Pass -1 to keep the default behaviour where sections stay anchored to
    /// their existing non-moved slides.
    virtual void moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection) = 0;

    /// Resize window with given id.
    /// @see kit::Document::resizeWindow().
    virtual void resizeWindow(unsigned nWindowId, const int width, const int height) = 0;

    /// Pass a nullptr terminated array of mime-type strings
    /// @see kit::Document::getClipboard for more details
    virtual int getClipboard(const char **pMimeTypes, size_t      *pOutCount,
                             char      ***pOutMimeTypes, size_t     **pOutSizes,
                             char      ***pOutStreams) = 0;

    /// @see kit::Document::setClipboard
    virtual int setClipboard(const size_t   nInCount, const char   **pInMimeTypes,
                             const size_t  *pInSizes, const char   **pInStreams) = 0;

    /// @see kit::Document::getSelectionType
    virtual COKitSelectionType getSelectionType() = 0;

    /// @see kit::Document::removeTextContext
    virtual void removeTextContext(unsigned nWindowId, int nBefore, int nAfter) = 0;

    /// @see kit::Document::sendDialogEvent
    virtual void sendDialogEvent(unsigned long long int nKitWindowId, const char* pArguments) = 0;

    /// @see kit::Document::renderFontOrientation().
    virtual unsigned char* renderFontOrientation(const char* pFontName, const char* pChar,
                                                 int* pFontWidth, int* pFontHeight,
                                                 int pOrientation) = 0;

    /// Switches view to viewId if viewId >= 0, and paints window
    /// @see kit::Document::paintWindowDPI().
    virtual void paintWindowForView(unsigned nWindowId, unsigned char* pBuffer, const int x,
                                    const int y, const int width, const int height,
                                    const double dpiscale, int viewId) = 0;

    /// @see kit::Document::completeFunction().
    virtual void completeFunction(const char* pFunctionName) = 0;

    /// @see kit::Document::setWindowTextSelection
    virtual void setWindowTextSelection(unsigned nWindowId, bool bSwap, int nX, int nY) = 0;

    /// @see kit::Document::sendFormFieldEvent
    virtual void sendFormFieldEvent(const char* pArguments) = 0;

    /// @see kit::Document::setBlockedCommandList
    virtual void setBlockedCommandList(int nViewId, const char* blockedCommandList) = 0;

    /// @see kit::Document::renderSearchResult
    virtual bool renderSearchResult(const char* pSearchResult, unsigned char** pBitmapBuffer,
                                    int* pWidth, int* pHeight, size_t* pByteSize) = 0;

    /// @see kit::Document::sendContentControlEvent().
    virtual void sendContentControlEvent(const char* pArguments) = 0;

    /// @see kit::Document::getSelectionTypeAndText
    virtual COKitSelectionType getSelectionTypeAndText(const char* pMimeType, char** pText,
                                                       char** pUsedMimeType) = 0;

    /// @see kit::Document::getDataArea().
    virtual void getDataArea(long nPart, long* pCol, long* pRow) = 0;

    /// @see kit::Document::getEditMode().
    virtual int getEditMode() = 0;

    /// @see kit::Document::setViewTimezone().
    virtual void setViewTimezone(int nId, const char* pTimezone) = 0;

    /// @see kit::Document::setAccessibilityState().
    virtual void setAccessibilityState(int nId, bool nEnabled) = 0;

    /// @see kit::Document::getA11yFocusedParagraph.
    virtual char* getA11yFocusedParagraph() = 0;

    /// @see kit::Document::getA11yCaretPosition.
    virtual int getA11yCaretPosition() = 0;

    /// @see kit::Document::setViewReadOnly().
    virtual void setViewReadOnly(int nId, const bool readOnly) = 0;

    /// @see kit::Document::setAllowChangeComments().
    virtual void setAllowChangeComments(int nId, const bool allow) = 0;

    /// @see kit::Document::getPresentationInfo
    virtual char* getPresentationInfo() = 0;

    /// @see kit::Document::createSlideRenderer
    virtual bool createSlideRenderer(const char* pSlideHash, int nSlideNumber,
                                     unsigned* nViewWidth, unsigned* nViewHeight,
                                     bool bRenderBackground, bool bRenderMasterPage) = 0;

    /// @see kit::Document::postSlideshowCleanup
    virtual void postSlideshowCleanup() = 0;

    /// @see kit::Document::renderNextSlideLayer
    virtual bool renderNextSlideLayer(unsigned char* pBuffer, bool* bIsBitmapLayer, double* pScale,
                                      char** pJsonMessage) = 0;

    /// @see kit::Document::setViewOption
    virtual void setViewOption(const char* pOption, const char* pValue) = 0;

    /// @see kit::Document::setColorPreviewState().
    virtual void setColorPreviewState(int nId, bool nEnabled) = 0;

    /// @see kit::Document::setAllowManageRedlines().
    virtual void setAllowManageRedlines(int nId, bool allow) = 0;

    /// @see kit::Document::transferClipboardFromView().
    virtual void transferClipboardFromView(int nSourceViewId) = 0;

    /// @see kit::Document::flushClipboard().
    virtual void flushClipboard() = 0;

    /// @see kit::Document::getPartUniqueId().
    virtual unsigned long long getPartUniqueId(int nPart, int nMode)= 0;

    /// @see kit::Document::getPartIndex().
    virtual int getPartIndex(int nPart, int nMode) = 0;

};

/*
 * The reasons this C++ code is not as pretty as it could be are:
 *  a) provide a pure C API - that's useful for some people
 *  b) allow ABI stability - C++ vtables are not good for that.
 *  c) avoid C++ types as part of the API.
 */
namespace kit
{

/// The kit::Document class represents one loaded document instance.
class Document
{
private:
    COKitDocument* mpDoc;

public:
    /// A kit::Document is typically created by the kit::Office::documentLoad() method.
    Document(COKitDocument* pDoc) :
        mpDoc(pDoc)
    {}

    ~Document()
    {
        mpDoc->destroy();
    }

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
    bool saveAs(const char* pUrl, const char* pFormat = NULL, const char* pFilterOptions = NULL)
    {
        return mpDoc->saveAs(pUrl, pFormat, pFilterOptions) != 0;
    }

    /// Gives access to the underlying C pointer.
    COKitDocument *get() { return mpDoc; }

    /**
     * Get document type.
     *
     * @return an element of the COKitDocumentType enum.
     */
    COKitDocumentType getDocumentType()
    {
        return mpDoc->getDocumentType();
    }

    /**
     * Get number of part that the document contains.
     *
     * Part refers to either individual sheets in a Calc, or slides in Impress,
     * and has no relevance for Writer.
     */
    int getParts()
    {
        return mpDoc->getParts();
    }

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
    char* getPartPageRectangles()
    {
        return mpDoc->getPartPageRectangles();
    }

    /// Get the current part number of the document. For a presentation or
    /// drawing document a part number is the page's stable unique identifier;
    /// for other document types it is the part's index.
    int getPart()
    {
        return mpDoc->getPart();
    }

    /// Set the current part of the document by its part number. The part
    /// number of a page that is gone selects nothing.
    void setPart(int nPart)
    {
        mpDoc->setPart(nPart);
    }

    /// Get the current part's name.
    char* getPartName(int nPart)
    {
        return mpDoc->getPartName(nPart);
    }

    /// Get the current part's hash.
    char* getPartHash(int nPart)
    {
        return mpDoc->getPartHash(nPart);
    }

    /**
     * Get the stable unique identifier of one part: a nonzero integer assigned
     * to the part for the whole document session, kept over part moves,
     * insertions and deletions of other parts. nMode selects the part list the
     * index addresses: 0 for the standard parts, 1 for the master pages, 2 for
     * the notes pages. Zero when there is no such part or the document has no
     * part identifiers.
     */
    unsigned long long getPartUniqueId(int nPart, int nMode)
    {
        return mpDoc->getPartUniqueId(nPart, nMode);
    }

    /**
     * Get the index the part with the given part number holds now. For a
     * presentation or drawing document a part number is the page's stable
     * unique identifier, and the result is the position of that page in the
     * part list nMode selects; -1 when no page carries that number any more.
     * For other document types the part number is the index itself.
     */
    int getPartIndex(int nPart, int nMode)
    {
        return mpDoc->getPartIndex(nPart, nMode);
    }

    void setPartMode(COKitPartMode eMode)
    {
        mpDoc->setPartMode(eMode);
    }

    int getEditMode()
    {
        return mpDoc->getEditMode();
    }

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
    void paintTile(unsigned char* pBuffer,
                          const int nCanvasWidth,
                          const int nCanvasHeight,
                          const int nTilePosX,
                          const int nTilePosY,
                          const int nTileWidth,
                          const int nTileHeight)
    {
        return mpDoc->paintTile(pBuffer, nCanvasWidth, nCanvasHeight,
                                nTilePosX, nTilePosY, nTileWidth, nTileHeight);
    }

    /**
     * Renders a window (dialog, popup, etc.) with give id
     *
     * @param nWindowId
     * @param pBuffer Buffer with enough memory allocated to render any dialog
     * @param x x-coordinate from where the dialog should start painting
     * @param y y-coordinate from where the dialog should start painting
     * @param width The width of the dialog image to be painted
     * @param height The height of the dialog image to be painted
     * @param dpiscale The dpi scale value used by the client.  Please note
     *                 that the x, y, width, height are supposed to be the
     *                 values with dpiscale applied (ie. dialog covering
     *                 100x100 "normal" pixels with dpiscale '2' will have
     *                 200x200 width x height), so that it is easy to compute
     *                 the buffer sizes etc.
     */
    void paintWindow(unsigned nWindowId,
                     unsigned char* pBuffer,
                     const int x,
                     const int y,
                     const int width,
                     const int height,
                     const double dpiscale = 1.0,
                     const int viewId = -1)
    {
        return mpDoc->paintWindowForView(nWindowId, pBuffer, x, y,
                                                 width, height, dpiscale, viewId);
    }

    /**
     * Posts a command to the window (dialog, popup, etc.) with given id
     *
     * @param nWindowid
     */
    void postWindow(unsigned nWindowId, COKitWindowAction eAction, const char* pData = nullptr)
    {
        return mpDoc->postWindow(nWindowId, eAction, pData);
    }

    /**
     * Gets the tile mode: the pixel format used for the pBuffer of paintTile().
     *
     * @return the pixel order the document's tiles use.
     */
    COKitTileMode getTileMode()
    {
        return mpDoc->getTileMode();
    }

    /// Get the document sizes in TWIPs.
    void getDocumentSize(long* pWidth, long* pHeight)
    {
        mpDoc->getDocumentSize(pWidth, pHeight);
    }

    /// Get the data area (in Calc last row and column).
    void getDataArea(long nPart, long* pCol, long* pRow)
    {
        mpDoc->getDataArea(nPart, pCol, pRow);
    }

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
    void initializeForRendering(const char* pArguments = NULL)
    {
        mpDoc->initializeForRendering(pArguments);
    }

    /**
     * Registers a callback. COKit will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    void registerCallback(COKitCallback pCallback, void* pData)
    {
        mpDoc->registerCallback(pCallback, pData);
    }

    /**
     * Posts a keyboard event to the focused frame.
     *
     * @param eType Event type, like press or release.
     * @param nCharCode contains the Unicode character generated by this event or 0
     * @param nKeyCode contains the integer code representing the key of the event (non-zero for control keys)
     */
    void postKeyEvent(COKitKeyEventType eType, int nCharCode, int nKeyCode)
    {
        mpDoc->postKeyEvent(eType, nCharCode, nKeyCode);
    }

    /**
     * Posts a keyboard event to the dialog
     *
     * @param nWindowId
     * @param eType Event type, like press or release.
     * @param nCharCode contains the Unicode character generated by this event or 0
     * @param nKeyCode contains the integer code representing the key of the event (non-zero for control keys)
     */
    void postWindowKeyEvent(unsigned nWindowId, COKitKeyEventType eType, int nCharCode,
                            int nKeyCode)
    {
        mpDoc->postWindowKeyEvent(nWindowId, eType, nCharCode, nKeyCode);
    }

    /**
     * Posts a mouse event to the document.
     *
     * @param eType Event type, like down, move or up.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nCount number of clicks: 1 for single click, 2 for double click
     * @param nButtons: which mouse buttons: 1 for left, 2 for middle, 4 right
     * @param nModifier: which keyboard modifier: (see include/vcl/vclenum.hxx for possible values)
     */
    void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount, int nButtons,
                        int nModifier)
    {
        mpDoc->postMouseEvent(eType, nX, nY, nCount, nButtons, nModifier);
    }

    /**
     * Posts a mouse event to the window with given id.
     *
     * @param nWindowId
     * @param eType Event type, like down, move or up.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nCount number of clicks: 1 for single click, 2 for double click
     * @param nButtons: which mouse buttons: 1 for left, 2 for middle, 4 right
     * @param nModifier: which keyboard modifier: (see include/vcl/vclenum.hxx for possible values)
     */
    void postWindowMouseEvent(unsigned nWindowId, COKitMouseEventType eType, int nX, int nY,
                              int nCount, int nButtons, int nModifier)
    {
        mpDoc->postWindowMouseEvent(nWindowId, eType, nX, nY, nCount, nButtons,
                                           nModifier);
    }

    /**
     * Posts a dialog event for the window with given id
     *
     * @param nWindowId id of the window to notify
     * @param pArguments arguments of the event.
     */
    void sendDialogEvent(unsigned long long int nWindowId, const char* pArguments = NULL)
    {
        mpDoc->sendDialogEvent(nWindowId, pArguments);
    }

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
    void postUnoCommand(const char* pCommand, const char* pArguments = NULL, bool bNotifyWhenFinished = false)
    {
        mpDoc->postUnoCommand(pCommand, pArguments, bNotifyWhenFinished);
    }

    /**
     * Sets the start or end of a text selection.
     *
     * @param nType @see COKitSetTextSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY)
    {
        mpDoc->setTextSelection(eType, nX, nY);
    }

    /**
     * Gets the currently selected text.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     * @param pUsedMimeType output parameter to inform about the determined format (suggested one or plain text).
     */
    char* getTextSelection(const char* pMimeType, char** pUsedMimeType = NULL)
    {
        return mpDoc->getTextSelection(pMimeType, pUsedMimeType);
    }

    /**
     * Gets the type of the selected content.
     *
     * In most cases it is more efficient to use getSelectionTypeAndText().
     *
     * @return what kind of selection the document holds.
     */
    COKitSelectionType getSelectionType()
    {
        return mpDoc->getSelectionType();
    }

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
    COKitSelectionType getSelectionTypeAndText(const char* pMimeType, char** pText,
                                              char** pUsedMimeType = NULL)
    {
        return mpDoc->getSelectionTypeAndText(pMimeType, pText, pUsedMimeType);
    }

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
    bool getClipboard(const char **pMimeTypes,
                      size_t      *pOutCount,
                      char      ***pOutMimeTypes,
                      size_t     **pOutSizes,
                      char      ***pOutStreams)
    {
        return mpDoc->getClipboard(pMimeTypes, pOutCount, pOutMimeTypes, pOutSizes, pOutStreams);
    }

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
    bool setClipboard(const size_t  nInCount,
                      const char  **pInMimeTypes,
                      const size_t *pInSizes,
                      const char  **pInStreams)
    {
        return mpDoc->setClipboard(nInCount, pInMimeTypes, pInSizes, pInStreams);
    }

    /**
     * Shares another view's live clipboard transferable into the current view's
     * clipboard by reference, without serializing (same-process only). The caller
     * must have made the destination the current view first.
     *
     * @param nSourceViewId the view whose clipboard contents to share.
     */
    void transferClipboardFromView(int nSourceViewId)
    {
        mpDoc->transferClipboardFromView(nSourceViewId);
    }

    /**
     * Renders every advertised clipboard format now, so the clipboard's
     * contents stay readable after this document is closed. Call it while the
     * document is still alive, when it produced the current clipboard content
     * and other documents remain open. A lazy transferable (Writer, Impress)
     * builds its own clip document; a self-contained one (Calc) is unaffected.
     */
    void flushClipboard()
    {
        mpDoc->flushClipboard();
    }

    /**
     * Pastes content at the current cursor position.
     *
     * @param pMimeType format of pData, for example text/plain;charset=utf-8.
     * @param pData the actual data to be pasted.
     * @return if the supplied data was pasted successfully.
     */
    bool paste(const char* pMimeType, const char* pData, size_t nSize)
    {
        return mpDoc->paste(pMimeType, pData, nSize);
    }

    /**
     * Adjusts the graphic selection.
     *
     * @param nType @see COKitSetGraphicSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY)
    {
        mpDoc->setGraphicSelection(eType, nX, nY);
    }

    /**
     * Gets rid of any text or graphic selection.
     */
    void resetSelection()
    {
        mpDoc->resetSelection();
    }

    /**
     * Returns a json mapping of the possible values for the given command
     * e.g. {commandName: ".uno:StyleApply", commandValues: {"familyName1" : ["list of style names in the family1"], etc.}}
     * @param pCommand a UNO command for which the possible values are requested
     * @return {commandName: unoCmd, commandValues: {possible_values}}
     *
     * The return value is dynamically allocated and should be
     * deallocated by calling the kit::Office::freeMemory() function.
     */
    char* getCommandValues(const char* pCommand)
    {
        return mpDoc->getCommandValues(pCommand);
    }

    /**
     * Save the client's view so that we can compute the right zoom level
     * for the mouse events. This only affects CALC.
     * @param nTilePixelWidth - tile width in pixels
     * @param nTilePixelHeight - tile height in pixels
     * @param nTileTwipWidth - tile width in twips
     * @param nTileTwipHeight - tile height in twips
     */
    void setClientZoom(
            int nTilePixelWidth,
            int nTilePixelHeight,
            int nTileTwipWidth,
            int nTileTwipHeight)
    {
        mpDoc->setClientZoom(nTilePixelWidth, nTilePixelHeight, nTileTwipWidth, nTileTwipHeight);
    }

    /**
     * Inform core about the currently visible area of the document on the
     * client, so that it can perform e.g. page down (which depends on the
     * visible height) in a sane way.
     *
     * @param nX - top left corner horizontal position
     * @param nY - top left corner vertical position
     * @param nWidth - area width
     * @param nHeight - area height
     */
    void setClientVisibleArea(int nX, int nY, int nWidth, int nHeight)
    {
        mpDoc->setClientVisibleArea(nX, nY, nWidth, nHeight);
    }

    /**
     * Show/Hide a single row/column header outline for Calc documents.
     *
     * @param bColumn - if we are dealing with a column or row group
     * @param nLevel - the level to which the group belongs
     * @param nIndex - the group entry index
     * @param bHidden - the new group state (collapsed/expanded)
     */
    void setOutlineState(bool bColumn, int nLevel, int nIndex, bool bHidden)
    {
        mpDoc->setOutlineState(bColumn, nLevel, nIndex, bHidden);
    }

    /**
     * Create a new view for an existing document with
     * options similar to documentLoadWithOptions.
     * By default a loaded document has 1 view.
     * @return the ID of the new view.
     */
    int createView(const char* pOptions = nullptr)
    {
        return mpDoc->createViewWithOptions(pOptions);
    }

    /**
     * Destroy a view of an existing document.
     * @param nId a view ID, returned by createView().
     */
    void destroyView(int nId)
    {
        mpDoc->destroyView(nId);
    }

    /**
     * Set an existing view of an existing document as current.
     * @param nId a view ID, returned by createView().
     */
    void setView(int nId)
    {
        mpDoc->setView(nId);
    }

    /**
     * Get the current view.
     * @return a view ID, previously returned by createView().
     */
    int getView()
    {
        return mpDoc->getView();
    }

    /**
     * Get number of views of this document.
     */
    int getViewsCount()
    {
        return mpDoc->getViewsCount();
    }

    /**
     * Paints a font name or character if provided to be displayed in the font list
     * @param pFontName the font to be painted
     */
    unsigned char* renderFont(const char *pFontName,
                          const char *pChar,
                          int *pFontWidth,
                          int *pFontHeight,
                          int pOrientation=0)
    {
        return mpDoc->renderFontOrientation(pFontName, pChar, pFontWidth, pFontHeight, pOrientation);
    }

    /**
     * Renders a subset of the document's part to a pre-allocated buffer.
     *
     * @param nPart the part number of the document of which the tile is painted. For a
     * presentation or drawing document that is the page's stable unique identifier, resolved to
     * the index the page holds when it paints; the part number of a page that is gone paints
     * nothing.
     * @see paintTile.
     */
    void paintPartTile(unsigned char* pBuffer,
                              const int nPart,
                              const int nMode,
                              const int nCanvasWidth,
                              const int nCanvasHeight,
                              const int nTilePosX,
                              const int nTilePosY,
                              const int nTileWidth,
                              const int nTileHeight)
    {
        return mpDoc->paintPartTile(pBuffer, nPart, nMode,
                                            nCanvasWidth, nCanvasHeight,
                                            nTilePosX, nTilePosY,
                                            nTileWidth, nTileHeight);
    }

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
    bool getViewIds(int* pArray,
                           size_t nSize)
    {
        return mpDoc->getViewIds(pArray, nSize);
    }

    /**
     * Set the language tag of the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param language Bcp47 languageTag, like en-US or so.
     */
    void setViewLanguage(int nId, const char* language)
    {
        mpDoc->setViewLanguage(nId, language);
    }

    /**
     * Post the text input from external input window, like IME, to given windowId
     *
     * @param nWindowId Specify the window id to post the input event to. If
     * nWindow is 0, the event is posted into the document
     * @param eType which stage of the input method's composition this is
     * @param pText Text for COKitExtTextInputType::TEXTINPUT
     */
    void postWindowExtTextInputEvent(unsigned nWindowId, COKitExtTextInputType eType,
                                     const char* pText)
    {
        mpDoc->postWindowExtTextInputEvent(nWindowId, eType, pText);
    }

    /**
     *  Insert certificate (in binary form) to the certificate store.
     */
    bool insertCertificate(const unsigned char* pCertificateBinary,
                           const int pCertificateBinarySize,
                           const unsigned char* pPrivateKeyBinary,
                           const int nPrivateKeyBinarySize)
    {
        return mpDoc->insertCertificate(pCertificateBinary, pCertificateBinarySize,
                                                pPrivateKeyBinary, nPrivateKeyBinarySize);
    }

    /**
     *  Add the certificate (in binary form) to the certificate store.
     *
     */
    bool addCertificate(const unsigned char* pCertificateBinary,
                         const int pCertificateBinarySize)
    {
        return mpDoc->addCertificate(pCertificateBinary, pCertificateBinarySize);
    }

    /**
     *  Verify signature of the document.
     *
     *  Check possible values in include/sfx2/signaturestate.hxx
     */
    int getSignatureState()
    {
        return mpDoc->getSignatureState();
    }

    /**
     * Gets an image of the selected shapes.
     * @param pOutput contains the result; use free to deallocate.
     * @return the size of *pOutput in bytes.
     */
    size_t renderShapeSelection(char** pOutput)
    {
        return mpDoc->renderShapeSelection(pOutput);
    }

    /**
     * Posts a gesture event to the window with given id.
     *
     * @param nWindowId
     * @param pType Event type, like panStart, panEnd, panUpdate.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nOffset difference value from when the gesture started to current value
     */
    void postWindowGestureEvent(unsigned nWindowId,
                              const char* pType,
                              int nX, int nY, int nOffset)
    {
        return mpDoc->postWindowGestureEvent(nWindowId, pType, nX, nY, nOffset);
    }

    /// Set a part's selection mode, naming the part by its part number.
    /// nSelect is 0 to deselect, 1 to select, and 2 to toggle.
    void selectPart(int nPart, int nSelect)
    {
        mpDoc->selectPart(nPart, nSelect);
    }

    /// Moves the selected pages/slides to a new position.
    /// nPosition is the new position where the selection
    /// should go. bDuplicate when true will copy instead of move.
    /// nIntoSection: when >= 0, re-anchor that section to the first moved
    /// slide (slide becomes the section's new first slide).  Pass -1 (default)
    /// to keep the existing section anchoring.
    void moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection = -1)
    {
        mpDoc->moveSelectedParts(nPosition, bDuplicate, nIntoSection);
    }

    /**
     * Resize a window (dialog, popup, etc.) with give id.
     *
     * @param nWindowId
     * @param width The width of the window.
     * @param height The height of the window.
     */
    void resizeWindow(unsigned nWindowId,
                      const int width,
                      const int height)
    {
        return mpDoc->resizeWindow(nWindowId, width, height);
    }

    /**
     * For deleting many characters all at once
     *
     * @param nWindowId Specify the window id to post the input event to. If
     * nWindow is 0, the event is posted into the document
     * @param nBefore The characters to be deleted before the cursor position
     * @param nAfter The characters to be deleted after the cursor position
     */
    void removeTextContext(unsigned nWindowId, int nBefore, int nAfter)
    {
        mpDoc->removeTextContext(nWindowId, nBefore, nAfter);
    }

    /**
     * Select the Calc function to be pasted into the formula input box
     *
     * @param nIndex is the index of the selected function
     */
    void completeFunction(const char* pFunctionName)
    {
        mpDoc->completeFunction(pFunctionName);
    }

    /**
     * Sets the start or end of a text selection for a dialog.
     *
     * @param nWindowId
     * @param bSwap swap anchor and cursor position of current selection
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    void setWindowTextSelection(unsigned nWindowId, bool bSwap, int nX, int nY)
    {
        mpDoc->setWindowTextSelection(nWindowId, bSwap, nX, nY);
    }

    /**
     * Posts an event for the form field at the cursor position.
     *
     * @param pArguments arguments of the event.
     */
    void sendFormFieldEvent(const char* pArguments)
    {
        mpDoc->sendFormFieldEvent(pArguments);
    }

    void setBlockedCommandList(int nViewId, const char* blockedCommandList)
    {
        mpDoc->setBlockedCommandList(nViewId, blockedCommandList);
    }
    /**
     * Render input search result to a bitmap buffer.
     *
     * @param pSearchResult payload containing the search result data
     * @param pBitmapBuffer contains the bitmap; use free to deallocate.
     * @param nWidth output bitmap width
     * @param nHeight output bitmap height
     * @param nByteSize output bitmap byte size
     * @return true if successful
     */
    bool renderSearchResult(const char* pSearchResult, unsigned char** pBitmapBuffer,
                            int* pWidth, int* pHeight, size_t* pByteSize)
    {
        return mpDoc->renderSearchResult(pSearchResult, pBitmapBuffer, pWidth, pHeight, pByteSize);
    }

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
    void sendContentControlEvent(const char* pArguments)
    {
        mpDoc->sendContentControlEvent(pArguments);
    }

    /**
     * Set the timezone of the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param timezone a timezone in the tzfile(5) format (e.g. Pacific/Auckland).
     */
    void setViewTimezone(int nId, const char* timezone)
    {
        mpDoc->setViewTimezone(nId, timezone);
    }

    /** Set if the view should be treated as readonly or not.
     *
     * @param nId view ID
     * @param readOnly
    */
    void setViewReadOnly(int nId, const bool readOnly)
    {
        mpDoc->setViewReadOnly(nId, readOnly);
    }

    /** Set if the view can edit comments on readonly mode or not.
     *
     * @param nId view ID
     * @param allow
    */
    void setAllowChangeComments(int nId, const bool allow)
    {
        mpDoc->setAllowChangeComments(nId, allow);
    }

    /** Set if the view can manage redlines in readonly mode or not.
     *
     * @param nId view ID
     * @param allow
    */
    void setAllowManageRedlines(int nId, bool allow)
    {
        mpDoc->setAllowManageRedlines(nId, allow);
    }

    /**
     * Enable/Disable accessibility support for the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param nEnabled true/false
     */
    void setAccessibilityState(int nId, bool nEnabled)
    {
        mpDoc->setAccessibilityState(nId, nEnabled);
    }

    /**
     *  Get the current focused paragraph info:
     *  {
     *      "content": paragraph content
     *      "start": selection start
     *      "end": selection end
     *  }
     */
    char* getA11yFocusedParagraph()
    {
        return mpDoc->getA11yFocusedParagraph();
    }

    /// Get the current text cursor position.
    int getA11yCaretPosition()
    {
        return mpDoc->getA11yCaretPosition();
    }

    /// Get the information about the current presentation (Impress only).
    char* getPresentationInfo()
    {
        return mpDoc->getPresentationInfo();
    }

    /// Create a slide renderer in core for the input slide.
    bool createSlideRenderer(
        const char* pSlideHash,
        int nSlideNumber, unsigned* nViewWidth, unsigned* nViewHeight,
        bool bRenderBackground, bool bRenderMasterPage)
    {
        return mpDoc->createSlideRenderer(pSlideHash, nSlideNumber, nViewWidth, nViewHeight, bRenderBackground, bRenderMasterPage);
    }

    /// Clean-up the slideshow (slide renderer)
    void postSlideshowCleanup()
    {
        mpDoc->postSlideshowCleanup();
    }

    /// Render the slide layer
    bool renderNextSlideLayer(unsigned char* pBuffer, bool* bIsBitmapLayer, double* pScale, char** pJsonMessage)
    {
        return mpDoc->renderNextSlideLayer(pBuffer, bIsBitmapLayer, pScale, pJsonMessage);
    }

    /// Set named view options
    void setViewOption(const char* pOption, const char* pValue)
    {
        mpDoc->setViewOption(pOption, pValue);
    }

    /**
     * Set color preview state for the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param nEnabled true/false
     */
    void setColorPreviewState(int nId, bool nEnabled)
    {
        mpDoc->setColorPreviewState(nId, nEnabled);
    }

};

/// The kit::Office class represents one started COKit instance.
class Office
{
private:
    COKit* mpThis;

public:
    /// A kit::Office is typically created by the kit_cpp_init() function.
    Office(COKit* pThis) :
        mpThis(pThis)
    {}

    ~Office()
    {
        mpThis->destroy();
    }

    /**
     * Loads a document from a URL.
     *
     * @param pUrl the URL of the document to load
     * @param pFilterOptions options for the import filter, e.g. SkipImages.
     *        Another useful FilterOption is "Language=...".  It is consumed
     *        by the documentLoad() itself, and when provided, COKit
     *        switches the language accordingly first.
     */
    Document* documentLoad(const char* pUrl, const char* pFilterOptions = NULL)
    {
        COKitDocument* pDoc = mpThis->documentLoadWithOptions(pUrl, pFilterOptions);

        if (pDoc == NULL)
            return NULL;

        return new Document(pDoc);
    }

    /// Returns the last error as a string. The returned pointer has to be freed by the caller
    /// by calling the freeError() member function.
    char* getError()
    {
        return mpThis->getError();
    }

    /**
     * Frees the memory pointed to by pFree.
     *
     * Use on dynamically allocated data returned by COKit
     * functions. In other cases than the value returned by
     * getError(), call freeMemory() instead for clarity.
     *
     */
    void freeError(char* pFree)
    {
        mpThis->freeError(pFree);
    }

    /**
     * Registers a callback. COKit will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    void registerCallback(COKitCallback pCallback, void* pData)
    {
        mpThis->registerCallback(pCallback, pData);
    }

    /**
     * Returns details of filter types.
     *
     * Example returned string:
     *
     * {
     *     "writer8": {
     *         "MediaType": "application/vnd.oasis.opendocument.text"
     *     },
     *     "calc8": {
     *         "MediaType": "application/vnd.oasis.opendocument.spreadsheet"
     *     }
     * }
     *
     */
    char* getFilterTypes()
    {
        return mpThis->getFilterTypes();
    }

    /**
     * Set bitmask of optional features supported by the client.
     *
     * @see COKitOptionalFeatures
     */
    void setOptionalFeatures(COKitOptionalFeatures features)
    {
        return mpThis->setOptionalFeatures(features);
    }

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
    void setDocumentPassword(char const* pURL, char const* pPassword)
    {
        mpThis->setDocumentPassword(pURL, pPassword);
    }

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
    char* getVersionInfo()
    {
        return mpThis->getVersionInfo();
    }

    /**
     * Run a macro.
     *
     * Same syntax as on command line is permissible (ie. the macro:// URI forms)
     *
     * @param pURL macro url to run
     */
    bool runMacro( const char* pURL)
    {
        return mpThis->runMacro(pURL );
    }

    /**
     * Exports the document and signs its content.
     */
    bool signDocument(const char* pURL,
                       const unsigned char* pCertificateBinary, const int nCertificateBinarySize,
                       const unsigned char* pPrivateKeyBinary, const int nPrivateKeyBinarySize)
    {
        return mpThis->signDocument(pURL,
                                            pCertificateBinary, nCertificateBinarySize,
                                            pPrivateKeyBinary, nPrivateKeyBinarySize);
    }

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
    void runLoop(COKitPollCallback pPollCallback,
                 COKitWakeCallback pWakeCallback,
                 void* pData)
    {
        mpThis->runLoop(pPollCallback, pWakeCallback, pData);
    }

    /**
     * Posts a dialog event for the window with given id
     *
     * @param nWindowId id of the window to notify
     * @param pArguments arguments of the event.
     */
    void sendDialogEvent(unsigned long long int nWindowId, const char* pArguments = NULL)
    {
        mpThis->sendDialogEvent(nWindowId, pArguments);
    }

    /**
     * Generic function to toggle and tweak various things in the core LO
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
    void setOption(const char* pOption, const char* pValue)
    {
        mpThis->setOption(pOption, pValue);
    }

    /**
     * Debugging tool for triggering a dump of internal state.
     *
     * COKit can get into an unhelpful state at run-time when
     * in heavy use. This provides a critical tool for inspecting
     * relevant internal state.
     *
     * @param pOption future expansion - string options.
     * @param pState - heap allocated, C string containing the state dump.
     */
    void dumpState(const char* pOption, char** pState)
    {
        mpThis->dumpState(pOption, pState);
    }

    char* extractRequest(const char* pFilePath)
    {
        return mpThis->extractRequest(pFilePath);
    }

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
    void trimMemory (int nTarget)
    {
        mpThis->trimMemory(nTarget);
    }

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
    void executeScript(char const * script, char ** result, char ** error,
                       void (*proxyCallback) (void * data, char const * payload) = nullptr,
                       void * proxyCallbackData = nullptr, bool * usedLegacyUnoApi = nullptr)
    {
        mpThis->executeScript(
            script, result, error, proxyCallback, proxyCallbackData, usedLegacyUnoApi);
    }

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
    void deliverProxyResult(char const * callId, char const * jsonValue)
    {
        mpThis->deliverProxyResult(callId, jsonValue);
    }

    /**
     * Cancel all in-flight JS-UNO proxy listener calls by unblocking any
     * ProxyInvocation::invoke currently waiting in Application::Yield, treating each pending
     * call as if the iframe had returned an empty value.  Intended to be called from
     * ChildSession destruction so the kit's main thread can't end up spinning on a
     * synchronous proxy result that will never come.
     */
    void cancelProxyCalls()
    {
        mpThis->cancelProxyCalls();
    }

    /**
     * Whether the current thread is inside a window where it has explicitly opted into a kitPoll
     * re-entry (via vcl::kit::pushExpectedReentry).  The host poll loop should suppress its
     * non-async-dialog warning while this is true.
     *
     * @return non-zero if a re-entry is expected.
     */
    bool isExpectedReentry()
    {
        return mpThis->isExpectedReentry();
    }

    /**
     * Returns and clears the process-wide "legacy UNO API use" flag set by the engine's UNO bridges
     * (Basic, Python, ...) when at runtime a script resolves an identifer in the legacy UNO API.
     *
     * @return true if at least one legacy identifier was resolved since the last call; false
     *         otherwise.
     */
    bool takeLegacyUnoApiUseFlag()
    {
        return mpThis->takeLegacyUnoApiUseFlag();
    }

    /**
     * Start a UNO acceptor using the function pointers provides to read and write data to/from the acceptor.
     *
     * @param pReceiveURPFromLOContext A pointer that will be passed to your fnRecieveURPFromLO function
     * @param pSendURPToLOContext A pointer that will be passed to your fnSendURPToLO function
     * @param fnReceiveURPFromLO A function pointer that LO should use to pass URP back to the caller
     * @param fnSendURPToLO A function pointer pointer that the caller should use to pass URP to LO
     */
    void* startURP(void* pReceiveURPFromLOContext, void* pSendURPToLOContext,
                   int (*fnReceiveURPFromLO)(void* pContext, const signed char* pBuffer, int nLen),
                   int (*fnSendURPToLO)(void* pContext, signed char* pBuffer, int nLen))
    {
        return mpThis->startURP(pReceiveURPFromLOContext, pSendURPToLOContext,
                                        fnReceiveURPFromLO, fnSendURPToLO);
    }

    /**
     * Stop a function based URP connection you previously started with startURP
     *
     * @param pURPContext the context returned by startURP  when starting the connection
     */
    void stopURP(void* pURPContext)
    {
        mpThis->stopURP(pURPContext);
    }

    /**
     * Joins all threads if possible to get down to a single process
     * which can be forked from safely.
     *
     * @returns non-zero for successful join, 0 for failure.
     */
    int joinThreads()
    {
        return mpThis->joinThreads();
    }

    /**
     * Starts all threads that are necessary to continue working
     * after a joinThreads().
     */
    void startThreads()
    {
        mpThis->startThreads();
    }

    /**
     * Informs that this process is either a parent, or a child
     * process post-fork, allowing improved resource sharing.
     */
    void setForkedChild(bool bIsChild)
    {
        return mpThis->setForkedChild(bIsChild);
    }

    char* extractDocumentStructureRequest(const char* pFilePath, const char* pFilter)
    {
        return mpThis->extractDocumentStructureRequest(pFilePath, pFilter);
    }

    /**
     * Registers a callback that can determine if there are any pending input events.
     */
    void registerAnyInputCallback(COKitAnyInputCallback pCallback, void* pData)
    {
        return mpThis->registerAnyInputCallback(pCallback, pData);
    }

    /**
     * Get number of documents of this COKit.
     */
    int getDocsCount()
    {
        return mpThis->getDocsCount();
    }

    /**
     * Registers a callback that can display an interactive file save dialog.
     */
    void registerFileSaveDialogCallback(COKitFileSaveDialogCallback pCallback)
    {
        return mpThis->registerFileSaveDialogCallback(pCallback);
    }

    /**
     * Registers a callback that reveals (and selects) a file in the native file manager.
     */
    void registerRevealInFileManagerCallback(COKitRevealInFileManagerCallback pCallback)
    {
        return mpThis->registerRevealInFileManagerCallback(pCallback);
    }

    /**
     * Installs a process-global clipboard provider and switches the kit to a
     * single shared clipboard for every view and document. Use this in the
     * in-process desktop app, where there is one local user and one platform
     * clipboard, so the clipboard survives closing an individual document. Pass
     * nullptr to remove the provider and return to the default per-view
     * clipboards (as used by the collaborative server).
     */
    void installClipboardProvider(const COKitClipboardProvider* pProvider)
    {
        mpThis->installClipboardProvider(pProvider);
    }

    /**
     * Read the desktop app's single process-wide clipboard. See
     * Document::getClipboard() for the parameters; this needs no document
     * because the shared clipboard is process-global. The distinct name marks
     * that it reads one global clipboard, not a per-view one.
     */
    bool getGlobalClipboard(const char **pMimeTypes,
                            size_t      *pOutCount,
                            char      ***pOutMimeTypes,
                            size_t     **pOutSizes,
                            char      ***pOutStreams)
    {
        return mpThis->getGlobalClipboard(pMimeTypes, pOutCount, pOutMimeTypes, pOutSizes,
                                          pOutStreams);
    }

    /**
     * Frees the memory pointed to by pFree.
     *
     * Use on dynamically allocated data returned by COKit
     * functions. Just a wrapper for freeError() with a better name.
     */
    void freeMemory(char* pFree)
    {
        freeError(pFree);
    }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
