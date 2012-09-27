/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "uno/mapping.h"

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
  std::unexpected_handler unexpectedHandler;
  std::terminate_handler terminateHandler;

  // The caught exception stack threads through here.
  __cxa_exception *nextException;

  // How many nested handlers have caught this exception.  A negated
  // value is a signal that this object has been rethrown.
  int handlerCount;

#ifdef __ARM_EABI_UNWINDER__
  // Stack of exceptions in cleanups.
  __cxa_exception* nextPropagatingException;

  // The nuber of active cleanup handlers for this exception.
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

// -----

//==================================================================================================
void raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );
//==================================================================================================
void fillUnoException(
    __cxa_exception * header, uno_Any *, uno_Mapping * pCpp2Uno );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
