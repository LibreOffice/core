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
#include <tools/toolsdllapi.h>
#include <ostream>
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

#define ERRCODE_ERROR_MASK               0x3fffffffUL
#define ERRCODE_WARNING_MASK             0x80000000UL
#define ERRCODE_RES_MASK                 0x7fff

#define ERRCODE_CLASS_SHIFT              8
#define ERRCODE_AREA_SHIFT               13
#define ERRCODE_DYNAMIC_SHIFT            26

#define ERRCODE_CLASS_MASK               (31UL << ERRCODE_CLASS_SHIFT)

#define ERRCODE_DYNAMIC_START            ( 1UL << ERRCODE_DYNAMIC_SHIFT)
#define ERRCODE_DYNAMIC_COUNT            31UL
#define ERRCODE_DYNAMIC_MASK             (31UL << ERRCODE_DYNAMIC_SHIFT)

/** we get included in contexts that don't have access to config_typesizes.h or C++11 */
#ifdef __RSC
#define ErrCodeUnderlyingType long
#else
#define ErrCodeUnderlyingType sal_uInt32
#endif

class ErrCode final
{
public:
    ErrCode(ErrCodeUnderlyingType value) : m_value(value) {}
    ErrCode() : m_value(0) {}

    explicit operator ErrCodeUnderlyingType() const { return m_value; }
    explicit operator bool() const { return m_value != 0; }

    bool operator<(ErrCode const & other) const { return m_value < other.m_value; }
    bool operator<=(ErrCode const & other) const { return m_value <= other.m_value; }
    bool operator>(ErrCode const & other) const { return m_value > other.m_value; }
    bool operator>=(ErrCode const & other) const { return m_value >= other.m_value; }
    bool operator==(ErrCode const & other) const { return m_value == other.m_value; }
    bool operator!=(ErrCode const & other) const { return m_value != other.m_value; }

    /** convert to ERRCODE_NONE if it's a warning, else return the error */
    ErrCode IgnoreWarning() const {
        return (m_value & ERRCODE_WARNING_MASK) ? ErrCode(0) : ErrCode(m_value & ERRCODE_ERROR_MASK);
    }

    bool IsWarning() const {
        return m_value & ERRCODE_WARNING_MASK;
    }

private:
    ErrCodeUnderlyingType m_value;
};

#ifndef __RSC
TOOLS_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const ErrCode&);
#endif


#define ERRCODE_AREA_TOOLS               (0   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SV                  (1   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SFX                 (2   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_INET                (3   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_IO                  ERRCODE_AREA_TOOLS

#define ERRCODE_AREA_LIB1                (8   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SVX                 ERRCODE_AREA_LIB1
#define ERRCODE_AREA_SVX_END             (ERRCODE_AREA_SO-1)
#define ERRCODE_AREA_SO                  (9   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SO_END              (ERRCODE_AREA_SBX-1)
#define ERRCODE_AREA_SBX                 (10   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_SBX_END             ((11  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_DB                  (11   << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_DB_END              ((12  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_JAVA                (12  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_JAVA_END            ((13  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_UUI                 (13  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_UUI_END             ((14  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_LIB2                (14  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_LIB2_END            ((15  << ERRCODE_AREA_SHIFT) - 1)
#define ERRCODE_AREA_CHAOS               (15  << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_CHAOS_END           ((16  << ERRCODE_AREA_SHIFT) - 1)

#define ERRCODE_AREA_APP1                (32 << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP2                (40 << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP3                (48 << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP4                (56 << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP5                (64 << ERRCODE_AREA_SHIFT)
#define ERRCODE_AREA_APP6                (72 << ERRCODE_AREA_SHIFT)

#define ERRCODE_AREA_SC                  ERRCODE_AREA_APP1
#define ERRCODE_AREA_SC_END              (ERRCODE_AREA_APP2-1)

#define ERRCODE_AREA_SD                  ERRCODE_AREA_APP2
#define ERRCODE_AREA_SD_END              (ERRCODE_AREA_APP3-1)

#define ERRCODE_AREA_SW                  ERRCODE_AREA_APP4
#define ERRCODE_AREA_SW_END              (ERRCODE_AREA_APP5-1)

#define ERRCODE_AREA_OFA_END             (ERRCODE_AREA_APP6-1)

