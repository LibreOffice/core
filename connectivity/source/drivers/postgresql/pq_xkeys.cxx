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

#include "pq_xkeys.hxx"
#include "pq_xkey.hxx"
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

Keys::Keys(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName)
    : Container( refMutex, origin, pSettings,  getStatics().KEY ),
      m_schemaName( schemaName ),
      m_tableName( tableName )
{}

Keys::~Keys()
{}

static sal_Int32 string2keytype( const OUString &type )
{
    sal_Int32 ret = com::sun::star::sdbcx::KeyType::UNIQUE;
    if ( type == "p" )
        ret = com::sun::star::sdbcx::KeyType::PRIMARY;
    else if ( type == "f" )
        ret =  com::sun::star::sdbcx::KeyType::FOREIGN;
    return ret;
}

static sal_Int32 string2keyrule( const OUString & rule )
{
    sal_Int32 ret = com::sun::star::sdbc::KeyRule::NO_ACTION;
    if( rule == "r" )
        ret = com::sun::star::sdbc::KeyRule::RESTRICT;
    else if( rule == "c" )
        ret = com::sun::star::sdbc::KeyRule::CASCADE;
    else if( rule == "n" )
        ret = com::sun::star::sdbc::KeyRule::SET_NULL;
    else if( rule == "d" )
        ret = com::sun::star::sdbc::KeyRule::SET_DEFAULT;
    return ret;
}

