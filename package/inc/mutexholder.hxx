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



#ifndef __MUTEXHOLDER_HXX_
#define __MUTEXHOLDER_HXX_

#include <osl/mutex.hxx>

class SotMutexHolder
{
    ::osl::Mutex m_aMutex;
    sal_Int32    m_nRefCount;

    public:
    SotMutexHolder() : m_nRefCount( 0 ) {}

    void AddRef()
    {
        m_nRefCount++;
    }

    void ReleaseRef()
    {
        if ( !--m_nRefCount )
            delete this;
    }

    ::osl::Mutex& GetMutex() { return m_aMutex; }
};

class SotMutexHolderRef
{
    SotMutexHolder* m_pHolder;

public:
    SotMutexHolderRef()
    : m_pHolder( NULL )
    {}

    SotMutexHolderRef( SotMutexHolder* pHolder )
    : m_pHolder( pHolder )
    {
        if ( m_pHolder )
            m_pHolder->AddRef();
    }

    SotMutexHolderRef( const SotMutexHolderRef& rRef )
    : m_pHolder( rRef.m_pHolder )
    {
        if ( m_pHolder )
            m_pHolder->AddRef();
    }

    ~SotMutexHolderRef()
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();
    }

    SotMutexHolderRef& operator =( const SotMutexHolderRef& rRef )
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();

        m_pHolder = rRef.m_pHolder;

        if ( m_pHolder )
            m_pHolder->AddRef();

        return *this;
    }

    SotMutexHolderRef& operator =( SotMutexHolder* pHolder )
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();

        m_pHolder = pHolder;

        if ( m_pHolder )
            m_pHolder->AddRef();
        return *this;
    }

    SotMutexHolder* operator ->() const
    {
        return m_pHolder;
    }

    SotMutexHolder& operator *() const
    {
        return *m_pHolder;
    }

    operator SotMutexHolder*() const
    {
        return m_pHolder;
    }

    sal_Bool Is() const
    {
        return m_pHolder != NULL;
    }
};

#endif

