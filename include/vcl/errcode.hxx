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

#ifndef INCLUDED_VCL_ERRCODE_HXX
#define INCLUDED_VCL_ERRCODE_HXX

#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/toolsdllapi.h>
#include <ostream>

/*

01234567012345670123456701234567
||   ||           ||   ||      |
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

enum class ErrCodeArea;

class SAL_WARN_UNUSED ErrCode final
{
public:
    explicit constexpr ErrCode(ErrCodeArea nArea, sal_uInt32 value) : m_value(value | (sal_uInt32(nArea) << ERRCODE_AREA_SHIFT)) {}
    explicit constexpr ErrCode(sal_uInt32 value) : m_value(value) {}
    constexpr ErrCode() : m_value(0) {}

    explicit operator sal_uInt32() const { return m_value; }
    explicit operator bool() const { return m_value != 0; }

    bool operator<(ErrCode const & other) const { return m_value < other.m_value; }
    bool operator<=(ErrCode const & other) const { return m_value <= other.m_value; }
    bool operator>(ErrCode const & other) const { return m_value > other.m_value; }
    bool operator>=(ErrCode const & other) const { return m_value >= other.m_value; }
    bool operator==(ErrCode const & other) const { return m_value == other.m_value; }
    bool operator!=(ErrCode const & other) const { return m_value != other.m_value; }

    /** convert to ERRCODE_NONE if it's a warning, else return the error */
    ErrCode IgnoreWarning() const {
        return (m_value & ERRCODE_WARNING_MASK)
              ? ErrCode(0)
              : ErrCode(static_cast<sal_uInt32>(m_value & ERRCODE_ERROR_MASK));
    }

    bool IsWarning() const {
        return m_value & ERRCODE_WARNING_MASK;
    }

    ErrCode MakeWarning() const {
        return ErrCode(m_value | ERRCODE_WARNING_MASK);
    }

    bool IsError() {
        return m_value && !IsWarning();
    }

    bool IsDynamic() const {
        return m_value & ERRCODE_DYNAMIC_MASK;
    }

    sal_uInt32 GetDynamic() const {
        return (m_value & ERRCODE_DYNAMIC_MASK) >> ERRCODE_DYNAMIC_SHIFT;
    }

    ErrCode StripDynamic() const {
        return ErrCode(m_value & ~ERRCODE_DYNAMIC_MASK);
    }

    ErrCodeArea GetArea() const {
        return static_cast<ErrCodeArea>((m_value >> ERRCODE_AREA_SHIFT) & 0x01fff);
    }

    sal_uInt32 GetClass() const {
        return m_value & ERRCODE_CLASS_MASK;
    }

    sal_uInt16 GetRest() const {
        return m_value & ERRCODE_RES_MASK;
    }

    OUString toHexString() const {
        return "0x" + OUString::number(m_value, 16);
    }

    bool anyOf(ErrCode v) const {
      return *this == v;
    }

    template<typename... Args>
    bool anyOf(ErrCode first, Args... args) const {
      return *this == first || anyOf(args...);
    }
private:
    sal_uInt32 m_value;
};

inline std::ostream& operator<<(std::ostream& os, const ErrCode& err)
{
    os << sal_uInt32(err); return os;
}

enum class ErrCodeArea {
    Io                  = 0 ,
    Sv                  = 1 ,
    Sfx                 = 2 ,
    Inet                = 3 ,
    Vcl                 = 4 ,
    Svx                 = 8 ,
    So                  = 9 ,
    Sbx                 = 10,
    Db                  = 11,
    Java                = 12,
    Uui                 = 13,
    Lib2                = 14,
    Chaos               = 15,
    Sc                  = 32,
    Sd                  = 40,
    Sw                  = 56,
};

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
#define ERRCODE_CLASS_SO                 (20  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_SBX                (21  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_RUNTIME            (22  << ERRCODE_CLASS_SHIFT)
#define ERRCODE_CLASS_COMPILER           (23  << ERRCODE_CLASS_SHIFT)

#define ERRCODE_NONE                     ErrCode(0)

