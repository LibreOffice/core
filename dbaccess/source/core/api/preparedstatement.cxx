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


#include <stringconstants.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <connectivity/dbtools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <preparedstatement.hxx>
#include "resultcolumn.hxx"
#include "resultset.hxx"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;


OPreparedStatement::OPreparedStatement(const Reference< XConnection > & _xConn,
                                      const Reference< XInterface > & _xStatement)
                   :OStatementBase(_xConn, _xStatement)
{
    m_xAggregateAsParameters.set( m_xAggregateAsSet, UNO_QUERY_THROW );

    Reference<XDatabaseMetaData> xMeta = _xConn->getMetaData();
    m_pColumns.reset( new OColumns(*this, m_aMutex, xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers(),std::vector< OUString>(), nullptr,nullptr) );
}

OPreparedStatement::~OPreparedStatement()
{
    m_pColumns->acquire();
    m_pColumns->disposing();
}

// css::lang::XTypeProvider
Sequence< Type > OPreparedStatement::getTypes()
{
    OTypeCollection aTypes(cppu::UnoType<XServiceInfo>::get(),
                           cppu::UnoType<XPreparedStatement>::get(),
                           cppu::UnoType<XParameters>::get(),
                           cppu::UnoType<XResultSetMetaDataSupplier>::get(),
                           cppu::UnoType<XColumnsSupplier>::get(),
                            OStatementBase::getTypes() );

    return aTypes.getTypes();
}

Sequence< sal_Int8 > OPreparedStatement::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// css::uno::XInterface
Any OPreparedStatement::queryInterface( const Type & rType )
{
    Any aIface = OStatementBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XServiceInfo * >( this ),
                    static_cast< XParameters * >( this ),
                    static_cast< XColumnsSupplier * >( this ),
                    static_cast< XResultSetMetaDataSupplier * >( this ),
                    static_cast< XPreparedBatchExecution * >( this ),
                    static_cast< XMultipleResults * >( this ),
                    static_cast< XPreparedStatement * >( this ));
    return aIface;
}

void OPreparedStatement::acquire() throw ()
{
    OStatementBase::acquire();
}

void OPreparedStatement::release() throw ()
{
    OStatementBase::release();
}

// XServiceInfo
OUString OPreparedStatement::getImplementationName(  )
{
    return OUString("com.sun.star.sdb.OPreparedStatement");
}

sal_Bool OPreparedStatement::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > OPreparedStatement::getSupportedServiceNames(  )
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = SERVICE_SDBC_PREPAREDSTATEMENT;
    aSNS.getArray()[1] = SERVICE_SDB_PREPAREDSTATMENT;
    return aSNS;
}

// OComponentHelper
void OPreparedStatement::disposing()
{
    {
        MutexGuard aGuard(m_aMutex);
        m_pColumns->disposing();
        m_xAggregateAsParameters = nullptr;
    }
    OStatementBase::disposing();
}

// css::sdbcx::XColumnsSupplier
Reference< css::container::XNameAccess > OPreparedStatement::getColumns()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // do we have to populate the columns
    if (!m_pColumns->isInitialized())
    {
        try
        {
            Reference< XResultSetMetaDataSupplier > xSuppMeta( m_xAggregateAsSet, UNO_QUERY_THROW );
            Reference< XResultSetMetaData > xMetaData( xSuppMeta->getMetaData(), UNO_SET_THROW );

            Reference< XConnection > xConn( getConnection(), UNO_SET_THROW );
            Reference< XDatabaseMetaData > xDBMeta( xConn->getMetaData(), UNO_SET_THROW );

            for (sal_Int32 i = 0, nCount = xMetaData->getColumnCount(); i < nCount; ++i)
            {
                // retrieve the name of the column
                OUString aName = xMetaData->getColumnName(i + 1);
                OResultColumn* pColumn = new OResultColumn(xMetaData, i + 1, xDBMeta);
                // don't silently assume that the name is unique - preparedStatement implementations
                // are allowed to return duplicate names, but we are required to have
                // unique column names
                if ( m_pColumns->hasByName( aName ) )
                    aName = ::dbtools::createUniqueName( m_pColumns.get(), aName );

                m_pColumns->append(aName, pColumn);
            }
        }
        catch (const SQLException& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        m_pColumns->setInitialized();
    }
    return m_pColumns.get();
}

// XResultSetMetaDataSupplier
Reference< XResultSetMetaData > OPreparedStatement::getMetaData()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XResultSetMetaDataSupplier >( m_xAggregateAsSet, UNO_QUERY_THROW )->getMetaData();
}

// XPreparedStatement
Reference< XResultSet >  OPreparedStatement::executeQuery()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    Reference< XResultSet > xResultSet;
    Reference< XResultSet > xDrvResultSet = Reference< XPreparedStatement >( m_xAggregateAsSet, UNO_QUERY_THROW )->executeQuery();
    if (xDrvResultSet.is())
    {
        xResultSet = new OResultSet(xDrvResultSet, *this, m_pColumns->isCaseSensitive());

        // keep the resultset weak
        m_aResultSet = xResultSet;
    }
    return xResultSet;
}

sal_Int32 OPreparedStatement::executeUpdate()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    return Reference< XPreparedStatement >( m_xAggregateAsSet, UNO_QUERY_THROW )->executeUpdate();
}

sal_Bool OPreparedStatement::execute()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    return Reference< XPreparedStatement >( m_xAggregateAsSet, UNO_QUERY_THROW )->execute();
}

Reference< XConnection > OPreparedStatement::getConnection()
{
    return Reference< XConnection > (m_xParent, UNO_QUERY);
}

// XParameters
void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setNull(parameterIndex, sqlType);
}

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setObjectNull(parameterIndex, sqlType, typeName);
}

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setBoolean(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setByte(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setShort(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setInt(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setLong(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setFloat(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setDouble(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const OUString& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setString(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setBytes(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const css::util::Date& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setDate(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const css::util::Time& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setTime(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setTimestamp(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setBinaryStream(parameterIndex, x, length);
}

void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setCharacterStream(parameterIndex, x, length);
}

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setObject(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 scale )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setObjectWithInfo(parameterIndex, x, targetSqlType, scale);
}

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setRef(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setBlob(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setClob(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setArray(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::clearParameters(  )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->clearParameters();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
