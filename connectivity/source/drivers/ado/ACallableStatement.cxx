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

#include "ado/ACallableStatement.hxx"
#include <connectivity/dbexception.hxx>

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

IMPLEMENT_SERVICE_INFO(OCallableStatement,"com.sun.star.sdbcx.ACallableStatement","com.sun.star.sdbc.CallableStatement");

#define GET_PARAM()                                                 \
    ADOParameter* pParam = NULL;                                    \
    m_pParameters->get_Item(OLEVariant(sal_Int32(columnIndex-1)),&pParam);      \
    if(pParam)                                                      \
        pParam->get_Value(&m_aValue);

//************ Class: java.sql.CallableStatement

OCallableStatement::OCallableStatement( OConnection* _pConnection,const OTypeInfoMap& _TypeInfo,const OUString& sql )
                : OPreparedStatement( _pConnection, _TypeInfo, sql )
{
    m_Command.put_CommandType(adCmdStoredProc);
}


Any SAL_CALL OCallableStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPreparedStatement::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(rType,static_cast< XRow*>(this));
}



sal_Bool SAL_CALL OCallableStatement::wasNull(  ) throw(SQLException, RuntimeException)
{
    return m_aValue.isNull();
}


sal_Bool SAL_CALL OCallableStatement::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}

sal_Int8 SAL_CALL OCallableStatement::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}

Sequence< sal_Int8 > SAL_CALL OCallableStatement::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}

::com::sun::star::util::Date SAL_CALL OCallableStatement::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}

double SAL_CALL OCallableStatement::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}


float SAL_CALL OCallableStatement::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}


sal_Int32 SAL_CALL OCallableStatement::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}


sal_Int64 SAL_CALL OCallableStatement::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return (sal_Int64)m_aValue.getCurrency().int64;
}


Any SAL_CALL OCallableStatement::getObject( sal_Int32 /*columnIndex*/, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getObject", *this );
    return Any();
}


sal_Int16 SAL_CALL OCallableStatement::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}


OUString SAL_CALL OCallableStatement::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}


 ::com::sun::star::util::Time SAL_CALL OCallableStatement::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}


 ::com::sun::star::util::DateTime SAL_CALL OCallableStatement::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}


void SAL_CALL OCallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& /*typeName*/ ) throw(SQLException, RuntimeException)
{
    ADOParameter* pParam = NULL;
    m_pParameters->get_Item(OLEVariant(sal_Int32(parameterIndex-1)),&pParam);
    if(pParam)
    {
        pParam->put_Type(ADOS::MapJdbc2ADOType(sqlType,m_pConnection->getEngineType()));
        pParam->put_Direction(adParamOutput);
    }
}

void SAL_CALL OCallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ADOParameter* pParam = NULL;
    m_pParameters->get_Item(OLEVariant(sal_Int32(parameterIndex-1)),&pParam);
    if(pParam)
    {
        pParam->put_Type(ADOS::MapJdbc2ADOType(sqlType,m_pConnection->getEngineType()));
        pParam->put_Direction(adParamOutput);
        pParam->put_NumericScale((sal_Int8)scale);
    }
}



Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getBinaryStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getBinaryStream", *this );
    return NULL;
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getCharacterStream", *this );
    return NULL;
}


Reference< XArray > SAL_CALL OCallableStatement::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getArray", *this );
    return NULL;
}


Reference< XClob > SAL_CALL OCallableStatement::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getClob", *this );
    return NULL;
}

Reference< XBlob > SAL_CALL OCallableStatement::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getBlob", *this );
    return NULL;
}


Reference< XRef > SAL_CALL OCallableStatement::getRef( sal_Int32 /*columnIndex*/) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRef", *this );
    return NULL;
}


void SAL_CALL OCallableStatement::acquire() throw()
{
    OPreparedStatement::acquire();
}

void SAL_CALL OCallableStatement::release() throw()
{
    OPreparedStatement::release();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
