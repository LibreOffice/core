/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_H
#define INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_H

#include <stddef.h>

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
// the unstable API needs C99's bool
#include <stdbool.h>
#include <stdint.h>
#endif

#include <LibreOfficeKit/LibreOfficeKitTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _LibreOfficeKit LibreOfficeKit;
typedef struct _LibreOfficeKitClass LibreOfficeKitClass;

typedef struct _LibreOfficeKitDocument LibreOfficeKitDocument;
typedef struct _LibreOfficeKitDocumentClass LibreOfficeKitDocumentClass;

// Do we have an extended member in this struct ?
#define LIBREOFFICEKIT_HAS_MEMBER(strct,member,nSize) \
    (offsetof(strct, member) < (nSize))

#define LIBREOFFICEKIT_HAS(pKit,member) LIBREOFFICEKIT_HAS_MEMBER(LibreOfficeKitClass,member,(pKit)->pClass->nSize)

struct _LibreOfficeKit
{
    LibreOfficeKitClass* pClass;
};

struct _LibreOfficeKitClass
{
    size_t  nSize;

    void (*destroy) (LibreOfficeKit* pThis);

    LibreOfficeKitDocument* (*documentLoad) (LibreOfficeKit* pThis,
                                             const char* pURL);

    char* (*getError) (LibreOfficeKit* pThis);

    /// @since LibreOffice 5.0
    LibreOfficeKitDocument* (*documentLoadWithOptions) (LibreOfficeKit* pThis,
                                                        const char* pURL,
                                                        const char* pOptions);
    /// @since LibreOffice 5.2
    void (*freeError) (char* pFree);

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
    void (*registerCallback) (LibreOfficeKit* pThis,
                              LibreOfficeKitCallback pCallback,
                              void* pData);

    /// @see lok::Office::getFilterTypes().
    char* (*getFilterTypes) (LibreOfficeKit* pThis);

    /// @see lok::Office::setOptionalFeatures().
    void (*setOptionalFeatures)(LibreOfficeKit* pThis, uint64_t features);

    /// @see lok::Office::setDocumentPassword().
    void (*setDocumentPassword) (LibreOfficeKit* pThis,
            char const* pURL,
            char const* pPassword);

    /// @see lok::Office::getVersionInfo().
    char* (*getVersionInfo) (LibreOfficeKit* pThis);
#endif

};

#define LIBREOFFICEKIT_DOCUMENT_HAS(pDoc,member) LIBREOFFICEKIT_HAS_MEMBER(LibreOfficeKitDocumentClass,member,(pDoc)->pClass->nSize)

struct _LibreOfficeKitDocument
{
    LibreOfficeKitDocumentClass* pClass;
};

struct _LibreOfficeKitDocumentClass
{
    size_t  nSize;

    void (*destroy) (LibreOfficeKitDocument* pThis);

    int (*saveAs) (LibreOfficeKitDocument* pThis,
                   const char* pUrl,
                   const char* pFormat,
                   const char* pFilterOptions);

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
    /// @see lok::Document::getDocumentType().
    int (*getDocumentType) (LibreOfficeKitDocument* pThis);

    /// @see lok::Document::getParts().
    int (*getParts) (LibreOfficeKitDocument* pThis);

    /// @see lok::Document::getPartPageRectangles().
    char* (*getPartPageRectangles) (LibreOfficeKitDocument* pThis);

    /// @see lok::Document::getPart().
    int (*getPart) (LibreOfficeKitDocument* pThis);

    /// @see lok::Document::setPart().
    void (*setPart) (LibreOfficeKitDocument* pThis,
                     int nPart);

    /// @see lok::Document::getPartName().
    char* (*getPartName) (LibreOfficeKitDocument* pThis,
                          int nPart);

    /// @see lok::Document::setPartMode().
    void (*setPartMode) (LibreOfficeKitDocument* pThis,
                         int nMode);

    /// @see lok::Document::paintTile().
    void (*paintTile) (LibreOfficeKitDocument* pThis,
                       unsigned char* pBuffer,
                       const int nCanvasWidth,
                       const int nCanvasHeight,
                       const int nTilePosX,
                       const int nTilePosY,
                       const int nTileWidth,
                       const int nTileHeight);

