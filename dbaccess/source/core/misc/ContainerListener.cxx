/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContainerListener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:16:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
