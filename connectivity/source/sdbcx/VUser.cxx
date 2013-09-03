/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
OUser::OUser(const OUString& _Name,sal_Bool _bCase) :    OUser_BASE(m_aMutex)
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
void SAL_CALL OUser::changePassword( const OUString& /*objPassword*/, const OUString& /*newPassword*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
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

sal_Int32 SAL_CALL OUser::getPrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::changePassword", *this );
#if !(defined(_MSC_VER) && defined(ENABLE_LTO))
    return 0;
#endif
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OUser::getGrantablePrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::getGrantablePrivileges", *this );
#if !(defined(_MSC_VER) && defined(ENABLE_LTO))
    return 0;
#endif
}
// -------------------------------------------------------------------------
void SAL_CALL OUser::grantPrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::grantPrivileges", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL OUser::revokePrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
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
OUString SAL_CALL OUser::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OUser::setName( const OUString& /*aName*/ ) throw(::com::sun::star::uno::RuntimeException)
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
