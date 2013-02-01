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
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>

#include "pq_xindexes.hxx"
#include "pq_xindex.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;

using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

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
using com::sun::star::container::XEnumerationAccess;
using com::sun::star::container::XEnumeration;

using com::sun::star::lang::WrappedTargetException;

using com::sun::star::sdbcx::XColumnsSupplier;

using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XDatabaseMetaData;

namespace pq_sdbc_driver
{

Indexes::Indexes(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const rtl::OUString &schemaName,
        const rtl::OUString &tableName)
    : Container( refMutex, origin, pSettings,  getStatics().KEY ),
      m_schemaName( schemaName ),
      m_tableName( tableName )
{
}

Indexes::~Indexes()
{}

void Indexes::refresh()
    throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        if( isLog( m_pSettings, LogLevel::INFO ) )
        {
            rtl::OStringBuffer buf;
            buf.append( "sdbcx.Indexes get refreshed for table " );
            buf.append( OUStringToOString( m_schemaName, m_pSettings->encoding ) );
            buf.append( "." );
            buf.append( OUStringToOString( m_tableName,m_pSettings->encoding ) );
            log( m_pSettings, LogLevel::INFO, buf.makeStringAndClear().getStr() );
        }

        osl::MutexGuard guard( m_refMutex->mutex );
        Statics & st = getStatics();

        Int2StringMap column2NameMap;
        fillAttnum2attnameMap( column2NameMap, m_origin, m_schemaName, m_tableName );

        // see XDatabaseMetaData::getIndexInfo()
        Reference< XPreparedStatement > stmt = m_origin->prepareStatement(
                "SELECT nspname, "      // 1
                   "pg_class.relname, " // 2
                   "class2.relname, "   // 3
                   "indisclustered, "   // 4
                   "indisunique, "      // 5
                   "indisprimary, "     // 6
                   "indkey "            // 7
                "FROM pg_index INNER JOIN pg_class ON indrelid = pg_class.oid "
                    "INNER JOIN pg_namespace ON pg_class.relnamespace = pg_namespace.oid "
                    "INNER JOIN pg_class as class2 ON pg_index.indexrelid = class2.oid "
                "WHERE nspname = ? AND pg_class.relname = ?" );

        Reference< XParameters > params( stmt, UNO_QUERY);
        params->setString( 1, m_schemaName );
        params->setString( 2, m_tableName );
        Reference< XResultSet > rs = stmt->executeQuery();

