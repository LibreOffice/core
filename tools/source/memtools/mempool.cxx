/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mempool.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:58:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include "mempool.hxx"

#ifndef _RTL_ALLOC_H_
#include "rtl/alloc.h"
#endif

#ifndef INCLUDED_STDIO_H
#include <stdio.h>
#endif

/*************************************************************************
|*
|*    FixedMemPool::FixedMemPool()
|*
*************************************************************************/

FixedMemPool::FixedMemPool (
    USHORT _nTypeSize, USHORT, USHORT)
{
    char name[RTL_CACHE_NAME_LENGTH + 1];
    snprintf (name, sizeof(name), "FixedMemPool_%d", (int)_nTypeSize);
    m_pImpl = (FixedMemPool_Impl*)rtl_cache_create (name, _nTypeSize, 0, NULL, NULL, NULL, 0, NULL, 0);
}

/*************************************************************************
|*
|*    FixedMemPool::~FixedMemPool()
|*
*************************************************************************/

FixedMemPool::~FixedMemPool()
{
    rtl_cache_destroy ((rtl_cache_type*)(m_pImpl));
}

/*************************************************************************
|*
|*    FixedMemPool::Alloc()
|*
*************************************************************************/

void* FixedMemPool::Alloc()
{
    return rtl_cache_alloc ((rtl_cache_type*)(m_pImpl));
}

/*************************************************************************
|*
|*    FixedMemPool::Free()
|*
*************************************************************************/

void FixedMemPool::Free( void* pFree )
{
    rtl_cache_free ((rtl_cache_type*)(m_pImpl), pFree);
}
