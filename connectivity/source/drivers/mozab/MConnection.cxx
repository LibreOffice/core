/*************************************************************************
 *
 *  $RCSfile: MConnection.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 14:39:44 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "MConnection.hxx"

#include "MDatabaseMetaData.hxx"
#ifndef CONNECTIVITY_SDRIVER_HXX
#include "MDriver.hxx"
#endif
#ifndef _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
#include "MColumnAlias.hxx"
#endif
#include "MStatement.hxx"
#include "MPreparedStatement.hxx"

#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_TRANSACTIONISOLATION_HPP_
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _DBHELPER_DBCHARSET_HXX_
#include <connectivity/dbcharset.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

#ifdef _DEBUG
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* _DEBUG */
# define OUtoCStr( x ) ("dummy")
#endif /* _DEBUG */

extern "C" void*  SAL_CALL OMozabConnection_CreateInstance(void* _pDriver)
{
    return (new connectivity::mozab::OConnection( reinterpret_cast<connectivity::mozab::MozabDriver*>(_pDriver) ));
}


using namespace connectivity::mozab;
//  using namespace connectivity;
using namespace dbtools;

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
// --------------------------------------------------------------------------------


//------------------------------------------------------------------

namespace connectivity {
    namespace mozab {
        // For the moment, we will connect the Mozilla address book to the Mozilla
        // top-level address book which will display whatever is in the preferences
        // file of Mozilla.
        static sal_Char*    MOZ_SCHEME_MOZILLA          = "moz-abdirectory://";
        // This one is a base uri which will be completed with the connection data.
        static sal_Char*    MOZ_SCHEME_LDAP             = "moz-abldapdirectory://";
        // These two uris will be used to obtain directory factories to access all
        // address books of the given type.
        static sal_Char*    MOZ_SCHEME_OUTLOOK_MAPI     = "moz-aboutlookdirectory://op/";
        static sal_Char*    MOZ_SCHEME_OUTLOOK_EXPRESS  = "moz-aboutlookdirectory://oe/";
    }
}
// -----------------------------------------------------------------------------
const sal_Char* OConnection::getSDBC_SCHEME_MOZILLA()
{
    static sal_Char*    SDBC_SCHEME_MOZILLA         = MOZAB_MOZILLA_SCHEMA;
    return SDBC_SCHEME_MOZILLA;
}
// -----------------------------------------------------------------------------
const sal_Char* OConnection::getSDBC_SCHEME_LDAP()
{
    static sal_Char*    SDBC_SCHEME_LDAP            = MOZAB_LDAP_SCHEMA;
    return SDBC_SCHEME_LDAP;
}
// -----------------------------------------------------------------------------
const sal_Char* OConnection::getSDBC_SCHEME_OUTLOOK_MAPI()
{
    static sal_Char*    SDBC_SCHEME_OUTLOOK_MAPI    = MOZAB_OUTLOOK_SCHEMA;
    return SDBC_SCHEME_OUTLOOK_MAPI;
}
// -----------------------------------------------------------------------------
const sal_Char* OConnection::getSDBC_SCHEME_OUTLOOK_EXPRESS()
{
    static sal_Char*    SDBC_SCHEME_OUTLOOK_EXPRESS = MOZAB_OUTLOOKEXP_SCHEMA;
    return SDBC_SCHEME_OUTLOOK_EXPRESS;
}
// -----------------------------------------------------------------------------
::rtl::OUString OConnection::getDriverImplementationName()
{
    return rtl::OUString::createFromAscii(MOZAB_DRIVER_IMPL_NAME);
}
// -----------------------------------------------------------------------------

