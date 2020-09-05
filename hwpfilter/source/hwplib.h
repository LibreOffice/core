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

#ifndef INCLUDED_HWPFILTER_SOURCE_HWPLIB_H
#define INCLUDED_HWPFILTER_SOURCE_HWPLIB_H
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
typedef unsigned short  hchar;
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

#if !defined(_WIN32) && !defined(MAX_PATH)
#  define MAX_PATH  260
#endif                                            /* MAX_PATH */

#ifndef TRUE
# define TRUE       1
# define FALSE      0
#endif

// HWP unit information
/**
 * HWP unit information
 */
#define ONE_POINT       25
#define ONE_ENG_CH_WIDTH    (ONE_POINT*(10/2))
#define ONE_MILI        (71)
#define ONE_DOT         (19)

/* HWP 3.0 문서 정보 */

#define NLanguage       7

/* 제어 문자 */
/**
 * Control character
 */
#define CH_ALIGNSPACE       0
#define CH_FIELD        5
#define CH_BOOKMARK     6

#define CH_DATE_FORM         7
#define CH_DATE_CODE         8
#define CH_TAB           9
#define CH_TEXT_BOX     10
#define CH_PICTURE      11

#define CH_END_PARA     13
#define CH_LINE         14
#define CH_HIDDEN       15
#define CH_HEADER_FOOTER    16
#define CH_FOOTNOTE     17
#define CH_AUTO_NUM     18                        /* pgnum, footnote/endnote, picture num */
#define CH_NEW_NUM      19                        /* pgnum, footnote/endnote, picture num */

#define CH_SHOW_PAGE_NUM    20
#define CH_PAGE_NUM_CTRL    21                    /* new chapter, hide pagenum */
#define CH_MAIL_MERGE       22
#define CH_COMPOSE      23
#define CH_HYPHEN       24

#define CH_TOC_MARK     25                        /* CONT_MARK, TBL_MARK, PIC_MARK */
#define CH_INDEX_MARK       26

#define CH_COL_DEF      27
#define CH_OUTLINE      28
#define CH_CROSSREF     29

#define CH_KEEP_SPACE       30
#define CH_FIXED_SPACE      31
#define CH_SPACE        32

#define HAVE_SUBPARA_MASK ((1<<CH_TEXT_BOX) | (1<<CH_PICTURE) | (1<<CH_HIDDEN) |  (1<<CH_HEADER_FOOTER) | (1<<CH_FOOTNOTE))

#define IS_SP_SKIP_BLOCK(hch) ((hch<5) || (hch==12) || (hch==27) || (hch == 29))

// file tag
#define FILETAG_END_OF_COMPRESSED   0x00000000

#define FILETAG_EMBEDDED_PICTURE    0x00000001
#define FILETAG_OLE_OBJECT      0x00000002
#define FILETAG_HYPERTEXT       0x00000003
#define FILETAG_PRESENTATION        0x00000004

#define FILETAG_END_OF_UNCOMPRESSED 0x80000000
#define FILETAG_PREVIEW_IMAGE       0x80000001
#define FILETAG_PREVIEW_TEXT        0x80000002

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
#endif // INCLUDED_HWPFILTER_SOURCE_HWPLIB_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
