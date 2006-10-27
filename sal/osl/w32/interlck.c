/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interlck.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-27 12:00:24 $
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
#else
#pragma message("WARNING: Using system InterlockedDecrement")
{
    return (InterlockedDecrement(pCount));
}
#endif
