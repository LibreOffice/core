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
#include "precompiled_forms.hxx"

#include "resettable.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <cppuhelper/weak.hxx>

//........................................................................
namespace frm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::form::XResetListener;
    using ::com::sun::star::lang::EventObject;
    /** === end UNO using === **/

    //====================================================================
    //= ResetHelper
    //====================================================================
    //--------------------------------------------------------------------
    void ResetHelper::addResetListener( const Reference< XResetListener >& _listener )
    {
        m_aResetListeners.addInterface( _listener );
    }

    //--------------------------------------------------------------------
    void ResetHelper::removeResetListener( const Reference< XResetListener >& _listener )
    {
        m_aResetListeners.removeInterface( _listener );
    }

    //--------------------------------------------------------------------
    bool ResetHelper::approveReset()
    {
        ::cppu::OInterfaceIteratorHelper aIter( m_aResetListeners );
        EventObject aResetEvent( m_rParent );

        sal_Bool bContinue = sal_True;
        while ( aIter.hasMoreElements() && bContinue )
            bContinue = static_cast< XResetListener* >( aIter.next() )->approveReset( aResetEvent );

        return bContinue;
    }

    //--------------------------------------------------------------------
    void ResetHelper::notifyResetted()
    {
        EventObject aResetEvent( m_rParent );
        m_aResetListeners.notifyEach( &XResetListener::resetted, aResetEvent );
    }

    //--------------------------------------------------------------------
    void ResetHelper::disposing()
    {
        EventObject aEvent( m_rParent );
        m_aResetListeners.disposeAndClear( aEvent );
    }

//........................................................................
} // namespace frm
//........................................................................
