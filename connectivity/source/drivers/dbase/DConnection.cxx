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


#include "dbase/DConnection.hxx"
#include "dbase/DDatabaseMetaData.hxx"
#include "dbase/DCatalog.hxx"
#include "dbase/DDriver.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "dbase/DPreparedStatement.hxx"
#include "dbase/DStatement.hxx"
#include <connectivity/dbexception.hxx>

using namespace connectivity::dbase;
using namespace connectivity::file;

typedef connectivity::file::OConnection  OConnection_BASE;


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

ODbaseConnection::ODbaseConnection(ODriver* _pDriver) : OConnection(_pDriver)
{
    m_aFilenameExtension = "dbf";
}

ODbaseConnection::~ODbaseConnection()
{
}

// XServiceInfo

IMPLEMENT_SERVICE_INFO(ODbaseConnection, "com.sun.star.sdbc.drivers.dbase.Connection", "com.sun.star.sdbc.Connection")


Reference< XDatabaseMetaData > SAL_CALL ODbaseConnection::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new ODbaseDatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

::com::sun::star::uno::Reference< XTablesSupplier > ODbaseConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        ODbaseCatalog *pCat = new ODbaseCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}

Reference< XStatement > SAL_CALL ODbaseConnection::createStatement(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XStatement > xReturn = new ODbaseStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL ODbaseConnection::prepareStatement( const OUString& sql ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    ODbasePreparedStatement* pStmt = new ODbasePreparedStatement(this);
    Reference< XPreparedStatement > xHoldAlive = pStmt;
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}

Reference< XPreparedStatement > SAL_CALL ODbaseConnection::prepareCall( const OUString& /*sql*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::prepareCall", *this );
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
