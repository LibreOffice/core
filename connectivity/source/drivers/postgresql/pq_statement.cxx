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

#include "pq_statement.hxx"
#include "pq_fakedupdateableresultset.hxx"
#include "pq_updateableresultset.hxx"
#include "pq_tools.hxx"
#include "pq_statics.hxx"

#include <osl/thread.h>
#include <osl/time.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <comphelper/sequence.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <string.h>

using osl::Mutex;
using osl::MutexGuard;


using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::UNO_QUERY;

using com::sun::star::lang::IllegalArgumentException;

using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XConnection;
using com::sun::star::sdbc::SQLException;

using com::sun::star::sdbcx::XColumnsSupplier;
using com::sun::star::sdbcx::XKeysSupplier;

using com::sun::star::beans::Property;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::XPropertySet;

using com::sun::star::container::XNameAccess;
using com::sun::star::container::XEnumerationAccess;
using com::sun::star::container::XEnumeration;
using com::sun::star::container::XIndexAccess;

namespace pq_sdbc_driver
{
static ::cppu::IPropertyArrayHelper & getStatementPropertyArrayHelper()
{
    static ::cppu::IPropertyArrayHelper *pArrayHelper;
    if( ! pArrayHelper )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pArrayHelper )
        {
            static Property aTable[] =
                {
                    Property(
                        "CursorName", 0,
                        ::cppu::UnoType<OUString>::get() , 0 ),
                    Property(
                        "EscapeProcessing", 1,
                        cppu::UnoType<bool>::get() , 0 ),
                    Property(
                        "FetchDirection", 2,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "FetchSize", 3,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "MaxFieldSize", 4,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "MaxRows", 5,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "QueryTimeOut", 6,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "ResultSetConcurrency", 7,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "ResultSetType", 8,
                        ::cppu::UnoType<sal_Int32>::get() , 0 )
                };
            static_assert( SAL_N_ELEMENTS(aTable) == STATEMENT_SIZE, "wrong number of elements" );
            static ::cppu::OPropertyArrayHelper arrayHelper( aTable, STATEMENT_SIZE, true );
            pArrayHelper = &arrayHelper;
        }
    }
    return *pArrayHelper;
}

Statement::Statement( const ::rtl::Reference< RefCountedMutex > & refMutex,
                      const Reference< XConnection > & conn,
                      struct ConnectionSettings *pSettings )
    : Statement_BASE( refMutex->mutex )
    , OPropertySetHelper( Statement_BASE::rBHelper )
    , m_connection( conn )
    , m_pSettings( pSettings )
    , m_refMutex( refMutex )
    , m_multipleResultAvailable(false)
    , m_multipleResultUpdateCount(0)
    , m_lastOidInserted(InvalidOid)
{
    m_props[STATEMENT_QUERY_TIME_OUT] = makeAny( (sal_Int32)0 );
    m_props[STATEMENT_MAX_ROWS] = makeAny( (sal_Int32)0 );
    m_props[STATEMENT_RESULT_SET_CONCURRENCY] = makeAny(
        css::sdbc::ResultSetConcurrency::READ_ONLY );
    m_props[STATEMENT_RESULT_SET_TYPE] = makeAny(
        css::sdbc::ResultSetType::SCROLL_INSENSITIVE );
}

Statement::~Statement()
{
    POSTGRE_TRACE( "dtor Statement" );
}

void Statement::checkClosed()
{
    if( ! m_pSettings || ! m_pSettings->pConnection )
        throw SQLException(
            "pq_driver: Statement or connection has already been closed !",
            *this, OUString(),1,Any());
}

Any Statement::queryInterface( const Type & rType )
{
    Any aRet = Statement_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}


Sequence< Type > Statement::getTypes()
{
    static Sequence< Type > *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static Sequence< Type > collection(
                ::comphelper::concatSequences(
                    OPropertySetHelper::getTypes(),
                    Statement_BASE::getTypes()));
            pCollection = &collection;
        }
    }
    return *pCollection;
}

Sequence< sal_Int8> Statement::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void Statement::close(  )
{
    // let the connection die without acquired mutex !
    Reference< XConnection > r;
    Reference< XCloseable > resultSet;
    {
        MutexGuard guard( m_refMutex->mutex );
        m_pSettings = nullptr;
        r = m_connection;
        m_connection.clear();

        resultSet = m_lastResultset;
        m_lastResultset.clear();
    }
    if( resultSet.is() )
    {
        resultSet->close();
        POSTGRE_TRACE( "statement closed" );
    }

}

