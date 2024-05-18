/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKitInit.h>

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
    Document(LibreOfficeKitDocument* pDoc) :
        mpDoc(pDoc)
    {}

    ~Document()
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
    bool saveAs(const char* pUrl, const char* pFormat = NULL, const char* pFilterOptions = NULL)
    {
        return mpDoc->pClass->saveAs(mpDoc, pUrl, pFormat, pFilterOptions) != 0;
    }

    /// Gives access to the underlying C pointer.
    LibreOfficeKitDocument *get() { return mpDoc; }

    /**
     * Get document type.
     *
     * @since LibreOffice 6.0
     * @return an element of the LibreOfficeKitDocumentType enum.
     */
    int getDocumentType()
    {
        return mpDoc->pClass->getDocumentType(mpDoc);
    }

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
    /**
     * Get number of part that the document contains.
     *
     * Part refers to either individual sheets in a Calc, or slides in Impress,
     * and has no relevance for Writer.
     */
    int getParts()
    {
        return mpDoc->pClass->getParts(mpDoc);
    }

    /**
     * Get the extent of each page in the document.
     *
     * This function is relevant for Writer documents only. It is a
     * mistake that the API has "part" in its name as Writer documents
     * don't have parts.
     *
     * @return a rectangle list, using the same format as
     * LOK_CALLBACK_TEXT_SELECTION.
     */
    char* getPartPageRectangles()
    {
        return mpDoc->pClass->getPartPageRectangles(mpDoc);
    }

    /// Get the current part of the document.
    int getPart()
    {
        return mpDoc->pClass->getPart(mpDoc);
    }

    /// Set the current part of the document.
    void setPart(int nPart)
    {
        mpDoc->pClass->setPart(mpDoc, nPart);
    }

    /// Get the current part's name.
    char* getPartName(int nPart)
    {
        return mpDoc->pClass->getPartName(mpDoc, nPart);
    }

    /// Get the current part's hash.
    char* getPartHash(int nPart)
    {
        return mpDoc->pClass->getPartHash(mpDoc, nPart);
    }

    void setPartMode(int nMode)
    {
        mpDoc->pClass->setPartMode(mpDoc, nMode);
    }

    int getEditMode()
    {
        return mpDoc->pClass->getEditMode(mpDoc);
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
        return mpDoc->pClass->paintWindowForView(mpDoc, nWindowId, pBuffer, x, y,
                                                 width, height, dpiscale, viewId);
    }

    /**
     * Posts a command to the window (dialog, popup, etc.) with given id
     *
     * @param nWindowid
     */
    void postWindow(unsigned nWindowId, int nAction, const char* pData = nullptr)
    {
        return mpDoc->pClass->postWindow(mpDoc, nWindowId, nAction, pData);
    }

    /**
     * Gets the tile mode: the pixel format used for the pBuffer of paintTile().
     *
     * @return an element of the LibreOfficeKitTileMode enum.
     */
    int getTileMode()
    {
        return mpDoc->pClass->getTileMode(mpDoc);
    }

    /// Get the document sizes in TWIPs.
    void getDocumentSize(long* pWidth, long* pHeight)
    {
        mpDoc->pClass->getDocumentSize(mpDoc, pWidth, pHeight);
    }

    /// Get the data area (in Calc last row and column).
    void getDataArea(long nPart, long* pCol, long* pRow)
    {
        mpDoc->pClass->getDataArea(mpDoc, nPart, pCol, pRow);
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
        mpDoc->pClass->initializeForRendering(mpDoc, pArguments);
    }

    /**
     * Registers a callback. LOK will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    void registerCallback(LibreOfficeKitCallback pCallback, void* pData)
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
    void postKeyEvent(int nType, int nCharCode, int nKeyCode)
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
     * @param nModifier: which keyboard modifier: (see include/vcl/vclenum.hxx for possible values)
     */
    void postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
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
     * Posts a dialog event for the window with given id
     *
     * @param nWindowId id of the window to notify
     * @param pArguments arguments of the event.
     */
    void sendDialogEvent(unsigned long long int nWindowId, const char* pArguments = NULL)
    {
        mpDoc->pClass->sendDialogEvent(mpDoc, nWindowId, pArguments);
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
        mpDoc->pClass->postUnoCommand(mpDoc, pCommand, pArguments, bNotifyWhenFinished);
    }

    /**
     * Sets the start or end of a text selection.
     *
     * @param nType @see LibreOfficeKitSetTextSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    void setTextSelection(int nType, int nX, int nY)
    {
        mpDoc->pClass->setTextSelection(mpDoc, nType, nX, nY);
    }

    /**
     * Gets the currently selected text.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     * @param pUsedMimeType output parameter to inform about the determined format (suggested one or plain text).
     */
    char* getTextSelection(const char* pMimeType, char** pUsedMimeType = NULL)
    {
        return mpDoc->pClass->getTextSelection(mpDoc, pMimeType, pUsedMimeType);
    }

    /**
     * Gets the type of the selected content.
     *
     * In most cases it is more efficient to use getSelectionTypeAndText().
     *
     * @return an element of the LibreOfficeKitSelectionType enum.
     */
    int getSelectionType()
    {
        return mpDoc->pClass->getSelectionType(mpDoc);
    }

    /**
     * Gets the type of the selected content and possibly its text.
     *
     * This function is a more efficient combination of getSelectionType() and getTextSelection().
     * It returns the same as getSelectionType(), and additionally if the return value is
     * LOK_SELTYPE_TEXT then it also returns the same as getTextSelection(), otherwise
     * pText and pUsedMimeType are unchanged.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     * @param pText the currently selected text
     * @param pUsedMimeType output parameter to inform about the determined format (suggested one or plain text).
     * @return an element of the LibreOfficeKitSelectionType enum.
     * @since LibreOffice 7.4
     */
    int getSelectionTypeAndText(const char* pMimeType, char** pText, char** pUsedMimeType = NULL)
    {
        if (LIBREOFFICEKIT_DOCUMENT_HAS(mpDoc, getSelectionTypeAndText))
            return mpDoc->pClass->getSelectionTypeAndText(mpDoc, pMimeType, pText, pUsedMimeType);
        int type = mpDoc->pClass->getSelectionType(mpDoc);
        if(type == LOK_SELTYPE_TEXT && pText)
            *pText = mpDoc->pClass->getTextSelection(mpDoc, pMimeType, pUsedMimeType);
        return type;
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
        return mpDoc->pClass->getClipboard(mpDoc, pMimeTypes, pOutCount, pOutMimeTypes, pOutSizes, pOutStreams);
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
        return mpDoc->pClass->setClipboard(mpDoc, nInCount, pInMimeTypes, pInSizes, pInStreams);
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
        return mpDoc->pClass->paste(mpDoc, pMimeType, pData, nSize);
    }

    /**
     * Adjusts the graphic selection.
     *
     * @param nType @see LibreOfficeKitSetGraphicSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    void setGraphicSelection(int nType, int nX, int nY)
    {
        mpDoc->pClass->setGraphicSelection(mpDoc, nType, nX, nY);
    }

    /**
     * Gets rid of any text or graphic selection.
     */
    void resetSelection()
    {
        mpDoc->pClass->resetSelection(mpDoc);
    }

    /**
     * Returns a json mapping of the possible values for the given command
     * e.g. {commandName: ".uno:StyleApply", commandValues: {"familyName1" : ["list of style names in the family1"], etc.}}
     * @param pCommand a UNO command for which the possible values are requested
     * @return {commandName: unoCmd, commandValues: {possible_values}}
     */
    char* getCommandValues(const char* pCommand)
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
    void setClientZoom(
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
    void setClientVisibleArea(int nX, int nY, int nWidth, int nHeight)
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
     * Create a new view for an existing document with
     * options similar to documentLoadWithOptions.
     * By default a loaded document has 1 view.
     * @return the ID of the new view.
     */
    int createView(const char* pOptions = nullptr)
    {
        if (LIBREOFFICEKIT_DOCUMENT_HAS(mpDoc, createViewWithOptions))
            return mpDoc->pClass->createViewWithOptions(mpDoc, pOptions);
        else
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
    int getViewsCount()
    {
        return mpDoc->pClass->getViewsCount(mpDoc);
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
        if (LIBREOFFICEKIT_DOCUMENT_HAS(mpDoc, renderFontOrientation))
            return mpDoc->pClass->renderFontOrientation(mpDoc, pFontName, pChar, pFontWidth, pFontHeight, pOrientation);
        else
            return mpDoc->pClass->renderFont(mpDoc, pFontName, pChar, pFontWidth, pFontHeight);
    }

    /**
     * Renders a subset of the document's part to a pre-allocated buffer.
     *
     * @param nPart the part number of the document of which the tile is painted.
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
        return mpDoc->pClass->paintPartTile(mpDoc, pBuffer, nPart, nMode,
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
     * Post the text input from external input window, like IME, to given windowId
     *
     * @param nWindowId Specify the window id to post the input event to. If
     * nWindow is 0, the event is posted into the document
     * @param nType see LibreOfficeKitExtTextInputType
     * @param pText Text for LOK_EXT_TEXTINPUT
     */
    void postWindowExtTextInputEvent(unsigned nWindowId, int nType, const char* pText)
    {
        mpDoc->pClass->postWindowExtTextInputEvent(mpDoc, nWindowId, nType, pText);
    }

    /**
     *  Insert certificate (in binary form) to the certificate store.
     */
    bool insertCertificate(const unsigned char* pCertificateBinary,
                           const int pCertificateBinarySize,
                           const unsigned char* pPrivateKeyBinary,
                           const int nPrivateKeyBinarySize)
    {
        return mpDoc->pClass->insertCertificate(mpDoc,
                                                pCertificateBinary, pCertificateBinarySize,
                                                pPrivateKeyBinary, nPrivateKeyBinarySize);
    }

    /**
     *  Add the certificate (in binary form) to the certificate store.
     *
     */
    bool addCertificate(const unsigned char* pCertificateBinary,
                         const int pCertificateBinarySize)
    {
        return mpDoc->pClass->addCertificate(mpDoc,
                                             pCertificateBinary, pCertificateBinarySize);
    }

    /**
     *  Verify signature of the document.
     *
     *  Check possible values in include/sfx2/signaturestate.hxx
     */
    int getSignatureState()
    {
        return mpDoc->pClass->getSignatureState(mpDoc);
    }

    /**
     * Gets an image of the selected shapes.
     * @param pOutput contains the result; use free to deallocate.
     * @return the size of *pOutput in bytes.
     */
    size_t renderShapeSelection(char** pOutput)
    {
        return mpDoc->pClass->renderShapeSelection(mpDoc, pOutput);
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
        return mpDoc->pClass->postWindowGestureEvent(mpDoc, nWindowId, pType, nX, nY, nOffset);
    }

    /// Set a part's selection mode.
    /// nSelect is 0 to deselect, 1 to select, and 2 to toggle.
    void selectPart(int nPart, int nSelect)
    {
        mpDoc->pClass->selectPart(mpDoc, nPart, nSelect);
    }

    /// Moves the selected pages/slides to a new position.
    /// nPosition is the new position where the selection
    /// should go. bDuplicate when true will copy instead of move.
    void moveSelectedParts(int nPosition, bool bDuplicate)
    {
        mpDoc->pClass->moveSelectedParts(mpDoc, nPosition, bDuplicate);
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
        return mpDoc->pClass->resizeWindow(mpDoc, nWindowId, width, height);
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
        mpDoc->pClass->removeTextContext(mpDoc, nWindowId, nBefore, nAfter);
    }

    /**
     * Select the Calc function to be pasted into the formula input box
     *
     * @param nIndex is the index of the selected function
     */
    void completeFunction(const char* pFunctionName)
    {
        mpDoc->pClass->completeFunction(mpDoc, pFunctionName);
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
        mpDoc->pClass->setWindowTextSelection(mpDoc, nWindowId, bSwap, nX, nY);
    }

    /**
     * Posts an event for the form field at the cursor position.
     *
     * @param pArguments arguments of the event.
     */
    void sendFormFieldEvent(const char* pArguments)
    {
        mpDoc->pClass->sendFormFieldEvent(mpDoc, pArguments);
    }

    void setBlockedCommandList(int nViewId, const char* blockedCommandList)
    {
        mpDoc->pClass->setBlockedCommandList(mpDoc, nViewId, blockedCommandList);
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
        return mpDoc->pClass->renderSearchResult(mpDoc, pSearchResult, pBitmapBuffer, pWidth, pHeight, pByteSize);
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
        mpDoc->pClass->sendContentControlEvent(mpDoc, pArguments);
    }

    /**
     * Set the timezone of the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param timezone a timezone in the tzfile(5) format (e.g. Pacific/Auckland).
     */
    void setViewTimezone(int nId, const char* timezone)
    {
        mpDoc->pClass->setViewTimezone(mpDoc, nId, timezone);
    }

    /** Set if the view should be treated as readonly or not.
     *
     * @param nId view ID
     * @param readOnly
    */
    void setViewReadOnly(int nId, const bool readOnly)
    {
        mpDoc->pClass->setViewReadOnly(mpDoc, nId, readOnly);
    }

    /** Set if the view can edit comments on readonly mode or not.
     *
     * @param nId view ID
     * @param allow
    */
    void setAllowChangeComments(int nId, const bool allow)
    {
        mpDoc->pClass->setAllowChangeComments(mpDoc, nId, allow);
    }

    /**
     * Enable/Disable accessibility support for the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param nEnabled true/false
     */
    void setAccessibilityState(int nId, bool nEnabled)
    {
        mpDoc->pClass->setAccessibilityState(mpDoc, nId, nEnabled);
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
        return mpDoc->pClass->getA11yFocusedParagraph(mpDoc);
    }

    /// Get the current text cursor position.
    int getA11yCaretPosition()
    {
        return mpDoc->pClass->getA11yCaretPosition(mpDoc);
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
    Office(LibreOfficeKit* pThis) :
        mpThis(pThis)
    {}

    ~Office()
    {
        mpThis->pClass->destroy(mpThis);
    }

    /**
     * Loads a document from a URL.
     *
     * @param pUrl the URL of the document to load
     * @param pFilterOptions options for the import filter, e.g. SkipImages.
     *        Another useful FilterOption is "Language=...".  It is consumed
     *        by the documentLoad() itself, and when provided, LibreOfficeKit
     *        switches the language accordingly first.
     * @since pFilterOptions argument added in LibreOffice 5.0
     */
    Document* documentLoad(const char* pUrl, const char* pFilterOptions = NULL)
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
    char* getError()
    {
        return mpThis->pClass->getError(mpThis);
    }

    /**
     * Frees the memory pointed to by pFree.
     *
     * @since LibreOffice 5.2
     */
    void freeError(char* pFree)
    {
        mpThis->pClass->freeError(pFree);
    }

    /**
     * Registers a callback. LOK will invoke this function when it wants to
     * inform the client about events.
     *
     * @since LibreOffice 6.0
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    void registerCallback(LibreOfficeKitCallback pCallback, void* pData)
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
     *
     * @since LibreOffice 6.0
     */
    char* getFilterTypes()
    {
        return mpThis->pClass->getFilterTypes(mpThis);
    }

    /**
     * Set bitmask of optional features supported by the client.
     *
     * @since LibreOffice 6.0
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
     * In response to LOK_CALLBACK_DOCUMENT_PASSWORD, a valid password
     * will continue loading the document, an invalid password will
     * result in another LOK_CALLBACK_DOCUMENT_PASSWORD request,
     * and a NULL password will abort loading the document.
     *
     * In response to LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY, a valid
     * password will continue loading the document, an invalid password will
     * result in another LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY request,
     * and a NULL password will continue loading the document in read-only
     * mode.
     *
     * @since LibreOffice 6.0
     */
    void setDocumentPassword(char const* pURL, char const* pPassword)
    {
        mpThis->pClass->setDocumentPassword(mpThis, pURL, pPassword);
    }

    /**
     * Get version information of the LOKit process
     *
     * @since LibreOffice 6.0
     * @returns JSON string containing version information in format:
     * {ProductName: <>, ProductVersion: <>, ProductExtension: <>, BuildId: <>}
     *
     * Eg: {"ProductName": "LibreOffice",
     * "ProductVersion": "5.3",
     * "ProductExtension": ".0.0.alpha0",
     * "BuildId": "<full 40 char git hash>"}
     */
    char* getVersionInfo()
    {
        return mpThis->pClass->getVersionInfo(mpThis);
    }

    /**
     * Run a macro.
     *
     * Same syntax as on command line is permissible (ie. the macro:// URI forms)
     *
     * @since LibreOffice 6.0
     * @param pURL macro url to run
     */
    bool runMacro( const char* pURL)
    {
        return mpThis->pClass->runMacro( mpThis, pURL );
    }

    /**
     * Exports the document and signs its content.
     */
    bool signDocument(const char* pURL,
                       const unsigned char* pCertificateBinary, const int nCertificateBinarySize,
                       const unsigned char* pPrivateKeyBinary, const int nPrivateKeyBinarySize)
    {
        return mpThis->pClass->signDocument(mpThis, pURL,
                                            pCertificateBinary, nCertificateBinarySize,
                                            pPrivateKeyBinary, nPrivateKeyBinarySize);
    }

    /**
     * Runs the main-loop in the current thread. To trigger this
     * mode you need to putenv a SAL_LOK_OPTIONS containing 'unipoll'.
     * The @pPollCallback is called to poll for events from the Kit client
     * and the @pWakeCallback can be called by internal LibreOfficeKit threads
     * to wake the caller of 'runLoop' ie. the main thread.
     *
     * it is expected that runLoop does not return until Kit exit.
     *
     * @pData is a context/closure passed to both methods.
     */
    void runLoop(LibreOfficeKitPollCallback pPollCallback,
                 LibreOfficeKitWakeCallback pWakeCallback,
                 void* pData)
    {
        mpThis->pClass->runLoop(mpThis, pPollCallback, pWakeCallback, pData);
    }

    /**
     * Posts a dialog event for the window with given id
     *
     * @param nWindowId id of the window to notify
     * @param pArguments arguments of the event.
     */
    void sendDialogEvent(unsigned long long int nWindowId, const char* pArguments = NULL)
    {
        mpThis->pClass->sendDialogEvent(mpThis, nWindowId, pArguments);
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
     * logging functionality" in LibreOffice internal API
     * documentation (include/sal/log.hxx). If the logging selector
     * has been set by this function to a non-empty value, that is used
     * instead of the environment variable SAL_LOG.
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
        mpThis->pClass->setOption(mpThis, pOption, pValue);
    }

    /**
     * Debugging tool for triggering a dump of internal state.
     *
     * LibreOfficeKit can get into an unhelpful state at run-time when
     * in heavy use. This provides a critical tool for inspecting
     * relevant internal state.
     *
     * @param pOption future expansion - string options.
     * @param pState - heap allocated, C string containing the state dump.
     */
    void dumpState(const char* pOption, char** pState)
    {
        mpThis->pClass->dumpState(mpThis, pOption, pState);
    }

    char* extractRequest(const char* pFilePath)
    {
        return mpThis->pClass->extractRequest(mpThis, pFilePath);
    }

    /**
     * Trim memory usage.
     *
     * LibreOfficeKit caches lots of information from large pixmaps
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
        mpThis->pClass->trimMemory(mpThis, nTarget);
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
        return mpThis->pClass->startURP(mpThis, pReceiveURPFromLOContext, pSendURPToLOContext,
                                        fnReceiveURPFromLO, fnSendURPToLO);
    }

    /**
     * Stop a function based URP connection you previously started with startURP
     *
     * @param pURPContext the context returned by startURP  when starting the connection
     */
    void stopURP(void* pURPContext)
    {
        mpThis->pClass->stopURP(mpThis, pURPContext);
    }

    /**
     * Joins all threads if possible to get down to a single process
     * which can be forked from safely.
     *
     * @returns non-zero for successful join, 0 for failure.
     */
    int joinThreads()
    {
        return mpThis->pClass->joinThreads(mpThis);
    }

    /**
     * Starts all threads that are necessary to continue working
     * after a joinThreads().
     */
    void startThreads()
    {
        mpThis->pClass->startThreads(mpThis);
    }

    /**
     * Informs that this process is either a parent, or a child
     * process post-fork, allowing improved resource sharing.
     */
    void setForkedChild(bool bIsChild)
    {
        return mpThis->pClass->setForkedChild(mpThis, bIsChild);
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
