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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_CONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_CONNECTION_HXX

#include <config_lgpl.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <com/sun/star/container/XNameAccess.hpp>

#include <rtl/ref.hxx>
#include <rtl/byteseq.hxx>

#include <salhelper/simplereferenceobject.hxx>

#include <cppuhelper/weakref.hxx>
#include <cppuhelper/compbase.hxx>
#include <functional>

#include <libpq-fe.h>
#include <unordered_map>

namespace pq_sdbc_driver
{
#ifdef POSTGRE_TRACE
#define POSTGRE_TRACE( x ) printf( "%s\n" , x )
#else
#define POSTGRE_TRACE(x) ((void)0)
#endif

class RefCountedMutex : public salhelper::SimpleReferenceObject
{
public:
    osl::Mutex mutex;
};

struct ConnectionSettings;


// Logging API

enum class LogLevel
{
    NONE    = 0,
    Error,
    Sql,
    Info,
    LAST = Info
};
bool isLog(ConnectionSettings *settings, LogLevel nLevel);
void log(ConnectionSettings *settings, LogLevel nLevel, const OUString &logString);
void log(ConnectionSettings *settings, LogLevel nLevel, const char *str);


class Tables;
class Views;
struct ConnectionSettings
{
    ConnectionSettings() :
        pConnection(nullptr),
        maxNameLen(0),
        maxIndexKeys(0),
        pTablesImpl(nullptr),
        pViewsImpl(nullptr),
        showSystemColumns( false ),
        logFile( nullptr ),
        m_nLogLevel(LogLevel::Info)
    {}
    static const rtl_TextEncoding encoding = RTL_TEXTENCODING_UTF8;
    PGconn *pConnection;
    sal_Int32 maxNameLen;
    sal_Int32 maxIndexKeys;
    css::uno::Reference< css::script::XTypeConverter > tc;
    css::uno::Reference< css::container::XNameAccess > tables;
    css::uno::Reference< css::container::XNameAccess > users;
    css::uno::Reference< css::container::XNameAccess > views;
    Tables *pTablesImpl;  // needed to implement renaming of tables / views
    Views *pViewsImpl;   // needed to implement renaming of tables / views
    OUString user;
    OUString catalog;
    bool showSystemColumns;
    FILE *logFile;
    LogLevel m_nLogLevel;
};


typedef cppu::WeakComponentImplHelper<
    css::sdbc::XConnection,
    css::sdbc::XWarningsSupplier,
    css::lang::XInitialization,
    css::sdbcx::XTablesSupplier,
    css::sdbcx::XViewsSupplier,
    css::sdbcx::XUsersSupplier > ConnectionBase;

// some types
struct HashByteSequence
{
    sal_Int32 operator () ( const ::rtl::ByteSequence & seq ) const
    {
        return *reinterpret_cast<sal_Int32 const *>(seq.getConstArray());
    }
};

typedef std::unordered_map<
    ::rtl::ByteSequence,
    css::uno::WeakReference< css::sdbc::XCloseable >,
    HashByteSequence > WeakHashMap;
typedef ::std::vector< OString > OStringVector;


typedef std::unordered_map
<
    sal_Int32,
    OUString
> Int2StringMap;

class Connection : public ConnectionBase
{
    css::uno::Reference< css::uno::XComponentContext > m_ctx;
    css::uno::Reference< css::container::XNameAccess > m_typeMap;
    ConnectionSettings m_settings;
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    css::uno::Reference< css::sdbc::XDatabaseMetaData > m_meta;
    WeakHashMap m_myStatements;

private:
    void checkClosed()
        throw ( css::sdbc::SQLException, css::uno::RuntimeException );

public:
    Connection(
        const rtl::Reference< RefCountedMutex > &refMutex,
        const css::uno::Reference< css::uno::XComponentContext > & ctx );

    virtual ~Connection( ) override;

public: // XCloseable
    virtual void SAL_CALL close()
        throw ( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

public: // XConnection

    virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement(
        const OUString& sql )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall(
        const OUString& sql )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL nativeSQL( const OUString& sql )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAutoCommit(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL commit(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rollback(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isClosed(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setReadOnly( sal_Bool readOnly )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isReadOnly(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCatalog( const OUString& catalog )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCatalog(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTransactionIsolation( sal_Int32 level )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getTransactionIsolation(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTypeMap(
        const css::uno::Reference< css::container::XNameAccess >& typeMap )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

public: // XWarningsSupplier
    virtual css::uno::Any SAL_CALL getWarnings(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearWarnings(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

public: // XInitialization
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

public: // XTablesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTables(  ) throw (css::uno::RuntimeException, std::exception) override;

public: // XUsersSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getUsers(  ) throw (css::uno::RuntimeException, std::exception) override;

public: // XViewsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getViews(  ) throw (css::uno::RuntimeException, std::exception) override;

public:
    virtual void SAL_CALL disposing() override;

public: // helper function
    void removeFromWeakMap( const ::rtl::ByteSequence & seq );
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
