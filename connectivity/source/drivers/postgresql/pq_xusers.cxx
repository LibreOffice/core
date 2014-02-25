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

#include "pq_xusers.hxx"
#include "pq_xuser.hxx"
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

namespace pq_sdbc_driver
{
Users::Users(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings )
    : Container( refMutex, origin, pSettings,  getStatics().USER )
{}

Users::~Users()
{}

void Users::refresh()
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    try
    {
        osl::MutexGuard guard( m_refMutex->mutex );
        Statics & st = getStatics();

        Reference< XStatement > stmt = m_origin->createStatement();

        Reference< XResultSet > rs = stmt->executeQuery( "SELECT usename FROM pg_shadow" );

        Reference< XRow > xRow( rs , UNO_QUERY );

        String2IntMap map;

        m_values = Sequence< com::sun::star::uno::Any > ( );
        sal_Int32 tableIndex = 0;
        while( rs->next() )
        {
            User * pUser =
                new User( m_refMutex, m_origin, m_pSettings );
            Reference< com::sun::star::beans::XPropertySet > prop = pUser;

            OUString name = xRow->getString( 1);
            pUser->setPropertyValue_NoBroadcast_public(
                st.NAME , makeAny(xRow->getString( TABLE_INDEX_CATALOG+1) ) );

            {
                const int currentTableIndex = tableIndex++;
                assert(currentTableIndex  == m_values.getLength());
                m_values.realloc( tableIndex );
                m_values[currentTableIndex] = makeAny( prop );
                map[ name ] = currentTableIndex;
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


void Users::appendByDescriptor(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::ElementExistException,
           ::com::sun::star::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard( m_refMutex->mutex );

    OUStringBuffer update( 128 );
    update.append( "CREATE USER " );
    bufferQuoteIdentifier( update, extractStringProperty( descriptor, getStatics().NAME ), m_pSettings );
    update.append( " PASSWORD " );
    bufferQuoteConstant( update, extractStringProperty( descriptor, getStatics().PASSWORD ), m_pSettings );

    Reference< XStatement > stmt = m_origin->createStatement( );
    DisposeGuard disposeGuard( stmt );
    stmt->executeUpdate( update.makeStringAndClear() );
}

void Users::dropByName( const OUString& elementName )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::NoSuchElementException,
           ::com::sun::star::uno::RuntimeException, std::exception)
{
    String2IntMap::const_iterator ii = m_name2index.find( elementName );
    if( ii == m_name2index.end() )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "User " );
        buf.append( elementName );
        buf.appendAscii( " is unknown, so it can't be dropped" );
        throw com::sun::star::container::NoSuchElementException(
            buf.makeStringAndClear(), *this );
    }
    dropByIndex( ii->second );
}

void Users::dropByIndex( sal_Int32 index )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException, std::exception)
{

    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >= m_values.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "USERS: Index out of range (allowed 0 to " );
        buf.append( (sal_Int32) (m_values.getLength() -1) );
        buf.appendAscii( ", got " );
        buf.append( index );
        buf.appendAscii( ")" );
        throw com::sun::star::lang::IndexOutOfBoundsException(
            buf.makeStringAndClear(), *this );
    }

    Reference< XPropertySet > set;
    m_values[index] >>= set;
    OUString name;
    set->getPropertyValue( getStatics().NAME ) >>= name;

    OUStringBuffer update( 128 );
    update.appendAscii( "DROP USER " );
    bufferQuoteIdentifier( update, name, m_pSettings );

    Reference< XStatement > stmt = m_origin->createStatement( );
    DisposeGuard disposeGuard( stmt );
    stmt->executeUpdate( update.makeStringAndClear() );
}


::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > Users::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return new UserDescriptor( m_refMutex, m_origin, m_pSettings  );
}

Reference< com::sun::star::container::XNameAccess > Users::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings )
{
    Users *pUsers = new Users( refMutex, origin, pSettings );
    Reference< com::sun::star::container::XNameAccess > ret = pUsers;
    pUsers->refresh();

    return ret;
}

void Users::disposing()
{
}

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
