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

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <typeinfo>

#include <cxxabi.h>
#include <dlfcn.h>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/genfunc.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <typelib/typeclass.h>
#include <typelib/typedescription.h>
#include <uno/any2.h>
#include <uno/mapping.h>

#include "abi.hxx"
#include <osl/mutex.hxx>
#include <unordered_map>

namespace {

OUString toUnoName(char const * name) {
    assert(name != 0);
    OUStringBuffer b;
    bool scoped = *name == 'N';
    if (scoped) {
        ++name;
    }
    for (;;) {
        assert(*name >= '0' && *name <= '9');
        std::size_t n = *name++ - '0';
        while (*name >= '0' && *name <= '9') {
            n = 10 * n + (*name++ - '0');
        }
        b.appendAscii(name, n);
        name += n;
        if (!scoped) {
            assert(*name == 0);
            break;
        }
        if (*name == 'E') {
            assert(name[1] == 0);
            break;
        }
        b.append('.');
    }
    return b.makeStringAndClear();
}

class Rtti {
public:
    Rtti(): app_(dlopen(0, RTLD_LAZY)) {}

    ~Rtti() { dlclose(app_); }

    std::type_info * getRtti(typelib_TypeDescription const & type);

private:
    typedef std::unordered_map<OUString, std::type_info *> Map;

    void * app_;

    osl::Mutex mutex_;
    Map map_;
};

std::type_info * Rtti::getRtti(typelib_TypeDescription const & type) {
    OUString unoName(type.pTypeName);
    osl::MutexGuard g(mutex_);
    Map::iterator i(map_.find(unoName));
    if (i == map_.end()) {
        OStringBuffer b;
        b.append("_ZTIN");
        for (sal_Int32 j = 0; j != -1;) {
            OString t(
                OUStringToOString(
                    unoName.getToken(0, '.', j), RTL_TEXTENCODING_ASCII_US));
            b.append(t.getLength());
            b.append(t);
        }
        b.append('E');
        OString sym(b.makeStringAndClear());
        std::type_info * rtti = static_cast<std::type_info *>(
            dlsym(app_, sym.getStr()));
        if (rtti == 0) {
            char const * rttiName = sym.getStr() + std::strlen("_ZTI");
            assert(type.eTypeClass == typelib_TypeClass_EXCEPTION);
            typelib_CompoundTypeDescription const & ctd
                = reinterpret_cast<typelib_CompoundTypeDescription const &>(
                    type);
            if (ctd.pBaseTypeDescription == 0) {
                rtti = new __cxxabiv1::__class_type_info(strdup(rttiName));
            } else {
                std::type_info * base = getRtti(
                    ctd.pBaseTypeDescription->aBase);
                rtti = new __cxxabiv1::__si_class_type_info(
                    strdup(rttiName),
                    static_cast<__cxxabiv1::__class_type_info *>(base));
            }
        }
        i = map_.insert(Map::value_type(unoName, rtti)).first;
    }
    return i->second;
}

struct theRttiFactory: public rtl::Static<Rtti, theRttiFactory> {};

std::type_info * getRtti(typelib_TypeDescription const & type) {
    return theRttiFactory::get().getRtti(type);
}

extern "C" void _GLIBCXX_CDTOR_CALLABI deleteException(void * exception) {
    abi_aarch64::__cxa_exception * header =
        static_cast<abi_aarch64::__cxa_exception *>(exception) - 1;
    OUString unoName(toUnoName(header->exceptionType->name()));
    typelib_TypeDescription * td = 0;
    typelib_typedescription_getByName(&td, unoName.pData);
    assert(td != 0);
    uno_destructData(exception, td, &css::uno::cpp_release);
    typelib_typedescription_release(td);
}

enum StructKind {
    STRUCT_KIND_EMPTY, STRUCT_KIND_FLOAT, STRUCT_KIND_DOUBLE, STRUCT_KIND_POD,
    STRUCT_KIND_DTOR
};

StructKind getStructKind(typelib_CompoundTypeDescription const * type) {
    StructKind k = type->pBaseTypeDescription == 0
        ? STRUCT_KIND_EMPTY : getStructKind(type->pBaseTypeDescription);
    for (sal_Int32 i = 0; i != type->nMembers; ++i) {
        StructKind k2 = StructKind();
        switch (type->ppTypeRefs[i]->eTypeClass) {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_ENUM:
            k2 = STRUCT_KIND_POD;
            break;
        case typelib_TypeClass_FLOAT:
            k2 = STRUCT_KIND_FLOAT;
            break;
        case typelib_TypeClass_DOUBLE:
            k2 = STRUCT_KIND_DOUBLE;
            break;
        case typelib_TypeClass_STRING:
        case typelib_TypeClass_TYPE:
        case typelib_TypeClass_ANY:
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_INTERFACE:
            k2 = STRUCT_KIND_DTOR;
            break;
        case typelib_TypeClass_STRUCT:
            {
                typelib_TypeDescription * td = 0;
                TYPELIB_DANGER_GET(&td, type->ppTypeRefs[i]);
                k2 = getStructKind(
                    reinterpret_cast<typelib_CompoundTypeDescription const *>(
                        td));
                TYPELIB_DANGER_RELEASE(td);
                break;
            }
        default:
            assert(false);
        }
        switch (k2) {
        case STRUCT_KIND_EMPTY:
            // this means an empty sub-object, which nevertheless obtains a byte
            // of storage (TODO: does it?), so the full object cannot be a
            // homogenous collection of float or double
        case STRUCT_KIND_POD:
            assert(k != STRUCT_KIND_DTOR);
            k = STRUCT_KIND_POD;
            break;
        case STRUCT_KIND_FLOAT:
        case STRUCT_KIND_DOUBLE:
            if (k == STRUCT_KIND_EMPTY) {
                k = k2;
            } else if (k != k2) {
                assert(k != STRUCT_KIND_DTOR);
                k = STRUCT_KIND_POD;
            }
            break;
        case STRUCT_KIND_DTOR:
            return STRUCT_KIND_DTOR;
        }
    }
    return k;
}

}

