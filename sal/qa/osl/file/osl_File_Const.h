/*************************************************************************
*
 *  $RCSfile: osl_File_Const.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:40:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _OSL_FILE_CONST_H_
#define _OSL_FILE_CONST_H_

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//------------------------------------------------------------------------
//------------------------------------------------------------------------


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
#   include <sys/statfs.h>
#   include <sys/statvfs.h>
#   include <sys/types.h>
#   define TEST_PLATFORM        ""
#   define TEST_PLATFORM_ROOT   "/"
#   define TEST_PLATFORM_TEMP   "tmp"
#   define PATH_LIST_DELIMITER  ":"
#   define PATH_SEPERATOR       "/"
#else                                   // Windows
#   include <windows.h>
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
    ::rtl::OUString a##str_name                 = rtl::OUString::createFromAscii( str_value )


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
OSLTEST_DECLARE( TmpName7, FILE_PREFIX TEST_PLATFORM "tmpname" );
OSLTEST_DECLARE( TmpName8, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpname/tmpdir" );
OSLTEST_DECLARE( TmpName9, FILE_PREFIX TEST_PLATFORM TEST_PLATFORM_TEMP "/tmpdir/../tmpdir/./" );
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
OSLTEST_DECLARE( VolURL2,  FILE_PREFIX  "dev/fd" );     //fd        Solaris
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


//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------


#endif /* _OSL_FILE_CONST_H_ */
