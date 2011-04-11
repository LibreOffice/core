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
#include "flat/EConnection.hxx"
#include "flat/EDatabaseMetaData.hxx"
#include "flat/ECatalog.hxx"
#include "flat/EDriver.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <tools/urlobj.hxx>
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
    osl_incrementInterlockedCount( &m_refCount );

    ::rtl::OUString aExt;
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

    osl_decrementInterlockedCount( &m_refCount );
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
