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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_TOOLS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_TOOLS_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>

#include "pq_connection.hxx"
namespace pq_sdbc_driver
{
bool isWhitespace( sal_Unicode c );

OUString concatQualified( const OUString & a, const OUString &b);

OString OUStringToOString( const OUString& str, ConnectionSettings *settings);

void bufferQuoteConstant( OUStringBuffer & buf, const OUString & str, ConnectionSettings *settings );
void bufferQuoteAnyConstant( OUStringBuffer & buf, const com::sun::star::uno::Any &val, ConnectionSettings *settings );

void bufferEscapeConstant( OUStringBuffer & buf, const OUString & str, ConnectionSettings *settings );

OUString sqltype2string(
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & column );


void bufferQuoteQualifiedIdentifier(
    OUStringBuffer & buf, const OUString &schema, const OUString &name, ConnectionSettings *settings );

void bufferQuoteQualifiedIdentifier(
    OUStringBuffer & buf,
    const OUString &schema,
    const OUString &name,
    const OUString &col,
    ConnectionSettings *settings );

void bufferQuoteIdentifier( OUStringBuffer & buf, const OUString &toQuote, ConnectionSettings *settings );
void bufferKey2TableConstraint(
    OUStringBuffer &buf,
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > &key,
    ConnectionSettings *settings  );

OUString extractStringProperty(
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & descriptor,
    const OUString &name );

sal_Int32 extractIntProperty(
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & descriptor,
    const OUString &name );

bool extractBoolProperty(
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & descriptor,
    const OUString &name );

void disposeNoThrow( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & r );
void disposeObject( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & r );

OUString extractTableFromInsert( const OUString & sql );
OString extractSingleTableFromSelect( const OStringVector &vec );

void tokenizeSQL( const OString & sql, OStringVector &vec  );
void splitSQL( const OString & sql, OStringVector &vec  );
com::sun::star::uno::Sequence< sal_Int32 > parseIntArray( const OUString & str );
com::sun::star::uno::Sequence< com::sun::star::uno::Any > parseArray( const OUString & str )
    throw( com::sun::star::sdbc::SQLException );

OUString array2String( const com::sun::star::uno::Sequence< com::sun::star::uno::Any > &seq );

com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > extractConnectionFromStatement(
    const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & stmt );

void splitConcatenatedIdentifier( const OUString & source, OUString *first, OUString *second);


void fillAttnum2attnameMap(
    Int2StringMap &map,
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &conn,
    const OUString &schema,
    const OUString &table );

com::sun::star::uno::Sequence< sal_Int32 > string2intarray( const OUString & str );

com::sun::star::uno::Sequence< OUString > convertMappedIntArray2StringArray(
    const Int2StringMap &map, const com::sun::star::uno::Sequence< sal_Int32> &source );

typedef std::unordered_map
<
    OString,
    OString,
    OStringHash,
    ::std::equal_to< OString >
> String2StringMap;

OUString querySingleValue(
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &connection,
    const OUString &query );

void extractNameValuePairsFromInsert( String2StringMap & map, const OString & lastQuery );
sal_Int32 typeNameToDataType( const OUString &typeName, const OUString &typtype );

// copied from connectivity/source/dbtools, can't use the function directly
bool implSetObject(	const com::sun::star::uno::Reference< com::sun::star::sdbc::XParameters >& _rxParameters,
                    const sal_Int32 _nColumnIndex, const com::sun::star::uno::Any& _rValue);

class DisposeGuard
{
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > d;
public:
    explicit DisposeGuard(const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & r );
    ~DisposeGuard();

};

class TransactionGuard
{
    com::sun::star::uno::Reference< com::sun::star::sdbc::XStatement > m_stmt;
    bool m_commited;
public:
    /// takes over ownership of given statement
    explicit TransactionGuard( const com::sun::star::uno::Reference< com::sun::star::sdbc::XStatement > &stmt );
    ~TransactionGuard( );

    void commit();
    void executeUpdate( const OUString & sql );
};

template < typename T > com::sun::star::uno::Sequence<T> sequence_of_vector ( const std::vector<T> &vec )
{
    if ( vec.empty() )
        return com::sun::star::uno::Sequence<T>();
    else
        return com::sun::star::uno::Sequence<T>( &vec[0], vec.size());
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
