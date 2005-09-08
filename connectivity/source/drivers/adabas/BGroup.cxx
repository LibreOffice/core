/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BGroup.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:21:17 $
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

#ifndef _CONNECTIVITY_ADABAS_GROUP_HXX_
#include "adabas/BGroup.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_USERS_HXX_
#include "adabas/BUsers.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
OAdabasGroup::OAdabasGroup( OAdabasConnection* _pConnection) : connectivity::sdbcx::OGroup(sal_True)
                    ,m_pConnection(_pConnection)
{
    construct();
    TStringVector aVector;
    m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}
// -------------------------------------------------------------------------
OAdabasGroup::OAdabasGroup( OAdabasConnection* _pConnection,
                const ::rtl::OUString& _Name
              ) : connectivity::sdbcx::OGroup(_Name,sal_True)
                ,m_pConnection(_pConnection)
{
    construct();
    refreshUsers();
}
// -------------------------------------------------------------------------
void OAdabasGroup::refreshUsers()
{
    if(!m_pConnection)
        return;

    TStringVector aVector;
        Reference< XStatement > xStmt = m_pConnection->createStatement(  );

    ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("SELECT DISTINCT USERNAME FROM DOMAIN.USERS WHERE USERNAME IS NOT NULL AND USERNAME <> ' ' AND USERNAME <> 'CONTROL' AND GROUPNAME = '");
    aSql += getName( );
    aSql += ::rtl::OUString::createFromAscii("'");

    Reference< XResultSet >  xResult = xStmt->executeQuery(aSql);
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(1));
        ::comphelper::disposeComponent(xResult);
    }
    ::comphelper::disposeComponent(xStmt);

    if(m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}