namespace abi_aarch64 {

void mapException(
    __cxa_exception * exception, std::type_info const * type, uno_Any * any, uno_Mapping * mapping)
{
    assert(exception != 0);
    assert(type != nullptr);
    OUString unoName(toUnoName(type->name()));
    typelib_TypeDescription * td = 0;
    typelib_typedescription_getByName(&td, unoName.pData);
    if (td == 0) {
        css::uno::RuntimeException e("exception type not found: " + unoName);
        uno_type_any_constructAndConvert(
            any, &e,
            cppu::UnoType<css::uno::RuntimeException>::get().getTypeLibType(),
            mapping);
    } else {
        uno_any_constructAndConvert(any, exception->adjustedPtr, td, mapping);
        typelib_typedescription_release(td);
    }
}

void raiseException(uno_Any * any, uno_Mapping * mapping) {
    typelib_TypeDescription * td = 0;
    TYPELIB_DANGER_GET(&td, any->pType);
    if (td == 0) {
        throw css::uno::RuntimeException(
            "no typedescription for " + OUString(any->pType->pTypeName));
    }
    void * exc = __cxxabiv1::__cxa_allocate_exception(td->nSize);
    uno_copyAndConvertData(exc, any->pData, td, mapping);
    uno_any_destruct(any, 0);
    std::type_info * rtti = getRtti(*td);
    TYPELIB_DANGER_RELEASE(td);
    __cxxabiv1::__cxa_throw(exc, rtti, deleteException);
}

ReturnKind getReturnKind(typelib_TypeDescription const * type) {
    switch (type->eTypeClass) {
    default:
        assert(false);
        // fall through to avoid warnings
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
    case typelib_TypeClass_ENUM:
        assert(type->nSize <= 16);
        return RETURN_KIND_REG;
    case typelib_TypeClass_STRING:
    case typelib_TypeClass_TYPE:
    case typelib_TypeClass_ANY:
    case typelib_TypeClass_SEQUENCE:
    case typelib_TypeClass_INTERFACE:
        return RETURN_KIND_INDIRECT;
    case typelib_TypeClass_STRUCT:
        if (type->nSize > 16) {
            return RETURN_KIND_INDIRECT;
        }
        switch (getStructKind(
                    reinterpret_cast<typelib_CompoundTypeDescription const *>(
                        type)))
        {
        case STRUCT_KIND_FLOAT:
            return RETURN_KIND_HFA_FLOAT;
        case STRUCT_KIND_DOUBLE:
            return RETURN_KIND_HFA_DOUBLE;
        case STRUCT_KIND_DTOR:
            return RETURN_KIND_INDIRECT;
        default:
            return RETURN_KIND_REG;
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
