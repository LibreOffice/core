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
#include <climits>
#include <ostream>
#include <o3tl/typed_flags_set.hxx>
#include <optional>

#if defined(DBG_UTIL)
#include <o3tl/source_location.hxx>
#ifdef LIBO_USE_SOURCE_LOCATION
// LIBO_USE_SOURCE_LOCATION may be defined without DBG_UTIL, so a separate define is needed
#define LIBO_ERRMSG_USE_SOURCE_LOCATION
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
    COMPHELPER_DLLPUBLIC OUString toString() const;

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
    ErrCodeMsg(ErrCode code, const OUString& arg, o3tl::source_location loc = o3tl::source_location::current())
        : mnCode(code), maArg1(arg),  mnDialogMask(DialogMask::NONE), moLoc(loc) {}
    ErrCodeMsg(ErrCode code, const OUString& arg1, const OUString& arg2, o3tl::source_location loc = o3tl::source_location::current())
        : mnCode(code), maArg1(arg1), maArg2(arg2), mnDialogMask(DialogMask::NONE), moLoc(loc) {}
    ErrCodeMsg(ErrCode code, o3tl::source_location loc = o3tl::source_location::current())
        : mnCode(code), mnDialogMask(DialogMask::NONE), moLoc(loc) {}
    ErrCodeMsg(ErrCode code, const OUString& arg, DialogMask mask, o3tl::source_location loc = o3tl::source_location::current())
        : mnCode(code), maArg1(arg), mnDialogMask(mask), moLoc(loc) {}
    ErrCodeMsg(ErrCode code, const OUString& arg1, const OUString& arg2, DialogMask mask, o3tl::source_location loc = o3tl::source_location::current())
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
    const std::optional<o3tl::source_location>& GetSourceLocation() const { return moLoc; }
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
    std::optional<o3tl::source_location> moLoc;
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

constexpr ErrCode ERRCODE_NONE                   {0};

constexpr ErrCode ERRCODE_IO_MISPLACEDCHAR       { ErrCodeArea::Io, ErrCodeClass::Parameter, 1 };
constexpr ErrCode ERRCODE_IO_NOTEXISTS           { ErrCodeArea::Io, ErrCodeClass::NotExists, 2 };
constexpr ErrCode ERRCODE_IO_ALREADYEXISTS       { ErrCodeArea::Io, ErrCodeClass::AlreadyExists, 3 };
constexpr ErrCode ERRCODE_IO_NOTADIRECTORY       { ErrCodeArea::Io, ErrCodeClass::Parameter, 4 };
constexpr ErrCode ERRCODE_IO_NOTAFILE            { ErrCodeArea::Io, ErrCodeClass::Parameter, 5 };
constexpr ErrCode ERRCODE_IO_INVALIDDEVICE       { ErrCodeArea::Io, ErrCodeClass::Path, 6 };
constexpr ErrCode ERRCODE_IO_ACCESSDENIED        { ErrCodeArea::Io, ErrCodeClass::Access, 7 };
constexpr ErrCode ERRCODE_IO_LOCKVIOLATION       { ErrCodeArea::Io, ErrCodeClass::Locking, 8 };
constexpr ErrCode ERRCODE_IO_OUTOFSPACE          { ErrCodeArea::Io, ErrCodeClass::Space, 9 };
constexpr ErrCode ERRCODE_IO_ISWILDCARD          { ErrCodeArea::Io, ErrCodeClass::Parameter, 11 };
constexpr ErrCode ERRCODE_IO_NOTSUPPORTED        { ErrCodeArea::Io, ErrCodeClass::NotSupported, 12 };
constexpr ErrCode ERRCODE_IO_GENERAL             { ErrCodeArea::Io, ErrCodeClass::General, 13 };
constexpr ErrCode ERRCODE_IO_TOOMANYOPENFILES    { ErrCodeArea::Io, ErrCodeClass::Space, 14 };
constexpr ErrCode ERRCODE_IO_CANTREAD            { ErrCodeArea::Io, ErrCodeClass::Read, 15 };
constexpr ErrCode ERRCODE_IO_CANTWRITE           { ErrCodeArea::Io, ErrCodeClass::Write, 16 };
constexpr ErrCode ERRCODE_IO_OUTOFMEMORY         { ErrCodeArea::Io, ErrCodeClass::Space, 17 };
constexpr ErrCode ERRCODE_IO_CANTSEEK            { ErrCodeArea::Io, ErrCodeClass::General, 18 };
constexpr ErrCode ERRCODE_IO_CANTTELL            { ErrCodeArea::Io, ErrCodeClass::General, 19 };
constexpr ErrCode ERRCODE_IO_WRONGVERSION        { ErrCodeArea::Io, ErrCodeClass::Version, 20 };
constexpr ErrCode ERRCODE_IO_WRONGFORMAT         { ErrCodeArea::Io, ErrCodeClass::Format, 21 };
constexpr ErrCode ERRCODE_IO_INVALIDCHAR         { ErrCodeArea::Io, ErrCodeClass::Parameter, 22 };
constexpr ErrCode ERRCODE_IO_UNKNOWN             { ErrCodeArea::Io, ErrCodeClass::Unknown, 23 };
constexpr ErrCode ERRCODE_IO_INVALIDACCESS       { ErrCodeArea::Io, ErrCodeClass::Access, 24 };
constexpr ErrCode ERRCODE_IO_CANTCREATE          { ErrCodeArea::Io, ErrCodeClass::Create, 25 };
constexpr ErrCode ERRCODE_IO_INVALIDPARAMETER    { ErrCodeArea::Io, ErrCodeClass::Parameter, 26 };
constexpr ErrCode ERRCODE_IO_ABORT               { ErrCodeArea::Io, ErrCodeClass::Abort, 27 };
constexpr ErrCode ERRCODE_IO_NOTEXISTSPATH       { ErrCodeArea::Io, ErrCodeClass::NotExists, 28 };
constexpr ErrCode ERRCODE_IO_PENDING             { ErrCodeArea::Io, ErrCodeClass::NotExists, 29 };
constexpr ErrCode ERRCODE_IO_RECURSIVE           { ErrCodeArea::Io, ErrCodeClass::Parameter, 30 };
constexpr ErrCode ERRCODE_IO_NAMETOOLONG         { ErrCodeArea::Io, ErrCodeClass::Parameter, 31 };
constexpr ErrCode ERRCODE_IO_INVALIDLENGTH       { ErrCodeArea::Io, ErrCodeClass::Parameter, 32 };
constexpr ErrCode ERRCODE_IO_CURRENTDIR          { ErrCodeArea::Io, ErrCodeClass::Parameter, 33 };
constexpr ErrCode ERRCODE_IO_NOTSAMEDEVICE       { ErrCodeArea::Io, ErrCodeClass::Parameter, 34 };
constexpr ErrCode ERRCODE_IO_DEVICENOTREADY      { ErrCodeArea::Io, ErrCodeClass::Read, 35 };
constexpr ErrCode ERRCODE_IO_BADCRC              { ErrCodeArea::Io, ErrCodeClass::Read, 36 };
constexpr ErrCode ERRCODE_IO_WRITEPROTECTED      { ErrCodeArea::Io, ErrCodeClass::Access, 37 };
constexpr ErrCode ERRCODE_IO_BROKENPACKAGE       { ErrCodeArea::Io, ErrCodeClass::Format, 38 };
constexpr ErrCode ERRCODE_IO_NOTSTORABLEINBINARYFORMAT { ErrCodeArea::Io, ErrCodeClass::Format, 39 };
constexpr ErrCode ERRCODE_IO_FILTERDISABLED      { ErrCodeArea::Io, ErrCodeClass::Format, 40 };