void Statement::raiseSQLException(
    const OUString & sql, const char * errorMsg )
{
    OUStringBuffer buf(128);
    buf.append( "pq_driver: ");
    buf.append(
        OUString( errorMsg, strlen(errorMsg), ConnectionSettings::encoding ) );
    buf.append( " (caused by statement '" );
    buf.append( sql );
    buf.append( "')" );
    OUString error = buf.makeStringAndClear();
    log(m_pSettings, LogLevel::Error, error);
    throw SQLException( error, *this, OUString(), 1, Any() );
}

Reference< XResultSet > Statement::executeQuery(const OUString& sql )
{
    Reference< XCloseable > lastResultSetHolder = m_lastResultset;
    if( lastResultSetHolder.is() )
        lastResultSetHolder->close();

    if( ! execute( sql ) )
    {
        raiseSQLException( sql, "not a query" );
    }
    return Reference< XResultSet > ( m_lastResultset, css::uno::UNO_QUERY );
}

sal_Int32 Statement::executeUpdate( const OUString& sql )
{
    if( execute( sql ) )
    {
        raiseSQLException( sql, "not a command" );
    }
    return m_multipleResultUpdateCount;
}

/// @throws SQLException
static void raiseSQLException(
    ConnectionSettings *pSettings,
    const Reference< XInterface> & owner,
    const OString & sql,
    const char * errorMsg,
    const char *errorType = nullptr )
{
    OUStringBuffer buf(128);
    buf.append( "pq_driver: ");
    if( errorType )
    {
        buf.append( "[" );
        buf.appendAscii( errorType );
        buf.append( "]" );
    }
    buf.append(
        OUString( errorMsg, strlen(errorMsg) , ConnectionSettings::encoding ) );
    buf.append( " (caused by statement '" );
    buf.append( OStringToOUString( sql, ConnectionSettings::encoding ) );
    buf.append( "')" );
    OUString error = buf.makeStringAndClear();
    log(pSettings, LogLevel::Error, error);
    throw SQLException( error, owner, OUString(), 1, Any() );
}


