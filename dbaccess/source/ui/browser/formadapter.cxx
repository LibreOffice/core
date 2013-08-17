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

#include "formadapter.hxx"
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <comphelper/types.hxx>
#include <comphelper/enumhelper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "dbu_brw.hrc"
#include "dbustrings.hrc"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

// SbaXFormAdapter

DBG_NAME(SbaXFormAdapter)
SbaXFormAdapter::SbaXFormAdapter()
            :m_aLoadListeners(*this, m_aMutex)
            ,m_aRowSetListeners(*this, m_aMutex)
            ,m_aRowSetApproveListeners(*this, m_aMutex)
            ,m_aErrorListeners(*this, m_aMutex)
            ,m_aParameterListeners(*this, m_aMutex)
            ,m_aSubmitListeners(*this, m_aMutex)
            ,m_aResetListeners(*this, m_aMutex)
            ,m_aPropertyChangeListeners(*this, m_aMutex)
            ,m_aVetoablePropertyChangeListeners(*this, m_aMutex)
            ,m_aPropertiesChangeListeners(*this, m_aMutex)
            ,m_aDisposeListeners(m_aMutex)
            ,m_aContainerListeners(m_aMutex)
            ,m_nNamePropHandle(-1)
{
    DBG_CTOR(SbaXFormAdapter,NULL);

}

SbaXFormAdapter::~SbaXFormAdapter()
{

    DBG_DTOR(SbaXFormAdapter,NULL);
}

Sequence< Type > SAL_CALL SbaXFormAdapter::getTypes(  ) throw (RuntimeException)
{
    return ::comphelper::concatSequences(
        SbaXFormAdapter_BASE1::getTypes(),
        SbaXFormAdapter_BASE2::getTypes(),
        SbaXFormAdapter_BASE3::getTypes()
    );
}

