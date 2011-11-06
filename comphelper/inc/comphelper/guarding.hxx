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



#ifndef _COMPHELPER_GUARDING_HXX_
#define _COMPHELPER_GUARDING_HXX_

#include <osl/mutex.hxx>

//.........................................................................
namespace comphelper
{
//.........................................................................

// ===================================================================================================
// = class MutexRelease -
// ===================================================================================================

/** opposite of OGuard :)
    (a mutex is released within the constructor and acquired within the desctructor)
    use only when you're sure the mutex is acquired !
*/
template <class MUTEX>
class ORelease
{
    MUTEX&  m_rMutex;

public:
    ORelease(MUTEX& _rMutex) : m_rMutex(_rMutex) { _rMutex.release(); }
    ~ORelease() { m_rMutex.acquire(); }
};

typedef ORelease< ::osl::Mutex >    MutexRelease;

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_GUARDING_HXX_

