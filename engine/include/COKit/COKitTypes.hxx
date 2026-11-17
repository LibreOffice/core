/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

/* The types of the COKit API, without its COKit and COKitDocument classes. */

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

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
 * Every callback runs synchronously on the engine's main-loop thread, the one that runs
 * the document's UNO commands. In the in-process desktop apps that thread is not the
 * app's UI thread. A callback must not re-enter the engine.
 *
 * @see COKit::installClipboardProvider().
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
     * Paste: return a vector of mime-type
     * strings the platform clipboard currently offers. No bytes are read.
     */
    std::vector<std::string> (*getMimeTypes)(void);

    /**
     * Paste: fetch the bytes for one mime type. On success set *pOutData to a
     * vector of data and return true; on failure
     * return false.
     */
    bool (*getDataForMimeType)(const char* pMimeType, std::vector<char>* pOutData);
};

/**
 * One file-type filter for a native file picker: a name the picker can show, and the
 * wildcards that match the type, separated by semicolons ("*.png;*.jpg").
 */
struct COKitFilePickerFilter
{
    const char* pName;
    const char* pWildcards;
};

/**
 * A file picker the host provides to the engine: a native dialog in an in-process desktop
 * app, or a request to the browser client in the Online server.
 *
 * When the provider is installed, a command that needs the user to pick a file calls it
 * instead of opening the engine's own file dialog.
 *
 * @see COKit::installFilePickerProvider().
 */
struct COKitFilePickerProvider
{
    /**
     * Show a native open-file picker. Called on the engine's main-loop thread and must
     * not block: marshal to the app's UI thread, show the picker there, and return at
     * once. Call pfnPicked exactly once, from any thread: pass pContext and the picked
     * file's URL ("file://..."), or nullptr for the URL when the user cancelled.
     *
     * pTitle names the purpose of the pick, translated. pFilters is an array of
     * nFilters entries; an empty array means any file can be picked.
     */
    void (*pick)(const char* pTitle, const COKitFilePickerFilter* pFilters, size_t nFilters,
                 void (*pfnPicked)(void* pContext, const char* pUrl), void* pContext);

    /**
     * Whether the URL pick passes on is the file the user picked, where it stays, rather than
     * a copy of it, as a native picker of a desktop app gives. A document may link to such a
     * file, the chapters of a master document for example; a copy goes away.
     */
    bool picksFilesInPlace;

    /**
     * Show a native save-file picker, where the user chooses the folder and the name of a
     * file the engine is about to create, the chapter of a master document for example.
     * The same thread and completion rules apply as for pick: pfnPicked gets the URL of
     * the chosen file ("file://..."), or nullptr when the user cancelled. The engine
     * creates the file at that URL and the document may link to it, so the URL must be
     * where the file stays.
     *
     * pSuggestedUrl is the file URL the picker starts with, its folder and its name.
     * pFilters is as for pick. Leave this nullptr when the app has no such picker; the
     * engine then does not offer to create such files.
     */
    void (*pickSaveLocation)(const char* pTitle, const char* pSuggestedUrl,
                             const COKitFilePickerFilter* pFilters, size_t nFilters,
                             void (*pfnPicked)(void* pContext, const char* pUrl), void* pContext);

