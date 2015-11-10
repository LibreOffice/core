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
#include <boost/functional/hash.hpp>

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

namespace LogLevel
{
// when you add a loglevel, extend the log function !
static const sal_Int32 NONE = 0;
static const sal_Int32 ERROR = 1;
static const sal_Int32 SQL = 2;
static const sal_Int32 INFO = 3;
static const sal_Int32 DATA = 4;
}
bool isLog( ConnectionSettings *settings, int loglevel );
void log( ConnectionSettings *settings, sal_Int32 level, const OUString &logString );
void log( ConnectionSettings *settings, sal_Int32 level, const char *str );


class Tables;
class Views;
struct ConnectionSettings
{
    ConnectionSettings() :
        encoding( RTL_TEXTENCODING_UTF8),
        pConnection(nullptr),
        maxNameLen(0),
        maxIndexKeys(0),
        pTablesImpl(nullptr),
        pViewsImpl(nullptr),
        showSystemColumns( false ),
        logFile( nullptr ),
        loglevel( LogLevel::INFO )
    {}
    rtl_TextEncoding encoding;
    PGconn *pConnection;
    sal_Int32 maxNameLen;
    sal_Int32 maxIndexKeys;
    ::com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter > tc;
    ::com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > tables;
    ::com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > users;
    ::com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > views;
    Tables *pTablesImpl;  // needed to implement renaming of tables / views
    Views *pViewsImpl;   // needed to implement renaming of tables / views
    OUString user;
    OUString catalog;
    bool showSystemColumns;
    FILE *logFile;
    sal_Int32 loglevel;
};


typedef cppu::WeakComponentImplHelper<
    com::sun::star::sdbc::XConnection,
    com::sun::star::sdbc::XWarningsSupplier,
    com::sun::star::lang::XInitialization,
    com::sun::star::sdbcx::XTablesSupplier,
    com::sun::star::sdbcx::XViewsSupplier,
    com::sun::star::sdbcx::XUsersSupplier > ConnectionBase;

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
    ::com::sun::star::uno::WeakReference< com::sun::star::sdbc::XCloseable >,
    HashByteSequence,
    ::std::equal_to< ::rtl::ByteSequence >
> WeakHashMap;
typedef ::std::vector< OString > OStringVector;



typedef std::unordered_map
<
    sal_Int32,
    OUString,
    ::boost::hash< sal_Int32 >,
    ::std::equal_to< sal_Int32 >
> Int2StringMap;

class Connection : public ConnectionBase
{
    ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_ctx;
    ::com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > m_typeMap;
    ConnectionSettings m_settings;
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    ::com::sun::star::uno::Reference< com::sun::star::sdbc::XDatabaseMetaData > m_meta;
    WeakHashMap m_myStatements;

private:
    void checkClosed()
        throw ( com::sun::star::sdbc::SQLException, com::sun::star::uno::RuntimeException );

public:
    Connection(
        const rtl::Reference< RefCountedMutex > &refMutex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & ctx );

    virtual ~Connection( );

public: // XCloseable
    virtual void SAL_CALL close()
        throw ( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

public: // XConnection

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement(
        const OUString& sql )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall(
        const OUString& sql )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL nativeSQL( const OUString& sql )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAutoCommit(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL commit(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rollback(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isClosed(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setReadOnly( sal_Bool readOnly )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isReadOnly(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCatalog( const OUString& catalog )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCatalog(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTransactionIsolation( sal_Int32 level )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getTransactionIsolation(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTypeMap(
        const ::com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XWarningsSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearWarnings(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XInitialization
    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XTablesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XUsersSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getUsers(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XViewsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getViews(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

public:
    virtual void SAL_CALL disposing() override;

public: // helper function
    void removeFromWeakMap( const ::rtl::ByteSequence & seq );
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