#define ERRCODE_CLASS_NONE               ( 0  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ABORT              ( 1  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_GENERAL            ( 2  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_NOTEXISTS          ( 3  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ALREADYEXISTS      ( 4  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_ACCESS             ( 5  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_PATH               ( 6  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_LOCKING            ( 7  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_PARAMETER          ( 8  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SPACE              ( 9  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_NOTSUPPORTED       (10  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_READ               (11  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_WRITE              (12  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_UNKNOWN            (13  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_VERSION            (14  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_FORMAT             (15  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_CREATE             (16  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_IMPORT             (17  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_EXPORT             (18  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_FILTER             (19  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SO                 (20  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SBX                (21  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_RUNTIME            (22  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_COMPILER           (23  << ERRCODE_CLASS_SHIFT)

#define ERRCODE_NONE                     ErrCode(0UL)
#define ERRCODE_ABORT                    ERRCODE_IO_ABORT

#define ERRCODE_IO_MISPLACEDCHAR         ErrCode(1UL  | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTEXISTS             ErrCode(2UL  | ERRCODE_CLASS_NOTEXISTS | ERRCODE_AREA_IO)
#define ERRCODE_IO_ALREADYEXISTS         ErrCode(3UL  | ERRCODE_CLASS_ALREADYEXISTS | ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTADIRECTORY         ErrCode(4UL  | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTAFILE              ErrCode(5UL  | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDDEVICE         ErrCode(6UL  | ERRCODE_CLASS_PATH | ERRCODE_AREA_IO)
#define ERRCODE_IO_ACCESSDENIED          ErrCode(7UL  | ERRCODE_CLASS_ACCESS | ERRCODE_AREA_IO)
#define ERRCODE_IO_LOCKVIOLATION         ErrCode(8UL  | ERRCODE_CLASS_LOCKING | ERRCODE_AREA_IO)
#define ERRCODE_IO_OUTOFSPACE            ErrCode(9UL  | ERRCODE_CLASS_SPACE | ERRCODE_AREA_IO)
#define ERRCODE_IO_ISWILDCARD            ErrCode(11UL | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSUPPORTED          ErrCode(12UL | ERRCODE_CLASS_NOTSUPPORTED | ERRCODE_AREA_IO)
#define ERRCODE_IO_GENERAL               ErrCode(13UL | ERRCODE_CLASS_GENERAL | ERRCODE_AREA_IO)
#define ERRCODE_IO_TOOMANYOPENFILES      ErrCode(14UL | ERRCODE_CLASS_SPACE | ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTREAD              ErrCode(15UL | ERRCODE_CLASS_READ | ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTWRITE             ErrCode(16UL | ERRCODE_CLASS_WRITE | ERRCODE_AREA_IO)
#define ERRCODE_IO_OUTOFMEMORY           ErrCode(17UL | ERRCODE_CLASS_SPACE | ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTSEEK              ErrCode(18UL | ERRCODE_CLASS_GENERAL | ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTTELL              ErrCode(19UL | ERRCODE_CLASS_GENERAL | ERRCODE_AREA_IO)
#define ERRCODE_IO_WRONGVERSION          ErrCode(20UL | ERRCODE_CLASS_VERSION | ERRCODE_AREA_IO)
#define ERRCODE_IO_WRONGFORMAT           ErrCode(21UL | ERRCODE_CLASS_FORMAT | ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDCHAR           ErrCode(22UL | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_UNKNOWN               ErrCode(23UL | ERRCODE_CLASS_UNKNOWN | ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDACCESS         ErrCode(24UL | ERRCODE_CLASS_ACCESS | ERRCODE_AREA_IO)
#define ERRCODE_IO_CANTCREATE            ErrCode(25UL | ERRCODE_CLASS_CREATE | ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDPARAMETER      ErrCode(26UL | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_ABORT                 ErrCode(27UL | ERRCODE_CLASS_ABORT | ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTEXISTSPATH         ErrCode(28UL | ERRCODE_CLASS_NOTEXISTS | ERRCODE_AREA_IO)
#define ERRCODE_IO_PENDING               ErrCode(29UL | ERRCODE_CLASS_NOTEXISTS | ERRCODE_AREA_IO)
#define ERRCODE_IO_RECURSIVE             ErrCode(30UL | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_NAMETOOLONG           ErrCode(31UL | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_INVALIDLENGTH         ErrCode(32UL | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_CURRENTDIR            ErrCode(33UL | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSAMEDEVICE         ErrCode(34UL | ERRCODE_CLASS_PARAMETER | ERRCODE_AREA_IO)
#define ERRCODE_IO_DEVICENOTREADY        ErrCode(35UL | ERRCODE_CLASS_READ | ERRCODE_AREA_IO)
#define ERRCODE_IO_BADCRC                ErrCode(36UL | ERRCODE_CLASS_READ | ERRCODE_AREA_IO)
#define ERRCODE_IO_WRITEPROTECTED        ErrCode(37UL | ERRCODE_CLASS_ACCESS | ERRCODE_AREA_IO)
#define ERRCODE_IO_BROKENPACKAGE         ErrCode(38UL | ERRCODE_CLASS_FORMAT | ERRCODE_AREA_IO)
#define ERRCODE_IO_NOTSTORABLEINBINARYFORMAT ErrCode(39UL | ERRCODE_CLASS_FORMAT | ERRCODE_AREA_IO)

