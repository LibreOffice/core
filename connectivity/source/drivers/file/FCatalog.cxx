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

#include <file/FCatalog.hxx>
#include <file/FConnection.hxx>
#include <file/FTables.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;


using namespace connectivity::file;

OFileCatalog::OFileCatalog(OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
{
}

void SAL_CALL OFileCatalog::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    typedef connectivity::sdbcx::OCatalog OFileCatalog_BASE;
    m_xMetaData.clear();
    OFileCatalog_BASE::disposing();
}

OUString OFileCatalog::buildName(const Reference< XRow >& _xRow)
{
    return _xRow->getString(3);
}

void OFileCatalog::refreshTables()
{
    ::std::vector< OUString> aVector;
    Sequence< OUString > aTypes;
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        "%", "%", aTypes);
    fillNames(xResult,aVector);

    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables.reset( new OTables(m_xMetaData,*this,m_aMutex,aVector) );
}


Any SAL_CALL OFileCatalog::queryInterface( const Type & rType )
{
    if( rType == cppu::UnoType<XGroupsSupplier>::get()||
        rType == cppu::UnoType<XUsersSupplier>::get()||
        rType == cppu::UnoType<XViewsSupplier>::get())
        return Any();


    typedef sdbcx::OCatalog OFileCatalog_BASE;
    return OFileCatalog_BASE::queryInterface(rType);
}

Sequence< Type > SAL_CALL OFileCatalog::getTypes(  )
{
    typedef sdbcx::OCatalog OFileCatalog_BASE;

    Sequence< Type > aTypes = OFileCatalog_BASE::getTypes();
    std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(*pBegin == cppu::UnoType<XGroupsSupplier>::get()||
            *pBegin == cppu::UnoType<XUsersSupplier>::get()||
            *pBegin == cppu::UnoType<XViewsSupplier>::get()))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    return Sequence< Type >(aOwnTypes.data(), aOwnTypes.size());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
