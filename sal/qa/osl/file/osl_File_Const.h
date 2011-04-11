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

#ifndef _OSL_FILE_CONST_H_
#define _OSL_FILE_CONST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sal/types.h>
#include <rtl/textenc.h>

#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>

#ifdef __cplusplus
extern "C"
{
#endif



//------------------------------------------------------------------------
// common used string resource
// these common used string will be used as assist resource in test
// they are mostly OS independent, some of the resource can be reused
// so, acommon test data repository will be better since it can be
// shared among all test code
//------------------------------------------------------------------------

const sal_Char pBuffer_Char[]   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const sal_Char pBuffer_Number[] = "1234567890";
const sal_Char pBuffer_Blank[]  = "";

#define TRUNC_LEN               10;
#define ENLARG_LEN              100;



//------------------------------------------------------------------------
// OS dependent/independent definitions/includes
// we use FILE_PREFIX for URL prefix,
//        TEST_PLATFORM for test platform initial,
//        TEST_PLATFORM_ROOT for root dir in comrresponding platform,
//        TEST_PLATFORM_TEMP for temp dir in comrresponding platform,
//        PATH_LIST_DELIMITER for seperator of path list in comrresponding platform,
//        PATH_SEPERATOR for seperator in URL or system path in comrresponding platform,
//        PATH_MAX/MAX_PATH for max path length in comrresponding platform,
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// OS independent const definition
//------------------------------------------------------------------------
#   define FILE_PREFIX          "file:///"
#   define TEST_FILE_SIZE       1024

//------------------------------------------------------------------------
// OS dependent declaration and includes
//------------------------------------------------------------------------
#if ( defined UNX ) || ( defined OS2 )  //Unix
#   include <unistd.h>
#   include <limits.h>
#   include <math.h>
#   include <errno.h>
#   include <fcntl.h>
#   include <sys/stat.h>
#   if !defined(MACOSX) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined (DRAGONFLY)
#       include <sys/statfs.h>
#   else
#       include <sys/param.h>
#       include <sys/mount.h>
#   endif
#   include <sys/statvfs.h>
#   include <sys/types.h>
#   define TEST_PLATFORM        ""
#   define TEST_PLATFORM_ROOT   "/"
#   define TEST_PLATFORM_TEMP   "tmp"
#   define PATH_LIST_DELIMITER  ":"
#   define PATH_SEPERATOR       "/"
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
#   define PATH_SEPERATOR       "/"
#endif


//------------------------------------------------------------------------
// macro definition for the ASCII array/OUString declarations,
// we use p### for the ASCII array,
//        a### for the OUString,
//        n###Len for its length
//------------------------------------------------------------------------

#define OSLTEST_DECLARE( str_name, str_value ) \
    ::rtl::OUString a##str_name = rtl::OUString::createFromAscii( ( str_value ) )

#define OSLTEST_DECLARE_UTF8(str_name, str_value ) \
    ::rtl::OUString a##str_name = ::rtl::Uri::decode( ::rtl::OUString::createFromAscii( ( str_value ) ), rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8)

//------------------------------------------------------------------------
// OS independent file definition
//------------------------------------------------------------------------
OSLTEST_DECLARE( NullURL,  "" );
OSLTEST_DECLARE( SlashURL, PATH_SEPERATOR );
OSLTEST_DECLARE( PreURL, FILE_PREFIX );
OSLTEST_DECLARE( RootURL,  FILE_PREFIX TEST_PLATFORM );

OSLTEST_DECLARE( TempDirectoryURL,  FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP );
OSLTEST_DECLARE( TempDirectorySys,  TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP );
OSLTEST_DECLARE( UserDirectoryURL,  FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "" );
OSLTEST_DECLARE( UserDirectorySys,  TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "" );

//------------------------------------------------------------------------
// common used URL:temp, canonical, root, relative, link,etc
//------------------------------------------------------------------------
OSLTEST_DECLARE( CanURL1,  FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/canonical.name" );
OSLTEST_DECLARE( CanURL2,  "ca@#;+.,$///78no\0ni..name" );
OSLTEST_DECLARE( CanURL3,  "ca@#;+.,$//tmp/678nonical//name" );
OSLTEST_DECLARE( CanURL4,  "canonical.name" );
OSLTEST_DECLARE( TmpName1, "tmpdir" );
OSLTEST_DECLARE( TmpName2, "tmpname" );
OSLTEST_DECLARE( TmpName3, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir" );
OSLTEST_DECLARE( TmpName4, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir/tmpname" );
OSLTEST_DECLARE( TmpName5, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir/../tmpdir/./tmpname" );
OSLTEST_DECLARE( TmpName6, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpname" );
OSLTEST_DECLARE( TmpName7, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/noaccess" );
OSLTEST_DECLARE( TmpName8, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpname/tmpdir" );
OSLTEST_DECLARE( TmpName9, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir/../tmpdir/./" );
OSLTEST_DECLARE_UTF8( TmpName10, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/%E6%9C%AA%E5%91%BD%E5%90%8Dzhgb18030" );

OSLTEST_DECLARE( RelURL1,  "relative/file1" );
OSLTEST_DECLARE( RelURL2,  "relative/./file2" );
OSLTEST_DECLARE( RelURL3,  "relative/../file3" );
OSLTEST_DECLARE( RelURL4,  "././relative/../file4" );
OSLTEST_DECLARE( RelURL5,  TEST_PLATFORM_TEMP "/./../" TEST_PLATFORM_TEMP );
OSLTEST_DECLARE( LnkURL1,  FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/link.file" );
OSLTEST_DECLARE( HidURL1,  ".hiddenfile" );

//------------------------------------------------------------------------
// common used System Path:temp, root,etc
//------------------------------------------------------------------------
OSLTEST_DECLARE( RootSys,  TEST_PLATFORM_ROOT );
OSLTEST_DECLARE( SysPath1, TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/system.path" );
OSLTEST_DECLARE( SysPath2, TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/system/path" );
OSLTEST_DECLARE( SysPath3, TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/tmpdir" );
OSLTEST_DECLARE( SysPath4, TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/tmpname" );
OSLTEST_DECLARE_UTF8( SysPath5, TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/%E6%9C%AA%E5%91%BD%E5%90%8Dzhgb18030" );
OSLTEST_DECLARE( SysPathLnk, TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/link.file" );
OSLTEST_DECLARE( FifoSys,  TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP "/tmpdir/fifo" );

//------------------------------------------------------------------------
// FileType URL, we pick some canonical file in corresponding system for test:
// socket, link, etc.
// Note that this may be changed in the different platform, so be careful to use.
//------------------------------------------------------------------------
#if ( defined UNX ) || ( defined OS2 )                                 //          Unix
OSLTEST_DECLARE( TypeURL1,  FILE_PREFIX "dev/ccv");                    //socket    Solaris/Linux
OSLTEST_DECLARE( TypeURL2,  FILE_PREFIX "devices/pseudo/tcp@0:tcp");   //special   Solaris/Linux
OSLTEST_DECLARE( TypeURL3,  FILE_PREFIX "lib" );                       //link      Solaris
#else                                                                  //          Windows
OSLTEST_DECLARE( TypeURL1,  FILE_PREFIX "" );
OSLTEST_DECLARE( TypeURL2,  FILE_PREFIX "" );
OSLTEST_DECLARE( TypeURL3,  FILE_PREFIX "" );
#endif

//------------------------------------------------------------------------
// Volume device URL, we pick some canonical volume device for test:
// UNIX file system, Floppy Disk, Proc file system, Temp file system, Compact Disk.
//------------------------------------------------------------------------
#if ( defined UNX ) || ( defined OS2 )                  //          Unix
OSLTEST_DECLARE( VolURL1,  FILE_PREFIX  "");            //ufs       Solaris/Linux
#ifdef SOLARIS
OSLTEST_DECLARE( VolURL2,  FILE_PREFIX  "dev/fd" );     //fd        Solaris
#else
OSLTEST_DECLARE( VolURL2,  FILE_PREFIX  "dev/floppy/0u1440" );  //fd0       Linux
#endif
OSLTEST_DECLARE( VolURL3,  FILE_PREFIX  "proc" );       //proc      Solaris/Linux
OSLTEST_DECLARE( VolURL4,  FILE_PREFIX  "staroffice" ); //nfs       Solaris/Linux
OSLTEST_DECLARE( VolURL5,  FILE_PREFIX  "tmp" );        //tmpfs     Solaris
OSLTEST_DECLARE( VolURL6,  FILE_PREFIX  "cdrom" );      //cd        Solaris
#else                                                   //          Windows
OSLTEST_DECLARE( VolURL1,  FILE_PREFIX  "c:/" );
OSLTEST_DECLARE( VolURL2,  FILE_PREFIX  "a:/" );
OSLTEST_DECLARE( VolURL3,  FILE_PREFIX  "" );
OSLTEST_DECLARE( VolURL4,  FILE_PREFIX  "" );
OSLTEST_DECLARE( VolURL5,  FILE_PREFIX  "c:/temp" );
OSLTEST_DECLARE( VolURL6,  FILE_PREFIX  "e:/" );
#endif

#ifdef __cplusplus
}
#endif

#endif /* _OSL_FILE_CONST_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
