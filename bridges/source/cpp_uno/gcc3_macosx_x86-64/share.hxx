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

#include <uno/mapping.h>

#include <typeinfo>
#include <exception>
#include <cstddef>

namespace CPPU_CURRENT_NAMESPACE
{

// From opensource.apple.com: libunwind-35.1/include/unwind.h

typedef enum {
    _URC_NO_REASON = 0,
    _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
    _URC_FATAL_PHASE2_ERROR = 2,
    _URC_FATAL_PHASE1_ERROR = 3,
    _URC_NORMAL_STOP = 4,
    _URC_END_OF_STACK = 5,
    _URC_HANDLER_FOUND = 6,
    _URC_INSTALL_CONTEXT = 7,
    _URC_CONTINUE_UNWIND = 8
} _Unwind_Reason_Code;

struct _Unwind_Exception
{
    uint64_t                   exception_class;
    void                     (*exception_cleanup)(_Unwind_Reason_Code reason, struct _Unwind_Exception* exc);
    uintptr_t                  private_1;        // non-zero means forced unwind
    uintptr_t                  private_2;        // holds sp that phase1 found for phase2 to use
#if !__LP64__
    // The gcc implementation of _Unwind_Exception used attribute mode on the above fields
    // which had the side effect of causing this whole struct to round up to 32 bytes in size.
    // To be more explicit, we add pad fields added for binary compatibility.
    uint32_t                reserved[3];
#endif
};


// From libcppabi-24.2/include/unwind-cxx.h

typedef unsigned _Unwind_Ptr __attribute__((__mode__(__pointer__)));

// A C++ exception object consists of a header, which is a wrapper around
// an unwind object header with additional C++ specific information,
// followed by the exception object itself.

struct __cxa_exception
{
#if __LP64__
#if 0
    // This is a new field added with LLVM 10
    // <https://github.com/llvm/llvm-project/commit/674ec1eb16678b8addc02a4b0534ab383d22fa77>
    // "[libcxxabi] Insert padding in __cxa_exception struct for compatibility".  The HACK in
    // fillUnoException (bridges/source/cpp_uno/gcc3_macosx_x86-64/except.cxx) tries to find out at
    // runtime whether a __cxa_exception has this member.  Once we can be sure that we only run
    // against new libcxxabi that has this member, we can drop the "#if 0" here and drop the hack
    // in fillUnoException.

    // Now _Unwind_Exception is marked with __attribute__((aligned)),
    // which implies __cxa_exception is also aligned. Insert padding
    // in the beginning of the struct, rather than before unwindHeader.
    void *reserve;
#endif

    // This is a new field to support C++ 0x exception_ptr.
    // For binary compatibility it is at the start of this
    // struct which is prepended to the object thrown in
    // __cxa_allocate_exception.
  size_t referenceCount;
#endif
  // Manage the exception object itself.
  std::type_info *exceptionType;
  void (*exceptionDestructor)(void *);

  // The C++ standard has entertaining rules wrt calling set_terminate
  // and set_unexpected in the middle of the exception cleanup process.
  void (*unexpectedHandler)(); // std::unexpected_handler dropped from C++17
  std::terminate_handler terminateHandler;

  // The caught exception stack threads through here.
  __cxa_exception *nextException;

  // How many nested handlers have caught this exception.  A negated
  // value is a signal that this object has been rethrown.
  int handlerCount;

#ifdef __ARM_EABI_UNWINDER__
  // Stack of exceptions in cleanups.
  __cxa_exception* nextPropagatingException;

  // The number of active cleanup handlers for this exception.
  int propagationCount;
#else
  // Cache parsed handler data from the personality routine Phase 1
  // for Phase 2 and __cxa_call_unexpected.
  int handlerSwitchValue;
  const unsigned char *actionRecord;
  const unsigned char *languageSpecificData;
  _Unwind_Ptr catchTemp;
  void *adjustedPtr;
#endif
#if !__LP64__
    // This is a new field to support C++ 0x exception_ptr.
    // For binary compatibility it is placed where the compiler
    // previously adding padded to 64-bit align unwindHeader.
  size_t referenceCount;
#endif

  // The generic exception header.  Must be last.
  _Unwind_Exception unwindHeader;
};

// Each thread in a C++ program has access to a __cxa_eh_globals object.
struct __cxa_eh_globals
{
  __cxa_exception *caughtExceptions;
  unsigned int uncaughtExceptions;
#ifdef __ARM_EABI_UNWINDER__
  __cxa_exception* propagatingExceptions;
#endif
};

}

extern "C" CPPU_CURRENT_NAMESPACE::__cxa_eh_globals *__cxa_get_globals () throw();

namespace CPPU_CURRENT_NAMESPACE
{

void raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

void fillUnoException(uno_Any *, uno_Mapping * pCpp2Uno);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
