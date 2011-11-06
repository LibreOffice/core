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
#include "precompiled_stoc.hxx"

#include "osl/doublecheckedlocking.h"
#include "base.hxx"

namespace stoc_rdbtdp
{

//------------------------------------------------------------------------------
::osl::Mutex & getMutex()
{
    static ::osl::Mutex * s_pmutex = 0;
    if (s_pmutex == 0)
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if (s_pmutex == 0)
        {
            static ::osl::Mutex s_mutex;
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            s_pmutex = &s_mutex;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return *s_pmutex;
}


TypeDescriptionImpl::~TypeDescriptionImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass TypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _eTypeClass;
}
//__________________________________________________________________________________________________
OUString TypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

}


