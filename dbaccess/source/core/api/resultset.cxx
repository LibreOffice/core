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

#include "resultset.hxx"
#include <sal/log.hxx>
#include <stringconstants.hxx>
#include <apitools.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include "datacolumn.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/thread.h>


using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;
using namespace dbtools;


OResultSet::OResultSet(const css::uno::Reference< css::sdbc::XResultSet >& _xResultSet,
                       const css::uno::Reference< css::uno::XInterface >& _xStatement,
                       bool _bCaseSensitive)
           :OResultSetBase(m_aMutex)
           ,OPropertySetHelper(OResultSetBase::rBHelper)
           ,m_xDelegatorResultSet(_xResultSet)
           ,m_aWarnings( Reference< XWarningsSupplier >( _xResultSet, UNO_QUERY ) )
           ,m_nResultSetConcurrency(0)
           ,m_bIsBookmarkable(false)
{
    m_pColumns.reset( new OColumns(*this, m_aMutex, _bCaseSensitive, std::vector< OUString>(), nullptr,nullptr) );

    try
    {
        m_aStatement = _xStatement;
        m_xDelegatorResultSetUpdate.set(m_xDelegatorResultSet, css::uno::UNO_QUERY);
        m_xDelegatorRow.set(m_xDelegatorResultSet, css::uno::UNO_QUERY);
        m_xDelegatorRowUpdate.set(m_xDelegatorResultSet, css::uno::UNO_QUERY);

        Reference< XPropertySet > xSet(m_xDelegatorResultSet, UNO_QUERY);
        sal_Int32 nResultSetType(0);
        xSet->getPropertyValue(PROPERTY_RESULTSETTYPE) >>= nResultSetType;
        xSet->getPropertyValue(PROPERTY_RESULTSETCONCURRENCY) >>= m_nResultSetConcurrency;

        // test for Bookmarks
        if (ResultSetType::FORWARD_ONLY != nResultSetType)
        {
            Reference <XPropertySetInfo > xInfo(xSet->getPropertySetInfo());
            if (xInfo->hasPropertyByName(PROPERTY_ISBOOKMARKABLE))
            {
                m_bIsBookmarkable = ::comphelper::getBOOL(xSet->getPropertyValue(PROPERTY_ISBOOKMARKABLE));
                OSL_ENSURE( !m_bIsBookmarkable || Reference< XRowLocate >(m_xDelegatorResultSet, UNO_QUERY).is(),
                    "OResultSet::OResultSet: aggregate is inconsistent in its bookmarkable attribute!" );
                m_bIsBookmarkable = m_bIsBookmarkable && Reference< XRowLocate >(m_xDelegatorResultSet, UNO_QUERY).is();
            }
        }
    }
    catch (const Exception&)
    {
    }
}

OResultSet::~OResultSet()
{
    m_pColumns->acquire();
    m_pColumns->disposing();
}

// css::lang::XTypeProvider
Sequence< Type > OResultSet::getTypes()
{
    OTypeCollection aTypes(cppu::UnoType<XPropertySet>::get(),
                           OResultSetBase::getTypes());

    return aTypes.getTypes();
}

Sequence< sal_Int8 > OResultSet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// css::uno::XInterface
Any OResultSet::queryInterface( const Type & rType )
{
    Any aIface = OResultSetBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ));

    return aIface;
}

void OResultSet::acquire() throw ()
{
    OResultSetBase::acquire();
}

void OResultSet::release() throw ()
{
    OResultSetBase::release();
}


// OResultSetBase
void OResultSet::disposing()
{
    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    // free the columns
    m_pColumns->disposing();

    // close the pending result set
    Reference< XCloseable > (m_xDelegatorResultSet, UNO_QUERY)->close();

    m_xDelegatorResultSet = nullptr;
    m_xDelegatorRow = nullptr;
    m_xDelegatorRowUpdate = nullptr;

    m_aStatement.clear();
}

// XCloseable
void OResultSet::close()
{
    {
        MutexGuard aGuard( m_aMutex );
        ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);
    }
    dispose();
}

// XServiceInfo
OUString OResultSet::getImplementationName(  )
{
    return OUString("com.sun.star.sdb.OResultSet");
}

sal_Bool OResultSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > OResultSet::getSupportedServiceNames(  )
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBC_RESULTSET;
    aSNS[1] = SERVICE_SDB_RESULTSET;
    return aSNS;
}

