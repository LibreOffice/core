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

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>

#include "pq_xcontainer.hxx"
#include "pq_tools.hxx"
#include "pq_statics.hxx"

#include <libpq-fe.h>
#include <string.h>

using rtl::OUString;
using rtl::OUStringBuffer;

using com::sun::star::beans::XPropertySet;

using com::sun::star::lang::XComponent;

using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XConnection;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XRow;

using com::sun::star::sdbcx::XColumnsSupplier;

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;

using com::sun::star::container::XEnumeration;
using com::sun::star::container::XEnumerationAccess;

namespace pq_sdbc_driver
{

rtl::OUString date2String( const com::sun::star::util::Date & x )
{
    char buffer[64];
    sprintf( buffer, "%d-%02d-%02d", x.Year, x.Month, x.Day );
    return OUString::createFromAscii( buffer );
}

com::sun::star::util::Date string2Date( const  rtl::OUString &date )
{
    // Format: Year-Month-Day
    com::sun::star::util::Date ret;

    ret.Year = (sal_Int32) rtl_ustr_toInt32( date.pData->buffer, 10 );

    int index = date.indexOf( '-' );
    if( index >= 0 )
    {
        ret.Month = (sal_Int32)rtl_ustr_toInt32( &(date.pData->buffer[ index+1]), 10 );
        int start = index;
        index = date.indexOf( '-', start+1 );
        if( index >= 0 )
        {
            ret.Day = (sal_Int32)rtl_ustr_toInt32( &date.pData->buffer[index+1], 10 );
        }
    }
    return ret;
}

rtl::OUString time2String( const com::sun::star::util::Time & x )
{
    char buffer[64];
    sprintf( buffer, "%02d:%02d:%02d.%02d", x.Hours, x.Minutes, x.Seconds, x.HundredthSeconds );
    return OUString::createFromAscii( buffer );

}


com::sun::star::util::Time string2Time( const rtl::OUString & time )
{
    com::sun::star::util::Time ret;

    sal_Unicode temp[4];

    temp[0] = time[0];
    temp[1] = time[1];
    temp[2] = 0;
    ret.Hours = (sal_Int32)rtl_ustr_toInt32( temp , 10 );

    temp[0] = time[3];
    temp[1] = time[4];
    ret.Minutes = (sal_Int32)rtl_ustr_toInt32( temp , 10 );

    temp[0] = time[6];
    temp[1] = time[7];
    ret.Seconds = (sal_Int32)rtl_ustr_toInt32( temp , 10 );

    if( time.getLength() >9 )
    {
        ret.HundredthSeconds = (sal_Int32)rtl_ustr_toInt32( &time.getStr()[9] , 10 );
    }
    return ret;

}



rtl::OUString dateTime2String( const com::sun::star::util::DateTime & x )
{
    char buffer[128];
    sprintf( buffer, "%d-%02d-%02d %02d:%02d:%02d.%02d",
             x.Year, x.Month, x.Day,
             x.Hours, x.Minutes, x.Seconds, x.HundredthSeconds );
    return OUString::createFromAscii( buffer );

}

com::sun::star::util::DateTime string2DateTime( const rtl::OUString & dateTime )
{
    int space = dateTime.indexOf( ' ' );
    com::sun::star::util::DateTime ret;

    if( space >= 0 )
    {
        com::sun::star::util::Date date ( string2Date( OUString( dateTime.getStr(), space ) ) );
        com::sun::star::util::Time time( string2Time( OUString( dateTime.getStr() + space +1 ) ) );
        ret.Day = date.Day;
        ret.Month = date.Month;
        ret.Year = date.Year;

        ret.Hours = time.Hours;
        ret.Minutes = time.Minutes;
        ret.Seconds = time.Seconds;
        ret.HundredthSeconds = time.HundredthSeconds;
    }
    return ret;
}

rtl::OUString concatQualified( const rtl::OUString & a, const rtl::OUString &b)
{
    rtl::OUStringBuffer buf( a.getLength() + 2 + b.getLength() );
    buf.append( a );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "." ) );
    buf.append( b );
    return buf.makeStringAndClear();
}

static inline rtl::OString iOUStringToOString( const rtl::OUString str, ConnectionSettings *settings) {
    OSL_ENSURE(settings, "pgsql-sdbc: OUStringToOString got NULL settings");
    return rtl::OUStringToOString( str, settings->encoding );
}

rtl::OString OUStringToOString( const rtl::OUString str, ConnectionSettings *settings) {
    return iOUStringToOString( str, settings );
}

