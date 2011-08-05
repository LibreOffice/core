/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  $RCSfile: pq_connection.cxx,v $
 *
 *  $Revision: 1.1.2.5 $
 *
 *  last change: $Author: jbu $ $Date: 2007/01/07 13:50:37 $
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
 *
 ************************************************************************/

#include <list>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "pq_connection.hxx"
#include "pq_statement.hxx"
#include "pq_preparedstatement.hxx"
#include "pq_databasemetadata.hxx"
#include "pq_xcontainer.hxx"
#include "pq_statics.hxx"
#include "pq_xtables.hxx"
#include "pq_xviews.hxx"
#include "pq_xusers.hxx"

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/uuid.h>
#include <rtl/bootstrap.hxx>
#include <osl/module.h>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

using rtl::OUStringBuffer;
using rtl::OUString;
using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUStringToOString;
using osl::MutexGuard;

using com::sun::star::container::XNameAccess;

using com::sun::star::lang::XComponent;
using com::sun::star::lang::XInitialization;
using com::sun::star::lang::IllegalArgumentException;

using com::sun::star::script::XTypeConverter;

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;

using com::sun::star::beans::PropertyValue;
using com::sun::star::beans::XPropertySet;

using com::sun::star::sdbc::XConnection;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XWarningsSupplier;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XDatabaseMetaData;

namespace pq_sdbc_driver
{

#define ASCII_STR(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )


// ______________________________________________________________________________
// Helper class for statement lifetime management
class ClosableReference : public cppu::WeakImplHelper1< com::sun::star::uno::XReference >
{
    Connection *m_conn;
    ::rtl::ByteSequence m_id;
public:
    ClosableReference( const ::rtl::ByteSequence & id , Connection *that )
      :  m_conn( that ), m_id( id )
    {
        that->acquire();
    }

    virtual ~ClosableReference()
    {
        if( m_conn )
            m_conn->release();
    }

