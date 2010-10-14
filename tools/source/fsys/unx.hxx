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

#ifndef _unx_hxx
#define _unx_hxx

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <unistd.h>
/* #include <sysent.h> */

#define FSYS_UNIX TRUE
#define DRIVE_EXISTS(c) ( TRUE )

#define _mkdir(p)       mkdir(p, 0777)
#define _rmdir          rmdir
#define _chdir          chdir
#define _unlink         unlink
#define _getcwd         getcwd
#define _access         access

#ifdef SYSV3
#define DEFSTYLE        FSYS_STYLE_SYSV
#else
#define DEFSTYLE        FSYS_STYLE_BSD
#endif

#define CMP_LOWER(s)    (s)
#define TEMPNAME()      tmpnam(0)
#define LOWER(aString)  (aString.Lower())

#include <time.h>
#include <tools/datetime.hxx>

inline Time Unx2Time( time_t nTime )
{
    tm atm;
    tm *pTime;
    pTime = localtime_r( &nTime, &atm );
    return Time( pTime->tm_hour,
                 pTime->tm_min,
                 pTime->tm_sec );
}

inline Date Unx2Date( time_t nDate )
{
    tm atm;
    tm *pTime;
    pTime = localtime_r( &nDate, &atm );
    return Date( pTime->tm_mday,
                 pTime->tm_mon + 1,
                 pTime->tm_year + 1900 );
}

inline void Unx2DateAndTime( time_t nDate, Time& rTime, Date& rDate )
{
    tm atm;
    tm *pTime;
    pTime = localtime_r( &nDate, &atm );
    rTime = Time( pTime->tm_hour, pTime->tm_min, pTime->tm_sec );
    rDate = Date( pTime->tm_mday, pTime->tm_mon + 1, pTime->tm_year + 1900 );
}

const char* TempDirImpl( char *pBuf );

#define FSysFailOnErrorImpl()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
