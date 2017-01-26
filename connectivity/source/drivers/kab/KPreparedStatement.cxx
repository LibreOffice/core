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


#include "KPreparedStatement.hxx"
#include "propertyids.hxx"
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include "resource/kab_res.hrc"
#include "resource/sharedresources.hxx"

using namespace connectivity::kab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(KabPreparedStatement, "com.sun.star.sdbc.drivers.KabPreparedStatement", "com.sun.star.sdbc.PreparedStatement");

void KabPreparedStatement::checkAndResizeParameters(sal_Int32 nParams)
{
    if ( !m_aParameterRow.is() )
        m_aParameterRow = new OValueVector();

    if (nParams < 1)
        ::dbtools::throwInvalidIndexException(*this);

    if (nParams >= (sal_Int32) (m_aParameterRow->get()).size())
        (m_aParameterRow->get()).resize(nParams);
}

void KabPreparedStatement::setKabFields() const
{
    ::rtl::Reference<connectivity::OSQLColumns> xColumns;   // selected columns

    xColumns = m_aSQLIterator.getSelectColumns();
    if (!xColumns.is())
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(
                STR_INVALID_COLUMN_SELECTION
             ) );
        ::dbtools::throwGenericSQLException(sError,nullptr);
    }
    m_xMetaData->setKabFields(xColumns);
}

void KabPreparedStatement::resetParameters() const
{
    m_nParameterIndex = 0;
}

void KabPreparedStatement::getNextParameter(OUString &rParameter) const
{
    if (m_nParameterIndex >= (sal_Int32) (m_aParameterRow->get()).size())
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(
                STR_INVALID_PARA_COUNT
             ) );
        ::dbtools::throwGenericSQLException(sError,*const_cast<KabPreparedStatement *>(this));
    } // if (m_nParameterIndex >= (sal_Int32) (*m_aParameterRow).size())

    rParameter = (m_aParameterRow->get())[m_nParameterIndex];

    m_nParameterIndex++;
}

KabPreparedStatement::KabPreparedStatement(
    KabConnection* _pConnection,
    const OUString& sql)
    : KabPreparedStatement_BASE(_pConnection),
      m_sSqlStatement(sql),
      m_bPrepared(false),
      m_nParameterIndex(0),
      m_aParameterRow()
{
}

KabPreparedStatement::~KabPreparedStatement()
{
}

void KabPreparedStatement::disposing()
{
    KabPreparedStatement_BASE::disposing();

    if (m_aParameterRow.is())
    {
        m_aParameterRow->get().clear();
        m_aParameterRow = nullptr;
    }
}

Reference< XResultSetMetaData > SAL_CALL KabPreparedStatement::getMetaData()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    if (!m_xMetaData.is())
    {
        m_xMetaData = new KabResultSetMetaData;
        setKabFields();
    }
    Reference< XResultSetMetaData > xMetaData = m_xMetaData.get();
    return xMetaData;
}

void SAL_CALL KabPreparedStatement::close()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    // Reset last warning message
    try {
        clearWarnings ();
        KabCommonStatement::close();
    }
    catch (SQLException &) {
        // If we get an error, ignore
    }

    // Remove this Statement object from the Connection object's
    // list
}

sal_Bool SAL_CALL KabPreparedStatement::execute()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet> xRS = KabCommonStatement::executeQuery(m_sSqlStatement);

    return xRS.is();
}

sal_Int32 SAL_CALL KabPreparedStatement::executeUpdate()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    // same as in statement with the difference that this statement also can contain parameter
    return 0;
}

Reference< XConnection > SAL_CALL KabPreparedStatement::getConnection()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    return m_pConnection;
}

Reference< XResultSet > SAL_CALL KabPreparedStatement::executeQuery()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > rs = KabCommonStatement::executeQuery(m_sSqlStatement);

    return rs;
}

void SAL_CALL KabPreparedStatement::setNull(sal_Int32 parameterIndex, sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    checkAndResizeParameters(parameterIndex);

    (m_aParameterRow->get())[parameterIndex - 1].setNull();
}

void SAL_CALL KabPreparedStatement::setObjectNull(sal_Int32, sal_Int32, const OUString&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setObjectNull", nullptr);
}

void SAL_CALL KabPreparedStatement::setBoolean(sal_Int32, sal_Bool)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setBoolean", nullptr);
}

void SAL_CALL KabPreparedStatement::setByte(sal_Int32, sal_Int8)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setByte", nullptr);
}

void SAL_CALL KabPreparedStatement::setShort(sal_Int32, sal_Int16)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setShort", nullptr);
}

void SAL_CALL KabPreparedStatement::setInt(sal_Int32, sal_Int32)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setInt", nullptr);
}

void SAL_CALL KabPreparedStatement::setLong(sal_Int32, sal_Int64)
{
    ::dbtools::throwFunctionNotSupportedSQLException("", nullptr);
}

void SAL_CALL KabPreparedStatement::setFloat(sal_Int32, float)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setFloat", nullptr);
}

void SAL_CALL KabPreparedStatement::setDouble(sal_Int32, double)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setDouble", nullptr);
}

void SAL_CALL KabPreparedStatement::setString(sal_Int32 parameterIndex, const OUString &x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    checkAndResizeParameters(parameterIndex);

    (m_aParameterRow->get())[parameterIndex - 1] = x;
}

void SAL_CALL KabPreparedStatement::setBytes(sal_Int32, const Sequence< sal_Int8 >&)
{
   ::dbtools::throwFunctionNotSupportedSQLException("setBytes", nullptr);
}

void SAL_CALL KabPreparedStatement::setDate(sal_Int32, const Date&)
{
   ::dbtools::throwFunctionNotSupportedSQLException("setDate", nullptr);
}

void SAL_CALL KabPreparedStatement::setTime(sal_Int32, const css::util::Time&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setTime", nullptr);
}

void SAL_CALL KabPreparedStatement::setTimestamp(sal_Int32, const DateTime&)
{

   ::dbtools::throwFunctionNotSupportedSQLException("setTimestamp", nullptr);
}

void SAL_CALL KabPreparedStatement::setBinaryStream(sal_Int32, const Reference< css::io::XInputStream >&, sal_Int32)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setBinaryStream", nullptr);
}

void SAL_CALL KabPreparedStatement::setCharacterStream(sal_Int32, const Reference< css::io::XInputStream >&, sal_Int32)
{
   ::dbtools::throwFunctionNotSupportedSQLException("setCharacterStream", nullptr);
}

void SAL_CALL KabPreparedStatement::setObject(sal_Int32 parameterIndex, const Any& x)
{
    if(!::dbtools::implSetObject(this,parameterIndex,x))
    {
        throw SQLException();
    }
}

void SAL_CALL KabPreparedStatement::setObjectWithInfo(sal_Int32, const Any&, sal_Int32, sal_Int32)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setObjectWithInfo", nullptr);
}

void SAL_CALL KabPreparedStatement::setRef(sal_Int32, const Reference< XRef >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setRef", nullptr);
}

void SAL_CALL KabPreparedStatement::setBlob(sal_Int32, const Reference< XBlob >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setBlob", nullptr);
}

void SAL_CALL KabPreparedStatement::setClob(sal_Int32, const Reference< XClob >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setClob", nullptr);
}

void SAL_CALL KabPreparedStatement::setArray(sal_Int32, const Reference< XArray >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setArray", nullptr);
}

void SAL_CALL KabPreparedStatement::clearParameters()
{
   ::dbtools::throwFunctionNotSupportedSQLException("clearParameters", nullptr);
}

void KabPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
{
    switch (nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            break;
        default:
            KabCommonStatement::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
