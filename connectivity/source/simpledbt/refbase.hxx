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



#ifndef CONNECTIVITY_DBTOOLS_REFBASE_HXX
#define CONNECTIVITY_DBTOOLS_REFBASE_HXX

#include <rtl/ref.hxx>

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= ORefBase
    //================================================================
    /// base class for all objects implementing the ::rtl::IReference interface
    class ORefBase : public ::rtl::IReference
    {
    protected:
        oslInterlockedCount     m_refCount;

    protected:
        ORefBase() : m_refCount(0) { }
        virtual ~ORefBase();

        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_DBTOOLS_REFBASE_HXX