void bufferEscapeConstant( rtl::OUStringBuffer & buf, const rtl::OUString & value, ConnectionSettings *settings )
{

    rtl::OString y = iOUStringToOString( value, settings );
    rtl::OStringBuffer strbuf( y.getLength() * 2 + 2 );
    int error;
    int len = PQescapeStringConn(settings->pConnection, ((char*)strbuf.getStr()), y.getStr() , y.getLength(), &error );
    if ( error )
    {
        char *errstr = PQerrorMessage(settings->pConnection);
        // As of PostgreSQL 9.1, the only possible errors "involve invalid multibyte encoding"
        // According to https://www2.opengroup.org/ogsys/jsp/publications/PublicationDetails.jsp?publicationid=11216
        // (X/Open SQL CLI, March 1995, ISBN: 1-85912-081-4, X/Open Document Number: C451)
        // 22018 is for "Invalid character value" and seems to be the best match.
        // We have no good XInterface Reference to pass here, so just give NULL
        throw SQLException(OUString(errstr, strlen(errstr), settings->encoding),
                           NULL,
                           OUString("22018"),
                           -1,
                           Any());
    }
    strbuf.setLength( len );
    // Previously here RTL_TEXTENCODING_ASCII_US; as we set the PostgreSQL client_encoding to UTF8,
    // we get UTF8 here, too. I'm not sure why it worked well before...
    buf.append( rtl::OStringToOUString( strbuf.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ) );
}

static inline void ibufferQuoteConstant( rtl::OUStringBuffer & buf, const rtl::OUString & value, ConnectionSettings *settings )
{
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "'" ) );
    bufferEscapeConstant( buf, value, settings );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "'" ) );
}

void bufferQuoteConstant( rtl::OUStringBuffer & buf, const rtl::OUString & value, ConnectionSettings *settings )
{
    return ibufferQuoteConstant( buf, value, settings );
}

void bufferQuoteAnyConstant( rtl::OUStringBuffer & buf, const Any &val, ConnectionSettings *settings )
{
    if( val.hasValue() )
    {
        OUString str;
        val >>= str;
        bufferQuoteConstant( buf, str, settings );
    }
    else
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "NULL" ) );
}

static inline void ibufferQuoteIdentifier( rtl::OUStringBuffer & buf, const rtl::OUString &toQuote, ConnectionSettings *settings )
{
    OSL_ENSURE(settings, "pgsql-sdbc: bufferQuoteIdentifier got NULL settings");

    rtl::OString y = iOUStringToOString( toQuote, settings );
    char *cstr = PQescapeIdentifier(settings->pConnection, y.getStr(), y.getLength());
    if ( cstr == NULL )
    {
        char *errstr = PQerrorMessage(settings->pConnection);
        // Implementation-defined SQLACCESS error
        throw SQLException(OUString(errstr, strlen(errstr), settings->encoding),
                           NULL,
                           OUString("22018"),
                           -1,
                           Any());
    }
    buf.append( rtl::OStringToOUString( cstr, RTL_TEXTENCODING_UTF8 ) );
    PQfreemem( cstr );
}

void bufferQuoteIdentifier( rtl::OUStringBuffer & buf, const rtl::OUString &toQuote, ConnectionSettings *settings )
{
    return ibufferQuoteIdentifier(buf, toQuote, settings);
}


void bufferQuoteQualifiedIdentifier(
    rtl::OUStringBuffer & buf, const rtl::OUString &schema, const rtl::OUString &table, ConnectionSettings *settings )
{
    ibufferQuoteIdentifier(buf, schema, settings);
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "." ) );
    ibufferQuoteIdentifier(buf, table, settings);
}

void bufferQuoteQualifiedIdentifier(
    rtl::OUStringBuffer & buf,
    const rtl::OUString &schema,
    const rtl::OUString &table,
    const rtl::OUString &col,
    ConnectionSettings *settings)
{
    ibufferQuoteIdentifier(buf, schema, settings);
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "." ) );
    ibufferQuoteIdentifier(buf, table, settings);
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "." ) );
    ibufferQuoteIdentifier(buf, col, settings);
}


rtl::OUString extractStringProperty(
    const Reference< XPropertySet > & descriptor, const rtl::OUString &name )
{
    rtl::OUString value;
    descriptor->getPropertyValue( name ) >>= value;
    return value;
}

sal_Bool extractBoolProperty(
    const Reference< XPropertySet > & descriptor, const rtl::OUString &name )
{
    sal_Bool value = sal_False;
    descriptor->getPropertyValue( name ) >>= value;
    return value;
}

sal_Int32 extractIntProperty(
    const Reference< XPropertySet > & descriptor, const rtl::OUString &name )
{
    sal_Int32 ret = 0;
    descriptor->getPropertyValue( name ) >>= ret;
    return ret;
}

void disposeObject( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & r )
{
    Reference< XComponent > comp( r, UNO_QUERY );
    if( comp.is() )
        comp->dispose();
}

void disposeNoThrow( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & r )
{
    try
    {
        disposeObject( r );
    }
    catch( SQLException & )
    {
        // ignore this
    }

}

Reference< XConnection > extractConnectionFromStatement( const Reference< XInterface > & stmt )
{
    Reference< XConnection > ret;

    Reference< com::sun::star::sdbc::XStatement > owner( stmt, UNO_QUERY );
    if( owner.is() )
        ret = owner->getConnection();
    else
    {
        Reference< com::sun::star::sdbc::XPreparedStatement > myowner( stmt, UNO_QUERY );
        if( myowner.is() )
            ret = myowner->getConnection();
        if( ! ret.is() )
            throw SQLException(
                "PQSDBC: Couldn't retrieve connection from statement",
                Reference< XInterface > () , rtl::OUString(), 0 , com::sun::star::uno::Any()  );
    }

    return ret;

}

