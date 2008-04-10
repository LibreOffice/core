/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MConnection.cxx,v $
 * $Revision: 1.28 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "MConnection.hxx"

#include "MDatabaseMetaData.hxx"
#include "MDriver.hxx"
#include "MColumnAlias.hxx"
#include "MStatement.hxx"
#include "MPreparedStatement.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <connectivity/dbcharset.hxx>
#include <connectivity/dbexception.hxx>
#include "diagnose_ex.h"

#include "resource/mozab_res.hrc"
#include <comphelper/officeresourcebundle.hxx>

#include <boost/shared_ptr.hpp>

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

extern "C" void*  SAL_CALL OMozabConnection_CreateInstance(void* _pDriver)
{
    return (new connectivity::mozab::OConnection( reinterpret_cast<connectivity::mozab::MozabDriver*>(_pDriver) ));
}

using namespace dbtools;

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
// --------------------------------------------------------------------------------

namespace connectivity { namespace mozab {

// =====================================================================
// = ConnectionImplData
// =====================================================================
struct ConnectionImplData
{
    ::boost::shared_ptr< ::comphelper::OfficeResourceBundle >   pResourceBundle;
};

// -----------------------------------------------------------------------------
#ifdef __MINGW32__
extern "C"
#endif
const sal_Char* getSdbcScheme( SdbcScheme _eScheme )
{
    switch ( _eScheme )
    {
        case SDBC_MOZILLA:          return "mozilla";
        case SDBC_THUNDERBIRD:      return "thunderbird";
        case SDBC_LDAP:             return "ldap";
        case SDBC_OUTLOOK_MAPI:     return "outlook";
        case SDBC_OUTLOOK_EXPRESS:  return "outlookexp";
    }
    return NULL;
}
// -----------------------------------------------------------------------------
::rtl::OUString OConnection::getDriverImplementationName()
{
    return rtl::OUString::createFromAscii(MOZAB_DRIVER_IMPL_NAME);
}

// -----------------------------------------------------------------------------
const sal_Char* getSchemeURI( MozillaScheme _eScheme )
{
    switch ( _eScheme )
    {
    case SCHEME_MOZILLA          : return "moz-abdirectory://";
    case SCHEME_MOZILLA_MDB      : return "moz-abmdbdirectory://";
    case SCHEME_LDAP             : return "moz-abldapdirectory://";
    case SCHEME_OUTLOOK_MAPI     : return "moz-aboutlookdirectory://op/";
    case SCHEME_OUTLOOK_EXPRESS  : return "moz-aboutlookdirectory://oe/";
    }
    return NULL;
}

// -----------------------------------------------------------------------------

OConnection::OConnection(MozabDriver*   _pDriver)
    :OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this)
    ,m_pDriver(_pDriver)
    ,m_pImplData( new ConnectionImplData )
    ,m_aColumnAlias( _pDriver->getMSFactory() )
    ,m_nMaxResultRecords( -1 )
    ,m_aNameMapper(NULL)
    ,m_eSDBCAddressType(SDBCAddress::Unknown)
{
    m_pDriver->acquire();

    try
    {
        Reference< XPropertySet > xFactoryProps( m_pDriver->getMSFactory(), UNO_QUERY_THROW );
        Reference< XComponentContext > xContext(
            xFactoryProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ),
            UNO_QUERY_THROW
        );
        m_pImplData->pResourceBundle.reset( new ::comphelper::OfficeResourceBundle( xContext, "cnr" ) );
    }
    catch( const Exception& )
    {
        OSL_ENSURE( false, "OConnection::OConnection: could not obtain the component context!" );
    }
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
    acquire();
    if(!isClosed())
        close();
    m_pDriver->release();
    m_pDriver = NULL;
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
    OSL_TRACE("IN OConnection::construct()\n" );
    //  open file
    setURL(url);
    //
    // Skip 'sdbc:mozab: part of URL
    //
    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    OSL_ENSURE( url.copy( 0, nLen ).equalsAscii( "sdbc:address" ), "OConnection::construct: invalid start of the URI - should never have survived XDriver::acceptsURL!" );

