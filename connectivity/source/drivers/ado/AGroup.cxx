/*************************************************************************
 *
 *  $RCSfile: AGroup.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-02 10:49:41 $
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

#ifndef _CONNECTIVITY_ADO_GROUP_HXX_
#include "ado/AGroup.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_USERS_HXX_
#include "ado/AUsers.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_BCONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

// -------------------------------------------------------------------------
void WpADOGroup::Create()
{
    IClassFactory2* pIUnknown   = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr = -1;
    ADOGroup* pCommand;
    hr = CoCreateInstance(ADOS::CLSID_ADOGROUP_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOGROUP_25,
                          (void**)&pCommand );


    if( !FAILED( hr ) )
        operator=(pCommand);
}
// -------------------------------------------------------------------------
OAdoGroup::OAdoGroup(OCatalog* _pParent,sal_Bool _bCase,    ADOGroup* _pGroup) : OGroup_ADO(_bCase),m_pCatalog(_pParent)
{
    construct();
    if(_pGroup)
        m_aGroup = WpADOGroup(_pGroup);
    else
        m_aGroup.Create();

}
// -------------------------------------------------------------------------
OAdoGroup::OAdoGroup(OCatalog* _pParent,sal_Bool _bCase, const ::rtl::OUString& _Name) : OGroup_ADO(_Name,_bCase),m_pCatalog(_pParent)
{
    construct();
    m_aGroup.Create();
    m_aGroup.put_Name(_Name);
}
// -------------------------------------------------------------------------
void OAdoGroup::refreshUsers()
{
    TStringVector aVector;

    ADOUsers* pUsers = m_aGroup.get_Users();
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
        m_pUsers->reFill(aVector);
    else
        m_pUsers = new OUsers(m_pCatalog,m_aMutex,aVector,pUsers,isCaseSensitive());
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoGroup::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OAdoGroup::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return OGroup_ADO::getSomething(rId);
}

// -------------------------------------------------------------------------
void OAdoGroup::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aGroup.IsValid())
    {

        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aGroup.put_Name(aVal);
                }
                break;
        }
    }
}
// -------------------------------------------------------------------------
void OAdoGroup::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_aGroup.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                rValue <<= m_aGroup.get_Name();
                break;
        }
    }
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OAdoGroup::getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    return MapRight(m_aGroup.GetPermissions(objName,MapObjectType(objType)));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OAdoGroup::getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    RightsEnum eNum = m_aGroup.GetPermissions(objName,MapObjectType(objType));
    if(eNum & adRightWithGrant)
        return MapRight(eNum);
    return 0;
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoGroup::grantPrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    m_aGroup.SetPermissions(objName,MapObjectType(objType),adAccessGrant,Map2Right(objPrivileges));
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoGroup::revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    m_aGroup.SetPermissions(objName,MapObjectType(objType),adAccessDeny,Map2Right(objPrivileges));
}

// -----------------------------------------------------------------------------
void SAL_CALL OAdoGroup::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OGroup_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoGroup::release() throw(::com::sun::star::uno::RuntimeException)
{
    OGroup_ADO::release();
}
// -----------------------------------------------------------------------------


