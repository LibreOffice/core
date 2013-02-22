/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/unoidl.hxx"
#include "osl/endian.h"
#include "osl/file.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"

#include "unoidlprovider.hxx"

// New binary format:
//
// Uses the following definitions:
//
// * UInt16: 2-byte value, LSB first
// * UInt32: 4-byte value, LSB first
// * UInt64: 8-byte value, LSB first
// * Offset: UInt32 value, counting bytes from start of file
// * NUL-Name: zero or more non-NUL US-ASCII bytes followed by a NUL byte
// * Len-Name: UInt32 number of characters, with 0x80000000 bit 1, followed by
//    that many (- 0x80000000) US-ASCII bytes
// * Idx-Name: either an Offset (with 0x80000000 bit 0) of a Len-Name, or a
//    Len-Name
// * Entry: Offset of NUL-Name followed by Offset of payload
// * Map: zero or more Entries
//
// Layout of per-entry payload in the root or a module Map:
//
// * kind byte:
// ** 0: module
// *** followed by:
// **** UInt32 number N1 of entries of Map
// **** N1 * Entry
// ** otherwise:
// *** 0x80 bit: 1 if published
// *** 0x40 bit: 1 if deprecated
// *** 0x20 bit: flag (may only be 1 for certain kinds, see below)
// *** remaining bits:
// **** 1: enum type
// ***** followed by:
// ****** UInt32 number N1 of members
// ****** N1 * tuple of:
// ******* Offset of Idx-Name
// ******* UInt32
// **** 2: plain struct type (with base if flag is 1)
// ***** followed by:
// ****** if "with base": Offset of Idx-Name
// ****** UInt32 number N1 of direct members
// ****** N1 * tuple of:
// ******* Offset of Idx-Name name
// ******* Offset of Idx-Name type
// **** 3: polymorphic struct type template
// ***** followed by:
// ****** UInt32 number N1 of type parameters
// ****** N1 * Offset of Idx-Name
// ****** UInt32 number N2 of members
// ****** N2 * tuple of:
// ******* kind byte: 0x01 bit is 1 if parameterized type
// ******* Offset of Idx-Name name
// ******* Offset of Idx-Name type
// **** 4: exception type (with base if flag is 1)
// ***** followed by:
// ****** if "with base": Offset of Idx-Name
// ****** UInt32 number N1 of direct members
// ****** N1 * tuple of:
// ******* Offset of Idx-Name name
// ******* Offset of Idx-Name type
// **** 5: interface type
// ***** followed by:
// ****** UInt32 number N1 of direct mandatory bases
// ****** N1 * Offset of Idx-Name
// ****** UInt32 number N2 of direct optional bases
// ****** N2 * Offset of Idx-Name
// ****** UInt32 number N3 of direct attributes
// ****** N3 * tuple of:
// ******* kind byte:
// ******** 0x02 bit: 1 if read-only
// ******** 0x01 bit: 1 if bound
// ******* Offset of Idx-Name name
// ******* Offset of Idx-Name type
// ******* UInt32 number N4 of get exceptions
// ******* N4 * Offset of Idx-Name
// ******* UInt32 number N5 of set exceptions
// ******* N5 * Offset of Idx-Name
// ****** UInt32 number N6 of direct methods
// ****** N6 * tuple of:
// ******* Offset of Idx-Name name
// ******* Offset of Idx-Name return type
// ******* UInt32 number N7 of parameters
// ******* N7 * tuple of:
// ******** direction byte: 0 for in, 1 for out, 2 for in-out
// ******** Offset of Idx-Name name
// ******** Offset of Idx-Name type
// ******* UInt32 number N8 of exceptions
// ******* N8 * Offset of Idx-Name
// **** 6: typedef
// ***** followed by:
// ****** Offset of Idx-Name
// **** 7: constant group
// ***** followed by:
// ****** UInt32 number N1 of entries of Map
// ****** N1 * Entry
// **** 8: single-interface--based service (with default constructor if flag is
//       1)
// ***** followed by:
// ****** Offset of Idx-Name
// ****** if not "with default constructor":
// ******* UInt32 number N1 of constructors
// ******* N1 * tuple of:
// ******** Offset of Idx-Name
// ******** UInt32 number N2 of parameters
// ******** N2 * tuple of
// ********* kind byte: 0x04 bit is 1 if rest parameter
// ********* Offset of Idx-Name name
// ********* Offset of Idx-Name type
// ******** UInt32 number N3 of exceptions
// ******** N3 * Offset of Idx-Name
// **** 9: accumulation-based service
// ***** followed by:
// ****** UInt32 number N1 of direct mandatory base services
// ****** N1 * Offset of Idx-Name
// ****** UInt32 number N2 of direct optional base services
// ****** N2 * Offset of Idx-Name
// ****** UInt32 number N3 of direct mandatory base interfaces
// ****** N3 * Offset of Idx-Name
// ****** UInt32 number N4 of direct optional base interfaces
// ****** N4 * Offset of Idx-Name
// ****** UInt32 number N5 of direct properties
// ****** N5 * tuple of:
// ******* UInt16 kind:
// ******** 0x0100 bit: 1 if optional
// ******** 0x0080 bit: 1 if removable
// ******** 0x0040 bit: 1 if maybedefault
// ******** 0x0020 bit: 1 if maybeambiguous
// ******** 0x0010 bit: 1 if readonly
// ******** 0x0008 bit: 1 if transient
// ******** 0x0004 bit: 1 if constrained
// ******** 0x0002 bit: 1 if bound
// ******** 0x0001 bit: 1 if maybevoid
// ******* Offset of Idx-Name name
// ******* Offset of Idx-Name type
// **** 10: interface-based singleton
// ***** followed by:
// ****** Offset of Idx-Name
// **** 11: service-based singleton
// ***** followed by:
// ****** Offset of Idx-Name
//
// Layout of per-entry payload in a constant group Map:
//
// * kind byte:
// ** 0x80 bit: 1 if deprecated
// ** remaining bits:
// *** 0: BOOLEAN
// **** followed by value byte, 0 represents false, 1 represents true
// *** 1: BYTE
// **** followed by value byte, representing values with two's complement
// *** 2: SHORT
// **** followed by UInt16 value, representing values with two's complement
// *** 3: UNSIGNED SHORT
// **** followed by UInt16 value
// *** 4: LONG
// **** followed by UInt32 value, representing values with two's complement
// *** 5: UNSIGNED LONG
// **** followed by UInt32 value
// *** 6: HYPER
// **** followed by UInt64 value, representing values with two's complement
// *** 7: UNSIGNED HYPER
// **** followed by UInt64 value
// *** 8: FLOAT
// **** followed by 4-byte value, representing values in ISO 60599 binary32
//       format, LSB first
// *** 9: DOUBLE
// **** followed by 8-byte value, representing values in ISO 60599 binary64
//       format, LSB first
//
// Memory layout:
//
// * 8 byte header "UNOIDL\0\xFF
// * Offset of root Map
// * UInt32 number of entries of root Map
// ...

