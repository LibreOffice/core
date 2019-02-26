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

#include <ado/ACatalog.hxx>
#include <ado/AConnection.hxx>
#include <ado/AGroups.hxx>
#include <ado/AUsers.hxx>
#include <ado/ATables.hxx>
#include <ado/AViews.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>


using namespace connectivity;
using namespace connectivity::ado;

OCatalog::OCatalog(_ADOCatalog* _pCatalog,OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_aCatalog(_pCatalog)
                ,m_pConnection(_pCon)
{
}

OCatalog::~OCatalog()
{
    if(m_aCatalog.IsValid())
        m_aCatalog.putref_ActiveConnection(nullptr);
    m_aCatalog.clear();
}

void OCatalog::refreshTables()
{
    ::std::vector< OUString> aVector;

    WpADOTables aTables(m_aCatalog.get_Tables());
    if ( aTables.IsValid() )
    {
        aTables.Refresh();
        sal_Int32 nCount = aTables.GetItemCount();
        aVector.reserve(nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            WpADOTable aElement = aTables.GetItem(i);
            if ( aElement.IsValid() )
            {
                OUString sTypeName = aElement.get_Type();
                if ( !sTypeName.equalsIgnoreAsciiCase("SYSTEM TABLE")
                     && !sTypeName.equalsIgnoreAsciiCase("ACCESS TABLE") )
                    aVector.push_back(aElement.get_Name());
            }
        }
    }

    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables.reset( new OTables(this,m_aMutex,aVector,aTables,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers()) );
}

void OCatalog::refreshViews()
{
    ::std::vector< OUString> aVector;

    WpADOViews aViews = m_aCatalog.get_Views();
    aViews.fillElementNames(aVector);

    if(m_pViews)
        m_pViews->reFill(aVector);
    else
        m_pViews.reset( new OViews(this,m_aMutex,aVector,aViews,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers()) );
}

void OCatalog::refreshGroups()
{
    ::std::vector< OUString> aVector;

    WpADOGroups aGroups = m_aCatalog.get_Groups();
    aGroups.fillElementNames(aVector);

    if(m_pGroups)
        m_pGroups->reFill(aVector);
    else
        m_pGroups.reset( new OGroups(this,m_aMutex,aVector,aGroups,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers()) );
}

void OCatalog::refreshUsers()
{
    ::std::vector< OUString> aVector;

    WpADOUsers aUsers = m_aCatalog.get_Users();
    aUsers.fillElementNames(aVector);

    if(m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers.reset( new OUsers(this,m_aMutex,aVector,aUsers,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers()) );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
