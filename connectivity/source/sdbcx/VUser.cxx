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
#include "connectivity/sdbcx/VUser.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include "TConnection.hxx"
#include "connectivity/sdbcx/VCollection.hxx"
#include <connectivity/dbexception.hxx>
#include <comphelper/sequence.hxx>

// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(OUser,"com.sun.star.sdbcx.VUser","com.sun.star.sdbcx.User");
// -------------------------------------------------------------------------
OUser::OUser(sal_Bool _bCase)  : OUser_BASE(m_aMutex)
                , ODescriptor(OUser_BASE::rBHelper,_bCase,sal_True)
                , m_pGroups(NULL)
{
}
// -------------------------------------------------------------------------
OUser::OUser(const ::rtl::OUString& _Name,sal_Bool _bCase) :    OUser_BASE(m_aMutex)
                        ,ODescriptor(OUser_BASE::rBHelper,_bCase)
                        ,m_pGroups(NULL)
{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OUser::~OUser( )
{
    delete m_pGroups;
}
// -------------------------------------------------------------------------
void OUser::disposing(void)
{
    OPropertySetHelper::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pGroups)
        m_pGroups->disposing();
}
// -------------------------------------------------------------------------
Any SAL_CALL OUser::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    return aRet.hasValue() ? aRet : OUser_BASE::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OUser::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OUser_BASE::getTypes());
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OUser::createArrayHelper( ) const
{
        Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);

}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OUser::getInfoHelper()
{
    return *const_cast<OUser*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
// XUser
void SAL_CALL OUser::changePassword( const ::rtl::OUString& /*objPassword*/, const ::rtl::OUString& /*newPassword*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XUser::changePassword", *this );
}
// -------------------------------------------------------------------------
// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OUser::getGroups(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pGroups )
            refreshGroups();
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

    return const_cast<OUser*>(this)->m_pGroups;
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OUser::getPrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::changePassword", *this );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OUser::getGrantablePrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::getGrantablePrivileges", *this );
    return 0;
}
// -------------------------------------------------------------------------
void SAL_CALL OUser::grantPrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::grantPrivileges", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL OUser::revokePrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::revokePrivileges", *this );
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OUser::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OUser::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OUser::setName( const ::rtl::OUString& /*aName*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    OSL_FAIL( "OUser::setName: not implemented!" );
        // not allowed to throw an SQLException here ...
}
// -----------------------------------------------------------------------------
// XInterface
void SAL_CALL OUser::acquire() throw()
{
    OUser_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OUser::release() throw()
{
    OUser_BASE::release();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
