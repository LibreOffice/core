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
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppu/unotype.hxx"
#include "rtl/byteseq.hxx"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"
#include "typelib/typedescription.hxx"
#include "uno/dispatcher.hxx"

#include "binaryany.hxx"
#include "bridge.hxx"
#include "cache.hxx"
#include "lessoperators.hxx"
#include "marshal.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

void write64(std::vector< unsigned char > * buffer, sal_uInt64 value) {
    Marshal::write8(buffer, value >> 56);
    Marshal::write8(buffer, (value >> 48) & 0xFF);
    Marshal::write8(buffer, (value >> 40) & 0xFF);
    Marshal::write8(buffer, (value >> 32) & 0xFF);
    Marshal::write8(buffer, (value >> 24) & 0xFF);
    Marshal::write8(buffer, (value >> 16) & 0xFF);
    Marshal::write8(buffer, (value >> 8) & 0xFF);
    Marshal::write8(buffer, value & 0xFF);
}

void writeCompressed(std::vector< unsigned char > * buffer, sal_uInt32 value) {
    if (value < 0xFF) {
        Marshal::write8(buffer, static_cast< sal_uInt8 >(value));
    } else {
        Marshal::write8(buffer, 0xFF);
        Marshal::write32(buffer, value);
    }
}

void writeString(
    std::vector< unsigned char > * buffer, OUString const & value)
{
    assert(buffer != 0);
    rtl::OString v;
    if (!value.convertToString(
            &v, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            "UNO string contains invalid UTF-16 sequence",
            css::uno::Reference< css::uno::XInterface >());
    }
    writeCompressed(buffer, static_cast< sal_uInt32 >(v.getLength()));
    buffer->insert(buffer->end(), v.getStr(), v.getStr() + v.getLength());
}

}

Marshal::Marshal(rtl::Reference< Bridge > const & bridge, WriterState & state):
    bridge_(bridge), state_(state)
{
    assert(bridge.is());
}

Marshal::~Marshal() {}

void Marshal::write8(std::vector< unsigned char > * buffer, sal_uInt8 value) {
    assert(buffer != 0);
    buffer->push_back(value);
}

void Marshal::write16(std::vector< unsigned char > * buffer, sal_uInt16 value) {
    write8(buffer, value >> 8);
    write8(buffer, value & 0xFF);
}

void Marshal::write32(std::vector< unsigned char > * buffer, sal_uInt32 value) {
    write8(buffer, value >> 24);
    write8(buffer, (value >> 16) & 0xFF);
    write8(buffer, (value >> 8) & 0xFF);
    write8(buffer, value & 0xFF);
}

void Marshal::writeValue(
    std::vector< unsigned char > * buffer,
    css::uno::TypeDescription const & type, BinaryAny const & value)
{
    assert(
        type.is() &&
        (type.get()->eTypeClass == typelib_TypeClass_ANY ||
         value.getType().equals(type)));
    writeValue(buffer, type, value.getValue(type));
}

void Marshal::writeType(
    std::vector< unsigned char > * buffer,
    css::uno::TypeDescription const & value)
{
    value.makeComplete();
    assert(value.is());
    typelib_TypeClass tc = value.get()->eTypeClass;
    if (tc <= typelib_TypeClass_ANY) {
        write8(buffer, static_cast< sal_uInt8 >(tc));
    } else {
        bool found;
        sal_uInt16 idx = state_.typeCache.add(value, &found);
        if (found) {
            write8(buffer, static_cast< sal_uInt8 >(tc));
            write16(buffer, idx);
        } else {
            write8(buffer, static_cast< sal_uInt8 >(tc) | 0x80);
            write16(buffer, idx);
            writeString(buffer, OUString(value.get()->pTypeName));
        }
    }
}

void Marshal::writeOid(
    std::vector< unsigned char > * buffer, OUString const & oid)
{
    bool found;
    sal_uInt16 idx;
    if (oid.getLength() == 0) {
        found = true;
        idx = cache::ignore;
    } else {
        idx = state_.oidCache.add(oid, &found);
    }
    if (found) {
        write8(buffer, 0);
    } else {
        writeString(buffer, oid);
    }
    write16(buffer, idx);
}

