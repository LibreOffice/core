/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sbamultiplex.hxx"
using namespace dbaui;




IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXStatusMultiplexer, ::com::sun::star::frame::XStatusListener)

void SAL_CALL SbaXStatusMultiplexer::statusChanged(const ::com::sun::star::frame::FeatureStateEvent& e) throw (::com::sun::star::uno::RuntimeException)
{
    m_aLastKnownStatus = e;
    m_aLastKnownStatus.Source = &m_rParent;
    ::cppu::OInterfaceIteratorHelper aIt( *this );
    while ( aIt.hasMoreElements() )
        static_cast< ::com::sun::star::frame::XStatusListener* >( aIt.next() )->statusChanged( m_aLastKnownStatus );
}                                                                                       \


IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, loaded, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, unloaded, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, unloading, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, reloading, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener, reloaded, ::com::sun::star::lang::EventObject)


IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener, cursorMoved, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener, rowChanged, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener, rowSetChanged, ::com::sun::star::lang::EventObject)


IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener, approveCursorMove, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener, approveRowChange, ::com::sun::star::sdb::RowChangeEvent)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener, approveRowSetChange, ::com::sun::star::lang::EventObject)


IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXSQLErrorMultiplexer, ::com::sun::star::sdb::XSQLErrorListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXSQLErrorMultiplexer, ::com::sun::star::sdb::XSQLErrorListener, errorOccured, ::com::sun::star::sdb::SQLErrorEvent)


IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXParameterMultiplexer, ::com::sun::star::form::XDatabaseParameterListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXParameterMultiplexer, ::com::sun::star::form::XDatabaseParameterListener, approveParameter, ::com::sun::star::form::DatabaseParameterEvent)


IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXSubmitMultiplexer, ::com::sun::star::form::XSubmitListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXSubmitMultiplexer, ::com::sun::star::form::XSubmitListener, approveSubmit, ::com::sun::star::lang::EventObject)


IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener, approveReset, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener, resetted, ::com::sun::star::lang::EventObject)


IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXPropertyChangeMultiplexer, ::com::sun::star::beans::XPropertyChangeListener, propertyChange, ::com::sun::star::beans::PropertyChangeEvent, (::com::sun::star::uno::RuntimeException))


IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXVetoableChangeMultiplexer, ::com::sun::star::beans::XVetoableChangeListener, vetoableChange, ::com::sun::star::beans::PropertyChangeEvent, (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException))


IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXPropertiesChangeMultiplexer, ::com::sun::star::beans::XPropertiesChangeListener);
void SbaXPropertiesChangeMultiplexer::propertiesChange(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent>& aEvts) throw(::com::sun::star::uno::RuntimeException)
{
    
    

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent> aMulti(aEvts);
    ::com::sun::star::beans::PropertyChangeEvent* pMulti = aMulti.getArray();
    for (sal_uInt16 i=0; i<aMulti.getLength(); ++i, ++pMulti)
        pMulti->Source = &m_rParent;

    ::cppu::OInterfaceIteratorHelper aIt(*this);
    while (aIt.hasMoreElements())
        ((::com::sun::star::beans::XPropertiesChangeListener*)aIt.next())->propertiesChange(aMulti);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
