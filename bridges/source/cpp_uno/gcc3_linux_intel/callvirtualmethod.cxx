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

#include "sal/config.h"

#include <cassert>

#include "cppu/macros.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

#include "callvirtualmethod.hxx"
#include "share.hxx"

// The call instruction within the asm block of callVirtualMethod may throw
// exceptions.  At least GCC 4.7.0 with -O0 would create (unnecessary)
// .gcc_exception_table call-site table entries around all other calls in this
// function that can throw, leading to std::terminate if the asm call throws an
// exception and the unwinding C++ personality routine finds the unexpected hole
// in the .gcc_exception_table.  Therefore, make sure this function explicitly
// only calls nothrow-functions (so GCC 4.7.0 with -O0 happens to not create a
// .gcc_exception_table section at all for this function).  For some reason,
// this also needs to be in a source file of its own.
//
// Also, this file should be compiled with -fnon-call-exceptions, and ideally
// there would be a way to tell the compiler that the asm block contains calls
// to functions that can potentially throw; see the mail thread starting at
// <http://gcc.gnu.org/ml/gcc/2012-03/msg00454.html> "C++: Letting compiler know
// asm block can call function that can throw?"

void CPPU_CURRENT_NAMESPACE::callVirtualMethod(
    void * pAdjustedThisPtr, sal_Int32 nVtableIndex, void * pRegisterReturn,
    typelib_TypeDescription * pReturnTypeDescr, bool bSimpleReturn,
    sal_Int32 * pStackLongs, sal_Int32 nStackLongs)
{
    // parameter list is mixed list of * and values
    // reference parameters are pointers

    assert(pStackLongs && pAdjustedThisPtr);
    assert(sizeof (void *) == 4 && sizeof (sal_Int32) == 4);
        // unexpected size of int
    assert(nStackLongs && pStackLongs); // no stack

#if defined __clang__ || __GNUC__ == 4 && __GNUC_MINOR__ <= 6
    if (! pAdjustedThisPtr) CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); // address something
#endif

    volatile long edx = 0, eax = 0; // for register returns
    void * stackptr;
    asm volatile (
        "mov   %%esp, %6\n\t"
        // preserve potential 128bit stack alignment
        "and   $0xfffffff0, %%esp\n\t"
        "mov   %0, %%eax\n\t"
        "lea   -4(,%%eax,4), %%eax\n\t"
        "and   $0xf, %%eax\n\t"
        "sub   $0xc, %%eax\n\t"
        "add   %%eax, %%esp\n\t"
        // copy values
        "mov   %0, %%eax\n\t"
        "mov   %%eax, %%edx\n\t"
        "dec   %%edx\n\t"
        "shl   $2, %%edx\n\t"
        "add   %1, %%edx\n"
        "Lcopy:\n\t"
        "pushl 0(%%edx)\n\t"
        "sub   $4, %%edx\n\t"
        "dec   %%eax\n\t"
        "jne   Lcopy\n\t"
        // do the actual call
        "mov   %2, %%edx\n\t"
        "mov   0(%%edx), %%edx\n\t"
        "mov   %3, %%eax\n\t"
        "shl   $2, %%eax\n\t"
        "add   %%eax, %%edx\n\t"
        "mov   0(%%edx), %%edx\n\t"
        "call  *%%edx\n\t"
        // save return registers
         "mov   %%eax, %4\n\t"
         "mov   %%edx, %5\n\t"
        // cleanup stack
        "mov   %6, %%esp\n\t"
        :
        : "m"(nStackLongs), "m"(pStackLongs), "m"(pAdjustedThisPtr),
          "m"(nVtableIndex), "m"(eax), "m"(edx), "m"(stackptr)
        : "eax", "ecx", "edx" );
    switch( pReturnTypeDescr->eTypeClass )
    {
        case typelib_TypeClass_VOID:
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            ((long*)pRegisterReturn)[1] = edx;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_ENUM:
            ((long*)pRegisterReturn)[0] = eax;
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(unsigned short*)pRegisterReturn = eax;
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            *(unsigned char*)pRegisterReturn = eax;
            break;
        case typelib_TypeClass_FLOAT:
            asm ( "fstps %0" : : "m"(*(char *)pRegisterReturn) );
            break;
        case typelib_TypeClass_DOUBLE:
            asm ( "fstpl %0\n\t" : : "m"(*(char *)pRegisterReturn) );
            break;
        default:
        {
#if defined (FREEBSD) || defined(NETBSD) || defined(OPENBSD) || defined(MACOSX) || \
    defined(DRAGONFLY)
            sal_Int32 const nRetSize = pReturnTypeDescr->nSize;
            if (bSimpleReturn && nRetSize <= 8 && nRetSize > 0)
            {
                if (nRetSize > 4)
                    static_cast<long *>(pRegisterReturn)[1] = edx;
                static_cast<long *>(pRegisterReturn)[0] = eax;
            }
#else
            (void)bSimpleReturn;
#endif
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
