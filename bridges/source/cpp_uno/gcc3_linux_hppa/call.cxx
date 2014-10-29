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

#include <malloc.h>
#include <rtl/alloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>

#include <bridges/cpp_uno/shared/bridge.hxx>
#include <bridges/cpp_uno/shared/types.hxx>
#include <bridges/cpp_uno/shared/unointerfaceproxy.hxx>
#include <bridges/cpp_uno/shared/vtables.hxx>

#include "share.hxx"

#include <stdio.h>
#include <string.h>

using namespace ::com::sun::star::uno;

void MapReturn(sal_uInt32 ret0, sal_uInt32 ret1, typelib_TypeDescription *pReturnTypeDescr, bool bRegisterReturn, sal_uInt32 *pRegisterReturn)
{
    register float fret asm("fr4");
    register double dret asm("fr4");

    switch (pReturnTypeDescr->eTypeClass)
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            pRegisterReturn[1] = ret1;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            pRegisterReturn[0] = ret0;
            break;
        case typelib_TypeClass_FLOAT:
            *(float*)pRegisterReturn = fret;
        break;
        case typelib_TypeClass_DOUBLE:
            *(double*)pRegisterReturn = dret;
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            if (bRegisterReturn)
            {
                pRegisterReturn[0] = ret0;
                pRegisterReturn[1] = ret1;
            }
            break;
        }
        default:
            break;
    }
}

//Moved callVirtual into this .cxx so that I can do this and get gcc to not
//touch r28 without having to learn any more pa-risc assembly than is
//strictly necessary
register sal_uInt32 r28 __asm__("%r28");

void callVirtualMethod(void * pThis, sal_uInt32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeDescription *pReturnTypeDescr, bool bRegisterReturn,
    sal_uInt32 *pStack, sal_uInt32 nStack, sal_uInt32 *pGPR, double *pFPR) __attribute__((noinline));

void callVirtualMethod(void * pThis, sal_uInt32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeDescription *pReturnTypeDescr, bool bRegisterReturn,
    sal_uInt32 *pStack, sal_uInt32 nStack, sal_uInt32 *pGPR, double *pFPR)
{
    register sal_uInt32* sp __asm__("%r30");

    sal_uInt32 pMethod = *((sal_uInt32*)pThis);
    pMethod += 4 * nVtableIndex;
    pMethod = *((sal_uInt32 *)pMethod);

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "this is %p\n", pGPR[0]);
    for (int i = 0; i < hppa::MAX_GPR_REGS ; ++i)
        fprintf(stderr, "normal reg %d is %d %x\n", i, pGPR[i], pGPR[i]);

    for (int i = 0; i < hppa::MAX_SSE_REGS ; ++i)
        fprintf(stderr, "float reg %d is %x\n", i, pFPR[i]);

    for (int i = 0; i < nStack; ++i)
        fprintf(stderr, "stack bytes are %x\n", pStack[i]);
#endif

    //Always reserve 4 slots, and align to 8 bytes
    sal_uInt32 nStackBytes = ( ( nStack + 4 + 1 ) >> 1 ) * 8;
    __builtin_alloca(nStackBytes);
    sal_uInt32 *stack = sp-8;
    int o = -5;
    for (sal_uInt32 i = 0; i < nStack; ++i, --o)
        stack[o] = pStack[i];

    typedef int (* FunctionCall )( sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32 );
    FunctionCall pFunc = (FunctionCall)pMethod;

    asm volatile("fldd %0, %%fr4" : : "m"(pFPR[0]) : "fr4");
    asm volatile("fldd %0, %%fr5" : : "m"(pFPR[1]) : "fr5");
    asm volatile("fldd %0, %%fr6" : : "m"(pFPR[2]) : "fr6");
    asm volatile("fldd %0, %%fr7" : : "m"(pFPR[3]) : "fr7");
    asm volatile("ldw %0, %%r28" : : "m"(pRegisterReturn) : "r28");
    (*pFunc)(pGPR[0], pGPR[1], pGPR[2], pGPR[3]);

    register sal_uInt32 r29 __asm__("%r29");
    MapReturn(r28, r29, pReturnTypeDescr, bRegisterReturn, (sal_uInt32*)pRegisterReturn);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