namespace {

// sizeof (Memory16) == 2
struct Memory16 {
    unsigned char byte[2];

    sal_uInt16 getUnsigned16() const {
        return static_cast< sal_uInt16 >(byte[0])
            | (static_cast< sal_uInt16 >(byte[1]) << 8);
    }
};

// sizeof (Memory32) == 4
struct Memory32 {
    unsigned char byte[4];

    sal_uInt32 getUnsigned32() const {
        return static_cast< sal_uInt32 >(byte[0])
            | (static_cast< sal_uInt32 >(byte[1]) << 8)
            | (static_cast< sal_uInt32 >(byte[2]) << 16)
            | (static_cast< sal_uInt32 >(byte[3]) << 24);
    }

    float getIso60599Binary32() const {
        union {
            unsigned char buf[4];
            float f; // assuming float is ISO 60599 binary32
        } sa;
#if defined OSL_LITENDIAN
        sa.buf[0] = byte[0];
        sa.buf[1] = byte[1];
        sa.buf[2] = byte[2];
        sa.buf[3] = byte[3];
#else
        sa.buf[0] = byte[3];
        sa.buf[1] = byte[2];
        sa.buf[2] = byte[1];
        sa.buf[3] = byte[0];
#endif
        return sa.f;
    }
};

// sizeof (Memory64) == 8
struct Memory64 {
    unsigned char byte[8];

    sal_uInt64 getUnsigned64() const {
        return static_cast< sal_uInt64 >(byte[0])
            | (static_cast< sal_uInt64 >(byte[1]) << 8)
            | (static_cast< sal_uInt64 >(byte[2]) << 16)
            | (static_cast< sal_uInt64 >(byte[3]) << 24)
            | (static_cast< sal_uInt64 >(byte[4]) << 32)
            | (static_cast< sal_uInt64 >(byte[5]) << 40)
            | (static_cast< sal_uInt64 >(byte[6]) << 48)
            | (static_cast< sal_uInt64 >(byte[7]) << 56);
        }

    double getIso60599Binary64() const {
        union {
            unsigned char buf[8];
            double d; // assuming double is ISO 60599 binary64
        } sa;
#if defined OSL_LITENDIAN
        sa.buf[0] = byte[0];
        sa.buf[1] = byte[1];
        sa.buf[2] = byte[2];
        sa.buf[3] = byte[3];
        sa.buf[4] = byte[4];
        sa.buf[5] = byte[5];
        sa.buf[6] = byte[6];
        sa.buf[7] = byte[7];
#else
        sa.buf[0] = byte[7];
        sa.buf[1] = byte[6];
        sa.buf[2] = byte[5];
        sa.buf[3] = byte[4];
        sa.buf[4] = byte[3];
        sa.buf[5] = byte[2];
        sa.buf[6] = byte[1];
        sa.buf[7] = byte[0];
#endif
        return sa.d;
    }
};

}

namespace cppu {

class MappedFile: public salhelper::SimpleReferenceObject {
public:
    explicit MappedFile(rtl::OUString const & fileUrl);

    sal_uInt8 read8(sal_uInt32 offset) const;

    sal_uInt16 read16(sal_uInt32 offset) const;

    sal_uInt32 read32(sal_uInt32 offset) const;

    sal_uInt64 read64(sal_uInt32 offset) const;

    float readIso60599Binary32(sal_uInt32 offset) const;