// returns the elements of the primary key of the given table
// static Sequence< Reference< css::beans::XPropertySet > > lookupKeys(
static std::vector< OUString > lookupKeys(
    const Reference< css::container::XNameAccess > &tables,
    const OUString & table,
    OUString *pSchema,
    OUString *pTable,
    ConnectionSettings *pSettings)
{
    std::vector< OUString  > ret;
    Reference< XKeysSupplier > keySupplier;
    Statics & st = getStatics();

    if( tables->hasByName( table ) )
        tables->getByName( table ) >>= keySupplier;
    else if( -1 == table.indexOf( '.' ) )
    {
        // it wasn't a fully qualified name. Now need to skip through all tables.
        Reference< XEnumerationAccess > enumerationAccess =
            Reference< XEnumerationAccess > ( tables, UNO_QUERY );

        Reference< css::container::XEnumeration > enumeration =
            enumerationAccess->createEnumeration();
        while( enumeration->hasMoreElements() )
        {
            Reference< XPropertySet > set;
            enumeration->nextElement() >>= set;
            OUString name;
//             OUString schema;

            if( set->getPropertyValue( st.NAME ) >>= name )
            {
//                 printf( "searching %s %s\n",
//                         OUStringToOString( schema, RTL_TEXTENCODING_ASCII_US ).getStr(),
//                         OUStringToOString( name, RTL_TEXTENCODING_ASCII_US ).getStr() );
                if( name == table )
                {

                    if( keySupplier.is() )
                    {
                        // is ambigous, as I don't know postgresql searchpath,
                        // I can't continue here, as I may write to a different table
                        keySupplier.clear();
                        if (isLog(pSettings, LogLevel::Info))
                        {
                            OStringBuffer buf( 128 );
                            buf.append( "Can't offer updateable result set because table " );
                            buf.append( OUStringToOString(name, ConnectionSettings::encoding) );
                            buf.append( " is duplicated, add schema to resolve ambiguity" );
                            log(pSettings, LogLevel::Info, buf.makeStringAndClear().getStr());
                        }
                        break;
                    }
                    keySupplier.set( set, UNO_QUERY );
                }
            }
        }
    }
    else
    {
        if (isLog(pSettings, LogLevel::Info))
        {
            OStringBuffer buf( 128 );
            buf.append( "Can't offer updateable result set ( table " );
            buf.append( OUStringToOString(table, ConnectionSettings::encoding) );
            buf.append( " is unknown)" );
            log(pSettings, LogLevel::Info, buf.makeStringAndClear().getStr());
        }
    }

    if( keySupplier.is() )
    {
        Reference< XPropertySet > set( keySupplier, UNO_QUERY );
        set->getPropertyValue( getStatics().NAME ) >>= (*pTable);
        set->getPropertyValue( getStatics().SCHEMA_NAME ) >>= (*pSchema );
        set.clear();

        Reference< XEnumerationAccess > keys ( keySupplier->getKeys(), UNO_QUERY );
        Reference< XEnumeration > enumeration = keys->createEnumeration();
        while( enumeration->hasMoreElements() )
        {
            enumeration->nextElement() >>= set;
            sal_Int32 keyType = 0;
            if( (set->getPropertyValue( st.TYPE ) >>= keyType ) &&
                keyType == css::sdbcx::KeyType::PRIMARY )
            {
                Reference< XColumnsSupplier > columns( set, UNO_QUERY );
                Reference< XIndexAccess > indexAccess =
                    Reference< XIndexAccess > ( columns->getColumns(), UNO_QUERY );

                int length = indexAccess->getCount();
                ret.resize( length );
//                 printf( "primary key for Table %s is ",
//                         OUStringToOString( table, RTL_TEXTENCODING_ASCII_US ).getStr() );
                for( int i = 0 ; i < length ; i ++ )
                {
                    indexAccess->getByIndex( i ) >>= set;
                    OUString name;
                    set->getPropertyValue( st.NAME ) >>= name;
                    ret[i] = name;
//                     printf( "%s," , OUStringToOString( name, RTL_TEXTENCODING_ASCII_US ).getStr() );
                }
//                 printf( "\n" );
            }
        }
        if( ! ret.size() )
        {
            if (isLog(pSettings, LogLevel::Info))
            {
                OStringBuffer buf( 128 );
                buf.append( "Can't offer updateable result set ( table " );
                buf.append( OUStringToOString(table, ConnectionSettings::encoding) );
                buf.append( " does not have a primary key)" );
                log(pSettings, LogLevel::Info, buf.makeStringAndClear().getStr());
            }
        }
    }
    return ret;
}

