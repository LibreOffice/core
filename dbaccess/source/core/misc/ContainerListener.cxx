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
#include "precompiled_dbaccess.hxx"

#ifndef DBA_CONTAINERLISTENER_HXX
#include "ContainerListener.hxx"
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::container::XContainerApproveListener;
    using ::com::sun::star::container::XContainerListener;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::util::XVeto;
    using ::com::sun::star::uno::Reference;
    /** === end UNO using === **/

    //====================================================================
    //= OContainerListener
    //====================================================================
    //--------------------------------------------------------------------
    OContainerListener::~OContainerListener()
    {
    }

    //--------------------------------------------------------------------
    Reference< XVeto > SAL_CALL OContainerListener::approveInsertElement( const ContainerEvent& _Event ) throw (WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        if ( m_bDisposed )
            return NULL;

        return dynamic_cast< XContainerApproveListener& >( m_rDestination ).approveInsertElement(  _Event );
    }

    //--------------------------------------------------------------------
    Reference< XVeto > SAL_CALL OContainerListener::approveReplaceElement( const ContainerEvent& _Event ) throw (WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        if ( m_bDisposed )
            return NULL;

        return dynamic_cast< XContainerApproveListener& >( m_rDestination ).approveReplaceElement(  _Event );
    }

    //--------------------------------------------------------------------
    Reference< XVeto > SAL_CALL OContainerListener::approveRemoveElement( const ContainerEvent& _Event ) throw (WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        if ( m_bDisposed )
            return NULL;

        return dynamic_cast< XContainerApproveListener& >( m_rDestination ).approveRemoveElement(  _Event );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OContainerListener::elementInserted( const ContainerEvent& _Event ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        if ( m_bDisposed )
            return;

        dynamic_cast< XContainerListener& >( m_rDestination ).elementInserted(  _Event );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OContainerListener::elementRemoved( const ContainerEvent& _Event ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        if ( m_bDisposed )
            return;

        dynamic_cast< XContainerListener& >( m_rDestination ).elementRemoved(  _Event );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OContainerListener::elementReplaced( const ContainerEvent& _Event ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        if ( m_bDisposed )
            return;

        dynamic_cast< XContainerListener& >( m_rDestination ).elementReplaced(  _Event );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OContainerListener::disposing( const EventObject& _Source ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        if ( m_bDisposed )
            return;

        dynamic_cast< XContainerListener& >( m_rDestination ).disposing(  _Source );
    }

//........................................................................
}   // namespace dbaccess
//........................................................................
