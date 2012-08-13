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

#ifndef _ERRCODE_HXX
#define _ERRCODE_HXX

#ifndef __RSC
#include <tools/solar.h>
#endif

/*

01234567012345670123456701234567
||   ||          |||   ||      |
Warning           ||   ||      |
 |   ||           ||   ||      |
 Dynamic          ||   ||      |
      |           ||   ||      |
      Subsystemarea|   ||      |
                   |   ||      |
                   |   ||      |
                   |   ||      |
                   Class|      |
                        |      |
                        |      |
                        |      |
                        Code
*/

#define ERRCODE_BUTTON_OK                 0x01
#define ERRCODE_BUTTON_CANCEL             0x02
#define ERRCODE_BUTTON_RETRY              0x04
#define ERRCODE_BUTTON_OK_CANCEL          0x03
#define ERRCODE_BUTTON_OK_RETRY_CANCEL    0x07
#define ERRCODE_BUTTON_NO                 0x08
#define ERRCODE_BUTTON_YES                0x10
#define ERRCODE_BUTTON_YES_NO             0x18
#define ERRCODE_BUTTON_YES_NO_CANCEL      0x1a

#define ERRCODE_BUTTON_DEF_OK             0x100
#define ERRCODE_BUTTON_DEF_CANCEL         0x200
#define ERRCODE_BUTTON_DEF_YES            0x300
#define ERRCODE_BUTTON_DEF_NO             0x400

#define ERRCODE_MSG_ERROR                 0x1000
#define ERRCODE_MSG_WARNING               0x2000
#define ERRCODE_MSG_INFO                  0x3000
#define ERRCODE_MSG_QUERY                 0x4000

#define ERRCODE_ERROR_MASK                0x3fffffffUL
#define ERRCODE_WARNING_MASK              0x80000000UL
#define ERRCODE_RES_MASK                  0x7fff

#define ERRCODE_CLASS_SHIFT               8
#define ERRCODE_AREA_SHIFT                13
#define ERRCODE_DYNAMIC_SHIFT             26

#define ERRCODE_CLASS_MASK                (31UL  <<ERRCODE_CLASS_SHIFT)

#define ERRCODE_DYNAMIC_START             (1UL   <<ERRCODE_DYNAMIC_SHIFT)
#define ERRCODE_DYNAMIC_COUNT             31UL
#define ERRCODE_DYNAMIC_MASK              (31UL  <<ERRCODE_DYNAMIC_SHIFT)

#ifdef __RSC

#define ERRCODE_TOERRID(x) (x & ~ERRCODE_DYNAMIC_MASK)
#define ERRCODE_TOERROR(x) \
    ((x & ERRCODE_WARNING_MASK) ? 0 : (x & ERRCODE_ERROR_MASK))

#else

typedef sal_uIntPtr ErrCode;

inline sal_uIntPtr ERRCODE_TOERRID( sal_uIntPtr x )
{
    return x & ~ERRCODE_DYNAMIC_MASK;
}

inline sal_uIntPtr ERRCODE_TOERROR( sal_uIntPtr x )
{
    return ((x & ERRCODE_WARNING_MASK) ? 0 : (x & ERRCODE_ERROR_MASK));
}

#endif

#define ERRCODE_AREA_TOOLS               (0UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SV                  (1UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SFX                 (2UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_INET                (3UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_IO                   ERRCODE_AREA_TOOLS

#define ERRCODE_AREA_LIB1                (8UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SVX                 ERRCODE_AREA_LIB1
#define ERRCODE_AREA_SVX_END             (ERRCODE_AREA_SO-1)
#define ERRCODE_AREA_SO                  (9UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SO_END              (ERRCODE_AREA_SBX-1)
#define ERRCODE_AREA_SBX                 (10UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SBX_END             ((11UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_DB                  (11UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_DB_END              ((12UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_JAVA                (12UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_JAVA_END            ((13UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_UUI                 (13UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_UUI_END             ((14UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_LIB2                (14UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_LIB2_END            ((15UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_CHAOS               (15UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_CHAOS_END           ((16UL  << ERRCODE_AREA_SHIFT) - 1)

#define ERRCODE_AREA_APP1                (32UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP2                (40UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP3                (48UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP4                (56UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP5                (64UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP6                (72UL  << ERRCODE_AREA_SHIFT)

#define ERRCODE_AREA_SC                  ERRCODE_AREA_APP1
#define ERRCODE_AREA_SC_END              (ERRCODE_AREA_APP2-1)

