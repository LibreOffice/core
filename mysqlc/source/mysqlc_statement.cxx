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

#include <sal/config.h>

#include <memory>
#include <stdio.h>

#include "mysqlc_connection.hxx"
#include "mysqlc_propertyids.hxx"
#include "mysqlc_resultset.hxx"
#include "mysqlc_statement.hxx"
#include "mysqlc_general.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>

#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>

using namespace connectivity::mysqlc;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using ::osl::MutexGuard;

OCommonStatement::OCommonStatement(OConnection* _pConnection, sql::Statement *_cppStatement)
    :OCommonStatement_IBase(m_aMutex)
    ,OPropertySetHelper(OCommonStatement_IBase::rBHelper)
    ,OStatement_CBase( static_cast<cppu::OWeakObject*>(_pConnection), this )
    ,m_pConnection(_pConnection)
    ,cppStatement(_cppStatement)
    ,rBHelper(OCommonStatement_IBase::rBHelper)
{
    OSL_TRACE("OCommonStatement::OCommonStatement");
    m_pConnection->acquire();
}

OCommonStatement::~OCommonStatement()
{
    OSL_TRACE("OCommonStatement::~OCommonStatement");
}

void OCommonStatement::disposeResultSet()
{
    OSL_TRACE("OCommonStatement::disposeResultSet");
    // free the cursor if alive
    delete cppStatement;
    cppStatement = NULL;
}

void OCommonStatement::disposing()
{
    OSL_TRACE("OCommonStatement::disposing");
    MutexGuard aGuard(m_aMutex);

    disposeResultSet();

    if (m_pConnection) {
        m_pConnection->release();
        m_pConnection = NULL;
    }
    delete cppStatement;

    dispose_ChildImpl();
    OCommonStatement_IBase::disposing();
}

Any SAL_CALL OCommonStatement::queryInterface(const Type & rType)
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::queryInterface");
    Any aRet = OCommonStatement_IBase::queryInterface(rType);
    if (!aRet.hasValue()) {
        aRet = OPropertySetHelper::queryInterface(rType);
    }
    return aRet;
}

Sequence< Type > SAL_CALL OCommonStatement::getTypes()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::getTypes");
    ::cppu::OTypeCollection aTypes( cppu::UnoType<XMultiPropertySet>::get(),
                                    cppu::UnoType<XFastPropertySet>::get(),
                                    cppu::UnoType<XPropertySet>::get());

    return concatSequences(aTypes.getTypes(), OCommonStatement_IBase::getTypes());
}

void SAL_CALL OCommonStatement::cancel()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::cancel");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);
    // cancel the current sql statement
}

void SAL_CALL OCommonStatement::close()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::close");
    /*
      We need a block for the checkDisposed call.
      After the check we can call dispose() as we are not under lock ??
    */
    {
        MutexGuard aGuard(m_aMutex);
        checkDisposed(rBHelper.bDisposed);
    }
    dispose();
}

void SAL_CALL OStatement::clearBatch()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OStatement::clearBatch");
    // if you support batches clear it here
}

sal_Bool SAL_CALL OCommonStatement::execute(const rtl::OUString& sql)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::execute");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);
    const rtl::OUString sSqlStatement = m_pConnection->transFormPreparedStatement( sql );

    bool success = false;
    try {
        success = cppStatement->execute(rtl::OUStringToOString(sSqlStatement, m_pConnection->getConnectionSettings().encoding).getStr());
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
    return success;
}

Reference< XResultSet > SAL_CALL OCommonStatement::executeQuery(const rtl::OUString& sql)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::executeQuery");

    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);
    const rtl::OUString sSqlStatement = m_pConnection->transFormPreparedStatement(sql);

    Reference< XResultSet > xResultSet;
    try {
        std::unique_ptr< sql::ResultSet > rset(cppStatement->executeQuery(rtl::OUStringToOString(sSqlStatement, m_pConnection->getConnectionEncoding()).getStr()));
        xResultSet = new OResultSet(this, rset.get(), m_pConnection->getConnectionEncoding());
        rset.release();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
    return xResultSet;
}

Reference< XConnection > SAL_CALL OCommonStatement::getConnection()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::getConnection");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    // just return our connection here
    return m_pConnection;
}

sal_Int32 SAL_CALL OCommonStatement::getUpdateCount()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::getUpdateCount");
    return 0;
}

Any SAL_CALL OStatement::queryInterface(const Type & rType)
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OStatement::queryInterface");
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    if (!aRet.hasValue()) {
        aRet = OCommonStatement::queryInterface(rType);
    }
    return aRet;
}

void SAL_CALL OStatement::addBatch(const rtl::OUString& sql)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OStatement::addBatch");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    m_aBatchList.push_back(sql);
}

Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OStatement::executeBatch");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    Sequence< sal_Int32 > aRet = Sequence< sal_Int32 >();
    return aRet;
}