    /// @see lok::Document::getTileMode().
    int (*getTileMode) (LibreOfficeKitDocument* pThis);

    /// @see lok::Document::getDocumentSize().
    void (*getDocumentSize) (LibreOfficeKitDocument* pThis,
                             long* pWidth,
                             long* pHeight);

    /// @see lok::Document::initializeForRendering().
    void (*initializeForRendering) (LibreOfficeKitDocument* pThis,
                                    const char* pArguments);

    /// @see lok::Document::registerCallback().
    void (*registerCallback) (LibreOfficeKitDocument* pThis,
                              LibreOfficeKitCallback pCallback,
                              void* pData);

    /// @see lok::Document::postKeyEvent
    void (*postKeyEvent) (LibreOfficeKitDocument* pThis,
                          int nType,
                          int nCharCode,
                          int nKeyCode);

    /// @see lok::Document::postMouseEvent
    void (*postMouseEvent) (LibreOfficeKitDocument* pThis,
                            int nType,
                            int nX,
                            int nY,
                            int nCount,
                            int nButtons,
                            int nModifier);

    /// @see lok::Document::postUnoCommand
    void (*postUnoCommand) (LibreOfficeKitDocument* pThis,
                            const char* pCommand,
                            const char* pArguments,
                            bool bNotifyWhenFinished);

    /// @see lok::Document::setTextSelection
    void (*setTextSelection) (LibreOfficeKitDocument* pThis,
                              int nType,
                              int nX,
                              int nY);

    /// @see lok::Document::getTextSelection
    char* (*getTextSelection) (LibreOfficeKitDocument* pThis,
                               const char* pMimeType,
                               char** pUsedMimeType);

    /// @see lok::Document::paste().
    bool (*paste) (LibreOfficeKitDocument* pThis,
                   const char* pMimeType,
                   const char* pData,
                   size_t nSize);

    /// @see lok::Document::setGraphicSelection
    void (*setGraphicSelection) (LibreOfficeKitDocument* pThis,
                                 int nType,
                                 int nX,
                                 int nY);

    /// @see lok::Document::resetSelection
    void (*resetSelection) (LibreOfficeKitDocument* pThis);

    /// @see lok::Document::getCommandValues().
    char* (*getCommandValues) (LibreOfficeKitDocument* pThis, const char* pCommand);

    /// @see lok::Document::setClientZoom().
    void (*setClientZoom) (LibreOfficeKitDocument* pThis,
            int nTilePixelWidth,
            int nTilePixelHeight,
            int nTileTwipWidth,
            int nTileTwipHeight);
    /// @see lok::Document::setVisibleArea).
    void (*setClientVisibleArea) (LibreOfficeKitDocument* pThis, int nX, int nY, int nWidth, int nHeight);

    /// @see lok::Document::createView().
    uintptr_t (*createView) (LibreOfficeKitDocument* pThis);
    /// @see lok::Document::destroyView().
    void (*destroyView) (LibreOfficeKitDocument* pThis, uintptr_t nId);
    /// @see lok::Document::setView().
    void (*setView) (LibreOfficeKitDocument* pThis, uintptr_t nId);
    /// @see lok::Document::getView().
    uintptr_t (*getView) (LibreOfficeKitDocument* pThis);
    /// @see lok::Document::getViews().
    int (*getViews) (LibreOfficeKitDocument* pThis);

    /// @see lok::Document::renderFont().
    unsigned char* (*renderFont) (LibreOfficeKitDocument* pThis,
                       const char* pFontName,
                       int* pFontWidth,
                       int* pFontHeight);

    /// @see lok::Document::getPartHash().
    char* (*getPartHash) (LibreOfficeKitDocument* pThis,
                          int nPart);

    /// Paints a tile from a specific part.
    /// @see lok::Document::paintTile().
    void (*paintPartTile) (LibreOfficeKitDocument* pThis,
                           unsigned char* pBuffer,
                           const int nPart,
                           const int nCanvasWidth,
                           const int nCanvasHeight,
                           const int nTilePosX,
                           const int nTilePosY,
                           const int nTileWidth,
                           const int nTileHeight);

#endif // defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
};

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
