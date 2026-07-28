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

// the API needs C99's bool
# ifndef _WIN32
#  include <stdbool.h>
# endif
# include <stdint.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "COKitEnums.h"

/** @see kit::Office::registerCallback(). */
typedef void (*COKitCallback)(int nType, const char* pPayload, void* pData);

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

typedef struct COKitClipboardProviderStruct COKitClipboardProvider;

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
struct COKitClipboardProviderStruct
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

typedef struct COKitStruct COKit;
typedef struct COKitClassStruct COKitClass;

typedef struct COKitDocumentStruct COKitDocument;
typedef struct COKitDocumentClassStruct COKitDocumentClass;

struct COKitStruct
{
    COKitClass* pClass;
};

struct COKitClassStruct
{
    void (*destroy) (COKit* pThis);

    COKitDocument* (*documentLoad) (COKit* pThis,
                                             const char* pURL);

    char* (*getError) (COKit* pThis);

    COKitDocument* (*documentLoadWithOptions) (COKit* pThis,
                                                        const char* pURL,
                                                        const char* pOptions);

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

    void (*freeError) (char* pFree);

    void (*registerCallback) (COKit* pThis,
                              COKitCallback pCallback,
                              void* pData);

    /** @see kit::Office::getFilterTypes(). */
    char* (*getFilterTypes) (COKit* pThis);

    /** @see kit::Office::setOptionalFeatures(). */
    void (*setOptionalFeatures)(COKit* pThis, unsigned long long features);

    /** @see kit::Office::setDocumentPassword(). */
    void (*setDocumentPassword) (COKit* pThis,
            char const* pURL,
            char const* pPassword);

    /** @see kit::Office::getVersionInfo(). */
    char* (*getVersionInfo) (COKit* pThis);

    /** @see kit::Office::runMacro(). */
    int (*runMacro) (COKit *pThis, const char* pURL);

    /** @see kit::Office::signDocument(). */
     bool (*signDocument) (COKit* pThis,
                           const char* pUrl,
                           const unsigned char* pCertificateBinary,
                           const int nCertificateBinarySize,
                           const unsigned char* pPrivateKeyBinary,
                           const int nPrivateKeyBinarySize);

    /// @see kit::Office::runLoop()
    void (*runLoop) (COKit* pThis,
                     COKitPollCallback pPollCallback,
                     COKitWakeCallback pWakeCallback,
                     void* pData);

    /// @see kit::Office::sendDialogEvent
    void (*sendDialogEvent) (COKit* pThis,
                            unsigned long long int nKitWindowId,
                            const char* pArguments);

    /// @see kit::Office::setOption
    void (*setOption) (COKit* pThis, const char* pOption, const char* pValue);

    /// @see kit::Office::dumpState
    void (*dumpState) (COKit* pThis, const char* pOptions, char** pState);

    /** @see kit::Office::extractRequest.
     */
    char* (*extractRequest) (COKit* pThis,
                           const char* pFilePath);

    /// @see kit::Office::trimMemory
    void (*trimMemory) (COKit* pThis, int nTarget);

    /// @see kit::Office::startURP
    void* (*startURP)(COKit* pThis,
                    void* pReceiveURPFromLOContext, void* pSendURPToLOContext,
                    int (*fnReceiveURPFromLO)(void* pContext, const signed char* pBuffer, int nLen),
                    int (*fnSendURPToLO)(void* pContext, signed char* pBuffer, int nLen));

    /// @see kit::Office::stopURP
    void (*stopURP)(COKit* pThis, void* pSendURPToLOContext);

    /// @see kit::Office::joinThreads
    int (*joinThreads)(COKit* pThis);

    /// @see kit::Office::startThreads
    void (*startThreads)(COKit* pThis);

    /// @see kit::Office::setForkedChild
    void (*setForkedChild)(COKit* pThis, bool bIsChild);

    /** @see kit::Office::extractDocumentStructureRequest.
     */
    char* (*extractDocumentStructureRequest)(COKit* pThis, const char* pFilePath,
                                             const char* pFilter);

