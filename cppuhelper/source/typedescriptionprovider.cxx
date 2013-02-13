/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cassert>
#include <cstring>
#include <set>
#include <stack>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/reflection/InvalidTypeNameException.hpp"
#include "com/sun/star/reflection/NoSuchTypeNameException.hpp"
#include "com/sun/star/reflection/TypeDescriptionSearchDepth.hpp"
#include "com/sun/star/reflection/XConstantTypeDescription.hpp"
#include "com/sun/star/reflection/XConstantsTypeDescription.hpp"
#include "com/sun/star/reflection/XEnumTypeDescription.hpp"
#include "com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceTypeDescription2.hpp"
#include "com/sun/star/reflection/XModuleTypeDescription.hpp"
#include "com/sun/star/reflection/XPublished.hpp"
#include "com/sun/star/reflection/XServiceTypeDescription2.hpp"
#include "com/sun/star/reflection/XSingletonTypeDescription2.hpp"
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include "com/sun/star/reflection/XTypeDescriptionEnumeration.hpp"
#include "com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/implbase1.hxx"
#include "osl/endian.h"
#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"

#include "paths.hxx"
#include "typedescriptionprovider.hxx"

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

struct MappedFile:
    public salhelper::SimpleReferenceObject, private boost::noncopyable
{
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

css::uno::Reference< css::reflection::XTypeDescription > resolve(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    rtl::OUString const & name)
{
    assert(context.is());
    try {
        return css::uno::Reference< css::reflection::XTypeDescription >(
            (css::uno::Reference< css::container::XHierarchicalNameAccess >(
                context->getValueByName(
                    "/singletons/"
                    "com.sun.star.reflection.theTypeDescriptionManager"),
                css::uno::UNO_QUERY_THROW)->
             getByHierarchicalName(name)),
            css::uno::UNO_QUERY_THROW);
    } catch (css::container::NoSuchElementException & e) {
        throw css::uno::DeploymentException(
            ("cannot resolve type \"" + name + "\"; NoSuchElementException: "
             + e.Message),
            e.Context);
    }
}

class PublishableDescription:
    public cppu::WeakImplHelper1< css::reflection::XPublished >,
    private boost::noncopyable
{
protected:
    PublishableDescription(bool published): published_(published) {}

    virtual ~PublishableDescription() {}

private:
    virtual sal_Bool SAL_CALL isPublished() throw (css::uno::RuntimeException)
    { return published_; }

    bool published_;
};

class ModuleDescription:
    public cppu::WeakImplHelper1< css::reflection::XModuleTypeDescription >,
    private boost::noncopyable
{
public:
    ModuleDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name,
        std::vector< rtl::OUString > const & items):
        context_(context), name_(name), items_(items)
    {}

private:
    virtual ~ModuleDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_MODULE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMembers() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    std::vector< rtl::OUString > items_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
ModuleDescription::getMembers() throw (css::uno::RuntimeException) {
    assert(items_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(items_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = resolve(context_, name_ + "." + items_[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XEnumTypeDescription >
EnumTypeDescription_Base;

class EnumTypeDescription: public EnumTypeDescription_Base {
public:
    struct Member {
        Member(rtl::OUString const & theName, sal_Int32 theValue):
            name(theName), value(theValue)
        {}

        rtl::OUString name;
        sal_Int32 value;
    };

    EnumTypeDescription(
        rtl::OUString const & name, bool published,
        std::vector< Member > const & members):
        EnumTypeDescription_Base(published), name_(name), members_(members)
    { assert(!members_.empty()); }

private:
    virtual ~EnumTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_ENUM; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual sal_Int32 SAL_CALL getDefaultEnumValue()
        throw (css::uno::RuntimeException)
    { return members_[0].value; }

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getEnumNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getEnumValues()
        throw (css::uno::RuntimeException);

    rtl::OUString name_;
    std::vector< Member > members_;
};

css::uno::Sequence< rtl::OUString > EnumTypeDescription::getEnumNames()
    throw (css::uno::RuntimeException)
{
    assert(members_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(members_.size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = members_[i].name;
    }
    return s;
}

css::uno::Sequence< sal_Int32 > EnumTypeDescription::getEnumValues()
    throw (css::uno::RuntimeException)
{
    assert(members_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(members_.size());
    css::uno::Sequence< sal_Int32 > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = members_[i].value;
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XStructTypeDescription >
PlainStructTypeDescription_Base;

class PlainStructTypeDescription: public PlainStructTypeDescription_Base {
public:
    struct Member {
        Member(rtl::OUString const & theName, rtl::OUString const & theType):
            name(theName), type(theType)
        {}

        rtl::OUString name;
        rtl::OUString type;
    };

    PlainStructTypeDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published, rtl::OUString const & base,
        std::vector< Member > const & directMembers):
        PlainStructTypeDescription_Base(published), context_(context),
        name_(name), base_(base), directMembers_(directMembers)
    {}

private:
    virtual ~PlainStructTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_STRUCT; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException) {
        return base_.isEmpty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : resolve(context_, base_);
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getMemberNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getTypeParameters()
        throw (css::uno::RuntimeException)
    { return css::uno::Sequence< rtl::OUString >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() throw (css::uno::RuntimeException) {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XTypeDescription > >();
    }

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    rtl::OUString base_;
    std::vector< Member > directMembers_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
PlainStructTypeDescription::getMemberTypes() throw (css::uno::RuntimeException)
{
    assert(directMembers_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(directMembers_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = resolve(context_, directMembers_[i].type);
    }
    return s;
}

css::uno::Sequence< rtl::OUString > PlainStructTypeDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    assert(directMembers_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(directMembers_.size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = directMembers_[i].name;
    }
    return s;
}

class ParameterizedMemberTypeDescription:
    public cppu::WeakImplHelper1< css::reflection::XTypeDescription >,
    private boost::noncopyable
{
public:
    explicit ParameterizedMemberTypeDescription(
        rtl::OUString const & typeParameterName):
        typeParameterName_(typeParameterName)
    {}

private:
    virtual ~ParameterizedMemberTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_UNKNOWN; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return typeParameterName_; }

    rtl::OUString typeParameterName_;
};

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XStructTypeDescription >
PolymorphicStructTypeTemplateDescription_Base;

class PolymorphicStructTypeTemplateDescription:
    public PolymorphicStructTypeTemplateDescription_Base
{
public:
    struct Member {
        Member(
            rtl::OUString const & theName, rtl::OUString const & theType,
            bool theParameterized):
            name(theName), type(theType), parameterized(theParameterized)
        {}

        rtl::OUString name;
        rtl::OUString type;
        bool parameterized;
    };

    PolymorphicStructTypeTemplateDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published,
        std::vector< rtl::OUString > const & typeParameters,
        std::vector< Member > const & members):
        PolymorphicStructTypeTemplateDescription_Base(published),
        context_(context), name_(name), typeParameters_(typeParameters),
        members_(members)
    {}

private:
    virtual ~PolymorphicStructTypeTemplateDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_STRUCT; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getMemberNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getTypeParameters()
        throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getTypeArguments() throw (css::uno::RuntimeException) {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XTypeDescription > >();
    }

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    std::vector< rtl::OUString > typeParameters_;
    std::vector< Member > members_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
PolymorphicStructTypeTemplateDescription::getMemberTypes()
    throw (css::uno::RuntimeException)
{
    assert(members_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(members_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = members_[i].parameterized
            ? new ParameterizedMemberTypeDescription(members_[i].type)
            : resolve(context_, members_[i].type);
    }
    return s;
}

css::uno::Sequence< rtl::OUString >
PolymorphicStructTypeTemplateDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    assert(members_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(members_.size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = members_[i].name;
    }
    return s;
}

css::uno::Sequence< rtl::OUString >
PolymorphicStructTypeTemplateDescription::getTypeParameters()
    throw (css::uno::RuntimeException)
{
    assert(typeParameters_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(typeParameters_.size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = typeParameters_[i];
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XCompoundTypeDescription >
ExceptionTypeDescription_Base;

class ExceptionTypeDescription: public ExceptionTypeDescription_Base {
public:
    struct Member {
        Member(rtl::OUString const & theName, rtl::OUString const & theType):
            name(theName), type(theType)
        {}

        rtl::OUString name;
        rtl::OUString type;
    };

    ExceptionTypeDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published, rtl::OUString const & base,
        std::vector< Member > const & directMembers):
        ExceptionTypeDescription_Base(published), context_(context),
        name_(name), base_(base), directMembers_(directMembers)
    {}

private:
    virtual ~ExceptionTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_EXCEPTION; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException) {
        return base_.isEmpty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : resolve(context_, base_);
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getMemberTypes() throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getMemberNames()
        throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    rtl::OUString base_;
    std::vector< Member > directMembers_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
ExceptionTypeDescription::getMemberTypes() throw (css::uno::RuntimeException) {
    assert(directMembers_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(directMembers_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = resolve(context_, directMembers_[i].type);
    }
    return s;
}

css::uno::Sequence< rtl::OUString > ExceptionTypeDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    assert(directMembers_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(directMembers_.size());
    css::uno::Sequence< rtl::OUString > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = directMembers_[i].name;
    }
    return s;
}

css::uno::Reference< css::reflection::XTypeDescription > resolveTypedefs(
    css::uno::Reference< css::reflection::XTypeDescription > const & type)
{
    css::uno::Reference< css::reflection::XTypeDescription > resolved(type);
    while (resolved->getTypeClass() == css::uno::TypeClass_TYPEDEF) {
        resolved
            = (css::uno::Reference< css::reflection::XIndirectTypeDescription >(
                   resolved, css::uno::UNO_QUERY_THROW)->
               getReferencedType());
    }
    return resolved;
}

class BaseOffset: private boost::noncopyable {
public:
    BaseOffset(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    sal_Int32 get() const { return offset_; }

private:
    void calculateBases(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    void calculate(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
            const & description);

    std::set< rtl::OUString > set_;
    sal_Int32 offset_;
};

BaseOffset::BaseOffset(
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > const &
        description):
    offset_(0)
{
    calculateBases(description);
}

void BaseOffset::calculateBases(
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > const &
        description)
{
    css::uno::Sequence<
        css::uno::Reference < css::reflection::XTypeDescription > > bases(
            description->getBaseTypes());
    for (sal_Int32 i = 0; i != bases.getLength(); ++i) {
        calculate(
            css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >(
                resolveTypedefs(bases[i]), css::uno::UNO_QUERY_THROW));
    }
}

void BaseOffset::calculate(
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > const &
        description)
{
    if (set_.insert(description->getName()).second) {
        calculateBases(description);
        offset_ += description->getMembers().getLength();
    }
}

struct Attribute {
    Attribute(
        rtl::OUString const & theName, rtl::OUString const & theType,
        bool theBound, bool theReadOnly,
        std::vector< rtl::OUString > const & theGetExceptions,
        std::vector< rtl::OUString > const & theSetExceptions):
        name(theName), type(theType), bound(theBound), readOnly(theReadOnly),
        getExceptions(theGetExceptions), setExceptions(theSetExceptions)
    {}

    rtl::OUString name;
    rtl::OUString type;
    bool bound;
    bool readOnly;
    std::vector< rtl::OUString > getExceptions;
    std::vector< rtl::OUString > setExceptions;
};

class AttributeDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XInterfaceAttributeTypeDescription2 >,
    private boost::noncopyable
{
public:
    AttributeDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, Attribute attribute, sal_Int32 position):
        context_(context), name_(name), attribute_(attribute),
        position_(position)
    {}

private:
    virtual ~AttributeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_INTERFACE_ATTRIBUTE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual rtl::OUString SAL_CALL getMemberName()
        throw (css::uno::RuntimeException)
    { return attribute_.name; }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    virtual sal_Bool SAL_CALL isReadOnly() throw (css::uno::RuntimeException)
    { return attribute_.readOnly; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException)
    { return resolve(context_, attribute_.type); }

    virtual sal_Bool SAL_CALL isBound() throw (css::uno::RuntimeException)
    { return attribute_.bound; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getGetExceptions() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getSetExceptions() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    Attribute attribute_;
    sal_Int32 position_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
AttributeDescription::getGetExceptions() throw (css::uno::RuntimeException) {
    assert(attribute_.getExceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(attribute_.getExceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolve(context_, attribute_.getExceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
AttributeDescription::getSetExceptions() throw (css::uno::RuntimeException) {
    assert(attribute_.setExceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(attribute_.setExceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolve(context_, attribute_.setExceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

struct Method {
    struct Parameter {
        enum Direction { DIRECTION_IN, DIRECTION_OUT, DIRECTION_IN_OUT };

        Parameter(
            rtl::OUString const & theName, rtl::OUString const & theType,
            Direction theDirection):
            name(theName), type(theType), direction(theDirection)
        {}

        rtl::OUString name;
        rtl::OUString type;
        Direction direction;
    };

    Method(
        rtl::OUString const & theName, rtl::OUString const & theReturnType,
        std::vector< Parameter > const & theParameters,
        std::vector< rtl::OUString > const & theExceptions):
        name(theName), returnType(theReturnType), parameters(theParameters),
        exceptions(theExceptions)
    {}

    rtl::OUString name;
    rtl::OUString returnType;
    std::vector< Parameter > parameters;
    std::vector< rtl::OUString > exceptions;
};

class MethodParameter:
    public cppu::WeakImplHelper1< css::reflection::XMethodParameter >,
    private boost::noncopyable
{
public:
    MethodParameter(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        Method::Parameter parameter, sal_Int32 position):
        context_(context), parameter_(parameter), position_(position)
    {}

private:
    virtual ~MethodParameter() {}

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return parameter_.name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException)
    { return resolve(context_, parameter_.type); }

    virtual sal_Bool SAL_CALL isIn() throw (css::uno::RuntimeException) {
        return parameter_.direction == Method::Parameter::DIRECTION_IN
            || parameter_.direction == Method::Parameter::DIRECTION_IN_OUT;
    }

    virtual sal_Bool SAL_CALL isOut() throw (css::uno::RuntimeException) {
        return parameter_.direction == Method::Parameter::DIRECTION_OUT
            || parameter_.direction == Method::Parameter::DIRECTION_IN_OUT;
    }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    css::uno::Reference< css::uno::XComponentContext > context_;
    Method::Parameter parameter_;
    sal_Int32 position_;
};

class MethodDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XInterfaceMethodTypeDescription >,
    private boost::noncopyable
{
public:
    MethodDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, Method method, sal_Int32 position):
        context_(context), name_(name), method_(method), position_(position)
    {}

private:
    virtual ~MethodDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_INTERFACE_METHOD; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual rtl::OUString SAL_CALL getMemberName()
        throw (css::uno::RuntimeException)
    { return method_.name; }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReturnType() throw (css::uno::RuntimeException)
    { return resolve(context_, method_.returnType); }

    virtual sal_Bool SAL_CALL isOneway() throw (css::uno::RuntimeException)
    { return false; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XMethodParameter > >
    SAL_CALL getParameters() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getExceptions() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    Method method_;
    sal_Int32 position_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XMethodParameter > >
MethodDescription::getParameters() throw (css::uno::RuntimeException) {
    assert(method_.parameters.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(method_.parameters.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XMethodParameter > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new MethodParameter(context_, method_.parameters[i], i);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
MethodDescription::getExceptions() throw (css::uno::RuntimeException) {
    assert(method_.exceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(method_.exceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = resolve(context_, method_.exceptions[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XInterfaceTypeDescription2 >
InterfaceTypeDescription_Base;

class InterfaceTypeDescription: public InterfaceTypeDescription_Base {
public:
    InterfaceTypeDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published,
        std::vector< rtl::OUString > const & mandatoryDirectBases,
        std::vector< rtl::OUString > const & optionalDirectBases,
        std::vector< Attribute > const & directAttributes,
        std::vector< Method > const & directMethods):
        InterfaceTypeDescription_Base(published), context_(context),
        name_(name), mandatoryDirectBases_(mandatoryDirectBases),
        optionalDirectBases_(optionalDirectBases),
        directAttributes_(directAttributes), directMethods_(directMethods)
    {}

private:
    virtual ~InterfaceTypeDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_INTERFACE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getBaseType() throw (css::uno::RuntimeException) {
        return mandatoryDirectBases_.empty()
            ? css::uno::Reference< css::reflection::XTypeDescription >()
            : resolve(context_, mandatoryDirectBases_[0]);
    }

    virtual css::uno::Uik SAL_CALL getUik() throw (css::uno::RuntimeException)
    { return css::uno::Uik(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference<
             css::reflection::XInterfaceMemberTypeDescription > >
    SAL_CALL getMembers() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getBaseTypes() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >
    SAL_CALL getOptionalBaseTypes() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    std::vector< rtl::OUString > mandatoryDirectBases_;
    std::vector< rtl::OUString > optionalDirectBases_;
    std::vector< Attribute > directAttributes_;
    std::vector< Method > directMethods_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XInterfaceMemberTypeDescription > >
InterfaceTypeDescription::getMembers() throw (css::uno::RuntimeException) {
    assert(
        directAttributes_.size() <= SAL_MAX_INT32
        && directMethods_.size() <= SAL_MAX_INT32 - directAttributes_.size());
    sal_Int32 n1 = static_cast< sal_Int32 >(directAttributes_.size());
    sal_Int32 n2 = static_cast< sal_Int32 >(directMethods_.size());
    css::uno::Sequence<
        css::uno::Reference<
            css::reflection::XInterfaceMemberTypeDescription > > s(n1 + n2);
    sal_Int32 off = BaseOffset(this).get();
    for (sal_Int32 i = 0; i != n1; ++i) {
        s[i] = new AttributeDescription(
            context_, name_ + "::" + directAttributes_[i].name,
            directAttributes_[i], off + i);
    }
    for (sal_Int32 i = 0; i != n2; ++i) {
        s[n1 + i] = new MethodDescription(
            context_, name_ + "::" + directMethods_[i].name,
            directMethods_[i], off + n1 + i);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InterfaceTypeDescription::getBaseTypes() throw (css::uno::RuntimeException) {
    assert(mandatoryDirectBases_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(mandatoryDirectBases_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = resolve(context_, mandatoryDirectBases_[i]);
    }
    return s;
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
InterfaceTypeDescription::getOptionalBaseTypes()
    throw (css::uno::RuntimeException)
{
    assert(optionalDirectBases_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(optionalDirectBases_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = resolve(context_, optionalDirectBases_[i]);
    }
    return s;
}

class ConstantDescription:
    public cppu::WeakImplHelper1< css::reflection::XConstantTypeDescription >,
    private boost::noncopyable
{
public:
    ConstantDescription(
        rtl::OUString const & name, css::uno::Any const & value):
        name_(name), value_(value)
    {}

private:
    virtual ~ConstantDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_CONSTANT; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Any SAL_CALL getConstantValue()
        throw (css::uno::RuntimeException)
    { return value_; }

    rtl::OUString name_;
    css::uno::Any value_;
};

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XConstantsTypeDescription >
ConstantGroupDescription_Base;

class ConstantGroupDescription: public ConstantGroupDescription_Base {
public:
    ConstantGroupDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published,
        std::vector< rtl::OUString > const & constants):
        ConstantGroupDescription_Base(published), context_(context),
        name_(name), constants_(constants)
    {}

private:
    virtual ~ConstantGroupDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_CONSTANTS; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XConstantTypeDescription > >
    SAL_CALL getConstants() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    std::vector< rtl::OUString > constants_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XConstantTypeDescription > >
ConstantGroupDescription::getConstants() throw (css::uno::RuntimeException) {
    assert(constants_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(constants_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XConstantTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolve(context_, name_ + "." + constants_[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XIndirectTypeDescription >
TypedefDescription_Base;

class TypedefDescription: public TypedefDescription_Base {
public:
    TypedefDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published, rtl::OUString const & type):
        TypedefDescription_Base(published), context_(context), name_(name),
        type_(type)
    {}

private:
    virtual ~TypedefDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_TYPEDEF; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getReferencedType() throw (css::uno::RuntimeException)
    { return resolve(context_, type_); }

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    rtl::OUString type_;
};

struct Constructor {
    struct Parameter {
        Parameter(
            rtl::OUString const & theName, rtl::OUString const & theType,
            bool theRest):
            name(theName), type(theType), rest(theRest)
        {}

        rtl::OUString name;
        rtl::OUString type;
        bool rest;
    };

    Constructor(): defaultConstructor(true) {}

    Constructor(
        rtl::OUString const & theName,
        std::vector< Parameter > const & theParameters,
        std::vector< rtl::OUString > const & theExceptions):
        defaultConstructor(false), name(theName), parameters(theParameters),
        exceptions(theExceptions)
    {}

    bool defaultConstructor;
    rtl::OUString name;
    std::vector< Parameter > parameters;
    std::vector< rtl::OUString > exceptions;
};

class ConstructorParameter:
    public cppu::WeakImplHelper1< css::reflection::XParameter >,
    private boost::noncopyable
{
public:
    ConstructorParameter(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        Constructor::Parameter parameter, sal_Int32 position):
        context_(context), parameter_(parameter), position_(position)
    {}

private:
    virtual ~ConstructorParameter() {}

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return parameter_.name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException)
    { return resolve(context_, parameter_.type); }

    virtual sal_Bool SAL_CALL isIn() throw (css::uno::RuntimeException)
    { return true; }

    virtual sal_Bool SAL_CALL isOut() throw (css::uno::RuntimeException)
    { return false; }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return position_; }

    virtual sal_Bool SAL_CALL isRestParameter()
        throw (css::uno::RuntimeException)
    { return parameter_.rest; }

    css::uno::Reference< css::uno::XComponentContext > context_;
    Constructor::Parameter parameter_;
    sal_Int32 position_;
};

class ConstructorDescription:
    public cppu::WeakImplHelper1<
        css::reflection::XServiceConstructorDescription >,
    private boost::noncopyable
{
public:
    ConstructorDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        Constructor const & constructor):
        context_(context), constructor_(constructor)
    {}

private:
    virtual ~ConstructorDescription() {}

    virtual sal_Bool SAL_CALL isDefaultConstructor()
        throw (css::uno::RuntimeException)
    { return constructor_.defaultConstructor; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return constructor_.name; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XParameter > >
    SAL_CALL getParameters() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
    SAL_CALL getExceptions() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    Constructor constructor_;
};

css::uno::Sequence< css::uno::Reference< css::reflection::XParameter > >
ConstructorDescription::getParameters() throw (css::uno::RuntimeException) {
    assert(constructor_.parameters.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(constructor_.parameters.size());
    css::uno::Sequence< css::uno::Reference< css::reflection::XParameter > > s(
        n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new ConstructorParameter(
            context_, constructor_.parameters[i], i);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
ConstructorDescription::getExceptions() throw (css::uno::RuntimeException) {
    assert(constructor_.exceptions.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(constructor_.exceptions.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolve(context_, constructor_.exceptions[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XServiceTypeDescription2 >
SingleInterfaceBasedServiceDescription_Base;

class SingleInterfaceBasedServiceDescription:
    public SingleInterfaceBasedServiceDescription_Base
{
public:
    SingleInterfaceBasedServiceDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published, rtl::OUString const & type,
        std::vector< Constructor > const & constructors):
        SingleInterfaceBasedServiceDescription_Base(published),
        context_(context), name_(name), type_(type), constructors_(constructors)
    {}

private:
    virtual ~SingleInterfaceBasedServiceDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SERVICE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getMandatoryServices() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XServiceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getOptionalServices() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference< css::reflection::XServiceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getMandatoryInterfaces() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XInterfaceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getOptionalInterfaces() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XInterfaceTypeDescription > >();
    }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > >
    SAL_CALL getProperties() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XPropertyTypeDescription > >();
    }

    virtual sal_Bool SAL_CALL isSingleInterfaceBased()
        throw (css::uno::RuntimeException)
    { return true; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getInterface() throw (css::uno::RuntimeException)
    { return resolve(context_, type_); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
    SAL_CALL getConstructors() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    rtl::OUString type_;
    std::vector< Constructor > constructors_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceConstructorDescription > >
SingleInterfaceBasedServiceDescription::getConstructors()
    throw (css::uno::RuntimeException)
{
    assert(constructors_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(constructors_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
            s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new ConstructorDescription(context_, constructors_[i]);
    }
    return s;
}

struct Property {
    Property(
        rtl::OUString const & theName, rtl::OUString const & theType,
        sal_uInt16 theAttributes):
        name(theName), type(theType), attributes(theAttributes)
    {}

    rtl::OUString name;
    rtl::OUString type;
    sal_Int16 attributes;
};

class PropertyDescription:
    public cppu::WeakImplHelper1< css::reflection::XPropertyTypeDescription >,
    private boost::noncopyable
{
public:
    PropertyDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        Property const & property):
        context_(context), property_(property)
    {}

private:
    virtual ~PropertyDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_PROPERTY; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return property_.name; }

    virtual sal_Int16 SAL_CALL getPropertyFlags()
        throw (css::uno::RuntimeException)
    { return property_.attributes; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getPropertyTypeDescription() throw (css::uno::RuntimeException)
    { return resolve(context_, property_.type); }

    css::uno::Reference< css::uno::XComponentContext > context_;
    Property property_;
};

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XServiceTypeDescription2 >
AccumulationBasedServiceDescription_Base;

class AccumulationBasedServiceDescription:
    public AccumulationBasedServiceDescription_Base
{
public:
    AccumulationBasedServiceDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published,
        std::vector< rtl::OUString > const & mandatoryDirectBaseServices,
        std::vector< rtl::OUString > const & optionalDirectBaseServices,
        std::vector< rtl::OUString > const & mandatoryDirectBaseInterfaces,
        std::vector< rtl::OUString > const & optionalDirectBaseInterfaces,
        std::vector< Property > const & directProperties):
        AccumulationBasedServiceDescription_Base(published), context_(context),
        name_(name), mandatoryDirectBaseServices_(mandatoryDirectBaseServices),
        optionalDirectBaseServices_(optionalDirectBaseServices),
        mandatoryDirectBaseInterfaces_(mandatoryDirectBaseInterfaces),
        optionalDirectBaseInterfaces_(optionalDirectBaseInterfaces),
        directProperties_(directProperties)
    {}

private:
    virtual ~AccumulationBasedServiceDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SERVICE; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getMandatoryServices() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > >
    SAL_CALL getOptionalServices() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getMandatoryInterfaces() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
    SAL_CALL getOptionalInterfaces() throw (css::uno::RuntimeException);

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > >
    SAL_CALL getProperties() throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isSingleInterfaceBased()
        throw (css::uno::RuntimeException)
    { return false; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getInterface() throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    virtual
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceConstructorDescription > >
    SAL_CALL getConstructors() throw (css::uno::RuntimeException)
    {
        return css::uno::Sequence<
            css::uno::Reference<
                css::reflection::XServiceConstructorDescription > >();
    }

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    std::vector< rtl::OUString > mandatoryDirectBaseServices_;
    std::vector< rtl::OUString > optionalDirectBaseServices_;
    std::vector< rtl::OUString > mandatoryDirectBaseInterfaces_;
    std::vector< rtl::OUString > optionalDirectBaseInterfaces_;
    std::vector< Property > directProperties_;
};

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceTypeDescription > >
AccumulationBasedServiceDescription::getMandatoryServices()
    throw (css::uno::RuntimeException)
{
    assert(mandatoryDirectBaseServices_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(mandatoryDirectBaseServices_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolve(context_, mandatoryDirectBaseServices_[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XServiceTypeDescription > >
AccumulationBasedServiceDescription::getOptionalServices()
    throw (css::uno::RuntimeException)
{
    assert(optionalDirectBaseServices_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(optionalDirectBaseServices_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XServiceTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolve(context_, optionalDirectBaseServices_[i]),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
AccumulationBasedServiceDescription::getMandatoryInterfaces()
    throw (css::uno::RuntimeException)
{
    assert(mandatoryDirectBaseInterfaces_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        mandatoryDirectBaseInterfaces_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > > s(
            n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolveTypedefs(
                resolve(context_, mandatoryDirectBaseInterfaces_[i])),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XInterfaceTypeDescription > >
AccumulationBasedServiceDescription::getOptionalInterfaces()
    throw (css::uno::RuntimeException)
{
    assert(optionalDirectBaseInterfaces_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(
        optionalDirectBaseInterfaces_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > > s(
            n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i].set(
            resolveTypedefs(
                resolve(context_, optionalDirectBaseInterfaces_[i])),
            css::uno::UNO_QUERY_THROW);
    }
    return s;
}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XPropertyTypeDescription > >
AccumulationBasedServiceDescription::getProperties()
    throw (css::uno::RuntimeException)
{
    assert(directProperties_.size() <= SAL_MAX_INT32);
    sal_Int32 n = static_cast< sal_Int32 >(directProperties_.size());
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XPropertyTypeDescription > > s(n);
    for (sal_Int32 i = 0; i != n; ++i) {
        s[i] = new PropertyDescription(context_, directProperties_[i]);
    }
    return s;
}

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XSingletonTypeDescription2 >
InterfaceBasedSingletonDescription_Base;

class InterfaceBasedSingletonDescription:
    public InterfaceBasedSingletonDescription_Base
{
public:
    InterfaceBasedSingletonDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published, rtl::OUString const & type):
        InterfaceBasedSingletonDescription_Base(published), context_(context),
        name_(name), type_(type)
    {}

private:
    virtual ~InterfaceBasedSingletonDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SINGLETON; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XServiceTypeDescription >
    SAL_CALL getService() throw (css::uno::RuntimeException)
    {
        return
            css::uno::Reference< css::reflection::XServiceTypeDescription >();
    }

    virtual sal_Bool SAL_CALL isInterfaceBased()
        throw (css::uno::RuntimeException)
    { return true; }

    virtual css::uno::Reference< css::reflection::XTypeDescription >
    SAL_CALL getInterface() throw (css::uno::RuntimeException)
    { return resolve(context_, type_); }

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    rtl::OUString type_;
};

typedef cppu::ImplInheritanceHelper1<
    PublishableDescription, css::reflection::XSingletonTypeDescription2 >
ServiceBasedSingletonDescription_Base;

class ServiceBasedSingletonDescription:
    public ServiceBasedSingletonDescription_Base
{
public:
    ServiceBasedSingletonDescription(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & name, bool published, rtl::OUString const & type):
        ServiceBasedSingletonDescription_Base(published), context_(context),
        name_(name), type_(type)
    {}

private:
    virtual ~ServiceBasedSingletonDescription() {}

    virtual css::uno::TypeClass SAL_CALL getTypeClass()
        throw (css::uno::RuntimeException)
    { return css::uno::TypeClass_SINGLETON; }

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return name_; }

    virtual css::uno::Reference< css::reflection::XServiceTypeDescription >
    SAL_CALL getService() throw (css::uno::RuntimeException)
    {
        return css::uno::Reference< css::reflection::XServiceTypeDescription >(
            resolve(context_, type_), css::uno::UNO_QUERY_THROW);
    }

    virtual sal_Bool SAL_CALL isInterfaceBased()
        throw (css::uno::RuntimeException)
    { return false; }

    virtual css::uno::Reference< css::reflection::XTypeDescription >
    SAL_CALL getInterface() throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::reflection::XTypeDescription >(); }

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::OUString name_;
    rtl::OUString type_;
};

// sizeof (MapEntry) == 8
struct MapEntry {
    Memory32 name;
    Memory32 data;
};

class Enumeration:
    public cppu::WeakImplHelper1<
        css::reflection::XTypeDescriptionEnumeration >,
    private boost::noncopyable
{
public:
    Enumeration(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::Reference< MappedFile > const & file, rtl::OUString const & prefix,
        MapEntry const * mapBegin, sal_uInt32 mapSize,
        css::uno::Sequence< css::uno::TypeClass > const & types, bool deep):
        context_(context), file_(file), types_(types), deep_(deep)
    {
        positions_.push(Position(prefix, mapBegin, mapSize, false));
        findMatch();
    }

private:
    virtual ~Enumeration() {}

    virtual sal_Bool SAL_CALL hasMoreElements()
        throw (css::uno::RuntimeException)
    { return !positions_.empty(); }

    virtual css::uno::Any SAL_CALL nextElement()
        throw (
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException, css::uno::RuntimeException)
    { return css::uno::makeAny(nextTypeDescription()); }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    nextTypeDescription()
        throw (
            css::container::NoSuchElementException, css::uno::RuntimeException);

    bool matches(css::uno::TypeClass tc) const;

    void proceed();

    void findMatch();

    struct Position {
        Position(
            rtl::OUString const & thePrefix, MapEntry const * mapBegin,
            sal_uInt32 mapSize, bool theConstantGroup):
            prefix(thePrefix), position(mapBegin), end(mapBegin + mapSize),
            constantGroup(theConstantGroup)
        {}

        rtl::OUString prefix;
        MapEntry const * position;
        MapEntry const * end;
        bool constantGroup;
    };

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::Reference< MappedFile > file_;
    css::uno::Sequence< css::uno::TypeClass > types_;
    bool deep_;

    osl::Mutex mutex_;
    std::stack< Position > positions_;
    rtl::OUString current_;
};

css::uno::Reference< css::reflection::XTypeDescription >
Enumeration::nextTypeDescription()
    throw (css::container::NoSuchElementException, css::uno::RuntimeException)
{
    rtl::OUString name;
    {
        osl::MutexGuard g(mutex_);
        if (positions_.empty()) {
            throw css::container::NoSuchElementException(
                "exhausted XTypeDescriptionEnumeration",
                static_cast< cppu::OWeakObject * >(this));
        }
        name = current_;
        proceed();
        findMatch();
    }
    return resolve(context_, name);
}

bool Enumeration::matches(css::uno::TypeClass tc) const {
    if (types_.getLength() == 0) {
        return true;
    }
    for (sal_Int32 i = 0; i != types_.getLength(); ++i) {
        if (types_[i] == tc) {
            return true;
        }
    }
    return false;
}

void Enumeration::proceed() {
    assert(!positions_.empty());
    assert(positions_.top().position < positions_.top().end);
    if (deep_) {
        sal_uInt32 off = positions_.top().position->data.getUnsigned32();
        int v = file_->read8(off);
        bool recurse;
        bool cgroup = bool();
        if (v == 0) {
            recurse = true;
            cgroup = false;
        } else if ((v & 0x3F) == 7 && matches(css::uno::TypeClass_CONSTANT)) {
            recurse = true;
            cgroup = true;
        } else {
            recurse = false;
        }
        if (recurse) {
            rtl::OUString prefix(
                positions_.top().prefix
                + file_->readNameNul(
                    positions_.top().position->name.getUnsigned32())
                + ".");
            sal_uInt32 mapSize = file_->read32(off + 1);
            if (8 * mapSize > file_->size - off - 5) { //TODO: overflow
                throw css::uno::DeploymentException(
                    "broken UNOIDL file: map offset + size too large",
                    css::uno::Reference< css::uno::XInterface >());
            }
            MapEntry const * mapBegin = reinterpret_cast< MapEntry const * >(
                static_cast< char const * >(file_->address) + off + 5);
            ++positions_.top().position;
            positions_.push(Position(prefix, mapBegin, mapSize, cgroup));
            return;
        }
    }
    ++positions_.top().position;
}

void Enumeration::findMatch() {
    assert(!positions_.empty());
    for (;; proceed()) {
        while (positions_.top().position == positions_.top().end) {
            positions_.pop();
            if (positions_.empty()) {
                return;
            }
        }
        bool match;
        if (positions_.top().constantGroup) {
            assert(matches(css::uno::TypeClass_CONSTANT));
            match = true;
        } else {
            sal_uInt32 off = positions_.top().position->data.getUnsigned32();
            int v = file_->read8(off);
            css::uno::TypeClass tc;
            switch (v & 0x1F) {
            case 0: // module
                tc = css::uno::TypeClass_MODULE;
                break;
            case 1: // enum type
                tc = css::uno::TypeClass_ENUM;
                break;
            case 2: // plain struct type (with or without base)
            case 3: // polymorphic struct type template
                tc = css::uno::TypeClass_STRUCT;
                break;
            case 4: // exception type (with or without base)
                tc = css::uno::TypeClass_EXCEPTION;
                break;
            case 5: // interface type
                tc = css::uno::TypeClass_INTERFACE;
                break;
            case 6: // typedef
                tc = css::uno::TypeClass_TYPEDEF;
                break;
            case 7: // constant group
                tc = css::uno::TypeClass_CONSTANTS;
                break;
            case 8: // single-interface--based service (with or without default
                    // constructor)
            case 9: // accumulation-based service
                tc = css::uno::TypeClass_SERVICE;
                break;
            case 10: // interface-based singleton
            case 11: // service-based singleton
                tc = css::uno::TypeClass_SINGLETON;
                break;
            default:
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: bad type byte "
                     + rtl::OUString::number(v)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            match = matches(tc);
        }
        if (match) {
            current_ = positions_.top().prefix
                + file_->readNameNul(
                    positions_.top().position->name.getUnsigned32());
            return;
        }
    }
}

typedef cppu::WeakComponentImplHelper2<
    css::container::XHierarchicalNameAccess,
    css::reflection::XTypeDescriptionEnumerationAccess >
Provider_Base;

class Provider:
    private osl::Mutex, public Provider_Base, private boost::noncopyable
{
public:
    Provider(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & uri);

    using Provider_Base::acquire;
    using Provider_Base::release;

private:
    enum Compare { COMPARE_LESS, COMPARE_GREATER, COMPARE_EQUAL };

    virtual ~Provider() {}

    virtual void SAL_CALL disposing() {} //TODO

    virtual css::uno::Any SAL_CALL getByHierarchicalName(
        rtl::OUString const & aName)
        throw (
            css::container::NoSuchElementException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByHierarchicalName(
        rtl::OUString const & aName) throw (css::uno::RuntimeException)
    { return find(aName) != 0; }

    virtual css::uno::Reference< css::reflection::XTypeDescriptionEnumeration >
    SAL_CALL createTypeDescriptionEnumeration(
        rtl::OUString const & moduleName,
        css::uno::Sequence< css::uno::TypeClass > const & types,
        css::reflection::TypeDescriptionSearchDepth depth)
        throw(
            css::reflection::NoSuchTypeNameException,
            css::reflection::InvalidTypeNameException,
            css::uno::RuntimeException);

    sal_uInt32 find(rtl::OUString const & name, bool * constant = 0) const;

    sal_uInt32 findInMap(
        rtl::OUString const & name, sal_Int32 nameOffset, sal_Int32 nameLength,
        MapEntry const * mapBegin, sal_uInt32 mapSize) const;

    Compare compare(
        rtl::OUString const & name, sal_Int32 nameOffset, sal_Int32 nameLength,
        MapEntry const * entry) const;

    css::uno::Reference< css::uno::XComponentContext > context_;
    rtl::Reference< MappedFile > file_;
    MapEntry const * mapBegin_;
    sal_uInt32 mapSize_;
};

Provider::Provider(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    rtl::OUString const & uri):
    Provider_Base(*static_cast< osl::Mutex * >(this)), context_(context),
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

css::uno::Any Provider::getByHierarchicalName(rtl::OUString const & aName)
    throw (css::container::NoSuchElementException, css::uno::RuntimeException)
{
    bool cnst;
    sal_uInt32 off = find(aName, &cnst);
    if (off == 0) {
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    if (cnst) {
        int v = file_->read8(off);
        int type = v & 0x7F;
        bool deprecated = (v & 0x80) != 0; (void)deprecated;//TODO
        css::uno::Any any;
        switch (type) {
        case 0: // BOOLEAN
            v = file_->read8(off + 1);
            switch (v) {
            case 0:
                any <<= false;
                break;
            case 1:
                any <<= true;
                break;
            default:
                throw css::uno::DeploymentException(
                    ("broken UNOIDL file: bad boolean constant value "
                     + rtl::OUString::number(v)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case 1: // BYTE
            any <<= static_cast< sal_Int8 >(file_->read8(off + 1));
                //TODO: implementation-defined behavior of conversion from
                // sal_uInt8 to sal_Int8 relies on two's complement
                // representation
            break;
        case 2: // SHORT
            any <<= static_cast< sal_Int16 >(file_->read16(off + 1));
                //TODO: implementation-defined behavior of conversion from
                // sal_uInt16 to sal_Int16 relies on two's complement
                // representation
            break;
        case 3: // UNSIGNED SHORT
            any <<= file_->read16(off + 1);
            break;
        case 4: // LONG
            any <<= static_cast< sal_Int32 >(file_->read32(off + 1));
                //TODO: implementation-defined behavior of conversion from
                // sal_uInt32 to sal_Int32 relies on two's complement
                // representation
            break;
        case 5: // UNSIGNED LONG
            any <<= file_->read32(off + 1);
            break;
        case 6: // HYPER
            any <<= static_cast< sal_Int64 >(file_->read64(off + 1));
                //TODO: implementation-defined behavior of conversion from
                // sal_uInt64 to sal_Int64 relies on two's complement
                // representation
            break;
        case 7: // UNSIGNED HYPER
            any <<= file_->read64(off + 1);
            break;
        case 8: // FLOAT
            any <<= file_->readIso60599Binary32(off + 1);
            break;
        case 9: // DOUBLE
            any <<= file_->readIso60599Binary64(off + 1);
            break;
        default:
            throw css::uno::DeploymentException(
                ("broken UNOIDL file: bad constant type byte "
                 + rtl::OUString::number(v)),
                css::uno::Reference< css::uno::XInterface >());
        }
        return css::uno::makeAny<
            css::uno::Reference< css::reflection::XTypeDescription > >(
                new ConstantDescription(aName, any));
    } else {
        int v = file_->read8(off);
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
                sal_uInt32 n = file_->read32(off + 1);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many items in module "
                         + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 5;
                std::vector< rtl::OUString > items;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    items.push_back(file_->readNameNul(file_->read32(off)));
                    off += 8;
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new ModuleDescription(context_, aName, items));
            }
        case 1: // enum type
            {
                sal_uInt32 n = file_->read32(off + 1);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many members of enum type "
                         + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 5;
                std::vector< EnumTypeDescription::Member > mems;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    rtl::OUString memName(file_->readNameLen(off, &off));
                    sal_Int32 memValue = static_cast< sal_Int32 >(
                        file_->read32(off));
                        //TODO: implementation-defined behavior of conversion
                        // from sal_uInt32 to sal_Int32 relies on two's
                        // complement representation
                    off += 4;
                    mems.push_back(
                        EnumTypeDescription::Member(memName, memValue));
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new EnumTypeDescription(aName, published, mems));
            }
        case 2: // plain struct type without base
        case 2 | 0x20: // plain struct type with base
            {
                ++off;
                rtl::OUString base;
                if (flag) {
                    base = file_->readNameLen(off, &off);
                    if (base.isEmpty()) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: empty base type name of plain"
                             " struct type " + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                }
                sal_uInt32 n = file_->read32(off);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many direct members of plain"
                         " struct type " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< PlainStructTypeDescription::Member > mems;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    rtl::OUString memName(file_->readNameLen(off, &off));
                    rtl::OUString memType(file_->readNameLen(off, &off));
                    mems.push_back(
                        PlainStructTypeDescription::Member(memName, memType));
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new PlainStructTypeDescription(
                            context_, aName, published, base, mems));
            }
        case 3: // polymorphic struct type template
            {
                sal_uInt32 n = file_->read32(off + 1);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many type parameters of"
                         " polymorphic struct type template " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 5;
                std::vector< rtl::OUString > params;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    params.push_back(file_->readNameLen(off, &off));
                }
                n = file_->read32(off);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many members of polymorphic"
                         " struct type template " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< PolymorphicStructTypeTemplateDescription::Member >
                    mems;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    v = file_->read8(off);
                    ++off;
                    rtl::OUString memName(file_->readNameLen(off, &off));
                    rtl::OUString memType(file_->readNameLen(off, &off));
                    if (v > 1) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: bad flags "
                             + rtl::OUString::number(v) + " for member "
                             + memName + " of polymorphic struct type template "
                             + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    mems.push_back(
                        PolymorphicStructTypeTemplateDescription::Member(
                            memName, memType, v == 1));
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new PolymorphicStructTypeTemplateDescription(
                            context_, aName, published, params, mems));
            }
        case 4: // exception type without base
        case 4 | 0x20: // exception type with base
            {
                ++off;
                rtl::OUString base;
                if (flag) {
                    base = file_->readNameLen(off, &off);
                    if (base.isEmpty()) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: empty base type name of"
                             " exception type " + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                }
                sal_uInt32 n = file_->read32(off);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many direct members of"
                         " exception type " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< ExceptionTypeDescription::Member > mems;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    rtl::OUString memName(file_->readNameLen(off, &off));
                    rtl::OUString memType(file_->readNameLen(off, &off));
                    mems.push_back(
                        ExceptionTypeDescription::Member(memName, memType));
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new ExceptionTypeDescription(
                            context_, aName, published, base, mems));
            }
        case 5: // interface type
            {
                sal_uInt32 n = file_->read32(off + 1);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many mandatory direct bases"
                         " of interface type " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 5;
                std::vector< rtl::OUString > mandBases;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    mandBases.push_back(file_->readNameLen(off, &off));
                }
                n = file_->read32(off);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many optional direct bases of"
                         " interface type " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< rtl::OUString > optBases;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    optBases.push_back(file_->readNameLen(off, &off));
                }
                sal_uInt32 nAttrs = file_->read32(off);
                if (nAttrs > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many direct attributes of"
                         " interface type " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< Attribute > attrs;
                for (sal_uInt32 i = 0; i != nAttrs; ++i) {
                    v = file_->read8(off);
                    ++off;
                    rtl::OUString attrName(file_->readNameLen(off, &off));
                    rtl::OUString attrType(file_->readNameLen(off, &off));
                    if (v > 0x03) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: bad flags for direct"
                             " attribute " + attrName + " of interface type "
                             + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    std::vector< rtl::OUString > getExcs;
                    sal_uInt32 m = file_->read32(off);
                    if (m > SAL_MAX_INT32) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: too many getter exceptions"
                             " for direct attribute " + attrName
                             + " of interface type " + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    off += 4;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        getExcs.push_back(file_->readNameLen(off, &off));
                    }
                    std::vector< rtl::OUString > setExcs;
                    if ((v & 0x02) == 0) {
                        m = file_->read32(off);
                        if (m > SAL_MAX_INT32) {
                            throw css::uno::DeploymentException(
                                ("broken UNOIDL file: too many setter"
                                 " exceptions for direct attribute " + attrName
                                 + " of interface type " + aName),
                                css::uno::Reference< css::uno::XInterface >());
                        }
                        off += 4;
                        for (sal_uInt32 j = 0; j != m; ++j) {
                            setExcs.push_back(file_->readNameLen(off, &off));
                        }
                    }
                    attrs.push_back(
                        Attribute(
                            attrName, attrType, (v & 0x01) != 0,
                            (v & 0x02) != 0, getExcs, setExcs));
                }
                sal_uInt32 nMeths = file_->read32(off);
                if (nMeths > SAL_MAX_INT32 - nAttrs) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many direct attributes and"
                         " methods of interface type " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< Method > meths;
                for (sal_uInt32 i = 0; i != nMeths; ++i) {
                    rtl::OUString methName(file_->readNameLen(off, &off));
                    rtl::OUString methType(file_->readNameLen(off, &off));
                    sal_uInt32 m = file_->read32(off);
                    if (m > SAL_MAX_INT32) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: too many parameters for"
                             " method " + methName + " of interface type "
                             + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    off += 4;
                    std::vector< Method::Parameter > params;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        v = file_->read8(off);
                        ++off;
                        rtl::OUString paramName(file_->readNameLen(off, &off));
                        rtl::OUString paramType(file_->readNameLen(off, &off));
                        Method::Parameter::Direction dir;
                        switch (v) {
                        case 0:
                            dir = Method::Parameter::DIRECTION_IN;
                            break;
                        case 1:
                            dir = Method::Parameter::DIRECTION_OUT;
                            break;
                        case 2:
                            dir = Method::Parameter::DIRECTION_IN_OUT;
                            break;
                        default:
                            throw css::uno::DeploymentException(
                                ("broken UNOIDL file: bad direction "
                                 + rtl::OUString::number(v) + " of parameter "
                                 + paramName + " for method " + methName
                                 + " of interface type " + aName),
                                css::uno::Reference< css::uno::XInterface >());
                        }
                        params.push_back(
                            Method::Parameter(paramName, paramType, dir));
                    }
                    std::vector< rtl::OUString > excs;
                    m = file_->read32(off);
                    if (m > SAL_MAX_INT32) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: too many exceptions for"
                             " method " + methName + " of interface type "
                             + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    off += 4;
                    for (sal_uInt32 j = 0; j != m; ++j) {
                        excs.push_back(file_->readNameLen(off, &off));
                    }
                    meths.push_back(Method(methName, methType, params, excs));
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new InterfaceTypeDescription(
                            context_, aName, published, mandBases, optBases,
                            attrs, meths));
            }
        case 6: // typedef
            return css::uno::makeAny<
                css::uno::Reference< css::reflection::XTypeDescription > >(
                    new TypedefDescription(
                        context_, aName, published,
                        file_->readNameLen(off + 1)));
        case 7: // constant group
            {
                sal_uInt32 n = file_->read32(off + 1);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many constants in constant"
                         " group " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 5;
                std::vector< rtl::OUString > consts;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    consts.push_back(file_->readNameNul(file_->read32(off)));
                    off += 8;
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new ConstantGroupDescription(
                            context_, aName, published, consts));
            }
        case 8: // single-interface--based service without default constructor
        case 8 | 0x20:
                // single-interface--based service with default constructor
            {
                rtl::OUString base(file_->readNameLen(off + 1, &off));
                std::vector< Constructor > ctors;
                if (flag) {
                    ctors.push_back(Constructor());
                } else {
                    sal_uInt32 n = file_->read32(off);
                    if (n > SAL_MAX_INT32) {
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: too many constructors of"
                             " single-interface--based service " + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    off += 4;
                    for (sal_uInt32 i = 0; i != n; ++i) {
                        rtl::OUString ctorName(file_->readNameLen(off, &off));
                        sal_uInt32 m = file_->read32(off);
                        if (m > SAL_MAX_INT32) {
                            throw css::uno::DeploymentException(
                                ("broken UNOIDL file: too many parameters for"
                                 " constructor " + ctorName
                                 + " of single-interface--based service "
                                 + aName),
                                css::uno::Reference< css::uno::XInterface >());
                        }
                        off += 4;
                        std::vector< Constructor::Parameter > params;
                        for (sal_uInt32 j = 0; j != m; ++j) {
                            v = file_->read8(off);
                            ++off;
                            rtl::OUString paramName(
                                file_->readNameLen(off, &off));
                            rtl::OUString paramType(
                                file_->readNameLen(off, &off));
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
                                     + rtl::OUString::number(v)
                                     + " of parameter " + paramName
                                     + " for constructor " + ctorName
                                     + " of single-interface--based service "
                                     + aName),
                                    css::uno::Reference<
                                        css::uno::XInterface >());
                            }
                            params.push_back(
                                Constructor::Parameter(
                                    paramName, paramType, rest));
                        }
                        std::vector< rtl::OUString > excs;
                        m = file_->read32(off);
                        if (m > SAL_MAX_INT32) {
                            throw css::uno::DeploymentException(
                                ("broken UNOIDL file: too many exceptions for"
                                 " constructor " + ctorName
                                 + " of single-interface--based service "
                                 + aName),
                                css::uno::Reference< css::uno::XInterface >());
                        }
                        off += 4;
                        for (sal_uInt32 j = 0; j != m; ++j) {
                            excs.push_back(file_->readNameLen(off, &off));
                        }
                        ctors.push_back(Constructor(ctorName, params, excs));
                    }
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new SingleInterfaceBasedServiceDescription(
                            context_, aName, published, base, ctors));
            }
        case 9: // accumulation-based service
            {
                sal_uInt32 n = file_->read32(off + 1);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many mandatory direct service"
                         " bases of accumulation-based service " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 5;
                std::vector< rtl::OUString > mandServs;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    mandServs.push_back(file_->readNameLen(off, &off));
                }
                n = file_->read32(off);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many optional direct service"
                         " bases of accumulation-based servcie " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< rtl::OUString > optServs;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    optServs.push_back(file_->readNameLen(off, &off));
                }
                n = file_->read32(off);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many mandatory direct"
                         " interface bases of accumulation-based servcie "
                         + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< rtl::OUString > mandIfcs;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    mandIfcs.push_back(file_->readNameLen(off, &off));
                }
                n = file_->read32(off);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many optional direct"
                         " interface bases of accumulation-based servcie "
                         + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< rtl::OUString > optIfcs;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    optIfcs.push_back(file_->readNameLen(off, &off));
                }
                n = file_->read32(off);
                if (n > SAL_MAX_INT32) {
                    throw css::uno::DeploymentException(
                        ("broken UNOIDL file: too many direct properties of"
                         " accumulation-based servcie " + aName),
                        css::uno::Reference< css::uno::XInterface >());
                }
                off += 4;
                std::vector< Property > props;
                for (sal_uInt32 i = 0; i != n; ++i) {
                    sal_uInt16 attrs = file_->read16(off);
                    off += 2;
                    rtl::OUString propName(file_->readNameLen(off, &off));
                    rtl::OUString propType(file_->readNameLen(off, &off));
                    if (attrs > 0x01FF) { // see css.beans.PropertyAttribute
                        throw css::uno::DeploymentException(
                            ("broken UNOIDL file: bad mode "
                             + rtl::OUString::number(v) + " of property "
                             + propName + " for accumulation-based servcie "
                             + aName),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    props.push_back(
                        Property(
                            propName, propType,
                            static_cast< sal_Int32 >(attrs)));
                }
                return css::uno::makeAny<
                    css::uno::Reference< css::reflection::XTypeDescription > >(
                        new AccumulationBasedServiceDescription(
                            context_, aName, published, mandServs, optServs,
                            mandIfcs, optIfcs, props));
            }
        case 10: // interface-based singleton
            return css::uno::makeAny<
                css::uno::Reference< css::reflection::XTypeDescription > >(
                    new InterfaceBasedSingletonDescription(
                        context_, aName, published,
                        file_->readNameLen(off + 1)));
        case 11: // service-based singleton
            return css::uno::makeAny<
                css::uno::Reference< css::reflection::XTypeDescription > >(
                    new ServiceBasedSingletonDescription(
                        context_, aName, published,
                        file_->readNameLen(off + 1)));
        default:
            throw css::uno::DeploymentException(
                "broken UNOIDL file: bad type byte " + rtl::OUString::number(v),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

css::uno::Reference< css::reflection::XTypeDescriptionEnumeration >
Provider::createTypeDescriptionEnumeration(
    rtl::OUString const & moduleName,
    css::uno::Sequence< css::uno::TypeClass > const & types,
    css::reflection::TypeDescriptionSearchDepth depth)
    throw(
        css::reflection::NoSuchTypeNameException,
        css::reflection::InvalidTypeNameException, css::uno::RuntimeException)
{
    rtl::OUString prefix;
    MapEntry const * mapBegin;
    sal_uInt32 mapSize;
    if (moduleName.isEmpty()) {
        mapBegin = mapBegin_;
        mapSize = mapSize_;
    } else {
        prefix = moduleName + ".";
        bool cnst;
        sal_uInt32 off = find(moduleName, &cnst);
        if (off == 0) {
            throw css::reflection::NoSuchTypeNameException(
                moduleName, static_cast< cppu::OWeakObject * >(this));
        }
        if (cnst || file_->read8(off) != 0) { // module
            throw css::reflection::InvalidTypeNameException(
                moduleName, static_cast< cppu::OWeakObject * >(this));
        }
        mapSize = file_->read32(off + 1);
        if (8 * mapSize > file_->size - off - 5) { //TODO: overflow
            throw css::uno::DeploymentException(
                "broken UNOIDL file: map offset + size too large",
                css::uno::Reference< css::uno::XInterface >());
        }
        mapBegin = reinterpret_cast< MapEntry const * >(
            static_cast< char const * >(file_->address) + off + 5);
    }
    return new Enumeration(
        context_, file_, prefix, mapBegin, mapSize, types,
        depth == css::reflection::TypeDescriptionSearchDepth_INFINITE);
}

sal_uInt32 Provider::find(rtl::OUString const & name, bool * constant) const {
    MapEntry const * mapBegin = mapBegin_;
    sal_uInt32 mapSize = mapSize_;
    bool cgroup = false;
    for (sal_Int32 i = 0;;) {
        sal_Int32 j = name.indexOf('.', i);
        if (j == -1) {
            j = name.getLength();
        }
        sal_Int32 off = findInMap(name, i, j - i, mapBegin, mapSize);
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

sal_uInt32 Provider::findInMap(
    rtl::OUString const & name, sal_Int32 nameOffset, sal_Int32 nameLength,
    MapEntry const * mapBegin, sal_uInt32 mapSize) const
{
    if (mapSize == 0) {
        return 0;
    }
    sal_uInt32 n = mapSize / 2;
    MapEntry const * p = mapBegin + n;
    switch (compare(name, nameOffset, nameLength, p)) {
    case COMPARE_LESS:
        return findInMap(name, nameOffset, nameLength, mapBegin, n);
    case COMPARE_GREATER:
        return findInMap(name, nameOffset, nameLength, p + 1, mapSize - n - 1);
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

Provider::Compare Provider::compare(
    rtl::OUString const & name, sal_Int32 nameOffset, sal_Int32 nameLength,
    MapEntry const * entry) const
{
    assert(entry != 0);
    sal_uInt32 off = entry->name.getUnsigned32();
    if (off > file_->size - 1) { // at least a trailing NUL
        throw css::uno::DeploymentException(
            "broken UNOIDL file: string offset too large",
            css::uno::Reference< css::uno::XInterface >());
    }
    assert(nameLength >= 0);
    sal_uInt64 min = std::min(
        static_cast< sal_uInt64 >(nameLength), file_->size - off);
    for (sal_uInt64 i = 0; i != min; ++i) {
        sal_Unicode c1 = name[nameOffset + i];
        sal_Unicode c2 = static_cast< unsigned char const * >(file_->address)[
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
        if (file_->size - off == min) {
            throw css::uno::DeploymentException(
                "broken UNOIDL file: string misses trailing NUL",
                css::uno::Reference< css::uno::XInterface >());
        }
        return
            static_cast< unsigned char const * >(file_->address)[off + min] == 0
            ? COMPARE_EQUAL : COMPARE_LESS;
    } else {
        return COMPARE_GREATER;
    }
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
readLegacyRdbFile(
    rtl::OUString const & uri,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    assert(serviceManager.is());
    css::uno::Reference< css::registry::XSimpleRegistry > reg(
        serviceManager->createInstanceWithContext(
            "com.sun.star.comp.stoc.SimpleRegistry", context),
        css::uno::UNO_QUERY_THROW);
    try {
        reg->open(uri, true, false);
    } catch (css::registry::InvalidRegistryException & e) {
        throw css::uno::DeploymentException(
            "Invalid registry " + uri + ":" + e.Message,
            css::uno::Reference< css::uno::XInterface >());
    }
    css::uno::Sequence< css::uno::Any > arg(1);
    arg[0] <<= reg;
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        serviceManager->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.stoc.RegistryTypeDescriptionProvider", arg,
            context),
        css::uno::UNO_QUERY_THROW);
}

void readRdbFile(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context,
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > *
            providers)
{
    assert(providers != 0);
    css::uno::Reference< css::container::XHierarchicalNameAccess > prov;
    try {
        prov = new Provider(context, uri);
    } catch (css::container::NoSuchElementException &) {
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return;
        }
        throw css::uno::DeploymentException(
            uri + ": no such file",
            css::uno::Reference< css::uno::XInterface >());
    } catch (css::registry::InvalidRegistryException &) {
        prov = readLegacyRdbFile(uri, serviceManager, context);
    }
    assert(prov.is());
    providers->push_back(prov);
}

void readRdbDirectory(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context,
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > *
            providers)
{
    osl::Directory dir(uri);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return;
        }
        // fall through
    default:
        throw css::uno::DeploymentException(
            "Cannot open directory " + uri,
            css::uno::Reference< css::uno::XInterface >());
    }
    for (;;) {
        rtl::OUString fileUri;
        if (!cppu::nextDirectoryItem(dir, &fileUri)) {
            break;
        }
        readRdbFile(fileUri, optional, serviceManager, context, providers);
    }
}

}

css::uno::Sequence<
    css::uno::Reference< css::container::XHierarchicalNameAccess > >
cppuhelper::createTypeDescriptionProviders(
    rtl::OUString const & uris,
    css::uno::Reference< css::lang::XMultiComponentFactory > const &
        serviceManager,
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    std::vector<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > provs;
    for (sal_Int32 i = 0; i != -1;) {
        rtl::OUString uri(uris.getToken(0, ' ', i));
        if (uri.isEmpty()) {
            continue;
        }
        bool optional;
        bool directory;
        cppu::decodeRdbUri(&uri, &optional, &directory);
        if (directory) {
            readRdbDirectory(uri, optional, serviceManager, context, &provs);
        } else {
            readRdbFile(uri, optional, serviceManager, context, &provs);
        }
    }
    css::uno::Sequence<
        css::uno::Reference< css::container::XHierarchicalNameAccess > > provs2(
            static_cast< sal_Int32 >(provs.size())); //TODO: check overflow
    std::vector<
        css::uno::Reference<
            css::container::XHierarchicalNameAccess > >::iterator i(
                provs.begin());
    for (sal_Int32 j = 0; j != provs2.getLength(); ++j) {
        provs2[j] = *i++;
    }
    return provs2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
