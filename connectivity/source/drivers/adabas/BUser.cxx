/*************************************************************************
 *
 *  $RCSfile: BUser.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:08:03 $
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

#ifndef _CONNECTIVITY_ADABAS_USER_HXX_
#include "adabas/BUser.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_GROUPS_HXX_
#include "adabas/BGroups.hxx"
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
#define CONNECTIVITY_PROPERTY_NAME_SPACE adabas
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
OAdabasUser::OAdabasUser(   OAdabasConnection* _pConnection) : connectivity::sdbcx::OUser(sal_True)
                ,m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
OAdabasUser::OAdabasUser(   OAdabasConnection* _pConnection,
                const ::rtl::OUString& _Name
            ) : connectivity::sdbcx::OUser(_Name,sal_True)
                ,m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
Any SAL_CALL OAdabasUser::queryInterface( const Type & rType ) throw(RuntimeException)
{
//      if(rType == ::getCppuType((const ::com::sun::star::uno::Reference< XGroupsSupplier>*)0))
//              return Any();

    return OUser_TYPEDEF::queryInterface(rType);
}
// -------------------------------------------------------------------------
void OAdabasUser::refreshGroups()
{
    if(!m_pConnection)
        return;

    ::std::vector< ::rtl::OUString> aVector;
        Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("SELECT DISTINCT GROUPNAME FROM DOMAIN.USERS WHERE GROUPNAME IS NOT NULL AND GROUPNAME <> ' ' AND USERNAME = '");
    aSql += getName( );
    aSql += ::rtl::OUString::createFromAscii("'");

        Reference< XResultSet > xResult = xStmt->executeQuery(aSql);
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
OUserExtend::OUserExtend(   OAdabasConnection* _pConnection) : OAdabasUser(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
OUserExtend::OUserExtend(   OAdabasConnection* _pConnection,const ::rtl::OUString& _Name) : OAdabasUser(_pConnection,_Name)
{
    construct();
}
// -------------------------------------------------------------------------
typedef connectivity::sdbcx::OUser  OUser_TYPEDEF;
void OUserExtend::construct()
{
    OUser_TYPEDEF::construct();
    registerProperty(PROPERTY_PASSWORD, PROPERTY_ID_PASSWORD,0,&m_Password,::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -----------------------------------------------------------------------------
cppu::IPropertyArrayHelper* OUserExtend::createArrayHelper() const
{
    Sequence< Property > aProps;
    return new cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
cppu::IPropertyArrayHelper & OUserExtend::getInfoHelper()
{
    return *OUserExtend_PROP::getArrayHelper();
}
// -----------------------------------------------------------------------------

