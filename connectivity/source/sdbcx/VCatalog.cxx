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

#include <comphelper/types.hxx>
#include <connectivity/sdbcx/VCatalog.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <TConnection.hxx>
#include <connectivity/dbtools.hxx>

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(OCatalog,"com.sun.star.comp.connectivity.OCatalog","com.sun.star.sdbcx.DatabaseDefinition")

OCatalog::OCatalog(const Reference< XConnection> &_xConnection) : OCatalog_BASE(m_aMutex)
{
    try
    {
        m_xMetaData = _xConnection->getMetaData();
    }
    catch(const Exception&)
    {
        OSL_FAIL("No Metadata available!");
    }
}

OCatalog::~OCatalog()
{
}

void SAL_CALL OCatalog::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pTables)
        m_pTables->disposing();
    if(m_pViews)
        m_pViews->disposing();
    if(m_pGroups)
        m_pGroups->disposing();
    if(m_pUsers)
        m_pUsers->disposing();

    OCatalog_BASE::disposing();
}

// XTablesSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getTables(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OCatalog_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pTables)
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

// XViewsSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getViews(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OCatalog_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pViews)
            refreshViews();
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

    return m_pViews.get();
}

// XUsersSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getUsers(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OCatalog_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pUsers)
            refreshUsers();
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

    return m_pUsers.get();
}

// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getGroups(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OCatalog_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pGroups)
            refreshGroups();
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

    return m_pGroups.get();
}

OUString OCatalog::buildName(const Reference< XRow >& _xRow)
{
    OUString sCatalog = _xRow->getString(1);
    if ( _xRow->wasNull() )
        sCatalog.clear();
    OUString sSchema  = _xRow->getString(2);
    if ( _xRow->wasNull() )
        sSchema.clear();
    OUString sTable   = _xRow->getString(3);
    if ( _xRow->wasNull() )
        sTable.clear();

    OUString sComposedName(
        ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, false, ::dbtools::EComposeRule::InDataManipulation ) );
    return sComposedName;
}

void OCatalog::fillNames(Reference< XResultSet >& _xResult,::std::vector< OUString>& _rNames)
{
    if ( _xResult.is() )
    {
        _rNames.reserve(20);
        Reference< XRow > xRow(_xResult,UNO_QUERY);
        while ( _xResult->next() )
        {
            _rNames.push_back( buildName(xRow) );
        }
        xRow.clear();
        ::comphelper::disposeComponent(_xResult);
    }
}

void ODescriptor::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : css::beans::PropertyAttribute::READONLY;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME), PROPERTY_ID_NAME ,nAttrib,&m_Name,::cppu::UnoType<OUString>::get());
}

ODescriptor::~ODescriptor()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
