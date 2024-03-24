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

#include <config_options.h>
#include <rtl/ustring.hxx>
#include <comphelper/comphelperdllapi.h>
#include <ostream>
#include <o3tl/typed_flags_set.hxx>
#include <optional>
#include <compare>

#if defined(DBG_UTIL)
#if __has_include(<version>)
#include <version>
#endif
#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907
#include <source_location>
#define LIBO_ERRMSG_USE_SOURCE_LOCATION std
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
#define LIBO_ERRMSG_USE_SOURCE_LOCATION std::experimental
#endif
#endif

/*

01234567012345670123456701234567
||   ||           ||   ||      |
Warning           ||   ||      |
 |   ||           ||   ||      |
 Unused           ||   ||      |
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

#define ERRCODE_CLASS_SHIFT              8
#define ERRCODE_AREA_SHIFT               13
#define ERRCODE_DYNAMIC_SHIFT            26

#define ERRCODE_CLASS_MASK               (31UL << ERRCODE_CLASS_SHIFT)

enum class ErrCodeArea;
enum class ErrCodeClass;

enum class WarningFlag { Yes };

class SAL_WARN_UNUSED ErrCode final
{
public:
    explicit constexpr ErrCode(WarningFlag, ErrCodeArea nArea, ErrCodeClass nClass, sal_uInt16 nCode)
        : m_value(ERRCODE_WARNING_MASK | (sal_uInt32(nArea) << ERRCODE_AREA_SHIFT) | (sal_uInt32(nClass) << ERRCODE_CLASS_SHIFT) | nCode)
    {
        assert(nCode <= 0xff && "code out of range");
    }
    explicit constexpr ErrCode(ErrCodeArea nArea, ErrCodeClass nClass, sal_uInt16 nCode)
        : m_value((sal_uInt32(nArea) << ERRCODE_AREA_SHIFT) | (sal_uInt32(nClass) << ERRCODE_CLASS_SHIFT) | nCode)
    {
        assert(nCode <= 0xff && "code out of range");
    }
    explicit constexpr ErrCode(ErrCodeArea nArea, sal_uInt16 nClassAndCode)
        : m_value((sal_uInt32(nArea) << ERRCODE_AREA_SHIFT) | nClassAndCode) {}
    explicit constexpr ErrCode(sal_uInt32 nValue)
        : m_value(nValue) {}
    constexpr ErrCode()
        : m_value(0) {}

    explicit operator sal_uInt32() const { return m_value; }
    explicit operator bool() const { return m_value != 0; }

    auto operator<=>(ErrCode const & other) const = default;

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

    bool IsError() const {
        return m_value && !IsWarning();
    }

    constexpr ErrCode StripWarning() const {
        return ErrCode(m_value & ~ERRCODE_WARNING_MASK);
    }

    constexpr ErrCodeArea GetArea() const {
        return static_cast<ErrCodeArea>((m_value >> ERRCODE_AREA_SHIFT) & 0x01fff);
    }

    constexpr ErrCodeClass GetClass() const {
        return static_cast<ErrCodeClass>((m_value >> ERRCODE_CLASS_SHIFT) & 0x1f);
    }

    constexpr sal_uInt8 GetCode() const {
        return static_cast<sal_uInt8>(m_value & 0xff);
    }

    OUString toHexString() const {
        return "0x" + OUString::number(m_value, 16);
    }

    /// Return a string suitable for debug output, the same as the operator<< function
    UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OUString toString() const;
    
    template <typename... Args> bool anyOf(Args... args) const
    {
        static_assert(sizeof...(args) > 0);
        return (... || (*this == args));
    }

private:
    sal_uInt32 m_value;
};

COMPHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const ErrCode& err);

enum class DialogMask
{
    NONE                    = 0x0000,
    ButtonsOk               = 0x0001,
    ButtonsCancel           = 0x0002,
    ButtonsRetry            = 0x0004,
    ButtonsNo               = 0x0008,
    ButtonsYes              = 0x0010,
    ButtonsYesNo            = 0x0018,

    ButtonDefaultsOk        = 0x0100,
    ButtonDefaultsCancel    = 0x0200,
    ButtonDefaultsYes       = 0x0300,
    ButtonDefaultsNo        = 0x0400,

    MessageError            = 0x1000,
    MessageWarning          = 0x2000,
    MessageInfo             = 0x3000,

    MAX                     = USHRT_MAX,
};
namespace o3tl
{
    template<> struct typed_flags<DialogMask> : is_typed_flags<DialogMask, 0xffff> {};
}

/** Wrap up an ErrCode and an explanation and the source location where the error was created,
    helps with debugging when finding the source of a problem.
*/
class SAL_WARN_UNUSED ErrCodeMsg
{
public:
    ErrCodeMsg() : mnCode(0), mnDialogMask(DialogMask::NONE) {}
#ifdef LIBO_ERRMSG_USE_SOURCE_LOCATION
    ErrCodeMsg(ErrCode code, const OUString& arg, LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location loc = LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location::current())
        : mnCode(code), maArg1(arg),  mnDialogMask(DialogMask::NONE), moLoc(loc) {}
    ErrCodeMsg(ErrCode code, const OUString& arg1, const OUString& arg2, LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location loc = LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location::current())
        : mnCode(code), maArg1(arg1), maArg2(arg2), mnDialogMask(DialogMask::NONE), moLoc(loc) {}
    ErrCodeMsg(ErrCode code, LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location loc = LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location::current())
        : mnCode(code), mnDialogMask(DialogMask::NONE), moLoc(loc) {}
    ErrCodeMsg(ErrCode code, const OUString& arg, DialogMask mask, LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location loc = LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location::current())
        : mnCode(code), maArg1(arg), mnDialogMask(mask), moLoc(loc) {}
    ErrCodeMsg(ErrCode code, const OUString& arg1, const OUString& arg2, DialogMask mask, LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location loc = LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location::current())
        : mnCode(code), maArg1(arg1), maArg2(arg2), mnDialogMask(mask), moLoc(loc) {}
#else
    ErrCodeMsg(ErrCode code, const OUString& arg)
        : mnCode(code), maArg1(arg), mnDialogMask(DialogMask::NONE) {}
    ErrCodeMsg(ErrCode code, const OUString& arg1, const OUString& arg2)
        : mnCode(code), maArg1(arg1), maArg2(arg2), mnDialogMask(DialogMask::NONE) {}
    ErrCodeMsg(ErrCode code)
        : mnCode(code), mnDialogMask(DialogMask::NONE) {}
    ErrCodeMsg(ErrCode code, const OUString& arg, DialogMask mask)
        : mnCode(code), maArg1(arg), mnDialogMask(mask) {}
    ErrCodeMsg(ErrCode code, const OUString& arg1, const OUString& arg2, DialogMask mask)
        : mnCode(code), maArg1(arg1), maArg2(arg2), mnDialogMask(mask) {}
#endif

