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

#include <cstddef>
#include <cstdint>
#include <typeinfo>

#include <cxxabi.h>

#include <config_cxxabi.h>
#include <uno/any2.h>
#include <uno/mapping.h>

#if !HAVE_CXXABI_H_CXA_EXCEPTION
// <https://github.com/emscripten-core/emscripten/>, system/lib/libcxxabi/src/cxa_exception.h:
namespace __cxxabiv1
{
struct __cxa_exception
{
    std::size_t referenceCount;
    std::type_info* exceptionType;
    // In wasm, destructors return 'this' as in ARM
    void* (*exceptionDestructor)(void*);
    std::uint8_t caught;
    std::uint8_t rethrown;
    void* adjustedPtr;
    // Add padding to ensure that the size of __cxa_exception is a multiple of
    // the maximum useful alignment for the target machine.  This ensures that
    // the thrown object that follows has that correct alignment.
    void* padding;
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
