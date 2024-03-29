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


#include <sdbcx/VGroup.hxx>
#include <comphelper/sequence.hxx>
#include <connectivity/dbexception.hxx>


using namespace ::connectivity::sdbcx;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

IMPLEMENT_SERVICE_INFO(OGroup,"com.sun.star.sdbcx.VGroup","com.sun.star.sdbcx.Group");

OGroup::OGroup(bool _bCase) :   OGroup_BASE(m_aMutex)
                ,   ODescriptor(OGroup_BASE::rBHelper,_bCase)
{
}

OGroup::OGroup(const OUString& Name, bool _bCase) :  OGroup_BASE(m_aMutex)
                        ,ODescriptor(OGroup_BASE::rBHelper,_bCase)
{
    m_Name = Name;
}

OGroup::~OGroup()
{
}

Any SAL_CALL OGroup::queryInterface( const Type & rType )
{
    Any aRet = ODescriptor::queryInterface( rType);
    return aRet.hasValue() ? aRet : OGroup_BASE::queryInterface( rType);
}

Sequence< Type > SAL_CALL OGroup::getTypes(  )
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OGroup_BASE::getTypes());
}

void OGroup::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pUsers)
        m_pUsers->disposing();
}

::cppu::IPropertyArrayHelper* OGroup::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

::cppu::IPropertyArrayHelper & OGroup::getInfoHelper()
{
    return *getArrayHelper();
}

Reference< XNameAccess > SAL_CALL OGroup::getUsers(  )
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

    return m_pUsers.get();
}


sal_Int32 SAL_CALL OGroup::getPrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);

    return 0;
}

sal_Int32 SAL_CALL OGroup::getGrantablePrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);

    return 0;
}

void SAL_CALL OGroup::grantPrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);
    throwFeatureNotImplementedSQLException( "XAuthorizable::grantPrivileges", *this );
}

void SAL_CALL OGroup::revokePrivileges( const OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);
    throwFeatureNotImplementedSQLException( "XAuthorizable::revokePrivileges", *this );
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OGroup::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

OUString SAL_CALL OGroup::getName(  )
{
    return m_Name;
}

void SAL_CALL OGroup::setName( const OUString& /*aName*/ )
{
    throwFeatureNotImplementedRuntimeException( "XNamed::setName", *this );
}

// XInterface
void SAL_CALL OGroup::acquire() noexcept
{
    OGroup_BASE::acquire();
}

void SAL_CALL OGroup::release() noexcept
{
    OGroup_BASE::release();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
