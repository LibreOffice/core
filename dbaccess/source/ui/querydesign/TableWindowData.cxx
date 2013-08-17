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

#include "TableWindowData.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

using namespace dbaui;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

// class OTableWindowData
DBG_NAME(OTableWindowData)
OTableWindowData::OTableWindowData( const Reference< XPropertySet>& _xTable
                                   ,const OUString& _rComposedName
                                   ,const OUString& rTableName
                                   ,const OUString& rWinName )
    :m_xTable(_xTable)
    ,m_aTableName( rTableName )
    ,m_aWinName( rWinName )
    ,m_sComposedName(_rComposedName)
    ,m_aPosition( Point(-1,-1) )
    ,m_aSize( Size(-1,-1) )
    ,m_bShowAll( sal_True )
    ,m_bIsQuery(false)
    ,m_bIsValid(true)
{
    DBG_CTOR(OTableWindowData,NULL);
    if( m_aWinName.isEmpty() )
        m_aWinName = m_aTableName;

    listen();
}

OTableWindowData::~OTableWindowData()
{
    DBG_DTOR(OTableWindowData,NULL);
    Reference<XComponent> xComponent( m_xTable, UNO_QUERY );
    if ( xComponent.is() )
        stopComponentListening( xComponent );
}

sal_Bool OTableWindowData::HasPosition() const
{
    return ( (m_aPosition.X() != -1) && (m_aPosition.Y() != -1) );
}

sal_Bool OTableWindowData::HasSize() const
{
    return ( (m_aSize.Width() != -1) && (m_aSize.Height() !=-1) );
}

void OTableWindowData::_disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // it doesn't matter which one was disposed
    m_xColumns.clear();
    m_xKeys.clear();
    m_xTable.clear();
}

bool OTableWindowData::init(const Reference< XConnection  >& _xConnection,bool _bAllowQueries)
{
    OSL_ENSURE(!m_xTable.is(),"We are already connected to a table!");

    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XQueriesSupplier > xSupQueries( _xConnection, UNO_QUERY_THROW );
    Reference< XNameAccess > xQueries( xSupQueries->getQueries(), UNO_QUERY_THROW );
    bool bIsKnownQuery = _bAllowQueries && xQueries->hasByName( m_sComposedName );

    Reference< XTablesSupplier > xSupTables( _xConnection, UNO_QUERY_THROW );
    Reference< XNameAccess > xTables( xSupTables->getTables(), UNO_QUERY_THROW );
    bool bIsKnownTable = xTables->hasByName( m_sComposedName );

    if ( bIsKnownQuery )
        m_xTable.set( xQueries->getByName( m_sComposedName ), UNO_QUERY );
    else if ( bIsKnownTable )
        m_xTable.set( xTables->getByName( m_sComposedName ), UNO_QUERY );
    else
        m_bIsValid = false;

    // if we survived so far, we know whether it's a query
    m_bIsQuery = bIsKnownQuery;

    listen();

    Reference< XIndexAccess > xColumnsAsIndex( m_xColumns,UNO_QUERY );
    return xColumnsAsIndex.is() && ( xColumnsAsIndex->getCount() > 0 );
}

void OTableWindowData::listen()
{
    if ( m_xTable.is() )
    {
        // listen for the object being disposed
        Reference<XComponent> xComponent( m_xTable, UNO_QUERY );
        if ( xComponent.is() )
            startComponentListening( xComponent );

        // obtain the columns
        Reference< XColumnsSupplier > xColumnsSups( m_xTable, UNO_QUERY);
        if ( xColumnsSups.is() )
            m_xColumns = xColumnsSups->getColumns();

        Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
        if ( xKeySup.is() )
            m_xKeys = xKeySup->getKeys();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