void Marshal::writeTid(
    std::vector< unsigned char > * buffer, rtl::ByteSequence const & tid)
{
    bool found;
    sal_uInt16 idx = state_.tidCache.add(tid, &found);
    if (found) {
        write8(buffer, 0);
    } else {
        sal_Sequence * p = tid.getHandle();
        writeValue(
            buffer,
            css::uno::TypeDescription(
                cppu::UnoType< css::uno::Sequence< sal_Int8 > >::get()), &p);
    }
    write16(buffer, idx);
}

void Marshal::writeValue(
    std::vector< unsigned char > * buffer,
    css::uno::TypeDescription const & type, void const * value)
{
    assert(buffer != 0 && type.is());
    type.makeComplete();
    switch (type.get()->eTypeClass) {
    case typelib_TypeClass_VOID:
        break;
    case typelib_TypeClass_BOOLEAN:
        assert(*static_cast< sal_uInt8 const * >(value) <= 1);
        // fall through
    case typelib_TypeClass_BYTE:
        write8(buffer, *static_cast< sal_uInt8 const * >(value));
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_CHAR:
        write16(buffer, *static_cast< sal_uInt16 const * >(value));
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_ENUM:
        write32(buffer, *static_cast< sal_uInt32 const * >(value));
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    case typelib_TypeClass_DOUBLE:
        write64(buffer, *static_cast< sal_uInt64 const * >(value));
        break;
    case typelib_TypeClass_STRING:
        writeString(
            buffer,
            OUString(*static_cast< rtl_uString * const * >(value)));
        break;
    case typelib_TypeClass_TYPE:
        writeType(
            buffer,
            css::uno::TypeDescription(
                *static_cast< typelib_TypeDescriptionReference * const * >(
                    value)));
        break;
    case typelib_TypeClass_ANY:
        {
            uno_Any const * p = static_cast< uno_Any const * >(value);
            css::uno::TypeDescription t(p->pType);
            writeType(buffer, t);
            writeValue(buffer, t, p->pData);
            break;
        }
    case typelib_TypeClass_SEQUENCE:
        {
            sal_Sequence * p = *static_cast< sal_Sequence * const * >(value);
            writeCompressed(buffer, static_cast< sal_uInt32 >(p->nElements));
            css::uno::TypeDescription ctd(
                reinterpret_cast< typelib_IndirectTypeDescription * >(
                    type.get())->
                pType);
            assert(ctd.is());
            if (ctd.get()->eTypeClass == typelib_TypeClass_BYTE) {
                buffer->insert(
                    buffer->end(), p->elements, p->elements + p->nElements);
            } else {
                for (sal_Int32 i = 0; i != p->nElements; ++i) {
                    writeValue(buffer, ctd, p->elements + i * ctd.get()->nSize);
                }
            }
            break;
        }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        writeMemberValues(buffer, type, value);
        break;
    case typelib_TypeClass_INTERFACE:
        writeOid(
            buffer,
            bridge_->registerOutgoingInterface(
                css::uno::UnoInterfaceReference(
                    *static_cast< uno_Interface * const * >(value)),
                type));
        break;
    default:
        assert(false); // this cannot happen
        break;
    }
}

void Marshal::writeMemberValues(
    std::vector< unsigned char > * buffer,
    css::uno::TypeDescription const & type, void const * aggregateValue)
{
    assert(
        type.is() &&
        (type.get()->eTypeClass == typelib_TypeClass_STRUCT ||
         type.get()->eTypeClass == typelib_TypeClass_EXCEPTION) &&
        aggregateValue != 0);
    type.makeComplete();
    typelib_CompoundTypeDescription * ctd =
        reinterpret_cast< typelib_CompoundTypeDescription * >(type.get());
    if (ctd->pBaseTypeDescription != 0) {
        writeMemberValues(
            buffer,
            css::uno::TypeDescription(&ctd->pBaseTypeDescription->aBase),
            aggregateValue);
    }
    for (sal_Int32 i = 0; i != ctd->nMembers; ++i) {
        writeValue(
            buffer, css::uno::TypeDescription(ctd->ppTypeRefs[i]),
            (static_cast< char const * >(aggregateValue) +
             ctd->pMemberOffsets[i]));
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
