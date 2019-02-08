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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_GCC3_LINUX_AARCH64_ABI_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_GCC3_LINUX_AARCH64_ABI_HXX

#include <sal/config.h>

#include <exception>
#include <typeinfo>

#include <typelib/typedescription.h>
#include <uno/any2.h>
#include <uno/mapping.h>

namespace abi_aarch64 {

// Following declarations from libstdc++-v3/libsupc++/unwind-cxx.h and
// lib/gcc/*-*-*/*/include/unwind.h:

struct _Unwind_Exception
{
    unsigned exception_class __attribute__((__mode__(__DI__)));
    void * exception_cleanup;
    unsigned private_1 __attribute__((__mode__(__word__)));
    unsigned private_2 __attribute__((__mode__(__word__)));
} __attribute__((__aligned__));

struct __cxa_exception
{
    std::type_info *exceptionType;
    void (*exceptionDestructor)(void *);

    void (*unexpectedHandler)(); // std::unexpected_handler dropped from C++17
    std::terminate_handler terminateHandler;

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

void mapException(
    __cxa_exception * exception, std::type_info const * type, uno_Any * any, uno_Mapping * mapping);

void raiseException(uno_Any * any, uno_Mapping * mapping);

enum ReturnKind {
    RETURN_KIND_REG, RETURN_KIND_HFA_FLOAT, RETURN_KIND_HFA_DOUBLE,
    RETURN_KIND_INDIRECT };

ReturnKind getReturnKind(typelib_TypeDescription const * type);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
