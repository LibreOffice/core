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
    sal_uInt64 *pStack, sal_uInt32 nStack, sal_uInt64 *pGPR, double * pFPR)
{
    // Work around Clang -fsanitize=address "inline assembly requires more
    // registers than available" error:
    struct Data {
        sal_uInt64 pMethod;
        sal_uInt64 * pStack;
        sal_uInt32 nStack;
        sal_uInt64 * pGPR;
        double * pFPR;
        // Return values:
        sal_uInt64 rax;
        sal_uInt64 rdx;
        double xmm0;
        double xmm1;
    } data;
    data.pStack = pStack;
    data.nStack = nStack;
    data.pGPR = pGPR;
    data.pFPR = pFPR;

    // Get pointer to method
    sal_uInt64 pMethod = *static_cast<sal_uInt64 *>(pThis);
    pMethod += 8 * nVtableIndex;
    data.pMethod = *reinterpret_cast<sal_uInt64 *>(pMethod);

    asm volatile (
        // Push arguments to stack
        "movq %%rsp, %%r12\n\t"
        "movl 16%0, %%ecx\n\t"
        "jrcxz .Lpushed\n\t"
        "xor %%rax, %%rax\n\t"
        "leaq (%%rax, %%rcx, 8), %%rax\n\t"
        "subq %%rax, %%rsp\n\t"
        "andq $-9, %%rsp\n\t" // 16-bytes aligned

        "movq 8%0, %%rsi\n\t"
        "\n.Lpush:\n\t"
        "decq %%rcx\n\t"
        "movq (%%rsi, %%rcx, 8), %%rax\n\t"
        "movq %%rax, (%%rsp, %%rcx, 8)\n\t"
        "jnz .Lpush\n\t"
        "\n.Lpushed:\n\t"

        // Fill the xmm registers
        "movq 32%0, %%rax\n\t"

        "movsd   (%%rax), %%xmm0\n\t"
        "movsd  8(%%rax), %%xmm1\n\t"
        "movsd 16(%%rax), %%xmm2\n\t"
        "movsd 24(%%rax), %%xmm3\n\t"
        "movsd 32(%%rax), %%xmm4\n\t"
        "movsd 40(%%rax), %%xmm5\n\t"
        "movsd 48(%%rax), %%xmm6\n\t"
        "movsd 56(%%rax), %%xmm7\n\t"

        // Fill the general purpose registers
        "movq 24%0, %%rax\n\t"

        "movq    (%%rax), %%rdi\n\t"
        "movq   8(%%rax), %%rsi\n\t"
        "movq  16(%%rax), %%rdx\n\t"
        "movq  24(%%rax), %%rcx\n\t"
        "movq  32(%%rax), %%r8\n\t"
        "movq  40(%%rax), %%r9\n\t"

        // Perform the call
        "movq 0%0, %%r11\n\t"
        "call *%%r11\n\t"

        // Fill the return values
        "movq   %%rax, 40%0\n\t"
        "movq   %%rdx, 48%0\n\t"
        "movsd %%xmm0, 56%0\n\t"
        "movsd %%xmm1, 64%0\n\t"

        // Reset %rsp
        "movq %%r12, %%rsp\n\t"
        :: "o" (data)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11", "r12",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );

    switch (pReturnTypeRef->eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *static_cast<sal_uInt64 *>( pRegisterReturn ) = data.rax;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
        *static_cast<sal_uInt32 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt32*>( &data.rax );
        break;
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *static_cast<sal_uInt16 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt16*>( &data.rax );
        break;
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
        *static_cast<sal_uInt8 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt8*>( &data.rax );
        break;
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_DOUBLE:
        *static_cast<double *>( pRegisterReturn ) = data.xmm0;
        break;
    default:
        {
            sal_Int32 const nRetSize = pReturnTypeRef->pType->nSize;
            if (bSimpleReturn && nRetSize <= 16 && nRetSize > 0)
            {
                sal_uInt64 longs[2];
                longs[0] = data.rax;
                longs[1] = data.rdx;

                double doubles[2];
                doubles[0] = data.xmm0;
                doubles[1] = data.xmm1;
                x86_64::fill_struct( pReturnTypeRef, &longs[0], &doubles[0], pRegisterReturn);
            }
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
