/*************************************************************************
 *
 *  $RCSfile: ACatalog.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
using namespace connectivity::ado;
// -------------------------------------------------------------------------
OCatalog::OCatalog(_ADOCatalog* _pCatalog,OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
                ,m_aCatalog(_pCatalog)
{
    osl_incrementInterlockedCount( &m_refCount );
    refreshTables();
    refreshViews();
    refreshGroups();
    refreshUsers();
    osl_decrementInterlockedCount( &m_refCount );
}
// -------------------------------------------------------------------------
void OCatalog::refreshTables()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOTables* pTables = m_aCatalog.get_Tables();
    if(pTables)
    {
        pTables->Refresh();

        sal_Int32 nCount = 0;
        pTables->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOTable* pTable = NULL;
            pTables->get_Item(OLEVariant(i),&pTable);
            if(pTable)
            {
                WpADOTable aTable(pTable);
                aVector.push_back(aTable.get_Name());
            }
        }
    }

    if(m_pTables)
        delete m_pTables;
    m_pTables = new OTables(*this,m_aMutex,aVector,pTables,m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshViews()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOViews* pViews = m_aCatalog.get_Views();
    if(pViews)
    {
        pViews->Refresh();

        sal_Int32 nCount = 0;
        pViews->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOView* pView = NULL;
            pViews->get_Item(OLEVariant(i),&pView);
            if(pView)
            {
                WpADOView aView(pView);
                aVector.push_back(aView.get_Name());
            }
        }
    }

    if(m_pViews)
        delete m_pViews;
    m_pViews = new OViews(*this,m_aMutex,aVector,pViews,m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshGroups()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOGroups* pGroups = m_aCatalog.get_Groups();
    if(pGroups)
    {
        pGroups->Refresh();

        sal_Int32 nCount = 0;
        pGroups->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOGroup* pGroup = NULL;
            pGroups->get_Item(OLEVariant(i),&pGroup);
            if(pGroup)
            {
                WpADOGroup aGroup(pGroup);
                aVector.push_back(aGroup.get_Name());
            }
        }
    }

    if(m_pGroups)
        delete m_pGroups;
    m_pGroups = new OGroups(*this,m_aMutex,aVector,pGroups,m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshUsers()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOUsers* pUsers = m_aCatalog.get_Users();
    if(pUsers)
    {
        pUsers->Refresh();

        sal_Int32 nCount = 0;
        pUsers->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOUser* pUser = NULL;
            pUsers->get_Item(OLEVariant(i),&pUser);
            if(pUser)
            {
                WpADOUser aUser(pUser);
                aVector.push_back(aUser.get_Name());
            }
        }
    }

    if(m_pUsers)
        delete m_pUsers;
    m_pUsers = new OUsers(*this,m_aMutex,aVector,pUsers,m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------


