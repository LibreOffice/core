/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <COKit/COKitTypes.hxx>

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
    virtual std::string getVersionInfo() = 0;

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
    virtual bool signDocument(const char* pUrl,
                              std::span<const unsigned char> aCertificateBinary,
                              std::span<const unsigned char> aPrivateKeyBinary) = 0;

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
     * @param aOptions future expansion - string options.
     * @return the state dump.
     */
    virtual std::string dumpState(std::string_view aOptions) = 0;

    virtual std::string extractRequest(const char* pFilePath) = 0;

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

    virtual std::string extractDocumentStructureRequest(const char* pFilePath, const char* pFilter) = 0;

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
     * On success the returned oResult holds the result of calling the function that the script
     * evaluates to with the @c arguments,
     * JSON-stringified, and oError is empty.  On error oResult is empty and oError holds the
     * JS exception message.  A result that stringifies to nothing, such as `undefined`,
     * leaves both empty.
     *
     * The @c script is UTF-8.
     *
     * @c consoleCallback receives every call the script makes on the global @c console object.
     * jsuno registers the WHATWG Console API methods @c log, @c info, @c warn, @c error, @c debug,
     * @c trace, and @c assert on that object; each dispatches to this callback with @c level set to
     * the method name and @c message set to the stringified-and-space-joined arguments (both UTF-8
     * strings, valid only for the duration of the callback).  @c assert only fires on failed
     * assertions.  @c warn, @c error, @c trace, and @c assert prepend the JS stack to @c message.
     * The callback runs synchronously on the JS thread and must not call back into jsuno.
     *
     * @c proxyCallback, if non-null, is captured by every JS-UNO proxy listener stub created
     * during this call, and fires when the stub later receives a UNO call.  It is called
     * with the @c proxyCallbackData pointer and a UTF-8 JSON payload describing the call (see
     * jsuno::execute), valid only for the duration of the callback.  The callback may fire
     * synchronously while @c script runs, or later from any thread for as long as the proxy is
     * registered, until detachProxyCallback is called for @c proxyCallbackData.  Each proxy keeps
     * the callback it captured at creation time, so a later executeScript with a different
     * callback only affects proxies created by that later call.
     *
     * @param script the script source, which evaluates to the function that is called.
     * @param source used for any exception stack frames that are reported back.
     * @param line (1-based) used for any exception stack frames that are reported back.
     * @param arguments UTF-8 JSON text of the array of the arguments for the function.
     * @param consoleCallback hook for console.* method calls.
     * @param consoleCallbackData opaque pointer passed to @c consoleCallback on each call.
     * @param proxyCallback hook for proxy listener fires; may be null.
     * @param proxyCallbackData opaque pointer passed to @c proxyCallback on each call.
     * @return the script's result or error, and whether it resolved an identifier in the
     *         legacy com.sun.star UNO API.
     */
    virtual COKitScriptResult executeScript(std::string const & script,
                               std::string_view source, int line,
                               std::string const & arguments,
                               std::function<void(
                                   void * data, std::string_view level, std::string_view message)>
                                   consoleCallback,
                               void * consoleCallbackData,
                               void (*proxyCallback) (void * data, std::string_view payload),
                               void * proxyCallbackData) = 0;

    /**
     * Deliver the iframe-side response value back to a JS-UNO proxy listener whose
     * `invoke` is currently waiting (synchronous return-value path).  @c callId matches a callId
     * that was previously sent in the proxyCallback payload.  @c jsonValue is the JSON
     * encoding of the JS-side return value, which the proxy will decode to the listener
     * method's declared return type.
     *
     * Both strings are UTF-8.
     *
     * Spurious callIds (no matching pending invoke) are silently ignored.
     *
     * @param callId opaque token matching one previously delivered to proxyCallback.
     * @param jsonValue JSON-encoded return value for the listener method.
     */
    virtual void deliverProxyResult(std::string_view callId, std::string_view jsonValue) = 0;

    /**
     * Cancel one in-flight JS-UNO proxy listener call by unblocking the
     * ProxyInvocation::invoke waiting for it in Application::Yield, which then throws a
     * DisposedException.
     *
     * Spurious callIds (no matching pending invoke) are silently ignored.
     *
     * @param callId opaque token matching one previously delivered to proxyCallback.
     */
    virtual void cancelProxyCall(std::string_view callId) = 0;

    /**
     * Detach the proxy callbacks that executeScript was given together with @c proxyCallbackData.
     * Once this returns, none of them is called again.  A call to a JS-UNO proxy listener that
     * captured one of them then does nothing for a method with a void return type, and throws a
     * DisposedException for any other method.  This waits for a call of such a callback that runs
     * on another thread, so it must not be called from within the callback.
     *
     * Unknown proxyCallbackData pointers are silently ignored.
     *
     * @param proxyCallbackData opaque pointer previously passed to executeScript.
     */
    virtual void detachProxyCallback(void * proxyCallbackData) = 0;

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
     * Give the engine a native file picker for the in-process desktop app. With a
     * provider installed, a command that needs the user to pick a file - compare
     * documents, insert an image - asks the provider instead of opening the engine's
     * own file dialog, and continues with the picked file when the provider delivers
     * it. Pass nullptr to remove the provider.
     */
    virtual void installFilePickerProvider(const COKitFilePickerProvider* pProvider) = 0;

    /**
     * The files COKitDocument::insertFiles() takes, as lowercase "*.ext"
     * wildcards separated by semicolons: every graphic import format and
     * every media format. Needs no document, so an app can check a name
     * off the engine thread.
     */
    virtual std::string getInsertableFileWildcards() = 0;

    /**
     * Read the desktop app's single process-wide clipboard. See
     * Document::getClipboard() for the parameters; this needs no document
     * because the shared clipboard is process-global. The distinct name marks
     * that it reads one global clipboard, not a per-view one.
     */
    virtual bool getGlobalClipboard(const char **pMimeTypes,
                                    std::vector<std::string>& rOutMimeTypes,
                                    std::vector<std::vector<char>>& rOutStreams) = 0;

    /**
     * Single-format convenience over getGlobalClipboard(), for the platform
     * glue serving one advertised clipboard format on demand. Returns true
     * and fills rOutData only when the shared clipboard holds a non-empty
     * payload for pMimeType.
     */
    bool getGlobalClipboardData(const char* pMimeType, std::vector<char>& rOutData)
    {
        const char* pFilter[] = { pMimeType, nullptr };
        std::vector<std::string> aOutMimeTypes;
        std::vector<std::vector<char>> aOutStreams;
        if (!getGlobalClipboard(pFilter, aOutMimeTypes, aOutStreams) || aOutStreams.empty()
            || aOutStreams[0].empty())
            return false;
        rOutData = std::move(aOutStreams[0]);
        return true;
    }

    /**
     * Translate an ODF file's "_"-marked strings into the given language
     * using the file's embedded "l10n" stream.
     *
     * With no "l10n" zip member the output is a byte-identical copy; with a
     * stream that matches no locale, or that cannot be read, the rewrite
     * only strips the "_" markers.
     * Untouched zip members are preserved byte-for-byte.  Input and output
     * may be the same file: the rewrite is then in place (atomic, skipped
     * when nothing would change), and a file carrying its
     * "l10n_template.xml" copies re-translates on every call until the
     * first save drops them.  An in-place rewrite of a file whose location
     * cannot be written (a read-only install) leaves the file alone and
     * returns READ_ONLY.
     *
     * @param pInputPath input file, system path or file URL
     * @param pOutputPath output file, overwritten if it exists
     * @param pBCP47Language requested language, e.g. "de-DE"
     */
    virtual COKitTranslateResult translateDocument(const char* pInputPath, const char* pOutputPath,
                                                   const char* pBCP47Language) = 0;
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

    /// Get the current part identifier of the document. For a presentation or
    /// drawing document that is the page's GUID as a braced string like
    /// {1BE1A269-4A03-4202-ACFE-0204C5E9BE1F}; for other document types it is
    /// the part's index in decimal form.
    virtual std::string getPart() = 0;

    /// Set the current part of the document by its part identifier. The
    /// identifier of a page that is gone selects nothing.
    virtual void setPart(const char* pPart) = 0;

    /// Get the current part's name.
    virtual std::string getPartName(int nPart) = 0;

    virtual void setPartMode(COKitPartMode eMode) = 0;

    /**
     * Renders a subset of the document to a pre-allocated buffer.
     *
     * Note that the buffer size and the tile size implicitly supports
     * rendering at different zoom levels, as the number of rendered pixels and
     * the rendered rectangle of the document are independent.
     *
     * @param aBuffer where the pixels go, four bytes each, so nCanvasWidth times
     *        nCanvasHeight times four bytes long.
     * @param nCanvasWidth number of pixels in a row of aBuffer.
     * @param nCanvasHeight number of pixels in a column of aBuffer.
     * @param nTilePosX logical X position of the top left corner of the rendered rectangle, in TWIPs.
     * @param nTilePosY logical Y position of the top left corner of the rendered rectangle, in TWIPs.
     * @param nTileWidth logical width of the rendered rectangle, in TWIPs.
     * @param nTileHeight logical height of the rendered rectangle, in TWIPs.
     */
    virtual void paintTile(std::span<unsigned char> aBuffer, const int nCanvasWidth,
                           const int nCanvasHeight, const int nTilePosX, const int nTilePosY,
                           const int nTileWidth, const int nTileHeight) = 0;

    /**
     * Gets the tile mode: the pixel format used for the pBuffer of paintTile().
     *
     * @return the pixel order the document's tiles use.
     */
    virtual COKitTileMode getTileMode() = 0;

    /// Get the document size in TWIPs, or zeroes when the document does not
    /// support tiled rendering.
    virtual COKitSize getDocumentSize() = 0;

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
     * @param eType for type @see COKitSetTextSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    virtual void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) = 0;

    /**
     * Gets the currently selected text.
     *
     * @param aMimeType the format to return the selection in, for example
     *        text/plain;charset=utf-8. An empty one asks for plain text.
     * @return the selection, empty when there is none or it cannot be had in
     *         that format.
     */
    virtual std::string getTextSelection(std::string_view aMimeType) = 0;

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
     * @param eType for type @see COKitSetGraphicSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    virtual void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) = 0;

    /**
     * Moves and resizes the object that is currently active for in-place editing.
     *
     * The area is in the same twips the "INPLACE" graphic selection reports, so a
     * client can hand back the rectangle it was given. The container may narrow the
     * request, for instance for an object protected against moving or resizing, or one
     * pushed past the edge of the page. Does nothing when no object is in-place active.
     *
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nWidth width in document coordinates
     * @param nHeight height in document coordinates
     */
    virtual void setInPlaceObjectArea(int nX, int nY, int nWidth, int nHeight) = 0;

    /**
     * Gets rid of any text or graphic selection.
     */
    virtual void resetSelection() = 0;

    /**
     * Embeds pictures and media files in one undo step. The engine
     * classifies each file by its own format tables; a list with a file
     * that is neither inserts nothing, so the caller can give the whole
     * list to its own paste. A text selection is replaced, as by a paste.
     *
     * @param rUrls file: URLs this process can read
     * @return the number of files inserted: all of them, or none
     */
    virtual std::size_t insertFiles(const std::vector<std::string>& rUrls) = 0;

    /**
     * Returns a json mapping of the possible values for the given command
     * e.g. {commandName: ".uno:StyleApply", commandValues: {"familyName1" : ["list of style names in the family1"], etc.}}
     * @param pCommand a UNO command for which the possible values are requested
     * @return {commandName: unoCmd, commandValues: {possible_values}}
     *
     * The caller owns the returned string and frees it.
     */
    virtual std::string getCommandValues(const char* pCommand) = 0;

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

    /**
     * Renders a subset of the document's part to a pre-allocated buffer.
     *
     * @param pPart the part identifier of the document of which the tile is painted. For a
     * presentation or drawing document that is the page's GUID, resolved to the index the page
     * holds when it paints; the identifier of a page that is gone paints nothing. For other
     * document types it is the part's index in decimal form.
     * @param bIsPreview true when the tile is a small preview of the part rather than a tile of
     * the editing viewport, even when it is a preview of the part currently being edited. A
     * presentation or drawing document hides its editing grid for the duration of such a render.
     * @see paintTile.
     */
    virtual void paintPartTile(std::span<unsigned char> aBuffer, const char* pPart, const int nMode,
                               const int nCanvasWidth, const int nCanvasHeight,
                               const int nTilePosX, const int nTilePosY, const int nTileWidth,
                               const int nTileHeight, bool bIsPreview = false) = 0;

    /**
     * Returns the viewID for each existing view, the most recently used one
     * first. Since viewIDs are not reused, viewIDs are not the same as the
     * index of the view in the view array over time. The list is empty when
     * the document has no views.
     */
    virtual std::vector<int> getViewIds() = 0;

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
    virtual void paintWindow(unsigned nWindowId, std::span<unsigned char> aBuffer, const int x,
                             const int y, const int width, const int height) = 0;

    /**
     * Posts a command to the window (dialog, popup, etc.) with given id
     *
     * @param nWindowId ID of window to pass command to
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

    virtual std::string getPartInfo(int nPart) = 0;

    /// Paints window with given id to the buffer with the give DPI scale
    /// (every pixel is dpiscale-times larger).
    /// @see COKitDocument::paintWindow().
    virtual void paintWindowDPI(unsigned nWindowId, std::span<unsigned char> aBuffer, const int x,
                                const int y, const int width, const int height,
                                const double dpiscale) = 0;

    /**
     *  Insert certificate (in binary form) to the certificate store.
     */
    virtual bool insertCertificate(std::span<const unsigned char> aCertificateBinary,
                                   std::span<const unsigned char> aPrivateKeyBinary) = 0;

    /**
     *  Add the certificate (in binary form) to the certificate store.
     *
     */
    virtual bool addCertificate(std::span<const unsigned char> aCertificateBinary) = 0;

    /**
     *  Verify signature of the document.
     *
     *  Check possible values in include/sfx2/signaturestate.hxx
     */
    virtual int getSignatureState() = 0;

    /**
     * Gets an image of the selected shapes.
     * @return the result
     */
    virtual std::vector<char> renderShapeSelection() = 0;

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

    /// Set a part's selection mode, naming the part by its part identifier.
    /// nSelect is 0 to deselect, 1 to select, and 2 to toggle.
    virtual void selectPart(const char* pPart, int nSelect) = 0;

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
     * Gets the content on the clipboard for the current view.
     *
     * @param pMimeTypes a nullptr terminated list of mime types to fetch, or
     *        nullptr for every type the clipboard offers.
     * @param bSkipDuplicateRenderings when true, a fetch of every type leaves
     *        out the EMF, WMF and BMP renderings. Those hold the same picture
     *        as the metafile and the bitmap they come from, only a native
     *        system clipboard reads them, and they take longer to draw than
     *        all the other formats together. Pass true when the content goes
     *        to another document; the default keeps every rendering, which is
     *        what a system clipboard needs.
     * @return one item per format, empty when the clipboard has nothing to give.
     */
    virtual std::vector<COKitClipboardItem> getClipboard(const char **pMimeTypes,
                                                         bool bSkipDuplicateRenderings = false) = 0;

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
     * Paints a font name or some text in that font, for display in a font list.
     *
     * @param pFontName the name of the font
     * @param pChar the text to paint in the font, or empty
     * @param nRequestedWidth the width to render at, or 0 for the font's own size
     * @param nRequestedHeight the height to render at, or 0 for the font's own size
     * @param nOrientation the rotation, in 10ths of a degree
     * @return the pixels and the size they cover, empty when nothing was rendered
     */
    virtual COKitBitmap renderFontOrientation(const char* pFontName, const char* pChar,
                                              int nRequestedWidth, int nRequestedHeight,
                                              int nOrientation) = 0;

    /**
     * Renders a window (dialog, popup, etc.) with the given id, switching to
     * viewId first when that is >= 0.
     *
     * @param aBuffer Buffer with enough memory allocated to render any dialog
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
    virtual void paintWindowForView(unsigned nWindowId, std::span<unsigned char> aBuffer,
                                    const int x, const int y, const int width, const int height,
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
     * Render input search result to a bitmap.
     *
     * @param pSearchResult payload containing the search result data
     * @return the bitmap, empty when there is nothing to show
     */
    virtual COKitBitmap renderSearchResult(const char* pSearchResult) = 0;

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
     * The type is the same as getSelectionType() gives, and for a text selection the text is the
     * same as getTextSelection() gives.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     */
    virtual COKitSelection getSelectionTypeAndText(const char* pMimeType) = 0;

    /// Get the data area of one part, that is, in Calc, its last column and
    /// last row.
    virtual COKitDataArea getDataArea(long nPart) = 0;

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
     * The client of the view has received its first tile, so the view shows the document
     * now.
     *
     * @param nId a view ID, returned by createView().
     */
    virtual void viewGotFirstTile(int nId) = 0;

    /**
     *  Get the current focused paragraph info:
     *  {
     *      "content": paragraph content
     *      "start": selection start
     *      "end": selection end
     *  }
     */
    virtual std::string getA11yFocusedParagraph() = 0;

    /// Get the current text cursor position.
    virtual int getA11yCaretPosition() = 0;

    /**
     * Puts the caret in a cell of the table last given with the focused paragraph.
     *
     * @param nRow the row of the cell, as given in that table
     * @param nCol the column of the cell, as given in that table
     * @param bEnd true for the end of the cell's text, false for its start
     * @returns false if that table has no such cell
     */
    virtual bool setA11yCaretInTableCell(int nRow, int nCol, bool bEnd) = 0;

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
    virtual std::string getPresentationInfo() = 0;

    /// Create a slide renderer in engine for the input slide. The size asked
    /// for is a maximum, and the size that comes back is the one the renderer
    /// will use, which is no larger. Nothing comes back when no renderer could
    /// be made.
    virtual std::optional<COKitPixelSize> createSlideRenderer(const char* pSlideHash,
                                                              int nSlideNumber,
                                                              COKitPixelSize aMaximumSize,
                                                              bool bRenderBackground,
                                                              bool bRenderMasterPage) = 0;

    /// Clean-up the slideshow (slide renderer)
    virtual void postSlideshowCleanup() = 0;

    /// Render the next layer of the slide into the given buffer, which holds
    /// four bytes per pixel for the size createSlideRenderer() settled on.
    ///
    /// @param aBuffer where the pixels go
    /// @param fScale the scale to render at
    virtual COKitSlideLayer renderNextSlideLayer(std::span<unsigned char> aBuffer,
                                                 double fScale) = 0;

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
     * Get the stable identifier of the part at the given index. For a
     * presentation or drawing document that is the page's GUID as a braced
     * string, kept over part moves, insertions and deletions of other parts,
     * and persistent across sessions when the file format stores it; nMode
     * selects the part list the index addresses: 0 for the standard parts, 1
     * for the master pages, 2 for the notes pages, 3 for the notes master
     * pages, 4 for the handout master page. For other document types the
     * identifier is the index itself in decimal form. Empty when there is no
     * such part.
     */
    virtual std::string getPartId(int nPart, int nMode) = 0;

    /**
     * Get the index the part with the given part identifier holds now. For a
     * presentation or drawing document the identifier is the page's GUID, and
     * the result is the position of that page in the part list nMode selects;
     * -1 when no page carries that identifier any more. For other document
     * types the identifier is the index itself in decimal form.
     */
    virtual int getPartIndex(const char* pPart, int nMode) = 0;

    /**
     * Inserts pages of the presentation at pUrl into this document.
     *
     * @param pUrl the presentation to take the pages from, as a URL.
     * @param pJsonOptions names the pages to take as "slides", an empty list taking every
     *        page, the slide to insert before as "at", whether the pages keep the design of
     *        the file they came from as "keepDesign", whether they stay linked to the
     *        document named by "source" as "link", and the time that source was last
     *        modified as "lastModifiedTime", recorded on each link. "sourcePositions"
     *        names the position each inserted page's slide holds in the source, counted from
     *        one, one for every page inserted or none at all: a page given a position is
     *        linked to that position rather than to one slide, so a refresh reads whichever
     *        slide stands there. "as" says what the pages go in as: "slides", the default;
     *        "section", a section of their own at "at" whose identifier in the source is
     *        "sourceId" and whose name there is "sourceName", linked to that source section
     *        when "link" is set; or "presentation", every page of the file at "at" in the
     *        sections of the file, the pages outside any section in one section named
     *        "sourceName" first, with a link over them all when "link" is set and one per
     *        section the file identifies. The pages that follow a section keep a section apart
     *        from it. A section asks for no "sourcePositions", and a presentation for no
     *        "slides", no "sourcePositions" and no "sourceId".
     * @return whether the pages were inserted.
     */
    virtual bool insertPagesFromFile(const char* pUrl, const char* pJsonOptions) = 0;

    /**
     * Get the links of the document to the slides and sections of other documents.
     *
     * @return JSON of the form {"links":[{"unit":"<slide|section|presentation>","id":"<id>",
     *         "parent":"<id>","source":"<source>","sourceId":"<id>","name":"<name>",
     *         "lastModifiedTime":"<time>","slides":["<part>",...]},...]}: one flat array in
     *         document order, a presentation before the sections it spans and a section before
     *         the slides it spans. The unit says what the link holds a copy of, the id is the
     *         identifier of the page or the section here, or the presentation's own, the parent
     *         the innermost link that spans the unit or empty, the source the source document
     *         as the user knows it, the sourceId the slide or section in the source and empty
     *         for a presentation, the name the name that unit had in the source at the last
     *         read, and the slides the parts the link spans. A document that links to nothing
     *         reports an empty array. Empty for a document that holds no pages, which is
     *         anything but a presentation or a drawing.
     */
    virtual std::string getSlideLinks() = 0;

    /**
     * Replace the span of one link with what the source holds for its unit now.
     *
     * @param pUnit "slide", "section" or "presentation", the kind of link.
     * @param pId the page, the section or the presentation, by the identifier getSlideLinks
     *        reports for it.
     * @param pSourceName the source document as the user knows it, as getSlideLinks reports it.
     * @param pUrl a file: URL of a file on this machine holding a presentation the source wrote
     *        out. A slide link reads the slide it follows from it. A section link reads the
     *        slides the file lists under the identifier of the source section, in order; the
     *        section then holds a fresh copy of each of those slides and no other slide, so a
     *        slide edited in this document is replaced as well. A presentation link reads every
     *        page and every section of the file: a section linked under the presentation already
     *        keeps its identifier here, a section new to the file comes in, and one the file no
     *        longer lists goes. Each page read is a page of its own, so the parts of the link
     *        change and a caller holding them asks for the list again.
     * @param pLastModifiedTime the time the source document was last modified now, recorded on
     *        the link so a later comparison can tell whether it is up to date.
     * @param pName the name the unit has in the source now. A section takes it when the section
     *        still has the name the source section had at the last read, and keeps a name given
     *        to it in this document. A presentation records it as the name of the source
     *        document, and keeps the name it recorded when none is given.
     * @param pNotUpdated when given, takes a JSON array of the slides the file held nothing for,
     *        by the identifier of each one, in document order: ["{...}","{...}"]. It is an empty
     *        array when every slide was read, and empty when the refresh did not run.
     * @return the number of slides replaced, or -1 when no such link holds or it is not linked to
     *         that source document, when the URL is not a file on this machine, when the time or
     *         the name is longer than 128 characters or holds a control character, when the view
     *         cannot edit, or when the file could not be read.
     */
    virtual int refreshSlideLink(const char* pUnit, const char* pId, const char* pSourceName,
                                 const char* pUrl, const char* pLastModifiedTime,
                                 const char* pName, std::string* pNotUpdated) = 0;

    /**
     * Refresh every link to one source document that is under no other link, from one file.
     *
     * @param pSourceName the source document as the user knows it, as getSlideLinks reports it.
     * @param pUrl a file: URL of a file on this machine holding a presentation the source wrote
     *        out, read as refreshSlideLink reads it for each link.
     * @param pLastModifiedTime the time the source document was last modified now.
     * @param pNotUpdated as for refreshSlideLink.
     * @return the number of slides replaced, or -1 when no link to that source document holds,
     *         when the URL is not a file on this machine, when the view cannot edit, or when the
     *         file could not be read.
     */
    virtual int refreshSlideLinks(const char* pSourceName, const char* pUrl,
                                  const char* pLastModifiedTime, std::string* pNotUpdated) = 0;

    /**
     * Take one link off.
     *
     * The page, the section or the presentation keeps the slides and the sections it holds and
     * becomes the document's own, so a refresh of the source it came from leaves it as it is.
     * This is one undo action, and undoing it gives the link back.
     *
     * @param pUnit "slide", "section" or "presentation", the kind of link.
     * @param pId the page, the section or the presentation, by the identifier getSlideLinks
     *        reports for it.
     * @param bDeep whether every link beneath comes off as well: the link of each slide of a
     *        section, and of each section and slide of a presentation. Without it, each one
     *        stays linked to its own unit of the source.
     * @return true when the link held and is off now.
     */
    virtual bool breakSlideLink(const char* pUnit, const char* pId, bool bDeep) = 0;

    /**
     * Writes the given pages out as a document of their own, in the order they are given.
     *
     * @param pParts a comma separated list of the pages to write, named the way this document
     *        names its parts or by the identifier a page keeps in ODF
     * @param pUrl the file the document is written to, as a URL, whose extension chooses the
     *        filter it is written with.
     * @return whether the document was written.
     */
    virtual bool exportPages(const char* pParts, const char* pUrl) = 0;

    /**
     * Marks the given objects in this view, and nothing else, as a click on them would.
     *
     * @param pObjectIds the objects to mark, as a comma separated list of the ids
     *        SdrObject::GetUniqueID() answers, empty to mark nothing.
     *
     * The objects of one selection all sit at the same level: directly on the page, or in one and
     * the same group. An id naming an object somewhere else, or naming nothing on the page the
     * view shows, is left out.
     */
    virtual void selectObjects(const char* pObjectIds) = 0;

    /**
     * Moves points of the path an object is drawn along, for an object that is drawn along one.
     *
     * @param nObjectId the object, by the id SdrObject::GetUniqueID() answers.
     * @param pChanges what moved and where to, in twips: one change is the polygon, the point of
     *        it, which of the three parts of that point moved - 0 the point itself, 1 the weight
     *        that bends the curve coming in, 2 the weight that bends the one going out - and the
     *        place it moved to, as "polygon,point,part,x,y", several of them separated by ";".
     *
     * Only what is named moves. The rest of the path is left as it stands, so nothing that was
     * not touched can change. Moving a point takes the weights of its curves along with it, and
     * moving a weight of a point that a curve runs smoothly or symmetrically through moves the
     * other weight of that point as much as it takes to keep it so. It is one step to undo. An
     * id naming nothing on the page the view shows, or naming an object that is not drawn along
     * a path, does nothing.
     */
    virtual void setObjectPoints(unsigned long long nObjectId, const char* pChanges) = 0;

    /**
     * Deletes one point of the path of a drawing object. A polygon left with fewer than two
     * points is deleted too, and an object left with no polygon is deleted. It is one undo step.
     *
     * @param nObjectId the object, by the id SdrObject::GetUniqueID() answers.
     * @param nPolygon the index of the polygon in the path, from zero, in the order of the render
     *        geometry.
     * @param nPoint the index of the point in that polygon, from zero, in the order of the render
     *        geometry.
     */
    virtual void deleteObjectPoint(unsigned long long nObjectId, int nPolygon, int nPoint) = 0;

    /**
     * Adds a point to the path of a drawing object, on the edge nearest to the given place. A
     * curved edge keeps its shape and passes through the new point. It is one undo step.
     *
     * @param nObjectId the object, by the id SdrObject::GetUniqueID() answers.
     * @param nX the horizontal position on the page, in twips.
     * @param nY the vertical position on the page, in twips.
     */
    virtual void insertObjectPoint(unsigned long long nObjectId, int nX, int nY) = 0;

    /**
     * Sets the continuity at one point of the path of a drawing object, and moves the weights of
     * the point to match. It is one undo step.
     *
     * @param nObjectId the object, by the id SdrObject::GetUniqueID() answers.
     * @param nPolygon which polygon of the path, counted from zero.
     * @param nPoint which point of that polygon, counted from zero.
     * @param nContinuity 0 for a corner, 1 for smooth, 2 for symmetric.
     */
    virtual void setObjectPointContinuity(unsigned long long nObjectId, int nPolygon, int nPoint,
                                          int nContinuity)
        = 0;

    /**
     * Makes the edge after one point of the path of a drawing object a curve or a straight line.
     * A new curve looks like the line at first. It is one undo step.
     *
     * @param nObjectId the object, by the id SdrObject::GetUniqueID() answers.
     * @param nPolygon which polygon of the path, counted from zero.
     * @param nPoint the index of the point in that polygon, from zero. The edge goes from this
     *        point to the next one. In a closed polygon, the last point's edge goes to the first.
     * @param bCurved true for a curve, false for a straight line.
     */
    virtual void setObjectSegmentCurved(unsigned long long nObjectId, int nPolygon, int nPoint,
                                        bool bCurved)
        = 0;

    /**
     * Splits the path of a drawing object at one of its points. A closed path opens there. An
     * open path becomes two objects that meet at the point, so the point cannot be one of its
     * ends. Only a path of one polygon can be split. It is one undo step.
     *
     * @param nObjectId the object, by the id SdrObject::GetUniqueID() answers.
     * @param nPolygon which polygon of the path, counted from zero.
     * @param nPoint which point of that polygon, counted from zero.
     */
    virtual void splitObjectPath(unsigned long long nObjectId, int nPolygon, int nPoint) = 0;

    /**
     * Closes the path of a drawing object by adding an edge from its last point to its first, or
     * opens it by removing that edge. A path can be closed only when one of its polygons has at
     * least three points. It is one undo step.
     *
     * @param nObjectId the object, by the id SdrObject::GetUniqueID() answers.
     * @param bClosed true to close the path, false to open it.
     */
    virtual void setObjectPathClosed(unsigned long long nObjectId, bool bClosed) = 0;

    /**
     * Turns every curved edge of the path of a drawing object that runs straight into a line. A
     * curve runs straight when its weights lie on the line between its ends. It is one undo step,
     * and none when nothing changes.
     *
     * @param nObjectId the object, by the id SdrObject::GetUniqueID() answers.
     * @param bPointsOnALine true to also delete the points that lie on a straight line between
     *        their neighbours.
     */
    virtual void simplifyObjectPath(unsigned long long nObjectId, bool bPointsOnALine) = 0;

    /**
     * Adds a drawing object drawn along the given points to the page the current view shows.
     * The object goes on the layer the view draws new objects on and takes the attributes the
     * view gives them, as an object drawn with the mouse does. It is selected afterwards, and
     * adding it is one undo step.
     *
     * @param pPoints the points in twips on the page, each as "x,y", separated by ";". A point
     *        equal to the one before it is left out. An open path needs two points at least and
     *        a closed one three.
     * @param bClosed true for a closed polygon, false for an open polyline.
     */
    virtual void insertPolygonObject(const char* pPoints, bool bClosed) = 0;

    /**
     * Moves one of the points a drawing object was given for a connector to tie itself to.
     *
     * The place is given on the object rather than on the page: 0 and 0 is its upper left corner
     * and 1 and 1 its lower right one, which is how the render geometry reports such a point.
     *
     * @param nObjectId the unique id of the object, as the render geometry names it
     * @param nAt which of the points of that object, counted from zero as they are listed
     * @param fX where the point goes on the object, across it
     * @param fY where the point goes on the object, down it
     */
    virtual void setObjectGluePoint(unsigned long long nObjectId, int nAt, double fX, double fY)
        = 0;

    /**
     * Gives a drawing object a new point for a connector to tie itself to, as the last of its
     * points. It is one step to undo.
     *
     * @param nObjectId the unique id of the object, as the render geometry names it
     * @param fX where the point goes on the object, across it, 0 at its left side and 1 at its
     * right one
     * @param fY where the point goes on the object, down it, 0 at its upper side and 1 at its
     * lower one
     */
    virtual void insertObjectGluePoint(unsigned long long nObjectId, double fX, double fY) = 0;

    /**
     * Takes one of the points a drawing object was given for a connector to tie itself to away.
     * A point the shape makes from its own geometry stays. It is one step to undo.
     *
     * @param nObjectId the unique id of the object, as the render geometry names it
     * @param nAt which of the points of that object, counted from zero as they are listed
     */
    virtual void deleteObjectGluePoint(unsigned long long nObjectId, int nAt) = 0;

    /**
     * Sets the way a connector leaves a drawing object at one of the points it was given for a
     * connector to tie itself to. A point the shape makes from its own geometry keeps its way.
     * It is one step to undo.
     *
     * @param nObjectId the unique id of the object, as the render geometry names it
     * @param nAt which of the points of that object, counted from zero as they are listed
     * @param pWayOut "left", "right", "top" or "bottom" for one side, "horizontal" or "vertical"
     * for the two sides that face each other across or down the object, or "smart" for whichever
     * way suits the connector
     */
    virtual void setObjectGluePointWayOut(unsigned long long nObjectId, int nAt,
                                          const char* pWayOut)
        = 0;

    /**
     * Sets whether one of the points a drawing object was given for a connector to tie itself to
     * keeps its distance from the side it is measured from when the object changes size, or
     * keeps its share of the size of the object. The point stays where it is on the page. A point
     * the shape makes from its own geometry stays as it is. It is one step to undo.
     *
     * @param nObjectId the unique id of the object, as the render geometry names it
     * @param nAt which of the points of that object, counted from zero as they are listed
     * @param bKeepsItsDistance true for a distance, false for a share of the size
     */
    virtual void setObjectGluePointKeepsItsDistance(unsigned long long nObjectId, int nAt,
                                                    bool bKeepsItsDistance)
        = 0;

    /**
     * Sets the side of a drawing object that one of the points it was given for a connector to
     * tie itself to is measured from, across the object or down it. The point stays where it is
     * on the page. A point the shape makes from its own geometry stays as it is. It is one step
     * to undo.
     *
     * @param nObjectId the unique id of the object, as the render geometry names it
     * @param nAt which of the points of that object, counted from zero as they are listed
     * @param bDown false for the side across the object, true for the side down it
     * @param pSide across the object "left", "middle" or "right"; down it "top", "middle" or
     * "bottom"
     */
    virtual void setObjectGluePointFrom(unsigned long long nObjectId, int nAt, bool bDown,
                                        const char* pSide)
        = 0;

    /**
     * Gives drawing objects the mapping they are to be drawn by, which moves, scales, turns or
     * shears them.
     *
     * @param pMappings one item per object, separated by semicolons. An item is the unique id of
     * the object, as the render geometry names it, then the six numbers of its mapping -
     * a, b, c, d, e and f - all separated by commas. The corner 0 and 0 of the object goes to
     * e and f, in twips, and its own axes are the other four. A mapping whose two axes are
     * turned over against each other turns the object over
     * @param pWhat what was done, which names the step it can be undone with: "move", "scale",
     * "turn" or "shear"
     */
    virtual void setObjectTransform(const char* pMappings, const char* pWhat) = 0;

    /**
     * Moves one of the points a custom shape is shaped by, which the shape reads as the value it
     * shapes itself from.
     *
     * @param nObjectId the unique id of the object, as the render geometry names it
     * @param nAt which of the points the shape offers, counted from zero as it lists them
     * @param nX where the point goes, on the page, in twips
     * @param nY where the point goes, on the page, in twips
     * @param bWithTheShape true to carry the shape along where the point is one that moves it,
     * which is what a callout's tail does when it is dragged without a modifier
     */
    virtual void setObjectControlPoint(unsigned long long nObjectId, int nAt, int nX, int nY,
                                       bool bWithTheShape)
        = 0;

    /**
     * Moves one handle of an object to a place on the page, by naming the object and the handle
     * rather than by what a view has selected. The object reads the move its own way: a corner
     * radius, the angle a piece of a circle begins at, the offset of a measurement.
     *
     * @param nObjectId the unique id of the object, as the render geometry names it
     * @param nKind the kind of handle, as the drawing layer numbers the kinds
     * @param nPolygon which polygon of the object the handle belongs to, zero where it belongs
     * to none
     * @param nPoint which point of that polygon, zero where it stands for no point
     * @param nAt the number the object itself gives the handle, for the handles that are told
     * apart by nothing else; -1 where the three above already name it
     * @param nX where the handle goes, on the page, in twips
     * @param nY where the handle goes, on the page, in twips
     * @param bOrtho whether the handle is moved as the office moves it while Shift is held: held
     * to the way it runs, or to the other handles, as the object says
     */
    virtual void setObjectHandle(unsigned long long nObjectId, int nKind, int nPolygon,
                                 int nPoint, int nAt, int nX, int nY, bool bOrtho)
        = 0;

    /**
     * Crops a picture to a new window onto its graphic, given by the window's upper left and
     * lower right corners. The corners are the window's own, along the picture's axes, so a
     * turned or sheared picture keeps its turn and shear. The graphic stays where it is on the
     * page and only the window onto it moves, the way the office crops. The crop values follow
     * from the window. Nothing needs to be selected for it.
     *
     * @param nObjectId the unique id of the picture
     * @param nUpperLeftX where the window's upper left corner goes, on the page, in twips
     * @param nUpperLeftY where the window's upper left corner goes, on the page, in twips
     * @param nLowerRightX where the window's lower right corner goes, on the page, in twips
     * @param nLowerRightY where the window's lower right corner goes, on the page, in twips
     */
    virtual void cropObject(unsigned long long nObjectId, int nUpperLeftX, int nUpperLeftY,
                            int nLowerRightX, int nLowerRightY) = 0;

    /**
     * Operates a form control on the page the view shows, the way a person using it does, while
     * the form controls of the view are not in design mode. The control the view holds for it
     * takes the action and calls the same listeners as after a click, so the events and macros
     * of the form run as in the office.
     *
     * @param nObjectId the unique id of the form control
     * @param pAction "press" presses a button, "state" sets the state of a check box to pValue,
     * "check" checks an option button, "text" types pValue into a field as its new text,
     * "select" selects the entries of a list that pValue names, "value" moves a scroll bar or
     * a spin button to pValue, "spin" presses the spin button of a field that pValue names,
     * "date" gives a date field the date pValue
     * @param pValue for "state": 0 not checked, 1 checked, 2 undetermined; for "text" the text,
     * in UTF-8; for "select" the places of the entries, counted from 0 and separated by commas,
     * empty for none; for "value" the number; for "spin" "up" or "down"; for "date" the date
     * as YYYY-MM-DD; empty for the others
     */
    virtual void formControlAction(unsigned long long nObjectId, const char* pAction,
                                   const char* pValue)
        = 0;

    /**
     * Selects a range of cells of a table on the page the view shows, from one cell to another,
     * both counted from 0, or no cell where the first column is below 0. The table is selected
     * as an object first where it is not.
     *
     * @param nObjectId the unique id of the table
     * @param nFirstColumn the column of the cell the range starts at, below 0 for no cell
     * @param nFirstRow the row of that cell
     * @param nLastColumn the column of the cell the range ends at
     * @param nLastRow the row of that cell
     */
    virtual void selectTableCells(unsigned long long nObjectId, int nFirstColumn, int nFirstRow,
                                  int nLastColumn, int nLastRow)
        = 0;

    /**
     * Moves the start or the end of a connector to a point and ties it to an object at one of its
     * glue points, or leaves it free there. The run is laid out anew, as after a drag of that
     * end, as one step to undo.
     *
     * @param nConnectorId the unique id of the connector
     * @param bStart true for the start, false for the end
     * @param nX where the end goes, on the page, in twips
     * @param nY where the end goes, on the page, in twips
     * @param nTiedTo the unique id of the object the end ties to, 0 for none
     * @param nGluePoint the glue point it ties to, counted as the glue point properties of a
     * connector count them: -1 for the best side, 0 to 3 for the middles of the sides, an own
     * glue point by its id and 3
     */
    virtual void setConnectorEnd(unsigned long long nConnectorId, bool bStart, int nX, int nY,
                                 unsigned long long nTiedTo, int nGluePoint)
        = 0;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