        Reference< XRow > row( rs, UNO_QUERY );
        String2IntMap map;
        m_values = Sequence< com::sun::star::uno::Any > ();
        sal_Int32 index = 0;
        while( rs->next() )
        {
            static const sal_Int32 C_SCHEMA = 1;
            static const sal_Int32 C_TABLENAME = 2;
            static const sal_Int32 C_INDEXNAME = 3;
            static const sal_Int32 C_IS_CLUSTERED = 4;
            static const sal_Int32 C_IS_UNIQUE = 5;
            static const sal_Int32 C_IS_PRIMARY = 6;
            static const sal_Int32 C_COLUMNS = 7;
            OUString currentIndexName = row->getString( C_INDEXNAME );
            Index *pIndex =
                new Index( m_refMutex, m_origin, m_pSettings,
                           m_schemaName, m_tableName );

            (void) C_SCHEMA; (void) C_TABLENAME;
            sal_Bool isUnique = row->getBoolean( C_IS_UNIQUE );
            sal_Bool isPrimary = row->getBoolean( C_IS_PRIMARY );
            sal_Bool isClusterd = row->getBoolean( C_IS_CLUSTERED );
            Reference< com::sun::star::beans::XPropertySet > prop = pIndex;
            pIndex->setPropertyValue_NoBroadcast_public(
                st.IS_UNIQUE, Any( &isUnique, getBooleanCppuType() ) );
            pIndex->setPropertyValue_NoBroadcast_public(
                st.IS_PRIMARY_KEY_INDEX, Any( &isPrimary, getBooleanCppuType() ) );
            pIndex->setPropertyValue_NoBroadcast_public(
                st.IS_CLUSTERED, Any( &isClusterd, getBooleanCppuType() ) );
            pIndex->setPropertyValue_NoBroadcast_public(
                st.NAME, makeAny( currentIndexName ) );

            Sequence< sal_Int32 > seq = parseIntArray( row->getString( C_COLUMNS ) );
            Sequence< OUString > columnNames(seq.getLength());
            for( int columns = 0 ; columns < seq.getLength() ; columns ++ )
            {
                columnNames[columns] = column2NameMap[ seq[columns] ];
            }

            pIndex->setPropertyValue_NoBroadcast_public(
                st.PRIVATE_COLUMN_INDEXES, makeAny( columnNames ));

            {
                const int currentIndex = index++;
                assert(currentIndex  == m_values.getLength());
                m_values.realloc( index );
                m_values[currentIndex] = makeAny( prop );
                map[ currentIndexName ] = currentIndex;
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


void Indexes::appendByDescriptor(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::ElementExistException,
           ::com::sun::star::uno::RuntimeException)
{
    Statics & st = getStatics();
    OUString name = extractStringProperty( descriptor, st.NAME );

    sal_Bool isUnique = extractBoolProperty( descriptor, st.IS_UNIQUE );

    OUStringBuffer buf( 128 );

    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "CREATE " ) );
    if( isUnique )
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "UNIQUE " ) );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "INDEX " ) );
    bufferQuoteIdentifier( buf, name, m_pSettings );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " ON " ) );
    bufferQuoteQualifiedIdentifier( buf, m_schemaName, m_tableName, m_pSettings );

    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " ( " ) );

    Reference< XColumnsSupplier > columns( descriptor, UNO_QUERY );
    if( columns.is() )
    {
        Reference< XEnumerationAccess > access( columns->getColumns(), UNO_QUERY );
        if( access.is() )
        {
            Reference< XEnumeration > xEnum( access->createEnumeration() );
            bool first = true;
            while( xEnum.is() && xEnum->hasMoreElements() )
            {
                Reference< XPropertySet > column( xEnum->nextElement(), UNO_QUERY );
                if( first )
                {
                    first = false;
                }
                else
                {
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
                }
                buf.append( extractStringProperty( column, st.NAME ) );
            }
        }
    }
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " ) " ) );

    m_origin->createStatement()->executeUpdate( buf.makeStringAndClear() );
    refresh();
}

void Indexes::dropByIndex( sal_Int32 index )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException)
{


    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >= m_values.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "Indexes: Index out of range (allowed 0 to " );
        buf.append( (sal_Int32) (m_values.getLength() -1) );
        buf.appendAscii( ", got " );
        buf.append( index );
        buf.appendAscii( ")" );
        throw com::sun::star::lang::IndexOutOfBoundsException(
            buf.makeStringAndClear(), *this );
    }

    Reference< XPropertySet > set;
    m_values[index] >>= set;
    Statics &st = getStatics();

    OUStringBuffer buf( 128 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "DROP INDEX " ) );
    bufferQuoteIdentifier( buf, extractStringProperty( set, st.NAME ), m_pSettings );
    m_origin->createStatement()->executeUpdate( buf.makeStringAndClear() );

    Container::dropByIndex( index );
}


::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > Indexes::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new IndexDescriptor( m_refMutex, m_origin, m_pSettings );
}

Reference< com::sun::star::container::XNameAccess > Indexes::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const rtl::OUString & schemaName,
    const rtl::OUString & tableName)
{
    Indexes *pIndexes = new Indexes( refMutex, origin, pSettings, schemaName, tableName );
    Reference< com::sun::star::container::XNameAccess > ret = pIndexes;
    pIndexes->refresh();
    return ret;
}


//_________________________________________________________________________________________
IndexDescriptors::IndexDescriptors(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings)
    : Container( refMutex, origin, pSettings,  getStatics().INDEX )
{}

Reference< com::sun::star::container::XNameAccess > IndexDescriptors::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings)
{
    return new IndexDescriptors( refMutex, origin, pSettings );
}

::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > IndexDescriptors::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new IndexDescriptor( m_refMutex, m_origin, m_pSettings );
}

};