Sequence< sal_Int8 > SAL_CALL SbaXFormAdapter::getImplementationId(  ) throw (RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

Any SAL_CALL SbaXFormAdapter::queryInterface(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = SbaXFormAdapter_BASE1::queryInterface( _rType );

    if (!aReturn.hasValue())
        aReturn = SbaXFormAdapter_BASE2::queryInterface( _rType );

    if (!aReturn.hasValue())
        aReturn = SbaXFormAdapter_BASE3::queryInterface( _rType );

    return aReturn;
}

void SbaXFormAdapter::StopListening()
{
    // log off all our multiplexers
    STOP_MULTIPLEXER_LISTENING(LoadListener, m_aLoadListeners, ::com::sun::star::form::XLoadable, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(RowSetListener, m_aRowSetListeners, ::com::sun::star::sdbc::XRowSet, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(RowSetApproveListener, m_aRowSetApproveListeners, ::com::sun::star::sdb::XRowSetApproveBroadcaster, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(SQLErrorListener, m_aErrorListeners, ::com::sun::star::sdb::XSQLErrorBroadcaster, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(SubmitListener, m_aSubmitListeners, ::com::sun::star::form::XSubmit, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(ResetListener, m_aResetListeners, ::com::sun::star::form::XReset, m_xMainForm);

    if (m_aParameterListeners.getLength())
    {
        Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeParameterListener(&m_aParameterListeners);
    }

    STOP_PROPERTY_MULTIPLEXER_LISTENING(PropertyChangeListener, m_aPropertyChangeListeners, ::com::sun::star::beans::XPropertySet, m_xMainForm);
    STOP_PROPERTY_MULTIPLEXER_LISTENING(VetoableChangeListener, m_aVetoablePropertyChangeListeners, ::com::sun::star::beans::XPropertySet, m_xMainForm);
    if (m_aPropertiesChangeListeners.getLength())
    {
        Reference< ::com::sun::star::beans::XMultiPropertySet >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removePropertiesChangeListener(&m_aPropertiesChangeListeners);
    }

    // log off ourself
    Reference< ::com::sun::star::lang::XComponent >  xComp(m_xMainForm, UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener((::com::sun::star::lang::XEventListener*)(::com::sun::star::beans::XPropertyChangeListener*)this);
}

void SbaXFormAdapter::StartListening()
{
    // log off all our multiplexers
    START_MULTIPLEXER_LISTENING(LoadListener, m_aLoadListeners, ::com::sun::star::form::XLoadable, m_xMainForm);
    START_MULTIPLEXER_LISTENING(RowSetListener, m_aRowSetListeners, ::com::sun::star::sdbc::XRowSet, m_xMainForm);
    START_MULTIPLEXER_LISTENING(RowSetApproveListener, m_aRowSetApproveListeners, ::com::sun::star::sdb::XRowSetApproveBroadcaster, m_xMainForm);
    START_MULTIPLEXER_LISTENING(SQLErrorListener, m_aErrorListeners, ::com::sun::star::sdb::XSQLErrorBroadcaster, m_xMainForm);
    START_MULTIPLEXER_LISTENING(SubmitListener, m_aSubmitListeners, ::com::sun::star::form::XSubmit, m_xMainForm);
    START_MULTIPLEXER_LISTENING(ResetListener, m_aResetListeners, ::com::sun::star::form::XReset, m_xMainForm);

    if (m_aParameterListeners.getLength())
    {
        Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addParameterListener(&m_aParameterListeners);
    }

    START_PROPERTY_MULTIPLEXER_LISTENING(PropertyChangeListener, m_aPropertyChangeListeners, ::com::sun::star::beans::XPropertySet, m_xMainForm);
    START_PROPERTY_MULTIPLEXER_LISTENING(VetoableChangeListener, m_aVetoablePropertyChangeListeners, ::com::sun::star::beans::XPropertySet, m_xMainForm);
    if (m_aPropertiesChangeListeners.getLength())
    {
        Reference< ::com::sun::star::beans::XMultiPropertySet >  xBroadcaster(m_xMainForm, UNO_QUERY);
        OUString sEmpty;
        if (xBroadcaster.is())
            xBroadcaster->addPropertiesChangeListener(::comphelper::StringSequence(&sEmpty, 1), &m_aPropertiesChangeListeners);
    }

    // log off ourself
    Reference< ::com::sun::star::lang::XComponent >  xComp(m_xMainForm, UNO_QUERY);
    if (xComp.is())
        xComp->addEventListener((::com::sun::star::lang::XEventListener*)(::com::sun::star::beans::XPropertyChangeListener*)this);
}

void SbaXFormAdapter::AttachForm(const Reference< ::com::sun::star::sdbc::XRowSet >& xNewMaster)
{
    if (xNewMaster == m_xMainForm)
        return;

    OSL_ENSURE(xNewMaster.get() != static_cast< ::com::sun::star::sdbc::XRowSet* >(this), "SbaXFormAdapter::AttachForm : invalid argument !");

    if (m_xMainForm.is())
    {
        StopListening();

        // if our old master is loaded we have to send an 'unloaded' event
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_xMainForm, UNO_QUERY);
        if (xLoadable->isLoaded())
        {
            ::com::sun::star::lang::EventObject aEvt(*this);
            ::cppu::OInterfaceIteratorHelper aIt(m_aLoadListeners);
            while (aIt.hasMoreElements())
                ((::com::sun::star::form::XLoadListener*)aIt.next())->unloaded(aEvt);
        }
    }

    m_xMainForm = xNewMaster;

    if (m_xMainForm.is())
    {
        StartListening();

        // if our new master is loaded we have to send an 'loaded' event
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_xMainForm, UNO_QUERY);
        if (xLoadable->isLoaded())
        {
            ::com::sun::star::lang::EventObject aEvt(*this);
            ::cppu::OInterfaceIteratorHelper aIt(m_aLoadListeners);
            while (aIt.hasMoreElements())
                ((::com::sun::star::form::XLoadListener*)aIt.next())->loaded(aEvt);
        }
    }

    // TODO : perhaps _all_ of our listeners should be notified about our new state
    // (nearly every aspect of us may have changed with new master form)
}

// ::com::sun::star::sdbc::XCloseable
void SAL_CALL SbaXFormAdapter::close() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XCloseable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->close();
}

// ::com::sun::star::sdbc::XResultSetMetaDataSupplier
Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL SbaXFormAdapter::getMetaData() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XResultSetMetaDataSupplier >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getMetaData();
    return Reference< ::com::sun::star::sdbc::XResultSetMetaData > ();
}

// ::com::sun::star::sdbc::XColumnLocate
sal_Int32 SAL_CALL SbaXFormAdapter::findColumn(const OUString& columnName) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XColumnLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->findColumn(columnName);
    return 0;
}

// ::com::sun::star::sdbcx::XColumnsSupplier
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL SbaXFormAdapter::getColumns() throw( RuntimeException )
{
    Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getColumns();
    return Reference< ::com::sun::star::container::XNameAccess > ();
}

// ::com::sun::star::sdbc::XRow
sal_Bool SAL_CALL SbaXFormAdapter::wasNull() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->wasNull();
    return sal_True;
}

OUString SAL_CALL SbaXFormAdapter::getString(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getString(columnIndex);
    return OUString();
}

sal_Bool SAL_CALL SbaXFormAdapter::getBoolean(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBoolean(columnIndex);
    return sal_False;
}

sal_Int8 SAL_CALL SbaXFormAdapter::getByte(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )

{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getByte(columnIndex);
    return 0;
}

sal_Int16 SAL_CALL SbaXFormAdapter::getShort(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getShort(columnIndex);
    return 0;
}

sal_Int32 SAL_CALL SbaXFormAdapter::getInt(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getInt(columnIndex);
    return 0;
}

sal_Int64 SAL_CALL SbaXFormAdapter::getLong(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getLong(columnIndex);
    return 0;
}

float SAL_CALL SbaXFormAdapter::getFloat(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getFloat(columnIndex);
    return 0.0;
}

double SAL_CALL SbaXFormAdapter::getDouble(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getDouble(columnIndex);
    return 0.0;
}

Sequence< sal_Int8 > SAL_CALL SbaXFormAdapter::getBytes(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBytes(columnIndex);
    return Sequence <sal_Int8> ();
}

::com::sun::star::util::Date SAL_CALL SbaXFormAdapter::getDate(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getDate(columnIndex);
    return ::com::sun::star::util::Date();
}

::com::sun::star::util::Time SAL_CALL SbaXFormAdapter::getTime(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getTime(columnIndex);
    return ::com::sun::star::util::Time();
}

::com::sun::star::util::DateTime SAL_CALL SbaXFormAdapter::getTimestamp(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getTimestamp(columnIndex);
    return ::com::sun::star::util::DateTime();
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL SbaXFormAdapter::getBinaryStream(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBinaryStream(columnIndex);
    return Reference< ::com::sun::star::io::XInputStream > ();
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL SbaXFormAdapter::getCharacterStream(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getCharacterStream(columnIndex);
    return Reference< ::com::sun::star::io::XInputStream > ();
}

Any SAL_CALL SbaXFormAdapter::getObject(sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getObject(columnIndex, typeMap);
    return Any();
}

Reference< ::com::sun::star::sdbc::XRef > SAL_CALL SbaXFormAdapter::getRef(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getRef(columnIndex);
    return Reference< ::com::sun::star::sdbc::XRef > ();
}

Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL SbaXFormAdapter::getBlob(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBlob(columnIndex);
    return Reference< ::com::sun::star::sdbc::XBlob > ();
}

Reference< ::com::sun::star::sdbc::XClob > SAL_CALL SbaXFormAdapter::getClob(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getClob(columnIndex);
    return Reference< ::com::sun::star::sdbc::XClob > ();
}

Reference< ::com::sun::star::sdbc::XArray > SAL_CALL SbaXFormAdapter::getArray(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getArray(columnIndex);
    return Reference< ::com::sun::star::sdbc::XArray > ();
}

// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL SbaXFormAdapter::getBookmark() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBookmark();
    return Any();
}

sal_Bool SAL_CALL SbaXFormAdapter::moveToBookmark(const Any& bookmark) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->moveToBookmark(bookmark);
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::moveRelativeToBookmark(const Any& bookmark, sal_Int32 rows) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->moveRelativeToBookmark(bookmark,rows);
    return sal_False;
}

sal_Int32 SAL_CALL SbaXFormAdapter::compareBookmarks(const Any& _first, const Any& _second) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->compareBookmarks(_first, _second);
    return 0;
}

