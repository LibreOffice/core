/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: aststack.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"
#include <rtl/alloc.h>
#include <idlc/aststack.hxx>
#include <idlc/astscope.hxx>

#define STACKSIZE_INCREMENT 64

AstStack::AstStack()
    : m_stack((AstScope**)rtl_allocateZeroMemory(sizeof(AstScope*) * STACKSIZE_INCREMENT))
    , m_size(STACKSIZE_INCREMENT)
    , m_top(0)
{
}

AstStack::~AstStack()
{
    for(sal_uInt32 i=0; i < m_top; i++)
    {
        if (m_stack[i])
            delete(m_stack[i]);
    }

    rtl_freeMemory(m_stack);
}

sal_uInt32 AstStack::depth()
{
    return m_top;
}

AstScope* AstStack::top()
{
    if (m_top < 1)
        return NULL;
    return m_stack[m_top - 1];
}

AstScope* AstStack::bottom()
{
    if (m_top == 0)
        return NULL;
    return m_stack[0];
}

AstScope* AstStack::nextToTop()
{
    AstScope *tmp, *retval;

    if (depth() < 2)
        return NULL;

    tmp = top();        // Save top
    (void) pop();       // Pop it
    retval = top();     // Get next one down
    (void) push(tmp);   // Push top back
    return retval;      // Return next one down
}

AstScope* AstStack::topNonNull()
{
    for (sal_uInt32 i = m_top; i > 0; i--)
    {
        if ( m_stack[i - 1] )
            return m_stack[i - 1];
      }
    return NULL;
}

AstStack* AstStack::push(AstScope* pScope)
{
    AstScope        **tmp;
//  AstDeclaration  *pDecl = ScopeAsDecl(pScope);
    sal_uInt32  newSize;
    sal_uInt32  i;

    // Make sure there's space for one more
    if (m_size == m_top)
    {
        newSize = m_size;
        newSize += STACKSIZE_INCREMENT;
        tmp = (AstScope**)rtl_allocateZeroMemory(sizeof(AstScope*) * newSize);

        for(i=0; i < m_size; i++)
            tmp[i] = m_stack[i];

        rtl_freeMemory(m_stack);
        m_stack = tmp;
    }

    // Insert new scope
    m_stack[m_top++] = pScope;

    return this;
}

void AstStack::pop()
{
    AstScope *pScope;

    if (m_top < 1)
        return;
    pScope = m_stack[--m_top];
}

void AstStack::clear()
{
    m_top = 0;
}