    /// @see kit::Office::registerAnyInputCallback()
    void (*registerAnyInputCallback)(COKit* pThis,
                                     COKitAnyInputCallback pCallback, void* pData);

    /// @see kit::Office::getDocsCount().
    int (*getDocsCount) (COKit* pThis);

    /// @see kit::Office::registerFileSaveDialogCallback()
    void (*registerFileSaveDialogCallback)(COKit* pThis,
            COKitFileSaveDialogCallback pCallback);

    /// @see kit::Office::executeScript().
    void (*executeScript) (
        char const * script, char ** result, char ** error,
        void (*proxyCallback) (void * data, char const * payload),
        void * proxyCallbackData, bool * usedLegacyUnoApi);

    /// @see kit::Office::deliverProxyResult().
    void (*deliverProxyResult) (char const * callId, char const * jsonValue);

    /// @see kit::Office::cancelProxyCalls().
    void (*cancelProxyCalls) (void);

    /// @see kit::Office::isExpectedReentry().
    int (*isExpectedReentry) (void);

    /// @see kit::Office::takeLegacyUnoApiUseFlag().
    bool (*takeLegacyUnoApiUseFlag) (void);

    /// @see kit::Office::registerRevealInFileManagerCallback()
    void (*registerRevealInFileManagerCallback)(COKit* pThis,
            COKitRevealInFileManagerCallback pCallback);

    /** @see kit::Office::installClipboardProvider(). */
    void (*installClipboardProvider) (COKit* pThis,
                                      const COKitClipboardProvider* pProvider);

    /** @see kit::Office::getGlobalClipboard(). */
    int (*getGlobalClipboard) (COKit* pThis,
                               const char **pMimeTypes,
                               size_t      *pOutCount,
                               char      ***pOutMimeTypes,
                               size_t     **pOutSizes,
                               char      ***pOutStreams);
};

struct COKitDocumentStruct
{
    COKitDocumentClass* pClass;
};

struct COKitDocumentClassStruct
{
    void (*destroy) (COKitDocument* pThis);

    int (*saveAs) (COKitDocument* pThis,
                   const char* pUrl,
                   const char* pFormat,
                   const char* pFilterOptions);

    /** @see kit::Document::getDocumentType(). */
    int (*getDocumentType) (COKitDocument* pThis);

    /// @see kit::Document::getParts().
    int (*getParts) (COKitDocument* pThis);

    /// @see kit::Document::getPartPageRectangles().
    char* (*getPartPageRectangles) (COKitDocument* pThis);

    /// @see kit::Document::getPart().
    int (*getPart) (COKitDocument* pThis);

    /// @see kit::Document::setPart().
    void (*setPart) (COKitDocument* pThis,
                     int nPart);

    /// @see kit::Document::getPartName().
    char* (*getPartName) (COKitDocument* pThis,
                          int nPart);

    /// @see kit::Document::setPartMode().
    void (*setPartMode) (COKitDocument* pThis,
                         int nMode);

    /// @see kit::Document::paintTile().
    void (*paintTile) (COKitDocument* pThis,
                       unsigned char* pBuffer,
                       const int nCanvasWidth,
                       const int nCanvasHeight,
                       const int nTilePosX,
                       const int nTilePosY,
                       const int nTileWidth,
                       const int nTileHeight);

    /// @see kit::Document::getTileMode().
    int (*getTileMode) (COKitDocument* pThis);

    /// @see kit::Document::getDocumentSize().
    void (*getDocumentSize) (COKitDocument* pThis,
                             long* pWidth,
                             long* pHeight);

    /// @see kit::Document::initializeForRendering().
    void (*initializeForRendering) (COKitDocument* pThis,
                                    const char* pArguments);

    /// @see kit::Document::registerCallback().
    void (*registerCallback) (COKitDocument* pThis,
                              COKitCallback pCallback,
                              void* pData);

    /// @see kit::Document::postKeyEvent
    void (*postKeyEvent) (COKitDocument* pThis,
                          int nType,
                          int nCharCode,
                          int nKeyCode);

