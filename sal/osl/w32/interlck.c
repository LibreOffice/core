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

#include "system.h"

#include <osl/interlck.h>
#include <osl/diagnose.h>

extern int osl_isSingleCPU;

/* For all Intel x86 above x486 we use a spezial inline assembler implementation.
   The main reason is that WIN9? does not return the result of the operation.
   Instead there is only returned a value greater than zero is the increment
   result is greater than zero, but not the result of the addition.
   For Windows NT the native function could be used, because the correct result
   is returned. Beacuse of simpler code maintance and performace reasons we use
   on every x86-Windows-Platform the inline assembler implementation.
*/

#if defined __MINGW32__
#pragma GCC diagnostic warning "-Wreturn-type"
#endif

/*****************************************************************************/
/* osl_incrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
#ifdef _M_IX86
#ifdef __MINGW32__
{
    asm
    (
     "  movl        %0, %%ecx\n"
     "      movl        $1, %%eax\n"
     "  movl        %1, %%edx\n"
     "  cmpl        $0, %%edx\n"
     "  je          1f\n"
     "      xadd        %%eax, (%%ecx)\n"
     "  jmp         2f\n"
     "1:\n"
     "      lock xadd   %%eax, (%%ecx)\n"
     "2:\n"
     "      incl        %%eax\n"
     ::"m"(pCount),"m"(osl_isSingleCPU)
    );
}
#else
#pragma warning(disable: 4035)
{
    __asm
    {
        mov         ecx, pCount
        mov         eax, 1
        mov         edx, osl_isSingleCPU
        cmp         edx, 0
        je          is_not_single
        xadd        dword ptr [ecx],eax
        jmp         cont
    is_not_single:
        lock xadd   dword ptr [ecx],eax
    cont:
        inc         eax
    }
}
#pragma warning(default: 4035)
#endif
#else
#pragma message("WARNING: Using system InterlockedIncrement")
{
    return (InterlockedIncrement(pCount));
}
#endif

/*****************************************************************************/
/* osl_decrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
#ifdef _M_IX86
#ifdef __MINGW32__
{
    asm
    (
     "  movl        %0, %%ecx\n"
     "      orl         $-1, %%eax\n"
     "  movl        %1, %%edx\n"
     "  cmpl        $0, %%edx\n"
     "  je          1f\n"
     "      xadd        %%eax, (%%ecx)\n"
     "  jmp         2f\n"
     "1:\n"
     "      lock xadd   %%eax, (%%ecx)\n"
     "2:\n"
     "      decl        %%eax\n"
     ::"m"(pCount),"m"(osl_isSingleCPU)
    );
}
#else
#pragma warning(disable: 4035)
{
    __asm
    {
        mov         ecx, pCount
        or          eax, -1
        mov         edx, osl_isSingleCPU
        cmp         edx, 0
        je          is_not_single
        xadd        dword ptr [ecx],eax
        jmp         cont
    is_not_single:
        lock xadd   dword ptr [ecx],eax
    cont:
        dec         eax
    }
}
#pragma warning(default: 4035)
#endif
#else
#pragma message("WARNING: Using system InterlockedDecrement")
{
    return (InterlockedDecrement(pCount));
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
