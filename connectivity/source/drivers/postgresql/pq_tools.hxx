/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  $RCSfile: pq_tools.hxx,v $
 *
 *  $Revision: 1.1.2.7 $
 *
 *  last change: $Author: jbu $ $Date: 2008/07/07 21:37:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *              2011 Lionel Elie Mamane <lionel@mamane.lu>
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
 *
 *  Non-Sun changes:
 *  * august 2011: switch to unordered_map instead of deprecated hash_map
 *
 ************************************************************************/

#ifndef _PQ_TOOLS_
#define _PQ_TOOLS_

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>

#include <pq_connection.hxx>
namespace pq_sdbc_driver
{
bool isWhitespace( sal_Unicode c );

rtl::OUString date2String( const com::sun::star::util::Date & date );
com::sun::star::util::Date string2Date( const rtl::OUString & str );

rtl::OUString time2String( const com::sun::star::util::Time & time );
com::sun::star::util::Time string2Time( const rtl::OUString & str );

rtl::OUString dateTime2String( const com::sun::star::util::DateTime & dateTime );
com::sun::star::util::DateTime string2DateTime( const rtl::OUString & dateTime );

rtl::OUString concatQualified( const rtl::OUString & a, const rtl::OUString &b);

rtl::OString OUStringToOString( rtl::OUString str, ConnectionSettings *settings);

void bufferQuoteConstant( rtl::OUStringBuffer & buf, const rtl::OUString & str, ConnectionSettings *settings );
    void bufferQuoteAnyConstant( rtl::OUStringBuffer & buf, const com::sun::star::uno::Any &val, ConnectionSettings *settings );

void bufferEscapeConstant( rtl::OUStringBuffer & buf, const rtl::OUString & str, ConnectionSettings *settings );

::rtl::OUString sqltype2string(
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & column );


void bufferQuoteQualifiedIdentifier(
    rtl::OUStringBuffer & buf, const rtl::OUString &schema, const rtl::OUString &name, ConnectionSettings *settings );

void bufferQuoteQualifiedIdentifier(
    rtl::OUStringBuffer & buf,
    const rtl::OUString &schema,
    const rtl::OUString &name,
    const rtl::OUString &col,
    ConnectionSettings *settings );

void bufferQuoteIdentifier( rtl::OUStringBuffer & buf, const rtl::OUString &toQuote, ConnectionSettings *settings );
void bufferKey2TableConstraint(
    rtl::OUStringBuffer &buf,
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > &key,
    ConnectionSettings *settings  );

rtl::OUString extractStringProperty(
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & descriptor,
    const rtl::OUString &name );

sal_Int32 extractIntProperty(
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & descriptor,
    const rtl::OUString &name );

sal_Bool extractBoolProperty(
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & descriptor,
    const rtl::OUString &name );

void disposeNoThrow( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & r );
void disposeObject( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & r );

void rollbackNoThrow( const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > & r );

::rtl::OUString extractTableFromInsert( const rtl::OUString & sql );
::rtl::OString extractSingleTableFromSelect( const OStringVector &vec );

void tokenizeSQL( const rtl::OString & sql, OStringVector &vec  );
void splitSQL( const rtl::OString & sql, OStringVector &vec  );
com::sun::star::uno::Sequence< sal_Int32 > parseIntArray( const ::rtl::OUString & str );
com::sun::star::uno::Sequence< com::sun::star::uno::Any > parseArray( const ::rtl::OUString & str )
    throw( com::sun::star::sdbc::SQLException );

rtl::OUString array2String( const com::sun::star::uno::Sequence< com::sun::star::uno::Any > &seq );

com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > extractConnectionFromStatement(
    const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & stmt );

void splitConcatenatedIdentifier( const rtl::OUString & source, rtl::OUString *first, rtl::OUString *second);


void fillAttnum2attnameMap(
    Int2StringMap &map,
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &conn,
    const rtl::OUString &schema,
    const rtl::OUString &table );

com::sun::star::uno::Sequence< sal_Int32 > string2intarray( const ::rtl::OUString & str );

com::sun::star::uno::Sequence< rtl::OUString > convertMappedIntArray2StringArray(
    const Int2StringMap &map, const com::sun::star::uno::Sequence< sal_Int32> &source );

typedef std::unordered_map
<
    ::rtl::OString,
    ::rtl::OString,
    ::rtl::OStringHash,
    ::std::equal_to< rtl::OString >,
    Allocator< ::std::pair< rtl::OString, ::rtl::OString > >
> String2StringMap;

rtl::OUString querySingleValue(
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &connection,
    const rtl::OUString &query );

void extractNameValuePairsFromInsert( String2StringMap & map, const rtl::OString & lastQuery );
sal_Int32 typeNameToDataType( const rtl::OUString &typeName, const rtl::OUString &typtype );

// copied from connectivity/source/dbtools, can't use the function directly
bool implSetObject(	const com::sun::star::uno::Reference< com::sun::star::sdbc::XParameters >& _rxParameters,
                    const sal_Int32 _nColumnIndex, const com::sun::star::uno::Any& _rValue);

class DisposeGuard
{
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > d;
public:
    DisposeGuard(const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & r );
    ~DisposeGuard();

};

class TransactionGuard
{
    com::sun::star::uno::Reference< com::sun::star::sdbc::XStatement > m_stmt;
    sal_Bool m_commited;
public:
    /// takes over ownership of given statemet
    TransactionGuard( const com::sun::star::uno::Reference< com::sun::star::sdbc::XStatement > &stmt );
    ~TransactionGuard( );

    void commit();
    void executeUpdate( const rtl::OUString & sql );
};

}

#endif
