/*************************************************************************
 *
 *  $RCSfile: interlck.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: martin.maher $ $Date: 2000-09-29 14:25:53 $
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

#include "system.h"

#include <osl/interlck.h>
#include <osl/diagnose.h>

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
           lock xadd   dword ptr [ecx],eax
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
           mov         eax, -1
           lock xadd   dword ptr [ecx],eax
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