void Keys::refresh()
    throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        if( isLog( m_pSettings, LogLevel::INFO ) )
        {
            OString buf( "sdbcx.Keys get refreshed for table " +
                         OUStringToOString( m_schemaName, m_pSettings->encoding ) +
                         "." + OUStringToOString( m_tableName,m_pSettings->encoding ));
            log( m_pSettings, LogLevel::INFO, buf.getStr() );
        }

        osl::MutexGuard guard( m_refMutex->mutex );
        Statics & st = getStatics();

        Int2StringMap mainMap;
        fillAttnum2attnameMap( mainMap, m_origin, m_schemaName, m_tableName );

        Reference< XPreparedStatement > stmt = m_origin->prepareStatement(
                "SELECT  conname, "            // 1
                        "contype, "            // 2
                        "confupdtype, "        // 3
                        "confdeltype, "        // 4
                        "class2.relname, "     // 5
                        "nmsp2.nspname, "      // 6
                        "conkey,"              // 7
                        "confkey "             // 8
                "FROM pg_constraint INNER JOIN pg_class ON conrelid = pg_class.oid "
                      "INNER JOIN pg_namespace ON pg_class.relnamespace = pg_namespace.oid "
                      "LEFT JOIN pg_class AS class2 ON confrelid = class2.oid "
                      "LEFT JOIN pg_namespace AS nmsp2 ON class2.relnamespace=nmsp2.oid "
                "WHERE pg_class.relname = ? AND pg_namespace.nspname = ?" );

        Reference< XParameters > paras( stmt, UNO_QUERY );
        paras->setString( 1 , m_tableName );
        paras->setString( 2 , m_schemaName );
        Reference< XResultSet > rs = stmt->executeQuery();

        Reference< XRow > xRow( rs , UNO_QUERY );

        String2IntMap map;
        m_values = Sequence< com::sun::star::uno::Any > ();
        int keyIndex = 0;
        while( rs->next() )
        {
            Key * pKey =
                new Key( m_refMutex, m_origin, m_pSettings , m_schemaName, m_tableName );
            Reference< com::sun::star::beans::XPropertySet > prop = pKey;

            pKey->setPropertyValue_NoBroadcast_public(
                st.NAME, makeAny( xRow->getString( 1 ) ) );
            sal_Int32 keyType = string2keytype( xRow->getString(2) );
            pKey->setPropertyValue_NoBroadcast_public( st.TYPE, makeAny( keyType  ) );
            pKey->setPropertyValue_NoBroadcast_public(
                st.UPDATE_RULE, makeAny( string2keyrule( xRow->getString(3) ) ) );
            pKey->setPropertyValue_NoBroadcast_public(
                st.DELETE_RULE, makeAny( string2keyrule( xRow->getString(4) ) ) );
            pKey->setPropertyValue_NoBroadcast_public(
                st.PRIVATE_COLUMNS,
                makeAny(
                    convertMappedIntArray2StringArray(
                        mainMap,
                        string2intarray( xRow->getString( 7 ) ) ) ) );

            if( com::sun::star::sdbcx::KeyType::FOREIGN == keyType )
            {
                OUStringBuffer buf( 128 );
                buf.append( xRow->getString( 6 ) + "." + xRow->getString( 5 ) );
                pKey->setPropertyValue_NoBroadcast_public(
                    st.REFERENCED_TABLE, makeAny( buf.makeStringAndClear() ) );

                Int2StringMap foreignMap;
                fillAttnum2attnameMap( foreignMap, m_origin, xRow->getString(6), xRow->getString(5));
                pKey->setPropertyValue_NoBroadcast_public(
                    st.PRIVATE_FOREIGN_COLUMNS,
                    makeAny(
                    convertMappedIntArray2StringArray(
                        foreignMap,
                        string2intarray( xRow->getString(8) ) ) ) );
            }


            {
                const int currentKeyIndex = keyIndex++;
                map[ xRow->getString( 1 ) ] = currentKeyIndex;
                assert(currentKeyIndex == m_values.getLength());
                m_values.realloc( keyIndex );
                m_values[currentKeyIndex] = makeAny( prop );
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


void Keys::appendByDescriptor(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::ElementExistException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );

    OUStringBuffer buf( 128 );
    buf.append( "ALTER TABLE " );
    bufferQuoteQualifiedIdentifier( buf, m_schemaName, m_tableName, m_pSettings );
    buf.append( " ADD " );
    bufferKey2TableConstraint( buf, descriptor, m_pSettings );

    Reference< XStatement > stmt =
        m_origin->createStatement();
    stmt->executeUpdate( buf.makeStringAndClear() );
}


void Keys::dropByIndex( sal_Int32 index )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >= m_values.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "TABLES: Index out of range (allowed 0 to " + OUString::number(m_values.getLength() -1) +
                    ", got " + OUString::number( index ) + ")" );
        throw com::sun::star::lang::IndexOutOfBoundsException(
            buf.makeStringAndClear(), *this );
    }


    Reference< XPropertySet > set;
    m_values[index] >>= set;

    OUStringBuffer buf( 128 );
    buf.append( "ALTER TABLE " );
    bufferQuoteQualifiedIdentifier( buf, m_schemaName, m_tableName, m_pSettings );
    buf.append( " DROP CONSTRAINT " );
    bufferQuoteIdentifier( buf, extractStringProperty( set , getStatics().NAME ), m_pSettings );
    m_origin->createStatement()->executeUpdate( buf.makeStringAndClear() );


    Container::dropByIndex( index );
}


::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > Keys::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new KeyDescriptor( m_refMutex, m_origin, m_pSettings );
}

Reference< com::sun::star::container::XIndexAccess > Keys::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const OUString & schemaName,
    const OUString & tableName)
{
    Keys *pKeys = new Keys( refMutex, origin, pSettings, schemaName, tableName );
    Reference< com::sun::star::container::XIndexAccess > ret = pKeys;
    pKeys->refresh();

    return ret;
}
//_________________________________________________________________________________________
KeyDescriptors::KeyDescriptors(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings)
    : Container( refMutex, origin, pSettings,  getStatics().KEY )
{}

Reference< com::sun::star::container::XIndexAccess > KeyDescriptors::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings)
{
    return new KeyDescriptors( refMutex, origin, pSettings );
}

::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > KeyDescriptors::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new KeyDescriptor( m_refMutex, m_origin, m_pSettings );
}

};
