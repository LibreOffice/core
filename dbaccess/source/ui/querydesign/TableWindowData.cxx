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
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
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

//==================================================================
// class OTableWindowData
//==================================================================
DBG_NAME(OTableWindowData)
//------------------------------------------------------------------------------
OTableWindowData::OTableWindowData( const Reference< XPropertySet>& _xTable
                                   ,const ::rtl::OUString& _rComposedName
                                   ,const ::rtl::OUString& rTableName
                                   ,const ::rtl::OUString& rWinName )
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
    if( !m_aWinName.getLength() )
        m_aWinName = m_aTableName;

    listen();
}

//------------------------------------------------------------------------------
OTableWindowData::~OTableWindowData()
{
    DBG_DTOR(OTableWindowData,NULL);
    Reference<XComponent> xComponent( m_xTable, UNO_QUERY );
    if ( xComponent.is() )
        stopComponentListening( xComponent );
}

//------------------------------------------------------------------------------
sal_Bool OTableWindowData::HasPosition() const
{
    return ( (m_aPosition.X() != -1) && (m_aPosition.Y() != -1) );
}

//------------------------------------------------------------------------------
sal_Bool OTableWindowData::HasSize() const
{
    return ( (m_aSize.Width() != -1) && (m_aSize.Height() !=-1) );
}
// -----------------------------------------------------------------------------
void OTableWindowData::_disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // it doesn't matter which one was disposed
    m_xColumns.clear();
    m_xKeys.clear();
    m_xTable.clear();
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