// StreamErrorCodes

constexpr ErrCode SVSTREAM_GENERALERROR          = ERRCODE_IO_GENERAL;
constexpr ErrCode SVSTREAM_FILE_NOT_FOUND        = ERRCODE_IO_NOTEXISTS;
constexpr ErrCode SVSTREAM_PATH_NOT_FOUND        = ERRCODE_IO_NOTEXISTSPATH;
constexpr ErrCode SVSTREAM_TOO_MANY_OPEN_FILES   = ERRCODE_IO_TOOMANYOPENFILES;
constexpr ErrCode SVSTREAM_ACCESS_DENIED         = ERRCODE_IO_ACCESSDENIED;
constexpr ErrCode SVSTREAM_SHARING_VIOLATION     = ERRCODE_IO_LOCKVIOLATION;
constexpr ErrCode SVSTREAM_LOCKING_VIOLATION     = ERRCODE_IO_LOCKVIOLATION;
constexpr ErrCode SVSTREAM_SHARE_BUFF_EXCEEDED   = ERRCODE_IO_LOCKVIOLATION;

constexpr ErrCode SVSTREAM_INVALID_ACCESS        = ERRCODE_IO_INVALIDACCESS;
constexpr ErrCode SVSTREAM_INVALID_HANDLE        = ERRCODE_IO_GENERAL;
constexpr ErrCode SVSTREAM_CANNOT_MAKE           = ERRCODE_IO_CANTCREATE;
constexpr ErrCode SVSTREAM_INVALID_PARAMETER     = ERRCODE_IO_INVALIDPARAMETER;

constexpr ErrCode SVSTREAM_READ_ERROR            = ERRCODE_IO_CANTREAD;
constexpr ErrCode SVSTREAM_WRITE_ERROR           = ERRCODE_IO_CANTWRITE;
constexpr ErrCode SVSTREAM_SEEK_ERROR            = ERRCODE_IO_CANTSEEK;

constexpr ErrCode SVSTREAM_OUTOFMEMORY           = ERRCODE_IO_OUTOFMEMORY;

constexpr ErrCode SVSTREAM_FILEFORMAT_ERROR      = ERRCODE_IO_WRONGFORMAT;
constexpr ErrCode SVSTREAM_WRONGVERSION          = ERRCODE_IO_WRONGVERSION;

constexpr ErrCode SVSTREAM_DISK_FULL             = ERRCODE_IO_OUTOFSPACE;

constexpr ErrCode PRINTER_ABORT                  = ERRCODE_IO_ABORT;
constexpr ErrCode PRINTER_GENERALERROR           = ERRCODE_IO_GENERAL;

constexpr ErrCode ERRCODE_ABORT                  = ERRCODE_IO_ABORT;

constexpr ErrCode ERRCODE_INET_NAME_RESOLVE      {ErrCodeArea::Inet, ErrCodeClass::Read,  1};
constexpr ErrCode ERRCODE_INET_CONNECT           {ErrCodeArea::Inet, ErrCodeClass::Read,  2};
constexpr ErrCode ERRCODE_INET_READ              {ErrCodeArea::Inet, ErrCodeClass::Read,  3};
constexpr ErrCode ERRCODE_INET_WRITE             {ErrCodeArea::Inet, ErrCodeClass::Write, 4};
constexpr ErrCode ERRCODE_INET_GENERAL           {ErrCodeArea::Inet, ErrCodeClass::Write, 5};
constexpr ErrCode ERRCODE_INET_OFFLINE           {ErrCodeArea::Inet, ErrCodeClass::Read,  6};
constexpr ErrCode ERRCODE_INET_CONNECT_MSG       {ErrCodeArea::Inet, ErrCodeClass::Read,  7};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
