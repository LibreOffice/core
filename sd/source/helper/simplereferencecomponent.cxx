/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SALHELPER_SIMPLEREFERENCECOMPONENT_HXX_
#include "helper/simplereferencecomponent.hxx"
#endif

#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/diagnose.h"

#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

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
    osl_incrementInterlockedCount(&m_nCount);
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
            OSL_ENSURE( 0, msg.getStr() );
#endif
        }
    }

    if(osl_decrementInterlockedCount(&m_nCount) == 0) delete this;
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
