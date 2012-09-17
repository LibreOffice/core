/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "diagnose_ex.h"
#include "MNSProfileDiscover.hxx"
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

using namespace dbtools;

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
// --------------------------------------------------------------------------------

namespace connectivity { namespace mork {

static const int defaultScope = 0x80;

// -----------------------------------------------------------------------------

OConnection::OConnection(MorkDriver* _pDriver)
    :OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this)
    ,m_pDriver(_pDriver)
    ,m_aColumnAlias( _pDriver->getFactory() )
{
    m_pDriver->acquire();
    m_pProfileAccess = new ProfileAccess();
    m_pMork = new MorkParser();
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
    acquire();
    if(!isClosed())
        close();
    m_pDriver->release();
    m_pDriver = NULL;
    delete m_pProfileAccess;
    delete m_pMork;
}
//-----------------------------------------------------------------------------
void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}
// -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void OConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    (void) info; // avoid warnings
    SAL_INFO("connectivity.mork", "=> OConnection::construct()" );
    //  open file
    setURL(url);
    //
    // Skip 'sdbc:mozab: part of URL
    //
    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    OSL_ENSURE( url.copy( 0, nLen ) == "sdbc:address", "OConnection::construct: invalid start of the URI - should never have survived XDriver::acceptsURL!" );

    ::rtl::OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    ::rtl::OUString aAddrbookScheme;
    ::rtl::OUString sAdditionalInfo;
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

    ::rtl::OUString defaultProfile = m_pProfileAccess->getDefaultProfile(::com::sun::star::mozilla::MozillaProductType_Thunderbird);
    SAL_INFO("connectivity.mork", "DefaultProfile: " << defaultProfile);

    ::rtl::OUString path = m_pProfileAccess->getProfilePath(::com::sun::star::mozilla::MozillaProductType_Thunderbird, defaultProfile);
    SAL_INFO("connectivity.mork", "ProfilePath: " << path);

    path += rtl::OUString( "/abook.mab" );

    SAL_INFO("connectivity.mork", "AdressbookPath: " << path);

    rtl::OString strPath = ::rtl::OUStringToOString(path, RTL_TEXTENCODING_UTF8 );

    // Open and parse mork file
    if (!m_pMork->open(strPath.getStr()))
    {
        SAL_WARN("connectivity.mork", "Can not parse mork file!");
        throwGenericSQLException( STR_COULD_NOT_LOAD_FILE, *this );
    }

    // check that we can retrieve the tables:
    MorkTableMap *Tables = m_pMork->getTables( defaultScope );
    MorkTableMap::iterator tableIter;
    if (Tables)
    {
        // Iterate all tables
        for ( tableIter = Tables->begin(); tableIter != Tables->end(); tableIter++ )
        {
            if ( 0 == tableIter->first ) continue;
            SAL_INFO("connectivity.mork", "table->first : " << tableIter->first);
        }
    }
}

// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.mork.OConnection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException)
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
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> OConnection::prepareStatement()" );
    SAL_INFO("connectivity.mork", "OConnection::prepareStatement( " << _sSql << " )");

    OSL_UNUSED( _sSql );
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
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> OConnection::prepareCall()" );
    SAL_INFO("connectivity.mork", "sql: " << _sSql);
    OSL_UNUSED( _sSql );
    ::dbtools::throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    SAL_INFO("connectivity.mork", "OConnection::prepareCall( " << _sSql << " )");
    return NULL;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::nativeSQL( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> OConnection::nativeSQL()" );
    SAL_INFO("connectivity.mork", "sql: " << _sSql);

    ::osl::MutexGuard aGuard( m_aMutex );
    // when you need to transform SQL92 to you driver specific you can do it here
    SAL_INFO("connectivity.mork", "OConnection::nativeSQL(" << _sSql << " )" );

    return _sSql;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setAutoCommit( sal_Bool /*autoCommit*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setAutoCommit", *this );
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    return sal_True;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException)
{
    // when you database does support transactions you should commit here
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    // same as commit but for the other case
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // just simple -> we are close when we are disposed taht means someone called dispose(); (XComponent)
    return OConnection_BASE::rBHelper.bDisposed;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException)
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
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setReadOnly( sal_Bool /*readOnly*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setReadOnly", *this );
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    // return if your connection to readonly
    return sal_False;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setCatalog( const ::rtl::OUString& /*catalog*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setCatalog", *this );
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTransactionIsolation", *this );
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
    return TransactionIsolation::NONE;
}
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    // if your driver has special database types you can return it here
    return NULL;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTypeMap", *this );
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException)
{
    // we just dispose us
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// --------------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    // when you collected some warnings -> return it
    return Any();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    // you should clear your collected warnings here
}
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    // we noticed that we should be destroied in near future so we have to dispose our statements
    ::osl::MutexGuard aGuard(m_aMutex);
    dispose_ChildImpl();
}
// -----------------------------------------------------------------------------

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
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
void OConnection::throwSQLException( const ErrorDescriptor& _rError, const Reference< XInterface >& _rxContext )
{
    if ( _rError.getResId() != 0 )
    {
        OSL_ENSURE( ( _rError.getErrorCondition() == 0 ),
            "OConnection::throwSQLException: unsupported error code combination!" );

        ::rtl::OUString sParameter( _rError.getParameter() );
        if ( !sParameter.isEmpty() )
        {
            const ::rtl::OUString sError( getResources().getResourceStringWithSubstitution(
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
        SQLError aErrorHelper( getDriver()->getFactory() );
        ::rtl::OUString sParameter( _rError.getParameter() );
        if ( !sParameter.isEmpty() )
            aErrorHelper.raiseException( _rError.getErrorCondition(), _rxContext, sParameter );
        else
            aErrorHelper.raiseException( _rError.getErrorCondition(), _rxContext);
        OSL_FAIL( "OConnection::throwSQLException: unreachable (3)!" );
    }

    throwGenericSQLException( STR_UNSPECIFIED_ERROR, _rxContext );
}

// -----------------------------------------------------------------------------
void OConnection::throwSQLException( const sal_uInt16 _nErrorResourceId, const Reference< XInterface >& _rxContext )
{
    ErrorDescriptor aError;
    aError.setResId( _nErrorResourceId );
    throwSQLException( aError, _rxContext );
}

} } // namespace connectivity::mork

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