DisposeGuard::DisposeGuard( const Reference< XInterface > & r )
    : d( r )
{}

DisposeGuard::~DisposeGuard()
{
    disposeNoThrow( d );
}

TransactionGuard::TransactionGuard( const Reference< XStatement > &stmt )
    : m_stmt( stmt ),
      m_commited( sal_False )
{
    m_stmt->executeUpdate( getStatics().BEGIN );
}

void TransactionGuard::commit()
{
    m_stmt->executeUpdate( getStatics().COMMIT );
    m_commited = sal_True;
}

void TransactionGuard::executeUpdate( const rtl::OUString & sql )
{
    m_stmt->executeUpdate( sql );
}

TransactionGuard::~TransactionGuard()
{
    try
    {
        if( ! m_commited )
            m_stmt->executeUpdate( getStatics().ROLLBACK );
    }
    catch( com::sun::star::uno::Exception & )
    {
        // ignore, we are within a dtor
    }

    disposeNoThrow( m_stmt );
}


bool isWhitespace( sal_Unicode c )
{
    return ' ' == c || 9 == c || 10 == c || 13 == c;
}

::rtl::OUString extractTableFromInsert( const rtl::OUString & sql )
{
    rtl::OUString ret;
    int i = 0;
    for( ; i < sql.getLength() && isWhitespace(sql[i])  ; i++ );

    if( 0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength(
            &sql.getStr()[i], sql.getLength() - i, "insert" , 6 ) )
    {
        i += 6;
        for( ; i < sql.getLength() && isWhitespace(sql[i]) ; i++ );
        if( 0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength(
            &sql.getStr()[i], sql.getLength() - i, "into" , 4 ) )
        {
            i +=4;
            for( ; i < sql.getLength() && isWhitespace(sql[i]) ; i++ );
            int start = i;
            bool quote = (sql[i] == '"');
            for( i++ ; i < sql.getLength() ; i ++ )
            {
                if( quote && sql[i] == '"' )
                {
                    for( i++ ; i < sql.getLength() && isWhitespace(sql[i]) ; i++ );
                    if( '.' == sql[i] )
                    {
                        for( i++ ; i < sql.getLength() && isWhitespace(sql[i]) ; i++ );
                        if( '"' == sql[i] )
                        {
                            // the second part of the table name does not use quotes
                            // parse on
                            quote = 0;
                        }
                    }
                    else
                    {
                        // end quoted name, ok
                        break;
                    }
                }
                else
                {
                    if( isWhitespace( sql[i] ) )
                    {
                        // found the end of an unquoted name
                        break;
                    }
                }
            }
            ret = rtl::OUString( &sql.getStr()[start], i - start ).trim();
//             printf( "pq_statement: parsed table name %s from insert\n" ,
//                     OUStringToOString( ret, RTL_TEXTENCODING_ASCII_US).getStr() );
        }
    }
    return ret;
}


static bool isOperator( char c )
{
    bool ret;
    switch(c)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '<':
    case '>':
    case '=':
    case '~':
    case '!':
    case '@':
    case '#':
    case '%':
    case '^':
    case '&':
    case '|':
    case '`':
    case '?':
    case '$':
        ret = true;
        break;
    default:
        ret = false;
    }
    return ret;
}

void splitSQL( const rtl::OString & sql, OStringVector &vec )
{
    int length = sql.getLength();

    int i = 0;
    bool singleQuote = false;
    bool doubleQuote = false;
    int start = 0;
    for( ; i < length ; i ++ )
    {
        char c = sql[i];
        if( doubleQuote  )
        {
            if( '"' == c )
            {
                vec.push_back( rtl::OString( &sql.getStr()[start], i-start+1  ) );
                start = i + 1;
                doubleQuote = false;
            }
        }
        else if( singleQuote )
        {
            if( '\'' == c && '\'' == sql[i+1] )
            {
                // two subsequent single quotes within a quoted string
                // mean a single quote within the string
                i ++;
            }
            else if( '\'' == c )
            {
                vec.push_back( rtl::OString( &sql.getStr()[start], i - start +1 ) );
                start = i + 1; // leave single quotes !
                singleQuote = false;
            }
        }
        else
        {
            if( '"' == c )
            {
                vec.push_back( rtl::OString( &sql.getStr()[start], i - start ) );
                doubleQuote = true;
                start = i;
            }
            else if( '\'' == c )
            {
                vec.push_back( rtl::OString( &sql.getStr()[start], i - start ) );
                singleQuote = true;
                start = i;
            }
        }
    }
    if( start < i )
        vec.push_back( rtl::OString( &sql.getStr()[start] , i - start ) );

//     for( i = 0 ; i < vec.size() ; i ++ )
//         printf( "%s!" , vec[i].getStr() );
//     printf( "\n" );

}

