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


#include <connectivity/PColumn.hxx>
#include <connectivity/dbtools.hxx>
#include <TConnection.hxx>

#include <comphelper/types.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::comphelper;
using namespace connectivity;
using namespace dbtools;
using namespace connectivity::parse;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;


OParseColumn::OParseColumn(const Reference<XPropertySet>& _xColumn, bool _bCase)
    : connectivity::sdbcx::OColumn( getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)))
                                ,   getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)))
                                ,   getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE)))
                                ,   getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION)))
                                ,   getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)))
                                ,   getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)))
                                ,   getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)))
                                ,   getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)))
                                ,   getBOOL(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)))
                                ,   false
                                ,   getBOOL(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)))
                                ,   _bCase
                                ,   getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME)))
                                ,   getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME)))
                                ,   getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TABLENAME)))
                                )
    , m_bFunction(false)
    , m_bDbasePrecisionChanged(false)
    , m_bAggregateFunction(false)
    , m_bIsSearchable( true )
{
    construct();
}


OParseColumn::OParseColumn( const OUString& Name,
                    const OUString& TypeName,
                    const OUString& DefaultValue,
                    const OUString& Description,
                    sal_Int32       IsNullable,
                    sal_Int32       Precision,
                    sal_Int32       Scale,
                    sal_Int32       Type,
                    bool            IsAutoIncrement,
                    bool            IsCurrency,
                    bool            _bCase,
                    const OUString& CatalogName,
                    const OUString& SchemaName,
                    const OUString& TableName
                ) : connectivity::sdbcx::OColumn(Name,
                                  TypeName,
                                  DefaultValue,
                                  Description,
                                  IsNullable,
                                  Precision,
                                  Scale,
                                  Type,
                                  IsAutoIncrement,
                                  false,
                                  IsCurrency,
                                  _bCase,
                                  CatalogName,
                                  SchemaName,
                                  TableName)
    , m_bFunction(false)
    , m_bDbasePrecisionChanged(false)
    , m_bAggregateFunction(false)
    , m_bIsSearchable( true )
{
    construct();
}


::rtl::Reference< OSQLColumns > OParseColumn::createColumnsForResultSet( const Reference< XResultSetMetaData >& _rxResMetaData,
    const Reference< XDatabaseMetaData >& _rxDBMetaData,const Reference< XNameAccess>& i_xQueryColumns )
{
    sal_Int32 nColumnCount = _rxResMetaData->getColumnCount();
    ::rtl::Reference aReturn( new OSQLColumns ); aReturn->get().reserve( nColumnCount );

    StringMap aColumnMap;
    for ( sal_Int32 i = 1; i <= nColumnCount; ++i )
    {
        OParseColumn* pColumn = createColumnForResultSet( _rxResMetaData, _rxDBMetaData, i,aColumnMap );
        aReturn->get().push_back( pColumn );
        if ( i_xQueryColumns.is() && i_xQueryColumns->hasByName(pColumn->getRealName()) )
        {
            Reference<XPropertySet> xColumn(i_xQueryColumns->getByName(pColumn->getRealName()),UNO_QUERY_THROW);
            OUString sLabel;
            xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_LABEL)) >>= sLabel;
            if ( !sLabel.isEmpty() )
                pColumn->setLabel(sLabel);
        }
    }

    return aReturn;
}


OParseColumn* OParseColumn::createColumnForResultSet( const Reference< XResultSetMetaData >& _rxResMetaData,
    const Reference< XDatabaseMetaData >& _rxDBMetaData, sal_Int32 _nColumnPos, StringMap& _rColumns )
{
    OUString sLabel = _rxResMetaData->getColumnLabel( _nColumnPos );
    // retrieve the name of the column
    // check for duplicate entries
    if(_rColumns.find(sLabel) != _rColumns.end())
    {
        OUString sAlias(sLabel);
        sal_Int32 searchIndex=1;
        while(_rColumns.find(sAlias) != _rColumns.end())
        {
            sAlias = sLabel + OUString::number(searchIndex++);
        }
        sLabel = sAlias;
    }
    _rColumns.emplace(sLabel,0);
    OParseColumn* pColumn = new OParseColumn(
        sLabel,
        _rxResMetaData->getColumnTypeName( _nColumnPos ),
        OUString(),
        OUString(),
        _rxResMetaData->isNullable( _nColumnPos ),
        _rxResMetaData->getPrecision( _nColumnPos ),
        _rxResMetaData->getScale( _nColumnPos ),
        _rxResMetaData->getColumnType( _nColumnPos ),
        _rxResMetaData->isAutoIncrement( _nColumnPos ),
        _rxResMetaData->isCurrency( _nColumnPos ),
        _rxDBMetaData->supportsMixedCaseQuotedIdentifiers(),
        _rxResMetaData->getCatalogName( _nColumnPos ),
        _rxResMetaData->getSchemaName( _nColumnPos ),
        _rxResMetaData->getTableName( _nColumnPos )
    );
    pColumn->setIsSearchable( _rxResMetaData->isSearchable( _nColumnPos ) );
    pColumn->setRealName(_rxResMetaData->getColumnName( _nColumnPos ));
    pColumn->setLabel(sLabel);
    return pColumn;
}


