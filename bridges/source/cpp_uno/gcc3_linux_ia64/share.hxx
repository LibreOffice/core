/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: share.hxx,v $
 * $Revision: 1.3 $
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
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

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

namespace ia64
{
    enum ia64limits { MAX_GPR_REGS = 8, MAX_SSE_REGS = 8, MAX_REG_SLOTS = 8 };

    bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef );
    bool return_via_r8_buffer( typelib_TypeDescriptionReference *pTypeRef );

    struct RegReturn
    {
        long r8;
        long r9;
        long r10;
        long r11;
    };
}

namespace bridges
{
    namespace cpp_uno
    {
        namespace shared
        {
            /*
                http://www.swag.uwaterloo.ca/asx/ABI.html
                On Itanium, function pointers are pairs: the function address followed
                by the global pointer value that should be used when calling the
                function (code address, gp value)
            */
            struct VtableFactory::Slot
            {
                sal_uInt64 code_address;
                sal_uInt64 gp_value;
            };
        }
    }
}
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
