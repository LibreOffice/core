/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: share.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-12-12 15:35:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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

    // -----

    //====================================================================
    void raiseException(
        uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );
    //====================================================================
    void fillUnoException(
        __cxa_exception * header, uno_Any *, uno_Mapping * pCpp2Uno );
}
#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