sal_Bool SAL_CALL SbaXFormAdapter::hasOrderedBookmarks() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->hasOrderedBookmarks();
    return sal_False;
}

sal_Int32 SAL_CALL SbaXFormAdapter::hashBookmark(const Any& bookmark) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->hashBookmark(bookmark);
    return 0;
}

// ::com::sun::star::sdbc::XRowUpdate
void SAL_CALL SbaXFormAdapter::updateNull(sal_Int32 columnIndex) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateNull(columnIndex);
}

void SAL_CALL SbaXFormAdapter::updateBoolean(sal_Int32 columnIndex, sal_Bool x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateBoolean(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateByte(sal_Int32 columnIndex, sal_Int8 x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateByte(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateShort(sal_Int32 columnIndex, sal_Int16 x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateShort(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateInt(sal_Int32 columnIndex, sal_Int32 x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateInt(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateLong(sal_Int32 columnIndex, sal_Int64 x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateLong(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateFloat(sal_Int32 columnIndex, float x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateFloat(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateDouble(sal_Int32 columnIndex, double x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateDouble(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateString(sal_Int32 columnIndex, const OUString& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateString(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateBytes(sal_Int32 columnIndex, const Sequence< sal_Int8 >& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateBytes(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateDate(sal_Int32 columnIndex, const ::com::sun::star::util::Date& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateDate(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateTime(sal_Int32 columnIndex, const ::com::sun::star::util::Time& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateTime(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateTimestamp(sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateTimestamp(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateBinaryStream(sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateBinaryStream(columnIndex, x, length);
}

void SAL_CALL SbaXFormAdapter::updateCharacterStream(sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateCharacterStream(columnIndex, x, length);
}

void SAL_CALL SbaXFormAdapter::updateObject(sal_Int32 columnIndex, const Any& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateObject(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateNumericObject(sal_Int32 columnIndex, const Any& x, sal_Int32 scale) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateNumericObject(columnIndex, x, scale);
}

// ::com::sun::star::sdbc::XResultSet
sal_Bool SAL_CALL SbaXFormAdapter::next() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->next();
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::isBeforeFirst() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->isBeforeFirst();
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::isAfterLast() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->isAfterLast();
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::isFirst() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->isFirst();
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::isLast() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->isLast();
    return sal_False;
}

void SAL_CALL SbaXFormAdapter::beforeFirst() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        m_xMainForm->beforeFirst();
}

void SAL_CALL SbaXFormAdapter::afterLast() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        m_xMainForm->afterLast();
}

sal_Bool SAL_CALL SbaXFormAdapter::first() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->first();
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::last() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->last();
    return sal_False;
}

sal_Int32 SAL_CALL SbaXFormAdapter::getRow() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->getRow();
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::absolute(sal_Int32 row) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->absolute(row);
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::relative(sal_Int32 rows) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->relative(rows);
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::previous() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->previous();
    return sal_False;
}

void SAL_CALL SbaXFormAdapter::refreshRow() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        m_xMainForm->refreshRow();
}

sal_Bool SAL_CALL SbaXFormAdapter::rowUpdated() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->rowUpdated();
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::rowInserted() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->rowInserted();
    return sal_False;
}

sal_Bool SAL_CALL SbaXFormAdapter::rowDeleted() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->rowDeleted();
    return sal_False;
}

Reference< XInterface > SAL_CALL SbaXFormAdapter::getStatement() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        return m_xMainForm->getStatement();
    return NULL;
}

// ::com::sun::star::sdbc::XResultSetUpdate
void SAL_CALL SbaXFormAdapter::insertRow() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->insertRow();
}

void SAL_CALL SbaXFormAdapter::updateRow() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateRow();
}

void SAL_CALL SbaXFormAdapter::deleteRow() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->deleteRow();
}

void SAL_CALL SbaXFormAdapter::cancelRowUpdates() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->cancelRowUpdates();
}

void SAL_CALL SbaXFormAdapter::moveToInsertRow() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->moveToInsertRow();
}

void SAL_CALL SbaXFormAdapter::moveToCurrentRow() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->moveToCurrentRow();
}

// ::com::sun::star::sdbc::XRowSet
void SAL_CALL SbaXFormAdapter::execute() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    if (m_xMainForm.is())
        m_xMainForm->execute();
}

IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, sdbc, RowSetListener, m_aRowSetListeners, ::com::sun::star::sdbc::XRowSet, m_xMainForm)

// ::com::sun::star::sdbcx::XDeleteRows
Sequence<sal_Int32> SAL_CALL SbaXFormAdapter::deleteRows(const Sequence< Any >& rows) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbcx::XDeleteRows >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->deleteRows(rows);
    return Sequence<sal_Int32>();
}

// ::com::sun::star::sdbc::XWarningsSupplier
Any SAL_CALL SbaXFormAdapter::getWarnings() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XWarningsSupplier >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getWarnings();
    return Any();
}

void SAL_CALL SbaXFormAdapter::clearWarnings() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XWarningsSupplier >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->clearWarnings();
}

// ::com::sun::star::sdb::XRowSetApproveBroadcaster
IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, sdb, RowSetApproveListener, m_aRowSetApproveListeners, ::com::sun::star::sdb::XRowSetApproveBroadcaster, m_xMainForm)

// com::sun::star::sdbc::XSQLErrorBroadcaster
IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, sdb, SQLErrorListener, m_aErrorListeners, ::com::sun::star::sdb::XSQLErrorBroadcaster, m_xMainForm)

// ::com::sun::star::sdb::XResultSetAccess
Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL SbaXFormAdapter::createResultSet() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdb::XResultSetAccess >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->createResultSet();
    return Reference< ::com::sun::star::sdbc::XResultSet > ();
}

// com::sun::star::form::XLoadable
void SAL_CALL SbaXFormAdapter::load() throw( RuntimeException )
{
    Reference< ::com::sun::star::form::XLoadable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->load();
}

void SAL_CALL SbaXFormAdapter::unload() throw( RuntimeException )
{
    Reference< ::com::sun::star::form::XLoadable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->unload();
}

void SAL_CALL SbaXFormAdapter::reload() throw( RuntimeException )
{
    Reference< ::com::sun::star::form::XLoadable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->reload();
}

sal_Bool SAL_CALL SbaXFormAdapter::isLoaded() throw( RuntimeException )
{
    Reference< ::com::sun::star::form::XLoadable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->isLoaded();
    return sal_False;
}

IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, form, LoadListener, m_aLoadListeners, ::com::sun::star::form::XLoadable, m_xMainForm)

// ::com::sun::star::sdbc::XParameters
void SAL_CALL SbaXFormAdapter::setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setNull(parameterIndex, sqlType);
}

