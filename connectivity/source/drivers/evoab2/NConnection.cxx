/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "NConnection.hxx"
#include "NDatabaseMetaData.hxx"
#include "NCatalog.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <tools/urlobj.hxx>
#include "NPreparedStatement.hxx"
#include "NStatement.hxx"
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include "NDebug.hxx"
#include <comphelper/sequence.hxx>

using namespace connectivity::evoab;
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
         aExceptionType =  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown")) ;

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
OEvoabConnection::OEvoabConnection( OEvoabDriver& _rDriver )
    :OSubComponent<OEvoabConnection, OConnection_BASE>( (::cppu::OWeakObject*)(&_rDriver), this )
    ,m_rDriver(_rDriver)
    ,m_xCatalog(NULL)
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

    if (url.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sdbc:address:evolution:groupwise")))
        setSDBCAddressType(SDBCAddress::EVO_GWISE);
    else if (url.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sdbc:address:evolution:ldap")))
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

    OEvoabPreparedStatement* pStmt = new OEvoabPreparedStatement( this );
    Reference< XPreparedStatement > xStmt = pStmt;
    pStmt->construct( sql );

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
    return m_aWarnings.getWarnings();
}
void SAL_CALL OEvoabConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    m_aWarnings.clearWarnings();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
