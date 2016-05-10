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

OUString concatQualified( const OUString & a, const OUString &b)
{
    OUStringBuffer buf( a.getLength() + 2 + b.getLength() );
    buf.append( a );
    buf.append( "." );
    buf.append( b );
    return buf.makeStringAndClear();
}

static inline OString iOUStringToOString( const OUString& str, ConnectionSettings *settings) {
    OSL_ENSURE(settings, "pgsql-sdbc: OUStringToOString got NULL settings");
    return OUStringToOString( str, settings->encoding );
}

OString OUStringToOString( const OUString& str, ConnectionSettings *settings) {
    return iOUStringToOString( str, settings );
}

void bufferEscapeConstant( OUStringBuffer & buf, const OUString & value, ConnectionSettings *settings )
{

    OString y = iOUStringToOString( value, settings );
    OStringBuffer strbuf( y.getLength() * 2 + 2 );
    int error;
    int len = PQescapeStringConn(settings->pConnection, const_cast<char*>(strbuf.getStr()), y.getStr() , y.getLength(), &error );
    if ( error )
    {
        char *errstr = PQerrorMessage(settings->pConnection);
        // As of PostgreSQL 9.1, the only possible errors "involve invalid multibyte encoding"
        // According to https://www2.opengroup.org/ogsys/jsp/publications/PublicationDetails.jsp?publicationid=11216
        // (X/Open SQL CLI, March 1995, ISBN: 1-85912-081-4, X/Open Document Number: C451)
        // 22018 is for "Invalid character value" and seems to be the best match.
        // We have no good XInterface Reference to pass here, so just give NULL
        throw SQLException(OUString(errstr, strlen(errstr), settings->encoding),
                           nullptr,
                           OUString("22018"),
                           -1,
                           Any());
    }
    strbuf.setLength( len );
    // Previously here RTL_TEXTENCODING_ASCII_US; as we set the PostgreSQL client_encoding to UTF8,
    // we get UTF8 here, too. I'm not sure why it worked well before...
    buf.append( OStringToOUString( strbuf.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ) );
}

static inline void ibufferQuoteConstant( OUStringBuffer & buf, const OUString & value, ConnectionSettings *settings )
{
    buf.append( "'" );
    bufferEscapeConstant( buf, value, settings );
    buf.append( "'" );
}

void bufferQuoteConstant( OUStringBuffer & buf, const OUString & value, ConnectionSettings *settings )
{
    return ibufferQuoteConstant( buf, value, settings );
}

void bufferQuoteAnyConstant( OUStringBuffer & buf, const Any &val, ConnectionSettings *settings )
{
    if( val.hasValue() )
    {
        OUString str;
        val >>= str;
        bufferQuoteConstant( buf, str, settings );
    }
    else
        buf.append( "NULL" );
}

static inline void ibufferQuoteIdentifier( OUStringBuffer & buf, const OUString &toQuote, ConnectionSettings *settings )
{
    OSL_ENSURE(settings, "pgsql-sdbc: bufferQuoteIdentifier got NULL settings");

    OString y = iOUStringToOString( toQuote, settings );
    char *cstr = PQescapeIdentifier(settings->pConnection, y.getStr(), y.getLength());
    if ( cstr == nullptr )
    {
        char *errstr = PQerrorMessage(settings->pConnection);
        // Implementation-defined SQLACCESS error
        throw SQLException(OUString(errstr, strlen(errstr), settings->encoding),
                           nullptr,
                           OUString("22018"),
                           -1,
                           Any());
    }
    buf.append( OStringToOUString( cstr, RTL_TEXTENCODING_UTF8 ) );
    PQfreemem( cstr );
}

void bufferQuoteIdentifier( OUStringBuffer & buf, const OUString &toQuote, ConnectionSettings *settings )
{
    return ibufferQuoteIdentifier(buf, toQuote, settings);
}


void bufferQuoteQualifiedIdentifier(
    OUStringBuffer & buf, const OUString &schema, const OUString &table, ConnectionSettings *settings )
{
    ibufferQuoteIdentifier(buf, schema, settings);
    buf.append( "." );
    ibufferQuoteIdentifier(buf, table, settings);
}

void bufferQuoteQualifiedIdentifier(
    OUStringBuffer & buf,
    const OUString &schema,
    const OUString &table,
    const OUString &col,
    ConnectionSettings *settings)
{
    ibufferQuoteIdentifier(buf, schema, settings);
    buf.append( "." );
    ibufferQuoteIdentifier(buf, table, settings);
    buf.append( "." );
    ibufferQuoteIdentifier(buf, col, settings);
}