void SAL_CALL SbaXFormAdapter::setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setObjectNull(parameterIndex, sqlType, typeName);
}

void SAL_CALL SbaXFormAdapter::setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setBoolean(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setByte(sal_Int32 parameterIndex, sal_Int8 x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setByte(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setShort(sal_Int32 parameterIndex, sal_Int16 x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setShort(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setInt(sal_Int32 parameterIndex, sal_Int32 x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setInt(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setLong(sal_Int32 parameterIndex, sal_Int64 x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setLong(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setFloat(sal_Int32 parameterIndex, float x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setFloat(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setDouble(sal_Int32 parameterIndex, double x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setDouble(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setString(sal_Int32 parameterIndex, const OUString& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setString(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setBytes(sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setBytes(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setDate(sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setDate(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setTime(sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setTime(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setTimestamp(sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setTimestamp(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setBinaryStream(sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setBinaryStream(parameterIndex, x, length);
}

void SAL_CALL SbaXFormAdapter::setCharacterStream(sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setCharacterStream(parameterIndex, x, length);
}

void SAL_CALL SbaXFormAdapter::setObject(sal_Int32 parameterIndex, const Any& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setObject(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setObjectWithInfo(sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setObjectWithInfo(parameterIndex, x, targetSqlType, scale);
}

void SAL_CALL SbaXFormAdapter::setRef(sal_Int32 parameterIndex, const Reference< ::com::sun::star::sdbc::XRef >& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setRef(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setBlob(sal_Int32 parameterIndex, const Reference< ::com::sun::star::sdbc::XBlob >& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setBlob(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setClob(sal_Int32 parameterIndex, const Reference< ::com::sun::star::sdbc::XClob >& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setClob(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setArray(sal_Int32 parameterIndex, const Reference< ::com::sun::star::sdbc::XArray >& x) throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setArray(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::clearParameters() throw( ::com::sun::star::sdbc::SQLException, RuntimeException )
{
    Reference< ::com::sun::star::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->clearParameters();
}

// ::com::sun::star::form::XDatabaseParameterBroadcaster
void SAL_CALL SbaXFormAdapter::addParameterListener(const Reference< ::com::sun::star::form::XDatabaseParameterListener >& aListener) throw( RuntimeException )
{
    m_aParameterListeners.addInterface(aListener);
    if (m_aParameterListeners.getLength() == 1)
    {
        Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addParameterListener(&m_aParameterListeners);
    }
}

void SAL_CALL SbaXFormAdapter::removeParameterListener(const Reference< ::com::sun::star::form::XDatabaseParameterListener >& aListener) throw( RuntimeException )
{
    if (m_aParameterListeners.getLength() == 1)
    {
        Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeParameterListener(&m_aParameterListeners);
    }
    m_aParameterListeners.removeInterface(aListener);
}

// ::com::sun::star::container::XChild
Reference< XInterface > SAL_CALL SbaXFormAdapter::getParent() throw( RuntimeException )
{
    return m_xParent;
}

void SAL_CALL SbaXFormAdapter::setParent(const Reference< XInterface >& Parent) throw( ::com::sun::star::lang::NoSupportException, RuntimeException )
{
    m_xParent = Parent;
}

// ::com::sun::star::form::XSubmit
void SAL_CALL SbaXFormAdapter::submit(const Reference< ::com::sun::star::awt::XControl >& aControl, const ::com::sun::star::awt::MouseEvent& aMouseEvt) throw( RuntimeException )
{
    Reference< ::com::sun::star::form::XSubmit >  xSubmit(m_xMainForm, UNO_QUERY);
    if (xSubmit.is())
        xSubmit->submit(aControl, aMouseEvt);
}

IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, form, SubmitListener, m_aSubmitListeners, ::com::sun::star::form::XSubmit, m_xMainForm)

// ::com::sun::star::awt::XTabControllerModel
sal_Bool SAL_CALL SbaXFormAdapter::getGroupControl() throw( RuntimeException )
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getGroupControl : not supported !");
    return sal_False;
}

void SAL_CALL SbaXFormAdapter::setGroupControl(sal_Bool /*GroupControl*/) throw( RuntimeException )
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::setGroupControl : not supported !");
}

void SAL_CALL SbaXFormAdapter::setControlModels(const Sequence< Reference< ::com::sun::star::awt::XControlModel >  >& /*Controls*/) throw( RuntimeException )
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::setControlModels : not supported !");
}

Sequence< Reference< ::com::sun::star::awt::XControlModel > > SAL_CALL SbaXFormAdapter::getControlModels() throw( RuntimeException )
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getControlModels : not supported !");
    return Sequence< Reference< ::com::sun::star::awt::XControlModel > >();
}

void SAL_CALL SbaXFormAdapter::setGroup(const Sequence< Reference< ::com::sun::star::awt::XControlModel >  >& /*_rGroup*/, const OUString& /*GroupName*/) throw( RuntimeException )
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::setGroup : not supported !");
}

sal_Int32 SAL_CALL SbaXFormAdapter::getGroupCount() throw( RuntimeException )
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getGroupCount : not supported !");
    return 0;
}

void SAL_CALL SbaXFormAdapter::getGroup(sal_Int32 /*nGroup*/, Sequence< Reference< ::com::sun::star::awt::XControlModel >  >& /*_rGroup*/, OUString& /*Name*/) throw( RuntimeException )
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getGroup : not supported !");
}

void SAL_CALL SbaXFormAdapter::getGroupByName(const OUString& /*Name*/, Sequence< Reference< ::com::sun::star::awt::XControlModel >  >& /*_rGroup*/) throw( RuntimeException )
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getGroupByName : not supported !");
}

// ::com::sun::star::lang::XComponent
void SAL_CALL SbaXFormAdapter::dispose() throw( RuntimeException )
{
    // log off all multiplexers
    if (m_xMainForm.is())
        StopListening();

    ::com::sun::star::lang::EventObject aEvt(*this);
    m_aLoadListeners.disposeAndClear(aEvt);
    m_aRowSetListeners.disposeAndClear(aEvt);
    m_aRowSetApproveListeners.disposeAndClear(aEvt);
    m_aErrorListeners.disposeAndClear(aEvt);
    m_aParameterListeners.disposeAndClear(aEvt);
    m_aSubmitListeners.disposeAndClear(aEvt);
    m_aResetListeners.disposeAndClear(aEvt);

    m_aVetoablePropertyChangeListeners.disposeAndClear();
    m_aPropertyChangeListeners.disposeAndClear();
    m_aPropertiesChangeListeners.disposeAndClear(aEvt);

    m_aDisposeListeners.disposeAndClear(aEvt);
    m_aContainerListeners.disposeAndClear(aEvt);

    // dispose all children
    for (   ::std::vector< Reference< ::com::sun::star::form::XFormComponent > >::iterator aIter = m_aChildren.begin();
            aIter != m_aChildren.end();
            ++aIter
        )
    {
        Reference< ::com::sun::star::beans::XPropertySet >  xSet(*aIter, UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener(PROPERTY_NAME, (::com::sun::star::beans::XPropertyChangeListener*)this);

        Reference< ::com::sun::star::container::XChild >  xChild(*aIter, UNO_QUERY);
        if (xChild.is())
            xChild->setParent(Reference< XInterface > ());

        Reference< ::com::sun::star::lang::XComponent >  xComp(*aIter, UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aChildren.clear();
}

void SAL_CALL SbaXFormAdapter::addEventListener(const Reference< ::com::sun::star::lang::XEventListener >& xListener) throw( RuntimeException )
{
    m_aDisposeListeners.addInterface(xListener);
}

void SAL_CALL SbaXFormAdapter::removeEventListener(const Reference< ::com::sun::star::lang::XEventListener >& aListener) throw( RuntimeException )
{
    m_aDisposeListeners.removeInterface(aListener);
}

// ::com::sun::star::beans::XFastPropertySet
void SAL_CALL SbaXFormAdapter::setFastPropertyValue(sal_Int32 nHandle, const Any& aValue) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XFastPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    OSL_ENSURE(xSet.is(), "SAL_CALL SbaXFormAdapter::setFastPropertyValue : have no master form !");

    if (m_nNamePropHandle == nHandle)
    {
        if (aValue.getValueType().getTypeClass() != TypeClass_STRING)
        {
            throw ::com::sun::star::lang::IllegalArgumentException();
        }

        // for notifying property listeners
        ::com::sun::star::beans::PropertyChangeEvent aEvt;
        aEvt.Source = *this;
        aEvt.PropertyName = PROPERTY_NAME;
        aEvt.PropertyHandle = m_nNamePropHandle;
        aEvt.OldValue <<= m_sName;
        aEvt.NewValue = aValue;

        aValue >>= m_sName;

        ::cppu::OInterfaceIteratorHelper aIt(*m_aPropertyChangeListeners.getContainer(PROPERTY_NAME));
        while (aIt.hasMoreElements())
            ((::com::sun::star::beans::XPropertyChangeListener*)aIt.next())->propertyChange(aEvt);

        return;
    }

    xSet->setFastPropertyValue(nHandle, aValue);
}

Any SAL_CALL SbaXFormAdapter::getFastPropertyValue(sal_Int32 nHandle) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XFastPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    OSL_ENSURE(xSet.is(), "SAL_CALL SbaXFormAdapter::getFastPropertyValue : have no master form !");

    if (m_nNamePropHandle == nHandle)
        return makeAny(m_sName);

    return xSet->getFastPropertyValue(nHandle);
}

// ::com::sun::star::container::XNamed
OUString SAL_CALL SbaXFormAdapter::getName() throw( RuntimeException )
{
    return ::comphelper::getString(getPropertyValue(PROPERTY_NAME));
}

void SAL_CALL SbaXFormAdapter::setName(const OUString& aName) throw( RuntimeException )
{
    setPropertyValue(PROPERTY_NAME, makeAny(aName));
}

// ::com::sun::star::io::XPersistObject
OUString SAL_CALL SbaXFormAdapter::getServiceName() throw( RuntimeException )
{
    Reference< ::com::sun::star::io::XPersistObject >  xPersist(m_xMainForm, UNO_QUERY);
    if (xPersist.is())
        return xPersist->getServiceName();
    return OUString();
}

void SAL_CALL SbaXFormAdapter::write(const Reference< ::com::sun::star::io::XObjectOutputStream >& _rxOutStream) throw( ::com::sun::star::io::IOException, RuntimeException )
{
    Reference< ::com::sun::star::io::XPersistObject >  xPersist(m_xMainForm, UNO_QUERY);
    if (xPersist.is())
        xPersist->write(_rxOutStream);
}

void SAL_CALL SbaXFormAdapter::read(const Reference< ::com::sun::star::io::XObjectInputStream >& _rxInStream) throw( ::com::sun::star::io::IOException, RuntimeException )
{
    Reference< ::com::sun::star::io::XPersistObject >  xPersist(m_xMainForm, UNO_QUERY);
    if (xPersist.is())
        xPersist->read(_rxInStream);
}

// ::com::sun::star::beans::XMultiPropertySet
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL SbaXFormAdapter::getPropertySetInfo() throw( RuntimeException )
{
    Reference< ::com::sun::star::beans::XMultiPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (!xSet.is())
        return Reference< ::com::sun::star::beans::XPropertySetInfo > ();

    Reference< ::com::sun::star::beans::XPropertySetInfo >  xReturn = xSet->getPropertySetInfo();
    if (-1 == m_nNamePropHandle)
    {
        // we need to determine the handle for the NAME property
 Sequence< ::com::sun::star::beans::Property> aProps = xReturn->getProperties();
        const ::com::sun::star::beans::Property* pProps = aProps.getConstArray();

        for (sal_Int32 i=0; i<aProps.getLength(); ++i, ++pProps)
        {
            if (pProps->Name.equals(PROPERTY_NAME))
            {
                ((SbaXFormAdapter*)this)->m_nNamePropHandle = pProps->Handle;
                break;
            }
        }
    }
    return xReturn;
}

void SAL_CALL SbaXFormAdapter::setPropertyValues(const Sequence< OUString >& PropertyNames, const Sequence< Any >& Values) throw( ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XMultiPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (xSet.is())
        xSet->setPropertyValues(PropertyNames, Values);
}

Sequence< Any > SAL_CALL SbaXFormAdapter::getPropertyValues(const Sequence< OUString >& aPropertyNames) throw( RuntimeException )
{
    Reference< ::com::sun::star::beans::XMultiPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (!xSet.is())
        return Sequence< Any>(aPropertyNames.getLength());

 Sequence< Any> aReturn = xSet->getPropertyValues(aPropertyNames);

    // search for (and fake) the NAME property
    const OUString* pNames = aPropertyNames.getConstArray();
    Any* pValues = aReturn.getArray();
    OSL_ENSURE(aReturn.getLength() == aPropertyNames.getLength(), "SAL_CALL SbaXFormAdapter::getPropertyValues : the main form returned an invalid-length sequence !");
    for (sal_Int32 i=0; i<aPropertyNames.getLength(); ++i, ++pNames, ++pValues)
        if (pNames->equals(PROPERTY_NAME))
        {
            (*pValues) <<= m_sName;
            break;
        }

    return aReturn;
}

void SAL_CALL SbaXFormAdapter::addPropertiesChangeListener(const Sequence< OUString>& /*aPropertyNames*/, const Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener) throw( RuntimeException )
{
    // we completely ignore the property names, _all_ changes of _all_ properties will be forwarded to _all_ listeners
    m_aPropertiesChangeListeners.addInterface(xListener);
    if (m_aPropertiesChangeListeners.getLength() == 1)
    {
        Reference< ::com::sun::star::beans::XMultiPropertySet >  xBroadcaster(m_xMainForm, UNO_QUERY);
        OUString sEmpty;
        if (xBroadcaster.is())
            xBroadcaster->addPropertiesChangeListener(Sequence< OUString>(&sEmpty, 1), &m_aPropertiesChangeListeners);
    }
}

void SAL_CALL SbaXFormAdapter::removePropertiesChangeListener(const Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener) throw( RuntimeException )
{
    if (m_aPropertiesChangeListeners.getLength() == 1)
    {
        Reference< ::com::sun::star::beans::XMultiPropertySet >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removePropertiesChangeListener(&m_aPropertiesChangeListeners);
    }
    m_aPropertiesChangeListeners.removeInterface(Listener);
}

void SAL_CALL SbaXFormAdapter::firePropertiesChangeEvent(const Sequence< OUString >& aPropertyNames, const Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener) throw( RuntimeException )
{
    Reference< ::com::sun::star::beans::XMultiPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (xSet.is())
        xSet->firePropertiesChangeEvent(aPropertyNames, xListener);
}

// ::com::sun::star::beans::XPropertySet
void SAL_CALL SbaXFormAdapter::setPropertyValue(const OUString& aPropertyName, const Any& aValue) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (!xSet.is())
        return;

    // special handling for the "name" property
    if (aPropertyName.equals(PROPERTY_NAME))
        setFastPropertyValue(m_nNamePropHandle, aValue);

    xSet->setPropertyValue(aPropertyName, aValue);
}

Any SAL_CALL SbaXFormAdapter::getPropertyValue(const OUString& PropertyName) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (!xSet.is())
        return Any();

    // special handling for the "name" property
    if (PropertyName.equals(PROPERTY_NAME))
        return getFastPropertyValue(m_nNamePropHandle);

    return xSet->getPropertyValue(PropertyName);
}

IMPLEMENT_PROPERTY_LISTENER_ADMINISTRATION(SbaXFormAdapter, PropertyChangeListener, m_aPropertyChangeListeners, ::com::sun::star::beans::XPropertySet, m_xMainForm);
IMPLEMENT_PROPERTY_LISTENER_ADMINISTRATION(SbaXFormAdapter, VetoableChangeListener, m_aVetoablePropertyChangeListeners, ::com::sun::star::beans::XPropertySet, m_xMainForm);

// ::com::sun::star::util::XCancellable
void SAL_CALL SbaXFormAdapter::cancel() throw( RuntimeException )
{
    Reference< ::com::sun::star::util::XCancellable >  xCancel(m_xMainForm, UNO_QUERY);
    if (xCancel.is())
        return;
    xCancel->cancel();
}

// ::com::sun::star::beans::XPropertyState
::com::sun::star::beans::PropertyState SAL_CALL SbaXFormAdapter::getPropertyState(const OUString& PropertyName) throw( ::com::sun::star::beans::UnknownPropertyException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XPropertyState >  xState(m_xMainForm, UNO_QUERY);
    if (xState.is())
        return xState->getPropertyState(PropertyName);
    return ::com::sun::star::beans::PropertyState_DEFAULT_VALUE;
}

Sequence< ::com::sun::star::beans::PropertyState> SAL_CALL SbaXFormAdapter::getPropertyStates(const Sequence< OUString >& aPropertyName) throw( ::com::sun::star::beans::UnknownPropertyException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XPropertyState >  xState(m_xMainForm, UNO_QUERY);
    if (xState.is())
        return xState->getPropertyStates(aPropertyName);

    // set them all to DEFAULT
     Sequence< ::com::sun::star::beans::PropertyState> aReturn(aPropertyName.getLength());
    ::com::sun::star::beans::PropertyState* pStates = aReturn.getArray();
    for (sal_uInt16 i=0; i<aPropertyName.getLength(); ++i, ++pStates)
        *pStates = ::com::sun::star::beans::PropertyState_DEFAULT_VALUE;
    return aReturn;
}

void SAL_CALL SbaXFormAdapter::setPropertyToDefault(const OUString& PropertyName) throw( ::com::sun::star::beans::UnknownPropertyException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XPropertyState >  xState(m_xMainForm, UNO_QUERY);
    if (xState.is())
        xState->setPropertyToDefault(PropertyName);
}

Any SAL_CALL SbaXFormAdapter::getPropertyDefault(const OUString& aPropertyName) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    Reference< ::com::sun::star::beans::XPropertyState >  xState(m_xMainForm, UNO_QUERY);
    if (xState.is())
        return xState->getPropertyDefault(aPropertyName);
    return Any();
}

// ::com::sun::star::form::XReset
void SAL_CALL SbaXFormAdapter::reset() throw( RuntimeException )
{
    Reference< ::com::sun::star::form::XReset >  xReset(m_xMainForm, UNO_QUERY);
    if (xReset.is())
        xReset->reset();
}

IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, form, ResetListener, m_aResetListeners, ::com::sun::star::form::XReset, m_xMainForm)

// ::com::sun::star::container::XNameContainer
void SbaXFormAdapter::implInsert(const Any& aElement, sal_Int32 nIndex, const OUString* pNewElName) throw( ::com::sun::star::lang::IllegalArgumentException )
{
    // extract the form component
    if (aElement.getValueType().getTypeClass() != TypeClass_INTERFACE)
    {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    Reference< ::com::sun::star::form::XFormComponent >  xElement(*(Reference< XInterface > *)aElement.getValue(), UNO_QUERY);
    if (!xElement.is())
    {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    // for the name we need the propset
    Reference< ::com::sun::star::beans::XPropertySet >  xElementSet(xElement, UNO_QUERY);
    if (!xElementSet.is())
    {
        throw ::com::sun::star::lang::IllegalArgumentException();
     }
    OUString sName;
    try
    {
        if (pNewElName)
            xElementSet->setPropertyValue(PROPERTY_NAME, makeAny(*pNewElName));

        xElementSet->getPropertyValue(PROPERTY_NAME) >>= sName;
    }
    catch(Exception&)
    {
        // the set didn't support the name prop
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    // check the index
    OSL_ASSERT(nIndex >= 0);
    if (sal::static_int_cast< sal_uInt32 >(nIndex) > m_aChildren.size())
        nIndex = m_aChildren.size();

    OSL_ENSURE(m_aChildren.size() == m_aChildNames.size(), "SAL_CALL SbaXFormAdapter::implInsert : inconsistent container state !");
    m_aChildren.insert(m_aChildren.begin() + nIndex, xElement);
    m_aChildNames.insert(m_aChildNames.begin() + nIndex, sName);

    // listen for a changes of the name
    xElementSet->addPropertyChangeListener(PROPERTY_NAME, (::com::sun::star::beans::XPropertyChangeListener*)this);

    // we are now the parent of the new element
    xElement->setParent((::com::sun::star::container::XContainer*)this);

    // notify the container listeners
    ::com::sun::star::container::ContainerEvent aEvt;
    aEvt.Source = *this;
    aEvt.Accessor <<= nIndex;
    aEvt.Element <<= xElement;
    ::cppu::OInterfaceIteratorHelper aIt(m_aContainerListeners);
    while (aIt.hasMoreElements())
        ((::com::sun::star::container::XContainerListener*)aIt.next())->elementInserted(aEvt);
}

sal_Int32 SbaXFormAdapter::implGetPos(const OUString& rName)
{
    ::std::vector< OUString>::iterator aIter = ::std::find_if(   m_aChildNames.begin(),
                                                                m_aChildNames.end(),
                                                                ::std::bind2nd(::std::equal_to< OUString>(),rName));

    if(aIter != m_aChildNames.end())
        return aIter - m_aChildNames.begin();

    return -1;
}

void SAL_CALL SbaXFormAdapter::insertByName(const OUString& aName, const Any& aElement) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    implInsert(aElement, m_aChildren.size(), &aName);
}

void SAL_CALL SbaXFormAdapter::removeByName(const OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    sal_Int32 nPos = implGetPos(Name);
    if (-1 == nPos)
    {
        throw ::com::sun::star::container::NoSuchElementException();
    }
    removeByIndex(nPos);
}

// ::com::sun::star::container::XNameReplace
void SAL_CALL SbaXFormAdapter::replaceByName(const OUString& aName, const Any& aElement) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    sal_Int32 nPos = implGetPos(aName);
    if (-1 == nPos)
    {
        throw ::com::sun::star::container::NoSuchElementException();
    }
    replaceByIndex(nPos, aElement);
}

// ::com::sun::star::container::XNameAccess
Any SAL_CALL SbaXFormAdapter::getByName(const OUString& aName) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    sal_Int32 nPos = implGetPos(aName);
    if (-1 == nPos)
    {
        throw ::com::sun::star::container::NoSuchElementException();
    }
    return makeAny(m_aChildren[nPos]);
}

Sequence< OUString > SAL_CALL SbaXFormAdapter::getElementNames() throw( RuntimeException )
{
    OUString *pChildNames = m_aChildNames.empty() ? 0 : &m_aChildNames[0];
    return Sequence< OUString >(pChildNames, m_aChildNames.size());
}

sal_Bool SAL_CALL SbaXFormAdapter::hasByName(const OUString& aName) throw( RuntimeException )
{
    return (-1 != implGetPos(aName));
}

// ::com::sun::star::container::XElementAccess
Type SAL_CALL SbaXFormAdapter::getElementType() throw(RuntimeException)
{
    return ::getCppuType(
        static_cast< Reference< ::com::sun::star::form::XFormComponent >* >(NULL));
}

sal_Bool SAL_CALL SbaXFormAdapter::hasElements() throw(RuntimeException)
{
    return m_aChildren.size() > 0;
}

// ::com::sun::star::container::XIndexContainer
void SAL_CALL SbaXFormAdapter::insertByIndex(sal_Int32 _rIndex, const Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    if ( ( _rIndex < 0 ) || ( (size_t)_rIndex >= m_aChildren.size() ) )
        throw ::com::sun::star::lang::IndexOutOfBoundsException();
    implInsert(Element, _rIndex);
}

void SAL_CALL SbaXFormAdapter::removeByIndex(sal_Int32 _rIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    if ( ( _rIndex < 0 ) || ( (size_t)_rIndex >= m_aChildren.size() ) )
        throw ::com::sun::star::lang::IndexOutOfBoundsException();

    Reference< ::com::sun::star::form::XFormComponent >  xAffected = *(m_aChildren.begin() + _rIndex);

    OSL_ENSURE(m_aChildren.size() == m_aChildNames.size(), "SAL_CALL SbaXFormAdapter::removeByIndex : inconsistent container state !");
    m_aChildren.erase(m_aChildren.begin() + _rIndex);
    m_aChildNames.erase(m_aChildNames.begin() + _rIndex);

    // no need to listen anymore
    Reference< ::com::sun::star::beans::XPropertySet >  xAffectedSet(xAffected, UNO_QUERY);
    xAffectedSet->removePropertyChangeListener(PROPERTY_NAME, (::com::sun::star::beans::XPropertyChangeListener*)this);

    // we are no longer the parent
    xAffected->setParent(Reference< XInterface > ());

    // notify container listeners
    ::com::sun::star::container::ContainerEvent aEvt;
    aEvt.Source = *this;
    aEvt.Element <<= xAffected;
    ::cppu::OInterfaceIteratorHelper aIt(m_aContainerListeners);
    while (aIt.hasMoreElements())
        ((::com::sun::star::container::XContainerListener*)aIt.next())->elementRemoved(aEvt);

}

// ::com::sun::star::container::XIndexReplace
void SAL_CALL SbaXFormAdapter::replaceByIndex(sal_Int32 _rIndex, const Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    if ( ( _rIndex < 0 ) || ( (size_t)_rIndex >= m_aChildren.size() ) )
        throw ::com::sun::star::lang::IndexOutOfBoundsException();

    // extract the form component
    if (Element.getValueType().getTypeClass() != TypeClass_INTERFACE)
    {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    Reference< ::com::sun::star::form::XFormComponent >  xElement(*(Reference< XInterface > *)Element.getValue(), UNO_QUERY);
    if (!xElement.is())
    {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    // for the name we need the propset
    Reference< ::com::sun::star::beans::XPropertySet >  xElementSet(xElement, UNO_QUERY);
    if (!xElementSet.is())
    {
        throw ::com::sun::star::lang::IllegalArgumentException();
     }
    OUString sName;
    try
    {
        xElementSet->getPropertyValue(PROPERTY_NAME) >>= sName;
    }
    catch(Exception&)
    {
        // the set didn't support the name prop
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    Reference< ::com::sun::star::form::XFormComponent >  xOld = *(m_aChildren.begin() + _rIndex);

    OSL_ENSURE(m_aChildren.size() == m_aChildNames.size(), "SAL_CALL SbaXFormAdapter::replaceByIndex : inconsistent container state !");
    *(m_aChildren.begin() + _rIndex) = xElement;
    *(m_aChildNames.begin() + _rIndex) = sName;

    // correct property change listening
    Reference< ::com::sun::star::beans::XPropertySet >  xOldSet(xOld, UNO_QUERY);
    xOldSet->removePropertyChangeListener(PROPERTY_NAME, (::com::sun::star::beans::XPropertyChangeListener*)this);
    xElementSet->addPropertyChangeListener(PROPERTY_NAME, (::com::sun::star::beans::XPropertyChangeListener*)this);

    // parent reset
    xOld->setParent(Reference< XInterface > ());
    xElement->setParent((::com::sun::star::container::XContainer*)this);

    // notify container listeners
    ::com::sun::star::container::ContainerEvent aEvt;
    aEvt.Source = *this;
    aEvt.Accessor <<= (sal_Int32)_rIndex;
    aEvt.Element <<= xElement;
    aEvt.ReplacedElement <<= xOld;

    ::cppu::OInterfaceIteratorHelper aIt(m_aContainerListeners);
    while (aIt.hasMoreElements())
        ((::com::sun::star::container::XContainerListener*)aIt.next())->elementReplaced(aEvt);
}

// ::com::sun::star::container::XIndexAccess
sal_Int32 SAL_CALL SbaXFormAdapter::getCount() throw( RuntimeException )
{
    return m_aChildren.size();
}

Any SAL_CALL SbaXFormAdapter::getByIndex(sal_Int32 _rIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
{
    if ( ( _rIndex < 0 ) || ( (size_t)_rIndex >= m_aChildren.size() ) )
        throw ::com::sun::star::lang::IndexOutOfBoundsException();

    Reference< ::com::sun::star::form::XFormComponent >  xElement = *(m_aChildren.begin() + _rIndex);
    return makeAny(xElement);
}

// ::com::sun::star::container::XContainer
void SAL_CALL SbaXFormAdapter::addContainerListener(const Reference< ::com::sun::star::container::XContainerListener >& xListener) throw( RuntimeException )
{
    m_aContainerListeners.addInterface(xListener);
}

void SAL_CALL SbaXFormAdapter::removeContainerListener(const Reference< ::com::sun::star::container::XContainerListener >& xListener) throw( RuntimeException )
{
    m_aContainerListeners.removeInterface(xListener);
}

// ::com::sun::star::container::XEnumerationAccess
Reference< ::com::sun::star::container::XEnumeration > SAL_CALL SbaXFormAdapter::createEnumeration() throw( RuntimeException )
{
    return new ::comphelper::OEnumerationByName(this);
}

// ::com::sun::star::beans::XPropertyChangeListener
void SAL_CALL SbaXFormAdapter::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( RuntimeException )
{
    if (evt.PropertyName.equals(PROPERTY_NAME))
    {
        ::std::vector<  ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > >::iterator aIter = ::std::find_if(  m_aChildren.begin(),
                                                                m_aChildren.end(),
                                                                ::std::bind2nd(::std::equal_to< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >(),evt.Source));

        if(aIter != m_aChildren.end())
        {
            sal_Int32 nPos = aIter - m_aChildren.begin();
            OSL_ENSURE(*(m_aChildNames.begin() + nPos) == ::comphelper::getString(evt.OldValue), "SAL_CALL SbaXFormAdapter::propertyChange : object has a wrong name !");
            *(m_aChildNames.begin() + nPos) = ::comphelper::getString(evt.NewValue);
        }
    }
}

// ::com::sun::star::lang::XEventListener
void SAL_CALL SbaXFormAdapter::disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException )
{
    // was it our main form ?
    if (Source.Source == m_xMainForm)
        dispose();

    ::std::vector<  ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > >::iterator aIter = ::std::find_if(  m_aChildren.begin(),
                                                                m_aChildren.end(),
                                                                ::std::bind2nd(::std::equal_to< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >(),Source.Source));
    if(aIter != m_aChildren.end())
            removeByIndex(aIter - m_aChildren.begin());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
