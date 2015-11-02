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

#ifndef INCLUDED_SAL_QA_OSL_FILE_OSL_FILE_CONST_H
#define INCLUDED_SAL_QA_OSL_FILE_OSL_FILE_CONST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sal/types.h>

#include <rtl/ustring.hxx>

#ifdef __cplusplus
extern "C"
{
#endif

// common used string resource
// these common used string will be used as assist resource in test
// they are mostly OS independent, some of the resource can be reused
// so, a common test data repository will be better since it can be
// shared among all test code

const sal_Char pBuffer_Char[]   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const sal_Char pBuffer_Number[] = "1234567890";
const sal_Char pBuffer_Blank[]  = "";

// OS dependent/independent definitions/includes
// we use FILE_PREFIX for URL prefix,
//        TEST_PLATFORM for test platform initial,
//        TEST_PLATFORM_ROOT for root dir in comrresponding platform,
//        TEST_PLATFORM_TEMP for temp dir in comrresponding platform,
//        PATH_LIST_DELIMITER for separator of path list in comrresponding platform,
//        PATH_SEPARATOR for separator in URL or system path in comrresponding platform,
//        PATH_MAX/MAX_PATH for max path length in comrresponding platform,

// OS independent const definition

#   define FILE_PREFIX          "file:///"
#   define TEST_FILE_SIZE       1024

// OS dependent declaration and includes

#if ( defined UNX )  //Unix
#   include <unistd.h>
#   include <limits.h>
#   include <math.h>
#   include <errno.h>
#   include <fcntl.h>
#   include <sys/stat.h>
#   if !defined(MACOSX) && !defined(IOS) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined (DRAGONFLY)
#       include <sys/statfs.h>
#   else
#       include <sys/param.h>
#       include <sys/mount.h>
#   endif
#   if !defined(ANDROID)
#        include <sys/statvfs.h>
#   endif
#   include <sys/types.h>
#   define TEST_PLATFORM        ""
#   define TEST_PLATFORM_ROOT   "/"
#   define TEST_PLATFORM_TEMP   "tmp"
#   define PATH_LIST_DELIMITER  ":"
#   define PATH_SEPARATOR       "/"
#endif

#if (defined WNT )                      // Windows
#       include <tchar.h>
#       include <io.h>
#       include <stdio.h>
#       include <stdlib.h>
#   define PATH_MAX             MAX_PATH
#   define TEST_PLATFORM        "c:/"
#   define TEST_PLATFORM_ROOT   "c:/"
#   define TEST_PLATFORM_TEMP   "temp"
#   define PATH_LIST_DELIMITER  ";"
#   define PATH_SEPARATOR       "/"
#endif

// OS independent file definition

OUString aNullURL( "" );
OUString aSlashURL( PATH_SEPARATOR );
OUString aPreURL( FILE_PREFIX );
OUString aRootURL( FILE_PREFIX TEST_PLATFORM );

OUString aTempDirectoryURL( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP );
OUString aTempDirectorySys( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP );
OUString aUserDirectoryURL( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "" );
OUString aUserDirectorySys( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "" );

// common used URL:temp, canonical, root, relative, link,etc

OUString aCanURL1( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/canonical.name" );
rtl::OUString aCanURL2(
    RTL_CONSTASCII_USTRINGPARAM("ca@#;+.,$///78no\0ni..name"));
OUString aCanURL3( "ca@#;+.,$//tmp/678nonical//name" );
OUString aCanURL4( "canonical.name" );
OUString aTmpName1( "tmpdir" );
OUString aTmpName2( "tmpname" );
OUString aTmpName3( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir" );
OUString aTmpName4( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir/tmpname" );
OUString aTmpName5( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir/../tmpdir/./tmpname" );
OUString aTmpName6( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpname" );
OUString aTmpName7( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/noaccess" );
OUString aTmpName8( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpname/tmpdir" );
OUString aTmpName9( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir/../tmpdir/./" );
OUString aTmpName10(RTL_CONSTASCII_USTRINGPARAM( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/\xE6\x9C\xAA\xE5\x91\xBD\xE5\x90\x8Dzhgb18030" ));

OUString aRelURL1( "relative/file1" );
OUString aRelURL2( "relative/./file2" );
OUString aRelURL3( "relative/../file3" );
OUString aRelURL4( "././relative/../file4" );
OUString aRelURL5( TEST_PLATFORM_TEMP "/./../" TEST_PLATFORM_TEMP );
OUString aLnkURL1( FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/link.file" );
OUString aHidURL1( ".hiddenfile" );

// common used System Path:temp, root,etc

OUString aRootSys( TEST_PLATFORM_ROOT );
OUString aSysPath1( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/system.path" );
OUString aSysPath2( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/system/path" );
OUString aSysPath3( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/tmpdir" );
OUString aSysPath4( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/tmpname" );
OUString aSysPath5(RTL_CONSTASCII_USTRINGPARAM( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/\xE6\x9C\xAA\xE5\x91\xBD\xE5\x90\x8Dzhgb18030" ));
OUString aSysPathLnk( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/link.file" );
OUString aFifoSys( TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/tmpdir/fifo" );

// FileType URL, we pick some canonical file in corresponding system for test:
// socket, link, etc.
// Note that this may be changed in the different platform, so be careful to use.

#if ( defined UNX )                                                    //          Unix
OUString aTypeURL1( FILE_PREFIX "dev/ccv");                    //socket    Solaris/Linux
OUString aTypeURL2( FILE_PREFIX "devices/pseudo/tcp@0:tcp");   //special   Solaris/Linux
OUString aTypeURL3( FILE_PREFIX "lib" );                       //link      Solaris
#else                                                                  //          Windows
OUString aTypeURL1( FILE_PREFIX "" );
OUString aTypeURL2( FILE_PREFIX "" );
OUString aTypeURL3( FILE_PREFIX "" );
#endif

// Volume device URL, we pick some canonical volume device for test:
// UNIX file system, Floppy Disk, Proc file system, Temp file system, Compact Disk.

#if ( defined UNX )                                     //          Unix
OUString aVolURL1( FILE_PREFIX  "");            //ufs       Solaris/Linux
#ifdef SOLARIS
OUString aVolURL2( FILE_PREFIX  "dev/fd" );     //fd        Solaris
#else
OUString aVolURL2( FILE_PREFIX  "dev/floppy/0u1440" );  //fd0       Linux
#endif
OUString aVolURL3( FILE_PREFIX  "proc" );       //proc      Solaris/Linux
OUString aVolURL4( FILE_PREFIX  "staroffice" ); //nfs       Solaris/Linux
OUString aVolURL5( FILE_PREFIX  "tmp" );        //tmpfs     Solaris
OUString aVolURL6( FILE_PREFIX  "cdrom" );      //cd        Solaris
#else                                                   //          Windows
OUString aVolURL1( FILE_PREFIX  "c:/" );
OUString aVolURL2( FILE_PREFIX  "a:/" );
OUString aVolURL3( FILE_PREFIX  "" );
OUString aVolURL4( FILE_PREFIX  "" );
OUString aVolURL5( FILE_PREFIX  "c:/temp" );
OUString aVolURL6( FILE_PREFIX  "e:/" );
#endif

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_SAL_QA_OSL_FILE_OSL_FILE_CONST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