    /**
     * Ask the user for a file the document is to link to, which the host keeps where the
     * engine cannot read it, the chapter of a master document in the storage of a server for
     * example. The same thread and completion rules apply as for pick: pfnPicked gets the file
     * the document links to, as an absolute URL or as its path relative to the document in
     * plain text, not percent-encoded and with / between its folders, and the URL of a copy of
     * the file the engine reads now, or nullptr for both when the user cancelled.
     *
     * Leave this nullptr when the host has no such picker.
     */
    void (*pickLinkTarget)(const char* pTitle, const COKitFilePickerFilter* pFilters,
                           size_t nFilters,
                           void (*pfnPicked)(void* pContext, const char* pLinkUrl,
                                             const char* pReadUrl),
                           void* pContext);
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

/// What running a script produced. At most one of oResult and oError holds a
/// value. Both are empty when the script ran and its value was one that
/// JSON.stringify drops, such as undefined, a function or a symbol.
/// oResult is the value, JSON-encoded. oError is either a JSON object
/// describing a JS exception or a plain message.
struct COKitScriptResult
{
    std::optional<std::string> oResult;
    std::optional<std::string> oError;
    bool bUsedLegacyUnoApi = false;
};

/// The last column and the last row of a Calc sheet that hold data, both
/// counted from zero. Both are 1 for a document with no data area, which is
/// every document that is not a spreadsheet.
struct COKitDataArea
{
    int64_t nLastColumn = 1;
    int64_t nLastRow = 1;
};

/// One layer of a slide. An empty aJsonMessage means no layer was rendered.
struct COKitSlideLayer
{
    /// No further layer follows this one.
    bool bIsDone = false;
    /// The layer is a bitmap, rather than a description of what to draw.
    bool bIsBitmapLayer = false;
    /// The scale the layer was rendered at.
    double fScale = 0;
    /// What the layer holds, as JSON.
    std::string aJsonMessage;
};

/// One format on the clipboard: its mime type and the bytes in that format.
/// The bytes can be empty for a type the clipboard offers but cannot produce.
struct COKitClipboardItem
{
    std::string aMimeType;
    std::vector<char> aData;
};

/// A size in pixels.
struct COKitPixelSize
{
    unsigned nWidth = 0;
    unsigned nHeight = 0;
};

/// A size in twips. Zero in both when the document cannot report one.
struct COKitSize
{
    int64_t nWidth = 0;
    int64_t nHeight = 0;
};

/// A rendered image. The pixels are four bytes each, so aPixels holds
/// nWidth * nHeight * 4 bytes. An empty aPixels means nothing was rendered.
struct COKitBitmap
{
    std::vector<unsigned char> aPixels;
    int nWidth = 0;
    int nHeight = 0;
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

/// What the document has selected, and the text of it when eType is
/// COKitSelectionType::TEXT. The text is empty for every other type.
struct COKitSelection
{
    COKitSelectionType eType = COKitSelectionType::NONE;
    std::string aText;
};

/** Result of COKit::translateDocument(). */
enum class COKitTranslateResult
{
    FAILED = -1,
    /// the l10n stream was found and the requested language matched
    TRANSLATED = 0,
    /// no l10n stream (byte copy), or no locale match (markers stripped)
    COPIED_UNTRANSLATED = 1,
    /// in place only: the file needs a rewrite but cannot be written, it is
    /// left as it is
    READ_ONLY = 2
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
     * The current part is changed.
     *
     * Payload is the part identifier: the page's GUID as a braced string for
     * a presentation or drawing document, the part's 0-based index in decimal
     * form for other document types.
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
    SHAPE_DRAG_PREVIEW = 74,

    /**
     * A vector-primitives delta for a slide that changed.
     *
     * Pushed after an invalidation, with the delta for the part that changed.
     * The payload is the id of the part, a newline, and then the same JSON the
     * .uno:VectorPrimitives command returns for a delta: a vectorprimitivesdelta
     * with the part, the version space it counts in, the version it starts from,
     * the version it brings the part to, the object order and the changed
     * objects. The page itself is the first object. Its entry comes along when
     * the background or the master page content changed.
     *
     * The delta describes the part rather than the view whose flush wrote it,
     * and it carries no view state, so one delta stands for every reader of
     * that part.
     */
    VECTOR_PRIMITIVES_DELTA = 75,

    /**
     * The presentation info of an Impress or Draw document changed and
     * should be re-sent.
     *
     * Pushed to every view of the document when a slide's content changes
     * (a shape is added, moved, resized or deleted) or the set or order of
     * pages changes. The payload is a JSON object naming the reason and the
     * changed part, for example { "reason": "gifupdate", "part": 3 }.
     */
    PRESENTATION_INFO = 76,

    /**
     * The pages of an Impress or Draw document that are linked to a source
     * document are not the same as they were.
     *
     * Pushed to every view of the document when a page loses the source it
     * records, which is what an edit of a linked page does, and when an undo
     * or a redo gives that source back. The payload is empty; the receiving
     * view reads the link list for itself.
     */
    SLIDE_LINKS_CHANGED = 77,

    /**
     * Recording of tracked changes was turned on for all views of a Writer
     * document.
     *
     * Pushed to every view of the document, the one that turned the recording
     * on included, when a request to record in all views starts the recording
     * in at least one view. Views that each turn on recording for themselves
     * only do not cause it. The payload is a JSON object whose "viewId" is the
     * id of the view that turned the recording on, for example
     * { "viewId": "1" }.
     */
    REDLINE_RECORDING_FOR_ALL_VIEWS = 78,

    /**
     * The per-user settings were written into the per-user config tree.
     *
     * Sent through the global callback after a dialog that changes settings
     * which outlive the session, such as AutoCorrect Options, was accepted
     * and the configuration was written out. The host can take the files
     * from the per-user config tree now instead of waiting for the end of
     * the session. The payload is empty.
     */
    USER_SETTINGS_PERSISTED = 79,

    /**
     * The user asked to edit a document that a master document links to.
     *
     * Sent to the view whose user chose Edit for a linked file in the
     * Navigator of a master document. The engine does not open the file
     * itself; the host opens it, for example in a window of its own. The
     * payload is a JSON object with the URL of the linked file, as the master
     * document resolves it, its path relative to the master document, and the
     * name of the section that shows it, for example
     * { "url": "file:///home/user/book/chapter1.odt", "id": "chapter1.odt",
     *   "section": "chapter1.odt" }.
     *
     * Also sent with only the URL for a document made of the current one,
     * the master document made by Create Master Document for example, which
     * the host opens the same way: { "url": "file:///home/user/book.odm" }.
     */
    OPEN_LINKED_DOCUMENT = 80,

