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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VUSER_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VUSER_HXX

#include <com/sun/star/sdbcx/XUser.hpp>
#include <com/sun/star/sdbcx/XGroupsSupplier.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
    namespace sdbcx
    {
        typedef OCollection OGroups;

        typedef ::cppu::WeakComponentImplHelper< css::sdbcx::XUser,
                                                 css::sdbcx::XGroupsSupplier,
                                                 css::container::XNamed,
                                                 css::lang::XServiceInfo> OUser_BASE;

        class OOO_DLLPUBLIC_DBTOOLS OUser :
                        public cppu::BaseMutex,
                        public OUser_BASE,
                        public IRefreshableGroups,
                        public ::comphelper::OPropertyArrayUsageHelper<OUser>,
                        public ODescriptor
        {
        protected:
            rtl::Reference<OGroups>  m_pGroups;

            using OUser_BASE::rBHelper;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
        public:
            OUser(bool _bCase);
            OUser(const OUString& Name,bool _bCase);

            virtual ~OUser( ) override;

            DECLARE_SERVICE_INFO();

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;
            //XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
            // XUser
            virtual void SAL_CALL changePassword( const OUString& objPassword, const OUString& newPassword ) override;
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) override;
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) override;
            virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) override;
            virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) override;
            // XGroupsSupplier
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getGroups(  ) override;

            // XNamed
            virtual OUString SAL_CALL getName(  ) override;
            virtual void SAL_CALL setName( const OUString& aName ) override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VUSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
