/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIBLIBREOFFICE_H
#define INCLUDED_DESKTOP_INC_LIBLIBREOFFICE_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _OfficeKit OfficeKit;
typedef struct _OfficeKitDocument OfficeKitDocument;

struct _OfficeKit
{
  int  nSize;

  void                 (*destroy)       (OfficeKit *pThis);
  int                  (*initialize)    (OfficeKit *pThis, const char *pInstallPath);
  OfficeKitDocument*   (*documentLoad)  (OfficeKit *pThis, const char *pURL);
  char*                (*getError)      (OfficeKit *pThis);
};

struct _OfficeKitDocument
{
  int  nSize;

  void (*destroy)   (OfficeKitDocument* pThis);
  int (*saveAs)     (OfficeKitDocument* pThis,
                     const char *pUrl,
                     const char *pFormat);
  int (*saveAsWithOptions) (OfficeKitDocument* pThis,
                            const char *pUrl,
                            const char *pFormat,
                            const char *pFilterOptions);
};

OfficeKit* officekit_init (const char* pInstallPath);

#ifdef __cplusplus
}
#endif

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
