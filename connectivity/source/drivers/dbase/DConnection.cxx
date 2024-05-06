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


#include <dbase/DConnection.hxx>
#include <dbase/DDatabaseMetaData.hxx>
#include <dbase/DCatalog.hxx>
#include <dbase/DDriver.hxx>
#include <dbase/DPreparedStatement.hxx>
#include <dbase/DStatement.hxx>
#include <connectivity/dbexception.hxx>

using namespace connectivity::dbase;
using namespace connectivity::file;

typedef connectivity::file::OConnection  OConnection_BASE;


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;

ODbaseConnection::ODbaseConnection(ODriver* _pDriver) : OConnection(_pDriver)
{
    m_aFilenameExtension = "dbf";
}

ODbaseConnection::~ODbaseConnection()
{
}

// XServiceInfo

IMPLEMENT_SERVICE_INFO(ODbaseConnection, u"com.sun.star.sdbc.drivers.dbase.Connection"_ustr, u"com.sun.star.sdbc.Connection"_ustr)


Reference< XDatabaseMetaData > SAL_CALL ODbaseConnection::getMetaData(  )
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

css::uno::Reference< XTablesSupplier > ODbaseConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        xTab = new ODbaseCatalog(this);
        m_xCatalog = xTab;
    }
    return xTab;
}

Reference< XStatement > SAL_CALL ODbaseConnection::createStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XStatement > xReturn = new ODbaseStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL ODbaseConnection::prepareStatement( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    rtl::Reference<ODbasePreparedStatement> pStmt = new ODbasePreparedStatement(this);
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}

Reference< XPreparedStatement > SAL_CALL ODbaseConnection::prepareCall( const OUString& /*sql*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( u"XConnection::prepareCall"_ustr, *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
