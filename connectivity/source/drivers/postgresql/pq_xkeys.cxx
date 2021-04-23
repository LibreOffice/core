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

#include <sal/config.h>

#include <string_view>

#include <sal/log.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <cppuhelper/exc_hlp.hxx>

#include "pq_xkeys.hxx"
#include "pq_xkey.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;


using css::beans::XPropertySet;

using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Reference;


using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XPreparedStatement;

namespace pq_sdbc_driver
{

Keys::Keys(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName)
    : Container( refMutex, origin, pSettings,  getStatics().KEY ),
      m_schemaName( schemaName ),
      m_tableName( tableName )
{}

Keys::~Keys()
{}

static sal_Int32 string2keytype( std::u16string_view type )
{
    sal_Int32 ret = css::sdbcx::KeyType::UNIQUE;
    if ( type == u"p" )
        ret = css::sdbcx::KeyType::PRIMARY;
    else if ( type == u"f" )
        ret =  css::sdbcx::KeyType::FOREIGN;
    return ret;
}

static sal_Int32 string2keyrule( std::u16string_view rule )
{
    sal_Int32 ret = css::sdbc::KeyRule::NO_ACTION;
    if( rule == u"r" )
        ret = css::sdbc::KeyRule::RESTRICT;
    else if( rule == u"c" )
        ret = css::sdbc::KeyRule::CASCADE;
    else if( rule == u"n" )
        ret = css::sdbc::KeyRule::SET_NULL;
    else if( rule == u"d" )
        ret = css::sdbc::KeyRule::SET_DEFAULT;
    return ret;
}

void Keys::refresh()
{
    try
    {
        SAL_INFO("connectivity.postgresql", "sdbcx.Keys get refreshed for table " << m_schemaName << "." << m_tableName);

        osl::MutexGuard guard( m_xMutex->GetMutex() );
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
        m_values.clear();
        int keyIndex = 0;
        while( rs->next() )
        {
            rtl::Reference<Key> pKey =
                new Key( m_xMutex, m_origin, m_pSettings , m_schemaName, m_tableName );
            Reference< css::beans::XPropertySet > prop = pKey;

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

            if( css::sdbcx::KeyType::FOREIGN == keyType )
            {
                OUString buf = xRow->getString( 6 ) + "." + xRow->getString( 5 );
                pKey->setPropertyValue_NoBroadcast_public(
                    st.REFERENCED_TABLE, makeAny( buf ) );

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
                map[ xRow->getString( 1 ) ] = keyIndex;
                m_values.push_back( makeAny( prop ) );
                ++keyIndex;
            }
        }
        m_name2index.swap( map );
    }
    catch ( css::sdbc::SQLException & e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException( e.Message,
                        e.Context, anyEx );
    }

    fire( RefreshedBroadcaster( *this ) );
}


void Keys::appendByDescriptor(
    const css::uno::Reference< css::beans::XPropertySet >& descriptor )
{
    osl::MutexGuard guard( m_xMutex->GetMutex() );

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
{
    osl::MutexGuard guard( m_xMutex->GetMutex() );
    if( index < 0 ||  index >= static_cast<sal_Int32>(m_values.size()) )
    {
        throw css::lang::IndexOutOfBoundsException(
            "TABLES: Index out of range (allowed 0 to " + OUString::number(m_values.size() -1)
            + ", got " + OUString::number( index ) + ")",
            *this );
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


css::uno::Reference< css::beans::XPropertySet > Keys::createDataDescriptor()
{
    return new KeyDescriptor( m_xMutex, m_origin, m_pSettings );
}

Reference< css::container::XIndexAccess > Keys::create(
    const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const OUString & schemaName,
    const OUString & tableName)
{
    rtl::Reference<Keys> pKeys = new Keys( refMutex, origin, pSettings, schemaName, tableName );
    pKeys->refresh();

    return pKeys;
}

KeyDescriptors::KeyDescriptors(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings)
    : Container( refMutex, origin, pSettings,  getStatics().KEY )
{}

Reference< css::container::XIndexAccess > KeyDescriptors::create(
    const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings)
{
    return new KeyDescriptors( refMutex, origin, pSettings );
}

css::uno::Reference< css::beans::XPropertySet > KeyDescriptors::createDataDescriptor()
{
    return new KeyDescriptor( m_xMutex, m_origin, m_pSettings );
}

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