    virtual void SAL_CALL dispose() throw ()
    {
        if( m_conn )
        {
            m_conn->removeFromWeakMap(m_id);
            m_conn->release();
            m_conn = 0;
        }
    }
};

OUString    ConnectionGetImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.connectivity.pq.Connection" ) );
}
com::sun::star::uno::Sequence<rtl::OUString> ConnectionGetSupportedServiceNames(void)
{
    OUString serv( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdbc.Connection" ) );
    return Sequence< OUString> (&serv,1);
}

static sal_Int32 readLogLevelFromConfiguration()
{
    sal_Int32 loglevel = LogLevel::NONE;
    OUString fileName;
    osl_getModuleURLFromAddress(
        (void*) readLogLevelFromConfiguration, (rtl_uString **) &fileName );
    fileName = OUString( fileName.getStr(), fileName.lastIndexOf( '/' )+1 );
    fileName += OUString::createFromAscii(  SAL_CONFIGFILE("postgresql-sdbc.uno") );
    rtl::Bootstrap bootstrapHandle( fileName );

    OUString str;
    if( bootstrapHandle.getFrom( ASCII_STR( "PQ_LOGLEVEL" ), str ) )
    {
        if( str.equalsAscii( "NONE" ) )
            loglevel = LogLevel::NONE;
        else if( str.equalsAscii( "ERROR" ) )
            loglevel = LogLevel::ERROR;
        else if( str.equalsAscii( "SQL" ) )
            loglevel = LogLevel::SQL;
        else if( str.equalsAscii( "INFO" ) )
            loglevel = LogLevel::SQL;
        else
        {
            fprintf( stderr, "unknown loglevel %s\n",
                     OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    return loglevel;
}

Connection::Connection(
    const rtl::Reference< RefCountedMutex > &refMutex,
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & ctx )
    : ConnectionBase( refMutex->mutex ),
      m_ctx( ctx ) ,
      m_refMutex( refMutex )
{
    m_settings.loglevel = readLogLevelFromConfiguration();

    if( m_settings.loglevel > LogLevel::NONE )
    {
        m_settings.logFile = fopen( "sdbc-pqsql.log", "a" );
        if( m_settings.logFile )
        {
            setvbuf( m_settings.logFile, 0, _IONBF, 0 );
            log( &m_settings, m_settings.loglevel , "set this loglevel" );
        }
        else
        {
            fprintf( stderr, "Couldn't open sdbc-pqsql.log file\n" );
        }
    }
}

Connection::~Connection()
{
    POSTGRE_TRACE( "dtor connection" );
    if( m_settings.pConnection )
    {
        PQfinish( m_settings.pConnection );
        m_settings.pConnection = 0;
    }
    if( m_settings.logFile )
    {
        fclose( m_settings.logFile );
        m_settings.logFile = 0;
    }
}
typedef ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XCloseable > ,
    ::pq_sdbc_driver::Allocator < ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XCloseable > > > CloseableList;

typedef ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > ,
    ::pq_sdbc_driver::Allocator < ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > > > DisposeableList;

void Connection::close() throw ( SQLException, RuntimeException )
{
    CloseableList lst;
    DisposeableList lstDispose;
    {
        MutexGuard guard( m_refMutex->mutex );
        // silently ignore, if the connection has been closed already
        if( m_settings.pConnection )
        {
            log( &m_settings, LogLevel::INFO, "closing connection" );
            PQfinish( m_settings.pConnection );
            m_settings.pConnection = 0;
        }

        lstDispose.push_back( Reference< XComponent > ( m_settings.users, UNO_QUERY ) );
        lstDispose.push_back( Reference< XComponent > ( m_settings.tables , UNO_QUERY ) );
        lstDispose.push_back( Reference< XComponent > ( m_meta, UNO_QUERY ) );
        m_meta.clear();
        m_settings.tables.clear();
        m_settings.users.clear();

        for( WeakHashMap::iterator ii = m_myStatements.begin() ;
             ii != m_myStatements.end() ;
             ++ii )
        {
            Reference< XCloseable > r = ii->second;
            if( r.is() )
                lst.push_back( r );
        }
    }

    // close all created statements
    for( CloseableList::iterator ii = lst.begin(); ii != lst.end() ; ++ii )
        ii->get()->close();

    // close all created statements
    for( DisposeableList::iterator iiDispose = lstDispose.begin();
         iiDispose != lstDispose.end() ; ++iiDispose )
    {
        if( iiDispose->is() )
            iiDispose->get()->dispose();
    }
}


void Connection::removeFromWeakMap( const ::rtl::ByteSequence & id )
{
    // shrink the list !
    MutexGuard guard( m_refMutex->mutex );
    WeakHashMap::iterator ii = m_myStatements.find( id );
    if( ii != m_myStatements.end() )
        m_myStatements.erase( ii );
}

Reference< XStatement > Connection::createStatement(  ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

    Statement *stmt = new Statement( m_refMutex, this , &m_settings );
    Reference< XStatement > ret( stmt );
    ::rtl::ByteSequence id( 16 );
    rtl_createUuid( (sal_uInt8*) id.getConstArray(), 0 , sal_False );
    m_myStatements[ id ] = Reference< XCloseable > ( stmt );
    stmt->queryAdapter()->addReference( new ClosableReference( id, this ) );
    return ret;
}

Reference< XPreparedStatement > Connection::prepareStatement( const ::rtl::OUString& sql )
        throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();

    rtl::OString byteSql = OUStringToOString( sql, m_settings.encoding );
    PreparedStatement *stmt = new PreparedStatement( m_refMutex, this, &m_settings, byteSql );
    Reference< XPreparedStatement > ret = stmt;

    ::rtl::ByteSequence id( 16 );
    rtl_createUuid( (sal_uInt8*) id.getConstArray(), 0 , sal_False );
    m_myStatements[ id ] = Reference< XCloseable > ( stmt );
    stmt->queryAdapter()->addReference( new ClosableReference( id, this ) );
    return ret;
}

Reference< XPreparedStatement > Connection::prepareCall( const ::rtl::OUString& sql )
        throw (SQLException, RuntimeException)
{
    throw SQLException(
        OUString(
            RTL_CONSTASCII_USTRINGPARAM( "pq_driver: Callable statements not supported" ) ),
        Reference< XInterface > (), OUString() , 1, Any() );
}


::rtl::OUString Connection::nativeSQL( const ::rtl::OUString& sql )
        throw (SQLException, RuntimeException)
{
    return sql;
}

void Connection::setAutoCommit( sal_Bool autoCommit ) throw (SQLException, RuntimeException)
{
    // UNSUPPORTED
}

sal_Bool Connection::getAutoCommit(  ) throw (SQLException, RuntimeException)
{
    // UNSUPPORTED
    return sal_True;
}

void Connection::commit(  ) throw (SQLException, RuntimeException)
{
    // UNSUPPORTED
}

void Connection::rollback(  ) throw (SQLException, RuntimeException)
{
    // UNSUPPORTED
}

sal_Bool Connection::isClosed(  ) throw (SQLException, RuntimeException)
{
    return m_settings.pConnection == 0;
}

Reference< XDatabaseMetaData > Connection::getMetaData(  )
        throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    if( ! m_meta.is() )
        m_meta = new DatabaseMetaData( m_refMutex, this, &m_settings );
    return m_meta;
}

void  Connection::setReadOnly( sal_Bool readOnly ) throw (SQLException, RuntimeException)
{
    // UNSUPPORTED

}

sal_Bool Connection::isReadOnly(  ) throw (SQLException, RuntimeException)
{
    // UNSUPPORTED
    return sal_False;
}

void Connection::setCatalog( const ::rtl::OUString& catalog )
        throw (SQLException, RuntimeException)
{
    // UNSUPPORTED
}

::rtl::OUString Connection::getCatalog(  ) throw (SQLException, RuntimeException)
{
    OUString ret;
    MutexGuard ( m_refMutex->mutex );
    if( m_settings.pConnection == 0 )
    {
        throw SQLException( ASCII_STR( "pq_connection: connection is closed" ), *this,
                            OUString(), 1, Any() );
    }
    char * p = PQdb(m_settings.pConnection );
    return OUString( p, strlen(p) ,  m_settings.encoding );
}

void Connection::setTransactionIsolation( sal_Int32 level )
        throw (SQLException, RuntimeException)
{
    // UNSUPPORTED
}

sal_Int32 Connection::getTransactionIsolation(  ) throw (SQLException, RuntimeException)
{
    // UNSUPPORTED
    return 0;
}

Reference< XNameAccess > Connection::getTypeMap(  ) throw (SQLException, RuntimeException)
{
    Reference< XNameAccess > t;
    {
        MutexGuard guard( m_refMutex->mutex );
        t = m_typeMap;
    }
    return t;
}

void Connection::setTypeMap( const Reference< XNameAccess >& typeMap )
        throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    m_typeMap = typeMap;
}
Any Connection::getWarnings(  ) throw (SQLException, RuntimeException)
{
    return Any();
}

void Connection::clearWarnings(  ) throw (SQLException, RuntimeException)
{
}


static OString properties2String( const OString initialString,
                                   const Sequence< PropertyValue > & args,
                                   const Reference< XTypeConverter> &tc )
{
    OStringBuffer ret;

    ret.append( initialString );
    if( initialString.getLength() )
        ret.append( " " );

    for( int i = 0; i < args.getLength() ; ++i )
    {
        bool append = true;
        if( args[i].Name.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "password" ) ) )
        {
            ret.append( "password=" );
        }
        else if( args[i].Name.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "user" ) ) )
        {
            ret.append( "user=" );
        }
        else if( args[i].Name.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "port" ) ) )
        {
            ret.append( "port=" );
        }
        else if( args[i].Name.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "dbname" ) ) )
        {
            ret.append( "dbname=" );
        }
        else if( args[i].Name.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "connect_timeout" ) ) )
        {
            ret.append( "connect_timeout=" );
        }
        else if( args[i].Name.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "options" ) ) )
        {
            ret.append( "options=" );
        }
        else if( args[i].Name.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "requiressl" ) ) )
        {
            ret.append( "requiressl=" );
        }
        else
        {
            append = false;
            // ignore for now
        OSL_TRACE("sdbc-postgresql: unknown argument '%s'", ::rtl::OUStringToOString( args[i].Name, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        if( append )
        {
            OUString value;
            tc->convertTo( args[i].Value ,getCppuType( &value) ) >>= value;
            ret.append( OUStringToOString( value, RTL_TEXTENCODING_UTF8) );
            ret.append( " " );
        }
    }

    return ret.makeStringAndClear();
}

void Connection::initialize( const Sequence< Any >& aArguments )
        throw (Exception, RuntimeException)
{
    OUString url;
    Sequence< PropertyValue > args;

    Reference< XTypeConverter > tc( m_ctx->getServiceManager()->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.Converter" ) ), m_ctx ),
                                    UNO_QUERY);
    if( ! tc.is() )
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("pq_driver: Couldn't instantiate converter service" )),
            Reference< XInterface > () );
    }
    if( aArguments.getLength() != 2 )
    {
        OUStringBuffer buf(128);
        buf.appendAscii( "pq_driver: expected 2 arguments, got " );
        buf.append( aArguments.getLength( ) );
        throw IllegalArgumentException(buf.makeStringAndClear(), Reference< XInterface > () , 0 );
    }

    if( ! (aArguments[0] >>= url) )
    {
        OUStringBuffer buf(128);
        buf.appendAscii( "pq_driver: expected string as first argument, got " );
        buf.append( aArguments[0].getValueType().getTypeName() );
        throw IllegalArgumentException( buf.makeStringAndClear() , *this, 0 );
    }

    tc->convertTo( aArguments[1], getCppuType( &args ) ) >>= args;

    OString o;
    int nColon = url.indexOf( ':' );
    if( nColon != -1 )
    {
        nColon = url.indexOf( ':' , 1+ nColon );
        if( nColon != -1 )
        {
             o = OUStringToOString( url.getStr()+nColon+1, m_settings.encoding );
        }
    }
    o = properties2String( o , args , tc );

    m_settings.pConnection = PQconnectdb( o.getStr() );
    if( ! m_settings.pConnection )
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM( "pq_driver: out of memory" ) ),
                                Reference< XInterface > () );
    if( PQstatus( m_settings.pConnection ) == CONNECTION_BAD )
    {
        OUStringBuffer buf( 128 );

        const char * error = PQerrorMessage( m_settings.pConnection );
        OUString errorMessage( error, strlen( error) , RTL_TEXTENCODING_ASCII_US );
        buf.appendAscii( "Couldn't establish database connection to '" );
        buf.append( url );
        buf.appendAscii( "' (" );
        buf.append( errorMessage );
        buf.appendAscii( ")" );
        PQfinish( m_settings.pConnection );
        m_settings.pConnection = 0;
        throw SQLException( buf.makeStringAndClear(), *this, errorMessage, CONNECTION_BAD, Any() );
    }
    PQsetClientEncoding( m_settings.pConnection, "UNICODE" );
    char *p = PQuser( m_settings.pConnection );
    m_settings.user = OUString( p, strlen(p), RTL_TEXTENCODING_UTF8);
    p = PQdb( m_settings.pConnection );
    m_settings.catalog = OUString( p, strlen(p), RTL_TEXTENCODING_UTF8);
    m_settings.tc = tc;

    if( isLog( &m_settings, LogLevel::INFO ) )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "connection to '" );
        buf.append( url );
        buf.appendAscii( "' successfully opened" );
        log( &m_settings, LogLevel::INFO, buf.makeStringAndClear() );
    }
}

