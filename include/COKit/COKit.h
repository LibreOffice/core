/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COKIT_COKIT_H
#define INCLUDED_COKIT_COKIT_H

#include <stddef.h>

// the unstable API needs C99's bool
// TODO remove the C99 types from the API before making stable
#if defined KIT_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
# ifndef _WIN32
#  include <stdbool.h>
# endif
# include <stdint.h>
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "COKitTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct COKitStruct COKit;
typedef struct COKitClassStruct COKitClass;

typedef struct COKitDocumentStruct COKitDocument;
typedef struct COKitDocumentClassStruct COKitDocumentClass;

// Do we have an extended member in this struct ?
#define COKIT_HAS_MEMBER(strct,member,nSize) \
    (offsetof(strct, member) < (nSize))

#define COKIT_HAS(pKit,member) COKIT_HAS_MEMBER(COKitClass,member,(pKit)->pClass->nSize)

struct COKitStruct
{
    COKitClass* pClass;
};

struct COKitClassStruct
{
    size_t  nSize;

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
                            unsigned long long int nLOKWindowId,
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
};

#define COKIT_DOCUMENT_HAS(pDoc,member) COKIT_HAS_MEMBER(COKitDocumentClass,member,(pDoc)->pClass->nSize)

struct COKitDocumentStruct
{
    COKitDocumentClass* pClass;
};

struct COKitDocumentClassStruct
{
    size_t  nSize;

    void (*destroy) (COKitDocument* pThis);

    int (*saveAs) (COKitDocument* pThis,
                   const char* pUrl,
                   const char* pFormat,
                   const char* pFilterOptions);

    /** @see kit::Document::getDocumentType(). */
    int (*getDocumentType) (COKitDocument* pThis);

#if defined KIT_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
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

    /// @see kit::Document::renderFont().
    unsigned char* (*renderFont) (COKitDocument* pThis,
                       const char* pFontName,
                       const char* pChar,
                       int* pFontWidth,
                       int* pFontHeight);

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
    void (*moveSelectedParts) (COKitDocument* pThis, int nPosition, bool bDuplicate);

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
                            unsigned long long int nLOKWindowId,
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

#endif // defined KIT_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
};

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_COKIT_COKIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
