 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NConnection.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:51:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#ifndef _CONNECTIVITY_EVOAB_CONNECTION_HXX_
#include "NConnection.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_DATABASEMETADATA_HXX_
#include "NDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_CATALOG_HXX_
#include "NCatalog.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_TRANSACTIONISOLATION_HPP_
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_EVOAB_PREPAREDSTATEMENT_HXX_
#include "NPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_STATEMENT_HXX_
#include "NStatement.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef CONNECTIVITY_EVOAB_DEBUG_HELPER_HXX
#include "NDebug.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

using namespace connectivity::evoab;
using namespace vos;
using namespace dbtools;

//------------------------------------------------------------------------------
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

::rtl::OUString implGetExceptionMsg( Exception& e, const ::rtl::OUString& aExceptionType_ )
{
     ::rtl::OUString aExceptionType = aExceptionType_;
     if( aExceptionType.getLength() == 0 )
         aExceptionType =  ::rtl::OUString::createFromAscii("Unknown" ) ;

     ::rtl::OUString aTypeLine( RTL_CONSTASCII_USTRINGPARAM("\nType: " ) );
     aTypeLine += aExceptionType;

     ::rtl::OUString aMessageLine( RTL_CONSTASCII_USTRINGPARAM("\nMessage: " ) );
         aMessageLine += ::rtl::OUString( e.Message );

     ::rtl::OUString aMsg(aTypeLine);
     aMsg += aMessageLine;
         return aMsg;
}

 // Exception type unknown
::rtl::OUString implGetExceptionMsg( Exception& e )
{
         ::rtl::OUString aMsg = implGetExceptionMsg( e, ::rtl::OUString() );
         return aMsg;
}

// --------------------------------------------------------------------------------
OEvoabConnection::OEvoabConnection(OEvoabDriver*    _pDriver)
    :OSubComponent<OEvoabConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this)
    ,m_pDriver(_pDriver)
    ,m_xCatalog(NULL)
    ,m_aPassword()
{
}
//-----------------------------------------------------------------------------
OEvoabConnection::~OEvoabConnection()
{
        ::osl::MutexGuard aGuard( m_aMutex );

    if(!isClosed()) {
        acquire();
        close();
    }

    m_pDriver = NULL;
}

//-----------------------------------------------------------------------------
void SAL_CALL OEvoabConnection::release() throw()
{
    relase_ChildImpl();
}

// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OEvoabConnection, "com.sun.star.sdbc.drivers.evoab.Connection", "com.sun.star.sdbc.Connection")

//-----------------------------------------------------------------------------
void OEvoabConnection::construct(const ::rtl::OUString& url, const Sequence< PropertyValue >& info)  throw(SQLException)
{
    osl_incrementInterlockedCount( &m_refCount );
    EVO_TRACE_STRING("OEvoabConnection::construct()::url = %s\n", url );

     ::rtl::OUString sPassword;
        const char* pPwd                = "password";

        const PropertyValue *pIter      = info.getConstArray();
        const PropertyValue *pEnd       = pIter + info.getLength();
        for(;pIter != pEnd;++pIter)
        {
                if(!pIter->Name.compareToAscii(pPwd))
                {
                        pIter->Value >>= sPassword;
                        break;
                }
        }

    if (url.equalsAscii("sdbc:address:evolution:groupwise"))
        setSDBCAddressType(SDBCAddress::EVO_GWISE);
    else if (url.equalsAscii("sdbc:address:evolution:ldap"))
        setSDBCAddressType(SDBCAddress::EVO_LDAP);
    else
        setSDBCAddressType(SDBCAddress::EVO_LOCAL);
    setURL(url);
    setPassword(::rtl::OUStringToOString(sPassword,RTL_TEXTENCODING_UTF8));
    osl_decrementInterlockedCount( &m_refCount );
}

// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabConnection::nativeSQL( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    // when you need to transform SQL92 to you driver specific you can do it here
    return _sSql;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OEvoabConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new OEvoabDatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< XTablesSupplier > OEvoabConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
     Reference< XTablesSupplier > xTab = m_xCatalog;
     if(!xTab.is())
     {
         OEvoabCatalog *pCat = new OEvoabCatalog(this);
         xTab = pCat;
         m_xCatalog = xTab;
     }
     return xTab;
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OEvoabConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    OStatement* pStmt = new OStatement(this);

    Reference< XStatement > xStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OEvoabConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    OEvoabPreparedStatement* pStmt = new OEvoabPreparedStatement(this, sql);
    Reference< XPreparedStatement > xStmt = pStmt;

    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}

Reference< XPreparedStatement > SAL_CALL OEvoabConnection::prepareCall( const ::rtl::OUString& /*sql*/ ) throw( SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    return NULL;
}
sal_Bool SAL_CALL OEvoabConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return OConnection_BASE::rBHelper.bDisposed;
}

// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OEvoabConnection::close(  ) throw(SQLException, RuntimeException)
{
    {  // we just dispose us
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);
    }
    dispose();
}

// --------------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL OEvoabConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();   // when you collected some warnings -> return it
}
void SAL_CALL OEvoabConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    // you should clear your collected warnings here
}
//------------------------------------------------------------------------------

void OEvoabConnection::disposing()
{
    // we noticed that we should be destroyed in near future so we have to dispose our statements
    ::osl::MutexGuard aGuard(m_aMutex);
    OConnection_BASE::disposing();
    dispose_ChildImpl();
}

// -------------------------------- stubbed methods ------------------------------------------------
void SAL_CALL OEvoabConnection::setAutoCommit( sal_Bool /*autoCommit*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setAutoCommit", *this );
}
sal_Bool SAL_CALL OEvoabConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
void SAL_CALL OEvoabConnection::commit(  ) throw(SQLException, RuntimeException)
{
}
void SAL_CALL OEvoabConnection::rollback(  ) throw(SQLException, RuntimeException)
{
}
void SAL_CALL OEvoabConnection::setReadOnly( sal_Bool /*readOnly*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setReadOnly", *this );
}
sal_Bool SAL_CALL OEvoabConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
void SAL_CALL OEvoabConnection::setCatalog( const ::rtl::OUString& /*catalog*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setCatalog", *this );
}

::rtl::OUString SAL_CALL OEvoabConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
void SAL_CALL OEvoabConnection::setTransactionIsolation( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTransactionIsolation", *this );
}

sal_Int32 SAL_CALL OEvoabConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    return TransactionIsolation::NONE;
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OEvoabConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::getTypeMap", *this );
    return NULL;
}
void SAL_CALL OEvoabConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTypeMap", *this );
}
