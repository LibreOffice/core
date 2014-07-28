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
#ifdef LOK_USE_UNSTABLE_API
  LibreOfficeKitDocumentType (*getDocumentType) (LibreOfficeKitDocument* pThis);

  // Part refers to either indivual sheets in a Spreadsheet, or slides
  // in a Slideshow, and has no relevance for wrtier documents.
  int (*getParts) (LibreOfficeKitDocument* pThis);

  int (*getPart)          (LibreOfficeKitDocument* pThis);
  void (*setPart)         (LibreOfficeKitDocument* pThis,
                           int nPart);

  // Get a pointer to a raw array, of size 3*nCanvasWidth*nCanvasHeight
  // Basebmp's bitmap device seems to round the width up if needed
  // for its internal buffer, i.e. the rowstride for the buffer may be larger
  // than the desired width, hence we need to be able to return the
  // rowstride too.
  void (*paintTile)       (LibreOfficeKitDocument* pThis,
                           unsigned char* pBuffer,
                           const int nCanvasWidth,
                           const int nCanvasHeight,
                           int* pRowStride,
                           const int nTilePosX,
                           const int nTilePosY,
                           const int nTileWidth,
                           const int nTileHeight);

  // Get the document sizes in twips.
  void (*getDocumentSize) (LibreOfficeKitDocument* pThis,
                           long* pWidth,
                           long* pHeight);
#endif // LOK_USE_UNSTABLE_API
};

#ifdef LINUX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>
#ifdef AIX
#  include <sys/ldr.h>
#endif

#define TARGET_LIB        "lib" "sofficeapp" ".so"
#define TARGET_MERGED_LIB "lib" "mergedlo" ".so"

typedef LibreOfficeKit *(HookFunction)( const char *install_path);


#if defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-function"
#endif

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-function"
#endif

static LibreOfficeKit *lok_init( const char *install_path )
{
    char *imp_lib;
    size_t partial_length;
    void *dlhandle;
    HookFunction *pSym;

    if (!install_path)
        return NULL;

    // allocate large enough buffer
    partial_length = strlen(install_path);
    imp_lib = (char *) malloc(partial_length + sizeof(TARGET_LIB) + sizeof(TARGET_MERGED_LIB) + 2);
    if (!imp_lib)
    {
        fprintf( stderr, "failed to open library : not enough memory\n");
        return NULL;
    }

    strcpy(imp_lib, install_path);

    imp_lib[partial_length++] = '/';
    strcpy(imp_lib + partial_length, TARGET_LIB);

    dlhandle = dlopen(imp_lib, RTLD_LAZY);
    if (!dlhandle)
    {
        strcpy(imp_lib + partial_length, TARGET_MERGED_LIB);

        dlhandle = dlopen(imp_lib, RTLD_LAZY);
        if (!dlhandle)
        {
            fprintf(stderr, "failed to open library '%s' or '%s' in '%s/'\n",
                    TARGET_LIB, TARGET_MERGED_LIB, install_path);
            free(imp_lib);
            return NULL;
        }
    }

    pSym = (HookFunction *) dlsym( dlhandle, "libreofficekit_hook" );
    if (!pSym)
    {
        fprintf( stderr, "failed to find hook in library '%s'\n", imp_lib );
        dlclose( dlhandle );
        free( imp_lib );
        return NULL;
    }

    free( imp_lib );
    return pSym( install_path );
}

#if defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#   pragma clang diagnostic pop
#endif

#endif // not LINUX => port me !

#ifdef __cplusplus
}
#endif

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
