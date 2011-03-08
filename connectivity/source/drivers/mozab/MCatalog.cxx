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
#include "MCatalog.hxx"
#include "MConnection.hxx"
#include "MTables.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <cppuhelper/interfacecontainer.h>

// -------------------------------------------------------------------------
using namespace connectivity::mozab;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::cppu;

// -------------------------------------------------------------------------
OCatalog::OCatalog(OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
                ,m_xMetaData(m_pConnection->getMetaData(  ))
{
//  osl_incrementInterlockedCount( &m_refCount );
//  refreshTables();
//  refreshViews();
//  refreshGroups();
//  refreshUsers();
//  osl_decrementInterlockedCount( &m_refCount );
}
// -------------------------------------------------------------------------
void OCatalog::refreshTables()
{
    TStringVector aVector;
    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%"));
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%")),::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%")),aTypes);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aName;
        while(xResult->next())
        {
            aName = xRow->getString(3);
            aVector.push_back(aName);
        }
    }
    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OCatalog::refreshViews()
{
}
// -------------------------------------------------------------------------
void OCatalog::refreshGroups()
{
}
// -------------------------------------------------------------------------
void OCatalog::refreshUsers()
{
}
// -------------------------------------------------------------------------
const ::rtl::OUString& OCatalog::getDot()
{
    static const ::rtl::OUString sDot = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("."));
    return sDot;
}
// -----------------------------------------------------------------------------

// XTablesSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getTables(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    try
    {
        if(!m_pTables || m_pConnection->getForceLoadTables())
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
