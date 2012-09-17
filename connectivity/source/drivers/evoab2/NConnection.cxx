/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "NConnection.hxx"
#include "NDatabaseMetaData.hxx"
#include "NCatalog.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include "NPreparedStatement.hxx"
#include "NStatement.hxx"
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/ustring.hxx>

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
         aExceptionType =  ::rtl::OUString("Unknown") ;

     ::rtl::OUString aTypeLine( "\nType: "  );
     aTypeLine += aExceptionType;

     ::rtl::OUString aMessageLine( "\nMessage: "  );
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
    SAL_INFO("evoab2", "OEvoabConnection::construct()::url = " << url );

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

    if ( url == "sdbc:address:evolution:groupwise" )
        setSDBCAddressType(SDBCAddress::EVO_GWISE);
    else if ( url == "sdbc:address:evolution:ldap" )
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
