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
#include "precompiled_comphelper.hxx"

#include "comphelper/componentbase.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::lang::NotInitializedException;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    /** === end UNO using === **/

    //====================================================================
    //= ComponentBase
    //====================================================================
    //--------------------------------------------------------------------
    void ComponentBase::impl_checkDisposed_throw() const
    {
        if ( m_rBHelper.bDisposed )
            throw DisposedException( ::rtl::OUString(), getComponent() );
    }

    //--------------------------------------------------------------------
    void ComponentBase::impl_checkInitialized_throw() const
    {
        if ( !m_bInitialized )
            throw NotInitializedException( ::rtl::OUString(), getComponent() );
    }

    //--------------------------------------------------------------------
    Reference< XInterface > ComponentBase::getComponent() const
    {
        return NULL;
    }

//........................................................................
} // namespace comphelper
//........................................................................