    /// @see kit::Document::postMouseEvent
    void (*postMouseEvent) (COKitDocument* pThis,
                            int nType,
                            int nX,
                            int nY,
                            int nCount,
                            int nButtons,
                            int nModifier);

    /// @see kit::Document::postUnoCommand
    void (*postUnoCommand) (COKitDocument* pThis,
                            const char* pCommand,
                            const char* pArguments,
                            bool bNotifyWhenFinished);

    /// @see kit::Document::setTextSelection
    void (*setTextSelection) (COKitDocument* pThis,
                              int nType,
                              int nX,
                              int nY);

    /// @see kit::Document::getTextSelection
    char* (*getTextSelection) (COKitDocument* pThis,
                               const char* pMimeType,
                               char** pUsedMimeType);

    /// @see kit::Document::paste().
    bool (*paste) (COKitDocument* pThis,
                   const char* pMimeType,
                   const char* pData,
                   size_t nSize);

    /// @see kit::Document::setGraphicSelection
    void (*setGraphicSelection) (COKitDocument* pThis,
                                 int nType,
                                 int nX,
                                 int nY);

    /// @see kit::Document::resetSelection
    void (*resetSelection) (COKitDocument* pThis);

    /// @see kit::Document::getCommandValues().
    char* (*getCommandValues) (COKitDocument* pThis, const char* pCommand);

    /// @see kit::Document::setClientZoom().
    void (*setClientZoom) (COKitDocument* pThis,
            int nTilePixelWidth,
            int nTilePixelHeight,
            int nTileTwipWidth,
            int nTileTwipHeight);

    /// @see kit::Document::setVisibleArea).
    void (*setClientVisibleArea) (COKitDocument* pThis, int nX, int nY, int nWidth, int nHeight);

    /// @see kit::Document::createView().
    int (*createView) (COKitDocument* pThis);
    /// @see kit::Document::destroyView().
    void (*destroyView) (COKitDocument* pThis, int nId);
    /// @see kit::Document::setView().
    void (*setView) (COKitDocument* pThis, int nId);
    /// @see kit::Document::getView().
    int (*getView) (COKitDocument* pThis);
    /// @see kit::Document::getViewsCount().
    int (*getViewsCount) (COKitDocument* pThis);

    /// @see kit::Document::getPartHash().
    char* (*getPartHash) (COKitDocument* pThis,
                          int nPart);

    /// Paints a tile from a specific part.
    /// @see kit::Document::paintTile().
    void (*paintPartTile) (COKitDocument* pThis,
                           unsigned char* pBuffer,
                           const int nPart,
                           const int nMode,
                           const int nCanvasWidth,
                           const int nCanvasHeight,
                           const int nTilePosX,
                           const int nTilePosY,
                           const int nTileWidth,
                           const int nTileHeight);

    /// @see kit::Document::getViewIds().
    bool (*getViewIds) (COKitDocument* pThis,
                       int* pArray,
                       size_t nSize);

    /// @see kit::Document::setOutlineState).
    void (*setOutlineState) (COKitDocument* pThis, bool bColumn, int nLevel, int nIndex, bool bHidden);

    /// Paints window with given id to the buffer
    /// @see kit::Document::paintWindow().
    void (*paintWindow) (COKitDocument* pThis, unsigned nWindowId,
                         unsigned char* pBuffer,
                         const int x, const int y,
                         const int width, const int height);

    /// @see kit::Document::postWindow().
    void (*postWindow) (COKitDocument* pThis, unsigned nWindowId, int nAction, const char* pData);

    /// @see kit::Document::postWindowKeyEvent().
    void (*postWindowKeyEvent) (COKitDocument* pThis,
                                unsigned nWindowId,
                                int nType,
                                int nCharCode,
                                int nKeyCode);

    /// @see kit::Document::postWindowMouseEvent().
    void (*postWindowMouseEvent) (COKitDocument* pThis,
                                  unsigned nWindowId,
                                  int nType,
                                  int nX,
                                  int nY,
                                  int nCount,
                                  int nButtons,
                                  int nModifier);

