/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <typeinfo>

#define __USING_WASM_EXCEPTIONS__
#include <cxxabi.h>

#include <bridges/emscriptencxxabi/cxxabi.hxx>
#include <config_cxxabi.h>
#include <uno/any2.h>
#include <uno/mapping.h>

// <https://github.com/emscripten-core/emscripten/>, system/lib/libcxxabi/src/private_typeinfo.h:
namespace __cxxabiv1
{
class _LIBCXXABI_TYPE_VIS __shim_type_info : public std::type_info
{
public:
    /*MODIFIED:*/ __shim_type_info(char const* name)
        : type_info(name)
    {
    }
    _LIBCXXABI_HIDDEN virtual ~__shim_type_info();

    _LIBCXXABI_HIDDEN virtual void noop1() const;
    _LIBCXXABI_HIDDEN virtual void noop2() const;
    _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info* thrown_type,
                                             void*& adjustedPtr) const = 0;
};
}

#if !HAVE_CXXABI_H_CLASS_TYPE_INFO
// <https://github.com/emscripten-core/emscripten/>, system/lib/libcxxabi/src/private_typeinfo.h:
namespace __cxxabiv1
{
class _LIBCXXABI_TYPE_VIS __class_type_info : public __shim_type_info
{
public:
    /*MODIFIED:*/ __class_type_info(char const* name)
        : __shim_type_info(name)
    {
    }
    _LIBCXXABI_HIDDEN virtual ~__class_type_info();

    _LIBCXXABI_HIDDEN void process_static_type_above_dst(void /*MODIFIED: __dynamic_cast_info*/*,
                                                         const void*, const void*, int) const;
    _LIBCXXABI_HIDDEN void process_static_type_below_dst(void /*MODIFIED: __dynamic_cast_info*/*,
                                                         const void*, int) const;
    _LIBCXXABI_HIDDEN void process_found_base_class(void /*MODIFIED: __dynamic_cast_info*/*, void*,
                                                    int) const;
    _LIBCXXABI_HIDDEN virtual void search_above_dst(void /*MODIFIED: __dynamic_cast_info*/*,
                                                    const void*, const void*, int, bool) const;
    _LIBCXXABI_HIDDEN virtual void search_below_dst(void /*MODIFIED: __dynamic_cast_info*/*,
                                                    const void*, int, bool) const;
    _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info*, void*&) const;
    _LIBCXXABI_HIDDEN virtual void
    has_unambiguous_public_base(void /*MODIFIED: __dynamic_cast_info*/*, void*, int) const;
};
}
#endif

#if !HAVE_CXXABI_H_SI_CLASS_TYPE_INFO
// <https://mentorembedded.github.io/cxx-abi/abi.html>,
// libstdc++-v3/libsupc++/cxxabi.h:
namespace __cxxabiv1
{
class _LIBCXXABI_TYPE_VIS __si_class_type_info : public __class_type_info
{
public:
    const __class_type_info* __base_type;

    /*MODIFIED:*/ __si_class_type_info(char const* name)
        : __class_type_info(name)
    {
    }
    _LIBCXXABI_HIDDEN virtual ~__si_class_type_info();

    _LIBCXXABI_HIDDEN virtual void search_above_dst(void /*MODIFIED: __dynamic_cast_info*/*,
                                                    const void*, const void*, int, bool) const;
    _LIBCXXABI_HIDDEN virtual void search_below_dst(void /*MODIFIED: __dynamic_cast_info*/*,
                                                    const void*, int, bool) const;
    _LIBCXXABI_HIDDEN virtual void
    has_unambiguous_public_base(void /*MODIFIED: __dynamic_cast_info*/*, void*, int) const;
};
}
#endif

#if !HAVE_CXXABI_H_CXA_EH_GLOBALS
// <https://github.com/emscripten-core/emscripten/>, system/lib/libcxxabi/src/cxa_exception.h:
namespace __cxxabiv1
{
struct __cxa_eh_globals
{
    __cxa_exception* caughtExceptions;
    unsigned int uncaughtExceptions;
#if defined _LIBCXXABI_ARM_EHABI
    __cxa_exception* propagatingExceptions;
#endif
};
}
#endif

#if !HAVE_CXXABI_H_CXA_GET_GLOBALS
// <https://github.com/emscripten-core/emscripten/>, system/lib/libcxxabi/src/cxa_exception.h:
namespace __cxxabiv1
{
extern "C" __cxa_eh_globals* __cxa_get_globals();
}
#endif

namespace abi_wasm
{
void mapException(__cxxabiv1::__cxa_exception* exception, std::type_info const* type, uno_Any* any,
                  uno_Mapping* mapping);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