    double readIso60599Binary64(sal_uInt32 offset) const;

    rtl::OUString readNameNul(sal_uInt32 offset) const;

    rtl::OUString readNameLen(sal_uInt32 offset, sal_uInt32 * newOffset = 0)
        const;

    oslFileHandle handle;
    sal_uInt64 size;
    void * address;

private:
    virtual ~MappedFile();

    sal_uInt8 get8(sal_uInt32 offset) const;

    sal_uInt16 get16(sal_uInt32 offset) const;

    sal_uInt32 get32(sal_uInt32 offset) const;

    sal_uInt64 get64(sal_uInt32 offset) const;

    float getIso60599Binary32(sal_uInt32 offset) const;

    double getIso60599Binary64(sal_uInt32 offset) const;
};

MappedFile::MappedFile(rtl::OUString const & fileUrl) {
    oslFileError e = osl_openFile(
        fileUrl.pData, &handle, osl_File_OpenFlag_Read);
    switch (e) {
    case osl_File_E_None:
        break;
    case osl_File_E_NOENT:
        throw css::container::NoSuchElementException(
            fileUrl, css::uno::Reference< css::uno::XInterface >());
    default:
        throw css::uno::RuntimeException(
            "cannot open " + fileUrl + ": " + rtl::OUString::number(e),
            css::uno::Reference< css::uno::XInterface >());
    }
    e = osl_getFileSize(handle, &size);
    if (e == osl_File_E_None) {
        e = osl_mapFile(
            handle, &address, size, 0, osl_File_MapFlag_RandomAccess);
    }
    if (e != osl_File_E_None) {
        oslFileError e2 = osl_closeFile(handle);
        SAL_WARN_IF(
            e2 != osl_File_E_None, "cppuhelper",
            "cannot close " << fileUrl << ": " << +e2);
        throw css::uno::RuntimeException(
            "cannot mmap " + fileUrl + ": " + rtl::OUString::number(e),
            css::uno::Reference< css::uno::XInterface >());
    }
}

sal_uInt8 MappedFile::read8(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 1) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: offset for 8-bit value too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    return get8(offset);
}

sal_uInt16 MappedFile::read16(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 2) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: offset for 16-bit value too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    return get16(offset);
}

sal_uInt32 MappedFile::read32(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 4) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: offset for 32-bit value too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    return get32(offset);
}

sal_uInt64 MappedFile::read64(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 8) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: offset for 64-bit value too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    return get64(offset);
}

float MappedFile::readIso60599Binary32(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 4) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: offset for 32-bit value too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    return getIso60599Binary32(offset);
}

double MappedFile::readIso60599Binary64(sal_uInt32 offset) const {
    assert(size >= 8);
    if (offset > size - 8) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: offset for 64-bit value too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    return getIso60599Binary64(offset);
}

rtl::OUString MappedFile::readNameNul(sal_uInt32 offset) const {
    if (offset > size) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: offset for string too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    sal_uInt64 end = offset;
    for (;; ++end) {
        if (end == size) {
            throw css::uno::DeploymentException(
                "broken UNOIDL file: string misses trailing NUL",
                css::uno::Reference< css::uno::XInterface >());
        }
        if (static_cast< char const * >(address)[end] == 0) {
            break;
        }
    }
    if (end - offset > SAL_MAX_INT32) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: string too long",
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name;
    if (!rtl_convertStringToUString(
            &name.pData, static_cast< char const * >(address) + offset,
            end - offset, RTL_TEXTENCODING_ASCII_US,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: name is not ASCII",
            css::uno::Reference< css::uno::XInterface >());
    }
    return name;
}

rtl::OUString MappedFile::readNameLen(sal_uInt32 offset, sal_uInt32 * newOffset)
    const
{
    sal_uInt32 len = read32(offset);
    if ((len & 0x80000000) == 0) {
        if (newOffset != 0) {
            *newOffset = offset + 4;
        }
        offset = len;
        len = read32(offset);
        if ((len & 0x80000000) == 0) {
            throw css::uno::DeploymentException(
                "broken UNOIDL file: name length high bit unset",
                css::uno::Reference< css::uno::XInterface >());
        }
        len &= ~0x80000000;
    } else {
        len &= ~0x80000000;
        if (newOffset != 0) {
            *newOffset = offset + 4 + len;
        }
    }
    if (len > SAL_MAX_INT32 || len > size - offset - 4) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: size of name is too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name;
    if (!rtl_convertStringToUString(
            &name.pData, static_cast< char const * >(address) + offset + 4, len,
            RTL_TEXTENCODING_ASCII_US,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: name is not ASCII",
            css::uno::Reference< css::uno::XInterface >());
    }
    return name;
}

MappedFile::~MappedFile() {
    oslFileError e = osl_unmapMappedFile(handle, address, size);
    SAL_WARN_IF(e != osl_File_E_None, "cppuhelper", "cannot unmap: " << +e);
    e = osl_closeFile(handle);
    SAL_WARN_IF(e != osl_File_E_None, "cppuhelper", "cannot close: " << +e);
}

sal_uInt8 MappedFile::get8(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 1);
    return static_cast< char const * >(address)[offset];
}