// css::beans::XPropertySet
Reference< XPropertySetInfo > OResultSet::getPropertySetInfo()
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(6)
        DECL_PROP1(CURSORNAME,              OUString,    READONLY);
        DECL_PROP0(FETCHDIRECTION,          sal_Int32);
        DECL_PROP0(FETCHSIZE,               sal_Int32);
        DECL_PROP1_BOOL(ISBOOKMARKABLE,         READONLY);
        DECL_PROP1(RESULTSETCONCURRENCY,    sal_Int32,      READONLY);
        DECL_PROP1(RESULTSETTYPE,           sal_Int32,      READONLY);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& OResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool OResultSet::convertFastPropertyValue(Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue )
{
    // be lazy ...
    rConvertedValue = rValue;
    getFastPropertyValue( rOldValue, nHandle );
    return true;
}

void OResultSet::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
{
    // set it for the driver result set
    Reference< XPropertySet > xSet(m_xDelegatorResultSet, UNO_QUERY);
    switch (nHandle)
    {
        case PROPERTY_ID_FETCHDIRECTION:
            xSet->setPropertyValue(PROPERTY_FETCHDIRECTION, rValue);
            break;
        case PROPERTY_ID_FETCHSIZE:
            xSet->setPropertyValue(PROPERTY_FETCHSIZE, rValue);
            break;
        default:
            SAL_WARN("dbaccess", "unknown Property");
    }
}

void OResultSet::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        {
            rValue <<= m_bIsBookmarkable;
        }   break;
        default:
        {
            // get the property name
            OUString aPropName;
            sal_Int16 nAttributes;
            const_cast<OResultSet*>(this)->getInfoHelper().
                fillPropertyMembersByHandle(&aPropName, &nAttributes, nHandle);
            OSL_ENSURE(!aPropName.isEmpty(), "property not found?");

            // now read the value
            rValue = Reference< XPropertySet >(m_xDelegatorResultSet, UNO_QUERY)->getPropertyValue(aPropName);
        }
    }
}

// XWarningsSupplier
Any OResultSet::getWarnings()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);
    return m_aWarnings.getWarnings();
}

void OResultSet::clearWarnings()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);
    m_aWarnings.clearWarnings();
}

// css::sdbc::XResultSetMetaDataSupplier
Reference< XResultSetMetaData > OResultSet::getMetaData()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return Reference< XResultSetMetaDataSupplier >(m_xDelegatorResultSet, UNO_QUERY)->getMetaData();
}

// css::sdbc::XColumnLocate
sal_Int32 OResultSet::findColumn(const OUString& columnName)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return Reference< XColumnLocate >(m_xDelegatorResultSet, UNO_QUERY)->findColumn(columnName);
}

