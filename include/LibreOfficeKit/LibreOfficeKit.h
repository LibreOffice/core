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

struct _LibreOfficeKit
{
    LibreOfficeKitClass* pClass;
};

void
lok_office_destroy              (LibreOfficeKit *pOffice);

int
lok_office_initialize           (LibreOfficeKit *pOffice,
                                 const char *pInstallPath);
LibreOfficeKitDocument*
lok_office_document_load        (LibreOfficeKit *pOffice,
                                 const char *pURL);
char*
lok_office_get_error            (LibreOfficeKit *pOffice);

struct _LibreOfficeKitClass
{
  size_t  nSize;

  void                    (*destroy)       (LibreOfficeKit *pThis);
  int                     (*initialize)    (LibreOfficeKit *pThis, const char *pInstallPath);
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
                     const char *pFormat);
  int (*saveAsWithOptions) (LibreOfficeKitDocument* pThis,
                            const char *pUrl,
                            const char *pFormat,
                            const char *pFilterOptions);
};

void
lok_document_destroy                (LibreOfficeKitDocument* pDocument);

int
lok_document_save_as                (LibreOfficeKitDocument* pDocument,
                                     const char *pUrl,
                                     const char *pFormat);

int
lok_document_save_as_with_options   (LibreOfficeKitDocument* pDocument,
                                     const char *pUrl,
                                     const char *pFormat,
                                     const char *pFilterOptions);

LibreOfficeKit* lok_init (const char* pInstallPath);

#ifdef __cplusplus
}
#endif

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