void tokenizeSQL( const rtl::OString & sql, OStringVector &vec  )
{
    int length = sql.getLength();

    int i = 0;
    bool singleQuote = false;
    bool doubleQuote = false;
    int start = 0;
    for( ; i < length ; i ++ )
    {
        char c = sql[i];
        if( doubleQuote  )
        {
            if( '"' == c )
            {
                vec.push_back( rtl::OString( &sql.getStr()[start], i-start  ) );
                start = i + 1;
                doubleQuote = false;
            }
        }
        else if( singleQuote )
        {
            if( '\'' == c )
            {
                vec.push_back( rtl::OString( &sql.getStr()[start], i - start +1 ) );
                start = i + 1; // leave single quotes !
                singleQuote = false;
            }
        }
        else
        {
            if( '"' == c )
            {
                doubleQuote = true;
                start = i +1; // skip double quotes !
            }
            else if( '\'' == c )
            {
                singleQuote = true;
                start = i; // leave single quotes
            }
            else if( isWhitespace( c ) )
            {
                if( i == start )
                    start ++;   // skip additional whitespace
                else
                {
                    vec.push_back( rtl::OString( &sql.getStr()[start], i - start  ) );
                    start = i +1;
                }
            }
            else if( ',' == c || isOperator( c ) || '(' == c || ')' == c )
            {
                if( i - start )
                    vec.push_back( rtl::OString( &sql.getStr()[start], i - start ) );
                vec.push_back( rtl::OString( &sql.getStr()[i], 1 ) );
                start = i + 1;
            }
            else if( '.' == c )
            {
                if( ( i > start && sql[start] >= '0' && sql[start] <= '9' ) ||
                    ( i == start && i > 1 && isWhitespace( sql[i-1] ) ) )
                {
                    // ignore, is a literal
                }
                else
                {
                    if( i - start )
                        vec.push_back( rtl::OString( &sql.getStr()[start], i - start ) );
                    vec.push_back( rtl::OString( RTL_CONSTASCII_STRINGPARAM( "." ) ) );
                    start = i + 1;
                }
            }
        }
    }
    if( start < i )
        vec.push_back( rtl::OString( &sql.getStr()[start] , i - start ) );

//     for( i = 0 ; i < vec.size() ; i ++ )
//         printf( "%s!" , vec[i].getStr() );
//     printf( "\n" );
}


void splitConcatenatedIdentifier( const rtl::OUString & source, rtl::OUString *first, rtl::OUString *second)
{
    OStringVector vec;
    tokenizeSQL( rtl::OUStringToOString( source, RTL_TEXTENCODING_UTF8 ), vec );
    if( vec.size() == 3 )
    {
        *first = rtl::OStringToOUString( vec[0] , RTL_TEXTENCODING_UTF8 );
        *second = rtl::OStringToOUString( vec[2], RTL_TEXTENCODING_UTF8 );
    }
}

typedef std::vector< sal_Int32 , Allocator< sal_Int32 > > IntVector;


rtl::OUString array2String( const com::sun::star::uno::Sequence< Any > &seq )
{
    OUStringBuffer buf(128);
    int len = seq.getLength();
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "{" ) );
    for( int i = 0 ; i < len ; i ++ )
    {
        OUString element;
        seq[i] >>= element;

        if( i > 0 )
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(",") );
        int strLength = element.getLength();
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "\"") );
        for( int j = 0 ; j < strLength ; j ++ )
        {
            sal_Unicode c = element[j];
            if( c == '\\' || c == '"' || c == '{' || c == '}' )
            {
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "\\" ) );
            }
            buf.append( c );
        }
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "\"" ) );
    }
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "}" ) );
    return buf.makeStringAndClear();
}

typedef
std::vector
<
    com::sun::star::uno::Any,
    Allocator< com::sun::star::uno::Any >
> AnyVector;