namespace
{
    Reference< XDatabaseMetaData > lcl_getDBMetaDataFromStatement_nothrow( const Reference< XInterface >& _rxStatement )
    {
        Reference< XDatabaseMetaData > xDBMetaData;
        try
        {
            Reference< XStatement > xStatement( _rxStatement, UNO_QUERY );
            Reference< XPreparedStatement > xPreparedStatement( _rxStatement, UNO_QUERY );
            Reference< XConnection > xConn;
            if ( xStatement.is() )
                xConn = xStatement->getConnection();
            else if ( xPreparedStatement.is() )
                xConn = xPreparedStatement->getConnection();
            if ( xConn.is() )
                xDBMetaData = xConn->getMetaData();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        return xDBMetaData;
    }
}

// css::sdbcx::XColumnsSupplier
Reference< css::container::XNameAccess > OResultSet::getColumns()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    // do we have to populate the columns
    if (!m_pColumns->isInitialized())
    {
        // get the metadata
        Reference< XResultSetMetaData > xMetaData = Reference< XResultSetMetaDataSupplier >(m_xDelegatorResultSet, UNO_QUERY)->getMetaData();

        sal_Int32 nColCount = 0;
        // do we have columns
        try
        {
            Reference< XDatabaseMetaData > xDBMetaData( lcl_getDBMetaDataFromStatement_nothrow( getStatement() ) );
            nColCount = xMetaData->getColumnCount();

            for ( sal_Int32 i = 0; i < nColCount; ++i)
            {
                // retrieve the name of the column
                OUString sName = xMetaData->getColumnName(i + 1);
                ODataColumn* pColumn = new ODataColumn(xMetaData, m_xDelegatorRow, m_xDelegatorRowUpdate, i + 1, xDBMetaData);

                // don't silently assume that the name is unique - result set implementations
                // are allowed to return duplicate names, but we are required to have
                // unique column names
                if ( m_pColumns->hasByName( sName ) )
                    sName = ::dbtools::createUniqueName( m_pColumns.get(), sName );

                m_pColumns->append( sName, pColumn );
            }
        }
        catch ( const SQLException& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        m_pColumns->setInitialized();

    #if OSL_DEBUG_LEVEL > 0
        // some sanity checks. Especially in case we auto-adjusted the column names above,
        // this might be reasonable
        try
        {
            const Reference< XNameAccess > xColNames( static_cast< XNameAccess* >( m_pColumns.get() ), UNO_SET_THROW );
            const Sequence< OUString > aNames( xColNames->getElementNames() );
            SAL_WARN_IF( aNames.getLength() != nColCount, "dbaccess",
                "OResultSet::getColumns: invalid column count!" );
            for (  auto const & name : aNames )
            {
                Reference< XPropertySet > xColProps( xColNames->getByName( name ), UNO_QUERY_THROW );
                OUString sName;
                OSL_VERIFY( xColProps->getPropertyValue( PROPERTY_NAME ) >>= sName );
                SAL_WARN_IF( sName != name, "dbaccess", "OResultSet::getColumns: invalid column name!" );
            }

        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    #endif
    }
    return m_pColumns.get();
}

// css::sdbc::XRow
sal_Bool OResultSet::wasNull()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->wasNull();
}

OUString OResultSet::getString(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getString(columnIndex);
}

sal_Bool OResultSet::getBoolean(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getBoolean(columnIndex);
}

sal_Int8 OResultSet::getByte(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getByte(columnIndex);
}

sal_Int16 OResultSet::getShort(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getShort(columnIndex);
}

sal_Int32 OResultSet::getInt(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getInt(columnIndex);
}

sal_Int64 OResultSet::getLong(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getLong(columnIndex);
}

float OResultSet::getFloat(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getFloat(columnIndex);
}

double OResultSet::getDouble(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getDouble(columnIndex);
}

Sequence< sal_Int8 > OResultSet::getBytes(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getBytes(columnIndex);
}

css::util::Date OResultSet::getDate(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getDate(columnIndex);
}

css::util::Time OResultSet::getTime(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getTime(columnIndex);
}

css::util::DateTime OResultSet::getTimestamp(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getTimestamp(columnIndex);
}

Reference< css::io::XInputStream >  OResultSet::getBinaryStream(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getBinaryStream(columnIndex);
}

Reference< css::io::XInputStream >  OResultSet::getCharacterStream(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getCharacterStream(columnIndex);
}

Any OResultSet::getObject(sal_Int32 columnIndex, const Reference< css::container::XNameAccess > & typeMap)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getObject(columnIndex, typeMap);
}

Reference< XRef >  OResultSet::getRef(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getRef(columnIndex);
}

Reference< XBlob >  OResultSet::getBlob(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getBlob(columnIndex);
}

Reference< XClob >  OResultSet::getClob(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getClob(columnIndex);
}

Reference< XArray >  OResultSet::getArray(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorRow->getArray(columnIndex);
}

// css::sdbc::XRowUpdate
void OResultSet::updateNull(sal_Int32 columnIndex)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateNull(columnIndex);
}

void OResultSet::updateBoolean(sal_Int32 columnIndex, sal_Bool x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateBoolean(columnIndex, x);
}

void OResultSet::updateByte(sal_Int32 columnIndex, sal_Int8 x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateByte(columnIndex, x);
}

void OResultSet::updateShort(sal_Int32 columnIndex, sal_Int16 x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateShort(columnIndex, x);
}

void OResultSet::updateInt(sal_Int32 columnIndex, sal_Int32 x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateInt(columnIndex, x);
}

void OResultSet::updateLong(sal_Int32 columnIndex, sal_Int64 x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateLong(columnIndex, x);
}

void OResultSet::updateFloat(sal_Int32 columnIndex, float x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateFloat(columnIndex, x);
}

void OResultSet::updateDouble(sal_Int32 columnIndex, double x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateDouble(columnIndex, x);
}

void OResultSet::updateString(sal_Int32 columnIndex, const OUString& x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateString(columnIndex, x);
}

void OResultSet::updateBytes(sal_Int32 columnIndex, const Sequence< sal_Int8 >& x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateBytes(columnIndex, x);
}

void OResultSet::updateDate(sal_Int32 columnIndex, const css::util::Date& x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateDate(columnIndex, x);
}

void OResultSet::updateTime(sal_Int32 columnIndex, const css::util::Time& x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateTime(columnIndex, x);
}

void OResultSet::updateTimestamp(sal_Int32 columnIndex, const css::util::DateTime& x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateTimestamp(columnIndex, x);
}

void OResultSet::updateBinaryStream(sal_Int32 columnIndex, const Reference< css::io::XInputStream > & x, sal_Int32 length)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateBinaryStream(columnIndex, x, length);
}

