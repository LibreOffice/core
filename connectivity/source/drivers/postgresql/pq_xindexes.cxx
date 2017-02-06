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
#include <rtl/strbuf.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>

#include "pq_xindexes.hxx"
#include "pq_xindex.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;


using com::sun::star::beans::XPropertySet;

using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::RuntimeException;

using com::sun::star::container::XEnumerationAccess;
using com::sun::star::container::XEnumeration;


using com::sun::star::sdbcx::XColumnsSupplier;

using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XDatabaseMetaData;

namespace pq_sdbc_driver
{

Indexes::Indexes(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName)
    : Container( refMutex, origin, pSettings,  getStatics().KEY ),
      m_schemaName( schemaName ),
      m_tableName( tableName )
{
}

Indexes::~Indexes()
{}

void Indexes::refresh()
{
    try
    {
        if (isLog(m_pSettings, LogLevel::Info))
        {
            OStringBuffer buf;
            buf.append( "sdbcx.Indexes get refreshed for table " );
            buf.append( OUStringToOString( m_schemaName, ConnectionSettings::encoding ) );
            buf.append( "." );
            buf.append( OUStringToOString( m_tableName, ConnectionSettings::encoding ) );
            log( m_pSettings, LogLevel::Info, buf.makeStringAndClear().getStr() );
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
        m_values.clear();
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
            bool isUnique = row->getBoolean( C_IS_UNIQUE );
            bool isPrimary = row->getBoolean( C_IS_PRIMARY );
            bool isClusterd = row->getBoolean( C_IS_CLUSTERED );
            Reference< css::beans::XPropertySet > prop = pIndex;
            pIndex->setPropertyValue_NoBroadcast_public(
                st.IS_UNIQUE, Any( isUnique ) );
            pIndex->setPropertyValue_NoBroadcast_public(
                st.IS_PRIMARY_KEY_INDEX, Any( isPrimary ) );
            pIndex->setPropertyValue_NoBroadcast_public(
                st.IS_CLUSTERED, Any( isClusterd ) );
            pIndex->setPropertyValue_NoBroadcast_public(
                st.NAME, makeAny( currentIndexName ) );

            std::vector< sal_Int32 > seq = parseIntArray( row->getString( C_COLUMNS ) );
            Sequence< OUString > columnNames(seq.size());
            for( size_t columns = 0 ; columns < seq.size() ; columns ++ )
            {
                columnNames[columns] = column2NameMap[ seq[columns] ];
            }

            pIndex->setPropertyValue_NoBroadcast_public(
                st.PRIVATE_COLUMN_INDEXES, makeAny( columnNames ));

            {
                m_values.push_back( makeAny( prop ) );
                map[ currentIndexName ] = index;
                ++index;
            }
        }
        m_name2index.swap( map );
    }
    catch ( css::sdbc::SQLException & e )
    {
        throw RuntimeException( e.Message , e.Context );
    }

    fire( RefreshedBroadcaster( *this ) );
}


void Indexes::appendByDescriptor(
    const css::uno::Reference< css::beans::XPropertySet >& descriptor )
{
    Statics & st = getStatics();
    OUString name = extractStringProperty( descriptor, st.NAME );

    bool isUnique = extractBoolProperty( descriptor, st.IS_UNIQUE );

    OUStringBuffer buf( 128 );

    buf.append( "CREATE " );
    if( isUnique )
        buf.append( "UNIQUE " );
    buf.append( "INDEX " );
    bufferQuoteIdentifier( buf, name, m_pSettings );
    buf.append( " ON " );
    bufferQuoteQualifiedIdentifier( buf, m_schemaName, m_tableName, m_pSettings );

    buf.append( " ( " );

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
                    buf.append( ", " );
                }
                buf.append( extractStringProperty( column, st.NAME ) );
            }
        }
    }
    buf.append( " ) " );

    m_origin->createStatement()->executeUpdate( buf.makeStringAndClear() );
    refresh();
}

void Indexes::dropByIndex( sal_Int32 index )
{


    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >= (sal_Int32)m_values.size() )
    {
        throw css::lang::IndexOutOfBoundsException(
            "Indexes: Index out of range (allowed 0 to "
            + OUString::number( m_values.size() -1 )
            + ", got " + OUString::number( index )
            + ")",
            *this );
    }

    Reference< XPropertySet > set;
    m_values[index] >>= set;
    Statics &st = getStatics();

    OUStringBuffer buf( 128 );
    buf.append( "DROP INDEX " );
    bufferQuoteIdentifier( buf, extractStringProperty( set, st.NAME ), m_pSettings );
    m_origin->createStatement()->executeUpdate( buf.makeStringAndClear() );

    Container::dropByIndex( index );
}


css::uno::Reference< css::beans::XPropertySet > Indexes::createDataDescriptor()
{
    return new IndexDescriptor( m_refMutex, m_origin, m_pSettings );
}

Reference< css::container::XNameAccess > Indexes::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const OUString & schemaName,
    const OUString & tableName)
{
    Indexes *pIndexes = new Indexes( refMutex, origin, pSettings, schemaName, tableName );
    Reference< css::container::XNameAccess > ret = pIndexes;
    pIndexes->refresh();
    return ret;
}


IndexDescriptors::IndexDescriptors(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings)
    : Container( refMutex, origin, pSettings,  getStatics().INDEX )
{}

Reference< css::container::XNameAccess > IndexDescriptors::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings)
{
    return new IndexDescriptors( refMutex, origin, pSettings );
}

css::uno::Reference< css::beans::XPropertySet > IndexDescriptors::createDataDescriptor()
{
    return new IndexDescriptor( m_refMutex, m_origin, m_pSettings );
}

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
