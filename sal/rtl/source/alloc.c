/*************************************************************************
 *
 *  $RCSfile: alloc.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:24 $
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

#ifndef MAC
  #ifdef MACOSX
  #include <sys/types.h>
  #include <sys/malloc.h>
  #else
  #include <malloc.h>
#endif
#else
#include <stdlib.h>
#endif
#include <string.h>

#include <rtl/alloc.h>

/*
 * rtl_allocateMemory.
 */
void * SAL_CALL rtl_allocateMemory(sal_uInt32 Bytes)
{
    if (Bytes == 0)
        return (NULL);

#ifdef _WIN16
    return (farmalloc(Bytes));
#else
    return (malloc(Bytes));
#endif
}

/*
 * rtl_reallocateMemory.
 */
void * SAL_CALL rtl_reallocateMemory(void *Ptr, sal_uInt32 Bytes)
{
    if (Bytes == 0)
    {
        if (Ptr != NULL)
            rtl_freeMemory(Ptr);

        return (NULL);
    }

    if (Ptr == NULL)
        return (rtl_allocateMemory(Bytes));

#ifdef _WIN16
    return (farrealloc(Ptr, Bytes));
#else
    return (realloc(Ptr, Bytes));
#endif
}

/*
 * rtl_freeMemory.
 */
void SAL_CALL rtl_freeMemory(void *Ptr)
{
    if (Ptr != NULL)
#ifdef _WIN16
        farfree(Ptr);
#else
        free(Ptr);
#endif
}

/*
 * rtl_allocateZeroMemory.
 */
void * SAL_CALL rtl_allocateZeroMemory (sal_uInt32 Bytes)
{
    if (Bytes == 0)
        return (NULL);

#ifdef _WIN16
    return (farcalloc (Bytes));
#else
    return (calloc (Bytes, 1));
#endif
}

/*
 * rtl_freeZeroMemory.
 */
void SAL_CALL rtl_freeZeroMemory (void *Ptr, sal_uInt32 Bytes)
{
    if (Ptr != NULL)
    {
#ifdef _WIN16
        _fmemset (Ptr, 0, Bytes);
        farfree (Ptr);
#else
        memset (Ptr, 0, Bytes);
        free (Ptr);
#endif
    }
}

#ifdef LINUX

#if defined(MALLOC_HOOKS)
extern void *(*__malloc_hook) (size_t __size, const void* caller);
#endif

void * calloc (size_t n, size_t elem_size)
{
      size_t sz;
      void *mem;

    sz = n * elem_size;

#   if defined(MALLOC_HOOKS)
      if ( __malloc_hook != NULL )
          mem = (*__malloc_hook) (sz, __builtin_return_address(0) );
    else
#   endif

    mem = malloc(sz);

       if (mem != NULL)
           memset (mem, 0, sz);

    return mem;
}

#endif /* LINUX */

