/*************************************************************************
 *
 *  $RCSfile: share.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:32:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <typeinfo>
#include <exception>
#include <cstddef>

namespace CPPU_CURRENT_NAMESPACE
{

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
