/*************************************************************************
 *
 *  $RCSfile: AGroup.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-24 16:11:26 $
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
#ifdef DELETE
#undef DELETE
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGEOBJECT_HPP_
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
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
#define CONNECTIVITY_PROPERTY_NAME_SPACE ado
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
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
    {
        pInterface = pCommand;
        pInterface->AddRef();
    }
}
// -------------------------------------------------------------------------
OAdoGroup::OAdoGroup(sal_Bool _bCase,   ADOGroup* _pGroup) : OGroup_ADO(_bCase)
{
    construct();
    if(_pGroup)
        m_aGroup = WpADOGroup(_pGroup);
    else
        m_aGroup.Create();

    refreshUsers();
}
// -------------------------------------------------------------------------
OAdoGroup::OAdoGroup(sal_Bool _bCase, const ::rtl::OUString& _Name) : OGroup_ADO(_Name,_bCase)
{
    construct();
    m_aGroup.Create();
    m_aGroup.put_Name(_Name);
    refreshUsers();
}
// -------------------------------------------------------------------------
void OAdoGroup::refreshUsers()
{
    ::std::vector< ::rtl::OUString> aVector;

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
        delete m_pUsers;
    m_pUsers = new OUsers(*this,m_aMutex,aVector,pUsers,isCaseSensitive());
}
// -------------------------------------------------------------------------
Any SAL_CALL OAdoGroup::queryInterface( const Type & rType ) throw(RuntimeException)
{
        Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
    if(aRet.hasValue())
        return aRet;
    return OGroup_ADO::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OAdoGroup::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OGroup_ADO::getTypes());
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

    return 0;
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
// -------------------------------------------------------------------------
ObjectTypeEnum OAdoGroup::MapObjectType(sal_Int32 _ObjType)
{
    ObjectTypeEnum eNumType= adPermObjTable;
    switch(_ObjType)
    {
        case PrivilegeObject::TABLE:
            break;
        case PrivilegeObject::VIEW:
            eNumType = adPermObjView;
            break;
        case PrivilegeObject::COLUMN:
            eNumType = adPermObjColumn;
            break;
    }
    return eNumType;
}
// -------------------------------------------------------------------------
sal_Int32 OAdoGroup::MapRight(RightsEnum _eNum)
{
    sal_Int32 nRight = 0;
    if(_eNum & adRightRead)
                nRight |= Privilege::SELECT;
    if(_eNum & adRightInsert)
                nRight |= Privilege::INSERT;
    if(_eNum & adRightUpdate)
                nRight |= Privilege::UPDATE;
    if(_eNum & adRightDelete)
                nRight |= Privilege::DELETE;
    if(_eNum & adRightReadDesign)
                nRight |= Privilege::READ;
    if(_eNum & adRightCreate)
                nRight |= Privilege::CREATE;
    if(_eNum & adRightWriteDesign)
                nRight |= Privilege::ALTER;
    if(_eNum & adRightReference)
                nRight |= Privilege::REFERENCE;
    if(_eNum & adRightDrop)
                nRight |= Privilege::DROP;

    return nRight;
}
// -------------------------------------------------------------------------
RightsEnum OAdoGroup::Map2Right(sal_Int32 _eNum)
{
    sal_Int32 nRight = adRightNone;
        if(_eNum & Privilege::SELECT)
        nRight |= adRightRead;

        if(_eNum & Privilege::INSERT)
        nRight |= adRightInsert;

        if(_eNum & Privilege::UPDATE)
        nRight |= adRightUpdate;

        if(_eNum & Privilege::DELETE)
        nRight |= adRightDelete;

        if(_eNum & Privilege::READ)
        nRight |= adRightReadDesign;

        if(_eNum & Privilege::CREATE)
        nRight |= adRightCreate;

        if(_eNum & Privilege::ALTER)
        nRight |= adRightWriteDesign;

        if(_eNum & Privilege::REFERENCE)
        nRight |= adRightReference;

        if(_eNum & Privilege::DROP)
        nRight |= adRightDrop;

    return (RightsEnum)nRight;
}