sal_uInt16 MappedFile::get16(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 2);
    return reinterpret_cast< Memory16 const * >(
        static_cast< char const * >(address) + offset)->getUnsigned16();
}

sal_uInt32 MappedFile::get32(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 4);
    return reinterpret_cast< Memory32 const * >(
        static_cast< char const * >(address) + offset)->getUnsigned32();
}

sal_uInt64 MappedFile::get64(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 8);
    return reinterpret_cast< Memory64 const * >(
        static_cast< char const * >(address) + offset)->getUnsigned64();
}

float MappedFile::getIso60599Binary32(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 4);
    return reinterpret_cast< Memory32 const * >(
        static_cast< char const * >(address) + offset)->getIso60599Binary32();
}

double MappedFile::getIso60599Binary64(sal_uInt32 offset) const {
    assert(size >= 8);
    assert(offset <= size - 8);
    return reinterpret_cast< Memory64 const * >(
        static_cast< char const * >(address) + offset)->getIso60599Binary64();
}

// sizeof (MapEntry) == 8
struct MapEntry {
    Memory32 name;
    Memory32 data;
};

namespace {

enum Compare { COMPARE_LESS, COMPARE_GREATER, COMPARE_EQUAL };

Compare compare(
    rtl::Reference< MappedFile > const & file, rtl::OUString const & name,
    sal_Int32 nameOffset, sal_Int32 nameLength, MapEntry const * entry)
{
    assert(file.is());
    assert(entry != 0);
    sal_uInt32 off = entry->name.getUnsigned32();
    if (off > file->size - 1) { // at least a trailing NUL
        throw css::uno::DeploymentException(
            "broken UNOIDL file: string offset too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    assert(nameLength >= 0);
    sal_uInt64 min = std::min(
        static_cast< sal_uInt64 >(nameLength), file->size - off);
    for (sal_uInt64 i = 0; i != min; ++i) {
        sal_Unicode c1 = name[nameOffset + i];
        sal_Unicode c2 = static_cast< unsigned char const * >(file->address)[
            off + i];
        if (c1 < c2) {
            return COMPARE_LESS;
        } else if (c1 > c2 || c2 == 0) {
            // ...the "|| c2 == 0" is for the odd case where name erroneously
            // contains NUL characters
            return COMPARE_GREATER;
        }
    }
    if (static_cast< sal_uInt64 >(nameLength) == min) {
        if (file->size - off == min) {
            throw css::uno::DeploymentException(
                "broken UNOIDL file: string misses trailing NUL",
                css::uno::Reference< css::uno::XInterface >());
        }
        return
            static_cast< unsigned char const * >(file->address)[off + min] == 0
            ? COMPARE_EQUAL : COMPARE_LESS;
    } else {
        return COMPARE_GREATER;
    }
}

sal_uInt32 findInMap(
    rtl::Reference< MappedFile > const & file, MapEntry const * mapBegin,
    sal_uInt32 mapSize, rtl::OUString const & name, sal_Int32 nameOffset,
    sal_Int32 nameLength)
{
    if (mapSize == 0) {
        return 0;
    }
    sal_uInt32 n = mapSize / 2;
    MapEntry const * p = mapBegin + n;
    switch (compare(file, name, nameOffset, nameLength, p)) {
    case COMPARE_LESS:
        return findInMap(file, mapBegin, n, name, nameOffset, nameLength);
    case COMPARE_GREATER:
        return findInMap(
            file, p + 1, mapSize - n - 1, name, nameOffset, nameLength);
    default: // COMPARE_EQUAL
        break;
    }
    sal_uInt32 off = mapBegin[n].data.getUnsigned32();
    if (off == 0) {
        throw css::uno::DeploymentException(
            "broken UNOIDL file: map entry data offset is null",
            css::uno::Reference< css::uno::XInterface >());
    }
    return off;
}

css::uno::Any readConstant(
    rtl::Reference< MappedFile > const & file, sal_uInt32 offset)
{
    assert(file.is());
    int v = file->read8(offset);
    int type = v & 0x7F;
    bool deprecated = (v & 0x80) != 0; (void)deprecated;//TODO
    switch (type) {
    case 0: // BOOLEAN
        v = file->read8(offset + 1);
        switch (v) {
        case 0:
            return css::uno::makeAny(false);
        case 1:
            return css::uno::makeAny(true);
        default:
            throw css::uno::DeploymentException(
                ("broken UNOIDL file: bad boolean constant value "
                 + rtl::OUString::number(v)),
                css::uno::Reference< css::uno::XInterface >());
        }
        break;
    case 1: // BYTE
        return css::uno::makeAny< sal_Int8 >(file->read8(offset + 1));
            //TODO: implementation-defined behavior of conversion from sal_uInt8
            // to sal_Int8 relies on two's complement representation
    case 2: // SHORT
        return css::uno::makeAny< sal_Int16 >(file->read16(offset + 1));
            //TODO: implementation-defined behavior of conversion from
            // sal_uInt16 to sal_Int16 relies on two's complement representation
    case 3: // UNSIGNED SHORT
        return css::uno::makeAny(file->read16(offset + 1));
    case 4: // LONG
        return css::uno::makeAny< sal_Int32 >(file->read32(offset + 1));
            //TODO: implementation-defined behavior of conversion from
            // sal_uInt32 to sal_Int32 relies on two's complement representation
    case 5: // UNSIGNED LONG
        return css::uno::makeAny(file->read32(offset + 1));
    case 6: // HYPER
        return css::uno::makeAny< sal_Int64 >(file->read64(offset + 1));
            //TODO: implementation-defined behavior of conversion from
            // sal_uInt64 to sal_Int64 relies on two's complement representation
    case 7: // UNSIGNED HYPER
        return css::uno::makeAny(file->read64(offset + 1));
    case 8: // FLOAT
        return css::uno::makeAny(file->readIso60599Binary32(offset + 1));
    case 9: // DOUBLE
        return css::uno::makeAny(file->readIso60599Binary64(offset + 1));
    default:
        throw css::uno::DeploymentException(
            ("broken UNOIDL file: bad constant type byte "
             + rtl::OUString::number(v)),
            css::uno::Reference< css::uno::XInterface >());
    }
}

rtl::Reference< unoidl::Entity > readEntity(
    rtl::Reference< MappedFile > const & file, sal_uInt32 offset);

class UnoidlCursor: public unoidl::MapCursor {
public:
    UnoidlCursor(
        rtl::Reference< MappedFile > file, MapEntry const * mapBegin,
        sal_uInt32 mapSize):
        file_(file), mapIndex_(mapBegin), mapEnd_(mapBegin + mapSize)
    {}

private:
    virtual ~UnoidlCursor() throw () {}

    virtual rtl::Reference< unoidl::Entity > getNext(rtl::OUString * name);

    rtl::Reference< MappedFile > file_;
    MapEntry const * mapIndex_;
    MapEntry const * mapEnd_;
};

rtl::Reference< unoidl::Entity > UnoidlCursor::getNext(rtl::OUString * name) {
    assert(name != 0);
    rtl::Reference< unoidl::Entity > ent;
    if (mapIndex_ != mapEnd_) {
        *name = file_->readNameNul(mapIndex_->name.getUnsigned32());
        ent = readEntity(file_, mapIndex_->data.getUnsigned32());
        ++mapIndex_;
    }
    return ent;
}

class UnoidlModuleEntity: public unoidl::ModuleEntity {
public:
    UnoidlModuleEntity(
        rtl::Reference< MappedFile > const & file, sal_uInt32 mapOffset,
        sal_uInt32 mapSize):
        file_(file),
        mapBegin_(
            reinterpret_cast< MapEntry const * >(
                static_cast< char const * >(file_->address) + mapOffset)),
        mapSize_(mapSize)
    { assert(file.is()); }

private:
    virtual ~UnoidlModuleEntity() throw () {}

    virtual std::vector< rtl::OUString > getMemberNames() const;

    virtual rtl::Reference< unoidl::MapCursor > createCursor() const
    { return new UnoidlCursor(file_, mapBegin_, mapSize_); }

    rtl::Reference< MappedFile > file_;
    MapEntry const * mapBegin_;
    sal_uInt32 mapSize_;
};

std::vector< rtl::OUString > UnoidlModuleEntity::getMemberNames() const {
    std::vector< rtl::OUString > names;
    for (sal_uInt32 i = 0; i != mapSize_; ++i) {
        names.push_back(file_->readNameNul(mapBegin_[i].name.getUnsigned32()));
    }
    return names;
}

rtl::Reference< unoidl::Entity > readEntity(
    rtl::Reference< MappedFile > const & file, sal_uInt32 offset)
{
    assert(file.is());
    int v = file->read8(offset);
    int type = v & 0x3F;
    bool published = (v & 0x80) != 0;
    bool deprecated = (v & 0x40) != 0; (void)deprecated;//TODO
    bool flag = (v & 0x20) != 0;
    switch (type) {
    case 0: // module
        {
            if (v != 0) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: bad module type byte "
                     + rtl::OUString::number(v)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    "broken UNOIDL file: too many items in module",
                    css::uno::Reference< css::uno::XInterface >());
            }
            if (offset + 5 + 8 * n > file->size) { //TODO: overflow
                throw css::uno::DeploymentException(
                    "broken UNOIDL file: module map offset + size too large",
                    css::uno::Reference< css::uno::XInterface >());
            }
            return new UnoidlModuleEntity(file, offset + 5, n);
        }
    case 1: // enum type
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    "broken UNOIDL file: too many members of enum type",
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 5;
            std::vector< unoidl::EnumTypeEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                rtl::OUString memName(file->readNameLen(offset, &offset));
                sal_Int32 memValue = static_cast< sal_Int32 >(
                    file->read32(offset));
                    //TODO: implementation-defined behavior of conversion from
                    // sal_uInt32 to sal_Int32 relies on two's complement
                    // representation
                offset += 4;
                mems.push_back(
                    unoidl::EnumTypeEntity::Member(memName, memValue));
            }
            return new unoidl::EnumTypeEntity(published, mems);
        }
    case 2: // plain struct type without base
    case 2 | 0x20: // plain struct type with base
        {
            ++offset;
            rtl::OUString base;
            if (flag) {
                base = file->readNameLen(offset, &offset);
                if (base.isEmpty()) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: empty base type name of plain"
                         " struct type"),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
            sal_uInt32 n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct members of plain"
                     " struct type"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< unoidl::PlainStructTypeEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                rtl::OUString memName(file->readNameLen(offset, &offset));
                rtl::OUString memType(file->readNameLen(offset, &offset));
                mems.push_back(
                    unoidl::PlainStructTypeEntity::Member(memName, memType));
            }
            return new unoidl::PlainStructTypeEntity(published, base, mems);
        }
    case 3: // polymorphic struct type template
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many type parameters of"
                     " polymorphic struct type template"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 5;
            std::vector< rtl::OUString > params;
            for (sal_uInt32 i = 0; i != n; ++i) {
                params.push_back(file->readNameLen(offset, &offset));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many members of polymorphic"
                     " struct type template"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< unoidl::PolymorphicStructTypeTemplateEntity::Member >
                mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                v = file->read8(offset);
                ++offset;
                rtl::OUString memName(file->readNameLen(offset, &offset));
                rtl::OUString memType(file->readNameLen(offset, &offset));
                if (v > 1) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: bad flags "
                         + rtl::OUString::number(v) + " for member " + memName
                         + " of polymorphic struct type template"),
                        css::uno::Reference< css::uno::XInterface >());
                }
                mems.push_back(
                    unoidl::PolymorphicStructTypeTemplateEntity::Member(
                        memName, memType, v == 1));
            }
            return new unoidl::PolymorphicStructTypeTemplateEntity(
                published, params, mems);
        }
    case 4: // exception type without base
    case 4 | 0x20: // exception type with base
        {
            ++offset;
            rtl::OUString base;
            if (flag) {
                base = file->readNameLen(offset, &offset);
                if (base.isEmpty()) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: empty base type name of"
                         " exception type"),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
            sal_uInt32 n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct members of"
                     " exception type"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< unoidl::ExceptionTypeEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                rtl::OUString memName(file->readNameLen(offset, &offset));
                rtl::OUString memType(file->readNameLen(offset, &offset));
                mems.push_back(
                    unoidl::ExceptionTypeEntity::Member(memName, memType));
            }
            return new unoidl::ExceptionTypeEntity(published, base, mems);
        }
    case 5: // interface type
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct mandatory bases of"
                     " interface type"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 5;
            std::vector< rtl::OUString > mandBases;
            for (sal_uInt32 i = 0; i != n; ++i) {
                mandBases.push_back(file->readNameLen(offset, &offset));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct optional bases of"
                     " interface type"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< rtl::OUString > optBases;
            for (sal_uInt32 i = 0; i != n; ++i) {
                optBases.push_back(file->readNameLen(offset, &offset));
            }
            sal_uInt32 nAttrs = file->read32(offset);
            if (nAttrs > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct attributes of"
                     " interface type"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< unoidl::InterfaceTypeEntity::Attribute > attrs;
            for (sal_uInt32 i = 0; i != nAttrs; ++i) {
                v = file->read8(offset);
                ++offset;
                rtl::OUString attrName(file->readNameLen(offset, &offset));
                rtl::OUString attrType(file->readNameLen(offset, &offset));
                if (v > 0x03) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: bad flags for direct attribute "
                         + attrName + " of interface type"),
                        css::uno::Reference< css::uno::XInterface >());
                }
                std::vector< rtl::OUString > getExcs;
                sal_uInt32 m = file->read32(offset);
                if (m > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many getter exceptions for"
                         " direct attribute " + attrName
                         + " of interface type"),
                        css::uno::Reference< css::uno::XInterface >());
                }
                offset += 4;
                for (sal_uInt32 j = 0; j != m; ++j) {
                    getExcs.push_back(file->readNameLen(offset, &offset));
                }
                std::vector< rtl::OUString > setExcs;
                if ((v & 0x02) == 0) {
                    m = file->read32(offset);
                    if (m > SAL_MAX_INT32) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: too many setter exceptions"
                             " for direct attribute " + attrName
                             + " of interface type"),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    offset += 4;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        setExcs.push_back(file->readNameLen(offset, &offset));
                    }
                }
                attrs.push_back(
                    unoidl::InterfaceTypeEntity::Attribute(
                        attrName, attrType, (v & 0x01) != 0, (v & 0x02) != 0,
                        getExcs, setExcs));
            }
            sal_uInt32 nMeths = file->read32(offset);
            if (nMeths > SAL_MAX_INT32 - nAttrs) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct attributes and"
                     " methods of interface type"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< unoidl::InterfaceTypeEntity::Method > meths;
            for (sal_uInt32 i = 0; i != nMeths; ++i) {
                rtl::OUString methName(file->readNameLen(offset, &offset));
                rtl::OUString methType(file->readNameLen(offset, &offset));
                sal_uInt32 m = file->read32(offset);
                if (m > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many parameters for method "
                         + methName + " of interface type"),
                        css::uno::Reference< css::uno::XInterface >());
                }
                offset += 4;
                std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >
                    params;
                for (sal_uInt32 j = 0; j != m; ++j) {
                    v = file->read8(offset);
                    ++offset;
                    rtl::OUString paramName(
                        file->readNameLen(offset, &offset));
                    rtl::OUString paramType(
                        file->readNameLen(offset, &offset));
                    unoidl::InterfaceTypeEntity::Method::Parameter::Direction
                        dir;
                    switch (v) {
                    case 0:
                        dir = unoidl::InterfaceTypeEntity::Method::Parameter::
                            DIRECTION_IN;
                        break;
                    case 1:
                        dir = unoidl::InterfaceTypeEntity::Method::Parameter::
                            DIRECTION_OUT;
                        break;
                    case 2:
                        dir = unoidl::InterfaceTypeEntity::Method::Parameter::
                            DIRECTION_IN_OUT;
                        break;
                    default:
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: bad direction "
                             + rtl::OUString::number(v) + " of parameter "
                             + paramName + " for method " + methName
                             + " of interface type"),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    params.push_back(
                        unoidl::InterfaceTypeEntity::Method::Parameter(
                            paramName, paramType, dir));
                }
                std::vector< rtl::OUString > excs;
                m = file->read32(offset);
                if (m > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many exceptions for method "
                         + methName + " of interface type"),
                        css::uno::Reference< css::uno::XInterface >());
                }
                offset += 4;
                for (sal_uInt32 j = 0; j != m; ++j) {
                    excs.push_back(file->readNameLen(offset, &offset));
                }
                meths.push_back(
                    unoidl::InterfaceTypeEntity::Method(
                        methName, methType, params, excs));
            }
            return new unoidl::InterfaceTypeEntity(
                published, mandBases, optBases, attrs, meths);
        }
    case 6: // typedef
        return new unoidl::TypedefEntity(
            published, file->readNameLen(offset + 1));
    case 7: // constant group
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    "broken UNOIDL file: too many constants in constant group",
                    css::uno::Reference< css::uno::XInterface >());
            }
            if (offset + 5 + 8 * n > file->size) { //TODO: overflow
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: constant group map offset + size too"
                     " large"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            MapEntry const * p = reinterpret_cast< MapEntry const * >(
                static_cast< char const * >(file->address) + offset + 5);
            std::vector< unoidl::ConstantGroupEntity::Member > mems;
            for (sal_uInt32 i = 0; i != n; ++i) {
                mems.push_back(
                    unoidl::ConstantGroupEntity::Member(
                        file->readNameNul(p[i].name.getUnsigned32()),
                        readConstant(file, p[i].data.getUnsigned32())));
            }
            return new unoidl::ConstantGroupEntity(published, mems);
        }
    case 8: // single-interface--based service without default constructor
    case 8 | 0x20: // single-interface--based service with default constructor
        {
            rtl::OUString base(file->readNameLen(offset + 1, &offset));
            std::vector<
                unoidl::SingleInterfaceBasedServiceEntity::Constructor >
                ctors;
            if (flag) {
                ctors.push_back(
                    unoidl::SingleInterfaceBasedServiceEntity::Constructor());
            } else {
                sal_uInt32 n = file->read32(offset);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many constructors of"
                         " single-interface--based service"),
                        css::uno::Reference< css::uno::XInterface >());
                }
                offset += 4;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    rtl::OUString ctorName(file->readNameLen(offset, &offset));
                    sal_uInt32 m = file->read32(offset);
                    if (m > SAL_MAX_INT32) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: too many parameters for"
                             " constructor " + ctorName
                             + " of single-interface--based service"),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    offset += 4;
                    std::vector<
                        unoidl::SingleInterfaceBasedServiceEntity::Constructor::
                        Parameter > params;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        v = file->read8(offset);
                        ++offset;
                        rtl::OUString paramName(
                            file->readNameLen(offset, &offset));
                        rtl::OUString paramType(
                            file->readNameLen(offset, &offset));
                        bool rest;
                        switch (v) {
                        case 0:
                            rest = false;
                            break;
                        case 0x04:
                            rest = true;
                            break;
                        default:
                            throw css::uno::DeploymentException(
                                ("broken UNOIDL file: bad mode "
                                 + rtl::OUString::number(v) + " of parameter "
                                 + paramName + " for constructor " + ctorName
                                 + " of single-interface--based service"),
                                css::uno::Reference< css::uno::XInterface >());
                        }
                        params.push_back(
                            unoidl::SingleInterfaceBasedServiceEntity::
                            Constructor::Parameter(
                                paramName, paramType, rest));
                    }
                    std::vector< rtl::OUString > excs;
                    m = file->read32(offset);
                    if (m > SAL_MAX_INT32) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: too many exceptions for"
                             " constructor " + ctorName
                             + " of single-interface--based service"),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    offset += 4;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        excs.push_back(file->readNameLen(offset, &offset));
                    }
                    ctors.push_back(
                        unoidl::SingleInterfaceBasedServiceEntity::Constructor(
                            ctorName, params, excs));
                }
            }
            return new unoidl::SingleInterfaceBasedServiceEntity(
                published, base, ctors);
        }
    case 9: // accumulation-based service
        {
            sal_uInt32 n = file->read32(offset + 1);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct mandatory service"
                     " bases of accumulation-based service"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 5;
            std::vector< rtl::OUString > mandServs;
            for (sal_uInt32 i = 0; i != n; ++i) {
                mandServs.push_back(file->readNameLen(offset, &offset));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct optional service"
                     " bases of accumulation-based servcie"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< rtl::OUString > optServs;
            for (sal_uInt32 i = 0; i != n; ++i) {
                optServs.push_back(file->readNameLen(offset, &offset));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct mandatory interface"
                     " bases of accumulation-based servcie"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< rtl::OUString > mandIfcs;
            for (sal_uInt32 i = 0; i != n; ++i) {
                mandIfcs.push_back(file->readNameLen(offset, &offset));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct optional interface"
                     " bases of accumulation-based servcie"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< rtl::OUString > optIfcs;
            for (sal_uInt32 i = 0; i != n; ++i) {
                optIfcs.push_back(file->readNameLen(offset, &offset));
            }
            n = file->read32(offset);
            if (n > SAL_MAX_INT32) {
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: too many direct properties of"
                     " accumulation-based servcie"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            offset += 4;
            std::vector< unoidl::AccumulationBasedServiceEntity::Property >
                props;
            for (sal_uInt32 i = 0; i != n; ++i) {
                sal_uInt16 attrs = file->read16(offset);
                offset += 2;
                rtl::OUString propName(file->readNameLen(offset, &offset));
                rtl::OUString propType(file->readNameLen(offset, &offset));
                if (attrs > 0x01FF) { // see css.beans.PropertyAttribute
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: bad mode "
                         + rtl::OUString::number(v) + " of property " + propName
                         + " for accumulation-based servcie"),
                        css::uno::Reference< css::uno::XInterface >());
                }
                props.push_back(
                    unoidl::AccumulationBasedServiceEntity::Property(
                        propName, propType,
                        static_cast<
                            unoidl::AccumulationBasedServiceEntity::Property::
                            Attributes >(
                                attrs)));
            }
            return new unoidl::AccumulationBasedServiceEntity(
                published, mandServs, optServs, mandIfcs, optIfcs, props);
        }
    case 10: // interface-based singleton
        return new unoidl::InterfaceBasedSingletonEntity(
            published, file->readNameLen(offset + 1));
    case 11: // service-based singleton
        return new unoidl::ServiceBasedSingletonEntity(
            published, file->readNameLen(offset + 1));
    default:
        throw css::uno::DeploymentException(
            "broken UNOIDL file: bad type byte " + rtl::OUString::number(v),
            css::uno::Reference< css::uno::XInterface >());
    }
}

}

