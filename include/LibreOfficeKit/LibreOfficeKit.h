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
  WRITER,
  SPREADSHEET,
  PRESENTATION,
  OTHER
}
LibreOfficeKitDocumentType;
#endif // LOK_USE_UNSTABLE_API

struct _LibreOfficeKit
{
    LibreOfficeKitClass* pClass;
};

struct _LibreOfficeKitClass
{
  size_t  nSize;

  void                    (*destroy)       (LibreOfficeKit *pThis);
  LibreOfficeKitDocument* (*documentLoad)  (LibreOfficeKit *pThis, const char *pURL);
  char*                   (*getError)      (LibreOfficeKit *pThis);
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
                     const char *pUrl,
                     const char *pFormat,
                     const char *pFilterOptions);
  int (*saveAsWithOptions) (LibreOfficeKitDocument* pThis,
                            const char *pUrl,
                            const char *pFormat,
                            const char *pFilterOptions);
#ifdef LOK_USE_UNSTABLE_API
  LibreOfficeKitDocumentType (*getDocumentType) (LibreOfficeKitDocument* pThis);

  // Part refers to either indivual sheets in a Spreadsheet, or slides
  // in a Slideshow, and has no relevance for wrtier documents.
  int (*getNumberOfParts) (LibreOfficeKitDocument* pThis);

  void (*setPart)         (LibreOfficeKitDocument* pThis,
                           int nPart);

  // pCanvas is a pointer to the appropriate type of graphics object:
  // Windows: HDC
  // iOS/OSX: CGContextRef
  // Unx: A full SystemGraphicsData
  // (This is as we need multiple pieces of data on Unx -- in the future
  // it would potentially be best to define our own simple equivalent
  // structure here which can then be copied into a SystemGraphicsData
  // within the paintTile implementation.)
  void (*paintTile)       (LibreOfficeKitDocument* pThis,
                           void* Canvas,
                           const int nCanvasWidth,
                           const int nCanvasHeight,
                           const int nTilePosX,
                           const int nTilePosY,
                           const int nTileWidth,
                           const int nTileHeight);
#endif // LOK_USE_UNSTABLE_API
};

LibreOfficeKit* lok_init (const char* pInstallPath);

#ifdef __cplusplus
}
#endif

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
