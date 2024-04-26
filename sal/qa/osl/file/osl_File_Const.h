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

#include <cppunit/extensions/HelperMacros.h>

static OUString getTempDirectoryURL_()
{
    OUString aDir;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("couldn't get system temp URL",
        osl::FileBase::E_None, osl::FileBase::getTempDirURL(aDir));
    // This resolves symlinks in the temp path if any
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
                         osl::FileBase::getAbsoluteFileURL(aDir, aDir, aDir));
    return aDir;
}

static OUString getTempDirectorySys_()
{
    OUString aDir;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("couldn't get system temp directory",
        osl::FileBase::E_None, osl::FileBase::getSystemPathFromFileURL(getTempDirectoryURL_(), aDir));
    return aDir;
}

#ifdef __cplusplus
extern "C"
{
#endif

// common used string resource
// these common used string will be used as assist resource in test
// they are mostly OS independent, some of the resource can be reused
// so, a common test data repository will be better since it can be
// shared among all test code

const char pBuffer_Char[]   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char pBuffer_Number[] = "1234567890";
const char pBuffer_Blank[]  = "";

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
#   if !defined(MACOSX) && !defined(IOS) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined (DRAGONFLY) && !defined(HAIKU)
#       include <sys/statfs.h>
#   else
#       include <sys/param.h>
#       ifndef HAIKU
#           include <sys/mount.h>
#       endif
#   endif
#   if !defined(ANDROID)
#        include <sys/statvfs.h>
#   endif
#   include <sys/types.h>
#   define TEST_PLATFORM_ROOT   "/"
#   define PATH_LIST_DELIMITER  ":"
#   define PATH_SEPARATOR       "/"
#endif

#if defined(_WIN32)                      // Windows
#       include <io.h>
#   define PATH_MAX             MAX_PATH
#   define TEST_PLATFORM_ROOT   "c:/"
#   define PATH_LIST_DELIMITER  ";"
#   define PATH_SEPARATOR       "/"
#endif

// OS independent file definition

OUString aNullURL( u""_ustr );
OUString aSlashURL( u"" PATH_SEPARATOR ""_ustr );
OUString aPreURL( u"" FILE_PREFIX ""_ustr );
OUString aRootURL( u"" FILE_PREFIX TEST_PLATFORM_ROOT ""_ustr );

OUString aTempDirectorySys(getTempDirectorySys_());
OUString aTempDirectoryURL(getTempDirectoryURL_());
OUString aUserDirectorySys( aTempDirectorySys + "" );
OUString aUserDirectoryURL( aTempDirectoryURL + "" );

// common used URL:temp, canonical, root, relative, link,etc

OUString aCanURL1( aTempDirectoryURL +  "/canonical.name" );
OUString aCanURL2(
    RTL_CONSTASCII_USTRINGPARAM("ca@#;+.,$///78no\0ni..name"));
OUString aCanURL3( u"ca@#;+.,$//tmp/678nonical//name"_ustr );
OUString aCanURL4( u"canonical.name"_ustr );
OUString aTmpName1( u"tmpdir"_ustr );
OUString aTmpName2( u"tmpname"_ustr );
OUString aTmpName3( aTempDirectoryURL + "/tmpdir" );
OUString aTmpName4( aTempDirectoryURL + "/tmpdir/tmpname" );
OUString aTmpName5( aTempDirectoryURL + "/tmpdir/../tmpdir/./tmpname" );
OUString aTmpName6( aTempDirectoryURL + "/tmpname" );
OUString aTmpName7( aTempDirectoryURL + "/noaccess" );
OUString aTmpName8( aTempDirectoryURL + "/tmpname/tmpdir" );
OUString aTmpName9( aTempDirectoryURL + "/tmpdir/../tmpdir/./" );
OUString aTmpName10(aTempDirectoryURL + u"/\xE6\x9C\xAA\xE5\x91\xBD\xE5\x90\x8Dzhgb18030");

OUString aRelURL1( u"relative/file1"_ustr );
OUString aRelURL2( u"relative/./file2"_ustr );
OUString aRelURL3( u"relative/../file3"_ustr );
OUString aRelURL4( u"././relative/../file4"_ustr );
OUString aRelURL5( aTempDirectoryURL + "/./../"     );
OUString aLnkURL1( aTempDirectoryURL + "/link.file" );
OUString aHidURL1( u".hiddenfile"_ustr );

// common used System Path:temp, root,etc

OUString aRootSys( u"" TEST_PLATFORM_ROOT ""_ustr);
OUString aSysPath1( aTempDirectorySys + "/system.path" );
OUString aSysPath2( aTempDirectorySys + "/system/path" );
OUString aSysPath3( aTempDirectorySys + "/tmpdir" );
OUString aSysPath4( aTempDirectorySys + "/tmpname" );
OUString aSysPath5( aTempDirectorySys + u"/\xE6\x9C\xAA\xE5\x91\xBD\xE5\x90\x8Dzhgb18030" );
OUString aSysPathLnk( aTempDirectorySys + "/link.file" );
OUString aFifoSys( aTempDirectorySys + "/tmpdir/fifo" );

// FileType URL, we pick some canonical file in corresponding system for test:
// socket, link, etc.
// Note that this may be changed in the different platform, so be careful to use.

#if ( defined UNX )                                                    //          Unix
OUString aTypeURL1( u"" FILE_PREFIX "dev/ccv"_ustr);                    //socket    Solaris/Linux
OUString aTypeURL2( u"" FILE_PREFIX "devices/pseudo/tcp@0:tcp"_ustr);   //special   Solaris/Linux
OUString aTypeURL3( u"" FILE_PREFIX "lib"_ustr );                       //link      Solaris
#else                                                                  //          Windows
OUString aTypeURL1( u"" FILE_PREFIX ""_ustr );
OUString aTypeURL2( u"" FILE_PREFIX ""_ustr );
OUString aTypeURL3( u"" FILE_PREFIX ""_ustr );
#endif

// Volume device URL, we pick some canonical volume device for test:
// UNIX file system, Floppy Disk, Proc file system, Temp file system, Compact Disk.

#if ( defined UNX )                                     //          Unix
OUString aVolURL1( u"" FILE_PREFIX  ""_ustr);            //ufs       Solaris/Linux
#ifdef __sun
OUString aVolURL2( u"" FILE_PREFIX  "dev/fd"_ustr );     //fd        Solaris
#else
OUString aVolURL2( u"" FILE_PREFIX  "dev/floppy/0u1440"_ustr );  //fd0       Linux
#endif
OUString aVolURL3( u"" FILE_PREFIX  "proc"_ustr );       //proc      Solaris/Linux
OUString aVolURL4( u"" FILE_PREFIX  "staroffice"_ustr ); //nfs       Solaris/Linux
OUString aVolURL5( u"" FILE_PREFIX  "tmp"_ustr );        //tmpfs     Solaris
OUString aVolURL6( u"" FILE_PREFIX  "cdrom"_ustr );      //cd        Solaris
#else                                                   //          Windows
OUString aVolURL1( u"" FILE_PREFIX  "c:/"_ustr );
OUString aVolURL2( u"" FILE_PREFIX  "a:/"_ustr );
OUString aVolURL3( u"" FILE_PREFIX  ""_ustr );
OUString aVolURL4( u"" FILE_PREFIX  ""_ustr );
OUString aVolURL5( u"" FILE_PREFIX  "c:/temp"_ustr );
OUString aVolURL6( u"" FILE_PREFIX  "e:/"_ustr );
#endif

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_SAL_QA_OSL_FILE_OSL_FILE_CONST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
