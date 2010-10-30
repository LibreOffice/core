/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_dbaccess.hxx"

#include "dbastrings.hrc"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <preparedstatement.hxx>
#include <resultcolumn.hxx>
#include <resultset.hxx>
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

DBG_NAME(OPreparedStatement)

OPreparedStatement::OPreparedStatement(const Reference< XConnection > & _xConn,
                                      const Reference< XInterface > & _xStatement)
                   :OStatementBase(_xConn, _xStatement)
{
    DBG_CTOR(OPreparedStatement, NULL);
    m_xAggregateAsParameters = Reference< XParameters >( m_xAggregateAsSet, UNO_QUERY_THROW );

    Reference<XDatabaseMetaData> xMeta = _xConn->getMetaData();
    m_pColumns = new OColumns(*this, m_aMutex, xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers(),::std::vector< ::rtl::OUString>(), NULL,NULL);
}

OPreparedStatement::~OPreparedStatement()
{
    m_pColumns->acquire();
    m_pColumns->disposing();
    delete m_pColumns;

    DBG_DTOR(OPreparedStatement, NULL);
}

// com::sun::star::lang::XTypeProvider
Sequence< Type > OPreparedStatement::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XServiceInfo > *)0 ),
                           ::getCppuType( (const Reference< XPreparedStatement > *)0 ),
                           ::getCppuType( (const Reference< XParameters > *)0 ),
                           ::getCppuType( (const Reference< XResultSetMetaDataSupplier > *)0 ),
                           ::getCppuType( (const Reference< XColumnsSupplier > *)0 ),
                            OStatementBase::getTypes() );

    return aTypes.getTypes();
}

Sequence< sal_Int8 > OPreparedStatement::getImplementationId() throw (RuntimeException)
{
        static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::uno::XInterface
Any OPreparedStatement::queryInterface( const Type & rType ) throw (RuntimeException)
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
rtl::OUString OPreparedStatement::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.OPreparedStatement"));
}

sal_Bool OPreparedStatement::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

Sequence< ::rtl::OUString > OPreparedStatement::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
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
        m_xAggregateAsParameters = NULL;
    }
    OStatementBase::disposing();
}

// ::com::sun::star::sdbcx::XColumnsSupplier
Reference< ::com::sun::star::container::XNameAccess > OPreparedStatement::getColumns(void) throw( RuntimeException )
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
                rtl::OUString aName = xMetaData->getColumnName(i + 1);
                OResultColumn* pColumn = new OResultColumn(xMetaData, i + 1, xDBMeta);
                m_pColumns->append(aName, pColumn);
            }
        }
        catch (const SQLException& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        m_pColumns->setInitialized();
    }
    return m_pColumns;
}

// XResultSetMetaDataSupplier
Reference< XResultSetMetaData > OPreparedStatement::getMetaData(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XResultSetMetaDataSupplier >( m_xAggregateAsSet, UNO_QUERY_THROW )->getMetaData();
}

// XPreparedStatement
Reference< XResultSet >  OPreparedStatement::executeQuery() throw( SQLException, RuntimeException )
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

sal_Int32 OPreparedStatement::executeUpdate() throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    return Reference< XPreparedStatement >( m_xAggregateAsSet, UNO_QUERY_THROW )->executeUpdate();
}

sal_Bool OPreparedStatement::execute() throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    return Reference< XPreparedStatement >( m_xAggregateAsSet, UNO_QUERY_THROW )->execute();
}

Reference< XConnection > OPreparedStatement::getConnection(void) throw( SQLException, RuntimeException )
{
    return Reference< XConnection > (m_xParent, UNO_QUERY);
}

// XParameters
void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setNull(parameterIndex, sqlType);
}

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setObjectNull(parameterIndex, sqlType, typeName);
}

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setBoolean(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setByte(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setShort(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setInt(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setLong(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setFloat(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setDouble(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setString(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setBytes(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setDate(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setTime(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setTimestamp(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setBinaryStream(parameterIndex, x, length);
}

void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setCharacterStream(parameterIndex, x, length);
}

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setObject(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setObjectWithInfo(parameterIndex, x, targetSqlType, scale);
}

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setRef(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setBlob(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setClob(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->setArray(parameterIndex, x);
}

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    m_xAggregateAsParameters->clearParameters();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
