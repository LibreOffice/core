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

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _LibreOfficeKit LibreOfficeKit;
typedef struct _LibreOfficeKitDocument LibreOfficeKitDocument;

struct _LibreOfficeKit
{
  int  nSize;

  void                    (*destroy)       (LibreOfficeKit *pThis);
  int                     (*initialize)    (LibreOfficeKit *pThis, const char *pInstallPath);
  LibreOfficeKitDocument* (*documentLoad)  (LibreOfficeKit *pThis, const char *pURL);
  char*                   (*getError)      (LibreOfficeKit *pThis);
};

struct _LibreOfficeKitDocument
{
  int  nSize;

  void (*destroy)   (LibreOfficeKitDocument* pThis);
  int (*saveAs)     (LibreOfficeKitDocument* pThis,
                     const char *pUrl,
                     const char *pFormat);
  int (*saveAsWithOptions) (LibreOfficeKitDocument* pThis,
                            const char *pUrl,
                            const char *pFormat,
                            const char *pFilterOptions);
};

LibreOfficeKit* lok_init (const char* pInstallPath);

#ifdef __cplusplus
}
#endif

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
