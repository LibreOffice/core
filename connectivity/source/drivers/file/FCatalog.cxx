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
OUString OFileCatalog::buildName(const Reference< XRow >& _xRow)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileCatalog::buildName" );
    return _xRow->getString(3);
}
// -------------------------------------------------------------------------
void OFileCatalog::refreshTables()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileCatalog::refreshTables" );
    TStringVector aVector;
    Sequence< OUString > aTypes;
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        OUString("%"),OUString("%"),aTypes);
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
