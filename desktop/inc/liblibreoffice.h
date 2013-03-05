/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * A simple C API to setup and use libreoffice
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int   errno;
  char *message;
} LOError;

typedef int loboolean;
typedef struct _LODocument LODocument;

loboolean lo_initialize (const char *install_path);

void     lo_error_free (LOError *error);
LOError *lo_error_new (int errno, const char *message);

LODocument *lo_document_load (const char *url, LOError **opt_error);
loboolean   lo_document_save (const char *url, LOError **opt_error);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

