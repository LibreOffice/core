/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SETUP_NATIVE_SOURCE_WIN32_WINTOOLS_MAKECAB_PARSEDDF_H
#define INCLUDED_SETUP_NATIVE_SOURCE_WIN32_WINTOOLS_MAKECAB_PARSEDDF_H

#ifdef __cplusplus
extern "C" {
#endif
#include <windows.h>
#include <fci.h>

typedef enum DDFLINETYPE { DDF_NONE, DDF_CMD, DDF_SRC } DDFLINETYPE;
typedef enum DDFERR { DDF_OK, DDFERR_INVALID_ENDL, DDFERR_NO_MORE_TOKENS, DDFERR_FUNC_FAILED, DDFERR_SRC_SKIPPED, DDFERR_UNREAD } DDFERR;
typedef enum DDFVAR { DDF_CABNAME, DDF_RESERVEPERCAB, DDF_MAXDISK, DDF_COMPTYPE, DDF_COMPRESS, DDF_COMPMEM, DDF_CABINET, DDF_DISKDIRTEMPLATE } DDFVAR;
typedef enum DDFLOGLEVEL { DDFLOG_ERR = 1, DDFLOG_WRN, DDFLOG_MSG } DDFLOGLEVEL;

#define DDF_MAX_CHARS 1024
#define DDF_MAX_LINES 1024
#define DDF_MAX_CABRESERVE 60000
#define DDF_MAX_CABNAME 50

typedef struct DDFSRCFILE
{
    char fileName[MAX_PATH];
    char cabName[DDF_MAX_CABNAME];
    BOOL extract;
    TCOMP cmpType;
    struct DDFSRCFILE * next;
} DDFSRCFILE;

unsigned int ParseDdf(char * ddfFile, PCCAB vars, DDFSRCFILE ** srcListH, unsigned int v);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_SETUP_NATIVE_SOURCE_WIN32_WINTOOLS_MAKECAB_PARSEDDF_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
