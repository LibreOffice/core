/*************************************************************************
 *
 *  $RCSfile: AUser.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-24 06:13:55 $
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

#ifndef _CONNECTIVITY_ADO_USER_HXX_
#include "ado/AUser.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_GROUPS_HXX_
#include "ado/AGroups.hxx"
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
#ifndef _CONNECTIVITY_ADO_CATALOG_HXX_
#include "ado/ACatalog.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
OAdoUser::OAdoUser(OCatalog* _pParent,sal_Bool _bCase, ADOUser* _pUser)
    : OUser_TYPEDEF(_bCase)
    ,m_pCatalog(_pParent)
{
    construct();

    if(_pUser)
        m_aUser = WpADOUser(_pUser);
    else
        m_aUser.Create();
}
// -------------------------------------------------------------------------
OAdoUser::OAdoUser(OCatalog* _pParent,sal_Bool _bCase,   const ::rtl::OUString& _Name)
    : OUser_TYPEDEF(_Name,_bCase)
    , m_pCatalog(_pParent)
{
    construct();
    m_aUser.Create();
    m_aUser.put_Name(_Name);
}
// -------------------------------------------------------------------------
void OAdoUser::refreshGroups()
{

    TStringVector aVector;

    ADOGroups* pGroups = m_aUser.get_Groups();
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
        m_pGroups->reFill(aVector);
    else
        m_pGroups = new OGroups(m_pCatalog,m_aMutex,aVector,pGroups,isCaseSensitive());
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoUser::getUnoTunnelImplementationId()
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
sal_Int64 OAdoUser::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            (sal_Int64)this
                :
            OUser_TYPEDEF::getSomething(rId);
}

// -------------------------------------------------------------------------
void OAdoUser::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aUser.IsValid())
    {

        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aUser.put_Name(aVal);
                }
                break;
        }
    }
}
// -------------------------------------------------------------------------
void OAdoUser::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_aUser.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                rValue <<= m_aUser.get_Name();
                break;
        }
    }
}
// -------------------------------------------------------------------------
OUserExtend::OUserExtend(OCatalog* _pParent,sal_Bool _bCase,    ADOUser* _pUser)
    : OAdoUser(_pParent,_bCase,_pUser)
{
}
// -------------------------------------------------------------------------
OUserExtend::OUserExtend(OCatalog* _pParent,sal_Bool _bCase, const ::rtl::OUString& _Name)
    : OAdoUser(_pParent,_bCase,_Name)
{
}

// -------------------------------------------------------------------------
void OUserExtend::construct()
{
    OUser_TYPEDEF::construct();
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD),    PROPERTY_ID_PASSWORD,0,&m_Password,::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -----------------------------------------------------------------------------
cppu::IPropertyArrayHelper* OUserExtend::createArrayHelper() const
{
    Sequence< com::sun::star::beans::Property > aProps;
    describeProperties(aProps);
    return new cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
cppu::IPropertyArrayHelper & OUserExtend::getInfoHelper()
{
    return *OUserExtend_PROP::getArrayHelper();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OAdoUser::acquire() throw(RuntimeException)
{
    OUser_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoUser::release() throw(RuntimeException)
{
    OUser_TYPEDEF::release();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OAdoUser::getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_TYPEDEF::rBHelper.bDisposed);

    return ADOS::mapAdoRights2Sdbc(m_aUser.GetPermissions(objName, ADOS::mapObjectType2Ado(objType)));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OAdoUser::getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_TYPEDEF::rBHelper.bDisposed);

    sal_Int32 nRights = 0;
    RightsEnum eRights = m_aUser.GetPermissions(objName, ADOS::mapObjectType2Ado(objType));
    if((eRights & adRightWithGrant) == adRightWithGrant)
        nRights = ADOS::mapAdoRights2Sdbc(eRights);
    ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),*this);
    return nRights;
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoUser::grantPrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_TYPEDEF::rBHelper.bDisposed);
    m_aUser.SetPermissions(objName,ADOS::mapObjectType2Ado(objType),adAccessGrant,RightsEnum(ADOS::mapRights2Ado(objPrivileges)));
    ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),*this);
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoUser::revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_TYPEDEF::rBHelper.bDisposed);
    m_aUser.SetPermissions(objName,ADOS::mapObjectType2Ado(objType),adAccessRevoke,RightsEnum(ADOS::mapRights2Ado(objPrivileges)));
    ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),*this);
}
// -----------------------------------------------------------------------------
// XUser
void SAL_CALL OAdoUser::changePassword( const ::rtl::OUString& objPassword, const ::rtl::OUString& newPassword ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_TYPEDEF::rBHelper.bDisposed);
    m_aUser.ChangePassword(objPassword,newPassword);
    ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),*this);
}
// -----------------------------------------------------------------------------