bool executePostgresCommand( const OString & cmd, struct CommandData *data )
{
    ConnectionSettings *pSettings = *(data->ppSettings);

    sal_Int32 duration = osl_getGlobalTimer();
    PGresult *result = PQexec( pSettings->pConnection, cmd.getStr() );
    duration = osl_getGlobalTimer() - duration;
    if( ! result )
        raiseSQLException(
            pSettings, data->owner, cmd, PQerrorMessage( pSettings->pConnection ) );

    ExecStatusType state = PQresultStatus( result );
    *(data->pLastOidInserted) = 0;
    (data->pLastTableInserted)->clear();
    *(data->pLastQuery) = cmd;

    bool ret = false;
    switch( state )
    {
    case PGRES_COMMAND_OK:
    {
        *(data->pMultipleResultUpdateCount) = atoi( PQcmdTuples( result ) );
        *(data->pMultipleResultAvailable) = false;

        // in case an oid value is available, we retrieve it
        *(data->pLastOidInserted) = PQoidValue( result );

        // in case it was a single insert, extract the name of the table,
        // otherwise the table name is empty
        *(data->pLastTableInserted) =
            extractTableFromInsert( OStringToOUString( cmd, ConnectionSettings::encoding ) );
        if( isLog( pSettings, LogLevel::Sql ) )
        {
            OStringBuffer buf( 128 );
            buf.append( "executed command '" );
            buf.append( cmd.getStr() );
            buf.append( "' successfully (" );
            buf.append( *( data->pMultipleResultUpdateCount ) );
            buf.append( ")" );
            buf.append( ", duration=" );
            buf.append( duration );
            buf.append( "ms" );
            if( *(data->pLastOidInserted) )
            {
                buf.append( ", usedOid=" );
                buf.append( *(data->pLastOidInserted) );
                buf.append( ", diagnosedTable=" );
                buf.append(
                    OUStringToOString( *data->pLastTableInserted, ConnectionSettings::encoding ) );
            }
            log(pSettings, LogLevel::Sql, buf.makeStringAndClear().getStr());
        }
        PQclear( result );
        break;
    }
    case PGRES_TUPLES_OK: // success
    {
        // In case it is a single table, it has a primary key and all columns
        // belonging to the primary key are in the result set, allow updateable result sets
        // otherwise, don't
        OUString table, schema;
        std::vector< OUString > sourceTableKeys;
        OStringVector vec;
        tokenizeSQL( cmd, vec );
        OUString sourceTable =
            OStringToOUString(
                extractSingleTableFromSelect( vec ), ConnectionSettings::encoding );

        if( data->concurrency ==
            css::sdbc::ResultSetConcurrency::UPDATABLE )
        {
            OString aReason;
            if( sourceTable.getLength() )
            {
                sourceTableKeys = lookupKeys(
                    pSettings->tables.is() ?
                           pSettings->tables : data->tableSupplier->getTables() ,
                    sourceTable,
                    &schema,
                    &table,
                    pSettings);

                // check, whether the columns are in the result set (required !)
                int i;
                for( i = 0 ; i < (int)sourceTableKeys.size() ;  i ++ )
                {
                    if( -1 == PQfnumber(
                            result,
                            OUStringToOString( sourceTableKeys[i] ,
                                               ConnectionSettings::encoding ).getStr()) )
                    {
                        break;
                    }
                }

                if( sourceTableKeys.size() && i == (int)sourceTableKeys.size() )
                {
                    *(data->pLastResultset) =
                        UpdateableResultSet::createFromPGResultSet(
                            data->refMutex, data->owner, data->ppSettings, result,
                            schema, table,sourceTableKeys );
                }
                else if( ! table.getLength() )
                {
                    OStringBuffer buf( 128 );
                    buf.append( "can't support updateable resultset, because a single table in the "
                                "WHERE part of the statement could not be identified (" );
                    buf.append( cmd );
                    buf.append( "." );
                    aReason = buf.makeStringAndClear();
                }
                else if( sourceTableKeys.size() )
                {
                    OStringBuffer buf( 128 );
                    buf.append( "can't support updateable resultset for table " );
                    buf.append( OUStringToOString( schema, ConnectionSettings::encoding ) );
                    buf.append( "." );
                    buf.append( OUStringToOString( table, ConnectionSettings::encoding ) );
                    buf.append( ", because resultset does not contain a part of the primary key ( column " );
                    buf.append( OUStringToOString( sourceTableKeys[i], ConnectionSettings::encoding ) );
                    buf.append( " is missing )" );
                    aReason = buf.makeStringAndClear();
                }
                else
                {

                    OStringBuffer buf( 128 );
                    buf.append( "can't support updateable resultset for table " );
                    buf.append( OUStringToOString( schema, ConnectionSettings::encoding ) );
                    buf.append( "." );
                    buf.append( OUStringToOString( table, ConnectionSettings::encoding ) );
                    buf.append( ", because resultset table does not have a primary key " );
                    aReason = buf.makeStringAndClear();
                }
            }
            else
            {
                OStringBuffer buf( 128 );
                buf.append( "can't support updateable result for selects with multiple tables (" );
                buf.append( cmd );
                buf.append( ")" );
                log(pSettings, LogLevel::Sql, buf.makeStringAndClear().getStr() );
            }
            if( ! (*(data->pLastResultset)).is() )
            {
                if (isLog( pSettings, LogLevel::Error))
                {
                    log(pSettings, LogLevel::Error,  aReason.getStr());
                }

                // TODO: How to react here correctly ?
                // remove this piece of code
                *(data->pLastResultset) =
                    new FakedUpdateableResultSet(
                        data->refMutex, data->owner,
                        data->ppSettings,result, schema, table,
                        OStringToOUString( aReason, ConnectionSettings::encoding) );
            }

        }
        else if( sourceTable.getLength() > 0)
        {
            splitConcatenatedIdentifier( sourceTable, &schema, &table );
        }

        sal_Int32 returnedRows = PQntuples( result );
        if( ! data->pLastResultset->is() )
            *(data->pLastResultset) =
                Reference< XCloseable > (
                    new ResultSet(
                        data->refMutex, data->owner,
                        data->ppSettings,result, schema, table ) );
        *(data->pMultipleResultAvailable) = true;
        ret = true;
        if (isLog(pSettings, LogLevel::Sql))
        {
            OStringBuffer buf( 128 );
            buf.append( "executed query '" );
            buf.append( cmd );
            buf.append( "' successfully" );
            buf.append( ", duration=" );
            buf.append( duration );
            buf.append( "ms, returnedRows=" );
            buf.append( returnedRows );
            buf.append( "." );
            log(pSettings, LogLevel::Sql, buf.makeStringAndClear().getStr());
        }
        break;
    }
    case PGRES_EMPTY_QUERY:
    case PGRES_COPY_OUT:
    case PGRES_COPY_IN:
    case PGRES_BAD_RESPONSE:
    case PGRES_NONFATAL_ERROR:
    case PGRES_FATAL_ERROR:
    default:
        raiseSQLException(
            pSettings, data->owner, cmd, PQresultErrorMessage( result ) , PQresStatus( state ) );
    }
    return ret;

}