OUString extractStringProperty(
    const Reference< XPropertySet > & descriptor, const OUString &name )
{
    OUString value;
    descriptor->getPropertyValue( name ) >>= value;
    return value;
}

bool extractBoolProperty(
    const Reference< XPropertySet > & descriptor, const OUString &name )
{
    bool value = false;
    descriptor->getPropertyValue( name ) >>= value;
    return value;
}

sal_Int32 extractIntProperty(
    const Reference< XPropertySet > & descriptor, const OUString &name )
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
                Reference< XInterface > () , OUString(), 0 , com::sun::star::uno::Any()  );
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
      m_commited( false )
{
    m_stmt->executeUpdate( getStatics().BEGIN );
}

void TransactionGuard::commit()
{
    m_stmt->executeUpdate( getStatics().COMMIT );
    m_commited = true;
}

void TransactionGuard::executeUpdate( const OUString & sql )
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

OUString extractTableFromInsert( const OUString & sql )
{
    OUString ret;
    int i = 0;
    while (i < sql.getLength() && isWhitespace(sql[i])) { i++; }

    if( sql.matchIgnoreAsciiCase("insert", i) )
    {
        i += 6;
        while (i < sql.getLength() && isWhitespace(sql[i])) { i++; }
        if( sql.matchIgnoreAsciiCase("into", i) )
        {
            i +=4;
            while (i < sql.getLength() && isWhitespace(sql[i])) { i++; }
            int start = i;
            bool quote = (sql[i] == '"');
            for( i++ ; i < sql.getLength() ; i ++ )
            {
                if( quote && sql[i] == '"' )
                {
                    while (i < sql.getLength() && isWhitespace(sql[i])) { i++; }
                    if( '.' == sql[i] )
                    {
                        while (i < sql.getLength() && isWhitespace(sql[i])) { i++; }
                        if( '"' == sql[i] )
                        {
                            // the second part of the table name does not use quotes
                            // parse on
                            quote = false;
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
            ret = sql.copy(start, i - start ).trim();
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

void splitSQL( const OString & sql, OStringVector &vec )
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
                vec.push_back( OString( &sql.getStr()[start], i-start+1  ) );
                start = i + 1;
                doubleQuote = false;
            }
        }
        else if( singleQuote )
        {
            if( '\'' == c && (i+1) < length && '\'' == sql[i+1] )
            {
                // two subsequent single quotes within a quoted string
                // mean a single quote within the string
                i ++;
            }
            else if( '\'' == c )
            {
                vec.push_back( OString( &sql.getStr()[start], i - start +1 ) );
                start = i + 1; // leave single quotes !
                singleQuote = false;
            }
        }
        else
        {
            if( '"' == c )
            {
                vec.push_back( OString( &sql.getStr()[start], i - start ) );
                doubleQuote = true;
                start = i;
            }
            else if( '\'' == c )
            {
                vec.push_back( OString( &sql.getStr()[start], i - start ) );
                singleQuote = true;
                start = i;
            }
        }
    }
    if( start < i )
        vec.push_back( OString( &sql.getStr()[start] , i - start ) );

//     for( i = 0 ; i < vec.size() ; i ++ )
//         printf( "%s!" , vec[i].getStr() );
//     printf( "\n" );

}

void tokenizeSQL( const OString & sql, OStringVector &vec  )
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
                vec.push_back( OString( &sql.getStr()[start], i-start  ) );
                start = i + 1;
                doubleQuote = false;
            }
        }
        else if( singleQuote )
        {
            if( '\'' == c )
            {
                vec.push_back( OString( &sql.getStr()[start], i - start +1 ) );
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
                    vec.push_back( OString( &sql.getStr()[start], i - start  ) );
                    start = i +1;
                }
            }
            else if( ',' == c || isOperator( c ) || '(' == c || ')' == c )
            {
                if( i - start )
                    vec.push_back( OString( &sql.getStr()[start], i - start ) );
                vec.push_back( OString( &sql.getStr()[i], 1 ) );
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
                        vec.push_back( OString( &sql.getStr()[start], i - start ) );
                    vec.push_back( OString( "." ) );
                    start = i + 1;
                }
            }
        }
    }
    if( start < i )
        vec.push_back( OString( &sql.getStr()[start] , i - start ) );

