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



#include "salhelper/simplereferenceobject.hxx"
#include "osl/diagnose.h"

#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

using salhelper::SimpleReferenceObject;

SimpleReferenceObject::~SimpleReferenceObject() SAL_THROW(())
{
    OSL_ASSERT(m_nCount == 0);
}

void * SimpleReferenceObject::operator new(std::size_t nSize)
    SAL_THROW((std::bad_alloc))
{
    return ::operator new(nSize);
}

void * SimpleReferenceObject::operator new(std::size_t nSize,
                                           std::nothrow_t const &)
    SAL_THROW(())
{
#if defined WNT
    return ::operator new(nSize);
        // WNT lacks a global nothrow operator new...
#else // WNT
    return ::operator new(nSize, std::nothrow);
#endif // WNT
}

void SimpleReferenceObject::operator delete(void * pPtr) SAL_THROW(())
{
    ::operator delete(pPtr);
}

void SimpleReferenceObject::operator delete(void * pPtr, std::nothrow_t const &)
    SAL_THROW(())
{
#if defined WNT
    ::operator delete(pPtr); // WNT lacks a global nothrow operator delete...
#else // WNT
    ::operator delete(pPtr, std::nothrow);
#endif // WNT
}

void SimpleReferenceObject::operator delete[](void * pPtr)
    SAL_THROW(())
{
    ::operator delete[](pPtr);
}
