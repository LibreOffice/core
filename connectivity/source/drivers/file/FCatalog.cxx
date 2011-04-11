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
#include "file/FCatalog.hxx"
#include "file/FConnection.hxx"
#include "file/FTables.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <rtl/logfile.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

// -------------------------------------------------------------------------
using namespace connectivity::file;
// -------------------------------------------------------------------------
OFileCatalog::OFileCatalog(OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileCatalog::OFileCatalog" );
}
// -------------------------------------------------------------------------
void SAL_CALL OFileCatalog::disposing()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileCatalog::disposing" );
    ::osl::MutexGuard aGuard(m_aMutex);

    typedef connectivity::sdbcx::OCatalog OFileCatalog_BASE;
m_xMetaData.clear();
    OFileCatalog_BASE::disposing();
}
// -----------------------------------------------------------------------------
::rtl::OUString OFileCatalog::buildName(const Reference< XRow >& _xRow)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileCatalog::buildName" );
    return _xRow->getString(3);
}
// -------------------------------------------------------------------------
void OFileCatalog::refreshTables()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileCatalog::refreshTables" );
    TStringVector aVector;
    Sequence< ::rtl::OUString > aTypes;
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),aTypes);
    fillNames(xResult,aVector);

    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(m_xMetaData,*this,m_aMutex,aVector);
}

// -------------------------------------------------------------------------
Any SAL_CALL OFileCatalog::queryInterface( const Type & rType ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileCatalog::queryInterface" );
    if( rType == ::getCppuType((const Reference<XGroupsSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XUsersSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XViewsSupplier>*)0))
        return Any();


    typedef sdbcx::OCatalog OFileCatalog_BASE;
    return OFileCatalog_BASE::queryInterface(rType);
}
// -----------------------------------------------------------------------------
Sequence< Type > SAL_CALL OFileCatalog::getTypes(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileCatalog::getTypes" );
    typedef sdbcx::OCatalog OFileCatalog_BASE;

    Sequence< Type > aTypes = OFileCatalog_BASE::getTypes();
    ::std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(*pBegin == ::getCppuType((const Reference<XGroupsSupplier>*)0) ||
            *pBegin == ::getCppuType((const Reference<XUsersSupplier>*)0)   ||
            *pBegin == ::getCppuType((const Reference<XViewsSupplier>*)0)))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    const Type *pTypes = aOwnTypes.empty() ? 0 : &aOwnTypes[0];
    return Sequence< Type >(pTypes, aOwnTypes.size());
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
