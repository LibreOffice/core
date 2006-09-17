/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbamultiplex.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:59:12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef _SBA_MULTIPLEX_HXX
#include "sbamultiplex.hxx"
#endif
using namespace dbaui;

//==================================================================
// the listener multiplexers
//==================================================================

// XStatusListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXStatusMultiplexer, ::com::sun::star::frame::XStatusListener)

void SAL_CALL SbaXStatusMultiplexer::statusChanged(const ::com::sun::star::frame::FeatureStateEvent& e) throw (::com::sun::star::uno::RuntimeException)
{
    m_aLastKnownStatus = e;
    m_aLastKnownStatus.Source = &m_rParent;
    ::cppu::OInterfaceIteratorHelper aIt( *this );
    while ( aIt.hasMoreElements() )
        static_cast< ::com::sun::star::frame::XStatusListener* >( aIt.next() )->statusChanged( m_aLastKnownStatus );
}                                                                                       \

// LoadListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, loaded, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, unloaded, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, unloading, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, reloading, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, reloaded, ::com::sun::star::lang::EventObject)

// ::com::sun::star::sdbc::XRowSetListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener, cursorMoved, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener, rowChanged, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener, rowSetChanged, ::com::sun::star::lang::EventObject)

// ::com::sun::star::sdb::XRowSetApproveListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener, approveCursorMove, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener, approveRowChange, ::com::sun::star::sdb::RowChangeEvent)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener, approveRowSetChange, ::com::sun::star::lang::EventObject)

// ::com::sun::star::sdb::XSQLErrorListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXSQLErrorMultiplexer, ::com::sun::star::sdb::XSQLErrorListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXSQLErrorMultiplexer, ::com::sun::star::sdb::XSQLErrorListener, errorOccured, ::com::sun::star::sdb::SQLErrorEvent)

// ::com::sun::star::form::XDatabaseParameterListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXParameterMultiplexer, ::com::sun::star::form::XDatabaseParameterListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXParameterMultiplexer, ::com::sun::star::form::XDatabaseParameterListener, approveParameter, ::com::sun::star::form::DatabaseParameterEvent)

// ::com::sun::star::form::XSubmitListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXSubmitMultiplexer, ::com::sun::star::form::XSubmitListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXSubmitMultiplexer, ::com::sun::star::form::XSubmitListener, approveSubmit, ::com::sun::star::lang::EventObject)

// ::com::sun::star::form::XResetListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener, approveReset, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener, resetted, ::com::sun::star::lang::EventObject)

// ::com::sun::star::beans::XPropertyChangeListener
IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXPropertyChangeMultiplexer, ::com::sun::star::beans::XPropertyChangeListener, propertyChange, ::com::sun::star::beans::PropertyChangeEvent, (::com::sun::star::uno::RuntimeException))

// ::com::sun::star::beans::XVetoableChangeListener
IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXVetoableChangeMultiplexer, ::com::sun::star::beans::XVetoableChangeListener, vetoableChange, ::com::sun::star::beans::PropertyChangeEvent, (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException))

// ::com::sun::star::beans::XPropertiesChangeListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXPropertiesChangeMultiplexer, ::com::sun::star::beans::XPropertiesChangeListener);
void SbaXPropertiesChangeMultiplexer::propertiesChange(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent>& aEvts) throw(::com::sun::star::uno::RuntimeException)
{
    // the SbaXPropertiesChangeMultiplexer doesn't care about the property names a listener logs on for, it simply
    // forwards _all_ changes to _all_ listeners

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent> aMulti(aEvts);
    ::com::sun::star::beans::PropertyChangeEvent* pMulti = aMulti.getArray();
    for (sal_uInt16 i=0; i<aMulti.getLength(); ++i, ++pMulti)
        pMulti->Source = &m_rParent;

    ::cppu::OInterfaceIteratorHelper aIt(*this);
    while (aIt.hasMoreElements())
        ((::com::sun::star::beans::XPropertiesChangeListener*)aIt.next())->propertiesChange(aMulti);
}