static Sequence< OUString > getPrimaryKeyColumnNames(
    const Reference< XConnection > & connection, const OUString &schemaName, const OUString &tableName )
{
    Sequence< OUString > ret;

    Int2StringMap mapIndex2Name;
    fillAttnum2attnameMap( mapIndex2Name, connection, schemaName, tableName );

    // retrieve the primary key ...
    Reference< XPreparedStatement > stmt = connection->prepareStatement(
            "SELECT conkey "              // 7
            "FROM pg_constraint INNER JOIN pg_class ON conrelid = pg_class.oid "
                      "INNER JOIN pg_namespace ON pg_class.relnamespace = pg_namespace.oid "
                      "LEFT JOIN pg_class AS class2 ON confrelid = class2.oid "
                      "LEFT JOIN pg_namespace AS nmsp2 ON class2.relnamespace=nmsp2.oid "
            "WHERE pg_class.relname = ? AND pg_namespace.nspname = ? AND pg_constraint.contype='p'" );
    DisposeGuard guard( stmt );
    Reference< XParameters > paras( stmt, UNO_QUERY );
    paras->setString( 1 , tableName );
    paras->setString( 2 , schemaName );
    Reference< XResultSet > rs = stmt->executeQuery();
    Reference< XRow > xRow( rs , UNO_QUERY );

    if( rs->next() )
    {
        ret = convertMappedIntArray2StringArray( mapIndex2Name, string2intarray(xRow->getString( 1 ) ) );
    }
    return ret;
}

static void getAutoValues(
    String2StringMap & result,
    const Reference< XConnection > & connection,
    const OUString &schemaName,
    const OUString & tableName )
{
    Reference< XPreparedStatement > stmt = connection->prepareStatement(
                  "SELECT   pg_attribute.attname, pg_attrdef.adsrc "
                  "FROM pg_class, pg_namespace, pg_attribute "
                  "LEFT JOIN pg_attrdef ON pg_attribute.attrelid = pg_attrdef.adrelid AND "
                                        "pg_attribute.attnum = pg_attrdef.adnum "
                  "WHERE pg_attribute.attrelid = pg_class.oid AND "
                        "pg_class.relnamespace = pg_namespace.oid AND "
                        "pg_namespace.nspname = ? AND "
                  // LEM TODO: this is weird; why "LIKE" and not "="?
                  // Most probably gives problems if tableName contains '%'
                        "pg_class.relname LIKE ? AND "
                        "pg_attrdef.adsrc != ''"
            );
    DisposeGuard guard( stmt );
    Reference< XParameters > paras( stmt, UNO_QUERY );
    paras->setString( 1 , schemaName );
    paras->setString( 2 , tableName );
    Reference< XResultSet > rs = stmt->executeQuery();
    Reference< XRow > xRow( rs , UNO_QUERY );

    while( rs->next() )
    {
        result[ OUStringToOString( xRow->getString( 1 ), RTL_TEXTENCODING_ASCII_US) ] =
            OUStringToOString( xRow->getString(2), RTL_TEXTENCODING_ASCII_US );
    }
}

