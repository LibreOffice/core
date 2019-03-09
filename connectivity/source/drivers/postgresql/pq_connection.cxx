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

#include <vector>
#include <time.h>
#include <string.h>

#include <memory>

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
#include <sal/log.hxx>
#include <o3tl/enumarray.hxx>
#include <osl/module.h>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

using osl::MutexGuard;

using com::sun::star::container::XNameAccess;

using com::sun::star::lang::XComponent;
using com::sun::star::lang::IllegalArgumentException;

using com::sun::star::script::Converter;
using com::sun::star::script::XTypeConverter;

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::Any;

using com::sun::star::beans::PropertyValue;

using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XDatabaseMetaData;

namespace pq_sdbc_driver
{


// Helper class for statement lifetime management
class ClosableReference : public cppu::WeakImplHelper< css::uno::XReference >
{
    rtl::Reference<Connection> m_conn;
    ::rtl::ByteSequence m_id;
public:
    ClosableReference( const ::rtl::ByteSequence & id , Connection *that )
      :  m_conn( that ), m_id( id )
    {
    }

    virtual void SAL_CALL dispose() override
    {
        if( m_conn.is() )
        {
            m_conn->removeFromWeakMap(m_id);
            m_conn.clear();
        }
    }
};

static OUString    ConnectionGetImplementationName()
{
    return OUString( "org.openoffice.comp.connectivity.pq.Connection.noext" );
}
static css::uno::Sequence<OUString> ConnectionGetSupportedServiceNames()
{
    return Sequence< OUString > { "com.sun.star.sdbc.Connection" };
}

static LogLevel readLogLevelFromConfiguration()
{
    LogLevel nLogLevel = LogLevel::NONE;
    OUString fileName;
    osl_getModuleURLFromFunctionAddress(
        reinterpret_cast<oslGenericFunction>(readLogLevelFromConfiguration), &fileName.pData );
    fileName = fileName.copy( fileName.lastIndexOf( '/' )+1 );
#ifdef MACOSX
    fileName += "../Resources/";
#endif
    fileName += "postgresql-sdbc.ini";
    rtl::Bootstrap bootstrapHandle( fileName );

    OUString str;
    if( bootstrapHandle.getFrom( "PQ_LOGLEVEL", str ) )
    {
        if ( str == "NONE" )
            nLogLevel = LogLevel::NONE;
        else if ( str == "ERROR" )
            nLogLevel = LogLevel::Error;
        else if ( str == "SQL" )
            nLogLevel = LogLevel::Sql;
        else if ( str == "INFO" )
            nLogLevel = LogLevel::Info;
        else
        {
            fprintf( stderr, "unknown loglevel %s\n",
                     OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    return nLogLevel;
}

Connection::Connection(
    const rtl::Reference< comphelper::RefCountedMutex > &refMutex,
    const css::uno::Reference< css::uno::XComponentContext > & ctx )
    : ConnectionBase( refMutex->GetMutex() ),
      m_ctx( ctx ) ,
      m_xMutex( refMutex )
{
    m_settings.m_nLogLevel = readLogLevelFromConfiguration();

    if (m_settings.m_nLogLevel != LogLevel::NONE)
    {
        m_settings.logFile = fopen( "sdbc-pqsql.log", "a" );
        if( m_settings.logFile )
        {
            setvbuf( m_settings.logFile, nullptr, _IONBF, 0 );
            log(&m_settings, m_settings.m_nLogLevel , "set this loglevel");
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
        m_settings.pConnection = nullptr;
    }
    if( m_settings.logFile )
    {
        fclose( m_settings.logFile );
        m_settings.logFile = nullptr;
    }
}
typedef std::vector< css::uno::Reference< css::sdbc::XCloseable > > CloseableVector;

typedef std::vector< css::uno::Reference< css::lang::XComponent > > DisposeableVector;

void Connection::close()
{
    CloseableVector vectorCloseable;
    DisposeableVector vectorDispose;
    {
        MutexGuard guard( m_xMutex->GetMutex() );
        // silently ignore, if the connection has been closed already
        if( m_settings.pConnection )
        {
            log(&m_settings, LogLevel::Info, "closing connection");
            PQfinish( m_settings.pConnection );
            m_settings.pConnection = nullptr;
        }

        vectorDispose.push_back( Reference< XComponent > ( m_settings.users, UNO_QUERY ) );
        vectorDispose.push_back( Reference< XComponent > ( m_settings.tables , UNO_QUERY ) );
        vectorDispose.push_back( Reference< XComponent > ( m_meta, UNO_QUERY ) );
        m_meta.clear();
        m_settings.tables.clear();
        m_settings.users.clear();

        for (auto const& statement : m_myStatements)
        {
            Reference< XCloseable > r = statement.second;
            if( r.is() )
                vectorCloseable.push_back( r );
        }
    }

    // close all created statements
    for (auto const& elem : vectorCloseable)
        elem.get()->close();

    // close all created statements
    for (auto const& elem : vectorDispose)
    {
        if( elem.is() )
            elem.get()->dispose();
    }
}


void Connection::removeFromWeakMap( const ::rtl::ByteSequence & id )
{
    // shrink the list !
    MutexGuard guard( m_xMutex->GetMutex() );
    WeakHashMap::iterator ii = m_myStatements.find( id );
    if( ii != m_myStatements.end() )
        m_myStatements.erase( ii );
}

Reference< XStatement > Connection::createStatement()
{
    MutexGuard guard( m_xMutex->GetMutex() );
    checkClosed();

    Statement *stmt = new Statement( m_xMutex, this , &m_settings );
    Reference< XStatement > ret( stmt );
    ::rtl::ByteSequence id( 16 );
    rtl_createUuid( reinterpret_cast<sal_uInt8*>(id.getArray()), nullptr, false );
    m_myStatements[ id ] = Reference< XCloseable > ( stmt );
    stmt->queryAdapter()->addReference( new ClosableReference( id, this ) );
    return ret;
}

Reference< XPreparedStatement > Connection::prepareStatement( const OUString& sql )
{
    MutexGuard guard( m_xMutex->GetMutex() );
    checkClosed();

    OString byteSql = OUStringToOString( sql, ConnectionSettings::encoding );
    PreparedStatement *stmt = new PreparedStatement( m_xMutex, this, &m_settings, byteSql );
    Reference< XPreparedStatement > ret = stmt;

    ::rtl::ByteSequence id( 16 );
    rtl_createUuid( reinterpret_cast<sal_uInt8*>(id.getArray()), nullptr, false );
    m_myStatements[ id ] = Reference< XCloseable > ( stmt );
    stmt->queryAdapter()->addReference( new ClosableReference( id, this ) );
    return ret;
}

Reference< XPreparedStatement > Connection::prepareCall( const OUString& )
{
    throw SQLException(
        "pq_driver: Callable statements not supported",
        Reference< XInterface > (), OUString() , 1, Any() );
}


OUString Connection::nativeSQL( const OUString& sql )
{
    return sql;
}

void Connection::setAutoCommit( sal_Bool )
{
    // UNSUPPORTED
}

sal_Bool Connection::getAutoCommit()
{
    // UNSUPPORTED
    return true;
}

void Connection::commit()
{
    // UNSUPPORTED
}

void Connection::rollback()
{
    // UNSUPPORTED
}

sal_Bool Connection::isClosed()
{
    return m_settings.pConnection == nullptr;
}

Reference< XDatabaseMetaData > Connection::getMetaData()
{
    MutexGuard guard( m_xMutex->GetMutex() );
    checkClosed();
    if( ! m_meta.is() )
        m_meta = new DatabaseMetaData( m_xMutex, this, &m_settings );
    return m_meta;
}

void  Connection::setReadOnly( sal_Bool )
{
    // UNSUPPORTED

}

sal_Bool Connection::isReadOnly()
{
    // UNSUPPORTED
    return false;
}

void Connection::setCatalog( const OUString& )
{
    // UNSUPPORTED
}

OUString Connection::getCatalog()
{
    MutexGuard guard( m_xMutex->GetMutex() );
    if( m_settings.pConnection == nullptr )
    {
        throw SQLException( "pq_connection: connection is closed", *this,
                            OUString(), 1, Any() );
    }
    char * p = PQdb(m_settings.pConnection );
    return OUString( p, strlen(p) ,  ConnectionSettings::encoding );
}

void Connection::setTransactionIsolation( sal_Int32 )
{
    // UNSUPPORTED
}

sal_Int32 Connection::getTransactionIsolation()
{
    // UNSUPPORTED
    return 0;
}

Reference< XNameAccess > Connection::getTypeMap()
{
    Reference< XNameAccess > t;
    {
        MutexGuard guard( m_xMutex->GetMutex() );
        t = m_typeMap;
    }
    return t;
}

void Connection::setTypeMap( const Reference< XNameAccess >& typeMap )
{
    MutexGuard guard( m_xMutex->GetMutex() );
    m_typeMap = typeMap;
}
Any Connection::getWarnings()
{
    return Any();
}

void Connection::clearWarnings()
{
}

class cstr_vector
{
    std::vector<char*> values;
    std::vector<bool>  acquired;
public:
    cstr_vector () : values(), acquired() { values.reserve(8); acquired.reserve(8); }
    ~cstr_vector ()
    {
        OSL_ENSURE(values.size() == acquired.size(), "pq_connection: cstr_vector values and acquired size mismatch");
        std::vector<bool>::const_iterator pa = acquired.begin();
        for( const auto& v : values )
        {
            if (*pa)
                free(v);
            ++pa;
        }
    }
    void push_back(const char* s, __sal_NoAcquire)
    {
        values.push_back(const_cast<char*>(s));
        acquired.push_back(false);
    }
    void push_back(char* s)
    {
        values.push_back(s);
        acquired.push_back(true);
    }
    // This const_cast is there for compatibility with PostgreSQL <= 9.1;
    // PostgreSQL >= 9.2 has the right const qualifiers in the headers
    // for a return type of "char const*const*".
    char const** c_array() const { return const_cast <const char**>(&values[0]); }
};

static void properties2arrays( const Sequence< PropertyValue > & args,
                               const Reference< XTypeConverter> &tc,
                               rtl_TextEncoding enc,
                               cstr_vector &keywords,
                               cstr_vector &values)
{
    // LEM TODO: can we just blindly take all properties?
    // I.e. they are prefiltered to have only relevant ones?
    // Else, at least support all keywords from
    // http://www.postgresql.org/docs/9.0/interactive/libpq-connect.html

    static const char* keyword_list[] = {
        "password",
        "user",
        "port",
        "dbname",
        "connect_timeout",
        "options",
        "requiressl"
    };

    for( int i = 0; i < args.getLength() ; ++i )
    {
        bool append = false;
        for(const char* j : keyword_list)
        {
            if( args[i].Name.equalsIgnoreAsciiCaseAscii( j ))
            {
                keywords.push_back( j, SAL_NO_ACQUIRE );
                append = true;
                break;
            }
        }

        if( append )
        {
            OUString value;
            tc->convertTo( args[i].Value, cppu::UnoType<decltype(value)>::get() ) >>= value;
            char *v = strdup(OUStringToOString(value, enc).getStr());
            values.push_back ( v );
        }
        else
        {
            // ignore for now
            SAL_WARN("connectivity.postgresql", "sdbc-postgresql: unknown argument '" << args[i].Name << "' having value: " << args[i].Value );
        }
    }
}

void Connection::initialize( const Sequence< Any >& aArguments )
{
    OUString url;
    Sequence< PropertyValue > args;

    Reference< XTypeConverter > tc( Converter::create(m_ctx) );
    if( ! tc.is() )
    {
        throw RuntimeException(
            "pq_driver: Couldn't instantiate converter service" );
    }
    if( aArguments.getLength() != 2 )
    {
        throw IllegalArgumentException(
            "pq_driver: expected 2 arguments, got " + OUString::number( aArguments.getLength( ) ),
            Reference< XInterface > () , 0 );
    }

    if( ! (aArguments[0] >>= url) )
    {
        throw IllegalArgumentException(
            "pq_driver: expected string as first argument, got "
            + aArguments[0].getValueType().getTypeName(),
            *this, 0 );
    }

    tc->convertTo( aArguments[1], cppu::UnoType<decltype(args)>::get() ) >>= args;

    OString o;
    int nColon = url.indexOf( ':' );
    if( nColon != -1 )
    {
        nColon = url.indexOf( ':' , 1+ nColon );
        if( nColon != -1 )
        {
             o = OUStringToOString( url.getStr()+nColon+1, ConnectionSettings::encoding );
        }
    }
    {
        cstr_vector keywords;
        cstr_vector values;

        if ( o.getLength() > 0 )
        {
            char *err;
            std::shared_ptr<PQconninfoOption> oOpts(PQconninfoParse(o.getStr(), &err), PQconninfoFree);
            if (oOpts == nullptr)
            {
                OUString errorMessage;
                if ( err != nullptr)
                {
                    errorMessage = OUString( err, strlen(err), ConnectionSettings::encoding );
                    free(err);
                }
                else
                    errorMessage = "#no error message#";
                // HY092 is "Invalid attribute/option identifier."
                // Just the most likely error; the error might be  HY024 "Invalid attribute value".
                throw SQLException(
                    "Error in database URL '" + url + "':\n"  + errorMessage,
                    *this, "HY092", 5, Any() );
            }

            for (  PQconninfoOption * opt = oOpts.get(); opt->keyword != nullptr; ++opt)
            {
                if ( opt->val != nullptr )
                {
                    keywords.push_back(strdup(opt->keyword));
                    values.push_back(strdup(opt->val));
                }
            }
        }
        properties2arrays( args , tc, ConnectionSettings::encoding, keywords, values );
        keywords.push_back(nullptr, SAL_NO_ACQUIRE);
        values.push_back(nullptr, SAL_NO_ACQUIRE);

        m_settings.pConnection = PQconnectdbParams( keywords.c_array(), values.c_array(), 0 );
    }
    if( ! m_settings.pConnection )
        throw RuntimeException("pq_driver: out of memory" );
    if( PQstatus( m_settings.pConnection ) == CONNECTION_BAD )
    {
        const char * error = PQerrorMessage( m_settings.pConnection );
        OUString errorMessage( error, strlen( error) , RTL_TEXTENCODING_ASCII_US );
        PQfinish( m_settings.pConnection );
        m_settings.pConnection = nullptr;
        throw SQLException(
            "Couldn't establish database connection to '" + url + "'\n"
            + errorMessage,
            *this, errorMessage, CONNECTION_BAD, Any() );
    }
    PQsetClientEncoding( m_settings.pConnection, "UNICODE" );
    char *p = PQuser( m_settings.pConnection );
    m_settings.user = OUString( p, strlen(p), RTL_TEXTENCODING_UTF8);
    p = PQdb( m_settings.pConnection );
    m_settings.catalog = OUString( p, strlen(p), RTL_TEXTENCODING_UTF8);
    m_settings.tc = tc;

    if (isLog(&m_settings, LogLevel::Info))
    {
        OUStringBuffer buf( 128 );
        buf.append( "connection to '" );
        buf.append( url );
        buf.append( "' successfully opened" );
        log(&m_settings, LogLevel::Info, buf.makeStringAndClear());
    }
}

void Connection::disposing()
{
    close();
}

void Connection::checkClosed()
{
    if( !m_settings.pConnection )
        throw SQLException( "pq_connection: Connection already closed",
                            *this, OUString(), 1, Any() );
}

Reference< XNameAccess > Connection::getTables()
{
    if (isLog(&m_settings, LogLevel::Info))
    {
        log(&m_settings, LogLevel::Info, "Connection::getTables() got called");
    }
    MutexGuard guard( m_xMutex->GetMutex() );
    if( !m_settings.tables.is() )
        m_settings.tables = Tables::create( m_xMutex, this, &m_settings , &m_settings.pTablesImpl);
    else
        // TODO: how to overcome the performance problem ?
        Reference< css::util::XRefreshable > ( m_settings.tables, UNO_QUERY )->refresh();
    return m_settings.tables;
}

Reference< XNameAccess > Connection::getViews()
{
    if (isLog(&m_settings, LogLevel::Info))
    {
        log(&m_settings, LogLevel::Info, "Connection::getViews() got called");
    }
    MutexGuard guard( m_xMutex->GetMutex() );
    if( !m_settings.views.is() )
        m_settings.views = Views::create( m_xMutex, this, &m_settings, &(m_settings.pViewsImpl) );
    else
        // TODO: how to overcome the performance problem ?
        Reference< css::util::XRefreshable > ( m_settings.views, UNO_QUERY )->refresh();
    return m_settings.views;
}


Reference< XNameAccess > Connection::getUsers()
{
    if (isLog(&m_settings, LogLevel::Info))
    {
        log(&m_settings, LogLevel::Info, "Connection::getUsers() got called");
    }

    MutexGuard guard( m_xMutex->GetMutex() );
    if( !m_settings.users.is() )
        m_settings.users = Users::create( m_xMutex, this, &m_settings );
    return m_settings.users;
}

/// @throws Exception
static Reference< XInterface >  ConnectionCreateInstance(
    const Reference< XComponentContext > & ctx )
{
    ::rtl::Reference< comphelper::RefCountedMutex > ref = new comphelper::RefCountedMutex;
    return * new Connection( ref, ctx );
}


bool isLog(ConnectionSettings const *settings, LogLevel nLevel)
{
    return static_cast<int>(settings->m_nLogLevel) >= static_cast<int>(nLevel)
           && settings->logFile;
}

void log(ConnectionSettings *settings, LogLevel nLevel, const OUString &logString)
{
    log( settings, nLevel, OUStringToOString( logString, ConnectionSettings::encoding ).getStr() );
}
void log(ConnectionSettings *settings, LogLevel nLevel, const char *str)
{
    if (isLog(settings, nLevel))
    {
        static const o3tl::enumarray<LogLevel, const char*> strLevel = {"NONE", "ERROR", "SQL", "INFO"};

        time_t t = ::time( nullptr );
        char *pString;
#ifdef _WIN32
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
        fprintf(settings->logFile, "%s [%s]: %s\n", pString, strLevel[nLevel], str);
    }
}


}


static const struct cppu::ImplementationEntry g_entries[] =
{
    {
        pq_sdbc_driver::ConnectionCreateInstance, pq_sdbc_driver::ConnectionGetImplementationName,
        pq_sdbc_driver::ConnectionGetSupportedServiceNames, cppu::createSingleComponentFactory,
        nullptr , 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};


extern "C"
{

SAL_DLLPUBLIC_EXPORT void * postgresql_sdbc_impl_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
