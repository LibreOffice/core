/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_HXX
#define INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_HXX

#include <cstddef>

#include "LibreOfficeKit.h"
#include "LibreOfficeKitInit.h"

/*
 * The reasons this C++ code is not as pretty as it could be are:
 *  a) provide a pure C API - that's useful for some people
 *  b) allow ABI stability - C++ vtables are not good for that.
 *  c) avoid C++ types as part of the API.
 */
namespace lok
{

/// The lok::Document class represents one loaded document instance.
class Document
{
private:
    LibreOfficeKitDocument* mpDoc;

public:
    /// A lok::Document is typically created by the lok::Office::documentLoad() method.
    inline Document(LibreOfficeKitDocument* pDoc) :
        mpDoc(pDoc)
    {}

    inline ~Document()
    {
        mpDoc->pClass->destroy(mpDoc);
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
    inline bool saveAs(const char* pUrl, const char* pFormat = NULL, const char* pFilterOptions = NULL)
    {
        return mpDoc->pClass->saveAs(mpDoc, pUrl, pFormat, pFilterOptions) != 0;
    }

    /// Gives access to the underlying C pointer.
    inline LibreOfficeKitDocument *get() { return mpDoc; }

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
    /**
     * Get document type.
     *
     * @return an element of the LibreOfficeKitDocumentType enum.
     */
    inline int getDocumentType()
    {
        return mpDoc->pClass->getDocumentType(mpDoc);
    }

    /**
     * Get number of part that the document contains.
     *
     * Part refers to either individual sheets in a Calc, or slides in Impress,
     * and has no relevance for Writer.
     */
    inline int getParts()
    {
        return mpDoc->pClass->getParts(mpDoc);
    }

    /**
     * Get the logical rectangle of each part in the document.
     *
     * A part refers to an individual page in Writer and has no relevant for
     * Calc or Impress.
     *
     * @return a rectangle list, using the same format as
     * LOK_CALLBACK_TEXT_SELECTION.
     */
    inline char* getPartPageRectangles()
    {
        return mpDoc->pClass->getPartPageRectangles(mpDoc);
    }

    /// Get the current part of the document.
    inline int getPart()
    {
        return mpDoc->pClass->getPart(mpDoc);
    }

    /// Set the current part of the document.
    inline void setPart(int nPart)
    {
        mpDoc->pClass->setPart(mpDoc, nPart);
    }

    /// Get the current part's name.
    inline char* getPartName(int nPart)
    {
        return mpDoc->pClass->getPartName(mpDoc, nPart);
    }

    /// Get the current part's hash.
    inline char* getPartHash(int nPart)
    {
        return mpDoc->pClass->getPartHash(mpDoc, nPart);
    }

    inline void setPartMode(int nMode)
    {
        mpDoc->pClass->setPartMode(mpDoc, nMode);
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
    inline void paintTile(unsigned char* pBuffer,
                          const int nCanvasWidth,
                          const int nCanvasHeight,
                          const int nTilePosX,
                          const int nTilePosY,
                          const int nTileWidth,
                          const int nTileHeight)
    {
        return mpDoc->pClass->paintTile(mpDoc, pBuffer, nCanvasWidth, nCanvasHeight,
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
     */
    void paintWindow(unsigned nWindowId,
                     unsigned char* pBuffer,
                     const int x,
                     const int y,
                     const int width,
                     const int height)
    {
        return mpDoc->pClass->paintWindow(mpDoc, nWindowId, pBuffer,
                                          x, y, width, height);
    }

    /**
     * Posts a command to the window (dialog, popup, etc.) with given id
     *
     * @param nWindowid
     */
    void postWindow(unsigned nWindowId, int nAction)
    {
        return mpDoc->pClass->postWindow(mpDoc, nWindowId, nAction);
    }

    /**
     * Gets the tile mode: the pixel format used for the pBuffer of paintTile().
     *
     * @return an element of the LibreOfficeKitTileMode enum.
     */
    inline int getTileMode()
    {
        return mpDoc->pClass->getTileMode(mpDoc);
    }

    /// Get the document sizes in TWIPs.
    inline void getDocumentSize(long* pWidth, long* pHeight)
    {
        mpDoc->pClass->getDocumentSize(mpDoc, pWidth, pHeight);
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
    inline void initializeForRendering(const char* pArguments = NULL)
    {
        mpDoc->pClass->initializeForRendering(mpDoc, pArguments);
    }

    /**
     * Registers a callback. LOK will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    inline void registerCallback(LibreOfficeKitCallback pCallback, void* pData)
    {
        mpDoc->pClass->registerCallback(mpDoc, pCallback, pData);
    }

    /**
     * Posts a keyboard event to the focused frame.
     *
     * @param nType Event type, like press or release.
     * @param nCharCode contains the Unicode character generated by this event or 0
     * @param nKeyCode contains the integer code representing the key of the event (non-zero for control keys)
     */
    inline void postKeyEvent(int nType, int nCharCode, int nKeyCode)
    {
        mpDoc->pClass->postKeyEvent(mpDoc, nType, nCharCode, nKeyCode);
    }

    /**
     * Posts a keyboard event to the dialog
     *
     * @param nWindowId
     * @param nType Event type, like press or release.
     * @param nCharCode contains the Unicode character generated by this event or 0
     * @param nKeyCode contains the integer code representing the key of the event (non-zero for control keys)
     */
    void postWindowKeyEvent(unsigned nWindowId, int nType, int nCharCode, int nKeyCode)
    {
        mpDoc->pClass->postWindowKeyEvent(mpDoc, nWindowId, nType, nCharCode, nKeyCode);
    }

    /**
     * Posts a mouse event to the document.
     *
     * @param nType Event type, like down, move or up.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nCount number of clicks: 1 for single click, 2 for double click
     * @param nButtons: which mouse buttons: 1 for left, 2 for middle, 4 right
     * @param nModifier: which keyboard modifier: (see include/rsc/rsc-vcl-shared-types.hxx for possible values)
     */
    inline void postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
    {
        mpDoc->pClass->postMouseEvent(mpDoc, nType, nX, nY, nCount, nButtons, nModifier);
    }

    /**
     * Posts a mouse event to the window with given id.
     *
     * @param nWindowId
     * @param nType Event type, like down, move or up.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nCount number of clicks: 1 for single click, 2 for double click
     * @param nButtons: which mouse buttons: 1 for left, 2 for middle, 4 right
     * @param nModifier: which keyboard modifier: (see include/vcl/vclenum.hxx for possible values)
     */
    void postWindowMouseEvent(unsigned nWindowId, int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
    {
        mpDoc->pClass->postWindowMouseEvent(mpDoc, nWindowId, nType, nX, nY, nCount, nButtons, nModifier);
    }

    /**
     * Posts an UNO command to the document.
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
    inline void postUnoCommand(const char* pCommand, const char* pArguments = NULL, bool bNotifyWhenFinished = false)
    {
        mpDoc->pClass->postUnoCommand(mpDoc, pCommand, pArguments, bNotifyWhenFinished);
    }

    /**
     * Sets the start or end of a text selection.
     *
     * @param nType @see LibreOfficeKitSetTextSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    inline void setTextSelection(int nType, int nX, int nY)
    {
        mpDoc->pClass->setTextSelection(mpDoc, nType, nX, nY);
    }

    /**
     * Gets the currently selected text.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     * @param pUsedMimeType output parameter to inform about the determined format (suggested one or plain text).
     */
    inline char* getTextSelection(const char* pMimeType, char** pUsedMimeType = NULL)
    {
        return mpDoc->pClass->getTextSelection(mpDoc, pMimeType, pUsedMimeType);
    }

    /**
     * Pastes content at the current cursor position.
     *
     * @param pMimeType format of pData, for example text/plain;charset=utf-8.
     * @param pData the actual data to be pasted.
     * @return if the supplied data was pasted successfully.
     */
    inline bool paste(const char* pMimeType, const char* pData, size_t nSize)
    {
        return mpDoc->pClass->paste(mpDoc, pMimeType, pData, nSize);
    }

    /**
     * Adjusts the graphic selection.
     *
     * @param nType @see LibreOfficeKitSetGraphicSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    inline void setGraphicSelection(int nType, int nX, int nY)
    {
        mpDoc->pClass->setGraphicSelection(mpDoc, nType, nX, nY);
    }

    /**
     * Gets rid of any text or graphic selection.
     */
    inline void resetSelection()
    {
        mpDoc->pClass->resetSelection(mpDoc);
    }

    /**
     * Returns a json mapping of the possible values for the given command
     * e.g. {commandName: ".uno:StyleApply", commandValues: {"familyName1" : ["list of style names in the family1"], etc.}}
     * @param pCommand a uno command for which the possible values are requested
     * @return {commandName: unoCmd, commandValues: {possible_values}}
     */
    inline char* getCommandValues(const char* pCommand)
    {
        return mpDoc->pClass->getCommandValues(mpDoc, pCommand);
    }

    /**
     * Save the client's view so that we can compute the right zoom level
     * for the mouse events. This only affects CALC.
     * @param nTilePixelWidth - tile width in pixels
     * @param nTilePixelHeight - tile height in pixels
     * @param nTileTwipWidth - tile width in twips
     * @param nTileTwipHeight - tile height in twips
     */
    inline void setClientZoom(
            int nTilePixelWidth,
            int nTilePixelHeight,
            int nTileTwipWidth,
            int nTileTwipHeight)
    {
        mpDoc->pClass->setClientZoom(mpDoc, nTilePixelWidth, nTilePixelHeight, nTileTwipWidth, nTileTwipHeight);
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
    inline void setClientVisibleArea(int nX, int nY, int nWidth, int nHeight)
    {
        mpDoc->pClass->setClientVisibleArea(mpDoc, nX, nY, nWidth, nHeight);
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
        mpDoc->pClass->setOutlineState(mpDoc, bColumn, nLevel, nIndex, bHidden);
    }

    /**
     * Create a new view for an existing document.
     * By default a loaded document has 1 view.
     * @return the ID of the new view.
     */
    int createView()
    {
        return mpDoc->pClass->createView(mpDoc);
    }

    /**
     * Destroy a view of an existing document.
     * @param nId a view ID, returned by createView().
     */
    void destroyView(int nId)
    {
        mpDoc->pClass->destroyView(mpDoc, nId);
    }

    /**
     * Set an existing view of an existing document as current.
     * @param nId a view ID, returned by createView().
     */
    void setView(int nId)
    {
        mpDoc->pClass->setView(mpDoc, nId);
    }

    /**
     * Get the current view.
     * @return a view ID, previously returned by createView().
     */
    int getView()
    {
        return mpDoc->pClass->getView(mpDoc);
    }

    /**
     * Get number of views of this document.
     */
    inline int getViewsCount()
    {
        return mpDoc->pClass->getViewsCount(mpDoc);
    }

    /**
     * Paints a font name or character if provided to be displayed in the font list
     * @param pFontName the font to be painted
     */
    inline unsigned char* renderFont(const char *pFontName,
                          const char *pChar,
                          int *pFontWidth,
                          int *pFontHeight)
    {
        return mpDoc->pClass->renderFont(mpDoc, pFontName, pChar, pFontWidth, pFontHeight);
    }

    /**
     * Renders a subset of the document's part to a pre-allocated buffer.
     *
     * @param nPart the part number of the document of which the tile is painted.
     * @see paintTile.
     */
    inline void paintPartTile(unsigned char* pBuffer,
                              const int nPart,
                              const int nCanvasWidth,
                              const int nCanvasHeight,
                              const int nTilePosX,
                              const int nTilePosY,
                              const int nTileWidth,
                              const int nTileHeight)
    {
        return mpDoc->pClass->paintPartTile(mpDoc, pBuffer, nPart,
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
    inline bool getViewIds(int* pArray,
                           size_t nSize)
    {
        return mpDoc->pClass->getViewIds(mpDoc, pArray, nSize);
    }

    /**
     * Set the language tag of the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param language Bcp47 languageTag, like en-US or so.
     */
    void setViewLanguage(int nId, const char* language)
    {
        mpDoc->pClass->setViewLanguage(mpDoc, nId, language);
    }

    /**
     * Post the text input from external input window, like IME
     *
     * @param nType see LibreOfficeKitExtTextInputType
     * @param pText Text for LOK_EXT_TEXTINPUT
     */
    void postExtTextInputEvent(int nType, const char* pText)
    {
        mpDoc->pClass->postExtTextInputEvent(mpDoc, nType, pText);
    }

#endif // defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
};

/// The lok::Office class represents one started LibreOfficeKit instance.
class Office
{
private:
    LibreOfficeKit* mpThis;

public:
    /// A lok::Office is typically created by the lok_cpp_init() function.
    inline Office(LibreOfficeKit* pThis) :
        mpThis(pThis)
    {}

    inline ~Office()
    {
        mpThis->pClass->destroy(mpThis);
    }

    /**
     * Loads a document from an URL.
     *
     * @param pUrl the URL of the document to load
     * @param pFilterOptions options for the import filter, e.g. SkipImages.
     *        Another useful FilterOption is "Language=...".  It is consumed
     *        by the documentLoad() itself, and when provided, LibreOfficeKit
     *        switches the language accordingly first.
     * @since pFilterOptions argument added in LibreOffice 5.0
     */
    inline Document* documentLoad(const char* pUrl, const char* pFilterOptions = NULL)
    {
        LibreOfficeKitDocument* pDoc = NULL;

        if (LIBREOFFICEKIT_HAS(mpThis, documentLoadWithOptions))
            pDoc = mpThis->pClass->documentLoadWithOptions(mpThis, pUrl, pFilterOptions);
        else
            pDoc = mpThis->pClass->documentLoad(mpThis, pUrl);

        if (pDoc == NULL)
            return NULL;

        return new Document(pDoc);
    }

    /// Returns the last error as a string, the returned pointer has to be freed by the caller.
    inline char* getError()
    {
        return mpThis->pClass->getError(mpThis);
    }

    /**
     * Frees the memory pointed to by pFree.
     *
     * @since LibreOffice 5.2
     */
    inline void freeError(char* pFree)
    {
        mpThis->pClass->freeError(pFree);
    }

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
    /**
     * Registers a callback. LOK will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    inline void registerCallback(LibreOfficeKitCallback pCallback, void* pData)
    {
        mpThis->pClass->registerCallback(mpThis, pCallback, pData);
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
     */
    inline char* getFilterTypes()
    {
        return mpThis->pClass->getFilterTypes(mpThis);
    }

    /**
     * Set bitmask of optional features supported by the client.
     *
     * @see LibreOfficeKitOptionalFeatures
     */
    void setOptionalFeatures(unsigned long long features)
    {
        return mpThis->pClass->setOptionalFeatures(mpThis, features);
    }

    /**
     * Set password required for loading or editing a document.
     *
     * Loading the document is blocked until the password is provided.
     *
     * @param pURL      the URL of the document, as sent to the callback
     * @param pPassword the password, nullptr indicates no password
     *
     * In response to LOK_CALLBACK_DOCUMENT_PASSWORD, a vaild password
     * will continue loading the document, an invalid password will
     * result in another LOK_CALLBACK_DOCUMENT_PASSWORD request,
     * and a NULL password will abort loading the document.
     *
     * In response to LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY, a vaild
     * password will continue loading the document, an invalid password will
     * result in another LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY request,
     * and a NULL password will continue loading the document in read-only
     * mode.
     */
    inline void setDocumentPassword(char const* pURL, char const* pPassword)
    {
        mpThis->pClass->setDocumentPassword(mpThis, pURL, pPassword);
    }

    /**
     * Get version information of the LOKit process
     *
     * @returns JSON string containing version information in format:
     * {ProductName: <>, ProductVersion: <>, ProductExtension: <>, BuildId: <>}
     *
     * Eg: {"ProductName": "LibreOffice",
     * "ProductVersion": "5.3",
     * "ProductExtension": ".0.0.alpha0",
     * "BuildId": "<full 40 char git hash>"}
     */
    inline char* getVersionInfo()
    {
        return mpThis->pClass->getVersionInfo(mpThis);
    }

    /**
     * Run a macro.
     *
     * Same syntax as on command line is permissible (ie. the macro:// URI forms)
     *
     * @param pURL macro url to run
     */

    inline bool runMacro( const char* pURL)
    {
        return mpThis->pClass->runMacro( mpThis, pURL );
    }
#endif // defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
};

/// Factory method to create a lok::Office instance.
inline Office* lok_cpp_init(const char* pInstallPath, const char* pUserProfileUrl = NULL)
{
    LibreOfficeKit* pThis = lok_init_2(pInstallPath, pUserProfileUrl);
    if (pThis == NULL || pThis->pClass->nSize == 0)
        return NULL;
    return new ::lok::Office(pThis);
}

}

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
