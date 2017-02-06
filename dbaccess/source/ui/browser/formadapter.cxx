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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "dbu_brw.hrc"
#include "dbustrings.hrc"
#include <connectivity/dbexception.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

// SbaXFormAdapter

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

}

SbaXFormAdapter::~SbaXFormAdapter()
{

}

Sequence< Type > SAL_CALL SbaXFormAdapter::getTypes(  )
{
    return ::comphelper::concatSequences(
        SbaXFormAdapter_BASE1::getTypes(),
        SbaXFormAdapter_BASE2::getTypes(),
        SbaXFormAdapter_BASE3::getTypes()
    );
}

Sequence< sal_Int8 > SAL_CALL SbaXFormAdapter::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

Any SAL_CALL SbaXFormAdapter::queryInterface(const Type& _rType)
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
    STOP_MULTIPLEXER_LISTENING(LoadListener, m_aLoadListeners, css::form::XLoadable, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(RowSetListener, m_aRowSetListeners, css::sdbc::XRowSet, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(RowSetApproveListener, m_aRowSetApproveListeners, css::sdb::XRowSetApproveBroadcaster, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(SQLErrorListener, m_aErrorListeners, css::sdb::XSQLErrorBroadcaster, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(SubmitListener, m_aSubmitListeners, css::form::XSubmit, m_xMainForm);
    STOP_MULTIPLEXER_LISTENING(ResetListener, m_aResetListeners, css::form::XReset, m_xMainForm);

    if (m_aParameterListeners.getLength())
    {
        Reference< css::form::XDatabaseParameterBroadcaster >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeParameterListener(&m_aParameterListeners);
    }

    STOP_PROPERTY_MULTIPLEXER_LISTENING(PropertyChangeListener, m_aPropertyChangeListeners, css::beans::XPropertySet, m_xMainForm);
    STOP_PROPERTY_MULTIPLEXER_LISTENING(VetoableChangeListener, m_aVetoablePropertyChangeListeners, css::beans::XPropertySet, m_xMainForm);
    if (m_aPropertiesChangeListeners.getLength())
    {
        Reference< css::beans::XMultiPropertySet >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removePropertiesChangeListener(&m_aPropertiesChangeListeners);
    }

    // log off ourself
    Reference< css::lang::XComponent >  xComp(m_xMainForm, UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener(static_cast<css::lang::XEventListener*>(static_cast<css::beans::XPropertyChangeListener*>(this)));
}

void SbaXFormAdapter::StartListening()
{
    // log off all our multiplexers
    START_MULTIPLEXER_LISTENING(LoadListener, m_aLoadListeners, css::form::XLoadable, m_xMainForm);
    START_MULTIPLEXER_LISTENING(RowSetListener, m_aRowSetListeners, css::sdbc::XRowSet, m_xMainForm);
    START_MULTIPLEXER_LISTENING(RowSetApproveListener, m_aRowSetApproveListeners, css::sdb::XRowSetApproveBroadcaster, m_xMainForm);
    START_MULTIPLEXER_LISTENING(SQLErrorListener, m_aErrorListeners, css::sdb::XSQLErrorBroadcaster, m_xMainForm);
    START_MULTIPLEXER_LISTENING(SubmitListener, m_aSubmitListeners, css::form::XSubmit, m_xMainForm);
    START_MULTIPLEXER_LISTENING(ResetListener, m_aResetListeners, css::form::XReset, m_xMainForm);

    if (m_aParameterListeners.getLength())
    {
        Reference< css::form::XDatabaseParameterBroadcaster >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addParameterListener(&m_aParameterListeners);
    }

    START_PROPERTY_MULTIPLEXER_LISTENING(PropertyChangeListener, m_aPropertyChangeListeners, css::beans::XPropertySet, m_xMainForm);
    START_PROPERTY_MULTIPLEXER_LISTENING(VetoableChangeListener, m_aVetoablePropertyChangeListeners, css::beans::XPropertySet, m_xMainForm);
    if (m_aPropertiesChangeListeners.getLength())
    {
        Reference< css::beans::XMultiPropertySet >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addPropertiesChangeListener(css::uno::Sequence<OUString>{""}, &m_aPropertiesChangeListeners);
    }

    // log off ourself
    Reference< css::lang::XComponent >  xComp(m_xMainForm, UNO_QUERY);
    if (xComp.is())
        xComp->addEventListener(static_cast<css::lang::XEventListener*>(static_cast<css::beans::XPropertyChangeListener*>(this)));
}

void SbaXFormAdapter::AttachForm(const Reference< css::sdbc::XRowSet >& xNewMaster)
{
    if (xNewMaster == m_xMainForm)
        return;

    OSL_ENSURE(xNewMaster.get() != static_cast< css::sdbc::XRowSet* >(this), "SbaXFormAdapter::AttachForm : invalid argument !");

    if (m_xMainForm.is())
    {
        StopListening();

        // if our old master is loaded we have to send an 'unloaded' event
        Reference< css::form::XLoadable >  xLoadable(m_xMainForm, UNO_QUERY);
        if (xLoadable->isLoaded())
        {
            css::lang::EventObject aEvt(*this);
            ::comphelper::OInterfaceIteratorHelper2 aIt(m_aLoadListeners);
            while (aIt.hasMoreElements())
                static_cast< css::form::XLoadListener*>(aIt.next())->unloaded(aEvt);
        }
    }

    m_xMainForm = xNewMaster;

    if (m_xMainForm.is())
    {
        StartListening();

        // if our new master is loaded we have to send an 'loaded' event
        Reference< css::form::XLoadable >  xLoadable(m_xMainForm, UNO_QUERY);
        if (xLoadable->isLoaded())
        {
            css::lang::EventObject aEvt(*this);
            ::comphelper::OInterfaceIteratorHelper2 aIt(m_aLoadListeners);
            while (aIt.hasMoreElements())
                static_cast< css::form::XLoadListener*>(aIt.next())->loaded(aEvt);
        }
    }

    // TODO : perhaps _all_ of our listeners should be notified about our new state
    // (nearly every aspect of us may have changed with new master form)
}

// css::sdbc::XCloseable
void SAL_CALL SbaXFormAdapter::close()
{
    Reference< css::sdbc::XCloseable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->close();
}

// css::sdbc::XResultSetMetaDataSupplier
Reference< css::sdbc::XResultSetMetaData > SAL_CALL SbaXFormAdapter::getMetaData()
{
    Reference< css::sdbc::XResultSetMetaDataSupplier >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getMetaData();
    return Reference< css::sdbc::XResultSetMetaData > ();
}

// css::sdbc::XColumnLocate
sal_Int32 SAL_CALL SbaXFormAdapter::findColumn(const OUString& columnName)
{
    Reference< css::sdbc::XColumnLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->findColumn(columnName);

    ::dbtools::throwInvalidColumnException( columnName, *this );
    assert(false);
    return 0; // Never reached
}

// css::sdbcx::XColumnsSupplier
Reference< css::container::XNameAccess > SAL_CALL SbaXFormAdapter::getColumns()
{
    Reference< css::sdbcx::XColumnsSupplier >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getColumns();
    return Reference< css::container::XNameAccess > ();
}

// css::sdbc::XRow
sal_Bool SAL_CALL SbaXFormAdapter::wasNull()
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->wasNull();
    return true;
}

OUString SAL_CALL SbaXFormAdapter::getString(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getString(columnIndex);
    return OUString();
}

sal_Bool SAL_CALL SbaXFormAdapter::getBoolean(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBoolean(columnIndex);
    return false;
}

sal_Int8 SAL_CALL SbaXFormAdapter::getByte(sal_Int32 columnIndex)

{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getByte(columnIndex);
    return 0;
}

sal_Int16 SAL_CALL SbaXFormAdapter::getShort(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getShort(columnIndex);
    return 0;
}

sal_Int32 SAL_CALL SbaXFormAdapter::getInt(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getInt(columnIndex);
    return 0;
}

sal_Int64 SAL_CALL SbaXFormAdapter::getLong(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getLong(columnIndex);
    return 0;
}

float SAL_CALL SbaXFormAdapter::getFloat(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getFloat(columnIndex);
    return 0.0;
}

double SAL_CALL SbaXFormAdapter::getDouble(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getDouble(columnIndex);
    return 0.0;
}

Sequence< sal_Int8 > SAL_CALL SbaXFormAdapter::getBytes(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBytes(columnIndex);
    return Sequence <sal_Int8> ();
}

css::util::Date SAL_CALL SbaXFormAdapter::getDate(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getDate(columnIndex);
    return css::util::Date();
}

css::util::Time SAL_CALL SbaXFormAdapter::getTime(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getTime(columnIndex);
    return css::util::Time();
}

css::util::DateTime SAL_CALL SbaXFormAdapter::getTimestamp(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getTimestamp(columnIndex);
    return css::util::DateTime();
}

Reference< css::io::XInputStream > SAL_CALL SbaXFormAdapter::getBinaryStream(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBinaryStream(columnIndex);
    return Reference< css::io::XInputStream > ();
}

Reference< css::io::XInputStream > SAL_CALL SbaXFormAdapter::getCharacterStream(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getCharacterStream(columnIndex);
    return Reference< css::io::XInputStream > ();
}

Any SAL_CALL SbaXFormAdapter::getObject(sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& typeMap)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getObject(columnIndex, typeMap);
    return Any();
}

Reference< css::sdbc::XRef > SAL_CALL SbaXFormAdapter::getRef(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getRef(columnIndex);
    return Reference< css::sdbc::XRef > ();
}

Reference< css::sdbc::XBlob > SAL_CALL SbaXFormAdapter::getBlob(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBlob(columnIndex);
    return Reference< css::sdbc::XBlob > ();
}

Reference< css::sdbc::XClob > SAL_CALL SbaXFormAdapter::getClob(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getClob(columnIndex);
    return Reference< css::sdbc::XClob > ();
}

Reference< css::sdbc::XArray > SAL_CALL SbaXFormAdapter::getArray(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRow >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getArray(columnIndex);
    return Reference< css::sdbc::XArray > ();
}

// css::sdbcx::XRowLocate
Any SAL_CALL SbaXFormAdapter::getBookmark()
{
    Reference< css::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getBookmark();
    return Any();
}

sal_Bool SAL_CALL SbaXFormAdapter::moveToBookmark(const Any& bookmark)
{
    Reference< css::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->moveToBookmark(bookmark);
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::moveRelativeToBookmark(const Any& bookmark, sal_Int32 rows)
{
    Reference< css::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->moveRelativeToBookmark(bookmark,rows);
    return false;
}

sal_Int32 SAL_CALL SbaXFormAdapter::compareBookmarks(const Any& _first, const Any& _second)
{
    Reference< css::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->compareBookmarks(_first, _second);
    return 0;
}

sal_Bool SAL_CALL SbaXFormAdapter::hasOrderedBookmarks()
{
    Reference< css::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->hasOrderedBookmarks();
    return false;
}

sal_Int32 SAL_CALL SbaXFormAdapter::hashBookmark(const Any& bookmark)
{
    Reference< css::sdbcx::XRowLocate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->hashBookmark(bookmark);
    return 0;
}

// css::sdbc::XRowUpdate
void SAL_CALL SbaXFormAdapter::updateNull(sal_Int32 columnIndex)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateNull(columnIndex);
}

void SAL_CALL SbaXFormAdapter::updateBoolean(sal_Int32 columnIndex, sal_Bool x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateBoolean(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateByte(sal_Int32 columnIndex, sal_Int8 x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateByte(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateShort(sal_Int32 columnIndex, sal_Int16 x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateShort(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateInt(sal_Int32 columnIndex, sal_Int32 x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateInt(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateLong(sal_Int32 columnIndex, sal_Int64 x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateLong(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateFloat(sal_Int32 columnIndex, float x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateFloat(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateDouble(sal_Int32 columnIndex, double x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateDouble(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateString(sal_Int32 columnIndex, const OUString& x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateString(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateBytes(sal_Int32 columnIndex, const Sequence< sal_Int8 >& x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateBytes(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateDate(sal_Int32 columnIndex, const css::util::Date& x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateDate(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateTime(sal_Int32 columnIndex, const css::util::Time& x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateTime(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateTimestamp(sal_Int32 columnIndex, const css::util::DateTime& x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateTimestamp(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateBinaryStream(sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateBinaryStream(columnIndex, x, length);
}

void SAL_CALL SbaXFormAdapter::updateCharacterStream(sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateCharacterStream(columnIndex, x, length);
}

void SAL_CALL SbaXFormAdapter::updateObject(sal_Int32 columnIndex, const Any& x)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateObject(columnIndex, x);
}

void SAL_CALL SbaXFormAdapter::updateNumericObject(sal_Int32 columnIndex, const Any& x, sal_Int32 scale)
{
    Reference< css::sdbc::XRowUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateNumericObject(columnIndex, x, scale);
}

// css::sdbc::XResultSet
sal_Bool SAL_CALL SbaXFormAdapter::next()
{
    if (m_xMainForm.is())
        return m_xMainForm->next();
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::isBeforeFirst()
{
    if (m_xMainForm.is())
        return m_xMainForm->isBeforeFirst();
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::isAfterLast()
{
    if (m_xMainForm.is())
        return m_xMainForm->isAfterLast();
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::isFirst()
{
    if (m_xMainForm.is())
        return m_xMainForm->isFirst();
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::isLast()
{
    if (m_xMainForm.is())
        return m_xMainForm->isLast();
    return false;
}

void SAL_CALL SbaXFormAdapter::beforeFirst()
{
    if (m_xMainForm.is())
        m_xMainForm->beforeFirst();
}

void SAL_CALL SbaXFormAdapter::afterLast()
{
    if (m_xMainForm.is())
        m_xMainForm->afterLast();
}

sal_Bool SAL_CALL SbaXFormAdapter::first()
{
    if (m_xMainForm.is())
        return m_xMainForm->first();
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::last()
{
    if (m_xMainForm.is())
        return m_xMainForm->last();
    return false;
}

sal_Int32 SAL_CALL SbaXFormAdapter::getRow()
{
    if (m_xMainForm.is())
        return m_xMainForm->getRow();
    return 0;
}

sal_Bool SAL_CALL SbaXFormAdapter::absolute(sal_Int32 row)
{
    if (m_xMainForm.is())
        return m_xMainForm->absolute(row);
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::relative(sal_Int32 rows)
{
    if (m_xMainForm.is())
        return m_xMainForm->relative(rows);
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::previous()
{
    if (m_xMainForm.is())
        return m_xMainForm->previous();
    return false;
}

void SAL_CALL SbaXFormAdapter::refreshRow()
{
    if (m_xMainForm.is())
        m_xMainForm->refreshRow();
}

sal_Bool SAL_CALL SbaXFormAdapter::rowUpdated()
{
    if (m_xMainForm.is())
        return m_xMainForm->rowUpdated();
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::rowInserted()
{
    if (m_xMainForm.is())
        return m_xMainForm->rowInserted();
    return false;
}

sal_Bool SAL_CALL SbaXFormAdapter::rowDeleted()
{
    if (m_xMainForm.is())
        return m_xMainForm->rowDeleted();
    return false;
}

Reference< XInterface > SAL_CALL SbaXFormAdapter::getStatement()
{
    if (m_xMainForm.is())
        return m_xMainForm->getStatement();
    return nullptr;
}

// css::sdbc::XResultSetUpdate
void SAL_CALL SbaXFormAdapter::insertRow()
{
    Reference< css::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->insertRow();
}

void SAL_CALL SbaXFormAdapter::updateRow()
{
    Reference< css::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->updateRow();
}

void SAL_CALL SbaXFormAdapter::deleteRow()
{
    Reference< css::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->deleteRow();
}

void SAL_CALL SbaXFormAdapter::cancelRowUpdates()
{
    Reference< css::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->cancelRowUpdates();
}

void SAL_CALL SbaXFormAdapter::moveToInsertRow()
{
    Reference< css::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->moveToInsertRow();
}

void SAL_CALL SbaXFormAdapter::moveToCurrentRow()
{
    Reference< css::sdbc::XResultSetUpdate >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->moveToCurrentRow();
}

// css::sdbc::XRowSet
void SAL_CALL SbaXFormAdapter::execute()
{
    if (m_xMainForm.is())
        m_xMainForm->execute();
}

IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, sdbc, RowSetListener, m_aRowSetListeners, css::sdbc::XRowSet, m_xMainForm)

// css::sdbcx::XDeleteRows
Sequence<sal_Int32> SAL_CALL SbaXFormAdapter::deleteRows(const Sequence< Any >& rows)
{
    Reference< css::sdbcx::XDeleteRows >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->deleteRows(rows);
    return Sequence<sal_Int32>();
}

// css::sdbc::XWarningsSupplier
Any SAL_CALL SbaXFormAdapter::getWarnings()
{
    Reference< css::sdbc::XWarningsSupplier >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->getWarnings();
    return Any();
}

void SAL_CALL SbaXFormAdapter::clearWarnings()
{
    Reference< css::sdbc::XWarningsSupplier >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->clearWarnings();
}

// css::sdb::XRowSetApproveBroadcaster
IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, sdb, RowSetApproveListener, m_aRowSetApproveListeners, css::sdb::XRowSetApproveBroadcaster, m_xMainForm)

// css::sdbc::XSQLErrorBroadcaster
IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, sdb, SQLErrorListener, m_aErrorListeners, css::sdb::XSQLErrorBroadcaster, m_xMainForm)

// css::sdb::XResultSetAccess
Reference< css::sdbc::XResultSet > SAL_CALL SbaXFormAdapter::createResultSet()
{
    Reference< css::sdb::XResultSetAccess >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->createResultSet();
    return Reference< css::sdbc::XResultSet > ();
}

// css::form::XLoadable
void SAL_CALL SbaXFormAdapter::load()
{
    Reference< css::form::XLoadable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->load();
}

void SAL_CALL SbaXFormAdapter::unload()
{
    Reference< css::form::XLoadable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->unload();
}

void SAL_CALL SbaXFormAdapter::reload()
{
    Reference< css::form::XLoadable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->reload();
}

sal_Bool SAL_CALL SbaXFormAdapter::isLoaded()
{
    Reference< css::form::XLoadable >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        return xIface->isLoaded();
    return false;
}

IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, form, LoadListener, m_aLoadListeners, css::form::XLoadable, m_xMainForm)

// css::sdbc::XParameters
void SAL_CALL SbaXFormAdapter::setNull(sal_Int32 parameterIndex, sal_Int32 sqlType)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setNull(parameterIndex, sqlType);
}

void SAL_CALL SbaXFormAdapter::setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setObjectNull(parameterIndex, sqlType, typeName);
}

void SAL_CALL SbaXFormAdapter::setBoolean(sal_Int32 parameterIndex, sal_Bool x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setBoolean(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setByte(sal_Int32 parameterIndex, sal_Int8 x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setByte(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setShort(sal_Int32 parameterIndex, sal_Int16 x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setShort(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setInt(sal_Int32 parameterIndex, sal_Int32 x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setInt(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setLong(sal_Int32 parameterIndex, sal_Int64 x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setLong(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setFloat(sal_Int32 parameterIndex, float x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setFloat(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setDouble(sal_Int32 parameterIndex, double x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setDouble(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setString(sal_Int32 parameterIndex, const OUString& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setString(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setBytes(sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setBytes(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setDate(sal_Int32 parameterIndex, const css::util::Date& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setDate(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setTime(sal_Int32 parameterIndex, const css::util::Time& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setTime(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setTimestamp(sal_Int32 parameterIndex, const css::util::DateTime& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setTimestamp(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setBinaryStream(sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setBinaryStream(parameterIndex, x, length);
}

void SAL_CALL SbaXFormAdapter::setCharacterStream(sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setCharacterStream(parameterIndex, x, length);
}

void SAL_CALL SbaXFormAdapter::setObject(sal_Int32 parameterIndex, const Any& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setObject(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setObjectWithInfo(sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 scale)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setObjectWithInfo(parameterIndex, x, targetSqlType, scale);
}

void SAL_CALL SbaXFormAdapter::setRef(sal_Int32 parameterIndex, const Reference< css::sdbc::XRef >& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setRef(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setBlob(sal_Int32 parameterIndex, const Reference< css::sdbc::XBlob >& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setBlob(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setClob(sal_Int32 parameterIndex, const Reference< css::sdbc::XClob >& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setClob(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::setArray(sal_Int32 parameterIndex, const Reference< css::sdbc::XArray >& x)
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->setArray(parameterIndex, x);
}

void SAL_CALL SbaXFormAdapter::clearParameters()
{
    Reference< css::sdbc::XParameters >  xIface(m_xMainForm, UNO_QUERY);
    if (xIface.is())
        xIface->clearParameters();
}

// css::form::XDatabaseParameterBroadcaster
void SAL_CALL SbaXFormAdapter::addParameterListener(const Reference< css::form::XDatabaseParameterListener >& aListener)
{
    m_aParameterListeners.addInterface(aListener);
    if (m_aParameterListeners.getLength() == 1)
    {
        Reference< css::form::XDatabaseParameterBroadcaster >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addParameterListener(&m_aParameterListeners);
    }
}

void SAL_CALL SbaXFormAdapter::removeParameterListener(const Reference< css::form::XDatabaseParameterListener >& aListener)
{
    if (m_aParameterListeners.getLength() == 1)
    {
        Reference< css::form::XDatabaseParameterBroadcaster >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeParameterListener(&m_aParameterListeners);
    }
    m_aParameterListeners.removeInterface(aListener);
}

// css::container::XChild
Reference< XInterface > SAL_CALL SbaXFormAdapter::getParent()
{
    return m_xParent;
}

void SAL_CALL SbaXFormAdapter::setParent(const Reference< XInterface >& Parent)
{
    m_xParent = Parent;
}

// css::form::XSubmit
void SAL_CALL SbaXFormAdapter::submit(const Reference< css::awt::XControl >& aControl, const css::awt::MouseEvent& aMouseEvt)
{
    Reference< css::form::XSubmit >  xSubmit(m_xMainForm, UNO_QUERY);
    if (xSubmit.is())
        xSubmit->submit(aControl, aMouseEvt);
}

IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, form, SubmitListener, m_aSubmitListeners, css::form::XSubmit, m_xMainForm)

// css::awt::XTabControllerModel
sal_Bool SAL_CALL SbaXFormAdapter::getGroupControl()
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getGroupControl : not supported !");
    return false;
}

void SAL_CALL SbaXFormAdapter::setGroupControl(sal_Bool /*GroupControl*/)
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::setGroupControl : not supported !");
}

void SAL_CALL SbaXFormAdapter::setControlModels(const Sequence< Reference< css::awt::XControlModel >  >& /*Controls*/)
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::setControlModels : not supported !");
}

Sequence< Reference< css::awt::XControlModel > > SAL_CALL SbaXFormAdapter::getControlModels()
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getControlModels : not supported !");
    return Sequence< Reference< css::awt::XControlModel > >();
}

void SAL_CALL SbaXFormAdapter::setGroup(const Sequence< Reference< css::awt::XControlModel >  >& /*_rGroup*/, const OUString& /*GroupName*/)
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::setGroup : not supported !");
}

sal_Int32 SAL_CALL SbaXFormAdapter::getGroupCount()
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getGroupCount : not supported !");
    return 0;
}

void SAL_CALL SbaXFormAdapter::getGroup(sal_Int32 /*nGroup*/, Sequence< Reference< css::awt::XControlModel >  >& /*_rGroup*/, OUString& /*Name*/)
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getGroup : not supported !");
}

void SAL_CALL SbaXFormAdapter::getGroupByName(const OUString& /*Name*/, Sequence< Reference< css::awt::XControlModel >  >& /*_rGroup*/)
{
    OSL_FAIL("SAL_CALL SbaXFormAdapter::getGroupByName : not supported !");
}

// css::lang::XComponent
void SAL_CALL SbaXFormAdapter::dispose()
{
    // log off all multiplexers
    if (m_xMainForm.is())
        StopListening();

    css::lang::EventObject aEvt(*this);
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
    for (   ::std::vector< Reference< css::form::XFormComponent > >::const_iterator aIter = m_aChildren.begin();
            aIter != m_aChildren.end();
            ++aIter
        )
    {
        Reference< css::beans::XPropertySet >  xSet(*aIter, UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener(PROPERTY_NAME, static_cast<css::beans::XPropertyChangeListener*>(this));

        Reference< css::container::XChild >  xChild(*aIter, UNO_QUERY);
        if (xChild.is())
            xChild->setParent(Reference< XInterface > ());

        Reference< css::lang::XComponent >  xComp(*aIter, UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aChildren.clear();
}

void SAL_CALL SbaXFormAdapter::addEventListener(const Reference< css::lang::XEventListener >& xListener)
{
    m_aDisposeListeners.addInterface(xListener);
}

void SAL_CALL SbaXFormAdapter::removeEventListener(const Reference< css::lang::XEventListener >& aListener)
{
    m_aDisposeListeners.removeInterface(aListener);
}

// css::beans::XFastPropertySet
void SAL_CALL SbaXFormAdapter::setFastPropertyValue(sal_Int32 nHandle, const Any& aValue)
{
    Reference< css::beans::XFastPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    OSL_ENSURE(xSet.is(), "SAL_CALL SbaXFormAdapter::setFastPropertyValue : have no master form !");

    if (m_nNamePropHandle == nHandle)
    {
        if (aValue.getValueType().getTypeClass() != TypeClass_STRING)
        {
            throw css::lang::IllegalArgumentException();
        }

        // for notifying property listeners
        css::beans::PropertyChangeEvent aEvt;
        aEvt.Source = *this;
        aEvt.PropertyName = PROPERTY_NAME;
        aEvt.PropertyHandle = m_nNamePropHandle;
        aEvt.OldValue <<= m_sName;
        aEvt.NewValue = aValue;

        aValue >>= m_sName;

        ::cppu::OInterfaceIteratorHelper aIt(*m_aPropertyChangeListeners.getContainer(PROPERTY_NAME));
        while (aIt.hasMoreElements())
            static_cast< css::beans::XPropertyChangeListener*>(aIt.next())->propertyChange(aEvt);

        return;
    }

    xSet->setFastPropertyValue(nHandle, aValue);
}

Any SAL_CALL SbaXFormAdapter::getFastPropertyValue(sal_Int32 nHandle)
{
    Reference< css::beans::XFastPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    OSL_ENSURE(xSet.is(), "SAL_CALL SbaXFormAdapter::getFastPropertyValue : have no master form !");

    if (m_nNamePropHandle == nHandle)
        return makeAny(m_sName);

    return xSet->getFastPropertyValue(nHandle);
}

// css::container::XNamed
OUString SAL_CALL SbaXFormAdapter::getName()
{
    return ::comphelper::getString(getPropertyValue(PROPERTY_NAME));
}

void SAL_CALL SbaXFormAdapter::setName(const OUString& aName)
{
    setPropertyValue(PROPERTY_NAME, makeAny(aName));
}

// css::io::XPersistObject
OUString SAL_CALL SbaXFormAdapter::getServiceName()
{
    Reference< css::io::XPersistObject >  xPersist(m_xMainForm, UNO_QUERY);
    if (xPersist.is())
        return xPersist->getServiceName();
    return OUString();
}

void SAL_CALL SbaXFormAdapter::write(const Reference< css::io::XObjectOutputStream >& _rxOutStream)
{
    Reference< css::io::XPersistObject >  xPersist(m_xMainForm, UNO_QUERY);
    if (xPersist.is())
        xPersist->write(_rxOutStream);
}

void SAL_CALL SbaXFormAdapter::read(const Reference< css::io::XObjectInputStream >& _rxInStream)
{
    Reference< css::io::XPersistObject >  xPersist(m_xMainForm, UNO_QUERY);
    if (xPersist.is())
        xPersist->read(_rxInStream);
}

// css::beans::XMultiPropertySet
Reference< css::beans::XPropertySetInfo > SAL_CALL SbaXFormAdapter::getPropertySetInfo()
{
    Reference< css::beans::XMultiPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (!xSet.is())
        return Reference< css::beans::XPropertySetInfo > ();

    Reference< css::beans::XPropertySetInfo >  xReturn = xSet->getPropertySetInfo();
    if (-1 == m_nNamePropHandle)
    {
        // we need to determine the handle for the NAME property
 Sequence< css::beans::Property> aProps = xReturn->getProperties();
        const css::beans::Property* pProps = aProps.getConstArray();

        for (sal_Int32 i=0; i<aProps.getLength(); ++i, ++pProps)
        {
            if (pProps->Name == PROPERTY_NAME)
            {
                m_nNamePropHandle = pProps->Handle;
                break;
            }
        }
    }
    return xReturn;
}

void SAL_CALL SbaXFormAdapter::setPropertyValues(const Sequence< OUString >& PropertyNames, const Sequence< Any >& Values)
{
    Reference< css::beans::XMultiPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (xSet.is())
        xSet->setPropertyValues(PropertyNames, Values);
}

Sequence< Any > SAL_CALL SbaXFormAdapter::getPropertyValues(const Sequence< OUString >& aPropertyNames)
{
    Reference< css::beans::XMultiPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (!xSet.is())
        return Sequence< Any>(aPropertyNames.getLength());

 Sequence< Any> aReturn = xSet->getPropertyValues(aPropertyNames);

    // search for (and fake) the NAME property
    const OUString* pNames = aPropertyNames.getConstArray();
    Any* pValues = aReturn.getArray();
    OSL_ENSURE(aReturn.getLength() == aPropertyNames.getLength(), "SAL_CALL SbaXFormAdapter::getPropertyValues : the main form returned an invalid-length sequence !");
    for (sal_Int32 i=0; i<aPropertyNames.getLength(); ++i, ++pNames, ++pValues)
        if (*pNames == PROPERTY_NAME)
        {
            (*pValues) <<= m_sName;
            break;
        }

    return aReturn;
}

void SAL_CALL SbaXFormAdapter::addPropertiesChangeListener(const Sequence< OUString>& /*aPropertyNames*/, const Reference< css::beans::XPropertiesChangeListener >& xListener)
{
    // we completely ignore the property names, _all_ changes of _all_ properties will be forwarded to _all_ listeners
    m_aPropertiesChangeListeners.addInterface(xListener);
    if (m_aPropertiesChangeListeners.getLength() == 1)
    {
        Reference< css::beans::XMultiPropertySet >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addPropertiesChangeListener(Sequence< OUString>{""}, &m_aPropertiesChangeListeners);
    }
}

void SAL_CALL SbaXFormAdapter::removePropertiesChangeListener(const Reference< css::beans::XPropertiesChangeListener >& Listener)
{
    if (m_aPropertiesChangeListeners.getLength() == 1)
    {
        Reference< css::beans::XMultiPropertySet >  xBroadcaster(m_xMainForm, UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removePropertiesChangeListener(&m_aPropertiesChangeListeners);
    }
    m_aPropertiesChangeListeners.removeInterface(Listener);
}

void SAL_CALL SbaXFormAdapter::firePropertiesChangeEvent(const Sequence< OUString >& aPropertyNames, const Reference< css::beans::XPropertiesChangeListener >& xListener)
{
    Reference< css::beans::XMultiPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (xSet.is())
        xSet->firePropertiesChangeEvent(aPropertyNames, xListener);
}

// css::beans::XPropertySet
void SAL_CALL SbaXFormAdapter::setPropertyValue(const OUString& aPropertyName, const Any& aValue)
{
    Reference< css::beans::XPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (!xSet.is())
        return;

    // special handling for the "name" property
    if (aPropertyName == PROPERTY_NAME)
        setFastPropertyValue(m_nNamePropHandle, aValue);

    xSet->setPropertyValue(aPropertyName, aValue);
}

Any SAL_CALL SbaXFormAdapter::getPropertyValue(const OUString& PropertyName)
{
    Reference< css::beans::XPropertySet >  xSet(m_xMainForm, UNO_QUERY);
    if (!xSet.is())
        return Any();

    // special handling for the "name" property
    if (PropertyName == PROPERTY_NAME)
        return getFastPropertyValue(m_nNamePropHandle);

    return xSet->getPropertyValue(PropertyName);
}

IMPLEMENT_PROPERTY_LISTENER_ADMINISTRATION(SbaXFormAdapter, PropertyChangeListener, m_aPropertyChangeListeners, css::beans::XPropertySet, m_xMainForm);
IMPLEMENT_PROPERTY_LISTENER_ADMINISTRATION(SbaXFormAdapter, VetoableChangeListener, m_aVetoablePropertyChangeListeners, css::beans::XPropertySet, m_xMainForm);

// css::util::XCancellable
void SAL_CALL SbaXFormAdapter::cancel()
{
    Reference< css::util::XCancellable >  xCancel(m_xMainForm, UNO_QUERY);
    if (xCancel.is())
        return;
    xCancel->cancel();
}

// css::beans::XPropertyState
css::beans::PropertyState SAL_CALL SbaXFormAdapter::getPropertyState(const OUString& PropertyName)
{
    Reference< css::beans::XPropertyState >  xState(m_xMainForm, UNO_QUERY);
    if (xState.is())
        return xState->getPropertyState(PropertyName);
    return css::beans::PropertyState_DEFAULT_VALUE;
}

Sequence< css::beans::PropertyState> SAL_CALL SbaXFormAdapter::getPropertyStates(const Sequence< OUString >& aPropertyName)
{
    Reference< css::beans::XPropertyState >  xState(m_xMainForm, UNO_QUERY);
    if (xState.is())
        return xState->getPropertyStates(aPropertyName);

    // set them all to DEFAULT
     Sequence< css::beans::PropertyState> aReturn(aPropertyName.getLength());
    css::beans::PropertyState* pStates = aReturn.getArray();
    for (sal_Int32 i=0; i<aPropertyName.getLength(); ++i, ++pStates)
        *pStates = css::beans::PropertyState_DEFAULT_VALUE;
    return aReturn;
}

void SAL_CALL SbaXFormAdapter::setPropertyToDefault(const OUString& PropertyName)
{
    Reference< css::beans::XPropertyState >  xState(m_xMainForm, UNO_QUERY);
    if (xState.is())
        xState->setPropertyToDefault(PropertyName);
}

Any SAL_CALL SbaXFormAdapter::getPropertyDefault(const OUString& aPropertyName)
{
    Reference< css::beans::XPropertyState >  xState(m_xMainForm, UNO_QUERY);
    if (xState.is())
        return xState->getPropertyDefault(aPropertyName);
    return Any();
}

// css::form::XReset
void SAL_CALL SbaXFormAdapter::reset()
{
    Reference< css::form::XReset >  xReset(m_xMainForm, UNO_QUERY);
    if (xReset.is())
        xReset->reset();
}

IMPLEMENT_LISTENER_ADMINISTRATION(SbaXFormAdapter, form, ResetListener, m_aResetListeners, css::form::XReset, m_xMainForm)

// css::container::XNameContainer
void SbaXFormAdapter::implInsert(const Any& aElement, sal_Int32 nIndex, const OUString* pNewElName)
{
    // extract the form component
    if (aElement.getValueType().getTypeClass() != TypeClass_INTERFACE)
    {
        throw css::lang::IllegalArgumentException();
    }

    Reference< css::form::XFormComponent >  xElement(aElement, UNO_QUERY);
    if (!xElement.is())
    {
        throw css::lang::IllegalArgumentException();
    }

    // for the name we need the propset
    Reference< css::beans::XPropertySet >  xElementSet(xElement, UNO_QUERY);
    if (!xElementSet.is())
    {
        throw css::lang::IllegalArgumentException();
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
        throw css::lang::IllegalArgumentException();
    }

    // check the index
    OSL_ASSERT(nIndex >= 0);
    if (sal::static_int_cast< sal_uInt32 >(nIndex) > m_aChildren.size())
        nIndex = m_aChildren.size();

    OSL_ENSURE(m_aChildren.size() == m_aChildNames.size(), "SAL_CALL SbaXFormAdapter::implInsert : inconsistent container state !");
    m_aChildren.insert(m_aChildren.begin() + nIndex, xElement);
    m_aChildNames.insert(m_aChildNames.begin() + nIndex, sName);

    // listen for a changes of the name
    xElementSet->addPropertyChangeListener(PROPERTY_NAME, static_cast<css::beans::XPropertyChangeListener*>(this));

    // we are now the parent of the new element
    xElement->setParent(static_cast<css::container::XContainer*>(this));

    // notify the container listeners
    css::container::ContainerEvent aEvt;
    aEvt.Source = *this;
    aEvt.Accessor <<= nIndex;
    aEvt.Element <<= xElement;
    ::comphelper::OInterfaceIteratorHelper2 aIt(m_aContainerListeners);
    while (aIt.hasMoreElements())
        static_cast< css::container::XContainerListener*>(aIt.next())->elementInserted(aEvt);
}

sal_Int32 SbaXFormAdapter::implGetPos(const OUString& rName)
{
    ::std::vector< OUString>::const_iterator aIter = ::std::find_if(   m_aChildNames.begin(),
                                                                m_aChildNames.end(),
                                                                ::std::bind2nd(::std::equal_to< OUString>(),rName));

    if(aIter != m_aChildNames.end())
        return aIter - m_aChildNames.begin();

    return -1;
}

void SAL_CALL SbaXFormAdapter::insertByName(const OUString& aName, const Any& aElement)
{
    implInsert(aElement, m_aChildren.size(), &aName);
}

void SAL_CALL SbaXFormAdapter::removeByName(const OUString& Name)
{
    sal_Int32 nPos = implGetPos(Name);
    if (-1 == nPos)
    {
        throw css::container::NoSuchElementException();
    }
    removeByIndex(nPos);
}

// css::container::XNameReplace
void SAL_CALL SbaXFormAdapter::replaceByName(const OUString& aName, const Any& aElement)
{
    sal_Int32 nPos = implGetPos(aName);
    if (-1 == nPos)
    {
        throw css::container::NoSuchElementException();
    }
    replaceByIndex(nPos, aElement);
}

// css::container::XNameAccess
Any SAL_CALL SbaXFormAdapter::getByName(const OUString& aName)
{
    sal_Int32 nPos = implGetPos(aName);
    if (-1 == nPos)
    {
        throw css::container::NoSuchElementException();
    }
    return makeAny(m_aChildren[nPos]);
}

Sequence< OUString > SAL_CALL SbaXFormAdapter::getElementNames()
{
    return Sequence< OUString >(m_aChildNames.data(), m_aChildNames.size());
}

sal_Bool SAL_CALL SbaXFormAdapter::hasByName(const OUString& aName)
{
    return (-1 != implGetPos(aName));
}

// css::container::XElementAccess
Type SAL_CALL SbaXFormAdapter::getElementType()
{
    return cppu::UnoType<css::form::XFormComponent>::get();
}

sal_Bool SAL_CALL SbaXFormAdapter::hasElements()
{
    return m_aChildren.size() > 0;
}

// css::container::XIndexContainer
void SAL_CALL SbaXFormAdapter::insertByIndex(sal_Int32 _rIndex, const Any& Element)
{
    if ( ( _rIndex < 0 ) || ( (size_t)_rIndex >= m_aChildren.size() ) )
        throw css::lang::IndexOutOfBoundsException();
    implInsert(Element, _rIndex);
}

void SAL_CALL SbaXFormAdapter::removeByIndex(sal_Int32 _rIndex)
{
    if ( ( _rIndex < 0 ) || ( (size_t)_rIndex >= m_aChildren.size() ) )
        throw css::lang::IndexOutOfBoundsException();

    Reference< css::form::XFormComponent >  xAffected = *(m_aChildren.begin() + _rIndex);

    OSL_ENSURE(m_aChildren.size() == m_aChildNames.size(), "SAL_CALL SbaXFormAdapter::removeByIndex : inconsistent container state !");
    m_aChildren.erase(m_aChildren.begin() + _rIndex);
    m_aChildNames.erase(m_aChildNames.begin() + _rIndex);

    // no need to listen anymore
    Reference< css::beans::XPropertySet >  xAffectedSet(xAffected, UNO_QUERY);
    xAffectedSet->removePropertyChangeListener(PROPERTY_NAME, static_cast<css::beans::XPropertyChangeListener*>(this));

    // we are no longer the parent
    xAffected->setParent(Reference< XInterface > ());

    // notify container listeners
    css::container::ContainerEvent aEvt;
    aEvt.Source = *this;
    aEvt.Element <<= xAffected;
    ::comphelper::OInterfaceIteratorHelper2 aIt(m_aContainerListeners);
    while (aIt.hasMoreElements())
        static_cast< css::container::XContainerListener*>(aIt.next())->elementRemoved(aEvt);

}

// css::container::XIndexReplace
void SAL_CALL SbaXFormAdapter::replaceByIndex(sal_Int32 _rIndex, const Any& Element)
{
    if ( ( _rIndex < 0 ) || ( (size_t)_rIndex >= m_aChildren.size() ) )
        throw css::lang::IndexOutOfBoundsException();

    // extract the form component
    if (Element.getValueType().getTypeClass() != TypeClass_INTERFACE)
    {
        throw css::lang::IllegalArgumentException();
    }

    Reference< css::form::XFormComponent >  xElement(Element, UNO_QUERY);
    if (!xElement.is())
    {
        throw css::lang::IllegalArgumentException();
    }

    // for the name we need the propset
    Reference< css::beans::XPropertySet >  xElementSet(xElement, UNO_QUERY);
    if (!xElementSet.is())
    {
        throw css::lang::IllegalArgumentException();
     }
    OUString sName;
    try
    {
        xElementSet->getPropertyValue(PROPERTY_NAME) >>= sName;
    }
    catch(Exception&)
    {
        // the set didn't support the name prop
        throw css::lang::IllegalArgumentException();
    }

    Reference< css::form::XFormComponent >  xOld = *(m_aChildren.begin() + _rIndex);

    OSL_ENSURE(m_aChildren.size() == m_aChildNames.size(), "SAL_CALL SbaXFormAdapter::replaceByIndex : inconsistent container state !");
    *(m_aChildren.begin() + _rIndex) = xElement;
    *(m_aChildNames.begin() + _rIndex) = sName;

    // correct property change listening
    Reference< css::beans::XPropertySet >  xOldSet(xOld, UNO_QUERY);
    xOldSet->removePropertyChangeListener(PROPERTY_NAME, static_cast<css::beans::XPropertyChangeListener*>(this));
    xElementSet->addPropertyChangeListener(PROPERTY_NAME, static_cast<css::beans::XPropertyChangeListener*>(this));

    // parent reset
    xOld->setParent(Reference< XInterface > ());
    xElement->setParent(static_cast<css::container::XContainer*>(this));

    // notify container listeners
    css::container::ContainerEvent aEvt;
    aEvt.Source = *this;
    aEvt.Accessor <<= (sal_Int32)_rIndex;
    aEvt.Element <<= xElement;
    aEvt.ReplacedElement <<= xOld;

    ::comphelper::OInterfaceIteratorHelper2 aIt(m_aContainerListeners);
    while (aIt.hasMoreElements())
        static_cast< css::container::XContainerListener*>(aIt.next())->elementReplaced(aEvt);
}

// css::container::XIndexAccess
sal_Int32 SAL_CALL SbaXFormAdapter::getCount()
{
    return m_aChildren.size();
}

Any SAL_CALL SbaXFormAdapter::getByIndex(sal_Int32 _rIndex)
{
    if ( ( _rIndex < 0 ) || ( (size_t)_rIndex >= m_aChildren.size() ) )
        throw css::lang::IndexOutOfBoundsException();

    Reference< css::form::XFormComponent >  xElement = *(m_aChildren.begin() + _rIndex);
    return makeAny(xElement);
}

// css::container::XContainer
void SAL_CALL SbaXFormAdapter::addContainerListener(const Reference< css::container::XContainerListener >& xListener)
{
    m_aContainerListeners.addInterface(xListener);
}

void SAL_CALL SbaXFormAdapter::removeContainerListener(const Reference< css::container::XContainerListener >& xListener)
{
    m_aContainerListeners.removeInterface(xListener);
}

// css::container::XEnumerationAccess
Reference< css::container::XEnumeration > SAL_CALL SbaXFormAdapter::createEnumeration()
{
    return new ::comphelper::OEnumerationByName(this);
}

// css::beans::XPropertyChangeListener
void SAL_CALL SbaXFormAdapter::propertyChange(const css::beans::PropertyChangeEvent& evt)
{
    if (evt.PropertyName == PROPERTY_NAME)
    {
        ::std::vector<  css::uno::Reference< css::form::XFormComponent > >::const_iterator aIter = ::std::find_if(  m_aChildren.begin(),
                                                                m_aChildren.end(),
                                                                ::std::bind2nd(::std::equal_to< css::uno::Reference< css::uno::XInterface > >(),evt.Source));

        if(aIter != m_aChildren.end())
        {
            sal_Int32 nPos = aIter - m_aChildren.begin();
            OSL_ENSURE(*(m_aChildNames.begin() + nPos) == ::comphelper::getString(evt.OldValue), "SAL_CALL SbaXFormAdapter::propertyChange : object has a wrong name !");
            *(m_aChildNames.begin() + nPos) = ::comphelper::getString(evt.NewValue);
        }
    }
}

// css::lang::XEventListener
void SAL_CALL SbaXFormAdapter::disposing(const css::lang::EventObject& Source)
{
    // was it our main form ?
    if (Source.Source == m_xMainForm)
        dispose();

    ::std::vector<  css::uno::Reference< css::form::XFormComponent > >::const_iterator aIter = ::std::find_if(  m_aChildren.begin(),
                                                                m_aChildren.end(),
                                                                ::std::bind2nd(::std::equal_to< css::uno::Reference< css::uno::XInterface > >(),Source.Source));
    if(aIter != m_aChildren.end())
            removeByIndex(aIter - m_aChildren.begin());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
