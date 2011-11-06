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
//#include <datetime.hxx>

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

