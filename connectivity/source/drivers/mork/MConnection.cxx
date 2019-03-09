/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "MConnection.hxx"
#include "MDriver.hxx"
#include "MDatabaseMetaData.hxx"
#include "MCatalog.hxx"
#include "MPreparedStatement.hxx"
#include "MorkParser.hxx"

#include <connectivity/dbexception.hxx>
#include <sal/log.hxx>

#include <strings.hrc>

#include <com/sun/star/sdbc/TransactionIsolation.hpp>

using namespace dbtools;


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


namespace connectivity { namespace mork {

static const int defaultScope = 0x80;


OConnection::OConnection(MorkDriver* _pDriver)
    :m_xDriver(_pDriver)
    ,m_aColumnAlias( _pDriver->getFactory() )
{
    m_pBook.reset( new MorkParser() );
    m_pHistory.reset( new MorkParser() );
}

OConnection::~OConnection()
{
    if(!isClosed())
        close();
    m_pBook.reset();
    m_pHistory.reset();
}

void OConnection::construct(const OUString& url)
{
    SAL_INFO("connectivity.mork", "=> OConnection::construct()" );
    //  open file
    setURL(url);

    // Skip 'sdbc:mozab: part of URL

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    OSL_ENSURE( url.startsWith("sdbc:address:"), "OConnection::construct: invalid start of the URI - should never have survived XDriver::acceptsURL!" );

    OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        // There isn't any subschema: - but could be just subschema
        if ( !aAddrbookURI.isEmpty() )
        {
            aAddrbookScheme= aAddrbookURI;
        }
        else
        {
            SAL_WARN("connectivity.mork", "No subschema given!!!");
            throwGenericSQLException( STR_URI_SYNTAX_ERROR, *this );
        }
    }
    else
    {
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);
    }

    SAL_INFO("connectivity.mork", "URI = " << aAddrbookURI );
    SAL_INFO("connectivity.mork", "Scheme = " << aAddrbookScheme );

    OUString abook;
    OUString history;
    const OUString UNITTEST_URL = "thunderbird:unittest:";
    sal_Int32 unittestIndex = url.indexOf(UNITTEST_URL);

    // production?
    if (unittestIndex == -1)
    {
        OUString path = m_xDriver->getProfilePath();
        SAL_INFO("connectivity.mork", "ProfilePath: " << path);
        abook = path + "/abook.mab";
        history = path + "/history.mab";
        SAL_INFO("connectivity.mork", "AdressbookPath (abook): " << abook);
        SAL_INFO("connectivity.mork", "AdressbookPath (history): " << history);
    }
    else
    {
        abook = aAddrbookURI.replaceFirst(UNITTEST_URL, "");
        SAL_INFO("connectivity.mork", "unit test: " << abook);
    }

    OString strPath = OUStringToOString(abook, RTL_TEXTENCODING_UTF8);

    // Open and parse mork file
    if (!m_pBook->open(strPath.getStr()))
    {
        SAL_WARN("connectivity.mork", "Can not parse abook mork file: " << strPath);
        const OUString sError( getResources().getResourceStringWithSubstitution(
            STR_COULD_NOT_LOAD_FILE, "$filename$", abook));
        ::dbtools::throwGenericSQLException( sError, *this );
    }

    // read history only in production
    if (unittestIndex == -1)
    {
        strPath = OUStringToOString(history, RTL_TEXTENCODING_UTF8);
        if (!m_pHistory->open(strPath.getStr()))
        {
            SAL_WARN("connectivity.mork", "Can not parse history mork file: " << strPath);
            const OUString sError( getResources().getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_FILE, "$filename$", history));
            ::dbtools::throwGenericSQLException( sError, *this );
        }
    }

    // check that we can retrieve the tables:
    MorkTableMap *Tables = m_pBook->getTables( defaultScope );
    if (Tables)
    {
        // Iterate all tables
        for ( const auto& rEntry : Tables->map )
        {
            if ( 0 == rEntry.first ) continue;
            SAL_INFO("connectivity.mork", "table->first : " << rEntry.first);
        }
    }
    // check that we can retrieve the history tables:
    MorkTableMap *Tables_hist = m_pHistory->getTables( defaultScope );
    if (Tables_hist)
    {
        // Iterate all tables
        for ( const auto& rEntry : Tables_hist->map )
        {
            if ( 0 == rEntry.first ) continue;
            SAL_INFO("connectivity.mork", "table->first : " << rEntry.first);
        }
    }
}

