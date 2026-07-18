/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include "MacabPreparedStatement.hxx"
#include "MacabAddressBook.hxx"
#include <propertyids.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace cpo::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(MacabPreparedStatement, "com.sun.star.sdbc.drivers.MacabPreparedStatement", "com.sun.star.sdbc.PreparedStatement");

void MacabPreparedStatement::checkAndResizeParameters(sal_Int32 nParams)
{
    if ( !m_aParameterRow.is() )
        m_aParameterRow = new OValueVector();

    if (nParams < 1)
        ::dbtools::throwInvalidIndexException(*this);

    if (nParams >= static_cast<sal_Int32>(m_aParameterRow->size()))
        m_aParameterRow->resize(nParams);
}

void MacabPreparedStatement::setMacabFields() const
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
    m_xMetaData->setMacabFields(xColumns);
}

void MacabPreparedStatement::resetParameters() const
{
    m_nParameterIndex = 0;
}

void MacabPreparedStatement::getNextParameter(OUString &rParameter) const
{
    if (m_nParameterIndex >= static_cast<sal_Int32>(m_aParameterRow->size()))
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(
                STR_INVALID_PARA_COUNT
             ) );
        ::dbtools::throwGenericSQLException(sError,*const_cast<MacabPreparedStatement *>(this));
    }

    rParameter = (*m_aParameterRow)[m_nParameterIndex].getString();

    m_nParameterIndex++;
}

MacabPreparedStatement::MacabPreparedStatement(
    MacabConnection* _pConnection,
    const OUString& sql)
    : MacabPreparedStatement_BASE(_pConnection),
      m_sSqlStatement(sql),
      m_bPrepared(false),
      m_nParameterIndex(0),
      m_aParameterRow()
{

}

MacabPreparedStatement::~MacabPreparedStatement()
{
}

void MacabPreparedStatement::disposing()
{
    MacabPreparedStatement_BASE::disposing();

    if (m_aParameterRow.is())
    {
        m_aParameterRow->clear();
        m_aParameterRow = nullptr;
    }
}

Reference< XResultSetMetaData > MacabPreparedStatement::getMetaData()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    if (!m_xMetaData.is())
    {
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        OUString sTableName = MacabAddressBook::getDefaultTableName();

        if(! xTabs.empty() )
        {

            // can only deal with one table at a time
            if(xTabs.size() == 1 && !m_aSQLIterator.hasErrors() )
                sTableName = xTabs.begin()->first;

        }
        m_xMetaData = new MacabResultSetMetaData(getOwnConnection(),sTableName);
        setMacabFields();
    }
    Reference< XResultSetMetaData > xMetaData = m_xMetaData;
    return xMetaData;
}

void MacabPreparedStatement::close()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    // Reset last warning message
    try {
        clearWarnings ();
        MacabCommonStatement::close();
    }
    catch (SQLException &) {
        // If we get an error, ignore
    }

    // Remove this Statement object from the Connection object's
    // list
}

bool MacabPreparedStatement::execute()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet> xRS = MacabCommonStatement::executeQuery(m_sSqlStatement);

    return xRS.is();
}

sal_Int32 MacabPreparedStatement::executeUpdate()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    // same as in statement with the difference that this statement also can contain parameter
    return 0;
}

Reference< XConnection > MacabPreparedStatement::getConnection()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    return m_pConnection;
}

Reference< XResultSet > MacabPreparedStatement::executeQuery()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > rs = MacabCommonStatement::executeQuery(m_sSqlStatement);

    return rs;
}

void MacabPreparedStatement::setNull(sal_Int32 parameterIndex, sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    checkAndResizeParameters(parameterIndex);

    (*m_aParameterRow)[parameterIndex - 1].setNull();
}

void MacabPreparedStatement::setObjectNull(sal_Int32, sal_Int32, const OUString&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setObjectNull", nullptr);
}

void MacabPreparedStatement::setBoolean(sal_Int32, bool)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setBoolean", nullptr);
}

void MacabPreparedStatement::setByte(sal_Int32, sal_Int8)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setByte", nullptr);
}

void MacabPreparedStatement::setShort(sal_Int32, sal_Int16)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setShort", nullptr);
}

void MacabPreparedStatement::setInt(sal_Int32, sal_Int32)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setInt", nullptr);
}

void MacabPreparedStatement::setLong(sal_Int32, sal_Int64)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setLong", nullptr);
}

void MacabPreparedStatement::setFloat(sal_Int32, float)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setFloat", nullptr);
}

void MacabPreparedStatement::setDouble(sal_Int32, double)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setDouble", nullptr);
}

void MacabPreparedStatement::setString(sal_Int32 parameterIndex, const OUString &x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    checkAndResizeParameters(parameterIndex);

    (*m_aParameterRow)[parameterIndex - 1] = x;
}

void MacabPreparedStatement::setBytes(sal_Int32, const Sequence< sal_Int8 >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setBytes", nullptr);
}

void MacabPreparedStatement::setDate(sal_Int32, const Date&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setDate", nullptr);
}

void MacabPreparedStatement::setTime(sal_Int32, const css::util::Time&)
{

    ::dbtools::throwFunctionNotSupportedSQLException("setTime", nullptr);
}

void MacabPreparedStatement::setTimestamp(sal_Int32, const DateTime&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setTimestamp", nullptr);
}

void MacabPreparedStatement::setBinaryStream(sal_Int32, const Reference< css::io::XInputStream >&, sal_Int32)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setBinaryStream", nullptr);
}

void MacabPreparedStatement::setCharacterStream(sal_Int32, const Reference< css::io::XInputStream >&, sal_Int32)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setCharacterStream", nullptr);
}

void MacabPreparedStatement::setObject(sal_Int32 parameterIndex, const Any& x)
{
    if(!::dbtools::implSetObject(this,parameterIndex,x))
    {
        const OUString sError( m_pConnection->getResources().getResourceStringWithSubstitution(
                STR_UNKNOWN_PARA_TYPE,
                "$position$", OUString::number(parameterIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
}

void MacabPreparedStatement::setObjectWithInfo(sal_Int32, const Any&, sal_Int32, sal_Int32)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setObjectWithInfo", nullptr);
}

void MacabPreparedStatement::setRef(sal_Int32, const Reference< XRef >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setRef", nullptr);
}

void MacabPreparedStatement::setBlob(sal_Int32, const Reference< XBlob >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setBlob", nullptr);
}

void MacabPreparedStatement::setClob(sal_Int32, const Reference< XClob >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setClob", nullptr);
}

void MacabPreparedStatement::setArray(sal_Int32, const Reference< XArray >&)
{
    ::dbtools::throwFunctionNotSupportedSQLException("setArray", nullptr);
}

void MacabPreparedStatement::clearParameters()
{
    ::dbtools::throwFunctionNotSupportedSQLException("clearParameters", nullptr);
}

void MacabPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
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
            MacabCommonStatement::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
