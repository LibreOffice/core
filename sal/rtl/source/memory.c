/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: memory.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:04:00 $
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


#include <string.h>
#include <rtl/memory.h>

void SAL_CALL rtl_zeroMemory(void *Ptr, sal_Size Bytes)
{
    memset(Ptr, 0, Bytes);
}

void SAL_CALL rtl_fillMemory(void *Ptr, sal_Size Bytes, sal_uInt8 Fill)
{
    memset(Ptr, Fill, Bytes);
}

void SAL_CALL rtl_copyMemory(void *Dst, const void *Src, sal_Size Bytes)
{
    memcpy(Dst, Src, Bytes);
}

void SAL_CALL rtl_moveMemory(void *Dst, const void *Src, sal_Size Bytes)
{
    memmove(Dst, Src, Bytes);
}

sal_Int32 SAL_CALL rtl_compareMemory(const void *MemA, const void *MemB, sal_Size Bytes)
{
    return memcmp(MemA, MemB, Bytes);
}

void* SAL_CALL rtl_findInMemory(const void *MemA, sal_uInt8 ch, sal_Size Bytes)
{
    return memchr(MemA, ch, Bytes);
}