UnoidlProvider::UnoidlProvider(rtl::OUString const & uri):
    file_(new MappedFile(uri))
{
    if (file_->size < 8 || std::memcmp(file_->address, "UNOIDL\0\xFF", 8) != 0)
    {
        throw css::registry::InvalidRegistryException(
            uri, css::uno::Reference< css::uno::XInterface >());
    }
    sal_uInt32 off = file_->read32(8);
    mapSize_ = file_->read32(12);
    if (off + 8 * mapSize_ > file_->size) { //TODO: overflow
        throw css::uno::DeploymentException(
            "broken UNOIDL file: root map offset + size too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    mapBegin_ = reinterpret_cast< MapEntry const * >(
        static_cast< char const * >(file_->address) + off);
}

rtl::Reference< unoidl::MapCursor > UnoidlProvider::createRootCursor() const {
    return new UnoidlCursor(file_, mapBegin_, mapSize_);
}

sal_uInt32 UnoidlProvider::find(rtl::OUString const & name, bool * constant)
    const
{
    MapEntry const * mapBegin = mapBegin_;
    sal_uInt32 mapSize = mapSize_;
    bool cgroup = false;
    for (sal_Int32 i = 0;;) {
        sal_Int32 j = name.indexOf('.', i);
        if (j == -1) {
            j = name.getLength();
        }
        sal_Int32 off = findInMap(file_, mapBegin, mapSize, name, i, j - i);
        if (off == 0) {
            return 0;
        }
        if (j == name.getLength()) {
            if (constant != 0) {
                *constant = cgroup;
            }
            return off;
        }
        if (cgroup) {
            return 0;
                //TODO: throw an exception instead here, where the segments of a
                // constant's name are a prefix of the requested name's
                // segments?
        }
        int v = file_->read8(off);
        if (v != 0) { // module
            if ((v & 0x3F) == 7) { // constant group
                cgroup = true;
            } else {
                return 0;
                    //TODO: throw an exception instead here, where the segments
                    // of a non-module, non-constant-group entity's name are a
                    // prefix of the requested name's segments?
            }
        }
        mapSize = file_->read32(off + 1);
        if (8 * mapSize > file_->size - off - 5) { //TODO: overflow
            throw css::uno::DeploymentException(
                "broken UNOIDL file: map offset + size too large",
                css::uno::Reference< css::uno::XInterface >());
        }
        mapBegin = reinterpret_cast< MapEntry const * >(
            static_cast< char const * >(file_->address) + off + 5);
        i = j + 1;
    }
}

rtl::Reference< unoidl::Entity > UnoidlProvider::getEntity(sal_uInt32 offset)
    const
{
    return readEntity(file_, offset);
}

css::uno::Any UnoidlProvider::getConstant(sal_uInt32 offset) const {
    return readConstant(file_, offset);
}

UnoidlProvider::~UnoidlProvider() throw () {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