OConnection::OConnection(MozabDriver*   _pDriver)
                         : OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this),
                         m_pDriver(_pDriver),
                         m_xMetaData(NULL),
                         m_nAnonABCount( 0 ),
                         m_nMaxResultRecords( -1 ),
                         m_eSDBCAddressType(SDBCAddress::Unknown),
                         m_aNameMapper(NULL)
{
    m_pDriver->acquire();

    // Initialise m_aColumnAlias.
    m_aColumnAlias.setAlias(_pDriver->getMSFactory());
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
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
    ::rtl::OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    ::rtl::OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        // There isn't any subschema: - but could be just subschema
        if ( aAddrbookURI.getLength() > 0 ) {
            aAddrbookScheme= aAddrbookURI;
        }
        else {
            OSL_TRACE( "No subschema given!!!\n");
            ::dbtools::throwGenericSQLException(
                        ::rtl::OUString::createFromAscii("No subschema provided"),NULL);
        }
    }
    else {
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);
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
    if ( aAddrbookScheme.compareToAscii( getSDBC_SCHEME_MOZILLA() ) == 0 ) {
        m_sMozillaURI = rtl::OUString::createFromAscii( MOZ_SCHEME_MOZILLA );
        m_eSDBCAddressType = SDBCAddress::Mozilla;
    }
    else if ( aAddrbookScheme.compareToAscii( getSDBC_SCHEME_LDAP() ) == 0 ) {
        rtl::OUString sHostName;
        rtl::OUString sBaseDN;
        sal_Int32     nPortNumber = -1;

        m_sMozillaURI = rtl::OUString::createFromAscii( MOZ_SCHEME_LDAP );
        m_eSDBCAddressType = SDBCAddress::LDAP;

        const PropertyValue* pInfo = info.getConstArray();
        const PropertyValue* pInfoEnd = pInfo + info.getLength();

        for (; pInfo != pInfoEnd; ++pInfo)
        {
            OSL_TRACE( "info[%d].Name = %s\n", pInfo - info.getConstArray(), OUtoCStr( pInfo->Name ) );

            if ( 0 == pInfo->Name.compareToAscii("HostName") )
            {
                pInfo->Value >>= sHostName;
            }
            else if ( 0 == pInfo->Name.compareToAscii("BaseDN") )
            {
                pInfo->Value >>= sBaseDN;
            }
            else if ( 0 == pInfo->Name.compareToAscii("PortNumber") )
            {
                pInfo->Value >>= nPortNumber;
            }
            else if ( 0 == pInfo->Name.compareToAscii("MaxRowCount") )
            {
                pInfo->Value >>= m_nMaxResultRecords;
            }
        }
        if ( sHostName.getLength() != 0 ) {
            m_sMozillaURI += sHostName;
        }
        else {
            ::dbtools::throwGenericSQLException(
                        ::rtl::OUString::createFromAscii("No HostName provided"),NULL);
        }

        if ( nPortNumber > 0 ) {
            m_sMozillaURI += rtl::OUString::createFromAscii( ":" );
            m_sMozillaURI += rtl::OUString::valueOf( nPortNumber );
        }

        if ( sBaseDN.getLength() != 0 ) {
            m_sMozillaURI += rtl::OUString::createFromAscii( "/" );
            m_sMozillaURI += sBaseDN;
        }
        else {
            ::dbtools::throwGenericSQLException(
                        ::rtl::OUString::createFromAscii("No BaseDN provided"),NULL);
        }
        // Addition of a fake query to enable the Mozilla LDAP directory to work correctly.
        m_sMozillaURI += ::rtl::OUString::createFromAscii("?(or(DisplayName,=,DontDoThisAtHome))");

    }
    else if ( aAddrbookScheme.compareToAscii( getSDBC_SCHEME_OUTLOOK_MAPI() ) == 0 ) {
        m_sMozillaURI       = ::rtl::OUString::createFromAscii( MOZ_SCHEME_OUTLOOK_MAPI );
        m_eSDBCAddressType = SDBCAddress::Outlook;
    }
    else if ( aAddrbookScheme.compareToAscii( getSDBC_SCHEME_OUTLOOK_EXPRESS() ) == 0 ) {
        m_sMozillaURI       = rtl::OUString::createFromAscii( MOZ_SCHEME_OUTLOOK_EXPRESS );
        m_eSDBCAddressType = SDBCAddress::OutlookExp;
    }
    else
    {
        OSL_TRACE("Invalid subschema given!!!\n");
        ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("Invalid subschema provided"),NULL);
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
            ::dbtools::throwGenericSQLException( _aDbHelper.getErrorString(), NULL);
        }
        else {
            OSL_TRACE("testLDAPConnection : SUCCESS\n" );
        }
    }

    // Test connection by getting to get the Table Names
    ::std::vector< ::rtl::OUString > tables;
    if ( !_aDbHelper.getTableStrings( this, tables, sal_True ) ) {
        ::dbtools::throwGenericSQLException( _aDbHelper.getErrorString(), NULL);
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
    OSL_TRACE("OConnection::prepareCall( %s )", OUtoCStr( _sSql ) );
    // not implemented yet :-) a task to do
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
void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    // here you  have to set your commit mode please have a look at the jdbc documentation to get a clear explanation
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
void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    // set you connection to readonly
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    // return if your connection to readonly
    return sal_False;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setCatalog( const ::rtl::OUString& catalog ) throw(SQLException, RuntimeException)
{
    // if your database doesn't work with catalogs you go to next method otherwise you kjnow what to do
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    // return your current catalog
    return ::rtl::OUString();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    // set your isolation level
    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
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
void SAL_CALL OConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    // the other way around
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

    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();

    m_xMetaData = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData>();

    if ( m_aNameMapper ) {
        MQuery::FreeNameMapper( m_aNameMapper );
        m_aNameMapper = NULL;
    }

    dispose_ChildImpl();
    OConnection_BASE::disposing();
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



