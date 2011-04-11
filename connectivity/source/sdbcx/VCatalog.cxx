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
#include "connectivity/sdbcx/VCatalog.hxx"
#include "connectivity/sdbcx/VCollection.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/sdbcx/VCollection.hxx"
#include "connectivity/sdbcx/VDescriptor.hxx"
#include "TConnection.hxx"
#include <comphelper/uno3.hxx>
#include "connectivity/dbtools.hxx"

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OCatalog,"com.sun.star.comp.connectivity.OCatalog","com.sun.star.sdbcx.DatabaseDefinition")
//------------------------------------------------------------------------------
OCatalog::OCatalog(const Reference< XConnection> &_xConnection) : OCatalog_BASE(m_aMutex)
            ,connectivity::OSubComponent<OCatalog, OCatalog_BASE>(_xConnection, this)
            ,m_pTables(NULL)
            ,m_pViews(NULL)
            ,m_pGroups(NULL)
            ,m_pUsers(NULL)
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
//-----------------------------------------------------------------------------
OCatalog::~OCatalog()
{
    delete m_pTables;
    delete m_pViews;
    delete m_pGroups;
    delete m_pUsers;
}
//-----------------------------------------------------------------------------
void SAL_CALL OCatalog::acquire() throw()
{
    OCatalog_BASE::acquire();
}
//------------------------------------------------------------------------------
void SAL_CALL OCatalog::release() throw()
{
    relase_ChildImpl();
}

//------------------------------------------------------------------------------
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

    dispose_ChildImpl();
    OCatalog_BASE::disposing();
}
//------------------------------------------------------------------------------
// XTablesSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getTables(  ) throw(RuntimeException)
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

    return const_cast<OCatalog*>(this)->m_pTables;
}
// -------------------------------------------------------------------------
// XViewsSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getViews(  ) throw(RuntimeException)
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

    return const_cast<OCatalog*>(this)->m_pViews;
}
// -------------------------------------------------------------------------
// XUsersSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getUsers(  ) throw(RuntimeException)
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

    return const_cast<OCatalog*>(this)->m_pUsers;
}
// -------------------------------------------------------------------------
// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getGroups(  ) throw(RuntimeException)
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

    return const_cast<OCatalog*>(this)->m_pGroups;
}
// -----------------------------------------------------------------------------
::rtl::OUString OCatalog::buildName(const Reference< XRow >& _xRow)
{
    ::rtl::OUString sCatalog = _xRow->getString(1);
    if ( _xRow->wasNull() )
        sCatalog = ::rtl::OUString();
    ::rtl::OUString sSchema  = _xRow->getString(2);
    if ( _xRow->wasNull() )
        sSchema = ::rtl::OUString();
    ::rtl::OUString sTable   = _xRow->getString(3);
    if ( _xRow->wasNull() )
        sTable = ::rtl::OUString();

    ::rtl::OUString sComposedName(
        ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, sal_False, ::dbtools::eInDataManipulation ) );
    return sComposedName;
}
// -----------------------------------------------------------------------------
void OCatalog::fillNames(Reference< XResultSet >& _xResult,TStringVector& _rNames)
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
// -------------------------------------------------------------------------
void ODescriptor::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : ::com::sun::star::beans::PropertyAttribute::READONLY;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME), PROPERTY_ID_NAME ,nAttrib,&m_Name,::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -------------------------------------------------------------------------
ODescriptor::~ODescriptor()
{
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
