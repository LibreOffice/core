/*************************************************************************
 *
 *  $RCSfile: BCatalog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-29 07:02:32 $
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

#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_GROUPS_HXX_
#include "adabas/BGroups.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_USERS_HXX_
#include "adabas/BUsers.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLES_HXX_
#include "adabas/BTables.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_VIEWS_HXX_
#include "adabas/BViews.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif


// -------------------------------------------------------------------------
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OAdabasCatalog::OAdabasCatalog(SQLHANDLE _aConnectionHdl, OAdabasConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
                ,m_aConnectionHdl(_aConnectionHdl)
                ,m_xMetaData(m_pConnection->getMetaData(  ))
{
    osl_incrementInterlockedCount( &m_refCount );
    refreshTables();
    refreshViews();
    refreshGroups();
    refreshUsers();
    osl_decrementInterlockedCount( &m_refCount );
}
// -------------------------------------------------------------------------
void OAdabasCatalog::refreshTables()
{
    ::std::vector< ::rtl::OUString> aVector;
    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString::createFromAscii("%");
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        ::rtl::OUString::createFromAscii("%"),::rtl::OUString::createFromAscii("%"),aTypes);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aName,aDot = ::rtl::OUString::createFromAscii(".");
        while(xResult->next())
        {
            aName = xRow->getString(2);
            aName += aDot;
            aName += xRow->getString(3);
            aVector.push_back(aName);
        }
    }
    if(m_pTables)
        delete m_pTables;
    m_pTables = new OTables(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OAdabasCatalog::refreshViews()
{
    ::std::vector< ::rtl::OUString> aVector;

    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    Reference< XResultSet > xResult = xStmt->executeQuery(
        ::rtl::OUString::createFromAscii("SELECT DISTINCT DOMAIN.VIEWDEFS.OWNER, DOMAIN.VIEWDEFS.VIEWNAME FROM DOMAIN.VIEWDEFS"));

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aName,aDot = ::rtl::OUString::createFromAscii(".");
        while(xResult->next())
        {
            aName = xRow->getString(1);
            aName += aDot;
            aName += xRow->getString(2);
            aVector.push_back(aName);
        }
    }
    if(m_pViews)
        delete m_pViews;
    m_pViews = new OViews(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OAdabasCatalog::refreshGroups()
{
    ::std::vector< ::rtl::OUString> aVector;
    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    Reference< XResultSet > xResult = xStmt->executeQuery(
        ::rtl::OUString::createFromAscii("SELECT DISTINCT GROUPNAME FROM DOMAIN.USERS WHERE GROUPNAME IS NOT NULL AND GROUPNAME <> ' '"));
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(1));
    }
    if(m_pGroups)
        delete m_pGroups;
    m_pGroups = new OGroups(*this,m_aMutex,aVector,m_pConnection,this);
}
// -------------------------------------------------------------------------
void OAdabasCatalog::refreshUsers()
{
    ::std::vector< ::rtl::OUString> aVector;
    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    Reference< XResultSet >  xResult = xStmt->executeQuery(
        ::rtl::OUString::createFromAscii("SELECT DISTINCT USERNAME FROM DOMAIN.USERS WHERE USERNAME IS NOT NULL AND USERNAME <> ' ' AND USERNAME <> 'CONTROL'"));
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(1));
    }
    if(m_pUsers)
        delete m_pUsers;
    m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}
// -------------------------------------------------------------------------


