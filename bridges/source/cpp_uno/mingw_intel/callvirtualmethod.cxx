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

#include "cppu/macros.hxx"
#include "osl/diagnose.h"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

#include "callvirtualmethod.hxx"
#include "share.hxx"
#include "smallstruct.hxx"

// For some reason, callVirtualMethod needs to be in a source file of its own,
// so that stack unwinding upon a thrown exception from within the asm block
// call works, at least with GCC 4.7.0 and --enable-dbgutil.

// The call instruction within the asm section of callVirtualMethod may throw
// exceptions.  So that the compiler handles this correctly, it is important
// that (a) callVirtualMethod might call dummy_can_throw_anything (although this
// never happens at runtime), which in turn can throw exceptions, and (b)
// callVirtualMethod is not inlined at its call site (so that any exceptions are
// caught which are thrown from the instruction calling callVirtualMethod).  [It
// is unclear how much of this comment is still relevent -- see the above
// comment.]
void CPPU_CURRENT_NAMESPACE::callVirtualMethod(
    void * pAdjustedThisPtr, sal_Int32 nVtableIndex, void * pRegisterReturn,
    typelib_TypeDescription const * returnType, sal_Int32 * pStackLongs,
    sal_Int32 nStackLongs)
{
    // parameter list is mixed list of * and values
    // reference parameters are pointers

    OSL_ENSURE( pStackLongs && pAdjustedThisPtr, "### null ptr!" );
    OSL_ENSURE( (sizeof(void *) == 4) && (sizeof(sal_Int32) == 4), "### unexpected size of int!" );
    OSL_ENSURE( nStackLongs && pStackLongs, "### no stack in callVirtualMethod !" );

    // never called
    if (! pAdjustedThisPtr) CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); // address something

    volatile long edx = 0, eax = 0; // for register returns
    void * stackptr;
    asm volatile (
        "mov   %%esp, %6\n\t"
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
    switch( returnType->eTypeClass )
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
        case typelib_TypeClass_STRUCT:
            if (bridges::cpp_uno::shared::isSmallStruct(returnType)) {
                        if (returnType->nSize <= 1) {
                    *(unsigned char*)pRegisterReturn = eax;
                    }
                    else if (returnType->nSize <= 2) {
                    *(unsigned short*)pRegisterReturn = eax;
                    }
                        else if (returnType->nSize <= 8) {
                    ((long*)pRegisterReturn)[0] = eax;
                        if (returnType->nSize > 4) {
                        ((long*)pRegisterReturn)[1] = edx;
                    }
                           }
                }
            break;
        default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