    /**
     * The user asked to read the files the document links to anew.
     *
     * Sent to the view whose user asked for the update, for example with
     * Update in the Navigator of a master document. The engine reads the
     * files at once, from where it reads them; a host that gave the engine
     * copies of them gives fresh copies, which the engine reads in turn. The
     * payload is a JSON object. Its "ids" array names the files to read anew
     * by their ids as getLinkedFiles() gives them, for example
     * { "ids": [ "chapter1.odt" ] }; without "ids" every file is meant.
     */
    REFRESH_LINKED_FILES = 81,

    /**
     * The list of the pages of a Writer document that serves its pages as
     * vector primitives changed: a page was added or removed, or a page took
     * the place of another.
     *
     * Pushed to every view of the document. The payload is a JSON object with
     * the id of each page in "pageids" and the rectangle of each page, in
     * twips, in "pagerectangles", both in page order and both read from the
     * layout at the same moment, for example
     * { "pageids": [ "5", "9" ],
     *   "pagerectangles": [ [ 284, 284, 12240, 15840 ], [ 284, 16408, 12240, 15840 ] ] }.
     * The ids are the part ids that .uno:VectorPrimitives takes.
     */
    VECTOR_PART_IDS = 82,

    /**
     * This is Calc specific. The cell range inventory of a sheet, its cell
     * ranges of interest, changed. Clients that keep it must fetch it again
     * with .uno:CellRangeInventory?part=N.
     *
     * Pushed to every view of the document, once per changed sheet. The
     * payload names the sheet, for example 'part=0'.
     */
    INVALIDATE_CELL_RANGE_INVENTORY = 83,

    /**
     * This is Calc specific. The cell selection of the view, as cell ranges
     * of the selected sheet e.g. '{"part": 0, "ranges": [[0, 1, 2, 4]]}',
     * each range as [startCol, startRow, endCol, endRow]. The ranges are an
     * empty list when only the cell cursor is shown.
     *
     * Sent alongside COKitCallbackType::CELL_SELECTION_AREA.
     */
    CELL_SELECTION = 84,

    /**
     * Same as COKitCallbackType::CELL_SELECTION, but for another view, with
     * its viewId and mode, e.g. '{"viewId": "1", "part": "0", "mode": "0",
     * "ranges": [[0, 1, 2, 4]]}'. The ranges are also empty when the view
     * goes away.
     *
     * Sent alongside COKitCallbackType::TEXT_VIEW_SELECTION.
     */
    CELL_VIEW_SELECTION = 85,

    /**
     * This is Calc specific. The cell cursor and the cell selection of the
     * view, as cell indexes of the selected sheet, e.g. '{"part": 0,
     * "cursor": {"cell": [0, 1], "visible": true}, "selection": [[0, 1, 2,
     * 4]]}'. It is the commandValues of .uno:CellCursorState.
     *
     * Sent only to a view that moves the cell cursor locally, in place of
     * COKitCallbackType::CELL_CURSOR and COKitCallbackType::CELL_SELECTION.
     */
    CELL_CURSOR_STATE = 86
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
    case COKitCallbackType::SHAPE_DRAG_PREVIEW:
        return "KIT_CALLBACK_SHAPE_DRAG_PREVIEW";
    case COKitCallbackType::VECTOR_PRIMITIVES_DELTA:
        return "KIT_CALLBACK_VECTOR_PRIMITIVES_DELTA";
    case COKitCallbackType::PRESENTATION_INFO:
        return "KIT_CALLBACK_PRESENTATION_INFO";
    case COKitCallbackType::SLIDE_LINKS_CHANGED:
        return "KIT_CALLBACK_SLIDE_LINKS_CHANGED";
    case COKitCallbackType::REDLINE_RECORDING_FOR_ALL_VIEWS:
        return "KIT_CALLBACK_REDLINE_RECORDING_FOR_ALL_VIEWS";
    case COKitCallbackType::USER_SETTINGS_PERSISTED:
        return "KIT_CALLBACK_USER_SETTINGS_PERSISTED";
    case COKitCallbackType::OPEN_LINKED_DOCUMENT:
        return "KIT_CALLBACK_OPEN_LINKED_DOCUMENT";
    case COKitCallbackType::REFRESH_LINKED_FILES:
        return "KIT_CALLBACK_REFRESH_LINKED_FILES";
    case COKitCallbackType::VECTOR_PART_IDS:
        return "KIT_CALLBACK_VECTOR_PART_IDS";
    case COKitCallbackType::INVALIDATE_CELL_RANGE_INVENTORY:
        return "KIT_CALLBACK_INVALIDATE_CELL_RANGE_INVENTORY";
    case COKitCallbackType::CELL_SELECTION:
        return "KIT_CALLBACK_CELL_SELECTION";
    case COKitCallbackType::CELL_VIEW_SELECTION:
        return "KIT_CALLBACK_CELL_VIEW_SELECTION";
    case COKitCallbackType::CELL_CURSOR_STATE:
        return "KIT_CALLBACK_CELL_CURSOR_STATE";
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
using COKitRevealInFileManagerCallback = std::function<void(const char* pUri)>;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
