/*************************************************************************
 *
 *  $RCSfile: sbamultiplex.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-26 14:44:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBA_MULTIPLEX_HXX
#include "sbamultiplex.hxx"
#endif
using namespace dbaui;

//==================================================================
// the listener multiplexers
//==================================================================

// XStatusListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXStatusMultiplexer, ::com::sun::star::frame::XStatusListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXStatusMultiplexer, ::com::sun::star::frame::XStatusListener, statusChanged, ::com::sun::star::frame::FeatureStateEvent)

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

// ::com::sun::star::form::XPositioningListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXPositioningMultiplexer, ::com::sun::star::form::XPositioningListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXPositioningMultiplexer, ::com::sun::star::form::XPositioningListener, positioned, ::com::sun::star::lang::EventObject)

// ::com::sun::star::form::XInsertListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXInsertMultiplexer, ::com::sun::star::form::XInsertListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXInsertMultiplexer, ::com::sun::star::form::XInsertListener, inserting, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXInsertMultiplexer, ::com::sun::star::form::XInsertListener, inserted, ::com::sun::star::lang::EventObject)

// ::com::sun::star::form::XRestoreListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXRestoreMultiplexer, ::com::sun::star::form::XRestoreListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRestoreMultiplexer, ::com::sun::star::form::XRestoreListener, restored, ::com::sun::star::lang::EventObject)

// ::com::sun::star::form::XDeleteListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXDeleteMultiplexer, ::com::sun::star::form::XDeleteListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXDeleteMultiplexer, ::com::sun::star::form::XDeleteListener, approveDelete, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXDeleteMultiplexer, ::com::sun::star::form::XDeleteListener, deleted, ::com::sun::star::lang::EventObject)

// ::com::sun::star::form::XUpdateListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXUpdateMultiplexer, ::com::sun::star::form::XUpdateListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXUpdateMultiplexer, ::com::sun::star::form::XUpdateListener, approveUpdate, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXUpdateMultiplexer, ::com::sun::star::form::XUpdateListener, updated, ::com::sun::star::lang::EventObject)

// ::com::sun::star::form::XErrorListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXErrorMultiplexer, ::com::sun::star::form::XErrorListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXErrorMultiplexer, ::com::sun::star::form::XErrorListener, errorOccured, ::com::sun::star::form::ErrorEvent)

// ::com::sun::star::form::XDatabaseParameterListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXParameterMultiplexer, ::com::sun::star::form::XDatabaseParameterListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXParameterMultiplexer, ::com::sun::star::form::XDatabaseParameterListener, approveParameter, ::com::sun::star::form::DatabaseParameterEvent)

// ::com::sun::star::util::XRefreshListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXRefreshMultiplexer, ::com::sun::star::util::XRefreshListener)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXRefreshMultiplexer, ::com::sun::star::util::XRefreshListener, refreshed, ::com::sun::star::lang::EventObject)

// ::com::sun::star::form::XSubmitListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXSubmitMultiplexer, ::com::sun::star::form::XSubmitListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXSubmitMultiplexer, ::com::sun::star::form::XSubmitListener, approveSubmit, ::com::sun::star::lang::EventObject)

// ::com::sun::star::form::XResetListener
IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener)
IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener, approveReset, ::com::sun::star::lang::EventObject)
IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener, resetted, ::com::sun::star::lang::EventObject)

// ::com::sun::star::beans::XPropertyChangeListener
IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXPropertyChangeMultiplexer, ::com::sun::star::beans::XPropertyChangeListener, propertyChange, ::com::sun::star::beans::PropertyChangeEvent)

// ::com::sun::star::beans::XVetoableChangeListener
IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXVetoableChangeMultiplexer, ::com::sun::star::beans::XVetoableChangeListener, vetoableChange, ::com::sun::star::beans::PropertyChangeEvent)

// ::com::sun::star::beans::XPropertyStateChangeListener
IMPLEMENT_PROPERTY_MULTIPLEXER(SbaXPropertyStateChangeMultiplexer, ::com::sun::star::beans::XPropertyStateChangeListener, propertyStateChange, ::com::sun::star::beans::PropertyStateChangeEvent)

// ::com::sun::star::beans::XPropertiesChangeListener
    IMPLEMENT_LISTENER_MULTIPLEXER_CORE(SbaXPropertiesChangeMultiplexer, ::com::sun::star::beans::XPropertiesChangeListener);
void SbaXPropertiesChangeMultiplexer::propertiesChange(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent>& aEvts)
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

