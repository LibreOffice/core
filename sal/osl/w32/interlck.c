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

#include "system.h"

#include <osl/interlck.h>
#include <osl/diagnose.h>

extern int osl_isSingleCPU;

/* For all Intel x86 above x486 we use a spezial inline assembler implementation.
   The main reason is that WIN9? does not return the result of the operation.
   Instead there is only returned a value greater than zero is the increment
   result is greater than zero, but not the the result of the addition.
   For Windows NT the native function could be used, because the correct result
   is returned. Beacuse of simpler code maintance and performace reasons we use
   on every x86-Windows-Platform the inline assembler implementation.
*/

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
