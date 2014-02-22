/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

    tmp = top();        
    (void) pop();       
    retval = top();     
    (void) push(tmp);   
    return retval;      
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

    sal_uInt32  newSize;
    sal_uInt32  i;

    
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

    
    m_stack[m_top++] = pScope;

    return this;
}

void AstStack::pop()
{
    if (m_top < 1)
        return;
    --m_top;
}

void AstStack::clear()
{
    m_top = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