OParseColumn::~OParseColumn()
{
}

void OParseColumn::construct()
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FUNCTION),                PROPERTY_ID_FUNCTION,               0,  &m_bFunction,               cppu::UnoType<decltype(m_bFunction)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_AGGREGATEFUNCTION),       PROPERTY_ID_AGGREGATEFUNCTION,      0,  &m_bAggregateFunction,      cppu::UnoType<decltype(m_bAggregateFunction)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME),                PROPERTY_ID_REALNAME,               0,  &m_aRealName,               cppu::UnoType<decltype(m_aRealName)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DBASEPRECISIONCHANGED),   PROPERTY_ID_DBASEPRECISIONCHANGED,  0,  &m_bDbasePrecisionChanged,  cppu::UnoType<decltype(m_bDbasePrecisionChanged)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISSEARCHABLE),            PROPERTY_ID_ISSEARCHABLE,           0,  &m_bIsSearchable,           cppu::UnoType<decltype(m_bIsSearchable)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_LABEL),                   PROPERTY_ID_LABEL,                  0,  &m_sLabel,                  cppu::UnoType<decltype(m_sLabel)>::get());
}

::cppu::IPropertyArrayHelper* OParseColumn::createArrayHelper() const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper & SAL_CALL OParseColumn::getInfoHelper()
{
    OSL_ENSURE( !isNew(), "OParseColumn::getInfoHelper: a *new* ParseColumn?" );
    return *OParseColumn_PROP::getArrayHelper();
}


OOrderColumn::OOrderColumn( const Reference<XPropertySet>& _xColumn, const OUString& i_rOriginatingTableName,
                            bool _bCase, bool _bAscending )
    : connectivity::sdbcx::OColumn(
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION))),
        getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))),
        getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))),
        getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))),
        getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))),
        getBOOL(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT))),
        false,
        getBOOL(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY))),
        _bCase,
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME))),
        i_rOriginatingTableName
    )
    ,m_bAscending(_bAscending)
{
    construct();
}


OOrderColumn::OOrderColumn( const Reference<XPropertySet>& _xColumn, bool _bCase, bool _bAscending )
    : connectivity::sdbcx::OColumn(
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION))),
        getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))),
        getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))),
        getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))),
        getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))),
        getBOOL(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT))),
        false,
        getBOOL(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY))),
        _bCase,
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME))),
        getString(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TABLENAME)))
    )
    ,m_bAscending(_bAscending)
{
    construct();
}


OOrderColumn::~OOrderColumn()
{
}


void OOrderColumn::construct()
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISASCENDING), PROPERTY_ID_ISASCENDING,
        PropertyAttribute::READONLY,  const_cast< bool* >( &m_bAscending ),    cppu::UnoType<decltype(m_bAscending)>::get() );
}

::cppu::IPropertyArrayHelper* OOrderColumn::createArrayHelper() const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper & SAL_CALL OOrderColumn::getInfoHelper()
{
    OSL_ENSURE( !isNew(), "OOrderColumn::getInfoHelper: a *new* OrderColumn?" );
    return *OOrderColumn_PROP::getArrayHelper();
}

css::uno::Sequence< OUString > SAL_CALL OOrderColumn::getSupportedServiceNames(  )
{
    css::uno::Sequence< OUString > aSupported { "com.sun.star.sdb.OrderColumn" };

    return aSupported;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
