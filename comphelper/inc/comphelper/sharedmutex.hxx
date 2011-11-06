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



#ifndef COMPHELPER_SHAREDMUTEX_HXX
#define COMPHELPER_SHAREDMUTEX_HXX

#include "comphelper/comphelperdllapi.h"

#include <osl/mutex.hxx>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace comphelper
{
//........................................................................

    //============================================================
    //= SharedMutex
    //============================================================
    class COMPHELPER_DLLPUBLIC SharedMutex
    {
    public:
        SharedMutex();
        SharedMutex( const SharedMutex& );
        SharedMutex& operator=( const SharedMutex& );
        ~SharedMutex()
        {
        }

        inline ::osl::Mutex& getMutex() { return *m_pMutexImpl; }
        inline operator ::osl::Mutex& () { return *m_pMutexImpl; }

    private:
        ::boost::shared_ptr< ::osl::Mutex >  m_pMutexImpl;
    };

    //============================================================
    //= SharedMutexBase
    //============================================================
    /** sometimes, it's necessary to have an initialized ::osl::Mutex to pass
        to some ctor call of your base class. In this case, you can't hold the
        SharedMutex as member, but you need to move it into another base class,
        which is initialized before the mutex-requiring class is.
    */
    class COMPHELPER_DLLPUBLIC SharedMutexBase
    {
    protected:
        SharedMutexBase()
        {
        }
        ~SharedMutexBase()
        {
        }

    protected:
        ::osl::Mutex&   getMutex() const { return m_aMutex; }
        SharedMutex&    getSharedMutex() const { return m_aMutex; }

    private:
        mutable SharedMutex m_aMutex;
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_SHAREDMUTEX_HXX
