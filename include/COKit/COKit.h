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
#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
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

    /// @since LibreOffice 5.0
    COKitDocument* (*documentLoadWithOptions) (COKit* pThis,
                                                        const char* pURL,
                                                        const char* pOptions);
    /// @since LibreOffice 5.2

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

    /// @since LibreOffice 6.0
    void (*registerCallback) (COKit* pThis,
                              COKitCallback pCallback,
                              void* pData);

    /** @see lok::Office::getFilterTypes().
        @since LibreOffice 6.0
     */
    char* (*getFilterTypes) (COKit* pThis);

    /** @see lok::Office::setOptionalFeatures().
        @since LibreOffice 6.0
     */
    void (*setOptionalFeatures)(COKit* pThis, unsigned long long features);

    /** @see lok::Office::setDocumentPassword().
        @since LibreOffice 6.0
     */
    void (*setDocumentPassword) (COKit* pThis,
            char const* pURL,
            char const* pPassword);

    /** @see lok::Office::getVersionInfo().
        @since LibreOffice 6.0
     */
    char* (*getVersionInfo) (COKit* pThis);

    /** @see lok::Office::runMacro().
        @since LibreOffice 6.0
     */
    int (*runMacro) (COKit *pThis, const char* pURL);

    /** @see lok::Office::signDocument().
        @since LibreOffice 6.2
     */
     bool (*signDocument) (COKit* pThis,
                           const char* pUrl,
                           const unsigned char* pCertificateBinary,
                           const int nCertificateBinarySize,
                           const unsigned char* pPrivateKeyBinary,
                           const int nPrivateKeyBinarySize);

    /// @see lok::Office::runLoop()
    void (*runLoop) (COKit* pThis,
                     COKitPollCallback pPollCallback,
                     COKitWakeCallback pWakeCallback,
                     void* pData);

    /// @see lok::Office::sendDialogEvent
    void (*sendDialogEvent) (COKit* pThis,
                            unsigned long long int nLOKWindowId,
                            const char* pArguments);

    /// @see lok::Office::setOption
    void (*setOption) (COKit* pThis, const char* pOption, const char* pValue);

    /// @see lok::Office::dumpState
    /// @since LibreOffice 7.5
    void (*dumpState) (COKit* pThis, const char* pOptions, char** pState);

    /** @see lok::Office::extractRequest.
     */
    char* (*extractRequest) (COKit* pThis,
                           const char* pFilePath);

    /// @see lok::Office::trimMemory
    /// @since LibreOffice 7.6
    void (*trimMemory) (COKit* pThis, int nTarget);

    /// @see lok::Office::startURP
    void* (*startURP)(COKit* pThis,
                    void* pReceiveURPFromLOContext, void* pSendURPToLOContext,
                    int (*fnReceiveURPFromLO)(void* pContext, const signed char* pBuffer, int nLen),
                    int (*fnSendURPToLO)(void* pContext, signed char* pBuffer, int nLen));

    /// @see lok::Office::stopURP
    void (*stopURP)(COKit* pThis, void* pSendURPToLOContext);

    /// @see lok::Office::joinThreads
    int (*joinThreads)(COKit* pThis);

    /// @see lok::Office::startThreads
    void (*startThreads)(COKit* pThis);

    /// @see lok::Office::setForkedChild
    void (*setForkedChild)(COKit* pThis, bool bIsChild);

    /** @see lok::Office::extractDocumentStructureRequest.
     */
    char* (*extractDocumentStructureRequest)(COKit* pThis, const char* pFilePath,
                                             const char* pFilter);

    /// @see lok::Office::registerAnyInputCallback()
    void (*registerAnyInputCallback)(COKit* pThis,
                                     COKitAnyInputCallback pCallback, void* pData);

    /// @see lok::Office::getDocsCount().
    int (*getDocsCount) (COKit* pThis);

    /// @see lok::Office::registerFileSaveDialogCallback()
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

    /** @see lok::Document::getDocumentType().
        @since LibreOffice 6.0
     */
    int (*getDocumentType) (COKitDocument* pThis);

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
    /// @see lok::Document::getParts().
    int (*getParts) (COKitDocument* pThis);

    /// @see lok::Document::getPartPageRectangles().
    char* (*getPartPageRectangles) (COKitDocument* pThis);

    /// @see lok::Document::getPart().
    int (*getPart) (COKitDocument* pThis);

    /// @see lok::Document::setPart().
    void (*setPart) (COKitDocument* pThis,
                     int nPart);

    /// @see lok::Document::getPartName().
    char* (*getPartName) (COKitDocument* pThis,
                          int nPart);

    /// @see lok::Document::setPartMode().
    void (*setPartMode) (COKitDocument* pThis,
                         int nMode);

    /// @see lok::Document::paintTile().
    void (*paintTile) (COKitDocument* pThis,
                       unsigned char* pBuffer,
                       const int nCanvasWidth,
                       const int nCanvasHeight,
                       const int nTilePosX,
                       const int nTilePosY,
                       const int nTileWidth,
                       const int nTileHeight);

    /// @see lok::Document::getTileMode().
    int (*getTileMode) (COKitDocument* pThis);

    /// @see lok::Document::getDocumentSize().
    void (*getDocumentSize) (COKitDocument* pThis,
                             long* pWidth,
                             long* pHeight);

    /// @see lok::Document::initializeForRendering().
    void (*initializeForRendering) (COKitDocument* pThis,
                                    const char* pArguments);

    /// @see lok::Document::registerCallback().
    void (*registerCallback) (COKitDocument* pThis,
                              COKitCallback pCallback,
                              void* pData);

    /// @see lok::Document::postKeyEvent
    void (*postKeyEvent) (COKitDocument* pThis,
                          int nType,
                          int nCharCode,
                          int nKeyCode);

    /// @see lok::Document::postMouseEvent
    void (*postMouseEvent) (COKitDocument* pThis,
                            int nType,
                            int nX,
                            int nY,
                            int nCount,
                            int nButtons,
                            int nModifier);

    /// @see lok::Document::postUnoCommand
    void (*postUnoCommand) (COKitDocument* pThis,
                            const char* pCommand,
                            const char* pArguments,
                            bool bNotifyWhenFinished);

    /// @see lok::Document::setTextSelection
    void (*setTextSelection) (COKitDocument* pThis,
                              int nType,
                              int nX,
                              int nY);

    /// @see lok::Document::getTextSelection
    char* (*getTextSelection) (COKitDocument* pThis,
                               const char* pMimeType,
                               char** pUsedMimeType);

    /// @see lok::Document::paste().
    bool (*paste) (COKitDocument* pThis,
                   const char* pMimeType,
                   const char* pData,
                   size_t nSize);

    /// @see lok::Document::setGraphicSelection
    void (*setGraphicSelection) (COKitDocument* pThis,
                                 int nType,
                                 int nX,
                                 int nY);

    /// @see lok::Document::resetSelection
    void (*resetSelection) (COKitDocument* pThis);

    /// @see lok::Document::getCommandValues().
    char* (*getCommandValues) (COKitDocument* pThis, const char* pCommand);

    /// @see lok::Document::setClientZoom().
    void (*setClientZoom) (COKitDocument* pThis,
            int nTilePixelWidth,
            int nTilePixelHeight,
            int nTileTwipWidth,
            int nTileTwipHeight);

    /// @see lok::Document::setVisibleArea).
    void (*setClientVisibleArea) (COKitDocument* pThis, int nX, int nY, int nWidth, int nHeight);

    /// @see lok::Document::createView().
    int (*createView) (COKitDocument* pThis);
    /// @see lok::Document::destroyView().
    void (*destroyView) (COKitDocument* pThis, int nId);
    /// @see lok::Document::setView().
    void (*setView) (COKitDocument* pThis, int nId);
    /// @see lok::Document::getView().
    int (*getView) (COKitDocument* pThis);
    /// @see lok::Document::getViewsCount().
    int (*getViewsCount) (COKitDocument* pThis);

    /// @see lok::Document::renderFont().
    unsigned char* (*renderFont) (COKitDocument* pThis,
                       const char* pFontName,
                       const char* pChar,
                       int* pFontWidth,
                       int* pFontHeight);

    /// @see lok::Document::getPartHash().
    char* (*getPartHash) (COKitDocument* pThis,
                          int nPart);

    /// Paints a tile from a specific part.
    /// @see lok::Document::paintTile().
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

    /// @see lok::Document::getViewIds().
    bool (*getViewIds) (COKitDocument* pThis,
                       int* pArray,
                       size_t nSize);

    /// @see lok::Document::setOutlineState).
    void (*setOutlineState) (COKitDocument* pThis, bool bColumn, int nLevel, int nIndex, bool bHidden);

    /// Paints window with given id to the buffer
    /// @see lok::Document::paintWindow().
    void (*paintWindow) (COKitDocument* pThis, unsigned nWindowId,
                         unsigned char* pBuffer,
                         const int x, const int y,
                         const int width, const int height);

    /// @see lok::Document::postWindow().
    void (*postWindow) (COKitDocument* pThis, unsigned nWindowId, int nAction, const char* pData);

    /// @see lok::Document::postWindowKeyEvent().
    void (*postWindowKeyEvent) (COKitDocument* pThis,
                                unsigned nWindowId,
                                int nType,
                                int nCharCode,
                                int nKeyCode);

    /// @see lok::Document::postWindowMouseEvent().
    void (*postWindowMouseEvent) (COKitDocument* pThis,
                                  unsigned nWindowId,
                                  int nType,
                                  int nX,
                                  int nY,
                                  int nCount,
                                  int nButtons,
                                  int nModifier);

    /// @see lok::Document::setViewLanguage().
    void (*setViewLanguage) (COKitDocument* pThis, int nId, const char* language);

    /// @see lok::Document::postWindowExtTextInputEvent
    void (*postWindowExtTextInputEvent) (COKitDocument* pThis,
                                         unsigned nWindowId,
                                         int nType,
                                         const char* pText);

    /// @see lok::Document::getPartInfo().
    char* (*getPartInfo) (COKitDocument* pThis, int nPart);

    /// Paints window with given id to the buffer with the give DPI scale
    /// (every pixel is dpiscale-times larger).
    /// @see lok::Document::paintWindow().
    void (*paintWindowDPI) (COKitDocument* pThis, unsigned nWindowId,
                            unsigned char* pBuffer,
                            const int x, const int y,
                            const int width, const int height,
                            const double dpiscale);

