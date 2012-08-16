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


#include <stdio.h>
#include <osl/diagnose.h>
#include "NPreparedStatement.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include "propertyids.hxx"
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <tools/diagnose_ex.h>

#include "resource/common_res.hrc"

using namespace connectivity::evoab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OEvoabPreparedStatement,"com.sun.star.sdbcx.evoab.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


OEvoabPreparedStatement::OEvoabPreparedStatement( OEvoabConnection* _pConnection )
    :OCommonStatement(_pConnection)
    ,m_sSqlStatement()
    ,m_xMetaData()
{
}

// -----------------------------------------------------------------------------
void OEvoabPreparedStatement::construct( const ::rtl::OUString& _sql )
{
    m_sSqlStatement = _sql;

    m_aQueryData = impl_getEBookQuery_throw( m_sSqlStatement );
    ENSURE_OR_THROW( m_aQueryData.getQuery(), "no EBookQuery" );
    ENSURE_OR_THROW( m_aQueryData.xSelectColumns.is(), "no SelectColumn" );

    // create our meta data
    OEvoabResultSetMetaData* pMeta = new OEvoabResultSetMetaData( m_aQueryData.sTable );
    m_xMetaData = pMeta;
    pMeta->setEvoabFields( m_aQueryData.xSelectColumns );
}

// -----------------------------------------------------------------------------
OEvoabPreparedStatement::~OEvoabPreparedStatement()
{
}

// -----------------------------------------------------------------------------
void SAL_CALL OEvoabPreparedStatement::acquire() throw()
{
    OCommonStatement::acquire();
}

// -----------------------------------------------------------------------------
void SAL_CALL OEvoabPreparedStatement::release() throw()
{
    OCommonStatement::release();
}

// -----------------------------------------------------------------------------
Any SAL_CALL OEvoabPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OCommonStatement::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OEvoabPreparedStatement::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(OPreparedStatement_BASE::getTypes(),OCommonStatement::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OEvoabPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    // the meta data should have been created at construction time
    ENSURE_OR_THROW( m_xMetaData.is(), "internal error: no meta data" );
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    free_column_resources();
    // Reset last warning message
    try {
        clearWarnings ();
        OCommonStatement::close();
    }
    catch (SQLException &) {
        // If we get an error, ignore
    }

}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    printf("OEvoabPreparedStatement::execute()\n");
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    Reference< XResultSet> xRS = impl_executeQuery_throw( m_aQueryData );
    return xRS.is();
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OEvoabPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XStatement::executeUpdate", *this );
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
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    return impl_getConnection();
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OEvoabPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    return impl_executeQuery_throw( m_aQueryData );
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

void SAL_CALL OEvoabPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if(!::dbtools::implSetObject(this,parameterIndex,x))
    {
        const ::rtl::OUString sError( getOwnConnection()->getResources().getResourceStringWithSubstitution(
                STR_UNKNOWN_PARA_TYPE,
                "$position$", ::rtl::OUString::valueOf(parameterIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
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


void SAL_CALL OEvoabPreparedStatement::setCharacterStream( sal_Int32 /*parameterIndex*/, const Reference< XInputStream >& /*x*/, sal_Int32 /*length*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setCharacterStream", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBinaryStream( sal_Int32 /*parameterIndex*/, const Reference< XInputStream >& /*x*/, sal_Int32 /*length*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XParameters::setBinaryStream", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OEvoabPreparedStatement::getResultSet(  ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabPreparedStatement::getUpdateCount(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabPreparedStatement::getMoreResults(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
