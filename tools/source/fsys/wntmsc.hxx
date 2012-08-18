/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _dosmsc_hxx
#define _dosmsc_hxx

#include <string.h>

#ifndef ICC
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <windows.h>
#include <tools/solar.h>
#include <rtl/strbuf.hxx>

#define DOS_DIRECT      _A_SUBDIR
#define DOS_VOLUMEID    0x08

#ifndef S_IFBLK
#define S_IFBLK         0x6000
#endif

#define dirent          _WIN32_FIND_DATAA
#define d_name          cFileName
#define d_type          dwFileAttributes

typedef struct
{
    _WIN32_FIND_DATAA aDirEnt;
    HANDLE           h;
    const char      *p;
} DIR;

#define PATHDELIMITER   ";"
#define DEFSTYLE        FSYS_STYLE_NTFS
#define MKDIR( p )      mkdir( p )
#define CMP_LOWER(s)    ( s.toAsciiLowerCase() )

inline sal_Bool DRIVE_EXISTS(char c)
{
    rtl::OStringBuffer aDriveRoot;
    aDriveRoot.append(c);
    aDriveRoot.append(":\\");
    return GetDriveType( aDriveRoot.getStr() ) > 1;
}

const char* TempDirImpl( char *pBuf );

#define FSysFailOnErrorImpl()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
