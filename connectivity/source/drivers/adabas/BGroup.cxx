/*************************************************************************
 *
 *  $RCSfile: BGroup.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:19 $
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
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
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
    ::std::vector< ::rtl::OUString> aVector;
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

    ::std::vector< ::rtl::OUString> aVector;
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
    }
    if(m_pUsers)
        delete m_pUsers;
    m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}


