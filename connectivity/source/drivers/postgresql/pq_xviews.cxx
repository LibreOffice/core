/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
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
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>

#include "pq_xviews.hxx"
#include "pq_xview.hxx"
#include "pq_xtables.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;


using com::sun::star::beans::XPropertySet;

using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Type;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::RuntimeException;

using com::sun::star::container::NoSuchElementException;
using com::sun::star::lang::WrappedTargetException;

using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XDatabaseMetaData;

//  using com::sun::star::sdbcx::Privilege;

namespace pq_sdbc_driver
{
Views::Views(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings )
    : Container( refMutex, origin, pSettings,  getStatics().VIEW )
{}

Views::~Views()
{}

void Views::refresh()
    throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        osl::MutexGuard guard( m_refMutex->mutex );
        Statics & st = getStatics();

        Reference< XStatement > stmt = m_origin->createStatement();

        Reference< XResultSet > rs = stmt->executeQuery("SELECT "
                                                        "DISTINCT ON( pg_namespace.nspname, relname) " // needed because of duplicates
                                                        "pg_namespace.nspname,"    // 1
                                                        "relname,"                 // 2
                                                        "pg_get_viewdef(ev_class) " // 3
                                                        "FROM pg_namespace, pg_class, pg_rewrite "
                                                       "WHERE pg_namespace.oid = relnamespace "
                                                         "AND pg_class.oid = ev_class "
                                                         "AND relkind=\'v\'" );

        Reference< XRow > xRow( rs , UNO_QUERY );

        m_values = Sequence< com::sun::star::uno::Any > ();
        String2IntMap map;
        sal_Int32 viewIndex = 0;

        while( rs->next() )
        {
            OUString table, schema, command;
            schema = xRow->getString( 1 );
            table = xRow->getString( 2 );
            command = xRow->getString( 3 );

            View *pView = new View (m_refMutex, m_origin, m_pSettings );
            Reference< com::sun::star::beans::XPropertySet > prop = pView;

            pView->setPropertyValue_NoBroadcast_public(st.NAME , makeAny(table) );
            pView->setPropertyValue_NoBroadcast_public(st.SCHEMA_NAME, makeAny(schema) );
            pView->setPropertyValue_NoBroadcast_public(st.COMMAND, makeAny(command) );

            {
                const int currentViewIndex = viewIndex++;
                assert(currentViewIndex  == m_values.getLength());
                m_values.realloc( viewIndex );
                m_values[currentViewIndex] = makeAny( prop );
                OUStringBuffer buf( table.getLength() + schema.getLength() + 1);
                buf.append( schema + "." + table );
                map[ buf.makeStringAndClear() ] = currentViewIndex;
            }
        }
        m_name2index.swap( map );
    }
    catch ( com::sun::star::sdbc::SQLException & e )
    {
        throw RuntimeException( e.Message , e.Context );
    }
    fire( RefreshedBroadcaster( *this ) );
}


void Views::appendByDescriptor(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::ElementExistException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );

    Statics &st = getStatics();
    OUString name,schema,command;
    descriptor->getPropertyValue( st.SCHEMA_NAME ) >>= schema;
    descriptor->getPropertyValue( st.NAME ) >>= name;
    descriptor->getPropertyValue( st.COMMAND ) >>= command;

    Reference< XStatement > stmt = m_origin->createStatement();

    OUStringBuffer buf( 128 );

    buf.append( "CREATE VIEW ");
    bufferQuoteQualifiedIdentifier( buf, schema, name, m_pSettings );
    buf.append(" AS " + command );

    stmt->executeUpdate( buf.makeStringAndClear() );

    disposeNoThrow( stmt );
    refresh();
    if( m_pSettings->tables.is() )
    {
        m_pSettings->pTablesImpl->refresh();
    }
}

void Views::dropByName( const OUString& elementName )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::NoSuchElementException,
           ::com::sun::star::uno::RuntimeException)
{
    String2IntMap::const_iterator ii = m_name2index.find( elementName );
    if( ii == m_name2index.end() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "View " + elementName + " is unknown, so it can't be dropped" );
        throw com::sun::star::container::NoSuchElementException(
            buf.makeStringAndClear(), *this );
    }
    dropByIndex( ii->second );
}

void Views::dropByIndex( sal_Int32 index )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >= m_values.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "VIEWS: Index out of range (allowed 0 to " + OUString::number(m_values.getLength() -1) +
                    ", got " + OUString::number( index ) + ")");
        throw com::sun::star::lang::IndexOutOfBoundsException(
            buf.makeStringAndClear(), *this );
    }

    Reference< XPropertySet > set;
    m_values[index] >>= set;
    Statics &st = getStatics();
    OUString name,schema;
    set->getPropertyValue( st.SCHEMA_NAME ) >>= schema;
    set->getPropertyValue( st.NAME ) >>= name;

    OUStringBuffer update( 128 );
    update.append( "DROP VIEW \"" + schema + "\".\"" + name + "\"" );

    Reference< XStatement > stmt = m_origin->createStatement( );

    stmt->executeUpdate( update.makeStringAndClear() );
}


::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > Views::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new ViewDescriptor( m_refMutex, m_origin, m_pSettings );
}

Reference< com::sun::star::container::XNameAccess > Views::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    Views **ppViews)
{
    *ppViews = new Views( refMutex, origin, pSettings );
    Reference< com::sun::star::container::XNameAccess > ret = *ppViews;
    (*ppViews)->refresh();

    return ret;
}

void Views::disposing()
{
    Container::disposing();
}



};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