Reference< XResultSet > getGeneratedValuesFromLastInsert(
    ConnectionSettings *pConnectionSettings,
    const Reference< XConnection > &connection,
    sal_Int32 nLastOid,
    const OUString & lastTableInserted,
    const OString & lastQuery )
{
    Reference< XResultSet > ret;
    OUString query;
    OUString schemaName, tableName;
    splitConcatenatedIdentifier(
        lastTableInserted, &schemaName, &tableName );

    if( nLastOid && lastTableInserted.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "SELECT * FROM " );
        if( schemaName.getLength() )
            bufferQuoteQualifiedIdentifier(buf, schemaName, tableName, pConnectionSettings );
        else
            bufferQuoteIdentifier( buf, lastTableInserted, pConnectionSettings );
        buf.append( " WHERE oid = " );
        buf.append( nLastOid );
        query = buf.makeStringAndClear();
    }
    else if ( lastTableInserted.getLength() && lastQuery.getLength() )
    {
        // extract nameValue Pairs
        String2StringMap namedValues;
        extractNameValuePairsFromInsert( namedValues, lastQuery );

        // debug ...
//         OStringBuffer buf( 128);
//         buf.append( "extracting name/value from '" );
//         buf.append( lastQuery.getStr() );
//         buf.append( "' to [" );
//         for( String2StringMap::iterator ii = namedValues.begin() ; ii != namedValues.end() ; ++ii )
//         {
//             buf.append( ii->first.getStr() );
//             buf.append( "=" );
//             buf.append( ii->second.getStr() );
//             buf.append( "," );
//         }
//         buf.append( "]\n" );
//         printf( "%s", buf.makeStringAndClear() );

        // TODO: make also unqualified tables names work here. Have a look at 2.8.3. The Schema Search Path
        //       in postgresql doc

        Sequence< OUString > keyColumnNames = getPrimaryKeyColumnNames( connection, schemaName, tableName );
        if( keyColumnNames.getLength() )
        {
            OUStringBuffer buf( 128 );
            buf.append( "SELECT * FROM " );
            bufferQuoteQualifiedIdentifier(buf, schemaName, tableName, pConnectionSettings );
            buf.append( " WHERE " );
            bool bAdditionalCondition = false;
            String2StringMap autoValues;
            for( int i = 0 ; i < keyColumnNames.getLength() ; i ++ )
            {
                OUString value;
                OString columnName = OUStringToOString( keyColumnNames[i], ConnectionSettings::encoding );
                String2StringMap::const_iterator ii = namedValues.begin();
                for( ; ii != namedValues.end() ; ++ii )
                {
                    if( columnName.equalsIgnoreAsciiCase( ii->first ) )
                    {
                        value = OStringToOUString( ii->second , ConnectionSettings::encoding );
                        break;
                    }
                }

                // check, if a column of the primary key was not inserted explicitly,
                if( ii == namedValues.end() )
                {

                    if( autoValues.begin() == autoValues.end() )
                    {
                        getAutoValues( autoValues, connection, schemaName, tableName );
                    }
                    // this could mean, that the column is a default or auto value, check this ...
                    String2StringMap::const_iterator j = autoValues.begin();
                    for( ; j != autoValues.end() ; ++j )
                    {
                        if( columnName.equalsIgnoreAsciiCase( j->first ) )
                        {
                            // it is indeed an auto value.
                            value = OStringToOUString(j->second, RTL_TEXTENCODING_ASCII_US );
                            // check, whether it is a sequence

                            if( j->second.startsWith("nextval(") )
                            {
                                // retrieve current sequence value:
                                OUStringBuffer myBuf(128 );
                                myBuf.append( "SELECT currval(" );
                                myBuf.appendAscii( &(j->second.getStr()[8]));
                                value = querySingleValue( connection, myBuf.makeStringAndClear() );
                            }
                            break;
                        }
                    }
                    if( j == autoValues.end() )
                    {
                        // it even was no autovalue, no sense to continue as we can't query the
                        // inserted row
                        buf.truncate();
                        break;
                    }
                }

                if( bAdditionalCondition )
                    buf.append( " AND " );
                bufferQuoteIdentifier( buf, keyColumnNames[i], pConnectionSettings );
                buf.append( " = " );
                buf.append( value );
                bAdditionalCondition = true;
            }
            query = buf.makeStringAndClear();
        }
    }

    if( query.getLength() )
    {
        Reference< css::sdbc::XStatement > stmt = connection->createStatement();
        ret = stmt->executeQuery( query );
    }

    return ret;

}

