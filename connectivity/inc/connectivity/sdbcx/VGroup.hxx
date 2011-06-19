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

#ifndef _CONNECTIVITY_SDBCX_GROUP_HXX_
#define _CONNECTIVITY_SDBCX_GROUP_HXX_

#include <osl/diagnose.h>


#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XAuthorizable.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase4.hxx>
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/sdbcx/VCollection.hxx"
#include <comphelper/propertycontainer.hxx>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "connectivity/sdbcx/VDescriptor.hxx"
#include "connectivity/dbtoolsdllapi.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity
{
    namespace sdbcx
    {
        typedef OCollection OUsers;

        typedef ::cppu::WeakComponentImplHelper4<   ::com::sun::star::sdbcx::XUsersSupplier,
                                                    ::com::sun::star::sdbcx::XAuthorizable,
                                                    ::com::sun::star::container::XNamed,
                                                    ::com::sun::star::lang::XServiceInfo> OGroup_BASE;

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
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        public:
            OGroup(sal_Bool _bCase);
            OGroup( const ::rtl::OUString& _Name,sal_Bool _bCase);
            virtual ~OGroup();
            DECLARE_SERVICE_INFO();

            // XInterface
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XUsersSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getUsers(  ) throw(::com::sun::star::uno::RuntimeException);
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL grantPrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // XNamed
            virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_GROUP_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
