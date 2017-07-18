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
#include <vector>

namespace pq_sdbc_driver
{
bool isWhitespace( sal_Unicode c );

OUString concatQualified( const OUString & a, const OUString &b);

OString OUStringToOString( const OUString& str, ConnectionSettings const *settings);

void bufferQuoteConstant( OUStringBuffer & buf, const OUString & str, ConnectionSettings *settings );
void bufferQuoteAnyConstant( OUStringBuffer & buf, const css::uno::Any &val, ConnectionSettings *settings );

void bufferEscapeConstant( OUStringBuffer & buf, const OUString & str, ConnectionSettings *settings );

OUString sqltype2string(
    const css::uno::Reference< css::beans::XPropertySet > & column );


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
    const css::uno::Reference< css::beans::XPropertySet > &key,
    ConnectionSettings *settings  );

OUString extractStringProperty(
    const css::uno::Reference< css::beans::XPropertySet > & descriptor,
    const OUString &name );

sal_Int32 extractIntProperty(
    const css::uno::Reference< css::beans::XPropertySet > & descriptor,
    const OUString &name );

bool extractBoolProperty(
    const css::uno::Reference< css::beans::XPropertySet > & descriptor,
    const OUString &name );

void disposeNoThrow( const css::uno::Reference< css::uno::XInterface > & r );
void disposeObject( const css::uno::Reference< css::uno::XInterface > & r );

OUString extractTableFromInsert( const OUString & sql );
OString extractSingleTableFromSelect( const OStringVector &vec );

void tokenizeSQL( const OString & sql, OStringVector &vec  );
void splitSQL( const OString & sql, OStringVector &vec  );
std::vector< sal_Int32 > parseIntArray( const OUString & str );
/// @throws css::sdbc::SQLException
std::vector< css::uno::Any > parseArray( const OUString & str );

OUString array2String( const css::uno::Sequence< css::uno::Any > &seq );

css::uno::Reference< css::sdbc::XConnection > extractConnectionFromStatement(
    const css::uno::Reference< css::uno::XInterface > & stmt );

void splitConcatenatedIdentifier( const OUString & source, OUString *first, OUString *second);


void fillAttnum2attnameMap(
    Int2StringMap &map,
    const css::uno::Reference< css::sdbc::XConnection > &conn,
    const OUString &schema,
    const OUString &table );

css::uno::Sequence< sal_Int32 > string2intarray( const OUString & str );

css::uno::Sequence< OUString > convertMappedIntArray2StringArray(
    const Int2StringMap &map, const css::uno::Sequence< sal_Int32> &source );

typedef std::unordered_map
<
    OString,
    OString,
    OStringHash > String2StringMap;

OUString querySingleValue(
    const css::uno::Reference< css::sdbc::XConnection > &connection,
    const OUString &query );

void extractNameValuePairsFromInsert( String2StringMap & map, const OString & lastQuery );
sal_Int32 typeNameToDataType( const OUString &typeName, const OUString &typtype );

// copied from connectivity/source/dbtools, can't use the function directly
bool implSetObject( const css::uno::Reference< css::sdbc::XParameters >& _rxParameters,
                    const sal_Int32 _nColumnIndex, const css::uno::Any& _rValue);

class DisposeGuard
{
    css::uno::Reference< css::uno::XInterface > d;
public:
    explicit DisposeGuard(const css::uno::Reference< css::uno::XInterface > & r );
    ~DisposeGuard();

};

class TransactionGuard
{
    css::uno::Reference< css::sdbc::XStatement > m_stmt;
    bool m_commited;
public:
    /// takes over ownership of given statement
    explicit TransactionGuard( const css::uno::Reference< css::sdbc::XStatement > &stmt );
    ~TransactionGuard( );

    void commit();
    void executeUpdate( const OUString & sql );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
