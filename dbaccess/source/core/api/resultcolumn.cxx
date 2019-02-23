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

#include "resultcolumn.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <stringconstants.hxx>
#include <apitools.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/thread.h>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace dbaccess;


OResultColumn::OResultColumn( const Reference < XResultSetMetaData >& _xMetaData, sal_Int32 _nPos,
        const Reference< XDatabaseMetaData >& _rxDBMeta )
    :OColumn( true )
    ,m_xMetaData( _xMetaData )
    ,m_xDBMetaData( _rxDBMeta )
    ,m_nPos( _nPos )
{
}

void OResultColumn::impl_determineIsRowVersion_nothrow()
{
    if ( m_aIsRowVersion.hasValue() )
        return;
    m_aIsRowVersion <<= false;

    OSL_ENSURE( m_xDBMetaData.is(), "OResultColumn::impl_determineIsRowVersion_nothrow: no DBMetaData!" );
    if ( !m_xDBMetaData.is() )
        return;

    try
    {
        OUString sCatalog, sSchema, sTable, sColumnName;
        getPropertyValue( PROPERTY_CATALOGNAME ) >>= sCatalog;
        getPropertyValue( PROPERTY_SCHEMANAME ) >>= sSchema;
        getPropertyValue( PROPERTY_TABLENAME ) >>= sTable;
        getPropertyValue( PROPERTY_NAME ) >>= sColumnName;

        try
        {
            Reference< XResultSet > xVersionColumns = m_xDBMetaData->getVersionColumns(
                makeAny( sCatalog ), sSchema, sTable );
            if ( xVersionColumns.is() ) // allowed to be NULL
            {
                Reference< XRow > xResultRow( xVersionColumns, UNO_QUERY_THROW );
                while ( xVersionColumns->next() )
                {
                    if ( xResultRow->getString( 2 ) == sColumnName )
                    {
                        m_aIsRowVersion <<= true;
                        break;
                    }
                }
            }
        }
        catch(const SQLException&)
        {
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

OResultColumn::~OResultColumn()
{
}

// css::lang::XTypeProvider
Sequence< sal_Int8 > OResultColumn::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
OUString OResultColumn::getImplementationName(  )
{
    return OUString("com.sun.star.sdb.OResultColumn");
}

Sequence< OUString > OResultColumn::getSupportedServiceNames(  )
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    aSNS[1] = SERVICE_SDB_RESULTCOLUMN;
    return aSNS;
}

// OComponentHelper
void OResultColumn::disposing()
{
    OColumn::disposing();

    MutexGuard aGuard(m_aMutex);
    m_xMetaData = nullptr;
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* OResultColumn::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(21)
        DECL_PROP1(CATALOGNAME,             OUString,    READONLY);
        DECL_PROP1(DISPLAYSIZE,             sal_Int32,          READONLY);
        DECL_PROP1_BOOL(ISAUTOINCREMENT,                        READONLY);
        DECL_PROP1_BOOL(ISCASESENSITIVE,                        READONLY);
        DECL_PROP1_BOOL(ISCURRENCY,                             READONLY);
        DECL_PROP1_BOOL(ISDEFINITELYWRITABLE,                   READONLY);
        DECL_PROP1(ISNULLABLE,              sal_Int32,          READONLY);
        DECL_PROP1_BOOL(ISREADONLY,                             READONLY);
        DECL_PROP1_BOOL(ISROWVERSION,                           READONLY);
        DECL_PROP1_BOOL(ISSEARCHABLE,                           READONLY);
        DECL_PROP1_BOOL(ISSIGNED,                               READONLY);
        DECL_PROP1_BOOL(ISWRITABLE,                             READONLY);
        DECL_PROP1(LABEL,                   OUString,    READONLY);
        DECL_PROP1(NAME,                    OUString,    READONLY);
        DECL_PROP1(PRECISION,               sal_Int32,          READONLY);
        DECL_PROP1(SCALE,                   sal_Int32,          READONLY);
        DECL_PROP1(SCHEMANAME,              OUString,    READONLY);
        DECL_PROP1(SERVICENAME,             OUString,    READONLY);
        DECL_PROP1(TABLENAME,               OUString,    READONLY);
        DECL_PROP1(TYPE,                    sal_Int32,          READONLY);
        DECL_PROP1(TYPENAME,                OUString,    READONLY);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& OResultColumn::getInfoHelper()
{
    return *static_cast< ::comphelper::OPropertyArrayUsageHelper< OResultColumn >* >(this)->getArrayHelper();
}

namespace
{
    template< typename T >
    void obtain( Any& _out_rValue, ::boost::optional< T > & _rCache, const sal_Int32 _nPos, const Reference < XResultSetMetaData >& _rxResultMeta, T (SAL_CALL XResultSetMetaData::*Getter)( sal_Int32 ) )
    {
        if ( !_rCache )
            _rCache = (_rxResultMeta.get()->*Getter)(_nPos);
        _out_rValue <<= *_rCache;
    }
}

void OResultColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    try
    {
        if ( OColumn::isRegisteredProperty( nHandle ) )
        {
            OColumn::getFastPropertyValue( rValue, nHandle );
        }
        else
        {
            switch (nHandle)
            {
                case PROPERTY_ID_ISROWVERSION:
                    const_cast< OResultColumn* >( this )->impl_determineIsRowVersion_nothrow();
                    rValue = m_aIsRowVersion;
                    break;
                case PROPERTY_ID_TABLENAME:
                    rValue <<= m_xMetaData->getTableName(m_nPos);
                    break;
                case PROPERTY_ID_SCHEMANAME:
                    rValue <<= m_xMetaData->getSchemaName(m_nPos);
                    break;
                case PROPERTY_ID_CATALOGNAME:
                    rValue <<= m_xMetaData->getCatalogName(m_nPos);
                    break;
                case PROPERTY_ID_ISSIGNED:
                    obtain( rValue, m_isSigned, m_nPos, m_xMetaData, &XResultSetMetaData::isSigned );
                    break;
                case PROPERTY_ID_ISCURRENCY:
                    obtain( rValue, m_isCurrency, m_nPos, m_xMetaData, &XResultSetMetaData::isCurrency );
                    break;
                case PROPERTY_ID_ISSEARCHABLE:
                    obtain( rValue, m_bSearchable, m_nPos, m_xMetaData, &XResultSetMetaData::isSearchable );
                    break;
                case PROPERTY_ID_ISCASESENSITIVE:
                    obtain( rValue, m_isCaseSensitive, m_nPos, m_xMetaData, &XResultSetMetaData::isCaseSensitive );
                    break;
                case PROPERTY_ID_ISREADONLY:
                    obtain( rValue, m_isReadOnly, m_nPos, m_xMetaData, &XResultSetMetaData::isReadOnly );
                    break;
                case PROPERTY_ID_ISWRITABLE:
                    obtain( rValue, m_isWritable, m_nPos, m_xMetaData, &XResultSetMetaData::isWritable );
                    break;
                case PROPERTY_ID_ISDEFINITELYWRITABLE:
                    obtain( rValue, m_isDefinitelyWritable, m_nPos, m_xMetaData, &XResultSetMetaData::isDefinitelyWritable );
                    break;
                case PROPERTY_ID_ISAUTOINCREMENT:
                    obtain( rValue, m_isAutoIncrement, m_nPos, m_xMetaData, &XResultSetMetaData::isAutoIncrement );
                    break;
                case PROPERTY_ID_SERVICENAME:
                    rValue <<= m_xMetaData->getColumnServiceName(m_nPos);
                    break;
                case PROPERTY_ID_LABEL:
                    obtain( rValue, m_sColumnLabel, m_nPos, m_xMetaData, &XResultSetMetaData::getColumnLabel );
                    break;
                case PROPERTY_ID_DISPLAYSIZE:
                    obtain( rValue, m_nColumnDisplaySize, m_nPos, m_xMetaData, &XResultSetMetaData::getColumnDisplaySize );
                    break;
                case PROPERTY_ID_TYPE:
                    obtain( rValue, m_nColumnType, m_nPos, m_xMetaData, &XResultSetMetaData::getColumnType );
                    break;
                case PROPERTY_ID_PRECISION:
                    obtain( rValue, m_nPrecision, m_nPos, m_xMetaData, &XResultSetMetaData::getPrecision );
                    break;
                case PROPERTY_ID_SCALE:
                    obtain( rValue, m_nScale, m_nPos, m_xMetaData, &XResultSetMetaData::getScale );
                    break;
                case PROPERTY_ID_ISNULLABLE:
                    obtain( rValue, m_isNullable, m_nPos, m_xMetaData, &XResultSetMetaData::isNullable );
                    break;
                case PROPERTY_ID_TYPENAME:
                    rValue <<= m_xMetaData->getColumnTypeName(m_nPos);
                    break;
                default:
                    OSL_FAIL( "OResultColumn::getFastPropertyValue: unknown property handle!" );
                    break;
            }
        }
    }
    catch (SQLException& )
    {
        // default handling if we caught an exception
        switch (nHandle)
        {
            case PROPERTY_ID_LABEL:
            case PROPERTY_ID_TYPENAME:
            case PROPERTY_ID_SERVICENAME:
            case PROPERTY_ID_TABLENAME:
            case PROPERTY_ID_SCHEMANAME:
            case PROPERTY_ID_CATALOGNAME:
                // empty string'S
                rValue <<= OUString();
                break;
            case PROPERTY_ID_ISROWVERSION:
            case PROPERTY_ID_ISAUTOINCREMENT:
            case PROPERTY_ID_ISWRITABLE:
            case PROPERTY_ID_ISDEFINITELYWRITABLE:
            case PROPERTY_ID_ISCASESENSITIVE:
            case PROPERTY_ID_ISSEARCHABLE:
            case PROPERTY_ID_ISCURRENCY:
            case PROPERTY_ID_ISSIGNED:
            {
                rValue <<= false;
            }   break;
            case PROPERTY_ID_ISREADONLY:
            {
                rValue <<= true;
            }   break;
            case PROPERTY_ID_SCALE:
            case PROPERTY_ID_PRECISION:
            case PROPERTY_ID_DISPLAYSIZE:
                rValue <<= sal_Int32(0);
                break;
            case PROPERTY_ID_TYPE:
                rValue <<= sal_Int32(DataType::SQLNULL);
                break;
            case PROPERTY_ID_ISNULLABLE:
                rValue <<= ColumnValue::NULLABLE_UNKNOWN;
                break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
