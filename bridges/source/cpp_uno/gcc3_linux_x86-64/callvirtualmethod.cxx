/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include <cstring>

#include "cppu/macros.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

#include "abi.hxx"
#include "callvirtualmethod.hxx"










//






void CPPU_CURRENT_NAMESPACE::callVirtualMethod(
    void * pThis, sal_uInt32 nVtableIndex, void * pRegisterReturn,
    typelib_TypeDescriptionReference * pReturnTypeRef, bool bSimpleReturn,
    sal_uInt64 *pStack, sal_uInt32 nStack, sal_uInt64 *pGPR, sal_uInt32 nGPR,
    double * pFPR, sal_uInt32 nFPR)
{
    
    if ( nFPR > x86_64::MAX_SSE_REGS )
        nFPR = x86_64::MAX_SSE_REGS;
    if ( nGPR > x86_64::MAX_GPR_REGS )
        nGPR = x86_64::MAX_GPR_REGS;

    
    sal_uInt64 pMethod = *((sal_uInt64 *)pThis);
    pMethod += 8 * nVtableIndex;
    pMethod = *((sal_uInt64 *)pMethod);

    
    sal_uInt64* pCallStack = NULL;
    if ( nStack )
    {
        
        sal_uInt32 nStackBytes = ( ( nStack + 1 ) >> 1 ) * 16;
        pCallStack = (sal_uInt64 *) __builtin_alloca( nStackBytes );
        std::memcpy( pCallStack, pStack, nStackBytes );
    }

    
    sal_uInt64 rax;
    sal_uInt64 rdx;
    double xmm0;
    double xmm1;

    asm volatile (

        
        "movq %6, %%rax\n\t"

        "movsd   (%%rax), %%xmm0\n\t"
        "movsd  8(%%rax), %%xmm1\n\t"
        "movsd 16(%%rax), %%xmm2\n\t"
        "movsd 24(%%rax), %%xmm3\n\t"
        "movsd 32(%%rax), %%xmm4\n\t"
        "movsd 40(%%rax), %%xmm5\n\t"
        "movsd 48(%%rax), %%xmm6\n\t"
        "movsd 56(%%rax), %%xmm7\n\t"

        
        "movq %5, %%rax\n\t"

        "movq    (%%rax), %%rdi\n\t"
        "movq   8(%%rax), %%rsi\n\t"
        "movq  16(%%rax), %%rdx\n\t"
        "movq  24(%%rax), %%rcx\n\t"
        "movq  32(%%rax), %%r8\n\t"
        "movq  40(%%rax), %%r9\n\t"

        
        "movq %4, %%r11\n\t"
        "movq %7, %%rax\n\t"
        "call *%%r11\n\t"

        
        "movq   %%rax, %0\n\t"
        "movq   %%rdx, %1\n\t"
        "movsd %%xmm0, %2\n\t"
        "movsd %%xmm1, %3\n\t"
        : "=m" ( rax ), "=m" ( rdx ), "=m" ( xmm0 ), "=m" ( xmm1 )
        : "m" ( pMethod ), "m" ( pGPR ), "m" ( pFPR ), "m" ( nFPR ),
          "m" ( pCallStack ) 
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