void OResultSet::updateCharacterStream(sal_Int32 columnIndex, const Reference< css::io::XInputStream > & x, sal_Int32 length)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateCharacterStream(columnIndex, x, length);
}

void OResultSet::updateNumericObject(sal_Int32 columnIndex, const Any& x, sal_Int32 scale)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateNumericObject(columnIndex, x, scale);
}

void OResultSet::updateObject(sal_Int32 columnIndex, const Any& x)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorRowUpdate->updateObject(columnIndex, x);
}

// css::sdbc::XResultSet
sal_Bool OResultSet::next()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->next();
}

sal_Bool OResultSet::isBeforeFirst()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->isBeforeFirst();
}

sal_Bool OResultSet::isAfterLast()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->isAfterLast();
}

sal_Bool OResultSet::isFirst()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->isFirst();
}

sal_Bool OResultSet::isLast()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->isLast();
}

void OResultSet::beforeFirst()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    m_xDelegatorResultSet->beforeFirst();
}

void OResultSet::afterLast()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    m_xDelegatorResultSet->afterLast();
}

sal_Bool OResultSet::first()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->first();
}

sal_Bool OResultSet::last()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->last();
}

sal_Int32 OResultSet::getRow()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->getRow();
}

sal_Bool OResultSet::absolute(sal_Int32 row)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->absolute(row);
}

sal_Bool OResultSet::relative(sal_Int32 rows)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->relative(rows);
}

sal_Bool OResultSet::previous()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->previous();
}

void OResultSet::refreshRow()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    m_xDelegatorResultSet->refreshRow();
}

sal_Bool OResultSet::rowUpdated()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->rowUpdated();
}

sal_Bool OResultSet::rowInserted()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->rowInserted();
}

sal_Bool OResultSet::rowDeleted()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_xDelegatorResultSet->rowDeleted();
}

Reference< XInterface > OResultSet::getStatement()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    return m_aStatement;
}

// css::sdbcx::XRowLocate
Any OResultSet::getBookmark()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkBookmarkable();

    return Reference< XRowLocate >(m_xDelegatorResultSet, UNO_QUERY)->getBookmark();
}

sal_Bool OResultSet::moveToBookmark(const Any& bookmark)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkBookmarkable();

    return Reference< XRowLocate >(m_xDelegatorResultSet, UNO_QUERY)->moveToBookmark(bookmark);
}

sal_Bool OResultSet::moveRelativeToBookmark(const Any& bookmark, sal_Int32 rows)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkBookmarkable();

    return Reference< XRowLocate >(m_xDelegatorResultSet, UNO_QUERY)->moveRelativeToBookmark(bookmark, rows);
}

sal_Int32 OResultSet::compareBookmarks(const Any& _first, const Any& _second)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkBookmarkable();

    return Reference< XRowLocate >(m_xDelegatorResultSet, UNO_QUERY)->compareBookmarks(_first, _second);
}

sal_Bool OResultSet::hasOrderedBookmarks()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkBookmarkable();

    return Reference< XRowLocate >(m_xDelegatorResultSet, UNO_QUERY)->hasOrderedBookmarks();
}

sal_Int32 OResultSet::hashBookmark(const Any& bookmark)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkBookmarkable();

    return Reference< XRowLocate >(m_xDelegatorResultSet, UNO_QUERY)->hashBookmark(bookmark);
}

// css::sdbc::XResultSetUpdate
void OResultSet::insertRow()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorResultSetUpdate->insertRow();
}

void OResultSet::updateRow()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorResultSetUpdate->updateRow();
}

void OResultSet::deleteRow()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorResultSetUpdate->deleteRow();
}

void OResultSet::cancelRowUpdates()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorResultSetUpdate->cancelRowUpdates();
}

void OResultSet::moveToInsertRow()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorResultSetUpdate->moveToInsertRow();
}

void OResultSet::moveToCurrentRow()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OResultSetBase::rBHelper.bDisposed);

    checkReadOnly();

    m_xDelegatorResultSetUpdate->moveToCurrentRow();
}

void OResultSet::checkReadOnly() const
{
    if  (   ( m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY )
        ||  !m_xDelegatorResultSetUpdate.is()
        )
        throwSQLException( "The result set is read-only.", StandardSQLState::GENERAL_ERROR, *const_cast< OResultSet* >( this ) );
}

void OResultSet::checkBookmarkable() const
{
    if ( !m_bIsBookmarkable )
        throwSQLException( "The result set does not have bookmark support.", StandardSQLState::GENERAL_ERROR, *const_cast< OResultSet* >( this ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
