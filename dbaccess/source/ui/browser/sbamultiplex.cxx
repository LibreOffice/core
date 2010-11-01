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

#include "sbamultiplex.hxx"
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