com::sun::star::uno::Sequence< Any > parseArray( const rtl::OUString & str ) throw( SQLException )
{
    com::sun::star::uno::Sequence< Any > ret;

    int len = str.getLength();
    bool doubleQuote = false;
    int brackets = 0;
    int i = 0;

    OUStringBuffer current;
    AnyVector elements;
    bool doubleQuotedValue = false;
    while( i < len )
    {
        sal_Unicode c = str[i];
        sal_Unicode cnext = str[i+1];
        if( doubleQuote )
        {
            if( '\\' == c )
            {
                i ++;
                current.append( cnext );
            }
            else if( '"' == c )
            {
                doubleQuote = false;
                doubleQuotedValue = true; // signal, that there was an empty element
            }
            else
            {
                current.append( c );
            }
        }
        else if ( '{' == c )
        {
            brackets ++;
        }
        else if( '}' == c )
        {
            brackets --;
            if( brackets < 0 )
            {

                OUStringBuffer buf;
                buf.appendAscii( "error during array parsing, didn't expect a } at position " );
                buf.append( (sal_Int32) i );
                buf.appendAscii( " ('" );
                buf.append( str );
                buf.appendAscii( "')" );
                throw SQLException(
                    buf.makeStringAndClear(),
                    Reference< XInterface > (), rtl::OUString(), 1, Any() );
            }
            if( brackets == 0 )
            {
                if( current.getLength() > 0 || doubleQuotedValue )
                    elements.push_back( makeAny( current.makeStringAndClear() ) );
            }
            else
            {
                current.append( c );
            }
        }
        else if( '"' == c )
        {
//                 if( current.getLength() != 0 )
//                 {
//                     OUStringBuffer buf;
//                     buf.appendAscii( "error during array parsing, didn't expect a \" at position " );
//                     buf.append( i );
//                     buf.append( " ('" );
//                     buf.append( str );
//                     buf.append( "')" );
//                     throw SDBCException(
//                         buf.makeStringAndClear(),
//                         Reference< XInterface > (), 1, Any() );
//                 }
//                 else
//                 {
            doubleQuote = true;
//                 }
        }
        else if( ',' == c && brackets == 1)
        {
            doubleQuotedValue = false;
            elements.push_back( makeAny( current.makeStringAndClear() ) );
        }
        else if( isWhitespace( c ) )
        {
            // ignore whitespace without quotes
        }
        else
        {
            current.append( c );
        }
        i++;
    }
    return sequence_of_vector(elements);
}

com::sun::star::uno::Sequence< sal_Int32 > parseIntArray( const ::rtl::OUString & str )
{
    sal_Int32 start = 0;
    IntVector vec;
//     printf( ">%s<\n" , OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
    for( sal_Int32 i = str.indexOf( ' ' ) ; i != -1 ; i = str.indexOf( ' ', start) )
    {
        vec.push_back( (sal_Int32)rtl_ustr_toInt32( &str.pData->buffer[start], 10 ) );
//         printf( "found %d\n" , rtl_ustr_toInt32( &str.pData->buffer[start], 10 ));
        start = i + 1;
    }
    vec.push_back( (sal_Int32)rtl_ustr_toInt32( &str.pData->buffer[start], 10 ) );
//     printf( "found %d\n" , rtl_ustr_toInt32( &str.pData->buffer[start], 10 ));
    return sequence_of_vector(vec);
}

void fillAttnum2attnameMap(
    Int2StringMap &map,
    const Reference< com::sun::star::sdbc::XConnection > &conn,
    const rtl::OUString &schema,
    const rtl::OUString &table )
{
    Reference< XPreparedStatement > prep = conn->prepareStatement(
                   "SELECT attname,attnum "
                   "FROM pg_attribute "
                         "INNER JOIN pg_class ON attrelid = pg_class.oid "
                         "INNER JOIN pg_namespace ON pg_class.relnamespace = pg_namespace.oid "
                   "WHERE relname=? AND nspname=?" );

    Reference< XParameters > paras( prep, UNO_QUERY_THROW );
    paras->setString( 1 , table );
    paras->setString( 2 , schema );
    Reference< XResultSet > rs = prep->executeQuery();

    Reference< XRow > xRow( rs , UNO_QUERY_THROW );
    while( rs->next() )
    {
        map[ xRow->getInt(2) ] = xRow->getString(1);
    }
}

::rtl::OString extractSingleTableFromSelect( const OStringVector &vec )
{
    rtl::OString ret;

    if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            vec[0].pData->buffer, vec[0].pData->length, "select" , 6 , 6 ) )
    {
        size_t token = 0;

        for( token = 1; token < vec.size() ; token ++ )
        {
            if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                    vec[token].getStr(), vec[token].getLength(), "from" , 4 , 4 ) )
            {
                // found from
                break;
            }
        }
        token ++;

        if( token < vec.size() && 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                vec[token].pData->buffer, vec[token].pData->length, "only " , 4 , 4 ) )
        {
            token ++;
        }

        if( token < vec.size() && rtl_str_compare_WithLength(
                vec[token].getStr(), vec[token].getLength(),
                RTL_CONSTASCII_STRINGPARAM("(") ) )
        {
            // it is a table or a function name
            rtl::OStringBuffer buf(128);
            if( '"' == vec[token][0] )
                buf.append( &(vec[token].getStr()[1]) , vec[token].getLength() -2 );
            else
                buf.append( vec[token] );
            token ++;

            if( token < vec.size() )
            {
                if( rtl_str_compare_WithLength(
                        vec[token].getStr(), vec[token].getLength(),
                        RTL_CONSTASCII_STRINGPARAM( "." ) ) == 0 )
                {
                    buf.append( vec[token] );
                    token ++;
                    if( token < vec.size() )
                    {
                        if( '"' == vec[token][0] )
                            buf.append( &(vec[token].getStr()[1]) , vec[token].getLength() -2 );
                        else
                            buf.append( vec[token] );
                        token ++;
                    }
                }
            }

            ret = buf.makeStringAndClear();
            // now got my table candidate

            if( token < vec.size() && rtl_str_compare_WithLength(
                    vec[token].getStr(), vec[token].getLength(),
                    RTL_CONSTASCII_STRINGPARAM( "(" ) ) == 0 )
            {
                // whoops, it is a function
                ret = rtl::OString();
            }
            else
            {
                if( token < vec.size() )
                {
                    if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                            vec[token].pData->buffer, vec[token].pData->length, "as" , 2, 2 ) )
                    {
                        token += 2; // skip alias
                    }
                }

                if( token < vec.size() )
                {
                    if( rtl_str_compare_WithLength(
                            vec[token].getStr(), vec[token].getLength(),
                            RTL_CONSTASCII_STRINGPARAM( "," ) ) == 0 )
                    {
                        // whoops, multiple tables are used
                        ret = rtl::OString();
                    }
                    else
                    {
                        static const char * forbiddenKeywords[] =
                            { "join", "natural", "outer", "inner", "left", "right", "full" , 0 };
                        for( int i = 0 ; forbiddenKeywords[i] ; i ++ )
                        {
                            if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                                 vec[token].pData->buffer, vec[token].pData->length,
                                 forbiddenKeywords[i], strlen(forbiddenKeywords[i]),
                                 strlen(forbiddenKeywords[i]) ) )
                            {
                                // whoops, it is a join
                                ret = rtl::OString();
                            }
                        }
                    }
                }
            }
        }
    }
    return ret;

}