#define ERRCODE_AREA_SD                  ERRCODE_AREA_APP2
#define ERRCODE_AREA_SD_END              (ERRCODE_AREA_APP3-1)

#define ERRCODE_AREA_SW                  ERRCODE_AREA_APP4
#define ERRCODE_AREA_SW_END              (ERRCODE_AREA_APP5-1)

#define ERRCODE_AREA_OFA                 ERRCODE_AREA_APP5
#define ERRCODE_AREA_OFA_END             (ERRCODE_AREA_APP6-1)

#define ERRCODE_CLASS_NONE               (0UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ABORT              (1UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_GENERAL            (2UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_NOTEXISTS          (3UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ALREADYEXISTS      (4UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ACCESS             (5UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_PATH               (6UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_LOCKING            (7UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_PARAMETER          (8UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SPACE              (9UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_NOTSUPPORTED       (10UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_READ               (11UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_WRITE              (12UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_UNKNOWN            (13UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_VERSION            (14UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_FORMAT             (15UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_CREATE             (16UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_IMPORT             (17UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_EXPORT             (18UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_FILTER             (19UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SO                 (20UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SBX                (21UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_RUNTIME            (22UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_COMPILER           (23UL  << ERRCODE_CLASS_SHIFT)

#define ERRCODE_NONE                  (0UL)
#define ERRCODE_ABORT                 ERRCODE_IO_ABORT

#define ERRCODE_IO_MISPLACEDCHAR      (1UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTEXISTS          (2UL |ERRCODE_CLASS_NOTEXISTS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_ALREADYEXISTS      (3UL |ERRCODE_CLASS_ALREADYEXISTS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTADIRECTORY      (4UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTAFILE           (5UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDDEVICE      (6UL |ERRCODE_CLASS_PATH|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_ACCESSDENIED       (7UL |ERRCODE_CLASS_ACCESS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_LOCKVIOLATION      (8UL |ERRCODE_CLASS_LOCKING|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_OUTOFSPACE         (9UL |ERRCODE_CLASS_SPACE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_ISWILDCARD         (11UL|ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSUPPORTED       (12UL|ERRCODE_CLASS_NOTSUPPORTED|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_GENERAL            (13UL|ERRCODE_CLASS_GENERAL|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_TOOMANYOPENFILES   (14UL|ERRCODE_CLASS_SPACE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTREAD           (15UL|ERRCODE_CLASS_READ|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTWRITE          (16UL|ERRCODE_CLASS_WRITE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_OUTOFMEMORY        (17UL|ERRCODE_CLASS_SPACE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTSEEK           (18UL|ERRCODE_CLASS_GENERAL|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTTELL           (19UL|ERRCODE_CLASS_GENERAL|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_WRONGVERSION       (20UL|ERRCODE_CLASS_VERSION|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_WRONGFORMAT        (21UL|ERRCODE_CLASS_FORMAT|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDCHAR        (22UL|ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_UNKNOWN            (23UL|ERRCODE_CLASS_UNKNOWN|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDACCESS      (24UL|ERRCODE_CLASS_ACCESS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTCREATE         (25UL|ERRCODE_CLASS_CREATE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDPARAMETER   (26UL|ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_ABORT              (27UL|ERRCODE_CLASS_ABORT|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTEXISTSPATH      (28UL |ERRCODE_CLASS_NOTEXISTS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_PENDING            (29UL |ERRCODE_CLASS_NOTEXISTS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_RECURSIVE          (30UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NAMETOOLONG        (31UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDLENGTH      (32UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CURRENTDIR         (33UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSAMEDEVICE      (34UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_DEVICENOTREADY     (35UL |ERRCODE_CLASS_READ|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_BADCRC             (36UL |ERRCODE_CLASS_READ|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_WRITEPROTECTED     (37UL |ERRCODE_CLASS_ACCESS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_BROKENPACKAGE      (38UL |ERRCODE_CLASS_FORMAT|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSTORABLEINBINARYFORMAT      (39UL |ERRCODE_CLASS_FORMAT|\
                                         ERRCODE_AREA_IO)

