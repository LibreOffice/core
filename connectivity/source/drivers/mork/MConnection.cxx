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

#include <connectivity/dbcharset.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/sqlerror.hxx>

#include "resource/mork_res.hrc"
#include "resource/common_res.hrc"

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>

#include <comphelper/officeresourcebundle.hxx>
#include <comphelper/processfactory.hxx>

using namespace dbtools;


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


namespace connectivity { namespace mork {

static const int defaultScope = 0x80;



OConnection::OConnection(MorkDriver* _pDriver)
    :OSubComponent<OConnection, OConnection_BASE>(static_cast<cppu::OWeakObject*>(_pDriver), this)
    ,m_pDriver(_pDriver)
    ,m_aColumnAlias( _pDriver->getFactory() )
{
    m_pDriver->acquire();
    m_pBook = new MorkParser();
    m_pHistory = new MorkParser();
}

OConnection::~OConnection()
{
    if(!isClosed())
        close();
    m_pDriver->release();
    m_pDriver = nullptr;
    delete m_pBook;
    delete m_pHistory;
}

void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}


void OConnection::construct(const OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    (void) info; // avoid warnings
    SAL_INFO("connectivity.mork", "=> OConnection::construct()" );
    //  open file
    setURL(url);

    // Skip 'sdbc:mozab: part of URL

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    OSL_ENSURE( url.copy( 0, nLen ) == "sdbc:address", "OConnection::construct: invalid start of the URI - should never have survived XDriver::acceptsURL!" );

    OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    OUString aAddrbookScheme;
    OUString sAdditionalInfo;
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
        sAdditionalInfo = aAddrbookURI.copy( nLen + 1 );
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
        OUString path = m_pDriver->getProfilePath();
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
    MorkTableMap::Map::iterator tableIter;
    if (Tables)
    {
        // Iterate all tables
        for ( tableIter = Tables->map.begin(); tableIter != Tables->map.end(); ++tableIter )
        {
            if ( 0 == tableIter->first ) continue;
            SAL_INFO("connectivity.mork", "table->first : " << tableIter->first);
        }
    }
    // check that we can retrieve the history tables:
    MorkTableMap *Tables_hist = m_pHistory->getTables( defaultScope );
    if (Tables_hist)
    {
        // Iterate all tables
        for ( tableIter = Tables_hist->map.begin(); tableIter != Tables_hist->map.end(); ++tableIter )
        {
            if ( 0 == tableIter->first ) continue;
            SAL_INFO("connectivity.mork", "table->first : " << tableIter->first);
        }
    }
}

// XServiceInfo

IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.mork.OConnection", "com.sun.star.sdbc.Connection")


Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException, std::exception)
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

Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const OUString& _sSql ) throw(SQLException, RuntimeException, std::exception)
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

Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const OUString& _sSql ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("connectivity.mork", "=> OConnection::prepareCall()" );
    SAL_INFO("connectivity.mork", "sql: " << _sSql);
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::prepareCall", *this );
    SAL_INFO("connectivity.mork", "OConnection::prepareCall( " << _sSql << " )");
    return nullptr;
}

OUString SAL_CALL OConnection::nativeSQL( const OUString& _sSql ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("connectivity.mork", "=> OConnection::nativeSQL()" );
    SAL_INFO("connectivity.mork", "sql: " << _sSql);

    ::osl::MutexGuard aGuard( m_aMutex );
    // when you need to transform SQL92 to you driver specific you can do it here
    SAL_INFO("connectivity.mork", "OConnection::nativeSQL(" << _sSql << " )" );

    return _sSql;
}

void SAL_CALL OConnection::setAutoCommit( sal_Bool /*autoCommit*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setAutoCommit", *this );
}

sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    return sal_True;
}

void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException, std::exception)
{
    // when you database does support transactions you should commit here
}

void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException, std::exception)
{
    // same as commit but for the other case
}

sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // just simple -> we are close when we are disposed that means someone called dispose(); (XComponent)
    return OConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
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

void SAL_CALL OConnection::setReadOnly( sal_Bool /*readOnly*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setReadOnly", *this );
}

sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException, std::exception)
{
    // return if your connection to readonly
    return sal_False;
}

void SAL_CALL OConnection::setCatalog( const OUString& /*catalog*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setCatalog", *this );
}

OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException, std::exception)
{
    return OUString();
}

void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setTransactionIsolation", *this );
}

sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException, std::exception)
{
    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
    return TransactionIsolation::NONE;
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException, std::exception)
{
    // if your driver has special database types you can return it here
    return nullptr;
}

void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setTypeMap", *this );
}

// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException, std::exception)
{
    // we just dispose us
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}

// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    // when you collected some warnings -> return it
    return Any();
}

void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    // you should clear your collected warnings here
}

void OConnection::disposing()
{
    // we noticed that we should be destroied in near future so we have to dispose our statements
    ::osl::MutexGuard aGuard(m_aMutex);
    dispose_ChildImpl();
    m_xCatalog.clear();
}


Reference< XTablesSupplier > SAL_CALL OConnection::createCatalog()
{
    OSL_TRACE("IN OConnection::createCatalog()" );
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!m_xCatalog.is())
    {
        OCatalog *pCat = new OCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    OSL_TRACE( "\tOUT OConnection::createCatalog()" );
    return xTab;
}



void OConnection::throwSQLException( const ErrorDescriptor& _rError, const Reference< XInterface >& _rxContext )
{
    if ( _rError.getResId() != 0 )
    {
        OSL_ENSURE( ( _rError.getErrorCondition() == 0 ),
            "OConnection::throwSQLException: unsupported error code combination!" );

        OUString sParameter( _rError.getParameter() );
        if ( !sParameter.isEmpty() )
        {
            const OUString sError( getResources().getResourceStringWithSubstitution(
                _rError.getResId(),
                "$1$", sParameter
             ) );
            ::dbtools::throwGenericSQLException( sError, _rxContext );
            OSL_FAIL( "OConnection::throwSQLException: unreachable (1)!" );
        }

        throwGenericSQLException( _rError.getResId(), _rxContext );
        OSL_FAIL( "OConnection::throwSQLException: unreachable (2)!" );
    }

    if ( _rError.getErrorCondition() != 0 )
    {
        SQLError aErrorHelper( comphelper::getComponentContext(getDriver()->getFactory()) );
        OUString sParameter( _rError.getParameter() );
        if ( !sParameter.isEmpty() )
            aErrorHelper.raiseException( _rError.getErrorCondition(), _rxContext, sParameter );
        else
            aErrorHelper.raiseException( _rError.getErrorCondition(), _rxContext);
        OSL_FAIL( "OConnection::throwSQLException: unreachable (3)!" );
    }

    throwGenericSQLException( STR_UNSPECIFIED_ERROR, _rxContext );
}


void OConnection::throwSQLException( const sal_uInt16 _nErrorResourceId, const Reference< XInterface >& _rxContext )
{
    ErrorDescriptor aError;
    aError.setResId( _nErrorResourceId );
    throwSQLException( aError, _rxContext );
}

} } // namespace connectivity::mork

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