//     for( i = 0 ; i < vec.size() ; i ++ )
//         printf( "%s!" , vec[i].getStr() );
//     printf( "\n" );
}


void splitConcatenatedIdentifier( const OUString & source, OUString *first, OUString *second)
{
    OStringVector vec;
    tokenizeSQL( OUStringToOString( source, RTL_TEXTENCODING_UTF8 ), vec );
    switch (vec.size())
    {
    case 1:
        first->clear();
        *second = OStringToOUString( vec[0], RTL_TEXTENCODING_UTF8 );
        break;
    case 3:
        *first  = OStringToOUString( vec[0], RTL_TEXTENCODING_UTF8 );
        *second = OStringToOUString( vec[2], RTL_TEXTENCODING_UTF8 );
        break;
    default:
         SAL_WARN("connectivity.postgresql",
                  "pq_tools::splitConcatenatedIdentifier unexpected number of tokens in identifier: "
                  << vec.size());
    }
}

typedef std::vector< sal_Int32 > IntVector;


OUString array2String( const com::sun::star::uno::Sequence< Any > &seq )
{
    OUStringBuffer buf(128);
    int len = seq.getLength();
    buf.append( "{" );
    for( int i = 0 ; i < len ; i ++ )
    {
        OUString element;
        seq[i] >>= element;

        if( i > 0 )
            buf.append( "," );
        int strLength = element.getLength();
        buf.append( "\"" );
        for( int j = 0 ; j < strLength ; j ++ )
        {
            sal_Unicode c = element[j];
            if( c == '\\' || c == '"' || c == '{' || c == '}' )
            {
                buf.append( "\\" );
            }
            buf.append( c );
        }
        buf.append( "\"" );
    }
    buf.append( "}" );
    return buf.makeStringAndClear();
}

typedef
std::vector
<
    com::sun::star::uno::Any
> AnyVector;

com::sun::star::uno::Sequence< Any > parseArray( const OUString & str ) throw( SQLException )
{
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
                buf.append( "error during array parsing, didn't expect a } at position " );
                buf.append( (sal_Int32) i );
                buf.append( " ('" );
                buf.append( str );
                buf.append( "')" );
                throw SQLException(
                    buf.makeStringAndClear(),
                    Reference< XInterface > (), OUString(), 1, Any() );
            }
            if( brackets == 0 )
            {
                if( !current.isEmpty() || doubleQuotedValue )
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

com::sun::star::uno::Sequence< sal_Int32 > parseIntArray( const OUString & str )
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
    const OUString &schema,
    const OUString &table )
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

OString extractSingleTableFromSelect( const OStringVector &vec )
{
    OString ret;

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

        if( token < vec.size() && vec[token] != "(" )
        {
            // it is a table or a function name
            OStringBuffer buf(128);
            if( '"' == vec[token][0] )
                buf.append( &(vec[token].getStr()[1]) , vec[token].getLength() -2 );
            else
                buf.append( vec[token] );
            token ++;

            if( token < vec.size() )
            {
                if( vec[token] == "." )
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

            if( token < vec.size() && vec[token] == "(" )
            {
                // whoops, it is a function
                ret.clear();
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
                    if( vec[token] == "," )
                    {
                        // whoops, multiple tables are used
                        ret.clear();
                    }
                    else
                    {
                        static const char * forbiddenKeywords[] =
                            { "join", "natural", "outer", "inner", "left", "right", "full" , nullptr };
                        for( int i = 0 ; forbiddenKeywords[i] ; i ++ )
                        {
                            size_t nKeywordLen = strlen(forbiddenKeywords[i]);
                            if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                                 vec[token].pData->buffer, vec[token].pData->length,
                                 forbiddenKeywords[i], nKeywordLen,
                                 nKeywordLen ) )
                            {
                                // whoops, it is a join
                                ret.clear();
                            }
                        }
                    }
                }
            }
        }
    }
    return ret;

}

