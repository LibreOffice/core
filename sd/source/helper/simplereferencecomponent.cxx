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


#include "helper/simplereferencecomponent.hxx"

#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/diagnose.h"

#include <new>

using com::sun::star::uno::RuntimeException;
using sd::SimpleReferenceComponent;

SimpleReferenceComponent::SimpleReferenceComponent()
: m_nCount(0)
, mbDisposed(false)
{
}

SimpleReferenceComponent::~SimpleReferenceComponent()
{
    OSL_ASSERT(m_nCount == 0);
    OSL_ASSERT(mbDisposed);
}

void SimpleReferenceComponent::acquire()
{
    osl_atomic_increment(&m_nCount);
}

void SimpleReferenceComponent::release()
{
    if((1 == m_nCount) && !mbDisposed)
    {
        try
        {
            Dispose();
        }
        catch (RuntimeException &
#if OSL_DEBUG_LEVEL > 0
            exc
#endif
            ) 
        {
#if OSL_DEBUG_LEVEL > 0
            OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_FAIL( msg.getStr() );
#endif
        }
    }

    if(osl_atomic_decrement(&m_nCount) == 0) delete this;
}

void SimpleReferenceComponent::Dispose()
{
    if( !mbDisposed )
    {
        mbDisposed = true;
        disposing();
    }
}

void SimpleReferenceComponent::disposing()
{
}

void * SimpleReferenceComponent::operator new(std::size_t nSize)
    SAL_THROW((std::bad_alloc))
{
    return ::operator new(nSize);
}

void * SimpleReferenceComponent::operator new(std::size_t nSize,
                                           std::nothrow_t const &
#ifndef WNT
                                           rNothrow
#endif
                                           )
    SAL_THROW(())
{
#if defined WNT
    return ::operator new(nSize);
        
#else 
    return ::operator new(nSize, rNothrow);
#endif 
}

void SimpleReferenceComponent::operator delete(void * pPtr) SAL_THROW(())
{
    ::operator delete(pPtr);
}

void SimpleReferenceComponent::operator delete(void * pPtr,
                                            std::nothrow_t const &
#ifndef WNT
                                            rNothrow
#endif
)
    SAL_THROW(())
{
#if defined WNT
    ::operator delete(pPtr); 
#else 
    ::operator delete(pPtr, rNothrow);
#endif 
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
