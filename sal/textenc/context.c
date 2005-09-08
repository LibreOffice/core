/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: context.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:27:53 $
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

#ifndef INCLUDED_RTL_TEXTENC_CONTEXT_H
#include "context.h"
#endif

#ifndef _RTL_ALLOC_H_
#include "rtl/alloc.h"
#endif

void * ImplCreateUnicodeToTextContext(void)
{
    void * pContext = rtl_allocateMemory(sizeof (ImplUnicodeToTextContext));
    ((ImplUnicodeToTextContext *) pContext)->m_nHighSurrogate = 0;
    return pContext;
}

void ImplResetUnicodeToTextContext(void * pContext)
{
    if (pContext)
        ((ImplUnicodeToTextContext *) pContext)->m_nHighSurrogate = 0;
}

void ImplDestroyContext(void * pContext)
{
    rtl_freeMemory(pContext);
}
