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

#ifndef INCLUDED_TOOLS_ERRCODE_HXX
#define INCLUDED_TOOLS_ERRCODE_HXX

#ifndef __RSC
#include <tools/solar.h>
#define CAST_TO_UINT32(a) ((sal_uInt32)(a))
#else
#define CAST_TO_UINT32(a) (a)
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

#define ERRCODE_ERROR_MASK                0x3fffffffUL
#define ERRCODE_WARNING_MASK              0x80000000UL
#define ERRCODE_RES_MASK                  0x7fff

#define ERRCODE_CLASS_SHIFT               8
#define ERRCODE_AREA_SHIFT                13
#define ERRCODE_DYNAMIC_SHIFT             26

#define ERRCODE_CLASS_MASK                CAST_TO_UINT32(31UL  <<ERRCODE_CLASS_SHIFT)

#define ERRCODE_DYNAMIC_START             CAST_TO_UINT32(1UL   <<ERRCODE_DYNAMIC_SHIFT)
#define ERRCODE_DYNAMIC_COUNT             31UL
#define ERRCODE_DYNAMIC_MASK              CAST_TO_UINT32(31UL  <<ERRCODE_DYNAMIC_SHIFT)

#ifdef __RSC

#define ERRCODE_TOERROR(x) \
    ((x & ERRCODE_WARNING_MASK) ? 0 : (x & ERRCODE_ERROR_MASK))

#else

typedef sal_uInt32 ErrCode;

inline sal_uInt32 ERRCODE_TOERROR( ErrCode x )
{
    return ((x & ERRCODE_WARNING_MASK) ? 0 : (x & ERRCODE_ERROR_MASK));
}

#endif

#define ERRCODE_AREA_TOOLS               CAST_TO_UINT32(0UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SV                  CAST_TO_UINT32(1UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SFX                 CAST_TO_UINT32(2UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_INET                CAST_TO_UINT32(3UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_IO                   ERRCODE_AREA_TOOLS

#define ERRCODE_AREA_LIB1                CAST_TO_UINT32(8UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SVX                 ERRCODE_AREA_LIB1
#define ERRCODE_AREA_SVX_END             (ERRCODE_AREA_SO-1)
#define ERRCODE_AREA_SO                  CAST_TO_UINT32(9UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SO_END              (ERRCODE_AREA_SBX-1)
#define ERRCODE_AREA_SBX                 CAST_TO_UINT32(10UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SBX_END             CAST_TO_UINT32((11UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_DB                  CAST_TO_UINT32(11UL   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_DB_END              CAST_TO_UINT32((12UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_JAVA                CAST_TO_UINT32(12UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_JAVA_END            CAST_TO_UINT32((13UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_UUI                 CAST_TO_UINT32(13UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_UUI_END             CAST_TO_UINT32((14UL  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_LIB2                CAST_TO_UINT32(14UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_LIB2_END            CAST_TO_UINT32((15UL  << ERRCODE_AREA_SHIFT) - 1)

#define ERRCODE_AREA_APP1                CAST_TO_UINT32(32UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP2                CAST_TO_UINT32(40UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP3                CAST_TO_UINT32(48UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP4                CAST_TO_UINT32(56UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP5                CAST_TO_UINT32(64UL  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP6                CAST_TO_UINT32(72UL  << ERRCODE_AREA_SHIFT)

#define ERRCODE_AREA_SC                  ERRCODE_AREA_APP1
#define ERRCODE_AREA_SC_END              (ERRCODE_AREA_APP2-1)

#define ERRCODE_AREA_SD                  ERRCODE_AREA_APP2
#define ERRCODE_AREA_SD_END              (ERRCODE_AREA_APP3-1)

#define ERRCODE_AREA_SW                  ERRCODE_AREA_APP4
#define ERRCODE_AREA_SW_END              (ERRCODE_AREA_APP5-1)

#define ERRCODE_AREA_OFA_END             (ERRCODE_AREA_APP6-1)

#define ERRCODE_CLASS_NONE               CAST_TO_UINT32(0UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ABORT              CAST_TO_UINT32(1UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_GENERAL            CAST_TO_UINT32(2UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_NOTEXISTS          CAST_TO_UINT32(3UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ALREADYEXISTS      CAST_TO_UINT32(4UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ACCESS             CAST_TO_UINT32(5UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_PATH               CAST_TO_UINT32(6UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_LOCKING            CAST_TO_UINT32(7UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_PARAMETER          CAST_TO_UINT32(8UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SPACE              CAST_TO_UINT32(9UL   << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_NOTSUPPORTED       CAST_TO_UINT32(10UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_READ               CAST_TO_UINT32(11UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_WRITE              CAST_TO_UINT32(12UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_UNKNOWN            CAST_TO_UINT32(13UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_VERSION            CAST_TO_UINT32(14UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_FORMAT             CAST_TO_UINT32(15UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_CREATE             CAST_TO_UINT32(16UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_IMPORT             CAST_TO_UINT32(17UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_EXPORT             CAST_TO_UINT32(18UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_FILTER             CAST_TO_UINT32(19UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SO                 CAST_TO_UINT32(20UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SBX                CAST_TO_UINT32(21UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_RUNTIME            CAST_TO_UINT32(22UL  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_COMPILER           CAST_TO_UINT32(23UL  << ERRCODE_CLASS_SHIFT)

