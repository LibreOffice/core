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
#ifndef _ARM_SHARE_HXX
#define _ARM_SHARE_HXX
#include "uno/mapping.h"

#include <typeinfo>
#include <exception>
#include <cstddef>
#include <unwind.h>

namespace CPPU_CURRENT_NAMESPACE
{

    void dummy_can_throw_anything( char const * );

    // -- following decl from libstdc++-v3/libsupc++/unwind-cxx.h and unwind.h

    struct __cxa_exception
    {
        ::std::type_info *exceptionType;
        void (*exceptionDestructor)(void *);

        ::std::unexpected_handler unexpectedHandler;
        ::std::terminate_handler terminateHandler;

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

    extern "C" void *__cxa_allocate_exception(
        std::size_t thrown_size ) throw();
    extern "C" void __cxa_throw (
        void *thrown_exception, std::type_info *tinfo,
        void (*dest) (void *) ) __attribute__((noreturn));

    struct __cxa_eh_globals
    {
        __cxa_exception *caughtExceptions;
        unsigned int uncaughtExceptions;
#ifdef __ARM_EABI__
    __cxa_exception *propagatingExceptions;
#endif
    };
    extern "C" __cxa_eh_globals *__cxa_get_globals () throw();


    void raiseException(
        uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );
    void fillUnoException(
        __cxa_exception * header, uno_Any *, uno_Mapping * pCpp2Uno );
}

namespace arm
{
    enum armlimits { MAX_GPR_REGS = 4, MAX_FPR_REGS = 8 };
    bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