com::sun::star::uno::Sequence< sal_Int32 > string2intarray( const ::rtl::OUString & str )
{
    com::sun::star::uno::Sequence< sal_Int32 > ret;
    const sal_Int32 strlen = str.getLength();
    if( str.getLength() > 1 )
    {
        sal_Int32 start = 0;
        while ( iswspace( str.iterateCodePoints(&start) ) )
            if ( start == strlen)
                return ret;
        if ( str.iterateCodePoints(&start) != L'{' )
            return ret;
        while ( iswspace( str.iterateCodePoints(&start) ) )
            if ( start == strlen)
                return ret;
        if ( str.iterateCodePoints(&start, 0) == L'}' )
            return ret;

        std::vector< sal_Int32, Allocator< sal_Int32 > > vec;
        do
        {
            ::rtl::OUString digits;
            sal_Int32 c;
            while ( isdigit( c = str.iterateCodePoints(&start) ) )
            {
                if ( start == strlen)
                    return ret;
                digits += OUString(c);
            }
            vec.push_back( digits.toInt32() );
            while ( iswspace( str.iterateCodePoints(&start) ) )
                if ( start == strlen)
                    return ret;
            if ( str.iterateCodePoints(&start, 0) == L'}' )
                break;
            if ( str.iterateCodePoints(&start) != L',' )
                return ret;
            if ( start == strlen)
                return ret;
        } while( true );
        // vec is guaranteed non-empty
        ret = com::sun::star::uno::Sequence< sal_Int32 > ( &vec[0] , vec.size() );
    }
    return ret;
}


Sequence< rtl::OUString > convertMappedIntArray2StringArray(
    const Int2StringMap &map, const Sequence< sal_Int32 > &intArray )
{
    Sequence< ::rtl::OUString > ret( intArray.getLength() );
    for( int i = 0; i < intArray.getLength() ; i ++ )
    {
        Int2StringMap::const_iterator ii = map.find( intArray[i] );
        if( ii != map.end() )
            ret[i] = ii->second;
    }
    return ret;
}


::rtl::OUString sqltype2string( const Reference< XPropertySet > & desc )
{
    OUStringBuffer typeName;
    typeName.append( extractStringProperty( desc, getStatics().TYPE_NAME ) );
    sal_Int32 precision = extractIntProperty( desc, getStatics().PRECISION );

    if( precision )
    {
        switch( extractIntProperty( desc, getStatics().TYPE ) )
        {
        case com::sun::star::sdbc::DataType::VARBINARY:
        case com::sun::star::sdbc::DataType::VARCHAR:
        case com::sun::star::sdbc::DataType::CHAR:
        {
            typeName.appendAscii( RTL_CONSTASCII_STRINGPARAM( "(" ) );
            typeName.append( precision );
            typeName.appendAscii( RTL_CONSTASCII_STRINGPARAM( ")" ) );
            break;
        }
        case com::sun::star::sdbc::DataType::DECIMAL:
        case com::sun::star::sdbc::DataType::NUMERIC:
        {
            typeName.appendAscii( RTL_CONSTASCII_STRINGPARAM( "(" ) );
            typeName.append( precision );
            typeName.appendAscii( RTL_CONSTASCII_STRINGPARAM( "," ) );
            typeName.append( extractIntProperty( desc, getStatics().SCALE ) );
            typeName.appendAscii( RTL_CONSTASCII_STRINGPARAM( ")" ) );
            break;
        }
        default:
            ((void)0);
        }
    }
    return typeName.makeStringAndClear();
}