    const ErrCode & GetCode() const { return mnCode; }
    const OUString & GetArg1() const { return maArg1; }
    const OUString & GetArg2() const { return maArg2; }
    DialogMask GetDialogMask() const { return mnDialogMask; }

#ifdef LIBO_ERRMSG_USE_SOURCE_LOCATION
    const std::optional<LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location> & GetSourceLocation() const { return moLoc; }
#endif

    /** convert to ERRCODE_NONE if it's a warning, else return the error */
    ErrCodeMsg IgnoreWarning() const { return mnCode.IsWarning() ? ErrCodeMsg(ErrCode(0)) : *this; }

    bool IsWarning() const { return mnCode.IsWarning(); }
    bool IsError() const { return mnCode.IsError(); }
    explicit operator bool() const { return bool(mnCode); }
    bool operator==(const ErrCodeMsg& rOther) const { return mnCode == rOther.mnCode; }
    bool operator!=(const ErrCodeMsg& rOther) const { return mnCode != rOther.mnCode; }

    /// Return a string suitable for debug output, the same as the operator<< function
    UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OUString toString() const;

private:
    ErrCode mnCode;
    OUString maArg1;
    OUString maArg2;
    DialogMask mnDialogMask;
#ifdef LIBO_ERRMSG_USE_SOURCE_LOCATION
    std::optional<LIBO_ERRMSG_USE_SOURCE_LOCATION::source_location> moLoc;
#endif
};

COMPHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const ErrCodeMsg& err);

inline bool operator==(const ErrCodeMsg& lhs, ErrCode rhs) { return lhs.GetCode() == rhs; }
inline bool operator!=(const ErrCodeMsg& lhs, ErrCode rhs) { return lhs.GetCode() != rhs; }
inline bool operator==(ErrCode lhs, const ErrCodeMsg& rhs) { return lhs == rhs.GetCode(); }
inline bool operator!=(ErrCode lhs, const ErrCodeMsg& rhs) { return lhs != rhs.GetCode(); }

enum class ErrCodeArea {
    Io                  = 0 ,
    Sfx                 = 2 ,
    Inet                = 3 ,
    Vcl                 = 4 ,
    Svx                 = 8 ,
    So                  = 9 ,
    Sbx                 = 10,
    Uui                 = 13,
    Sc                  = 32,
    Sd                  = 40,
    Sw                  = 56,
};

enum class ErrCodeClass {
    NONE               =  0,
    Abort              =  1,
    General            =  2,
    NotExists          =  3,
    AlreadyExists      =  4,
    Access             =  5,
    Path               =  6,
    Locking            =  7,
    Parameter          =  8,
    Space              =  9,
    NotSupported       = 10,
    Read               = 11,
    Write              = 12,
    Unknown            = 13,
    Version            = 14,
    Format             = 15,
    Create             = 16,
    Import             = 17,
    Export             = 18,
    So                 = 20,
    Sbx                = 21,
    Runtime            = 22,
    Compiler           = 23
};

#define ERRCODE_NONE                     ErrCode(0)

