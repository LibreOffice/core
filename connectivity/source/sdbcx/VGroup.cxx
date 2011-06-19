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

#include <stdio.h>
#include "connectivity/sdbcx/VGroup.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include <comphelper/sequence.hxx>
#include <connectivity/dbexception.hxx>

// -------------------------------------------------------------------------
using namespace ::connectivity::sdbcx;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

IMPLEMENT_SERVICE_INFO(OGroup,"com.sun.star.sdbcx.VGroup","com.sun.star.sdbcx.Group");
// -------------------------------------------------------------------------
OGroup::OGroup(sal_Bool _bCase) :   OGroup_BASE(m_aMutex)
                ,   ODescriptor(OGroup_BASE::rBHelper,_bCase)
                ,   m_pUsers(NULL)
{
}
// -------------------------------------------------------------------------
OGroup::OGroup(const ::rtl::OUString& _Name,sal_Bool _bCase) :  OGroup_BASE(m_aMutex)
                        ,ODescriptor(OGroup_BASE::rBHelper,_bCase)
                        ,m_pUsers(NULL)
{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OGroup::~OGroup()
{
    delete m_pUsers;
}
// -------------------------------------------------------------------------
Any SAL_CALL OGroup::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    return aRet.hasValue() ? aRet : OGroup_BASE::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OGroup::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OGroup_BASE::getTypes());
}
// -------------------------------------------------------------------------
void OGroup::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pUsers)
        m_pUsers->disposing();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OGroup::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OGroup::getInfoHelper()
{
    return *const_cast<OGroup*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL OGroup::getUsers(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pUsers )
            refreshUsers();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return const_cast<OGroup*>(this)->m_pUsers;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OGroup::getPrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);

    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OGroup::getGrantablePrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);

    return 0;
}
// -------------------------------------------------------------------------
void SAL_CALL OGroup::grantPrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);
    throwFeatureNotImplementedException( "XAuthorizable::grantPrivileges", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL OGroup::revokePrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);
    throwFeatureNotImplementedException( "XAuthorizable::revokePrivileges", *this );
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OGroup::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGroup::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setName( const ::rtl::OUString& /*aName*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    throwFeatureNotImplementedException( "XNamed::setName", *this );
}
// -----------------------------------------------------------------------------
// XInterface
void SAL_CALL OGroup::acquire() throw()
{
    OGroup_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::release() throw()
{
    OGroup_BASE::release();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
