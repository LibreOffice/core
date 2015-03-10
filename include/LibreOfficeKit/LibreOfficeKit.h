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

#ifdef LOK_USE_UNSTABLE_API
typedef void (*LibreOfficeKitCallback)(int nType, const char* pPayload, void* pData);
#endif // LOK_USE_UNSTABLE_API

struct _LibreOfficeKit
{
    LibreOfficeKitClass* pClass;
};

struct _LibreOfficeKitClass
{
  size_t  nSize;

  void                    (*destroy)       (LibreOfficeKit* pThis);
  LibreOfficeKitDocument* (*documentLoad)  (LibreOfficeKit* pThis, const char* pURL);
  char*                   (*getError)      (LibreOfficeKit* pThis);
  LibreOfficeKitDocument* (*documentLoadWithOptions)  (LibreOfficeKit* pThis,
                                                      const char* pURL,
                                                      const char* pOptions);
};

#define LIBREOFFICEKIT_DOCUMENT_HAS(pDoc,member) LIBREOFFICEKIT_HAS_MEMBER(LibreOfficeKitDocumentClass,member,(pDoc)->pClass->nSize)

struct _LibreOfficeKitDocument
{
    LibreOfficeKitDocumentClass* pClass;
};

struct _LibreOfficeKitDocumentClass
{
  size_t  nSize;

  void (*destroy)   (LibreOfficeKitDocument* pThis);
  int (*saveAs)     (LibreOfficeKitDocument* pThis,
                     const char* pUrl,
                     const char* pFormat,
                     const char* pFilterOptions);
#ifdef LOK_USE_UNSTABLE_API
  /** Get document type.
   *
   * @returns an element of the LibreOfficeKitDocumentType enum.
   */
  int (*getDocumentType) (LibreOfficeKitDocument* pThis);

  /** Get number of part that the document contains.
   * Part refers to either indivual sheets in a Spreadsheet,
   * or slides in a Slideshow, and has no relevance for
   * writer documents.
   */
  int (*getParts) (LibreOfficeKitDocument* pThis);

  /** Get current part of the document */
  int (*getPart)          (LibreOfficeKitDocument* pThis);
  void (*setPart)         (LibreOfficeKitDocument* pThis,
                           int nPart);

  char* (*getPartName)    (LibreOfficeKitDocument* pThis,
                           int nPart);

  /** Sets mode of the current part.
   *
   * @param nMode - element from the LibreOfficeKitPartMode enum.
   */
  void (*setPartMode)     (LibreOfficeKitDocument* pThis,
                           int nMode);

  void (*paintTile)       (LibreOfficeKitDocument* pThis,
                           unsigned char* pBuffer,
                           const int nCanvasWidth,
                           const int nCanvasHeight,
                           int* pRowStride,
                           const int nTilePosX,
                           const int nTilePosY,
                           const int nTileWidth,
                           const int nTileHeight);

  /** Get the document sizes in twips. */
  void (*getDocumentSize) (LibreOfficeKitDocument* pThis,
                           long* pWidth,
                           long* pHeight);

  /** Initialize document for rendering.
   * Sets the rendering and document parameters to default values
   * that are needed to render the document correctly using
   * tiled rendering.
   */
  void (*initializeForRendering) (LibreOfficeKitDocument* pThis);

  void (*registerCallback)   (LibreOfficeKitDocument* pThis,
                              LibreOfficeKitCallback pCallback,
                              void* pData);

  /// @see lok::Document::postKeyEvent
  void (*postKeyEvent)(LibreOfficeKitDocument* pThis,
                       int nType,
                       int nCharCode,
                       int nKeyCode);
  /// @see lok::Document::postMouseEvent
  void (*postMouseEvent)(LibreOfficeKitDocument* pThis,
                         int nType,
                         int nX,
                         int nY,
                         int nCount);
  /// @see lok::Document::setTextSelection
  void (*setTextSelection)(LibreOfficeKitDocument* pThis,
                         int nType,
                         int nX,
                         int nY);
  /// @see lok::Document::setGraphicSelection
  void (*setGraphicSelection)(LibreOfficeKitDocument* pThis,
                         int nType,
                         int nX,
                         int nY);
  /// @see lok::Document::resetSelection
  void (*resetSelection)(LibreOfficeKitDocument* pThis);
#endif // LOK_USE_UNSTABLE_API
};

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
