/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableWindowData.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-29 14:27:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    ,m_bShowAll( TRUE )
    ,m_bIsQuery(false)
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
BOOL OTableWindowData::HasPosition() const
{
    return ( (m_aPosition.X() != -1) && (m_aPosition.Y() != -1) );
}

//------------------------------------------------------------------------------
BOOL OTableWindowData::HasSize() const
{
    return ( (m_aSize.Width() != -1) && (m_aSize.Height() !=-1) );
}
// -----------------------------------------------------------------------------
void OTableWindowData::_disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // it doesn't matter which one was disposed
    m_xTable.clear();
    m_xColumns.clear();;
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
        m_xTable.set( xQueries->getByName( m_sComposedName ), UNO_QUERY_THROW );
    else if ( bIsKnownTable )
        m_xTable.set( xTables->getByName( m_sComposedName ), UNO_QUERY_THROW );
    else
        DBG_ERROR( "OTableWindow::Init: this is neither a query (or no queries are allowed) nor a table!" );

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
        Reference< XColumnsSupplier > xColumnsSups( m_xTable, UNO_QUERY_THROW );
        m_xColumns = xColumnsSups->getColumns();
        xComponent.set( m_xColumns, UNO_QUERY );
        if ( xComponent.is() )
            startComponentListening( xComponent );
    }
}
// -----------------------------------------------------------------------------