#define ERRCODE_IO_MISPLACEDCHAR         ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 1 )
#define ERRCODE_IO_NOTEXISTS             ErrCode( ErrCodeArea::Io, ErrCodeClass::NotExists, 2 )
#define ERRCODE_IO_ALREADYEXISTS         ErrCode( ErrCodeArea::Io, ErrCodeClass::AlreadyExists, 3 )
#define ERRCODE_IO_NOTADIRECTORY         ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 4 )
#define ERRCODE_IO_NOTAFILE              ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 5 )
#define ERRCODE_IO_INVALIDDEVICE         ErrCode( ErrCodeArea::Io, ErrCodeClass::Path, 6 )
#define ERRCODE_IO_ACCESSDENIED          ErrCode( ErrCodeArea::Io, ErrCodeClass::Access, 7 )
#define ERRCODE_IO_LOCKVIOLATION         ErrCode( ErrCodeArea::Io, ErrCodeClass::Locking, 8 )
#define ERRCODE_IO_OUTOFSPACE            ErrCode( ErrCodeArea::Io, ErrCodeClass::Space, 9 )
#define ERRCODE_IO_ISWILDCARD            ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 11 )
#define ERRCODE_IO_NOTSUPPORTED          ErrCode( ErrCodeArea::Io, ErrCodeClass::NotSupported, 12 )
#define ERRCODE_IO_GENERAL               ErrCode( ErrCodeArea::Io, ErrCodeClass::General, 13 )
#define ERRCODE_IO_TOOMANYOPENFILES      ErrCode( ErrCodeArea::Io, ErrCodeClass::Space, 14 )
#define ERRCODE_IO_CANTREAD              ErrCode( ErrCodeArea::Io, ErrCodeClass::Read, 15 )
#define ERRCODE_IO_CANTWRITE             ErrCode( ErrCodeArea::Io, ErrCodeClass::Write, 16 )
#define ERRCODE_IO_OUTOFMEMORY           ErrCode( ErrCodeArea::Io, ErrCodeClass::Space, 17 )
#define ERRCODE_IO_CANTSEEK              ErrCode( ErrCodeArea::Io, ErrCodeClass::General, 18 )
#define ERRCODE_IO_CANTTELL              ErrCode( ErrCodeArea::Io, ErrCodeClass::General, 19 )
#define ERRCODE_IO_WRONGVERSION          ErrCode( ErrCodeArea::Io, ErrCodeClass::Version, 20 )
#define ERRCODE_IO_WRONGFORMAT           ErrCode( ErrCodeArea::Io, ErrCodeClass::Format, 21 )
#define ERRCODE_IO_INVALIDCHAR           ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 22 )
#define ERRCODE_IO_UNKNOWN               ErrCode( ErrCodeArea::Io, ErrCodeClass::Unknown, 23 )
#define ERRCODE_IO_INVALIDACCESS         ErrCode( ErrCodeArea::Io, ErrCodeClass::Access, 24 )
#define ERRCODE_IO_CANTCREATE            ErrCode( ErrCodeArea::Io, ErrCodeClass::Create, 25 )
#define ERRCODE_IO_INVALIDPARAMETER      ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 26 )
#define ERRCODE_IO_ABORT                 ErrCode( ErrCodeArea::Io, ErrCodeClass::Abort, 27 )
#define ERRCODE_IO_NOTEXISTSPATH         ErrCode( ErrCodeArea::Io, ErrCodeClass::NotExists, 28 )
#define ERRCODE_IO_PENDING               ErrCode( ErrCodeArea::Io, ErrCodeClass::NotExists, 29 )
#define ERRCODE_IO_RECURSIVE             ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 30 )
#define ERRCODE_IO_NAMETOOLONG           ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 31 )
#define ERRCODE_IO_INVALIDLENGTH         ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 32 )
#define ERRCODE_IO_CURRENTDIR            ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 33 )
#define ERRCODE_IO_NOTSAMEDEVICE         ErrCode( ErrCodeArea::Io, ErrCodeClass::Parameter, 34 )
#define ERRCODE_IO_DEVICENOTREADY        ErrCode( ErrCodeArea::Io, ErrCodeClass::Read, 35 )
#define ERRCODE_IO_BADCRC                ErrCode( ErrCodeArea::Io, ErrCodeClass::Read, 36 )
#define ERRCODE_IO_WRITEPROTECTED        ErrCode( ErrCodeArea::Io, ErrCodeClass::Access, 37 )
#define ERRCODE_IO_BROKENPACKAGE         ErrCode( ErrCodeArea::Io, ErrCodeClass::Format, 38 )
#define ERRCODE_IO_NOTSTORABLEINBINARYFORMAT ErrCode( ErrCodeArea::Io, ErrCodeClass::Format, 39 )
#define ERRCODE_IO_FILTERDISABLED        ErrCode( ErrCodeArea::Io, ErrCodeClass::Format, 40 )

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

#define ERRCODE_INET_NAME_RESOLVE        ErrCode(ErrCodeArea::Inet, ErrCodeClass::Read,  1)
#define ERRCODE_INET_CONNECT             ErrCode(ErrCodeArea::Inet, ErrCodeClass::Read,  2)
#define ERRCODE_INET_READ                ErrCode(ErrCodeArea::Inet, ErrCodeClass::Read,  3)
#define ERRCODE_INET_WRITE               ErrCode(ErrCodeArea::Inet, ErrCodeClass::Write, 4)
#define ERRCODE_INET_GENERAL             ErrCode(ErrCodeArea::Inet, ErrCodeClass::Write, 5)
#define ERRCODE_INET_OFFLINE             ErrCode(ErrCodeArea::Inet, ErrCodeClass::Read,  6)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
