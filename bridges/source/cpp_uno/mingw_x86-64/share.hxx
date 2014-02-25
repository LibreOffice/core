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

#include "uno/mapping.h"

#include <typeinfo>
#include <exception>
#include <cstddef>

#include "cppu/macros.hxx"
#include "uno/any2.h"

namespace CPPU_CURRENT_NAMESPACE
{

void dummy_can_throw_anything( char const * );

// ----- following decl from libstdc++-v3/libsupc++/unwind-cxx.h and unwind.h

struct _Unwind_Exception
{
    unsigned exception_class __attribute__((__mode__(__DI__)));
    void * exception_cleanup;
    unsigned private_1 __attribute__((__mode__(__word__)));
    unsigned private_2 __attribute__((__mode__(__word__)));
} __attribute__((__aligned__));

struct __cxa_exception
{
    ::std::type_info *exceptionType;
    void (*exceptionDestructor)(void *);

    ::std::unexpected_handler unexpectedHandler;
    ::std::terminate_handler terminateHandler;

    __cxa_exception *nextException;

    int handlerCount;

    int handlerSwitchValue;
    const unsigned char *actionRecord;
    const unsigned char *languageSpecificData;
    void *catchTemp;
    void *adjustedPtr;

    _Unwind_Exception unwindHeader;
};

struct __cxa_eh_globals
{
    __cxa_exception *caughtExceptions;
    unsigned int uncaughtExceptions;
};

}

extern "C" CPPU_CURRENT_NAMESPACE::__cxa_eh_globals *__cxa_get_globals () throw();

namespace CPPU_CURRENT_NAMESPACE
{

// The following are in cxxabi.h since GCC 4.7 (they are wrapped in
// CPPU_CURRENT_NAMESPACE here as different GCC versions have slightly different
// declarations for them, e.g., with or without throw() specification, so would
// complain about redeclarations of these somewhat implicitly declared
// functions):
#if __GNUC__ == 4 && __GNUC_MINOR__ <= 6
extern "C" void *__cxa_allocate_exception(
    std::size_t thrown_size ) throw();
extern "C" void __cxa_throw (
    void *thrown_exception, void *tinfo, void (*dest) (void *) ) __attribute__((noreturn));
#endif


void raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

void fillUnoException(
    __cxa_exception * header, uno_Any *, uno_Mapping * pCpp2Uno );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