// XServiceInfo

IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.mork.OConnection", "com.sun.star.sdbc.Connection")


Reference< XStatement > SAL_CALL OConnection::createStatement(  )
{
    SAL_INFO("connectivity.mork", "=> OConnection::createStatement()" );

    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // create a statement
    // the statement can only be executed once
    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const OUString& _sSql )
{
    SAL_INFO("connectivity.mork", "=> OConnection::prepareStatement()" );
    SAL_INFO("connectivity.mork", "OConnection::prepareStatement( " << _sSql << " )");

    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // the pre
    // create a statement
    // the statement can only be executed more than once
    OPreparedStatement* pPrepared = new OPreparedStatement(this,_sSql);
    Reference< XPreparedStatement > xReturn = pPrepared;
    pPrepared->lateInit();

    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const OUString& _sSql )
{
    SAL_INFO("connectivity.mork", "=> OConnection::prepareCall()" );
    SAL_INFO("connectivity.mork", "sql: " << _sSql);
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::prepareCall", *this );
    SAL_INFO("connectivity.mork", "OConnection::prepareCall( " << _sSql << " )");
    return nullptr;
}

OUString SAL_CALL OConnection::nativeSQL( const OUString& _sSql )
{
    SAL_INFO("connectivity.mork", "=> OConnection::nativeSQL()" );
    SAL_INFO("connectivity.mork", "sql: " << _sSql);

    ::osl::MutexGuard aGuard( m_aMutex );
    // when you need to transform SQL92 to you driver specific you can do it here
    SAL_INFO("connectivity.mork", "OConnection::nativeSQL(" << _sSql << " )" );

    return _sSql;
}

void SAL_CALL OConnection::setAutoCommit( sal_Bool /*autoCommit*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setAutoCommit", *this );
}

sal_Bool SAL_CALL OConnection::getAutoCommit(  )
{
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    return true;
}

void SAL_CALL OConnection::commit(  )
{
    // when you database does support transactions you should commit here
}

void SAL_CALL OConnection::rollback(  )
{
    // same as commit but for the other case
}

sal_Bool SAL_CALL OConnection::isClosed(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // just simple -> we are close when we are disposed that means someone called dispose(); (XComponent)
    return OConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  )
{
    SAL_INFO("connectivity.mork", "=> OConnection::getMetaData()" );

    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // here we have to create the class with biggest interface
    // The answer is 42 :-)
    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new ODatabaseMetaData(this); // need the connection because it can return it
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void SAL_CALL OConnection::setReadOnly( sal_Bool /*readOnly*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setReadOnly", *this );
}

sal_Bool SAL_CALL OConnection::isReadOnly(  )
{
    // return if your connection to readonly
    return false;
}

void SAL_CALL OConnection::setCatalog( const OUString& /*catalog*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setCatalog", *this );
}

OUString SAL_CALL OConnection::getCatalog(  )
{
    return OUString();
}

void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 /*level*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setTransactionIsolation", *this );
}

sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  )
{
    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
    return TransactionIsolation::NONE;
}

Reference< css::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  )
{
    // if your driver has special database types you can return it here
    return nullptr;
}

void SAL_CALL OConnection::setTypeMap( const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setTypeMap", *this );
}

// XCloseable
void SAL_CALL OConnection::close(  )
{
    // we just dispose us
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}

// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  )
{
    // when you collected some warnings -> return it
    return Any();
}

void SAL_CALL OConnection::clearWarnings(  )
{
    // you should clear your collected warnings here
}

void OConnection::disposing()
{
    // we noticed that we should be destroyed in near future so we have to dispose our statements
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xCatalog.clear();
}

Reference< XTablesSupplier > OConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!m_xCatalog.is())
    {
        OCatalog *pCat = new OCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}

void OConnection::throwSQLException( const ErrorDescriptor& _rError, const Reference< XInterface >& _rxContext )
{
    if (_rError.getResId() != nullptr)
    {
        throwGenericSQLException( _rError.getResId(), _rxContext );
        OSL_FAIL( "OConnection::throwSQLException: unreachable (2)!" );
    }

    throwGenericSQLException( STR_UNSPECIFIED_ERROR, _rxContext );
}

void OConnection::throwSQLException( const char* pErrorResourceId, const Reference< XInterface >& _rxContext )
{
    ErrorDescriptor aError;
    aError.setResId(pErrorResourceId);
    throwSQLException(aError, _rxContext);
}

} } // namespace connectivity::mork

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
