/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MacabPreparedStatement.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_connectivity.hxx"

#include "MacabPreparedStatement.hxx"
#include "MacabAddressBook.hxx"
#include "propertyids.hxx"
#include <connectivity/dbexception.hxx>

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(MacabPreparedStatement, "com.sun.star.sdbc.drivers.MacabPreparedStatement", "com.sun.star.sdbc.PreparedStatement");
// -------------------------------------------------------------------------
void MacabPreparedStatement::checkAndResizeParameters(sal_Int32 nParams) throw(SQLException)
{
    if ( !m_aParameterRow.isValid() )
        m_aParameterRow = new OValueVector();

    if (nParams < 1)
        ::dbtools::throwSQLException(
            "SQL statement parameters are numbered starting at 1.",
            ::dbtools::SQL_INVALID_DESCRIPTOR_INDEX,
            *(MacabPreparedStatement *) this);

    if (nParams >= (sal_Int32) (*m_aParameterRow).size())
        (*m_aParameterRow).resize(nParams);
}
// -------------------------------------------------------------------------
void MacabPreparedStatement::setMacabFields() const throw(SQLException)
{
    ::vos::ORef<connectivity::OSQLColumns> xColumns;    // selected columns

    xColumns = m_aSQLIterator.getSelectColumns();
    if (!xColumns.isValid())
    {
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii("Invalid selection of columns"),
            NULL);
    }
    m_xMetaData->setMacabFields(xColumns);
}
// -------------------------------------------------------------------------
void MacabPreparedStatement::resetParameters() const throw(SQLException)
{
    m_nParameterIndex = 0;
}
// -------------------------------------------------------------------------
void MacabPreparedStatement::getNextParameter(::rtl::OUString &rParameter) const throw(SQLException)
{
    if (m_nParameterIndex >= (sal_Int32) (*m_aParameterRow).size())
        ::dbtools::throwSQLException(
            "More parameters in SQL statement than set.",
            ::dbtools::SQL_INVALID_DESCRIPTOR_INDEX,
            *(MacabPreparedStatement *) this);

    rParameter = (*m_aParameterRow)[m_nParameterIndex];

    m_nParameterIndex++;
}
// -------------------------------------------------------------------------
MacabPreparedStatement::MacabPreparedStatement(
    MacabConnection* _pConnection,
    const ::rtl::OUString& sql)
    : MacabPreparedStatement_BASE(_pConnection),
      m_sSqlStatement(sql),
      m_bPrepared(sal_False),
      m_nParameterIndex(0),
      m_aParameterRow()
{

}
// -------------------------------------------------------------------------
MacabPreparedStatement::~MacabPreparedStatement()
{
}
// -------------------------------------------------------------------------
void MacabPreparedStatement::disposing()
{
    MacabPreparedStatement_BASE::disposing();

    if (m_aParameterRow.isValid())
    {
        m_aParameterRow->clear();
        m_aParameterRow = NULL;
    }
}
// -------------------------------------------------------------------------
Reference< XResultSetMetaData > SAL_CALL MacabPreparedStatement::getMetaData() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    if (!m_xMetaData.is())
    {
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        ::rtl::OUString sTableName = MacabAddressBook::getDefaultTableName();

        if(! xTabs.empty() )
        {

            // can only deal with one table at a time
            if(xTabs.size() == 1 && !m_aSQLIterator.hasErrors() )
                sTableName = xTabs.begin()->first;

        }
        m_xMetaData = new MacabResultSetMetaData(getOwnConnection(),sTableName);
        setMacabFields();
    }
    Reference< XResultSetMetaData > xMetaData = m_xMetaData.get();
    return xMetaData;
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::close() throw(SQLException, RuntimeException)
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabPreparedStatement::execute() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet> xRS = MacabCommonStatement::executeQuery(m_sSqlStatement);

    return xRS.is();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabPreparedStatement::executeUpdate() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    // same as in statement with the difference that this statement also can contain parameter
    return 0;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL MacabPreparedStatement::getConnection() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >) m_pConnection;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL MacabPreparedStatement::executeQuery() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > rs = MacabCommonStatement::executeQuery(m_sSqlStatement);

    return rs;
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setNull(sal_Int32 parameterIndex, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    checkAndResizeParameters(parameterIndex);

    (*m_aParameterRow)[parameterIndex - 1].setNull();
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setObjectNull(sal_Int32, sal_Int32, const ::rtl::OUString&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setBoolean(sal_Int32, sal_Bool) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setByte(sal_Int32, sal_Int8) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setShort(sal_Int32, sal_Int16) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setInt(sal_Int32, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setLong(sal_Int32, sal_Int64) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setFloat(sal_Int32, float) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setDouble(sal_Int32, double) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setString(sal_Int32 parameterIndex, const ::rtl::OUString &x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    checkAndResizeParameters(parameterIndex);

    (*m_aParameterRow)[parameterIndex - 1] = x;
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setBytes(sal_Int32, const Sequence< sal_Int8 >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setDate(sal_Int32, const Date&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setTime(sal_Int32, const Time&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setTimestamp(sal_Int32, const DateTime&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setBinaryStream(sal_Int32, const Reference< ::com::sun::star::io::XInputStream >&, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setCharacterStream(sal_Int32, const Reference< ::com::sun::star::io::XInputStream >&, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setObject(sal_Int32, const Any&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setObjectWithInfo(sal_Int32, const Any&, sal_Int32, sal_Int32) throw(SQLException, RuntimeException)
{
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setRef(sal_Int32, const Reference< XRef >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setBlob(sal_Int32, const Reference< XBlob >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setClob(sal_Int32, const Reference< XClob >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::setArray(sal_Int32, const Reference< XArray >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL MacabPreparedStatement::clearParameters() throw(SQLException, RuntimeException)
{
::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void MacabPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
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
