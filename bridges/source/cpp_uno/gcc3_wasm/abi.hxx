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
#include <exception>
#include <typeinfo>

#include <cxxabi.h>

#include <config_cxxabi.h>
#include <uno/any2.h>
#include <uno/mapping.h>

#if !HAVE_CXXABI_H_CXA_EXCEPTION

// <https://github.com/emscripten-core/emscripten/>, system/lib/libunwind/include/unwind_itanium.h,
// except where MODIFIED:
typedef std::/*MODIFIED*/ uint64_t _Unwind_Exception_Class;
struct _Unwind_Exception
{
    _Unwind_Exception_Class exception_class;
    void (*exception_cleanup)(/*MODIFIED: _Unwind_Reason_Code reason, _Unwind_Exception* exc*/);
#if defined(__SEH__) && !defined(__USING_SJLJ_EXCEPTIONS__)
    std::/*MODIFIED*/ uintptr_t private_[6];
#elif !defined(__USING_WASM_EXCEPTIONS__)
    std::/*MODIFIED*/ uintptr_t private_1; // non-zero means forced unwind
    std::/*MODIFIED*/ uintptr_t private_2; // holds sp that phase1 found for phase2 to use
#endif
#if __SIZEOF_POINTER__ == 4
    // The implementation of _Unwind_Exception uses an attribute mode on the
    // above fields which has the side effect of causing this whole struct to
    // round up to 32 bytes in size (48 with SEH). To be more explicit, we add
    // pad fields added for binary compatibility.
    std::/*MODIFIED*/ uint32_t reserved[3];
#endif
    // The Itanium ABI requires that _Unwind_Exception objects are "double-word
    // aligned".  GCC has interpreted this to mean "use the maximum useful
    // alignment for the target"; so do we.
} __attribute__((__aligned__));

// <https://github.com/emscripten-core/emscripten/>, system/lib/libcxxabi/src/cxa_exception.h,
// except where MODIFIED:
namespace __cxxabiv1
{
struct __cxa_exception
{
#if defined(__LP64__) || defined(_WIN64) || defined(_LIBCXXABI_ARM_EHABI)
    // Now _Unwind_Exception is marked with __attribute__((aligned)),
    // which implies __cxa_exception is also aligned. Insert padding
    // in the beginning of the struct, rather than before unwindHeader.
    void* reserve;
    // This is a new field to support C++11 exception_ptr.
    // For binary compatibility it is at the start of this
    // struct which is prepended to the object thrown in
    // __cxa_allocate_exception.
    std::/*MODIFIED*/ size_t referenceCount;
#endif
    //  Manage the exception object itself.
    std::type_info* exceptionType;
#if 1 //MODIFIED: #ifdef __USING_WASM_EXCEPTIONS__
    // In wasm, destructors return their argument
    void*(_LIBCXXABI_DTOR_FUNC* exceptionDestructor)(void*);
#else
    void(_LIBCXXABI_DTOR_FUNC* exceptionDestructor)(void*);
#endif
    void* /*MODIFIED: std::unexpected_handler*/ unexpectedHandler;
    std::terminate_handler terminateHandler;
    __cxa_exception* nextException;
    int handlerCount;
#if defined(_LIBCXXABI_ARM_EHABI)
    __cxa_exception* nextPropagatingException;
    int propagationCount;
#else
    int handlerSwitchValue;
    const unsigned char* actionRecord;
    const unsigned char* languageSpecificData;
    void* catchTemp;
    void* adjustedPtr;
#endif
#if !defined(__LP64__) && !defined(_WIN64) && !defined(_LIBCXXABI_ARM_EHABI)
    // This is a new field to support C++11 exception_ptr.
    // For binary compatibility it is placed where the compiler
    // previously added padding to 64-bit align unwindHeader.
    std::/*MODIFIED*/ size_t referenceCount;
#endif
    _Unwind_Exception unwindHeader;
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
