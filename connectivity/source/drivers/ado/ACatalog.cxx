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
#include "ado/ACatalog.hxx"
#include "ado/AConnection.hxx"
#include "ado/AGroups.hxx"
#include "ado/AUsers.hxx"
#include "ado/ATables.hxx"
#include "ado/AViews.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>


// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::ado;
// -------------------------------------------------------------------------
OCatalog::OCatalog(_ADOCatalog* _pCatalog,OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
                ,m_aCatalog(_pCatalog)
{
}
// -----------------------------------------------------------------------------
OCatalog::~OCatalog()
{
    if(m_aCatalog.IsValid())
        m_aCatalog.putref_ActiveConnection(NULL);
    m_aCatalog.clear();
}
// -----------------------------------------------------------------------------
void OCatalog::refreshTables()
{
    TStringVector aVector;

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
              ::rtl::OUString sTypeName = aElement.get_Type();
                  if ( !sTypeName.equalsIgnoreAsciiCaseAscii("SYSTEM TABLE") && !sTypeName.equalsIgnoreAsciiCaseAscii("ACCESS TABLE") )
                     aVector.push_back(aElement.get_Name());
               }
         }
     }

    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(this,m_aMutex,aVector,aTables,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshViews()
{
    TStringVector aVector;

    WpADOViews aViews = m_aCatalog.get_Views();
    aViews.fillElementNames(aVector);

    if(m_pViews)
        m_pViews->reFill(aVector);
    else
        m_pViews = new OViews(this,m_aMutex,aVector,aViews,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshGroups()
{
    TStringVector aVector;

    WpADOGroups aGroups = m_aCatalog.get_Groups();
    aGroups.fillElementNames(aVector);

    if(m_pGroups)
        m_pGroups->reFill(aVector);
    else
        m_pGroups = new OGroups(this,m_aMutex,aVector,aGroups,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshUsers()
{
    TStringVector aVector;

    WpADOUsers aUsers = m_aCatalog.get_Users();
    aUsers.fillElementNames(aVector);

    if(m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers = new OUsers(this,m_aMutex,aVector,aUsers,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
