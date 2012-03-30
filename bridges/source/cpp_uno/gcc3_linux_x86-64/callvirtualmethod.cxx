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

#include <cstring>

#include "cppu/macros.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

#include "abi.hxx"
#include "callvirtualmethod.hxx"

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
    void * pThis, sal_uInt32 nVtableIndex, void * pRegisterReturn,
    typelib_TypeDescriptionReference * pReturnTypeRef, bool bSimpleReturn,
    sal_uInt64 *pStack, sal_uInt32 nStack, sal_uInt64 *pGPR, sal_uInt32 nGPR,
    double * pFPR, sal_uInt32 nFPR)
{
    // Should not happen, but...
    if ( nFPR > x86_64::MAX_SSE_REGS )
        nFPR = x86_64::MAX_SSE_REGS;
    if ( nGPR > x86_64::MAX_GPR_REGS )
        nGPR = x86_64::MAX_GPR_REGS;

    // Get pointer to method
    sal_uInt64 pMethod = *((sal_uInt64 *)pThis);
    pMethod += 8 * nVtableIndex;
    pMethod = *((sal_uInt64 *)pMethod);

    // Load parameters to stack, if necessary
    if ( nStack )
    {
        // 16-bytes aligned
        sal_uInt32 nStackBytes = ( ( nStack + 1 ) >> 1 ) * 16;
        sal_uInt64 *pCallStack = (sal_uInt64 *) __builtin_alloca( nStackBytes );
        std::memcpy( pCallStack, pStack, nStackBytes );
    }

    // Return values
    sal_uInt64 rax;
    sal_uInt64 rdx;
    double xmm0;
    double xmm1;

    asm volatile (

        // Fill the xmm registers
        "movq %6, %%rax\n\t"

        "movsd   (%%rax), %%xmm0\n\t"
        "movsd  8(%%rax), %%xmm1\n\t"
        "movsd 16(%%rax), %%xmm2\n\t"
        "movsd 24(%%rax), %%xmm3\n\t"
        "movsd 32(%%rax), %%xmm4\n\t"
        "movsd 40(%%rax), %%xmm5\n\t"
        "movsd 48(%%rax), %%xmm6\n\t"
        "movsd 56(%%rax), %%xmm7\n\t"

        // Fill the general purpose registers
        "movq %5, %%rax\n\t"

        "movq    (%%rax), %%rdi\n\t"
        "movq   8(%%rax), %%rsi\n\t"
        "movq  16(%%rax), %%rdx\n\t"
        "movq  24(%%rax), %%rcx\n\t"
        "movq  32(%%rax), %%r8\n\t"
        "movq  40(%%rax), %%r9\n\t"

        // Perform the call
        "movq %4, %%r11\n\t"
        "movq %7, %%rax\n\t"
        "call *%%r11\n\t"

        // Fill the return values
        "movq   %%rax, %0\n\t"
        "movq   %%rdx, %1\n\t"
        "movsd %%xmm0, %2\n\t"
        "movsd %%xmm1, %3\n\t"
        : "=m" ( rax ), "=m" ( rdx ), "=m" ( xmm0 ), "=m" ( xmm1 )
        : "m" ( pMethod ), "m" ( pGPR ), "m" ( pFPR ), "m" ( nFPR )
        : "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );

    switch (pReturnTypeRef->eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *reinterpret_cast<sal_uInt64 *>( pRegisterReturn ) = rax;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
        *reinterpret_cast<sal_uInt32 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt32*>( &rax );
        break;
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *reinterpret_cast<sal_uInt16 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt16*>( &rax );
        break;
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
        *reinterpret_cast<sal_uInt8 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt8*>( &rax );
        break;
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_DOUBLE:
        *reinterpret_cast<double *>( pRegisterReturn ) = xmm0;
        break;
    default:
        {
            sal_Int32 const nRetSize = pReturnTypeRef->pType->nSize;
            if (bSimpleReturn && nRetSize <= 16 && nRetSize > 0)
            {
                sal_uInt64 longs[2];
                longs[0] = rax;
                longs[1] = rdx;

                double doubles[2];
                doubles[0] = xmm0;
                doubles[1] = xmm1;
                x86_64::fill_struct( pReturnTypeRef, &longs[0], &doubles[0], pRegisterReturn);
            }
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
