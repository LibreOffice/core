/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: KPreparedStatement.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-12-19 16:50:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "KPreparedStatement.hxx"

#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace connectivity::kab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(KabPreparedStatement, "com.sun.star.sdbc.drivers.KabPreparedStatement", "com.sun.star.sdbc.PreparedStatement");
// -------------------------------------------------------------------------
void KabPreparedStatement::checkParameterIndex(sal_Int32 _parameterIndex)
{
    // no parameters allowed in this implementation
    throw SQLException();
}
// -------------------------------------------------------------------------
void KabPreparedStatement::setKabFields() const throw(SQLException)
{
    ::vos::ORef<connectivity::OSQLColumns> xColumns;    // selected columns
    KabResultSetMetaData *pMeta;                // meta information - holds the list of KAddressBook fields

    xColumns = m_aSQLIterator.getSelectColumns();
    if (!xColumns.isValid())
    {
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii("Invalid selection of columns"),
            NULL);
    }
    m_xMetaData->setKabFields(xColumns);
}
// -------------------------------------------------------------------------
KabPreparedStatement::KabPreparedStatement(
    KabConnection* _pConnection,
    const ::rtl::OUString& sql)
    : KabPreparedStatement_BASE(_pConnection),
      m_bPrepared(sal_False),
      m_sSqlStatement(sql)
{
}
// -------------------------------------------------------------------------
KabPreparedStatement::~KabPreparedStatement()
{
}
// -------------------------------------------------------------------------
Reference< XResultSetMetaData > SAL_CALL KabPreparedStatement::getMetaData() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    if (!m_xMetaData.is())
    {
        m_xMetaData = new KabResultSetMetaData(getOwnConnection());
        setKabFields();
    }
    Reference< XResultSetMetaData > xMetaData = m_xMetaData.get();
    return xMetaData;
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::close() throw(SQLException, RuntimeException)
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabPreparedStatement::execute() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet> xRS = KabCommonStatement::executeQuery(m_sSqlStatement);

    return xRS.is();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabPreparedStatement::executeUpdate() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    // same as in statement with the difference that this statement also can contain parameter
    return 0;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL KabPreparedStatement::getConnection() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >) m_pConnection;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabPreparedStatement::executeQuery() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > rs = KabCommonStatement::executeQuery(m_sSqlStatement);

    return rs;
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setByte(sal_Int32 parameterIndex, sal_Int8 x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setShort(sal_Int32 parameterIndex, sal_Int16 x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setInt(sal_Int32 parameterIndex, sal_Int32 x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setLong(sal_Int32 parameterIndex, sal_Int64 aVal) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setFloat(sal_Int32 parameterIndex, float x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setDouble(sal_Int32 parameterIndex, double x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setString(sal_Int32 parameterIndex, const ::rtl::OUString& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setBytes(sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setDate(sal_Int32 parameterIndex, const Date& aData) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setTime(sal_Int32 parameterIndex, const Time& aVal) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setTimestamp(sal_Int32 parameterIndex, const DateTime& aVal) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setBinaryStream(sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setCharacterStream(sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setObject(sal_Int32 parameterIndex, const Any& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setObjectWithInfo(sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale) throw(SQLException, RuntimeException)
{
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setRef(sal_Int32 parameterIndex, const Reference< XRef >& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setBlob(sal_Int32 parameterIndex, const Reference< XBlob >& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setClob(sal_Int32 parameterIndex, const Reference< XClob >& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::setArray(sal_Int32 parameterIndex, const Reference< XArray >& x) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void SAL_CALL KabPreparedStatement::clearParameters() throw(SQLException, RuntimeException)
{
::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("Not Implemented"), NULL);
}
// -------------------------------------------------------------------------
void KabPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
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
