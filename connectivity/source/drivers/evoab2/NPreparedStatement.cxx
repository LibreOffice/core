 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: NPreparedStatement.cxx,v $
 * $Revision: 1.7 $
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

#include <stdio.h>
#include <osl/diagnose.h>
#include "NPreparedStatement.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include "propertyids.hxx"
#include <connectivity/dbexception.hxx>

using namespace connectivity::evoab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OEvoabPreparedStatement,"com.sun.star.sdbcx.evoab.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


OEvoabPreparedStatement::OEvoabPreparedStatement( OEvoabConnection* _pConnection, const ::rtl::OUString& sql)
    :OStatement_BASE2(_pConnection)
    ,m_nNumParams(0)
    ,m_sSqlStatement(sql)
    ,m_bPrepared(sal_False)
{
}
// -----------------------------------------------------------------------------
OEvoabPreparedStatement::~OEvoabPreparedStatement()
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OEvoabPreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OEvoabPreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
Any SAL_CALL OEvoabPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OEvoabPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::comphelper::concatSequences(OPreparedStatement_BASE::getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OEvoabPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    if(!m_xMetaData.is())
        m_xMetaData = new OEvoabResultSetMetaData(m_pConnection->getCurrentTableName());
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    free_column_resources();
    // Reset last warning message
    try {
        clearWarnings ();
        OStatement_BASE2::close();
    }
    catch (SQLException &) {
        // If we get an error, ignore
    }

}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet> xRS = OStatement_Base::executeQuery( m_sSqlStatement );
    // same as in statement with the difference that this statement also can contain parameter

    return xRS.is();
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OEvoabPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // same as in statement with the difference that this statement also can contain parameter
    return 0;
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setString( sal_Int32 /*parameterIndex*/, const ::rtl::OUString& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setString", *this );
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OEvoabPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OEvoabPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > rs = OStatement_Base::executeQuery( m_sSqlStatement );
    return rs;
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBoolean( sal_Int32 /*parameterIndex*/, sal_Bool /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setBoolean", *this );

}
// -------------------------------------------------------------------------
void SAL_CALL OEvoabPreparedStatement::setByte( sal_Int32 /*parameterIndex*/, sal_Int8 /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setByte", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setDate( sal_Int32 /*parameterIndex*/, const Date& /*aData*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setDate", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setTime( sal_Int32 /*parameterIndex*/, const Time& /*aVal*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setTime", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setTimestamp( sal_Int32 /*parameterIndex*/, const DateTime& /*aVal*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setTimestamp", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setDouble( sal_Int32 /*parameterIndex*/, double /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setDouble", *this );
}

// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setFloat( sal_Int32 /*parameterIndex*/, float /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setFloat", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setInt( sal_Int32 /*parameterIndex*/, sal_Int32 /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setInt", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setLong( sal_Int32 /*parameterIndex*/, sal_Int64 /*aVal*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setLong", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setNull( sal_Int32 /*parameterIndex*/, sal_Int32 /*sqlType*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setNull", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setClob( sal_Int32 /*parameterIndex*/, const Reference< XClob >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setClob", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBlob( sal_Int32 /*parameterIndex*/, const Reference< XBlob >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setBlob", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setArray", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setRef", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setObjectWithInfo( sal_Int32 /*parameterIndex*/, const Any& /*x*/, sal_Int32 /*sqlType*/, sal_Int32 /*scale*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setObjectWithInfo", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setObjectNull( sal_Int32 /*parameterIndex*/, sal_Int32 /*sqlType*/, const ::rtl::OUString& /*typeName*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setObjectNull", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setObject( sal_Int32 /*parameterIndex*/, const Any& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setObject", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setShort( sal_Int32 /*parameterIndex*/, sal_Int16 /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setShort", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBytes( sal_Int32 /*parameterIndex*/, const Sequence< sal_Int8 >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setBytes", *this );
}
// -------------------------------------------------------------------------


void SAL_CALL OEvoabPreparedStatement::setCharacterStream( sal_Int32 /*parameterIndex*/, const Reference< ::com::sun::star::io::XInputStream >& /*x*/, sal_Int32 /*length*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setCharacterStream", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBinaryStream( sal_Int32 /*parameterIndex*/, const Reference< ::com::sun::star::io::XInputStream >& /*x*/, sal_Int32 /*length*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setBinaryStream", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void OEvoabPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    switch(nHandle)
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
            OStatement_Base::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL OEvoabPreparedStatement::getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabPreparedStatement::getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabPreparedStatement::getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}
// -----------------------------------------------------------------------------
