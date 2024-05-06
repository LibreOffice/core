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


#include "NPreparedStatement.hxx"
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <rtl/ref.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <strings.hrc>

using namespace connectivity::evoab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OEvoabPreparedStatement,u"com.sun.star.sdbcx.evoab.PreparedStatement"_ustr,u"com.sun.star.sdbc.PreparedStatement"_ustr);


OEvoabPreparedStatement::OEvoabPreparedStatement( OEvoabConnection* _pConnection )
    :OCommonStatement(_pConnection)
{
}


void OEvoabPreparedStatement::construct( const OUString& _sql )
{
    m_sSqlStatement = _sql;

    m_aQueryData = impl_getEBookQuery_throw( m_sSqlStatement );
    ENSURE_OR_THROW( m_aQueryData.getQuery(), "no EBookQuery" );
    ENSURE_OR_THROW( m_aQueryData.xSelectColumns.is(), "no SelectColumn" );

    // create our meta data
    rtl::Reference<OEvoabResultSetMetaData> pMeta
        = new OEvoabResultSetMetaData( m_aQueryData.sTable );
    m_xMetaData = pMeta;
    pMeta->setEvoabFields( m_aQueryData.xSelectColumns );
}


OEvoabPreparedStatement::~OEvoabPreparedStatement()
{
}


void SAL_CALL OEvoabPreparedStatement::acquire() noexcept
{
    OCommonStatement::acquire();
}


void SAL_CALL OEvoabPreparedStatement::release() noexcept
{
    OCommonStatement::release();
}


Any SAL_CALL OEvoabPreparedStatement::queryInterface( const Type & rType )
{
    Any aRet = OCommonStatement::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    return aRet;
}

Sequence< Type > SAL_CALL OEvoabPreparedStatement::getTypes(  )
{
    return ::comphelper::concatSequences(OPreparedStatement_BASE::getTypes(),OCommonStatement::getTypes());
}


Reference< XResultSetMetaData > SAL_CALL OEvoabPreparedStatement::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    // the meta data should have been created at construction time
    ENSURE_OR_THROW( m_xMetaData.is(), "internal error: no meta data" );
    return m_xMetaData;
}


void SAL_CALL OEvoabPreparedStatement::close(  )
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


sal_Bool SAL_CALL OEvoabPreparedStatement::execute(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    Reference< XResultSet> xRS = impl_executeQuery_throw( m_aQueryData );
    return xRS.is();
}


sal_Int32 SAL_CALL OEvoabPreparedStatement::executeUpdate(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedSQLException( u"XStatement::executeUpdate"_ustr, *this );
    return 0;
}


void SAL_CALL OEvoabPreparedStatement::setString( sal_Int32 /*parameterIndex*/, const OUString& /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setString"_ustr, *this );
}


Reference< XConnection > SAL_CALL OEvoabPreparedStatement::getConnection(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    return impl_getConnection();
}


Reference< XResultSet > SAL_CALL OEvoabPreparedStatement::executeQuery(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    return impl_executeQuery_throw( m_aQueryData );
}


void SAL_CALL OEvoabPreparedStatement::setBoolean( sal_Int32 /*parameterIndex*/, sal_Bool /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setBoolean"_ustr, *this );

}

void SAL_CALL OEvoabPreparedStatement::setByte( sal_Int32 /*parameterIndex*/, sal_Int8 /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setByte"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setDate( sal_Int32 /*parameterIndex*/, const Date& /*aData*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setDate"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setTime( sal_Int32 /*parameterIndex*/, const css::util::Time& /*aVal*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setTime"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setTimestamp( sal_Int32 /*parameterIndex*/, const DateTime& /*aVal*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setTimestamp"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setDouble( sal_Int32 /*parameterIndex*/, double /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setDouble"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setFloat( sal_Int32 /*parameterIndex*/, float /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setFloat"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setInt( sal_Int32 /*parameterIndex*/, sal_Int32 /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setInt"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setLong( sal_Int32 /*parameterIndex*/, sal_Int64 /*aVal*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setLong"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setNull( sal_Int32 /*parameterIndex*/, sal_Int32 /*sqlType*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setNull"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setClob( sal_Int32 /*parameterIndex*/, const Reference< XClob >& /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setClob"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setBlob( sal_Int32 /*parameterIndex*/, const Reference< XBlob >& /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setBlob"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setArray"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setRef"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setObjectWithInfo( sal_Int32 /*parameterIndex*/, const Any& /*x*/, sal_Int32 /*sqlType*/, sal_Int32 /*scale*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setObjectWithInfo"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setObjectNull( sal_Int32 /*parameterIndex*/, sal_Int32 /*sqlType*/, const OUString& /*typeName*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setObjectNull"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x )
{
    if(!::dbtools::implSetObject(this,parameterIndex,x))
    {
        const OUString sError( getOwnConnection()->getResources().getResourceStringWithSubstitution(
                STR_UNKNOWN_PARA_TYPE,
                "$position$", OUString::number(parameterIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
}


void SAL_CALL OEvoabPreparedStatement::setShort( sal_Int32 /*parameterIndex*/, sal_Int16 /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setShort"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setBytes( sal_Int32 /*parameterIndex*/, const Sequence< sal_Int8 >& /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setBytes"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setCharacterStream( sal_Int32 /*parameterIndex*/, const Reference< XInputStream >& /*x*/, sal_Int32 /*length*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setCharacterStream"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::setBinaryStream( sal_Int32 /*parameterIndex*/, const Reference< XInputStream >& /*x*/, sal_Int32 /*length*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XParameters::setBinaryStream"_ustr, *this );
}


void SAL_CALL OEvoabPreparedStatement::clearParameters(  )
{
}

Reference< XResultSet > SAL_CALL OEvoabPreparedStatement::getResultSet(  )
{
    return nullptr;
}

sal_Int32 SAL_CALL OEvoabPreparedStatement::getUpdateCount(  )
{
    return 0;
}

sal_Bool SAL_CALL OEvoabPreparedStatement::getMoreResults(  )
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
