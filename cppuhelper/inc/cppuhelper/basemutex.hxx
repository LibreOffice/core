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



#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#define _CPPUHELPER_BASEMUTEX_HXX_

#include <osl/mutex.hxx>

namespace cppu
{
    /** base class for all classes who want derive from
     ::cppu::WeakComponentImplHelperXX.

     Implmentation classes have first to derive from BaseMutex and then from
     ::cppu::WeakComponentImplHelperXX to ensure that the BaseMutex is completely
     initialized when the mutex is used to intialize the
     ::cppu::WeakComponentImplHelperXX
    */
    class BaseMutex
    {
    protected:
        mutable ::osl::Mutex m_aMutex;
    };
}
#endif // _CPPUHELPER_BASEMUTEX_HXX_
