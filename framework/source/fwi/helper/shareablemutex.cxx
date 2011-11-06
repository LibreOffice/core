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
#include "precompiled_framework.hxx"
#include <helper/shareablemutex.hxx>

namespace framework
{

ShareableMutex::ShareableMutex()
{
    pMutexRef = new MutexRef;
    pMutexRef->acquire();
}

ShareableMutex::ShareableMutex( const ShareableMutex& rShareableMutex )
{
    pMutexRef = rShareableMutex.pMutexRef;
    if ( pMutexRef )
        pMutexRef->acquire();
}

const ShareableMutex& ShareableMutex::operator=( const ShareableMutex& rShareableMutex )
{
    if ( rShareableMutex.pMutexRef )
        rShareableMutex.pMutexRef->acquire();
    if ( pMutexRef )
        pMutexRef->release();
    pMutexRef = rShareableMutex.pMutexRef;
    return *this;
}

ShareableMutex::~ShareableMutex()
{
    if ( pMutexRef )
        pMutexRef->release();
}

void ShareableMutex::acquire()
{
    if ( pMutexRef )
        pMutexRef->m_oslMutex.acquire();
}

void ShareableMutex::release()
{
    if ( pMutexRef )
        pMutexRef->m_oslMutex.release();
}

::osl::Mutex& ShareableMutex::getShareableOslMutex()
{
    return pMutexRef->m_oslMutex;
}

}
