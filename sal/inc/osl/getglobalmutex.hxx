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



#if !defined INCLUDED_OSL_GETGLOBALMUTEX_HXX
#define INCLUDED_OSL_GETGLOBALMUTEX_HXX

#include "osl/mutex.hxx"

namespace osl {

/** A helper functor for the rtl_Instance template.

    See the rtl_Instance template for examples of how this class is used.
 */
class GetGlobalMutex
{
public:
    ::osl::Mutex * operator()()
    {
        return ::osl::Mutex::getGlobalMutex();
    }
};

}

#endif // INCLUDED_OSL_GETGLOBALMUTEX_HXX