void Connection::disposing()
{
    close();
}

void Connection::checkClosed() throw ( SQLException, RuntimeException )
{
    if( !m_settings.pConnection )
        throw SQLException( ASCII_STR( "pq_connection: Connection already closed" ),
                            *this, OUString(), 1, Any() );
}

Reference< XNameAccess > Connection::getTables(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    if( isLog( &m_settings, LogLevel::INFO ) )
    {
        log( &m_settings, LogLevel::INFO, "Connection::getTables() got called" );
    }
    MutexGuard guard( m_refMutex->mutex );
    if( !m_settings.tables.is() )
        m_settings.tables = Tables::create( m_refMutex, this, &m_settings , &m_settings.pTablesImpl);
    else
        // TODO: how to overcome the performance problem ?
        Reference< com::sun::star::util::XRefreshable > ( m_settings.tables, UNO_QUERY )->refresh();
    return m_settings.tables;
}

Reference< XNameAccess > Connection::getViews(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    if( isLog( &m_settings, LogLevel::INFO ) )
    {
        log( &m_settings, LogLevel::INFO, "Connection::getViews() got called" );
    }
    MutexGuard guard( m_refMutex->mutex );
    if( !m_settings.views.is() )
        m_settings.views = Views::create( m_refMutex, this, &m_settings, &(m_settings.pViewsImpl) );
    else
        // TODO: how to overcome the performance problem ?
        Reference< com::sun::star::util::XRefreshable > ( m_settings.views, UNO_QUERY )->refresh();
    return m_settings.views;
}