static void keyType2String( OUStringBuffer & buf, sal_Int32 keyType )
{
    if( com::sun::star::sdbc::KeyRule::CASCADE == keyType )
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "CASCADE " ) );
    }
    else if( com::sun::star::sdbc::KeyRule::RESTRICT  == keyType )
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "RESTRICT " ) );
    }
    else if( com::sun::star::sdbc::KeyRule::SET_DEFAULT  == keyType )
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "SET DEFAULT " ) );
    }
    else if( com::sun::star::sdbc::KeyRule::SET_NULL  == keyType )
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "SET NULL " ) );
    }
    else //if( com::sun::star::sdbc::KeyRule::NO_ACTION == keyType )
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "NO ACTION " ) );
    }
}

void bufferKey2TableConstraint(
    OUStringBuffer &buf, const Reference< XPropertySet > &key, ConnectionSettings *settings )
{
    Statics &st = getStatics();
    sal_Int32 type = extractIntProperty( key, st.TYPE );
    OUString referencedTable = extractStringProperty( key, st.REFERENCED_TABLE );
    sal_Int32 updateRule = extractIntProperty( key, st.UPDATE_RULE );
    sal_Int32 deleteRule = extractIntProperty( key, st.DELETE_RULE );
    bool foreign = false;
    if( type == com::sun::star::sdbcx::KeyType::UNIQUE )
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "UNIQUE( " ) );
    }
    else if( type == com::sun::star::sdbcx::KeyType::PRIMARY )
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "PRIMARY KEY( " ) );
    }
    else if( type == com::sun::star::sdbcx::KeyType::FOREIGN )
    {
        foreign = true;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "FOREIGN KEY( " ) );
    }

    Reference< XColumnsSupplier > columns( key, UNO_QUERY );
    if( columns.is() )
    {
        Reference< XEnumerationAccess > colEnumAccess( columns->getColumns(), UNO_QUERY );
        if( colEnumAccess.is() )
        {
            Reference< XEnumeration > colEnum = colEnumAccess->createEnumeration();
            bool first = true;
            while(colEnum.is() && colEnum->hasMoreElements() )
            {
                if( first )
                {
                    first = false;
                }
                else
                {
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
                }
                Reference< XPropertySet > keyColumn( colEnum->nextElement(), UNO_QUERY_THROW );
                bufferQuoteIdentifier(buf, extractStringProperty( keyColumn, st.NAME ), settings );
            }
        }
    }
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ") " ));

    if( foreign )
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "REFERENCES " ) );
        OUString schema;
        OUString tableName;
        splitConcatenatedIdentifier( referencedTable, &schema, &tableName );
        bufferQuoteQualifiedIdentifier(buf , schema, tableName, settings );
        if(columns.is() )
        {
            Reference< XEnumerationAccess > colEnumAccess( columns->getColumns(), UNO_QUERY);
            if( colEnumAccess.is() )
            {
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) );
                Reference< XEnumeration > colEnum(colEnumAccess->createEnumeration());
                bool first = true;
                while(colEnum.is() && colEnum->hasMoreElements() )
                {
                    if( first )
                    {
                        first = false;
                    }
                    else
                    {
                        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
                    }
                    Reference< XPropertySet > keyColumn( colEnum->nextElement(), UNO_QUERY_THROW );
                    bufferQuoteIdentifier(
                                          buf, extractStringProperty( keyColumn, st.RELATED_COLUMN ), settings );
                }
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ") " ) );
            }
        }

        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "ON DELETE " ) );
        keyType2String( buf, deleteRule );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " ON UPDATE " ) );
        keyType2String( buf, updateRule );
    }

}

static bool equalsIgnoreCase( const rtl::OString & str, const char *str2, int length2 )
{
    return 0 == rtl_str_compareIgnoreAsciiCase_WithLength(
        str.pData->buffer, str.pData->length, str2, length2 );
}

void extractNameValuePairsFromInsert( String2StringMap & map, const rtl::OString & lastQuery )
{
    OStringVector vec;
    tokenizeSQL( lastQuery, vec  );

    int nSize = vec.size();
//     printf( "1 %d\n", nSize );
    if( nSize > 6  &&
        equalsIgnoreCase( vec[0] , RTL_CONSTASCII_STRINGPARAM( "insert" ) ) &&
        equalsIgnoreCase( vec[1] , RTL_CONSTASCII_STRINGPARAM( "into" ) ) )
    {
        int n = 2;

//         printf( "1a\n" );
        // extract table name
        rtl::OString tableName;
        if( equalsIgnoreCase( vec[n+1], RTL_CONSTASCII_STRINGPARAM( "." ) ) )
        {
            tableName = vec[n];
            tableName += vec[n+1];
            tableName += vec[n+2];
            n +=2;
        }
        else
        {
            tableName = vec[n];
        }

        OStringVector names;
        n ++;
        if( equalsIgnoreCase( vec[n], RTL_CONSTASCII_STRINGPARAM( "(" ) ) )
        {
//             printf( "2\n" );
            // extract names
            n++;
            while( nSize > n && ! equalsIgnoreCase(vec[n] , RTL_CONSTASCII_STRINGPARAM( ")" ) ) )
            {
                names.push_back( vec[n] );
                if( nSize > n+1 && equalsIgnoreCase( vec[n+1] , RTL_CONSTASCII_STRINGPARAM( "," ) ) )
                {
                    n ++;
                }
                n++;
            }
            n++;

            // now read the values
            if( nSize > n +1 && equalsIgnoreCase( vec[n], RTL_CONSTASCII_STRINGPARAM("VALUES") ) &&
                equalsIgnoreCase(vec[n+1], RTL_CONSTASCII_STRINGPARAM( "(" ) ) )
            {
                n +=2;
//                 printf( "3\n" );
                for ( OStringVector::size_type i = 0 ; i < names.size() && nSize > n ; i ++ )
                {
                    map[names[i]] = vec[n];
                    if( nSize > n+1 && equalsIgnoreCase( vec[n+1] , RTL_CONSTASCII_STRINGPARAM(",") ) )
                    {
                        n ++;
                    }
                    n++;
                }
            }
        }
    }
}