sal_Bool Statement::execute( const OUString& sql )
{
    osl::MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    OString cmd = OUStringToOString( sql, m_pSettings );

    m_lastResultset.clear();
    m_lastTableInserted.clear();

    struct CommandData data;
    data.refMutex = m_refMutex;
    data.ppSettings = &m_pSettings;
    data.pLastOidInserted = &m_lastOidInserted;
    data.pLastQuery = &m_lastQuery;
    data.pMultipleResultUpdateCount = &m_multipleResultUpdateCount;
    data.pMultipleResultAvailable = &m_multipleResultAvailable;
    data.pLastTableInserted = &m_lastTableInserted;
    data.pLastResultset = &m_lastResultset;
    data.owner = *this;
    data.tableSupplier.set( m_connection, UNO_QUERY );
    data.concurrency =
        extractIntProperty( this, getStatics().RESULT_SET_CONCURRENCY );
    return executePostgresCommand( cmd , &data );
}

Reference< XConnection > Statement::getConnection(  )
{
    Reference< XConnection > ret;
    {
        MutexGuard guard( m_refMutex->mutex );
        checkClosed();
        ret = m_connection;
    }
    return ret;
}


Any Statement::getWarnings(  )
{
    return Any();
}

void Statement::clearWarnings(  )
{
}

Reference< css::sdbc::XResultSetMetaData > Statement::getMetaData()
{
    Reference< css::sdbc::XResultSetMetaData > ret;
    Reference< css::sdbc::XResultSetMetaDataSupplier > supplier( m_lastResultset, UNO_QUERY );
    if( supplier.is() )
        ret = supplier->getMetaData();
    return ret;
}


::cppu::IPropertyArrayHelper & Statement::getInfoHelper()
{
    return getStatementPropertyArrayHelper();
}


sal_Bool Statement::convertFastPropertyValue(
        Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue )
{
    rOldValue = m_props[nHandle];
    bool bRet;
    switch( nHandle )
    {
    case STATEMENT_CURSOR_NAME:
    {
        OUString val;
        bRet = ( rValue >>= val );
        rConvertedValue = makeAny( val );
        break;
    }
    case STATEMENT_ESCAPE_PROCESSING:
    {
        bool val(false);
        bRet = ( rValue >>= val );
        rConvertedValue = makeAny( val );
        break;
    }
    case STATEMENT_FETCH_DIRECTION:
    case STATEMENT_FETCH_SIZE:
    case STATEMENT_MAX_FIELD_SIZE:
    case STATEMENT_MAX_ROWS:
    case STATEMENT_QUERY_TIME_OUT:
    case STATEMENT_RESULT_SET_CONCURRENCY:
    case STATEMENT_RESULT_SET_TYPE:
    {
        sal_Int32 val;
        bRet = ( rValue >>= val );
        rConvertedValue = makeAny( val );
        break;
    }
    default:
    {
        throw IllegalArgumentException(
            "pq_statement: Invalid property handle ("
            + OUString::number( nHandle ) + ")",
            *this, 2 );
    }
    }
    return bRet;
}


void Statement::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle,const Any& rValue )
{
    m_props[nHandle] = rValue;
}

void Statement::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    rValue = m_props[nHandle];
}

Reference < XPropertySetInfo >  Statement::getPropertySetInfo()
{
    return OPropertySetHelper::createPropertySetInfo( getStatementPropertyArrayHelper() );
}


Reference< XResultSet > Statement::getResultSet(  )
{
    return Reference< XResultSet > ( m_lastResultset, css::uno::UNO_QUERY );
}

sal_Int32 Statement::getUpdateCount(  )
{
    return m_multipleResultUpdateCount;
}

sal_Bool Statement::getMoreResults(  )
{
    return false;
}


void Statement::disposing()
{
    close();
}

Reference< XResultSet > Statement::getGeneratedValues(  )
{
    osl::MutexGuard guard( m_refMutex->mutex );
    return getGeneratedValuesFromLastInsert(
        m_pSettings, m_connection, m_lastOidInserted, m_lastTableInserted, m_lastQuery );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
