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
#pragma once
#include <sal/config.h>

#include <typeinfo>
#include <exception>
#include <cstddef>
#include <unwind.h>

#include <cxxabi.h>
#ifndef _GLIBCXX_CDTOR_CALLABI // new in GCC 4.7 cxxabi.h
#define _GLIBCXX_CDTOR_CALLABI
#endif

#include <config_cxxabi.h>
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

#if !HAVE_CXXABI_H_CXA_EXCEPTION
namespace __cxxabiv1 {
    struct __cxa_exception
    {
#if defined _LIBCPPABI_VERSION // detect libc++abi
#if defined __LP64__ || defined __ARM_EABI__
        // Quoting android-ndk-r18b/sources/cxx-stl/llvm-libc++abi/src/cxa_exception.hpp: "This is a
        // new field to support C++ 0x exception_ptr. For binary compatibility it is at the start of
        // this struct which is prepended to the object thrown in __cxa_allocate_exception."
        std::size_t referenceCount;
#endif
#endif

        std::type_info *exceptionType;
        void (*exceptionDestructor)(void *);

        void (*unexpectedHandler)(); // std::unexpected_handler dropped from C++17
        std::terminate_handler terminateHandler;

        __cxa_exception *nextException;

        int handlerCount;
#ifdef __ARM_EABI__
        __cxa_exception *nextPropagatingException;
        int propagationCount;
#else
        int handlerSwitchValue;
        const unsigned char *actionRecord;
        const unsigned char *languageSpecificData;
        void *catchTemp;
        void *adjustedPtr;
#endif
        _Unwind_Exception unwindHeader;
    };

}
#endif

namespace CPPU_CURRENT_NAMESPACE
{

    void dummy_can_throw_anything( char const * );

    // -- following decl from libstdc++-v3/libsupc++/unwind-cxx.h and unwind.h

#if !HAVE_CXXABI_H_CXA_ALLOCATE_EXCEPTION
    extern "C" void *__cxa_allocate_exception(
        std::size_t thrown_size ) throw();
#endif
#if !HAVE_CXXABI_H_CXA_THROW
    extern "C" void __cxa_throw (
        void *thrown_exception, std::type_info *tinfo,
        void (*dest) (void *) ) __attribute__((noreturn));
#endif

}

#if !HAVE_CXXABI_H_CXA_EH_GLOBALS
namespace __cxxabiv1 {
    struct __cxa_eh_globals
    {
        __cxa_exception *caughtExceptions;
        unsigned int uncaughtExceptions;
#ifdef __ARM_EABI__
    __cxa_exception *propagatingExceptions;
#endif
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

namespace CPPU_CURRENT_NAMESPACE
{
    void raiseException(
        uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );
    void fillUnoException(uno_Any *, uno_Mapping * pCpp2Uno);
}

extern "C" void privateSnippetExecutor();

namespace arm
{
    enum armlimits { MAX_GPR_REGS = 4, MAX_FPR_REGS = 8 };
    bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
