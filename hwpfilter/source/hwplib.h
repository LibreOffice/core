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

#pragma once

#include "precompile.h"

#include <limits>
#include <new>
#include <string>

#include <o3tl/safeint.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

/* hwp96부터 hunit가 4byte가 되었다. */
/**
 * size of hunit is 4 since hwp96 version
 */
typedef char16_t  hchar;
typedef int     hunit;
typedef unsigned char       kchar;

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

typedef ::std::basic_string<hchar> hchar_string;

inline OUString fromHcharStringToOUString(hchar_string const & s) {
    if (s.length() > o3tl::make_unsigned(std::numeric_limits<sal_Int32>::max())) {
        throw std::bad_alloc();
    }
    return {reinterpret_cast<char16_t const *>(s.c_str()), sal_Int32(s.length())};
}

/**
 * @short Point
 */
typedef struct
{
    int   x, y;
} ZZPoint;

/**
 * @short Size
 */
typedef struct
{
    int   w, h;
} ZZSize;

/**
 * @short Rectangle
 */
typedef struct
{
    int   x, y;
    int   w, h;
} ZZRect;

/**
 * @short Parall
 */
typedef struct
{
    ZZPoint pt[3];
} ZZParall;

/**
 * @short Scale
 */
typedef struct
{
    int   mulX, divX;
    int   mulY, divY;
} ZZScaleXY;

#pragma once
# define TRUE       1
# define FALSE      0
#endif

// HWP unit information
/**
 * HWP unit information
 */
       25
    (ONE_POINT*(10/2))
        (71)
         (19)

/* HWP 3.0 문서 정보 */

       7

/* 제어 문자 */
/**
 * Control character
 */
       0
        5
     6

         7
         8
           9
     10
      11

     13
         14
       15
    16
     17
     18                        /* pgnum, footnote/endnote, picture num */
      19                        /* pgnum, footnote/endnote, picture num */

    20
    21                    /* new chapter, hide pagenum */
       22
      23
       24

     25                        /* CONT_MARK, TBL_MARK, PIC_MARK */
       26

      27
      28
     29

       30
      31
        32

 ((1<<CH_TEXT_BOX) | (1<<CH_PICTURE) | (1<<CH_HIDDEN) |  (1<<CH_HEADER_FOOTER) | (1<<CH_FOOTNOTE))

(hch) ((hch<5) || (hch==12) || (hch==27) || (hch == 29))

// file tag
   0x00000000

    0x00000001
      0x00000002
       0x00000003
        0x00000004

 0x80000000
       0x80000001
        0x80000002

// user error code
enum ErrorCode
{
    HWP_NoError,
// system error = errno
//    USER_ERROR_BIT = (1 << 16),
    HWP_InvalidFileName,
    HWP_InvalidFileFormat,
    HWP_BrokenFile,
    HWP_UNSUPPORTED_VERSION,
    HWP_EMPTY_FILE
};

// debug code
enum DebugMask
{
    DBG_READ = 0x01,
    DBG_DISPLAY = 0x02
};

#ifdef _WIN32
#ifdef HWPLIB_DLL
//#  define DLLEXPORT __declspec(dllexport)
#  define DLLEXPORT
#else
//#  define DLLEXPORT __declspec(dllimport)
#  define DLLEXPORT
#endif
#else
#  define DLLEXPORT
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef _WIN32
#  include <direct.h>
#  include <io.h>
#  define access _access
#  define stat _stat
#  define mkdir(x,y) _mkdir(x)
#  define rmdir _rmdir
#  define open _open
#  define write _write
#  define close _close
#  define unlink _unlink
#  define DIRSEP '\\'
#else
#  include <unistd.h>
#  define DIRSEP '/'
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
