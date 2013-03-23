/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#pragma once
#if 1



#ifdef UNIX


#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#endif //end of UNIX


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#define SO_SERVER_PORT 8100


#ifdef UNIX
#define STAROFFICE_VERSION_FILE "/.sversionrc"
#define STAROFFICE_EXE_FILE_NAME "soffice"
#endif
#ifdef WNT
#define STAROFFICE_VERSION_FILE "\\sversion.ini"
#define STAROFFICE_EXE_FILE_NAME "soffice.exe"
#endif

#define SECTION_NAME     "Versions"
#define SOFFICE_VERSION  "StarOffice 8"

#define APP_ERROR 1
#define APP_CHAR  2
#define APP_WCHAR 3

// return the install path of staroffice, return value like "/home/build/staroffice"
const char* findInstallDir();

// return SO program dir absolute path, like "/home/build/staroffice/program"
const char* findProgramDir();

#ifdef WNT
// return SO executable absolute path, like "/home/build/staroffice/program/soffice"
const char* findSofficeExecutable();

// change Dos path such as c:\program\soffice to c:/program/soffice
int DosToUnixPath(char*);
#endif

//change Unix path such as program\soffice to program/soffice
int UnixToDosPath(char*);

void NSP_WriteLog(int level,  const char* pFormat, ...);

int restoreUTF8(char* pPath);

char* NSP_getPluginName();

char* NSP_getPluginDesc();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
