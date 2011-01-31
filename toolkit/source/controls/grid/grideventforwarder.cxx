/*************************************************************************
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

#include "precompiled_toolkit.hxx"

#include "grideventforwarder.hxx"
#include "gridcontrol.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

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
    using ::com::sun::star::awt::grid::GridDataEvent;
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::lang::EventObject;
    /** === end UNO using === **/

    //==================================================================================================================
    //= GridEventForwarder
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    GridEventForwarder::GridEventForwarder( UnoGridControl& i_parent )
        :m_parent( i_parent )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    GridEventForwarder::~GridEventForwarder()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::acquire() throw()
    {
        m_parent.acquire();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::release() throw()
    {
        m_parent.release();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::rowsInserted( const GridDataEvent& i_event ) throw (RuntimeException)
    {
        Reference< XGridDataListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->rowsInserted( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::rowsRemoved( const GridDataEvent& i_event ) throw (RuntimeException)
    {
        Reference< XGridDataListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->rowsRemoved( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::dataChanged( const GridDataEvent& i_event ) throw (RuntimeException)
    {
        Reference< XGridDataListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->dataChanged( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::rowHeadingChanged( const GridDataEvent& i_event ) throw (RuntimeException)
    {
        Reference< XGridDataListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->rowHeadingChanged( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        Reference< XContainerListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->elementInserted( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        Reference< XContainerListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->elementRemoved( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::elementReplaced( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        Reference< XContainerListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->elementReplaced( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL GridEventForwarder::disposing( const EventObject& i_event ) throw (RuntimeException)
    {
        Reference< XEventListener > xPeer( m_parent.getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->disposing( i_event );
    }

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................