    /// @see kit::Document::setViewLanguage().
    void (*setViewLanguage) (COKitDocument* pThis, int nId, const char* language);

    /// @see kit::Document::postWindowExtTextInputEvent
    void (*postWindowExtTextInputEvent) (COKitDocument* pThis,
                                         unsigned nWindowId,
                                         int nType,
                                         const char* pText);

    /// @see kit::Document::getPartInfo().
    char* (*getPartInfo) (COKitDocument* pThis, int nPart);

    /// Paints window with given id to the buffer with the give DPI scale
    /// (every pixel is dpiscale-times larger).
    /// @see kit::Document::paintWindow().
    void (*paintWindowDPI) (COKitDocument* pThis, unsigned nWindowId,
                            unsigned char* pBuffer,
                            const int x, const int y,
                            const int width, const int height,
                            const double dpiscale);

// CERTIFICATE AND SIGNING

    /// @see kit::Document::insertCertificate().
    bool (*insertCertificate) (COKitDocument* pThis,
                                const unsigned char* pCertificateBinary,
                                const int nCertificateBinarySize,
                                const unsigned char* pPrivateKeyBinary,
                                const int nPrivateKeyBinarySize);

    /// @see kit::Document::addCertificate().
    bool (*addCertificate) (COKitDocument* pThis,
                                const unsigned char* pCertificateBinary,
                                const int nCertificateBinarySize);

    /// @see kit::Document::getSignatureState().
    int (*getSignatureState) (COKitDocument* pThis);
// END CERTIFICATE AND SIGNING

    /// @see kit::Document::renderShapeSelection
    size_t (*renderShapeSelection)(COKitDocument* pThis, char** pOutput);

    /// @see kit::Document::postWindowGestureEvent().
    void (*postWindowGestureEvent) (COKitDocument* pThis,
                                  unsigned nWindowId,
                                  const char* pType,
                                  int nX,
                                  int nY,
                                  int nOffset);

    /// @see kit::Document::createViewWithOptions().
    int (*createViewWithOptions) (COKitDocument* pThis, const char* pOptions);

    /// @see kit::Document::selectPart().
    void (*selectPart) (COKitDocument* pThis, int nPart, int nSelect);

    /// @see kit::Document::moveSelectedParts().
    /// nIntoSection: when >= 0, the section at that index will be re-anchored
    /// to the first moved slide (i.e. the slide becomes the new section start).
    /// Pass -1 to keep the default behaviour where sections stay anchored to
    /// their existing non-moved slides.
    void (*moveSelectedParts) (COKitDocument* pThis, int nPosition, bool bDuplicate, int nIntoSection);

    /// Resize window with given id.
    /// @see kit::Document::resizeWindow().
    void (*resizeWindow) (COKitDocument* pThis, unsigned nWindowId,
                          const int width, const int height);

    /// Pass a nullptr terminated array of mime-type strings
    /// @see kit::Document::getClipboard for more details
    int (*getClipboard) (COKitDocument* pThis,
                         const char **pMimeTypes,
                         size_t      *pOutCount,
                         char      ***pOutMimeTypes,
                         size_t     **pOutSizes,
                         char      ***pOutStreams);

    /// @see kit::Document::setClipboard
    int (*setClipboard) (COKitDocument* pThis,
                         const size_t   nInCount,
                         const char   **pInMimeTypes,
                         const size_t  *pInSizes,
                         const char   **pInStreams);

    /// @see kit::Document::getSelectionType
    int (*getSelectionType) (COKitDocument* pThis);

    /// @see kit::Document::removeTextContext
    void (*removeTextContext) (COKitDocument* pThis,
                               unsigned nWindowId,
                               int nBefore,
                               int nAfter);

    /// @see kit::Document::sendDialogEvent
    void (*sendDialogEvent) (COKitDocument* pThis,
                            unsigned long long int nKitWindowId,
                            const char* pArguments);

    /// @see kit::Document::renderFontOrientation().
    unsigned char* (*renderFontOrientation) (COKitDocument* pThis,
                       const char* pFontName,
                       const char* pChar,
                       int* pFontWidth,
                       int* pFontHeight,
                       int pOrientation);

