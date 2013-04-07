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

#ifndef _CONNECTIVITY_SDBCX_USER_HXX_
#define _CONNECTIVITY_SDBCX_USER_HXX_

#include <osl/diagnose.h>
#include <com/sun/star/sdbcx/XUser.hpp>
#include <com/sun/star/sdbcx/XGroupsSupplier.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase4.hxx>
#include "connectivity/CommonTools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/sdbcx/VCollection.hxx"
#include <com/sun/star/container/XNamed.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "connectivity/sdbcx/VDescriptor.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    namespace sdbcx
    {
        typedef OCollection OGroups;

        typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::sdbcx::XUser,
                                                  ::com::sun::star::sdbcx::XGroupsSupplier,
                                                  ::com::sun::star::container::XNamed,
                                                  ::com::sun::star::lang::XServiceInfo> OUser_BASE;

        class OOO_DLLPUBLIC_DBTOOLS OUser :
                        public comphelper::OBaseMutex,
                        public OUser_BASE,
                        public IRefreshableGroups,
                        public ::comphelper::OPropertyArrayUsageHelper<OUser>,
                        public ODescriptor
        {
        protected:
            OGroups*        m_pGroups;

            using OUser_BASE::rBHelper;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        public:
            OUser(sal_Bool _bCase);
            OUser(const OUString& _Name,sal_Bool _bCase);

            virtual ~OUser( );

            DECLARE_SERVICE_INFO();

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XUser
            virtual void SAL_CALL changePassword( const OUString& objPassword, const OUString& newPassword ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XGroupsSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getGroups(  ) throw(::com::sun::star::uno::RuntimeException);

            // XNamed
            virtual OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_USER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
