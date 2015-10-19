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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VGROUP_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VGROUP_HXX

#include <osl/diagnose.h>


#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XAuthorizable.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <comphelper/propertycontainer.hxx>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity
{
    namespace sdbcx
    {
        typedef OCollection OUsers;

        typedef ::cppu::WeakComponentImplHelper<   css::sdbcx::XUsersSupplier,
                                                   css::sdbcx::XAuthorizable,
                                                   css::container::XNamed,
                                                   css::lang::XServiceInfo> OGroup_BASE;

        class OOO_DLLPUBLIC_DBTOOLS OGroup :
                        public comphelper::OBaseMutex,
                        public OGroup_BASE,
                        public IRefreshableUsers,
                        public ::comphelper::OPropertyArrayUsageHelper<OGroup>,
                        public ODescriptor
        {
        protected:
            OUsers*         m_pUsers;

            using OGroup_BASE::rBHelper;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
        public:
            OGroup(bool _bCase);
            OGroup( const OUString& _Name, bool _bCase);
            virtual ~OGroup();
            DECLARE_SERVICE_INFO();

            // XInterface
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;

            //XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XUsersSupplier
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getUsers(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            // XNamed
            virtual OUString SAL_CALL getName(  ) throw(css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VGROUP_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
