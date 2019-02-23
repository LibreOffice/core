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

#include "MCatalog.hxx"
#include "MConnection.hxx"
#include "MTables.hxx"

#include <com/sun/star/sdbc/XRow.hpp>


using namespace connectivity::mork;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;


OCatalog::OCatalog(OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
{
//  osl_atomic_increment( &m_refCount );
//  refreshTables();
//  refreshViews();
//  refreshGroups();
//  refreshUsers();
//  osl_atomic_decrement( &m_refCount );
}

void OCatalog::refreshTables()
{
    ::std::vector< OUString> aVector;
    Sequence< OUString > aTypes { "%" };
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        "%", "%", aTypes);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        OUString aName;
        while(xResult->next())
        {
            aName = xRow->getString(3);
            aVector.push_back(aName);
        }
    }
    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables.reset( new OTables(m_xMetaData,*this,m_aMutex,aVector) );
}

void OCatalog::refreshViews()
{
}

void OCatalog::refreshGroups()
{
}

void OCatalog::refreshUsers()
{
}


// XTablesSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getTables(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    try
    {
        if(!m_pTables || OConnection::getForceLoadTables())
            refreshTables();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return m_pTables.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