    /// Switches view to viewId if viewId >= 0, and paints window
    /// @see kit::Document::paintWindowDPI().
    void (*paintWindowForView) (COKitDocument* pThis, unsigned nWindowId,
                                unsigned char* pBuffer,
                                const int x, const int y,
                                const int width, const int height,
                                const double dpiscale,
                                int viewId);

    /// @see kit::Document::completeFunction().
    void (*completeFunction) (COKitDocument* pThis, const char* pFunctionName);

    /// @see kit::Document::setWindowTextSelection
    void (*setWindowTextSelection) (COKitDocument* pThis,
                                    unsigned nWindowId,
                                    bool bSwap,
                                    int nX,
                                    int nY);

    /// @see kit::Document::sendFormFieldEvent
    void (*sendFormFieldEvent) (COKitDocument* pThis,
                                const char* pArguments);

    /// @see kit::Document::setBlockedCommandList
    void (*setBlockedCommandList) (COKitDocument* pThis,
                                int nViewId,
                                const char* blockedCommandList);

    /// @see kit::Document::renderSearchResult
    bool (*renderSearchResult) (COKitDocument* pThis,
                                const char* pSearchResult,
                                unsigned char** pBitmapBuffer,
                                int* pWidth, int* pHeight, size_t* pByteSize);

    /// @see kit::Document::sendContentControlEvent().
    void (*sendContentControlEvent)(COKitDocument* pThis, const char* pArguments);

    /// @see kit::Document::getSelectionTypeAndText
    int (*getSelectionTypeAndText) (COKitDocument* pThis,
                                    const char* pMimeType,
                                    char** pText,
                                    char** pUsedMimeType);

    /// @see kit::Document::getDataArea().
    void (*getDataArea) (COKitDocument* pThis,
                         long nPart,
                         long* pCol,
                         long* pRow);

    /// @see kit::Document::getEditMode().
    int (*getEditMode) (COKitDocument* pThis);

    /// @see kit::Document::setViewTimezone().
    void (*setViewTimezone) (COKitDocument* pThis, int nId, const char* timezone);

    /// @see kit::Document::setAccessibilityState().
    void (*setAccessibilityState) (COKitDocument* pThis, int nId, bool nEnabled);

    /// @see kit::Document::getA11yFocusedParagraph.
    char* (*getA11yFocusedParagraph) (COKitDocument* pThis);

    /// @see kit::Document::getA11yCaretPosition.
    int (*getA11yCaretPosition) (COKitDocument* pThis);

    /// @see kit::Document::setViewReadOnly().
    void (*setViewReadOnly) (COKitDocument* pThis, int nId, const bool readOnly);

    /// @see kit::Document::setAllowChangeComments().
    void (*setAllowChangeComments) (COKitDocument* pThis, int nId, const bool allow);

    /// @see kit::Document::getPresentationInfo
    char* (*getPresentationInfo) (COKitDocument* pThis);

    /// @see kit::Document::createSlideRenderer
    bool (*createSlideRenderer) (
        COKitDocument* pThis,
        const char* pSlideHash,
        int nSlideNumber, unsigned* nViewWidth, unsigned* nViewHeight,
        bool bRenderBackground, bool bRenderMasterPage);

    /// @see kit::Document::postSlideshowCleanup
    void (*postSlideshowCleanup)(COKitDocument* pThis);

    /// @see kit::Document::renderNextSlideLayer
    bool (*renderNextSlideLayer)(
        COKitDocument* pThis, unsigned char* pBuffer, bool* bIsBitmapLayer, double* pScale, char** pJsonMessage);

    /// @see kit::Document::setViewOption
    void (*setViewOption)(COKitDocument* pThis, const char* pOption, const char* pValue);

    /// @see kit::Document::setColorPreviewState().
    void (*setColorPreviewState) (COKitDocument* pThis, int nId, bool nEnabled);

    /// @see kit::Document::setAllowManageRedlines().
    void (*setAllowManageRedlines)(COKitDocument* pThis, int nId, bool allow);

