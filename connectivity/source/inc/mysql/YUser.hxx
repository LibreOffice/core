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

#ifndef _CONNECTIVITY_MYSQL_USER_HXX_
#define _CONNECTIVITY_MYSQL_USER_HXX_

#include "connectivity/sdbcx/VUser.hxx"
#include "com/sun/star/sdbc/XConnection.hpp"

namespace connectivity
{
    namespace mysql
    {
        typedef connectivity::sdbcx::OUser OUser_TYPEDEF;

        class OMySQLUser : public OUser_TYPEDEF
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

            ::rtl::OUString getPrivilegeString(sal_Int32 nRights) const;
            // return the privileges and additional the grant rights
            void findPrivilegesAndGrantPrivileges(const ::rtl::OUString& objName, sal_Int32 objType,sal_Int32& nRights,sal_Int32& nRightsWithGrant) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        public:
            virtual void refreshGroups();
        public:
            OMySQLUser(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);
            OMySQLUser(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,const ::rtl::OUString& _Name);

            // XUser
            virtual void SAL_CALL changePassword( const ::rtl::OUString& objPassword, const ::rtl::OUString& newPassword ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL grantPrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };

        class OUserExtend;
        typedef ::comphelper::OPropertyArrayUsageHelper<OUserExtend> OUserExtend_PROP;

        class OUserExtend : public OMySQLUser,
                            public OUserExtend_PROP
        {
        protected:
            ::rtl::OUString m_Password;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        public:
            OUserExtend(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);

            virtual void construct();
        };
    }
}
#endif // _CONNECTIVITY_MYSQL_USER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
