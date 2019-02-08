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


#include <connectivity/sdbcx/VUser.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include <TConnection.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/sequence.hxx>


using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(OUser,"com.sun.star.sdbcx.VUser","com.sun.star.sdbcx.User");

OUser::OUser(bool _bCase)  : OUser_BASE(m_aMutex)
                , ODescriptor(OUser_BASE::rBHelper,_bCase,true)
{
}

OUser::OUser(const OUString& Name, bool _bCase) :    OUser_BASE(m_aMutex)
                        ,ODescriptor(OUser_BASE::rBHelper,_bCase)
{
    m_Name = Name;
}

OUser::~OUser( )
{
}

void OUser::disposing()
{
    OPropertySetHelper::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pGroups)
        m_pGroups->disposing();
}

Any SAL_CALL OUser::queryInterface( const Type & rType )
{
    Any aRet = ODescriptor::queryInterface( rType);
    return aRet.hasValue() ? aRet : OUser_BASE::queryInterface( rType);
}

Sequence< Type > SAL_CALL OUser::getTypes(  )
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OUser_BASE::getTypes());
}

::cppu::IPropertyArrayHelper* OUser::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);

}

::cppu::IPropertyArrayHelper & OUser::getInfoHelper()
{
    return *getArrayHelper();
}

// XUser
void SAL_CALL OUser::changePassword( const OUString& /*objPassword*/, const OUString& /*newPassword*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedSQLException( "XUser::changePassword", *this );
}

// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OUser::getGroups(  )
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

    return m_pGroups.get();
}


SAL_WNOUNREACHABLE_CODE_PUSH

sal_Int32 SAL_CALL OUser::getPrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedSQLException( "XAuthorizable::changePassword", *this );
    return 0;
}

sal_Int32 SAL_CALL OUser::getGrantablePrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedSQLException( "XAuthorizable::getGrantablePrivileges", *this );
    return 0;
}

SAL_WNOUNREACHABLE_CODE_POP


void SAL_CALL OUser::grantPrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedSQLException( "XAuthorizable::grantPrivileges", *this );
}

void SAL_CALL OUser::revokePrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedSQLException( "XAuthorizable::revokePrivileges", *this );
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OUser::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

OUString SAL_CALL OUser::getName(  )
{
    return m_Name;
}

void SAL_CALL OUser::setName( const OUString& /*aName*/ )
{
    OSL_FAIL( "OUser::setName: not implemented!" );
        // not allowed to throw an SQLException here ...
}

// XInterface
void SAL_CALL OUser::acquire() throw()
{
    OUser_BASE::acquire();
}

void SAL_CALL OUser::release() throw()
{
    OUser_BASE::release();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
