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
            ) // don't break throw ()
        {
#if OSL_DEBUG_LEVEL > 0
            rtl::OString msg( rtl::OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
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
        // WNT lacks a global nothrow operator new...
#else // WNT
    return ::operator new(nSize, rNothrow);
#endif // WNT
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
    ::operator delete(pPtr); // WNT lacks a global nothrow operator delete...
#else // WNT
    ::operator delete(pPtr, rNothrow);
#endif // WNT
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