    ::rtl::OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    ::rtl::OUString aAddrbookScheme;
    ::rtl::OUString sAdditionalInfo;
    if ( nLen == -1 )
    {
        // There isn't any subschema: - but could be just subschema
        if ( aAddrbookURI.getLength() > 0 )
        {
            aAddrbookScheme= aAddrbookURI;
        }
        else
        {
            OSL_TRACE( "No subschema given!!!\n");
            throwGenericSQLException( STR_URI_SYNTAX_ERROR );
        }
    }
    else
    {
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);
        sAdditionalInfo = aAddrbookURI.copy( nLen + 1 );
    }

    OSL_TRACE("URI = %s\n", ((OUtoCStr(aAddrbookURI)) ? (OUtoCStr(aAddrbookURI)):("NULL")) );
    OSL_TRACE("Scheme = %s\n", ((OUtoCStr(aAddrbookScheme)) ?  (OUtoCStr(aAddrbookScheme)):("NULL")) );

    //
    // Now we have a URI convert it to a MozillaURI
    //
    // The Mapping being used is:
    //
    // * for Mozilla
    //      "sdbc:address:mozilla:"        -> abdirectory://
    // * for LDAP
    //      "sdbc:address:ldap:"           -> abldapdirectory://
    // * for Outlook (using MAPI API)
    //      "sdbc:address:outlook:"        -> aboutlookdirectory://op/
    // * for windows system address book
    //      "sdbc:address:outlookexp:"     -> aboutlookdirectory://oe/
    //
        m_sBindDN   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(""));
        m_sPassword = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(""));
        m_bUseSSL   = sal_False;

    if ( aAddrbookScheme.compareToAscii( getSdbcScheme( SDBC_MOZILLA ) ) == 0 ) {
        m_sMozillaURI = rtl::OUString::createFromAscii( getSchemeURI( SCHEME_MOZILLA ) );
        m_eSDBCAddressType = SDBCAddress::Mozilla;
        if(sAdditionalInfo.getLength())
            m_sMozillaProfile = sAdditionalInfo;
    }
    else
    if ( aAddrbookScheme.compareToAscii( getSdbcScheme( SDBC_THUNDERBIRD ) ) == 0 ) {
        //Yes. I am sure it is SCHEME_MOZILLA
        m_sMozillaURI = rtl::OUString::createFromAscii( getSchemeURI( SCHEME_MOZILLA ) );
        m_eSDBCAddressType = SDBCAddress::ThunderBird;
        if(sAdditionalInfo.getLength())
            m_sMozillaProfile = sAdditionalInfo;
    }
    else if ( aAddrbookScheme.compareToAscii( getSdbcScheme( SDBC_LDAP ) ) == 0 ) {
        rtl::OUString sBaseDN;
        sal_Int32     nPortNumber = -1;

        m_sMozillaURI = rtl::OUString::createFromAscii( getSchemeURI( SCHEME_LDAP ) );
        m_eSDBCAddressType = SDBCAddress::LDAP;

        if ( !m_sHostName.getLength() )
        {
            // see whether the URI contains a hostname/port
            if ( sAdditionalInfo.getLength() )
            {
                sal_Int32 nPortSeparator = sAdditionalInfo.indexOf( ':' );
                if ( nPortSeparator == -1 )
                    m_sHostName = sAdditionalInfo;
                else
                {
                    m_sHostName = sAdditionalInfo.copy( 0, nPortSeparator );
                    nPortNumber = sAdditionalInfo.copy( nPortSeparator + 1 ).toInt32();
                    OSL_ENSURE( nPortNumber != 0, "OConnection::construct: invalid LDAP port number in the URL!" );
                    if ( nPortNumber == 0 )
                        nPortNumber = -1;
                }
            }
        }

        const PropertyValue* pInfo = info.getConstArray();
        const PropertyValue* pInfoEnd = pInfo + info.getLength();

        for (; pInfo != pInfoEnd; ++pInfo)
        {
            OSL_TRACE( "info[%d].Name = %s\n", pInfo - info.getConstArray(), OUtoCStr( pInfo->Name ) );

            if ( 0 == pInfo->Name.compareToAscii("HostName") )
            {
                pInfo->Value >>= m_sHostName;
            }
            else if ( 0 == pInfo->Name.compareToAscii("BaseDN") )
            {
                pInfo->Value >>= sBaseDN;
            }
            else if ( 0 == pInfo->Name.compareToAscii("user") )
            {
                pInfo->Value >>= m_sBindDN;
            }
            else if ( 0 == pInfo->Name.compareToAscii("password") )
            {
                pInfo->Value >>= m_sPassword;
            }
            else if ( 0 == pInfo->Name.compareToAscii("UseSSL") )
            {
                pInfo->Value >>= m_bUseSSL;
            }
            else if ( 0 == pInfo->Name.compareToAscii("PortNumber") )
            {
                OSL_VERIFY( pInfo->Value >>= nPortNumber );
            }
            else if ( 0 == pInfo->Name.compareToAscii("MaxRowCount") )
            {
                pInfo->Value >>= m_nMaxResultRecords;
            }
        }

        if ( m_sHostName.getLength() != 0 ) {
            m_sMozillaURI += m_sHostName;
        }
        else
            throwGenericSQLException( STR_NO_HOSTNAME );

        if ( nPortNumber > 0 ) {
            m_sMozillaURI += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(":") );
            m_sMozillaURI += rtl::OUString::valueOf( nPortNumber );
        }

        if ( sBaseDN.getLength() != 0 ) {
            m_sMozillaURI += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") );
            m_sMozillaURI += sBaseDN;
        }
        else
            throwGenericSQLException( STR_NO_BASEDN );

        // Addition of a fake query to enable the Mozilla LDAP directory to work correctly.
        m_sMozillaURI += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("?(or(DisplayName,=,DontDoThisAtHome)))"));

    }
    else if ( aAddrbookScheme.compareToAscii( getSdbcScheme( SDBC_OUTLOOK_MAPI ) ) == 0 ) {
        m_sMozillaURI       = ::rtl::OUString::createFromAscii( getSchemeURI( SCHEME_OUTLOOK_MAPI ) );
        m_eSDBCAddressType = SDBCAddress::Outlook;
    }
    else if ( aAddrbookScheme.compareToAscii( getSdbcScheme( SDBC_OUTLOOK_EXPRESS ) ) == 0 ) {
        m_sMozillaURI       = rtl::OUString::createFromAscii( getSchemeURI( SCHEME_OUTLOOK_EXPRESS ) );
        m_eSDBCAddressType = SDBCAddress::OutlookExp;
    }
    else
    {
        OSL_TRACE("Invalid subschema given!!!\n");
        throwGenericSQLException( STR_URI_SYNTAX_ERROR );
    }

    OSL_TRACE("Moz URI = %s, %s\n", ((OUtoCStr(m_sMozillaURI)) ? (OUtoCStr(m_sMozillaURI)):("NULL")), usesFactory() ? "uses factory" : "no factory");
    OSL_TRACE( "\tOUT OConnection::construct()\n" );

    MDatabaseMetaDataHelper     _aDbHelper;

    // The creation of the nsIAbDirectory i/f for LDAP doesn't actually test
    // the validity of the connection, it's normally delayed until the query
    // is executed, but it's a bit late then to fail...
    if ( isLDAP() ) {
        if ( !_aDbHelper.testLDAPConnection( this ) ) {
            OSL_TRACE("testLDAPConnection : FAILED\n" );
            throwGenericSQLException( _aDbHelper.getErrorResourceId() );
        }
        else {
            OSL_TRACE("testLDAPConnection : SUCCESS\n" );
        }
    }

    // Test connection by getting to get the Table Names
    ::std::vector< ::rtl::OUString > tables;
    ::std::vector< ::rtl::OUString > types;
    if ( !_aDbHelper.getTableStrings( this, tables, types ) ) {
        throwGenericSQLException( _aDbHelper.getErrorResourceId() );
    }

}
// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.mozab.OConnection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
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
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    OSL_TRACE("OConnection::prepareStatement( %s )", OUtoCStr( _sSql ) );
    // the pre
    // create a statement
    // the statement can only be executed more than once
    Reference< XPreparedStatement > xReturn = new OPreparedStatement(this,_sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    OSL_UNUSED( _sSql );
    ::dbtools::throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    OSL_TRACE("OConnection::prepareCall( %s )", OUtoCStr( _sSql ) );
    return NULL;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::nativeSQL( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // when you need to transform SQL92 to you driver specific you can do it here
    OSL_TRACE("OConnection::nativeSQL( %s )", OUtoCStr( _sSql ) );

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
    // return your current catalog
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

    m_pImplData->pResourceBundle.reset();

    OConnection_BASE::disposing();
    if ( m_aNameMapper ) {
        MQuery::FreeNameMapper( m_aNameMapper );
        m_aNameMapper = NULL;
    }

    dispose_ChildImpl();
}
// -----------------------------------------------------------------------------

Reference< XTablesSupplier > SAL_CALL OConnection::createCatalog()
{
    OSL_TRACE("IN OConnection::createCatalog()\n" );
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!m_xCatalog.is())
    {
        OCatalog *pCat = new OCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    OSL_TRACE( "\tOUT OConnection::createCatalog()\n" );
    return xTab;
}
// -----------------------------------------------------------------------------

MNameMapper* OConnection::getNameMapper ()
{
    if (m_aNameMapper==NULL)
        m_aNameMapper = MQuery::CreateNameMapper();

    return m_aNameMapper;
}
// -----------------------------------------------------------------------------

void OConnection::throwGenericSQLException( sal_Int32 _nErrorResourceId )
{
    ::boost::shared_ptr< ::comphelper::OfficeResourceBundle > pResourceBundle;
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        pResourceBundle = m_pImplData->pResourceBundle;
    }

    OSL_ENSURE( pResourceBundle.get(), "OConnection::throwGenericSQLException: no resource bundle?" );
        // this means that we're disposed, and how could anybody request us to throw an exception then?

    ::rtl::OUString sErrorMessage;
    if ( pResourceBundle.get() && _nErrorResourceId )
        sErrorMessage = pResourceBundle->loadString( _nErrorResourceId );
    ::dbtools::throwGenericSQLException( sErrorMessage, *this );
}

} } // namespace connectivity::mozab