// FsysErrorCodes
#define FSYS_ERR_OK                     ERRCODE_NONE
#define FSYS_ERR_MISPLACEDCHAR          ERRCODE_IO_MISPLACEDCHAR
#define FSYS_ERR_INVALIDCHAR            ERRCODE_IO_INVALIDCHAR
#define FSYS_ERR_NOTEXISTS              ERRCODE_IO_NOTEXISTS
#define FSYS_ERR_ALREADYEXISTS          ERRCODE_IO_ALREADYEXISTS
#define FSYS_ERR_NOTADIRECTORY          ERRCODE_IO_NOTADIRECTORY
#define FSYS_ERR_NOTAFILE               ERRCODE_IO_NOTAFILE
#define FSYS_ERR_INVALIDDEVICE          ERRCODE_IO_INVALIDDEVICE
#define FSYS_ERR_ACCESSDENIED           ERRCODE_IO_ACCESSDENIED
#define FSYS_ERR_LOCKVIOLATION          ERRCODE_IO_LOCKVIOLATION
#define FSYS_ERR_VOLUMEFULL             ERRCODE_IO_OUTOFSPACE
#define FSYS_ERR_ISWILDCARD             ERRCODE_IO_ISWILDCARD
#define FSYS_ERR_NOTSUPPORTED           ERRCODE_IO_NOTSUPPORTED
#define FSYS_ERR_UNKNOWN                ERRCODE_IO_UNKNOWN

// StreamErrorCodes
#define SVSTREAM_OK                     ERRCODE_NONE

#define SVSTREAM_GENERALERROR           ERRCODE_IO_GENERAL
#define SVSTREAM_FILE_NOT_FOUND         ERRCODE_IO_NOTEXISTS
#define SVSTREAM_PATH_NOT_FOUND         ERRCODE_IO_NOTEXISTSPATH
#define SVSTREAM_TOO_MANY_OPEN_FILES    ERRCODE_IO_TOOMANYOPENFILES
#define SVSTREAM_ACCESS_DENIED          ERRCODE_IO_ACCESSDENIED
#define SVSTREAM_SHARING_VIOLATION      ERRCODE_IO_LOCKVIOLATION
#define SVSTREAM_LOCKING_VIOLATION      ERRCODE_IO_LOCKVIOLATION
#define SVSTREAM_SHARE_BUFF_EXCEEDED    ERRCODE_IO_LOCKVIOLATION

#define SVSTREAM_INVALID_ACCESS         ERRCODE_IO_INVALIDACCESS
#define SVSTREAM_INVALID_HANDLE         ERRCODE_IO_GENERAL
#define SVSTREAM_CANNOT_MAKE            ERRCODE_IO_CANTCREATE
#define SVSTREAM_INVALID_PARAMETER      ERRCODE_IO_INVALIDPARAMETER

#define SVSTREAM_READ_ERROR             ERRCODE_IO_CANTREAD
#define SVSTREAM_WRITE_ERROR            ERRCODE_IO_CANTWRITE
#define SVSTREAM_SEEK_ERROR             ERRCODE_IO_CANTSEEK
#define SVSTREAM_TELL_ERROR             ERRCODE_IO_CANTTELL

#define SVSTREAM_OUTOFMEMORY            ERRCODE_IO_OUTOFMEMORY

#define SVSTREAM_FILEFORMAT_ERROR       ERRCODE_IO_WRONGFORMAT
#define SVSTREAM_WRONGVERSION           ERRCODE_IO_WRONGVERSION

#define SVSTREAM_DISK_FULL              ERRCODE_IO_OUTOFSPACE

// For the EditEngine:
#define SVSTREAM_ERRBASE_USER           ERRCODE_AREA_LIB1

#define PRINTER_OK                      ERRCODE_NONE
#define PRINTER_ABORT                   ERRCODE_IO_ABORT
#define PRINTER_OUTOFMEMORY             ERRCODE_IO_OUTOFMEMORY
#define PRINTER_GENERALERROR            ERRCODE_IO_GENERAL
#define PRINTER_ACCESSDENIED            ERRCODE_IO_ACCESSDENIED

#define ERRCODE_INET_NAME_RESOLVE (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 1)
#define ERRCODE_INET_CONNECT      (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 2)
#define ERRCODE_INET_READ         (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 3)
#define ERRCODE_INET_WRITE        (ERRCODE_AREA_INET | ERRCODE_CLASS_WRITE| 4)
#define ERRCODE_INET_GENERAL      (ERRCODE_AREA_INET | ERRCODE_CLASS_WRITE |5)
#define ERRCODE_INET_OFFLINE      (ERRCODE_AREA_INET | ERRCODE_CLASS_READ |6)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