rtl::OUString querySingleValue(
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &connection,
    const rtl::OUString &query )
{
    OUString ret;
    Reference< XStatement > stmt = connection->createStatement();
    DisposeGuard guard( stmt );
    Reference< XResultSet > rs = stmt->executeQuery( query );
    Reference< XRow > xRow( rs, UNO_QUERY );
    if( rs->next() )
        ret = xRow->getString( 1 );
    return ret;
}


// copied from connectivity/source/dbtools, can't use the function directly
bool implSetObject(	const Reference< XParameters >& _rxParameters,
                        const sal_Int32 _nColumnIndex, const Any& _rValue)
{
    sal_Bool bSuccessfullyReRouted = sal_True;
    switch (_rValue.getValueTypeClass())
    {
        case typelib_TypeClass_HYPER:
        {
            sal_Int64 nValue = 0;
            _rxParameters->setLong( _nColumnIndex, nValue );
        }
        break;

        case typelib_TypeClass_VOID:
            _rxParameters->setNull(_nColumnIndex,com::sun::star::sdbc::DataType::VARCHAR);
            break;

        case typelib_TypeClass_STRING:
            _rxParameters->setString(_nColumnIndex, *(rtl::OUString*)_rValue.getValue());
            break;

        case typelib_TypeClass_BOOLEAN:
            _rxParameters->setBoolean(_nColumnIndex, *(sal_Bool *)_rValue.getValue());
            break;

        case typelib_TypeClass_BYTE:
            _rxParameters->setByte(_nColumnIndex, *(sal_Int8 *)_rValue.getValue());
            break;

        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_SHORT:
            _rxParameters->setShort(_nColumnIndex, *(sal_Int16*)_rValue.getValue());
            break;

        case typelib_TypeClass_CHAR:
            _rxParameters->setString(_nColumnIndex, ::rtl::OUString((sal_Unicode *)_rValue.getValue(),1));
            break;

        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_LONG:
            _rxParameters->setInt(_nColumnIndex, *(sal_Int32*)_rValue.getValue());
            break;

        case typelib_TypeClass_FLOAT:
            _rxParameters->setFloat(_nColumnIndex, *(float*)_rValue.getValue());
            break;

        case typelib_TypeClass_DOUBLE:
            _rxParameters->setDouble(_nColumnIndex, *(double*)_rValue.getValue());
            break;

        case typelib_TypeClass_SEQUENCE:
            if (_rValue.getValueType() == ::getCppuType((const Sequence< sal_Int8 > *)0))
            {
                _rxParameters->setBytes(_nColumnIndex, *(Sequence<sal_Int8>*)_rValue.getValue());
            }
            else
                bSuccessfullyReRouted = sal_False;
            break;
        case typelib_TypeClass_STRUCT:
            if (_rValue.getValueType() == ::getCppuType((const com::sun::star::util::DateTime*)0))
                _rxParameters->setTimestamp(_nColumnIndex, *(com::sun::star::util::DateTime*)_rValue.getValue());
            else if (_rValue.getValueType() == ::getCppuType((const com::sun::star::util::Date*)0))
                _rxParameters->setDate(_nColumnIndex, *(com::sun::star::util::Date*)_rValue.getValue());
            else if (_rValue.getValueType() == ::getCppuType((const com::sun::star::util::Time*)0))
                _rxParameters->setTime(_nColumnIndex, *(com::sun::star::util::Time*)_rValue.getValue());
            else
                bSuccessfullyReRouted = sal_False;
            break;

        case typelib_TypeClass_INTERFACE:
        {
            Reference< com::sun::star::io::XInputStream >  xStream;
            if (_rValue >>= xStream)
            {
                _rValue >>= xStream;
                _rxParameters->setBinaryStream(_nColumnIndex, xStream, xStream->available());
                break;
            }
        }
            // run through
        default:
            bSuccessfullyReRouted = sal_False;

    }

    return bSuccessfullyReRouted;
}

}
