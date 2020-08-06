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

#include <typeinfo>
#include <exception>
#include <cstddef>

#include <cxxabi.h>
#ifndef _GLIBCXX_CDTOR_CALLABI // new in GCC 4.7 cxxabi.h
#define _GLIBCXX_CDTOR_CALLABI
#endif
#include <unwind.h>

#include <config_cxxabi.h>
#include <uno/any2.h>
#include <uno/mapping.h>

#if !HAVE_CXXABI_H_CLASS_TYPE_INFO
// <https://mentorembedded.github.io/cxx-abi/abi.html>,
// libstdc++-v3/libsupc++/cxxabi.h:
namespace __cxxabiv1 {
class __class_type_info: public std::type_info {
public:
    explicit __class_type_info(char const * n): type_info(n) {}
    ~__class_type_info() override;
};
}
#endif

#if !HAVE_CXXABI_H_SI_CLASS_TYPE_INFO
// <https://mentorembedded.github.io/cxx-abi/abi.html>,
// libstdc++-v3/libsupc++/cxxabi.h:
namespace __cxxabiv1 {
class __si_class_type_info: public __class_type_info {
public:
    __class_type_info const * __base_type;
    explicit __si_class_type_info(
        char const * n, __class_type_info const *base):
        __class_type_info(n), __base_type(base) {}
    ~__si_class_type_info() override;
};
}
#endif

#if !HAVE_CXXABI_H_BASE_CLASS_TYPE_INFO
// <https://mentorembedded.github.io/cxx-abi/abi.html>,
// libstdc++-v3/libsupc++/cxxabi.h:
namespace __cxxabiv1 {
struct __base_class_type_info {
    __class_type_info const * __base_type;
#if defined _GLIBCXX_LLP64
    long long __offset_flags;
#else
    long __offset_flags;
#endif
    enum __offset_flags_masks {
        __virtual_mask = 0x1,
        __public_mask = 0x2,
        __offset_shift = 8
    };
};
}
#endif

#if !HAVE_CXXABI_H_VMI_CLASS_TYPE_INFO
// <https://mentorembedded.github.io/cxx-abi/abi.html>,
// libstdc++-v3/libsupc++/cxxabi.h:
namespace __cxxabiv1 {
class __vmi_class_type_info: public __class_type_info {
public:
    unsigned int __flags;
    unsigned int __base_count;
    __base_class_type_info __base_info[1];
    enum __flags_masks {
        __non_diamond_repeat_mask = 0x1,
        __diamond_shaped_mask = 0x2,
        __flags_unknown_mask = 0x10
    };
    explicit __vmi_class_type_info(char const * n, int flags):
        __class_type_info(n), __flags(flags), __base_count(0) {}
    ~__vmi_class_type_info() override;
};
}
#endif

#if !HAVE_CXXABI_H_CXA_EXCEPTION
// <https://mentorembedded.github.io/cxx-abi/abi-eh.html>,
// libcxxabi/src/cxa_exception.hpp:
namespace __cxxabiv1 {
struct __cxa_exception {
#if defined _LIBCPPABI_VERSION // detect libc++abi
#if defined __LP64__ || LIBCXXABI_ARM_EHABI
#if 0
    // This is a new field added with LLVM 11
    // <https://github.com/llvm/llvm-project/commit/f2a436058fcbc11291e73badb44e243f61046183>
    // "[libcxxabi] Insert padding in __cxa_exception struct for compatibility".  The HACK in
    // fillUnoException (bridges/source/cpp_uno/gcc3_linux_x86-64/except.cxx) tries to find out at
    // runtime whether a __cxa_exception has this member.  Once we can be sure that we only run
    // against new libcxxabi that has this member, we can drop the "#if 0" here and drop the hack
    // in fillUnoException.

    // Now _Unwind_Exception is marked with __attribute__((aligned)),
    // which implies __cxa_exception is also aligned. Insert padding
    // in the beginning of the struct, rather than before unwindHeader.
    void *reserve;
#endif
    std::size_t referenceCount;
#endif
#endif
    std::type_info * exceptionType;
    void (* exceptionDestructor)(void *);
    void (*unexpectedHandler)(); // std::unexpected_handler dropped from C++17
    std::terminate_handler terminateHandler;
    __cxa_exception * nextException;
    int handlerCount;
    int handlerSwitchValue;
    char const * actionRecord;
    char const * languageSpecificData;
    void * catchTemp;
    void * adjustedPtr;
    _Unwind_Exception unwindHeader;
};
}
#endif

#if !HAVE_CXXABI_H_CXA_EH_GLOBALS
// <https://mentorembedded.github.io/cxx-abi/abi-eh.html>:
namespace __cxxabiv1 {
struct __cxa_eh_globals {
    __cxa_exception * caughtExceptions;
    unsigned int uncaughtExceptions;
};
}
#endif

#if !HAVE_CXXABI_H_CXA_GET_GLOBALS
namespace __cxxabiv1 {
extern "C" __cxa_eh_globals * __cxa_get_globals() throw();
}
#endif

#if !HAVE_CXXABI_H_CXA_CURRENT_EXCEPTION_TYPE
namespace __cxxabiv1 {
extern "C" std::type_info *__cxa_current_exception_type() throw();
}
#endif

#if !HAVE_CXXABI_H_CXA_ALLOCATE_EXCEPTION
namespace __cxxabiv1 {
extern "C" void * __cxa_allocate_exception(std::size_t thrown_size) throw();
}
#endif

#if !HAVE_CXXABI_H_CXA_THROW
namespace __cxxabiv1 {
extern "C" void __cxa_throw(
    void * thrown_exception, void * tinfo, void (* dest)(void *))
    __attribute__((noreturn));
}
#endif

extern "C" void privateSnippetExecutor( ... );

namespace CPPU_CURRENT_NAMESPACE
{

void raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

void fillUnoException(uno_Any *, uno_Mapping * pCpp2Uno);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
