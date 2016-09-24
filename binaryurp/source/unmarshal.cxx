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

#include "sal/config.h"

#include <cassert>
#include <cstdlib>
#include <new>
#include <vector>

#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppu/unotype.hxx"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"
#include "typelib/typedescription.hxx"
#include "uno/any2.h"
#include "uno/data.h"
#include "uno/dispatcher.hxx"

#include "binaryany.hxx"
#include "bridge.hxx"
#include "cache.hxx"
#include "readerstate.hxx"
#include "unmarshal.hxx"

namespace binaryurp {

namespace {

void * allocate(sal_Size size) {
    void * p = rtl_allocateMemory(size);
    if (p == nullptr) {
        throw std::bad_alloc();
    }
    return p;
}

std::vector< BinaryAny >::iterator copyMemberValues(
    css::uno::TypeDescription const & type,
    std::vector< BinaryAny >::iterator const & it, void * buffer) throw ()
{
    assert(
        type.is() &&
        (type.get()->eTypeClass == typelib_TypeClass_STRUCT ||
         type.get()->eTypeClass == typelib_TypeClass_EXCEPTION) &&
        buffer != nullptr);
    type.makeComplete();
    std::vector< BinaryAny >::iterator i(it);
    typelib_CompoundTypeDescription * ctd =
        reinterpret_cast< typelib_CompoundTypeDescription * >(type.get());
    if (ctd->pBaseTypeDescription != nullptr) {
        i = copyMemberValues(
            css::uno::TypeDescription(&ctd->pBaseTypeDescription->aBase), i,
            buffer);
    }
    for (sal_Int32 j = 0; j != ctd->nMembers; ++j) {
        uno_type_copyData(
            static_cast< char * >(buffer) + ctd->pMemberOffsets[j],
            i++->getValue(css::uno::TypeDescription(ctd->ppTypeRefs[j])),
            ctd->ppTypeRefs[j], nullptr);
    }
    return i;
}

}

Unmarshal::Unmarshal(
    rtl::Reference< Bridge > const & bridge, ReaderState & state,
    css::uno::Sequence< sal_Int8 > const & buffer):
    bridge_(bridge), state_(state), buffer_(buffer)
{
    data_ = reinterpret_cast< sal_uInt8 const * >(buffer_.getConstArray());
    end_ = data_ + buffer_.getLength();
}

Unmarshal::~Unmarshal() {}

sal_uInt8 Unmarshal::read8() {
    check(1);
    return *data_++;
}

sal_uInt16 Unmarshal::read16() {
    check(2);
    sal_uInt16 n = static_cast< sal_uInt16 >(*data_++) << 8;
    return n | *data_++;
}

sal_uInt32 Unmarshal::read32() {
    check(4);
    sal_uInt32 n = static_cast< sal_uInt32 >(*data_++) << 24;
    n |= static_cast< sal_uInt32 >(*data_++) << 16;
    n |= static_cast< sal_uInt32 >(*data_++) << 8;
    return n | *data_++;
}

css::uno::TypeDescription Unmarshal::readType() {
    sal_uInt8 flags = read8();
    typelib_TypeClass tc = static_cast< typelib_TypeClass >(flags & 0x7F);
    switch (tc) {
    case typelib_TypeClass_VOID:
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_DOUBLE:
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_STRING:
    case typelib_TypeClass_TYPE:
    case typelib_TypeClass_ANY:
        if ((flags & 0x80) != 0) {
            throw css::io::IOException(
                "binaryurp::Unmarshal: cache flag of simple type is set");
        }
        return css::uno::TypeDescription(
            *typelib_static_type_getByTypeClass(
                static_cast< typelib_TypeClass >(tc)));
    case typelib_TypeClass_SEQUENCE:
    case typelib_TypeClass_ENUM:
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    case typelib_TypeClass_INTERFACE:
        {
            sal_uInt16 idx = readCacheIndex();
            if ((flags & 0x80) == 0) {
                if (idx == cache::ignore || !state_.typeCache[idx].is()) {
                    throw css::io::IOException(
                        "binaryurp::Unmarshal: unknown type cache index");
                }
                return state_.typeCache[idx];
            } else {
                OUString const str(readString());
                css::uno::TypeDescription t(str);
                if (!t.is() ||
                    t.get()->eTypeClass != static_cast< typelib_TypeClass >(tc))
                {

                    throw css::io::IOException(
                        "binaryurp::Unmarshal: type with unknown name: " + str);
                }
                for (css::uno::TypeDescription t2(t);
                     t2.get()->eTypeClass == typelib_TypeClass_SEQUENCE;)
                {
                    t2.makeComplete();
                    t2 = css::uno::TypeDescription(
                        reinterpret_cast< typelib_IndirectTypeDescription * >(
                            t2.get())->pType);
                    if (!t2.is()) {
                        throw css::io::IOException(
                            "binaryurp::Unmarshal: sequence type with unknown"
                            " component type");
                    }
                    switch (t2.get()->eTypeClass) {
                    case typelib_TypeClass_VOID:
                    case typelib_TypeClass_EXCEPTION:
                        throw css::io::IOException(
                            "binaryurp::Unmarshal: sequence type with bad"
                            " component type");
                    default:
                        break;
                    }
                }
                if (idx != cache::ignore) {
                    state_.typeCache[idx] = t;
                }
                return t;
            }
        }
    default:
        throw css::io::IOException(
            "binaryurp::Unmarshal: type of unknown type class");
    }
}

OUString Unmarshal::readOid() {
    OUString oid(readString());
    for (sal_Int32 i = 0; i != oid.getLength(); ++i) {
        if (oid[i] > 0x7F) {
            throw css::io::IOException(
                "binaryurp::Unmarshal: OID contains non-ASCII character");
        }
    }
    sal_uInt16 idx = readCacheIndex();
    if (oid.isEmpty() && idx != cache::ignore) {
        if (state_.oidCache[idx].isEmpty()) {
            throw css::io::IOException(
                "binaryurp::Unmarshal: unknown OID cache index");
        }
        return state_.oidCache[idx];
    }
    if (idx != cache::ignore) {
        state_.oidCache[idx] = oid;
    }
    return oid;
}

rtl::ByteSequence Unmarshal::readTid() {
    rtl::ByteSequence tid(
        *static_cast< sal_Sequence * const * >(
            readSequence(
                css::uno::TypeDescription(
                    cppu::UnoType< css::uno::Sequence< sal_Int8 > >::get())).
            getValue(
                css::uno::TypeDescription(
                    cppu::UnoType< css::uno::Sequence< sal_Int8 > >::get()))));
    sal_uInt16 idx = readCacheIndex();
    if (tid.getLength() == 0) {
        if (idx == cache::ignore || state_.tidCache[idx].getLength() == 0) {
            throw css::io::IOException(
                "binaryurp::Unmarshal: unknown TID cache index");
        }
        return state_.tidCache[idx];
    }
    if (idx != cache::ignore) {
        state_.tidCache[idx] = tid;
    }
    return tid;
}

BinaryAny Unmarshal::readValue(css::uno::TypeDescription const & type) {
    assert(type.is());
    switch (type.get()->eTypeClass) {
    default:
        std::abort(); // this cannot happen
        // pseudo fall-through to avoid compiler warnings
    case typelib_TypeClass_VOID:
        return BinaryAny();
    case typelib_TypeClass_BOOLEAN:
        {
            sal_uInt8 v = read8();
            if (v > 1) {
                throw css::io::IOException(
                    "binaryurp::Unmarshal: boolean of unknown value");
            }
            return BinaryAny(type, &v);
        }
    case typelib_TypeClass_BYTE:
        {
            sal_uInt8 v = read8();
            return BinaryAny(type, &v);
        }
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_CHAR:
        {
            sal_uInt16 v = read16();
            return BinaryAny(type, &v);
        }
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_FLOAT:
        {
            sal_uInt32 v = read32();
            return BinaryAny(type, &v);
        }
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    case typelib_TypeClass_DOUBLE:
        {
            sal_uInt64 v = read64();
            return BinaryAny(type, &v);
        }
    case typelib_TypeClass_STRING:
        {
            OUString v(readString());
            return BinaryAny(type, &v.pData);
        }
    case typelib_TypeClass_TYPE:
        {
            css::uno::TypeDescription v(readType());
            typelib_TypeDescription * p = v.get();
            return BinaryAny(type, &p);
        }
    case typelib_TypeClass_ANY:
        {
            css::uno::TypeDescription t(readType());
            if (t.get()->eTypeClass == typelib_TypeClass_ANY) {
                throw css::io::IOException(
                    "binaryurp::Unmarshal: any of type ANY");
            }
            return readValue(t);
        }
    case typelib_TypeClass_SEQUENCE:
        type.makeComplete();
        return readSequence(type);
    case typelib_TypeClass_ENUM:
        {
            sal_Int32 v = static_cast< sal_Int32 >(read32());
            type.makeComplete();
            typelib_EnumTypeDescription * etd =
                reinterpret_cast< typelib_EnumTypeDescription * >(type.get());
            bool bFound = false;
            for (sal_Int32 i = 0; i != etd->nEnumValues; ++i) {
                if (etd->pEnumValues[i] == v) {
                    bFound = true;
                    break;
                }
            }
            if (!bFound) {
                throw css::io::IOException(
                    "binaryurp::Unmarshal: unknown enum value");
            }
            return BinaryAny(type, &v);
        }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        {
            std::vector< BinaryAny > as;
            readMemberValues(type, &as);
            void * buf = allocate(type.get()->nSize);
            copyMemberValues(type, as.begin(), buf);
            uno_Any raw;
            raw.pType = reinterpret_cast< typelib_TypeDescriptionReference * >(
                type.get());
            raw.pData = buf;
            raw.pReserved = nullptr;
            return BinaryAny(raw);
        }
    case typelib_TypeClass_INTERFACE:
        {
            css::uno::UnoInterfaceReference obj(
                bridge_->registerIncomingInterface(readOid(), type));
            return BinaryAny(type, &obj.m_pUnoI);
        }
    }
}

void Unmarshal::done() const {
    if (data_ != end_) {
        throw css::io::IOException(
            "binaryurp::Unmarshal: block contains excess data");
    }
}

void Unmarshal::check(sal_Int32 size) const {
    if (end_ - data_ < size) {
        throw css::io::IOException(
            "binaryurp::Unmarshal: trying to read past end of block");
    }
}

sal_uInt32 Unmarshal::readCompressed() {
    sal_uInt8 n = read8();
    return n == 0xFF ? read32() : n;
}

sal_uInt16 Unmarshal::readCacheIndex() {
    sal_uInt16 idx = read16();
    if (idx >= cache::size && idx != cache::ignore) {
        throw css::io::IOException(
            "binaryurp::Unmarshal: cache index out of range");
    }
    return idx;
}

sal_uInt64 Unmarshal::read64() {
    check(8);
    sal_uInt64 n = static_cast< sal_uInt64 >(*data_++) << 56;
    n |= static_cast< sal_uInt64 >(*data_++) << 48;
    n |= static_cast< sal_uInt64 >(*data_++) << 40;
    n |= static_cast< sal_uInt64 >(*data_++) << 32;
    n |= static_cast< sal_uInt64 >(*data_++) << 24;
    n |= static_cast< sal_uInt64 >(*data_++) << 16;
    n |= static_cast< sal_uInt64 >(*data_++) << 8;
    return n | *data_++;
}

OUString Unmarshal::readString() {
    sal_uInt32 n = readCompressed();
    if (n > SAL_MAX_INT32) {
        throw css::uno::RuntimeException(
            "binaryurp::Unmarshal: string size too large");
    }
    check(static_cast< sal_Int32 >(n));
    OUString s;
    if (!rtl_convertStringToUString(
            &s.pData, reinterpret_cast< char const * >(data_),
            static_cast< sal_Int32 >(n), RTL_TEXTENCODING_UTF8,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw css::io::IOException(
            "binaryurp::Unmarshal: string does not contain UTF-8");
    }
    data_ += n;
    return s;
}

BinaryAny Unmarshal::readSequence(css::uno::TypeDescription const & type) {
    assert(type.is() && type.get()->eTypeClass == typelib_TypeClass_SEQUENCE);
    sal_uInt32 n = readCompressed();
    if (n > SAL_MAX_INT32) {
        throw css::uno::RuntimeException(
            "binaryurp::Unmarshal: sequence size too large");
    }
    if (n == 0) {
        return BinaryAny(type, nullptr);
    }
    css::uno::TypeDescription ctd(
        reinterpret_cast< typelib_IndirectTypeDescription * >(
            type.get())->pType);
    if (ctd.get()->eTypeClass == typelib_TypeClass_BYTE) {
        check(static_cast< sal_Int32 >(n));
        rtl::ByteSequence s(
            reinterpret_cast< sal_Int8 const * >(data_),
            static_cast< sal_Int32 >(n));
        data_ += n;
        sal_Sequence * p = s.getHandle();
        return BinaryAny(type, &p);
    }
    std::vector< BinaryAny > as;
    for (sal_uInt32 i = 0; i != n; ++i) {
        as.push_back(readValue(ctd));
    }
    assert(ctd.get()->nSize >= 0);
    sal_uInt64 size = static_cast< sal_uInt64 >(n) *
        static_cast< sal_uInt64 >(ctd.get()->nSize);
        // sal_uInt32 * sal_Int32 -> sal_uInt64 cannot overflow
    if (size > SAL_MAX_SIZE - SAL_SEQUENCE_HEADER_SIZE) {
        throw css::uno::RuntimeException(
            "binaryurp::Unmarshal: sequence size too large");
    }
    void * buf = allocate(
        SAL_SEQUENCE_HEADER_SIZE + static_cast< sal_Size >(size));
    static_cast< sal_Sequence * >(buf)->nRefCount = 0;
    static_cast< sal_Sequence * >(buf)->nElements =
        static_cast< sal_Int32 >(n);
    for (sal_uInt32 i = 0; i != n; ++i) {
        uno_copyData(
            static_cast< sal_Sequence * >(buf)->elements + i * ctd.get()->nSize,
            as[i].getValue(ctd), ctd.get(), nullptr);
    }
    return BinaryAny(type, &buf);
}

void Unmarshal::readMemberValues(
    css::uno::TypeDescription const & type, std::vector< BinaryAny > * values)
{
    assert(
        type.is() &&
        (type.get()->eTypeClass == typelib_TypeClass_STRUCT ||
         type.get()->eTypeClass == typelib_TypeClass_EXCEPTION) &&
        values != nullptr);
    type.makeComplete();
    typelib_CompoundTypeDescription * ctd =
        reinterpret_cast< typelib_CompoundTypeDescription * >(type.get());
    if (ctd->pBaseTypeDescription != nullptr) {
        readMemberValues(
            css::uno::TypeDescription(&ctd->pBaseTypeDescription->aBase),
            values);
    }
    for (sal_Int32 i = 0; i != ctd->nMembers; ++i) {
        values->push_back(
            readValue(css::uno::TypeDescription(ctd->ppTypeRefs[i])));
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
