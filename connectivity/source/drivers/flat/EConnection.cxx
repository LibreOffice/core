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

#include "flat/EConnection.hxx"
#include "flat/EDatabaseMetaData.hxx"
#include "flat/ECatalog.hxx"
#include "flat/EDriver.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "flat/EPreparedStatement.hxx"
#include "flat/EStatement.hxx"
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>

using namespace connectivity::flat;
using namespace connectivity::file;

typedef connectivity::file::OConnection  OConnection_B;

//------------------------------------------------------------------------------
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

// --------------------------------------------------------------------------------
OFlatConnection::OFlatConnection(ODriver*   _pDriver) : OConnection(_pDriver)
    ,m_nMaxRowsToScan(50)
    ,m_bHeaderLine(sal_True)
    ,m_cFieldDelimiter(';')
    ,m_cStringDelimiter('"')
    ,m_cDecimalDelimiter(',')
    ,m_cThousandDelimiter('.')
{
}
//-----------------------------------------------------------------------------
OFlatConnection::~OFlatConnection()
{
}

// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OFlatConnection, "com.sun.star.sdbc.drivers.flat.Connection", "com.sun.star.sdbc.Connection")

//-----------------------------------------------------------------------------
void OFlatConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    osl_atomic_increment( &m_refCount );

    const PropertyValue *pBegin  = info.getConstArray();
    const PropertyValue *pEnd    = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!pBegin->Name.compareToAscii("HeaderLine"))
            OSL_VERIFY( pBegin->Value >>= m_bHeaderLine );
        else if(!pBegin->Name.compareToAscii("FieldDelimiter"))
        {
            ::rtl::OUString aVal;
            OSL_VERIFY( pBegin->Value >>= aVal );
            m_cFieldDelimiter = aVal.toChar();
        }
        else if(!pBegin->Name.compareToAscii("StringDelimiter"))
        {
            ::rtl::OUString aVal;
            OSL_VERIFY( pBegin->Value >>= aVal );
            m_cStringDelimiter = aVal.toChar();
        }
        else if(!pBegin->Name.compareToAscii("DecimalDelimiter"))
        {
            ::rtl::OUString aVal;
            OSL_VERIFY( pBegin->Value >>= aVal );
            m_cDecimalDelimiter = aVal.toChar();
        }
        else if(!pBegin->Name.compareToAscii("ThousandDelimiter"))
        {
            ::rtl::OUString aVal;
            OSL_VERIFY( pBegin->Value >>= aVal );
            m_cThousandDelimiter = aVal.toChar();
        }
        else if ( !pBegin->Name.compareToAscii("MaxRowScan") )
        {
            pBegin->Value >>= m_nMaxRowsToScan;
        }
    }

    osl_atomic_decrement( &m_refCount );
    OConnection::construct(url,info);
    m_bShowDeleted = sal_True; // we do not supported rows for this type
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OFlatConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new OFlatDatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< XTablesSupplier > OFlatConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        OFlatCatalog *pCat = new OFlatCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OFlatConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);

    OFlatStatement* pStmt = new OFlatStatement(this);

    Reference< XStatement > xStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OFlatConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);


    OFlatPreparedStatement* pStmt = new OFlatPreparedStatement(this);
    Reference< XPreparedStatement > xStmt = pStmt;
    pStmt->construct(sql);

    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OFlatConnection::prepareCall( const ::rtl::OUString& /*sql*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    return NULL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
