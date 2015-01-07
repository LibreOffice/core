/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_H
#define INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_H

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
    ((((size_t)((unsigned char *)&((strct *) 0)->member) +  \
      sizeof ((strct *) 0)->member)) <= (nSize))

#define LIBREOFFICEKIT_HAS(pKit,member) LIBREOFFICEKIT_HAS_MEMBER(LibreOfficeKitClass,member,(pKit)->pClass->nSize)

#ifdef LOK_USE_UNSTABLE_API
typedef enum
{
  LOK_DOCTYPE_TEXT,
  LOK_DOCTYPE_SPREADSHEET,
  LOK_DOCTYPE_PRESENTATION,
  LOK_DOCTYPE_DRAWING,
  LOK_DOCTYPE_OTHER
}
LibreOfficeKitDocumentType;

typedef enum
{
    LOK_PARTMODE_DEFAULT,
    LOK_PARTMODE_SLIDE,
    LOK_PARTMODE_NOTES,
    LOK_PARTMODE_SLIDENOTES,
    LOK_PARTMODE_EMBEDDEDOBJ
}
LibreOfficeKitPartMode;

typedef enum
{
    /**
     * Any tiles which are over the rectangle described in the payload are no
     * longer valid.
     *
     * Rectangle format: "width,height,x,y", where all numbers are document
     * coordinates, in twips.
     */
    LOK_CALLBACK_INVALIDATE_TILES
}
LibreOfficeKitCallbackType;

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
  LibreOfficeKitDocumentType (*getDocumentType) (LibreOfficeKitDocument* pThis);

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

  void (*setPartMode)     (LibreOfficeKitDocument* pThis,
                           LibreOfficeKitPartMode eMode);

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
#endif // LOK_USE_UNSTABLE_API
};

#ifdef __cplusplus
}
#endif

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
