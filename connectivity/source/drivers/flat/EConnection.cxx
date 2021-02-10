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

#include <flat/EConnection.hxx>
#include <flat/EDatabaseMetaData.hxx>
#include <flat/ECatalog.hxx>
#include <flat/EDriver.hxx>
#include <flat/EPreparedStatement.hxx>
#include <flat/EStatement.hxx>
#include <connectivity/dbexception.hxx>
#include <sal/log.hxx>

using namespace connectivity::flat;
using namespace connectivity::file;

typedef connectivity::file::OConnection  OConnection_B;


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;


OFlatConnection::OFlatConnection(ODriver*   _pDriver) : OConnection(_pDriver)
    ,m_nMaxRowsToScan(50)
    ,m_bHeaderLine(true)
    ,m_cFieldDelimiter(';')
    ,m_cStringDelimiter('"')
    ,m_cDecimalDelimiter(',')
    ,m_cThousandDelimiter('.')
{
}

OFlatConnection::~OFlatConnection()
{
}

// XServiceInfo

IMPLEMENT_SERVICE_INFO(OFlatConnection, "com.sun.star.sdbc.drivers.flat.Connection", "com.sun.star.sdbc.Connection")


void OFlatConnection::construct(const OUString& url,const Sequence< PropertyValue >& info)
{
    osl_atomic_increment( &m_refCount );

    const PropertyValue *pBegin  = info.getConstArray();
    const PropertyValue *pEnd    = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(pBegin->Name == "HeaderLine")
        {
            if( ! (pBegin->Value >>= m_bHeaderLine) )
                SAL_WARN("connectivity.flat", "construct: unable to get property HeaderLine");
        }
        else if(pBegin->Name == "FieldDelimiter")
        {
            OUString aVal;
            if( ! (pBegin->Value >>= aVal) )
                SAL_WARN("connectivity.flat", "construct: unable to get property FieldDelimiter");

            m_cFieldDelimiter = aVal.toChar();
        }
        else if(pBegin->Name == "StringDelimiter")
        {
            OUString aVal;
            if( ! (pBegin->Value >>= aVal) )
                SAL_WARN("connectivity.flat", "construct: unable to get property StringDelimiter");

            m_cStringDelimiter = aVal.toChar();
        }
        else if(pBegin->Name == "DecimalDelimiter")
        {
            OUString aVal;
            if( ! (pBegin->Value >>= aVal) )
                SAL_WARN("connectivity.flat", "construct: unable to get property DecimalDelimiter");

            m_cDecimalDelimiter = aVal.toChar();
        }
        else if(pBegin->Name == "ThousandDelimiter")
        {
            OUString aVal;
            if( ! (pBegin->Value >>= aVal) )
                SAL_WARN("connectivity.flat", "construct: unable to get property ThousandDelimiter");

            m_cThousandDelimiter = aVal.toChar();
        }
        else if ( pBegin->Name == "MaxRowScan" )
        {
            pBegin->Value >>= m_nMaxRowsToScan;
        }
    }

    osl_atomic_decrement( &m_refCount );
    OConnection::construct(url,info);
    m_bShowDeleted = true; // we do not supported rows for this type
}

Reference< XDatabaseMetaData > SAL_CALL OFlatConnection::getMetaData(  )
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

css::uno::Reference< XTablesSupplier > OFlatConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        xTab = new OFlatCatalog(this);
        m_xCatalog = xTab;
    }
    return xTab;
}

Reference< XStatement > SAL_CALL OFlatConnection::createStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);

    rtl::Reference<OFlatStatement> pStmt = new OFlatStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}

Reference< XPreparedStatement > SAL_CALL OFlatConnection::prepareStatement( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);

    rtl::Reference<OFlatPreparedStatement> pStmt = new OFlatPreparedStatement(this);
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}

Reference< XPreparedStatement > SAL_CALL OFlatConnection::prepareCall( const OUString& /*sql*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::prepareCall", *this );
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