// CERTIFICATE AND SIGNING

    /// @see lok::Document::insertCertificate().
    bool (*insertCertificate) (COKitDocument* pThis,
                                const unsigned char* pCertificateBinary,
                                const int nCertificateBinarySize,
                                const unsigned char* pPrivateKeyBinary,
                                const int nPrivateKeyBinarySize);

    /// @see lok::Document::addCertificate().
    bool (*addCertificate) (COKitDocument* pThis,
                                const unsigned char* pCertificateBinary,
                                const int nCertificateBinarySize);

    /// @see lok::Document::getSignatureState().
    int (*getSignatureState) (COKitDocument* pThis);
// END CERTIFICATE AND SIGNING

    /// @see lok::Document::renderShapeSelection
    size_t (*renderShapeSelection)(COKitDocument* pThis, char** pOutput);

    /// @see lok::Document::postWindowGestureEvent().
    void (*postWindowGestureEvent) (COKitDocument* pThis,
                                  unsigned nWindowId,
                                  const char* pType,
                                  int nX,
                                  int nY,
                                  int nOffset);

    /// @see lok::Document::createViewWithOptions().
    int (*createViewWithOptions) (COKitDocument* pThis, const char* pOptions);

    /// @see lok::Document::selectPart().
    void (*selectPart) (COKitDocument* pThis, int nPart, int nSelect);

    /// @see lok::Document::moveSelectedParts().
    void (*moveSelectedParts) (COKitDocument* pThis, int nPosition, bool bDuplicate);

    /// Resize window with given id.
    /// @see lok::Document::resizeWindow().
    void (*resizeWindow) (COKitDocument* pThis, unsigned nWindowId,
                          const int width, const int height);

    /// Pass a nullptr terminated array of mime-type strings
    /// @see lok::Document::getClipboard for more details
    int (*getClipboard) (COKitDocument* pThis,
                         const char **pMimeTypes,
                         size_t      *pOutCount,
                         char      ***pOutMimeTypes,
                         size_t     **pOutSizes,
                         char      ***pOutStreams);

    /// @see lok::Document::setClipboard
    int (*setClipboard) (COKitDocument* pThis,
                         const size_t   nInCount,
                         const char   **pInMimeTypes,
                         const size_t  *pInSizes,
                         const char   **pInStreams);

    /// @see lok::Document::getSelectionType
    int (*getSelectionType) (COKitDocument* pThis);

    /// @see lok::Document::removeTextContext
    void (*removeTextContext) (COKitDocument* pThis,
                               unsigned nWindowId,
                               int nBefore,
                               int nAfter);

    /// @see lok::Document::sendDialogEvent
    void (*sendDialogEvent) (COKitDocument* pThis,
                            unsigned long long int nLOKWindowId,
                            const char* pArguments);

    /// @see lok::Document::renderFontOrientation().
    unsigned char* (*renderFontOrientation) (COKitDocument* pThis,
                       const char* pFontName,
                       const char* pChar,
                       int* pFontWidth,
                       int* pFontHeight,
                       int pOrientation);

    /// Switches view to viewId if viewId >= 0, and paints window
    /// @see lok::Document::paintWindowDPI().
    void (*paintWindowForView) (COKitDocument* pThis, unsigned nWindowId,
                                unsigned char* pBuffer,
                                const int x, const int y,
                                const int width, const int height,
                                const double dpiscale,
                                int viewId);

    /// @see lok::Document::completeFunction().
    void (*completeFunction) (COKitDocument* pThis, const char* pFunctionName);

    /// @see lok::Document::setWindowTextSelection
    void (*setWindowTextSelection) (COKitDocument* pThis,
                                    unsigned nWindowId,
                                    bool bSwap,
                                    int nX,
                                    int nY);

    /// @see lok::Document::sendFormFieldEvent
    void (*sendFormFieldEvent) (COKitDocument* pThis,
                                const char* pArguments);

    /// @see lok::Document::setBlockedCommandList
    void (*setBlockedCommandList) (COKitDocument* pThis,
                                int nViewId,
                                const char* blockedCommandList);

    /// @see lok::Document::renderSearchResult
    bool (*renderSearchResult) (COKitDocument* pThis,
                                const char* pSearchResult,
                                unsigned char** pBitmapBuffer,
                                int* pWidth, int* pHeight, size_t* pByteSize);

    /// @see lok::Document::sendContentControlEvent().
    void (*sendContentControlEvent)(COKitDocument* pThis, const char* pArguments);

    /// @see lok::Document::getSelectionTypeAndText
    /// @since LibreOffice 7.4
    int (*getSelectionTypeAndText) (COKitDocument* pThis,
                                    const char* pMimeType,
                                    char** pText,
                                    char** pUsedMimeType);

    /// @see lok::Document::getDataArea().
    void (*getDataArea) (COKitDocument* pThis,
                         long nPart,
                         long* pCol,
                         long* pRow);

    /// @see lok::Document::getEditMode().
    int (*getEditMode) (COKitDocument* pThis);

    /// @see lok::Document::setViewTimezone().
    void (*setViewTimezone) (COKitDocument* pThis, int nId, const char* timezone);

    /// @see lok::Document::setAccessibilityState().
    void (*setAccessibilityState) (COKitDocument* pThis, int nId, bool nEnabled);

    /// @see lok::Document::getA11yFocusedParagraph.
    char* (*getA11yFocusedParagraph) (COKitDocument* pThis);

    /// @see lok::Document::getA11yCaretPosition.
    int (*getA11yCaretPosition) (COKitDocument* pThis);

    /// @see lok::Document::setViewReadOnly().
    void (*setViewReadOnly) (COKitDocument* pThis, int nId, const bool readOnly);

    /// @see lok::Document::setAllowChangeComments().
    void (*setAllowChangeComments) (COKitDocument* pThis, int nId, const bool allow);

    /// @see lok::Document::getPresentationInfo
    char* (*getPresentationInfo) (COKitDocument* pThis);

    /// @see lok::Document::createSlideRenderer
    bool (*createSlideRenderer) (
        COKitDocument* pThis,
        const char* pSlideHash,
        int nSlideNumber, unsigned* nViewWidth, unsigned* nViewHeight,
        bool bRenderBackground, bool bRenderMasterPage);

    /// @see lok::Document::postSlideshowCleanup
    void (*postSlideshowCleanup)(COKitDocument* pThis);

    /// @see lok::Document::renderNextSlideLayer
    bool (*renderNextSlideLayer)(
        COKitDocument* pThis, unsigned char* pBuffer, bool* bIsBitmapLayer, double* pScale, char** pJsonMessage);

    /// @see lok::Document::setViewOption
    void (*setViewOption)(COKitDocument* pThis, const char* pOption, const char* pValue);

    /// @see lok::Document::setColorPreviewState().
    void (*setColorPreviewState) (COKitDocument* pThis, int nId, bool nEnabled);

    /// @see lok::Document::setAllowManageRedlines().
    void (*setAllowManageRedlines)(COKitDocument* pThis, int nId, bool allow);

#endif // defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
};

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_COKIT_COKIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
