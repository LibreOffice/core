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

typedef struct _LibreOffice LibreOffice;
typedef struct _LibreOfficeDocument LibreOfficeDocument;

struct _LibreOffice
{
  int  nSize;

  void                 (*destroy)       (LibreOffice *pThis);
  int                  (*initialize)    (LibreOffice *pThis, const char *pInstallPath);
  LibreOfficeDocument* (*documentLoad)  (LibreOffice *pThis, const char *pURL);
  char*                (*getError)      (LibreOffice *pThis);
};

struct _LibreOfficeDocument
{
  int  nSize;

  void (*destroy)   (LibreOfficeDocument* pThis);
  int (*saveAs)     (LibreOfficeDocument* pThis,
                     const char *pUrl,
                     const char *pFormat);
};

LibreOffice* lo_init (const char* pInstallPath);

#ifdef __cplusplus
  }
#endif

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