sal_Int32 SAL_CALL OCommonStatement::executeUpdate(const rtl::OUString& sql)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::executeUpdate");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);
    const rtl::OUString sSqlStatement = m_pConnection->transFormPreparedStatement(sql);

    sal_Int32 affectedRows = 0;
    try {
        affectedRows = cppStatement->executeUpdate(rtl::OUStringToOString(sSqlStatement, m_pConnection->getConnectionEncoding()).getStr());
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
    return affectedRows;
}

Reference< XResultSet > SAL_CALL OCommonStatement::getResultSet()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::getResultSet");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    Reference< XResultSet > xResultSet;
    try {
        std::unique_ptr< sql::ResultSet > rset(cppStatement->getResultSet());
        xResultSet = new OResultSet(this, rset.get(), m_pConnection->getConnectionEncoding());
        rset.release();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
    return xResultSet;
}

sal_Bool SAL_CALL OCommonStatement::getMoreResults()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::getMoreResults");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    // if your driver supports more than only one resultset
    // and has one more at this moment return true
    return sal_False;
}

Any SAL_CALL OCommonStatement::getWarnings()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::getWarnings");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}

void SAL_CALL OCommonStatement::clearWarnings()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::clearWarnings");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    m_aLastWarning = SQLWarning();
}

::cppu::IPropertyArrayHelper* OCommonStatement::createArrayHelper( ) const
{
    OSL_TRACE("OCommonStatement::createArrayHelper");
    // this properties are define by the service statement
    // they must in alphabetic order
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = Property("CursorName", PROPERTY_ID_CURSORNAME, cppu::UnoType<rtl::OUString>::get(), 0);
    pProperties[nPos++] = Property("EscapeProcessing", PROPERTY_ID_ESCAPEPROCESSING, cppu::UnoType<bool>::get(), 0);
    pProperties[nPos++] = Property("FetchDirection", PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("FetchSize", PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("MaxFieldSize", PROPERTY_ID_MAXFIELDSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("MaxRows", PROPERTY_ID_MAXROWS, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("QueryTimeOut", PROPERTY_ID_QUERYTIMEOUT, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("ResultSetConcurrency", PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("ResultSetType", PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("UseBookmarks", PROPERTY_ID_USEBOOKMARKS, cppu::UnoType<bool>::get(), 0);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

::cppu::IPropertyArrayHelper & OCommonStatement::getInfoHelper()
{
    OSL_TRACE("OCommonStatement::getInfoHelper");
    return *getArrayHelper();
}

sal_Bool OCommonStatement::convertFastPropertyValue(
        Any & /* rConvertedValue */, Any & /* rOldValue */,
        sal_Int32 /* nHandle */, const Any& /* rValue */)
    throw (IllegalArgumentException)
{
    OSL_TRACE("OCommonStatement::convertFastPropertyValue");
    bool bConverted = false;
    // here we have to try to convert
    return bConverted;
}

void OCommonStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& /* rValue */)
    throw (Exception, std::exception)
{
    OSL_TRACE("OCommonStatement::setFastPropertyValue_NoBroadcast");
    // set the value to what ever is necessary
    switch (nHandle) {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}

void OCommonStatement::getFastPropertyValue(Any& _rValue, sal_Int32 nHandle) const
{
    OSL_TRACE("OCommonStatement::getFastPropertyValue");
    switch (nHandle)    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            _rValue <<= sal_False;
            break;
        default:
            ;
    }
}

rtl::OUString OStatement::getImplementationName() throw (css::uno::RuntimeException, std::exception)
{
    return rtl::OUString("com.sun.star.sdbcx.OStatement");
}

css::uno::Sequence<rtl::OUString> OStatement::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<rtl::OUString> s(1);
    s[0] = "com.sun.star.sdbc.Statement";
    return s;
}

sal_Bool OStatement::supportsService(rtl::OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

void SAL_CALL OCommonStatement::acquire()
    throw()
{
    OSL_TRACE("OCommonStatement::acquire");
    OCommonStatement_IBase::acquire();
}

void SAL_CALL OCommonStatement::release()
    throw()
{
    OSL_TRACE("OCommonStatement::release");
    relase_ChildImpl();
}

void SAL_CALL OStatement::acquire()
    throw()
{
    OSL_TRACE("OStatement::acquire");
    OCommonStatement::acquire();
}

void SAL_CALL OStatement::release()
    throw()
{
    OSL_TRACE("OStatement::release");
    OCommonStatement::release();
}

Reference< css::beans::XPropertySetInfo > SAL_CALL OCommonStatement::getPropertySetInfo()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OCommonStatement::getPropertySetInfo");
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
