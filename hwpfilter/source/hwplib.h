/*************************************************************************
 *
 *  $RCSfile: hwplib.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:37:17 $
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
 *  Copyright 2001 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2001 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* $Id: hwplib.h,v 1.1 2003-10-15 14:37:17 dvo Exp $ */

#ifndef _HWPLIB_H_
#define _HWPLIB_H_
#include "precompile.h"
//hwp96부터 hunit가 4byte가 되었다.
/**
 * size of hunit is 4 since hwp96 version
 */
#ifndef _HCHAR_
#define _HCHAR_
typedef unsigned short  hchar;
typedef int     hunit;
typedef unsigned char       kchar;
typedef unsigned char       echar;
#endif                                            // _HCHAR_

#if !defined(WIN32)
#ifndef _BOOL_T_
typedef unsigned short  BOOL;
#endif                                            /* _BOOL_T_ */
typedef unsigned short  WORD;
typedef int     SIZE;
typedef unsigned long   DWORD;
typedef long        LONG;
#endif                                            /* WIN32 */

#ifndef _UTYPE_
#define _UTYPE_
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;
#endif                                            /* _UTYPE_ */

#ifndef _ZZRECT_
#define _ZZRECT_
/**
 * @short Point
 */
typedef struct
{
    int   x, y;
} ZZPoint;

inline void allocPoint(ZZPoint &target, ZZPoint &src)
{
     target.x = src.x;
     target.y = src.y;
}

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
#endif                                            /* _ZZRECT_ */

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

// HWP 3.0 문서 정보

#define NLanguage       7

// 제어 문자
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
#define FILETAG_UNCOMPRESSED_BIT    0x80000000
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
#endif                                            /* _HWPLIB_H_*/
