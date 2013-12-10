/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "uno/mapping.h"

#include <typeinfo>
#include <exception>
#include <cstddef>

namespace CPPU_CURRENT_NAMESPACE
{

void dummy_can_throw_anything( char const * );

// ----- the following structure is compatible with the one declared in libunwind's unwind.h

struct _Unwind_Exception
{
    unsigned long exception_class;
    void * exception_cleanup;
    uintptr_t private_1;
    uintptr_t private_2;
} __attribute__((__aligned__));

struct __cxa_exception
{
#if __LP64__ // ----- from libcxxabi/src/cxa_exception.hpp
    // This is a new field to support C++ 0x exception_ptr.
    // For binary compatibility it is at the start of this
    // struct which is prepended to the object thrown in
    // __cxa_allocate_exception.
    size_t referenceCount;
#endif

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

extern "C" void *__cxa_allocate_exception(
    std::size_t thrown_size ) throw();
extern "C" void __cxa_throw (
    void *thrown_exception, std::type_info *tinfo, void (*dest) (void *) ) __attribute__((noreturn));

struct __cxa_eh_globals
{
    __cxa_exception *caughtExceptions;
    unsigned int uncaughtExceptions;
};
extern "C" __cxa_eh_globals *__cxa_get_globals () throw();

// -----

//==================================================================================================
void raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );
//==================================================================================================
void fillUnoException(
    __cxa_exception * header, uno_Any *, uno_Mapping * pCpp2Uno );
}