Reference< XNameAccess > Connection::getUsers(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    if( isLog( &m_settings, LogLevel::INFO ) )
    {
        log( &m_settings, LogLevel::INFO, "Connection::getUsers() got called" );
    }

    MutexGuard guard( m_refMutex->mutex );
    if( !m_settings.users.is() )
        m_settings.users = Users::create( m_refMutex, this, &m_settings );
    return m_settings.users;
}


Reference< XInterface >  ConnectionCreateInstance(
    const Reference< XComponentContext > & ctx ) throw (Exception)
{
    ::rtl::Reference< RefCountedMutex > ref = new RefCountedMutex();
    return * new Connection( ref, ctx );
}



bool isLog( ConnectionSettings *settings, int loglevel )
{
    return settings->loglevel >= loglevel && settings->logFile;
}

void log( ConnectionSettings *settings, sal_Int32 level, const OUString &logString )
{
    log( settings, level, OUStringToOString( logString, settings->encoding ).getStr() );
}
void log( ConnectionSettings *settings, sal_Int32 level, const char *str )
{
    if( isLog( settings, level ) )
    {
        static const char *strLevel[] = { "NONE", "ERROR", "SQL", "INFO", "DATA" };

        time_t t = ::time( 0 );
        char *pString;
#ifdef SAL_W32
        pString = asctime( localtime( &t ) );
#else
        struct tm timestruc;
        char timestr[50];
        memset( timestr, 0 , 50);
        pString = timestr;
        ::localtime_r( &t , &timestruc );
        asctime_r( &timestruc, timestr );
#endif
        for( int i = 0 ; pString[i] ; i ++ )
        {
            if( pString[i] <= 13 )
            {
                pString[i] = 0;
                break;
            }
        }
        fprintf( settings->logFile, "%s [%s]: %s\n", pString, strLevel[level], str );
    }
}


}



static struct cppu::ImplementationEntry g_entries[] =
{
    {
        pq_sdbc_driver::ConnectionCreateInstance, pq_sdbc_driver::ConnectionGetImplementationName,
        pq_sdbc_driver::ConnectionGetSupportedServiceNames, cppu::createSingleComponentFactory,
        0 , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};


extern "C"
{

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return cppu::component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}