com::sun::star::uno::Sequence< sal_Int32 > string2intarray( const OUString & str )
{
    com::sun::star::uno::Sequence< sal_Int32 > ret;
    const sal_Int32 strlen = str.getLength();
    if( str.getLength() > 1 )
    {
        sal_Int32 start = 0;
        sal_uInt32 c;
        while ( iswspace( (c=str.iterateCodePoints(&start)) ) )
            if ( start == strlen)
                return ret;
        if ( c != L'{' )
            return ret;
        while ( iswspace( c=str.iterateCodePoints(&start) ) )
            if ( start == strlen)
                return ret;
        if ( c == L'}' )
            return ret;

        std::vector< sal_Int32 > vec;
        do
        {
            OUString digits;
            do
            {
                if(!iswspace(c))
                    break;
                if ( start == strlen)
                    return ret;
            } while ( (c=str.iterateCodePoints(&start)) );
            do
            {
                if (!iswdigit(c))
                    break;
                if ( start == strlen)
                    return ret;
                digits += OUString(&c, 1);
            } while ( (c = str.iterateCodePoints(&start)) );
            vec.push_back( digits.toInt32() );
            do
            {
                if(!iswspace(c))
                    break;
                if ( start == strlen)
                    return ret;
            } while ( (c=str.iterateCodePoints(&start)) );
            if ( c == L'}' )
                break;
            if ( str.iterateCodePoints(&start) != L',' )
                return ret;
            if ( start == strlen)
                return ret;
        } while( true );
        // vec is guaranteed non-empty
        assert(vec.size() > 0);
        ret = com::sun::star::uno::Sequence< sal_Int32 > ( &vec[0] , vec.size() );
    }
    return ret;
}


Sequence< OUString > convertMappedIntArray2StringArray(
    const Int2StringMap &map, const Sequence< sal_Int32 > &intArray )
{
    Sequence< OUString > ret( intArray.getLength() );
    for( int i = 0; i < intArray.getLength() ; i ++ )
    {
        Int2StringMap::const_iterator ii = map.find( intArray[i] );
        if( ii != map.end() )
            ret[i] = ii->second;
    }
    return ret;
}


OUString sqltype2string( const Reference< XPropertySet > & desc )
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
            typeName.append( "(" );
            typeName.append( precision );
            typeName.append( ")" );
            break;
        }
        case com::sun::star::sdbc::DataType::DECIMAL:
        case com::sun::star::sdbc::DataType::NUMERIC:
        {
            typeName.append( "(" );
            typeName.append( precision );
            typeName.append( "," );
            typeName.append( extractIntProperty( desc, getStatics().SCALE ) );
            typeName.append( ")" );
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
        buf.append( "CASCADE " );
    }
    else if( com::sun::star::sdbc::KeyRule::RESTRICT  == keyType )
    {
        buf.append( "RESTRICT " );
    }
    else if( com::sun::star::sdbc::KeyRule::SET_DEFAULT  == keyType )
    {
        buf.append( "SET DEFAULT " );
    }
    else if( com::sun::star::sdbc::KeyRule::SET_NULL  == keyType )
    {
        buf.append( "SET NULL " );
    }
    else //if( com::sun::star::sdbc::KeyRule::NO_ACTION == keyType )
    {
        buf.append( "NO ACTION " );
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
        buf.append( "UNIQUE( " );
    }
    else if( type == com::sun::star::sdbcx::KeyType::PRIMARY )
    {
        buf.append( "PRIMARY KEY( " );
    }
    else if( type == com::sun::star::sdbcx::KeyType::FOREIGN )
    {
        foreign = true;
        buf.append( "FOREIGN KEY( " );
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
                    buf.append( ", " );
                }
                Reference< XPropertySet > keyColumn( colEnum->nextElement(), UNO_QUERY_THROW );
                bufferQuoteIdentifier(buf, extractStringProperty( keyColumn, st.NAME ), settings );
            }
        }
    }
    buf.append( ") " );

    if( foreign )
    {
        buf.append( "REFERENCES " );
        OUString schema;
        OUString tableName;
        splitConcatenatedIdentifier( referencedTable, &schema, &tableName );
        bufferQuoteQualifiedIdentifier(buf , schema, tableName, settings );
        if(columns.is() )
        {
            Reference< XEnumerationAccess > colEnumAccess( columns->getColumns(), UNO_QUERY);
            if( colEnumAccess.is() )
            {
                buf.append( " (" );
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
                        buf.append( ", " );
                    }
                    Reference< XPropertySet > keyColumn( colEnum->nextElement(), UNO_QUERY_THROW );
                    bufferQuoteIdentifier(
                                          buf, extractStringProperty( keyColumn, st.RELATED_COLUMN ), settings );
                }
                buf.append( ") " );
            }
        }

        buf.append( "ON DELETE " );
        keyType2String( buf, deleteRule );
        buf.append( " ON UPDATE " );
        keyType2String( buf, updateRule );
    }

}