    /// @see kit::Document::transferClipboardFromView().
    void (*transferClipboardFromView)(COKitDocument* pThis, int nSourceViewId);

    /// @see kit::Document::flushClipboard().
    void (*flushClipboard)(COKitDocument* pThis);

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
    COKitDocument *get() { return mpDoc; }

    /**
     * Get document type.
     *
     * @return an element of the COKitDocumentType enum.
     */
    int getDocumentType()
    {
        return mpDoc->pClass->getDocumentType(mpDoc);
    }

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
     * KIT_CALLBACK_TEXT_SELECTION.
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
     * @return an element of the COKitTileMode enum.
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
     * Registers a callback. COKit will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    void registerCallback(COKitCallback pCallback, void* pData)
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
     * @param nType @see COKitSetTextSelectionType
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
     * @return an element of the COKitSelectionType enum.
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
     * KIT_SELTYPE_TEXT then it also returns the same as getTextSelection(), otherwise
     * pText and pUsedMimeType are unchanged.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     * @param pText the currently selected text
     * @param pUsedMimeType output parameter to inform about the determined format (suggested one or plain text).
     * @return an element of the COKitSelectionType enum.
     */
    int getSelectionTypeAndText(const char* pMimeType, char** pText, char** pUsedMimeType = NULL)
    {
        return mpDoc->pClass->getSelectionTypeAndText(mpDoc, pMimeType, pText, pUsedMimeType);
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
     * Shares another view's live clipboard transferable into the current view's
     * clipboard by reference, without serializing (same-process only). The caller
     * must have made the destination the current view first.
     *
     * @param nSourceViewId the view whose clipboard contents to share.
     */
    void transferClipboardFromView(int nSourceViewId)
    {
        mpDoc->pClass->transferClipboardFromView(mpDoc, nSourceViewId);
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
        mpDoc->pClass->flushClipboard(mpDoc);
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
     * @param nType @see COKitSetGraphicSelectionType
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
     *
     * The return value is dynamically allocated and should be
     * deallocated by calling the kit::Office::freeMemory() function.
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
        return mpDoc->pClass->createViewWithOptions(mpDoc, pOptions);
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
        return mpDoc->pClass->renderFontOrientation(mpDoc, pFontName, pChar, pFontWidth, pFontHeight, pOrientation);
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
     * @param nType see COKitExtTextInputType
     * @param pText Text for KIT_EXT_TEXTINPUT
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
    /// nIntoSection: when >= 0, re-anchor that section to the first moved
    /// slide (slide becomes the section's new first slide).  Pass -1 (default)
    /// to keep the existing section anchoring.
    void moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection = -1)
    {
        mpDoc->pClass->moveSelectedParts(mpDoc, nPosition, bDuplicate, nIntoSection);
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

    /** Set if the view can manage redlines in readonly mode or not.
     *
     * @param nId view ID
     * @param allow
    */
    void setAllowManageRedlines(int nId, bool allow)
    {
        mpDoc->pClass->setAllowManageRedlines(mpDoc, nId, allow);
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

    /// Get the information about the current presentation (Impress only).
    char* getPresentationInfo()
    {
        return mpDoc->pClass->getPresentationInfo(mpDoc);
    }

    /// Create a slide renderer in core for the input slide.
    bool createSlideRenderer(
        const char* pSlideHash,
        int nSlideNumber, unsigned* nViewWidth, unsigned* nViewHeight,
        bool bRenderBackground, bool bRenderMasterPage)
    {
        return mpDoc->pClass->createSlideRenderer(
            mpDoc, pSlideHash, nSlideNumber, nViewWidth, nViewHeight, bRenderBackground, bRenderMasterPage);
    }

    /// Clean-up the slideshow (slide renderer)
    void postSlideshowCleanup()
    {
        mpDoc->pClass->postSlideshowCleanup(mpDoc);
    }

    /// Render the slide layer
    bool renderNextSlideLayer(unsigned char* pBuffer, bool* bIsBitmapLayer, double* pScale, char** pJsonMessage)
    {
        return mpDoc->pClass->renderNextSlideLayer(mpDoc, pBuffer, bIsBitmapLayer, pScale, pJsonMessage);
    }

    /// Set named view options
    void setViewOption(const char* pOption, const char* pValue)
    {
        mpDoc->pClass->setViewOption(mpDoc, pOption, pValue);
    }

    /**
     * Set color preview state for the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param nEnabled true/false
     */
    void setColorPreviewState(int nId, bool nEnabled)
    {
        mpDoc->pClass->setColorPreviewState(mpDoc, nId, nEnabled);
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
        mpThis->pClass->destroy(mpThis);
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
        COKitDocument* pDoc = mpThis->pClass->documentLoadWithOptions(mpThis, pUrl, pFilterOptions);

        if (pDoc == NULL)
            return NULL;

        return new Document(pDoc);
    }

    /// Returns the last error as a string. The returned pointer has to be freed by the caller
    /// by calling the freeError() member function.
    char* getError()
    {
        return mpThis->pClass->getError(mpThis);
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
        mpThis->pClass->freeError(pFree);
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
     */
    char* getFilterTypes()
    {
        return mpThis->pClass->getFilterTypes(mpThis);
    }

    /**
     * Set bitmask of optional features supported by the client.
     *
     * @see COKitOptionalFeatures
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
     * In response to KIT_CALLBACK_DOCUMENT_PASSWORD, a valid password
     * will continue loading the document, an invalid password will
     * result in another KIT_CALLBACK_DOCUMENT_PASSWORD request,
     * and a NULL password will abort loading the document.
     *
     * In response to KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY, a valid
     * password will continue loading the document, an invalid password will
     * result in another KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY request,
     * and a NULL password will continue loading the document in read-only
     * mode.
     */
    void setDocumentPassword(char const* pURL, char const* pPassword)
    {
        mpThis->pClass->setDocumentPassword(mpThis, pURL, pPassword);
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
        return mpThis->pClass->getVersionInfo(mpThis);
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
        mpThis->pClass->setOption(mpThis, pOption, pValue);
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
        mpThis->pClass->dumpState(mpThis, pOption, pState);
    }

    char* extractRequest(const char* pFilePath)
    {
        return mpThis->pClass->extractRequest(mpThis, pFilePath);
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
        mpThis->pClass->trimMemory(mpThis, nTarget);
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
        mpThis->pClass->executeScript(
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
        mpThis->pClass->deliverProxyResult(callId, jsonValue);
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
        mpThis->pClass->cancelProxyCalls();
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
        return mpThis->pClass->isExpectedReentry();
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
        return mpThis->pClass->takeLegacyUnoApiUseFlag();
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

    char* extractDocumentStructureRequest(const char* pFilePath, const char* pFilter)
    {
        return mpThis->pClass->extractDocumentStructureRequest(mpThis, pFilePath, pFilter);
    }

    /**
     * Registers a callback that can determine if there are any pending input events.
     */
    void registerAnyInputCallback(COKitAnyInputCallback pCallback, void* pData)
    {
        return mpThis->pClass->registerAnyInputCallback(mpThis, pCallback, pData);
    }

    /**
     * Get number of documents of this COKit.
     */
    int getDocsCount()
    {
        return mpThis->pClass->getDocsCount(mpThis);
    }

    /**
     * Registers a callback that can display an interactive file save dialog.
     */
    void registerFileSaveDialogCallback(COKitFileSaveDialogCallback pCallback)
    {
        return mpThis->pClass->registerFileSaveDialogCallback(mpThis, pCallback);
    }

    /**
     * Registers a callback that reveals (and selects) a file in the native file manager.
     */
    void registerRevealInFileManagerCallback(COKitRevealInFileManagerCallback pCallback)
    {
        return mpThis->pClass->registerRevealInFileManagerCallback(mpThis, pCallback);
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
        mpThis->pClass->installClipboardProvider(mpThis, pProvider);
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
        return mpThis->pClass->getGlobalClipboard(mpThis, pMimeTypes, pOutCount, pOutMimeTypes, pOutSizes, pOutStreams);
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