// FsysErrorCodes

// StreamErrorCodes
#define SVSTREAM_OK                      ERRCODE_NONE

#define SVSTREAM_GENERALERROR            ERRCODE_IO_GENERAL
#define SVSTREAM_FILE_NOT_FOUND          ERRCODE_IO_NOTEXISTS
#define SVSTREAM_PATH_NOT_FOUND          ERRCODE_IO_NOTEXISTSPATH
#define SVSTREAM_TOO_MANY_OPEN_FILES     ERRCODE_IO_TOOMANYOPENFILES
#define SVSTREAM_ACCESS_DENIED           ERRCODE_IO_ACCESSDENIED
#define SVSTREAM_SHARING_VIOLATION       ERRCODE_IO_LOCKVIOLATION
#define SVSTREAM_LOCKING_VIOLATION       ERRCODE_IO_LOCKVIOLATION
#define SVSTREAM_SHARE_BUFF_EXCEEDED     ERRCODE_IO_LOCKVIOLATION

#define SVSTREAM_INVALID_ACCESS          ERRCODE_IO_INVALIDACCESS
#define SVSTREAM_INVALID_HANDLE          ERRCODE_IO_GENERAL
#define SVSTREAM_CANNOT_MAKE             ERRCODE_IO_CANTCREATE
#define SVSTREAM_INVALID_PARAMETER       ERRCODE_IO_INVALIDPARAMETER

#define SVSTREAM_READ_ERROR              ERRCODE_IO_CANTREAD
#define SVSTREAM_WRITE_ERROR             ERRCODE_IO_CANTWRITE
#define SVSTREAM_SEEK_ERROR              ERRCODE_IO_CANTSEEK

#define SVSTREAM_OUTOFMEMORY             ERRCODE_IO_OUTOFMEMORY

#define SVSTREAM_FILEFORMAT_ERROR        ERRCODE_IO_WRONGFORMAT
#define SVSTREAM_WRONGVERSION            ERRCODE_IO_WRONGVERSION

#define SVSTREAM_DISK_FULL               ERRCODE_IO_OUTOFSPACE

// For the EditEngine:
#define SVSTREAM_ERRBASE_USER            ERRCODE_AREA_LIB1

#define PRINTER_OK                       ERRCODE_NONE
#define PRINTER_ABORT                    ERRCODE_IO_ABORT
#define PRINTER_GENERALERROR             ERRCODE_IO_GENERAL

#define ERRCODE_INET_NAME_RESOLVE        (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 1)
#define ERRCODE_INET_CONNECT             (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 2)
#define ERRCODE_INET_READ                (ERRCODE_AREA_INET | ERRCODE_CLASS_READ | 3)
#define ERRCODE_INET_WRITE               (ERRCODE_AREA_INET | ERRCODE_CLASS_WRITE| 4)
#define ERRCODE_INET_GENERAL             (ERRCODE_AREA_INET | ERRCODE_CLASS_WRITE |5)
#define ERRCODE_INET_OFFLINE             (ERRCODE_AREA_INET | ERRCODE_CLASS_READ |6)


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
