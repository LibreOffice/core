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



#ifndef __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_
#define __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <fwidllapi.h>

namespace framework
{

class FWI_DLLPUBLIC ShareableMutex
{
    public:
        ShareableMutex();
        ShareableMutex( const ShareableMutex& rShareableMutex );
        const ShareableMutex& operator=( const ShareableMutex& rShareableMutex );

        ~ShareableMutex();

        void acquire();
        void release();
        ::osl::Mutex& getShareableOslMutex();

    private:
        struct MutexRef
        {
            MutexRef() : m_refCount(0) {}
            void acquire()
            {
                osl_incrementInterlockedCount( &m_refCount );
            }

            void release()
            {
                if ( osl_decrementInterlockedCount( &m_refCount ) == 0 )
                    delete this;
            }

            oslInterlockedCount m_refCount;
            osl::Mutex          m_oslMutex;
        };

        MutexRef* pMutexRef;
};

class ShareGuard
{
    public:
        ShareGuard( ShareableMutex& rShareMutex ) :
            m_rShareMutex( rShareMutex )
        {
            m_rShareMutex.acquire();
        }

        ~ShareGuard()
        {
            m_rShareMutex.release();
        }

    private:
        ShareGuard();
        ShareGuard& operator=( const ShareGuard& );

        ShareableMutex& m_rShareMutex;
};

}

#endif // #ifndef __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_
