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

#ifndef _os2_hxx
#define _os2_hxx


#define INCL_DOSEXCEPTIONS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#define INCL_DOSMISC
#define INCL_DOSNLS   /* National Language Support values */
#include <svpm.h>

#include <dirent.h>
#include <string.h>

#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <ctype.h>
#include <emx/syscalls.h>

#define FSYS_UNIX FALSE

#define DOS_DIRECT      _A_SUBDIR
#define DOS_VOLUMEID    _A_VOLID

#define _mkdir(p)       mkdir(p, 0777)

const char* TempDirImpl( char *pBuf );
String ToLowerImpl( const String& );

#define DEFSTYLE            FSYS_STYLE_OS2
#define MKDIR( p )          mkdir( (unsigned char*) p )
#define CMP_LOWER(s)    ( s.ToLowerAscii() )

#define START_DRV       'a'

inline BOOL DRIVE_EXISTS( char c )
{
    ULONG  nCur, nMap;
    APIRET nRet = DosQueryCurrentDisk( &nCur, &nMap );
    return ( nMap & 1 << (c - 'a') ) != 0;
}

#include <time.h>

inline Time MsDos2Time( FTIME* aTime )
{
    return Time( aTime->hours, aTime->minutes, 2*aTime->twosecs );
}

inline Date MsDos2Date( FDATE* aDate )
{
    return Date( aDate->day, aDate->month, aDate->year );
}

Time MsDos2Time( const time_t *pTimeT );

Date MsDos2Date( const time_t *pTimeT );

#define FSysFailOnErrorImpl()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
