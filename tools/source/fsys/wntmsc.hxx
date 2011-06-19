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

#ifndef _dosmsc_hxx
#define _dosmsc_hxx

#include <string.h>

#ifndef ICC
#include <io.h>
#endif
#include <sys\types.h>
#include <sys\stat.h>
#include <direct.h>

#include <windows.h>
#include <tools/solar.h>

#include <tools/string.hxx>

//--------------------------------------------------------------------

#define FSYS_UNIX sal_False

#define DOS_DIRECT      _A_SUBDIR
#define DOS_VOLUMEID    0x08
#ifndef S_IFBLK
#define S_IFBLK         0x6000
#endif
#define setdrive(n,a)   _chdrive(n)
#define GETDRIVE(n)     (n = _getdrive())

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
#define CMP_LOWER(s)    ( ByteString(s).ToLowerAscii() )

#define START_DRV 'a'

inline sal_Bool DRIVE_EXISTS(char c)
{
    ByteString aDriveRoot( c );
    aDriveRoot += ":\\";
    return GetDriveType( aDriveRoot.GetBuffer() ) > 1;
}

const char* TempDirImpl( char *pBuf );

#define FSysFailOnErrorImpl()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