void extractNameValuePairsFromInsert( String2StringMap & map, const OString & lastQuery )
{
    OStringVector vec;
    tokenizeSQL( lastQuery, vec  );

    int nSize = vec.size();
//     printf( "1 %d\n", nSize );
    if( nSize > 6  &&
        vec[0].equalsIgnoreAsciiCase( "insert" ) &&
        vec[1].equalsIgnoreAsciiCase( "into" ) )
    {
        int n = 2;

//         printf( "1a\n" );
        // extract table name
        OString tableName;
        if( vec[n+1].equalsIgnoreAsciiCase( "." ) )
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
        if( vec[n].equalsIgnoreAsciiCase( "(" ) )
        {
//             printf( "2\n" );
            // extract names
            n++;
            while( nSize > n && ! vec[n].equalsIgnoreAsciiCase( ")" ) )
            {
                names.push_back( vec[n] );
                if( nSize > n+1 && vec[n+1].equalsIgnoreAsciiCase( "," ) )
                {
                    n ++;
                }
                n++;
            }
            n++;

            // now read the values
            if( nSize > n +1 && vec[n].equalsIgnoreAsciiCase("VALUES") &&
                vec[n+1].equalsIgnoreAsciiCase( "(" ) )
            {
                n +=2;
//                 printf( "3\n" );
                for ( OStringVector::size_type i = 0 ; i < names.size() && nSize > n ; i ++ )
                {
                    map[names[i]] = vec[n];
                    if( nSize > n+1 && vec[n+1].equalsIgnoreAsciiCase(",") )
                    {
                        n ++;
                    }
                    n++;
                }
            }
        }
    }
}

OUString querySingleValue(
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &connection,
    const OUString &query )
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
    bool bSuccessfullyReRouted = true;
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
            _rxParameters->setString(_nColumnIndex, *static_cast<OUString const *>(_rValue.getValue()));
            break;

        case typelib_TypeClass_BOOLEAN:
            _rxParameters->setBoolean(_nColumnIndex, *static_cast<sal_Bool const *>(_rValue.getValue()));
            break;

        case typelib_TypeClass_BYTE:
            _rxParameters->setByte(_nColumnIndex, *static_cast<sal_Int8 const *>(_rValue.getValue()));
            break;

        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_SHORT:
            _rxParameters->setShort(_nColumnIndex, *static_cast<sal_Int16 const *>(_rValue.getValue()));
            break;

        case typelib_TypeClass_CHAR:
            _rxParameters->setString(_nColumnIndex, OUString(static_cast<sal_Unicode const *>(_rValue.getValue()),1));
            break;

        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_LONG:
            _rxParameters->setInt(_nColumnIndex, *static_cast<sal_Int32 const *>(_rValue.getValue()));
            break;

        case typelib_TypeClass_FLOAT:
            _rxParameters->setFloat(_nColumnIndex, *static_cast<float const *>(_rValue.getValue()));
            break;

        case typelib_TypeClass_DOUBLE:
            _rxParameters->setDouble(_nColumnIndex, *static_cast<double const *>(_rValue.getValue()));
            break;

        case typelib_TypeClass_SEQUENCE:
            if (_rValue.getValueType() == cppu::UnoType<Sequence< sal_Int8 >>::get())
            {
                _rxParameters->setBytes(_nColumnIndex, *static_cast<Sequence<sal_Int8> const *>(_rValue.getValue()));
            }
            else
                bSuccessfullyReRouted = false;
            break;
        case typelib_TypeClass_STRUCT:
            if (_rValue.getValueType() == cppu::UnoType<com::sun::star::util::DateTime>::get())
                _rxParameters->setTimestamp(_nColumnIndex, *static_cast<com::sun::star::util::DateTime const *>(_rValue.getValue()));
            else if (_rValue.getValueType() == cppu::UnoType<com::sun::star::util::Date>::get())
                _rxParameters->setDate(_nColumnIndex, *static_cast<com::sun::star::util::Date const *>(_rValue.getValue()));
            else if (_rValue.getValueType() == cppu::UnoType<com::sun::star::util::Time>::get())
                _rxParameters->setTime(_nColumnIndex, *static_cast<com::sun::star::util::Time const *>(_rValue.getValue()));
            else
                bSuccessfullyReRouted = false;
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
            SAL_FALLTHROUGH;
        }
        default:
            bSuccessfullyReRouted = false;

    }

    return bSuccessfullyReRouted;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