#define ERRCODE_NONE                  CAST_TO_UINT32(0UL)
#define ERRCODE_ABORT                 ERRCODE_IO_ABORT

#define ERRCODE_IO_MISPLACEDCHAR      CAST_TO_UINT32(1UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTEXISTS          CAST_TO_UINT32(2UL |ERRCODE_CLASS_NOTEXISTS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_ALREADYEXISTS      CAST_TO_UINT32(3UL |ERRCODE_CLASS_ALREADYEXISTS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTADIRECTORY      CAST_TO_UINT32(4UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTAFILE           CAST_TO_UINT32(5UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDDEVICE      CAST_TO_UINT32(6UL |ERRCODE_CLASS_PATH|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_ACCESSDENIED       CAST_TO_UINT32(7UL |ERRCODE_CLASS_ACCESS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_LOCKVIOLATION      CAST_TO_UINT32(8UL |ERRCODE_CLASS_LOCKING|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_OUTOFSPACE         CAST_TO_UINT32(9UL |ERRCODE_CLASS_SPACE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_ISWILDCARD         CAST_TO_UINT32(11UL|ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSUPPORTED       CAST_TO_UINT32(12UL|ERRCODE_CLASS_NOTSUPPORTED|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_GENERAL            CAST_TO_UINT32(13UL|ERRCODE_CLASS_GENERAL|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_TOOMANYOPENFILES   CAST_TO_UINT32(14UL|ERRCODE_CLASS_SPACE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTREAD           CAST_TO_UINT32(15UL|ERRCODE_CLASS_READ|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTWRITE          CAST_TO_UINT32(16UL|ERRCODE_CLASS_WRITE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_OUTOFMEMORY        CAST_TO_UINT32(17UL|ERRCODE_CLASS_SPACE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTSEEK           CAST_TO_UINT32(18UL|ERRCODE_CLASS_GENERAL|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTTELL           CAST_TO_UINT32(19UL|ERRCODE_CLASS_GENERAL|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_WRONGVERSION       CAST_TO_UINT32(20UL|ERRCODE_CLASS_VERSION|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_WRONGFORMAT        CAST_TO_UINT32(21UL|ERRCODE_CLASS_FORMAT|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDCHAR        CAST_TO_UINT32(22UL|ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_UNKNOWN            CAST_TO_UINT32(23UL|ERRCODE_CLASS_UNKNOWN|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDACCESS      CAST_TO_UINT32(24UL|ERRCODE_CLASS_ACCESS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTCREATE         CAST_TO_UINT32(25UL|ERRCODE_CLASS_CREATE|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDPARAMETER   CAST_TO_UINT32(26UL|ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_ABORT              CAST_TO_UINT32(27UL|ERRCODE_CLASS_ABORT|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTEXISTSPATH      CAST_TO_UINT32(28UL |ERRCODE_CLASS_NOTEXISTS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_PENDING            CAST_TO_UINT32(29UL |ERRCODE_CLASS_NOTEXISTS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_RECURSIVE          CAST_TO_UINT32(30UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NAMETOOLONG        CAST_TO_UINT32(31UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDLENGTH      CAST_TO_UINT32(32UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_CURRENTDIR         CAST_TO_UINT32(33UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSAMEDEVICE      CAST_TO_UINT32(34UL |ERRCODE_CLASS_PARAMETER|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_DEVICENOTREADY     CAST_TO_UINT32(35UL |ERRCODE_CLASS_READ|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_BADCRC             CAST_TO_UINT32(36UL |ERRCODE_CLASS_READ|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_WRITEPROTECTED     CAST_TO_UINT32(37UL |ERRCODE_CLASS_ACCESS|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_BROKENPACKAGE      CAST_TO_UINT32(38UL |ERRCODE_CLASS_FORMAT|\
                                         ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSTORABLEINBINARYFORMAT      CAST_TO_UINT32(39UL |ERRCODE_CLASS_FORMAT|\
                                         ERRCODE_AREA_IO)

// FsysErrorCodes

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

#define SVSTREAM_OUTOFMEMORY            ERRCODE_IO_OUTOFMEMORY

#define SVSTREAM_FILEFORMAT_ERROR       ERRCODE_IO_WRONGFORMAT
#define SVSTREAM_WRONGVERSION           ERRCODE_IO_WRONGVERSION

#define SVSTREAM_DISK_FULL              ERRCODE_IO_OUTOFSPACE

// For the EditEngine:
#define SVSTREAM_ERRBASE_USER           ERRCODE_AREA_LIB1

#define PRINTER_OK                      ERRCODE_NONE
#define PRINTER_ABORT                   ERRCODE_IO_ABORT
#define PRINTER_GENERALERROR            ERRCODE_IO_GENERAL

#define ERRCODE_INET_NAME_RESOLVE (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 1)
#define ERRCODE_INET_CONNECT      (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 2)
#define ERRCODE_INET_READ         (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 3)
#define ERRCODE_INET_WRITE        (ERRCODE_AREA_INET | ERRCODE_CLASS_WRITE| 4)
#define ERRCODE_INET_GENERAL      (ERRCODE_AREA_INET | ERRCODE_CLASS_WRITE |5)
#define ERRCODE_INET_OFFLINE      (ERRCODE_AREA_INET | ERRCODE_CLASS_READ |6)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
