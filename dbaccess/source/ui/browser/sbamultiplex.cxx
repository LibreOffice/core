/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sbamultiplex.hxx"
using namespace dbaui;

// the listener multiplexers

// XStatusListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXStatusMultiplexer, css::frame::XStatusListener)

void SAL_CALL SbaXStatusMultiplexer::statusChanged(const css::frame::FeatureStateEvent& e)
{
    m_aLastKnownStatus = e;
    m_aLastKnownStatus.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper2 aIt( *this );
    while ( aIt.hasMoreElements() )
        static_cast< css::frame::XStatusListener* >( aIt.next() )->statusChanged( m_aLastKnownStatus );
}

// LoadListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXLoadMultiplexer, css::form::XLoadListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, css::form::XLoadListener, loaded, css::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, css::form::XLoadListener, unloaded, css::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, css::form::XLoadListener, unloading, css::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, css::form::XLoadListener, reloading, css::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, css::form::XLoadListener, reloaded, css::lang::EventObject)

// css::sdbc::XRowSetListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXRowSetMultiplexer, css::sdbc::XRowSetListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, css::sdbc::XRowSetListener, cursorMoved, css::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, css::sdbc::XRowSetListener, rowChanged, css::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, css::sdbc::XRowSetListener, rowSetChanged, css::lang::EventObject)

// css::sdb::XRowSetApproveListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXRowSetApproveMultiplexer, css::sdb::XRowSetApproveListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, css::sdb::XRowSetApproveListener, approveCursorMove, css::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, css::sdb::XRowSetApproveListener, approveRowChange, css::sdb::RowChangeEvent)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, css::sdb::XRowSetApproveListener, approveRowSetChange, css::lang::EventObject)

// css::sdb::XSQLErrorListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXSQLErrorMultiplexer, css::sdb::XSQLErrorListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXSQLErrorMultiplexer, css::sdb::XSQLErrorListener, errorOccured, css::sdb::SQLErrorEvent)

// css::form::XDatabaseParameterListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXParameterMultiplexer, css::form::XDatabaseParameterListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXParameterMultiplexer, css::form::XDatabaseParameterListener, approveParameter, css::form::DatabaseParameterEvent)

// css::form::XSubmitListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXSubmitMultiplexer, css::form::XSubmitListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXSubmitMultiplexer, css::form::XSubmitListener, approveSubmit, css::lang::EventObject)

// css::form::XResetListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXResetMultiplexer, css::form::XResetListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXResetMultiplexer, css::form::XResetListener, approveReset, css::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXResetMultiplexer, css::form::XResetListener, resetted, css::lang::EventObject)

// css::beans::XPropertyChangeListener
IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXPropertyChangeMultiplexer, css::beans::XPropertyChangeListener, propertyChange, css::beans::PropertyChangeEvent)

// css::beans::XVetoableChangeListener
IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXVetoableChangeMultiplexer, css::beans::XVetoableChangeListener, vetoableChange, css::beans::PropertyChangeEvent)

// css::beans::XPropertiesChangeListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXPropertiesChangeMultiplexer, css::beans::XPropertiesChangeListener);
void SbaXPropertiesChangeMultiplexer::propertiesChange(const css::uno::Sequence< css::beans::PropertyChangeEvent>& aEvts)
{
    // the SbaXPropertiesChangeMultiplexer doesn't care about the property names a listener logs on for, it simply
    // forwards _all_ changes to _all_ listeners

    css::uno::Sequence< css::beans::PropertyChangeEvent> aMulti(aEvts);
    for (css::beans::PropertyChangeEvent & rEvent : aMulti)
        rEvent.Source = &m_rParent;

    ::comphelper::OInterfaceIteratorHelper2 aIt(*this);
    while (aIt.hasMoreElements())
        static_cast< css::beans::XPropertiesChangeListener*>(aIt.next())->propertiesChange(aMulti);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
