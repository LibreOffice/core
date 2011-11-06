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



#ifndef COMPHELPER_COMPONENTGUARD_HXX
#define COMPHELPER_COMPONENTGUARD_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>

//......................................................................................................................
namespace comphelper
{
//......................................................................................................................

    //==================================================================================================================
    //= ComponentGuard
    //==================================================================================================================
    class ComponentGuard
    {
    public:
        ComponentGuard( ::cppu::OWeakObject& i_component, ::cppu::OBroadcastHelper & i_broadcastHelper )
            :m_aGuard( i_broadcastHelper.rMutex )
        {
            if ( i_broadcastHelper.bDisposed )
                throw ::com::sun::star::lang::DisposedException( ::rtl::OUString(), &i_component );
        }

        ~ComponentGuard()
        {
        }

        void clear()    { m_aGuard.clear(); }
        void reset()    { m_aGuard.reset(); }

    private:
        ::osl::ResettableMutexGuard m_aGuard;
    };

//......................................................................................................................
} // namespace comphelper
//......................................................................................................................

#endif // COMPHELPER_COMPONENTGUARD_HXX
