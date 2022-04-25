/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <cstdint>
#include <cstring>
#include <typeinfo>

#include <dlfcn.h>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/genfunc.h>
#include <o3tl/string_view.hxx>
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
    assert(name != nullptr);
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
    Rtti(): app_(dlopen(nullptr, RTLD_LAZY)) {}

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
                    o3tl::getToken(unoName, 0, '.', j), RTL_TEXTENCODING_ASCII_US));
            b.append(t.getLength());
            b.append(t);
        }
        b.append('E');
        OString sym(b.makeStringAndClear());
        std::type_info * rtti = static_cast<std::type_info *>(
            dlsym(app_, sym.getStr()));
        if (rtti == nullptr) {
            char const * rttiName = strdup(sym.getStr() + std::strlen("_ZTI"));
            if (rttiName == nullptr) {
                throw std::bad_alloc();
            }
#if defined MACOSX
            // For the Apple ARM64 ABI, if the most significant ("non-unique RTTI") bit is set, it
            // means that the instance of the name is not unique (and thus RTTI equality needs to be
            // determined by string comparison rather than by pointer comparison):
            rttiName = reinterpret_cast<char const *>(
                reinterpret_cast<std::uintptr_t>(rttiName) | 0x8000'0000'0000'0000);
#endif
            assert(type.eTypeClass == typelib_TypeClass_EXCEPTION);
            typelib_CompoundTypeDescription const & ctd
                = reinterpret_cast<typelib_CompoundTypeDescription const &>(
                    type);
            if (ctd.pBaseTypeDescription == nullptr) {
                rtti = new __cxxabiv1::__class_type_info(rttiName);
            } else {
                std::type_info * base = getRtti(
                    ctd.pBaseTypeDescription->aBase);
                rtti = new __cxxabiv1::__si_class_type_info(
                    rttiName,
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
    __cxxabiv1::__cxa_exception * header =
        static_cast<__cxxabiv1::__cxa_exception *>(exception) - 1;
#if !defined MACOSX && defined _LIBCPPABI_VERSION // detect libc++abi
    // First, the libcxxabi commit
    // <http://llvm.org/viewvc/llvm-project?view=revision&revision=303175>
    // "[libcxxabi] Align unwindHeader on a double-word boundary" towards
    // LLVM 5.0 changed the size of __cxa_exception by adding
    //
    //   __attribute__((aligned))
    //
    // to the final member unwindHeader, on x86-64 effectively adding a hole of
    // size 8 in front of that member (changing its offset from 88 to 96,
    // sizeof(__cxa_exception) from 120 to 128, and alignof(__cxa_exception)
    // from 8 to 16); the "header1" hack below to dynamically determine whether we run against a
    // LLVM 5 libcxxabi is to look at the exceptionDestructor member, which must
    // point to this function (the use of __cxa_exception in mapException is
    // unaffected, as it only accesses members towards the start of the struct,
    // through a pointer known to actually point at the start).  The libcxxabi commit
    // <https://github.com/llvm/llvm-project/commit/9ef1daa46edb80c47d0486148c0afc4e0d83ddcf>
    // "Insert padding before the __cxa_exception header to ensure the thrown" in LLVM 6
    // removes the need for this hack, so the "header1" hack can be removed again once we can be
    // sure that we only run against libcxxabi from LLVM >= 6.
    //
    // Second, the libcxxabi commit
    // <https://github.com/llvm/llvm-project/commit/674ec1eb16678b8addc02a4b0534ab383d22fa77>
    // "[libcxxabi] Insert padding in __cxa_exception struct for compatibility" in LLVM 10 changed
    // the layout of the start of __cxa_exception to
    //
    //  [8 byte  void *reserve]
    //   8 byte  size_t referenceCount
    //
    // so the "header2" hack below to dynamically determine whether we run against a LLVM >= 10
    // libcxxabi is to look whether the exceptionDestructor (with its known value) has increased its
    // offset by 8.  As described in the definition of __cxa_exception
    // (bridges/source/cpp_uno/gcc3_linux_aarch64/abi.hxx), the "header2" hack (together with the
    // "#ifdef MACOSX" in the definition of __cxa_exception and the corresponding hack in call in
    // bridges/source/cpp_uno/gcc3_linux_aarch64/uno2cpp.cxx) can be dropped once we can be sure
    // that we only run against new libcxxabi that has the reserve member.
    if (header->exceptionDestructor != &deleteException) {
        auto const header1 = reinterpret_cast<__cxxabiv1::__cxa_exception *>(
            reinterpret_cast<char *>(header) - 8);
        if (header1->exceptionDestructor == &deleteException) {
            header = header1;
        } else {
            auto const header2 = reinterpret_cast<__cxxabiv1::__cxa_exception *>(
                reinterpret_cast<char *>(header) + 8);
            if (header2->exceptionDestructor == &deleteException) {
                header = header2;
            } else {
                assert(false);
            }
        }
    }
#endif
    assert(header->exceptionDestructor == &deleteException);
    OUString unoName(toUnoName(header->exceptionType->name()));
    typelib_TypeDescription * td = nullptr;
    typelib_typedescription_getByName(&td, unoName.pData);
    assert(td != nullptr);
    uno_destructData(exception, td, &css::uno::cpp_release);
    typelib_typedescription_release(td);
}

enum StructKind {
    STRUCT_KIND_EMPTY, STRUCT_KIND_FLOAT, STRUCT_KIND_DOUBLE, STRUCT_KIND_POD,
    STRUCT_KIND_DTOR
};

StructKind getStructKind(typelib_CompoundTypeDescription const * type) {
    StructKind k = type->pBaseTypeDescription == nullptr
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
                typelib_TypeDescription * td = nullptr;
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
            // homogeneous collection of float or double
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
    __cxxabiv1::__cxa_exception * exception, std::type_info const * type, uno_Any * any, uno_Mapping * mapping)
{
    assert(exception != nullptr);
    assert(type != nullptr);
    OUString unoName(toUnoName(type->name()));
    typelib_TypeDescription * td = nullptr;
    typelib_typedescription_getByName(&td, unoName.pData);
    if (td == nullptr) {
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
    typelib_TypeDescription * td = nullptr;
    TYPELIB_DANGER_GET(&td, any->pType);
    if (td == nullptr) {
        throw css::uno::RuntimeException(
            "no typedescription for " + OUString::unacquired(&any->pType->pTypeName));
    }
    void * exc = __cxxabiv1::__cxa_allocate_exception(td->nSize);
    uno_copyAndConvertData(exc, any->pData, td, mapping);
    uno_any_destruct(any, nullptr);
    std::type_info * rtti = getRtti(*td);
    TYPELIB_DANGER_RELEASE(td);
    __cxxabiv1::__cxa_throw(exc, rtti, deleteException);
}

ReturnKind getReturnKind(typelib_TypeDescription const * type) {
    switch (type->eTypeClass) {
    default:
        assert(false);
#ifdef NDEBUG
        [[fallthrough]];
#endif
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
