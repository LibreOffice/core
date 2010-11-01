/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "ContainerListener.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
