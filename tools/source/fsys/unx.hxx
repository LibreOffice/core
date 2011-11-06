/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