#define ERRCODE_IO_MISPLACEDCHAR         ErrCode( ErrCodeArea::Io, 1UL  | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_NOTEXISTS             ErrCode( ErrCodeArea::Io, 2UL  | ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_IO_ALREADYEXISTS         ErrCode( ErrCodeArea::Io, 3UL  | ERRCODE_CLASS_ALREADYEXISTS )
#define ERRCODE_IO_NOTADIRECTORY         ErrCode( ErrCodeArea::Io, 4UL  | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_NOTAFILE              ErrCode( ErrCodeArea::Io, 5UL  | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_INVALIDDEVICE         ErrCode( ErrCodeArea::Io, 6UL  | ERRCODE_CLASS_PATH )
#define ERRCODE_IO_ACCESSDENIED          ErrCode( ErrCodeArea::Io, 7UL  | ERRCODE_CLASS_ACCESS )
#define ERRCODE_IO_LOCKVIOLATION         ErrCode( ErrCodeArea::Io, 8UL  | ERRCODE_CLASS_LOCKING )
#define ERRCODE_IO_OUTOFSPACE            ErrCode( ErrCodeArea::Io, 9UL  | ERRCODE_CLASS_SPACE )
#define ERRCODE_IO_ISWILDCARD            ErrCode( ErrCodeArea::Io, 11UL | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_NOTSUPPORTED          ErrCode( ErrCodeArea::Io, 12UL | ERRCODE_CLASS_NOTSUPPORTED )
#define ERRCODE_IO_GENERAL               ErrCode( ErrCodeArea::Io, 13UL | ERRCODE_CLASS_GENERAL )
#define ERRCODE_IO_TOOMANYOPENFILES      ErrCode( ErrCodeArea::Io, 14UL | ERRCODE_CLASS_SPACE )
#define ERRCODE_IO_CANTREAD              ErrCode( ErrCodeArea::Io, 15UL | ERRCODE_CLASS_READ )
#define ERRCODE_IO_CANTWRITE             ErrCode( ErrCodeArea::Io, 16UL | ERRCODE_CLASS_WRITE )
#define ERRCODE_IO_OUTOFMEMORY           ErrCode( ErrCodeArea::Io, 17UL | ERRCODE_CLASS_SPACE )
#define ERRCODE_IO_CANTSEEK              ErrCode( ErrCodeArea::Io, 18UL | ERRCODE_CLASS_GENERAL )
#define ERRCODE_IO_CANTTELL              ErrCode( ErrCodeArea::Io, 19UL | ERRCODE_CLASS_GENERAL )
#define ERRCODE_IO_WRONGVERSION          ErrCode( ErrCodeArea::Io, 20UL | ERRCODE_CLASS_VERSION )
#define ERRCODE_IO_WRONGFORMAT           ErrCode( ErrCodeArea::Io, 21UL | ERRCODE_CLASS_FORMAT )
#define ERRCODE_IO_INVALIDCHAR           ErrCode( ErrCodeArea::Io, 22UL | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_UNKNOWN               ErrCode( ErrCodeArea::Io, 23UL | ERRCODE_CLASS_UNKNOWN )
#define ERRCODE_IO_INVALIDACCESS         ErrCode( ErrCodeArea::Io, 24UL | ERRCODE_CLASS_ACCESS )
#define ERRCODE_IO_CANTCREATE            ErrCode( ErrCodeArea::Io, 25UL | ERRCODE_CLASS_CREATE )
#define ERRCODE_IO_INVALIDPARAMETER      ErrCode( ErrCodeArea::Io, 26UL | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_ABORT                 ErrCode( ErrCodeArea::Io, 27UL | ERRCODE_CLASS_ABORT )
#define ERRCODE_IO_NOTEXISTSPATH         ErrCode( ErrCodeArea::Io, 28UL | ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_IO_PENDING               ErrCode( ErrCodeArea::Io, 29UL | ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_IO_RECURSIVE             ErrCode( ErrCodeArea::Io, 30UL | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_NAMETOOLONG           ErrCode( ErrCodeArea::Io, 31UL | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_INVALIDLENGTH         ErrCode( ErrCodeArea::Io, 32UL | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_CURRENTDIR            ErrCode( ErrCodeArea::Io, 33UL | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_NOTSAMEDEVICE         ErrCode( ErrCodeArea::Io, 34UL | ERRCODE_CLASS_PARAMETER )
#define ERRCODE_IO_DEVICENOTREADY        ErrCode( ErrCodeArea::Io, 35UL | ERRCODE_CLASS_READ )
#define ERRCODE_IO_BADCRC                ErrCode( ErrCodeArea::Io, 36UL | ERRCODE_CLASS_READ )
#define ERRCODE_IO_WRITEPROTECTED        ErrCode( ErrCodeArea::Io, 37UL | ERRCODE_CLASS_ACCESS )
#define ERRCODE_IO_BROKENPACKAGE         ErrCode( ErrCodeArea::Io, 38UL | ERRCODE_CLASS_FORMAT )
#define ERRCODE_IO_NOTSTORABLEINBINARYFORMAT ErrCode( ErrCodeArea::Io, 39UL | ERRCODE_CLASS_FORMAT )

// StreamErrorCodes

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

#define PRINTER_ABORT                    ERRCODE_IO_ABORT
#define PRINTER_GENERALERROR             ERRCODE_IO_GENERAL

#define ERRCODE_ABORT                    ERRCODE_IO_ABORT

#define ERRCODE_INET_NAME_RESOLVE        ErrCode(ErrCodeArea::Inet, ERRCODE_CLASS_READ | 1)
#define ERRCODE_INET_CONNECT             ErrCode(ErrCodeArea::Inet, ERRCODE_CLASS_READ | 2)
#define ERRCODE_INET_READ                ErrCode(ErrCodeArea::Inet, ERRCODE_CLASS_READ | 3)
#define ERRCODE_INET_WRITE               ErrCode(ErrCodeArea::Inet, ERRCODE_CLASS_WRITE| 4)
#define ERRCODE_INET_GENERAL             ErrCode(ErrCodeArea::Inet, ERRCODE_CLASS_WRITE| 5)
#define ERRCODE_INET_OFFLINE             ErrCode(ErrCodeArea::Inet, ERRCODE_CLASS_READ | 6)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
