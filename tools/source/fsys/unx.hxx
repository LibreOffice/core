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
#ifndef _unx_hxx
#define _unx_hxx

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <unistd.h>

#define DRIVE_EXISTS(c) ( TRUE )

#define _mkdir(p)       mkdir(p, 0777)
#define _rmdir          rmdir
#define _chdir          chdir
#define _unlink         unlink
#define _getcwd         getcwd
#define _access         access

#define DEFSTYLE        FSYS_STYLE_BSD

#define CMP_LOWER(s)    (s)
#define LOWER(aString)  (aString.Lower())

#include <time.h>

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
