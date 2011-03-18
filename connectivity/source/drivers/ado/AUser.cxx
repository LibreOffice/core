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
#include "ado/AUser.hxx"
#include "ado/ACatalog.hxx"
#include "ado/AGroups.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "ado/AConnection.hxx"
#include "ado/Awrapado.hxx"

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
    WpADOGroups aGroups(m_aUser.get_Groups());
    aGroups.fillElementNames(aVector);
    if(m_pGroups)
        m_pGroups->reFill(aVector);
    else
        m_pGroups = new OGroups(m_pCatalog,m_aMutex,aVector,aGroups,isCaseSensitive());
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
                ? reinterpret_cast< sal_Int64 >( this )
                : OUser_TYPEDEF::getSomething(rId);
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
void SAL_CALL OAdoUser::acquire() throw()
{
    OUser_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoUser::release() throw()
{
    OUser_TYPEDEF::release();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OAdoUser::getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_TYPEDEF::rBHelper.bDisposed);

    return ADOS::mapAdoRights2Sdbc(m_aUser.GetPermissions(objName, ADOS::mapObjectType2Ado(objType)));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OAdoUser::getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_TYPEDEF::rBHelper.bDisposed);

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
    checkDisposed(OUser_BASE_TYPEDEF::rBHelper.bDisposed);
    m_aUser.SetPermissions(objName,ADOS::mapObjectType2Ado(objType),adAccessGrant,RightsEnum(ADOS::mapRights2Ado(objPrivileges)));
    ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),*this);
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoUser::revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_TYPEDEF::rBHelper.bDisposed);
    m_aUser.SetPermissions(objName,ADOS::mapObjectType2Ado(objType),adAccessRevoke,RightsEnum(ADOS::mapRights2Ado(objPrivileges)));
    ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),*this);
}
// -----------------------------------------------------------------------------
// XUser
void SAL_CALL OAdoUser::changePassword( const ::rtl::OUString& objPassword, const ::rtl::OUString& newPassword ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_TYPEDEF::rBHelper.bDisposed);
    m_aUser.ChangePassword(objPassword,newPassword);
    ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),*this);
}
// -----------------------------------------------------------------------------





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
