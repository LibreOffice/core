/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ACatalog.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:26:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ADO_CATALOG_HXX_
#include "ado/ACatalog.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_BCONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_GROUPS_HXX_
#include "ado/AGroups.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_USERS_HXX_
#include "ado/AUsers.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_TABLES_HXX_
#include "ado/ATables.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_VIEWS_HXX_
#include "ado/AViews.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif


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
    aTables.fillElementNames(aVector);

    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(this,m_aMutex,aVector,aTables,m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
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
        m_pViews = new OViews(this,m_aMutex,aVector,aViews,m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
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
        m_pGroups = new OGroups(this,m_aMutex,aVector,aGroups,m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
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
        m_pUsers = new OUsers(this,m_aMutex,aVector,aUsers,m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------


