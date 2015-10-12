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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YUSER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YUSER_HXX

#include <connectivity/sdbcx/VUser.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>

namespace connectivity
{
    namespace mysql
    {
        typedef connectivity::sdbcx::OUser OUser_TYPEDEF;

        class OMySQLUser : public OUser_TYPEDEF
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

            static OUString getPrivilegeString(sal_Int32 nRights);
            // return the privileges and additional the grant rights
            void findPrivilegesAndGrantPrivileges(const OUString& objName, sal_Int32 objType,sal_Int32& nRights,sal_Int32& nRightsWithGrant) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        public:
            virtual void refreshGroups() override;
        public:
            OMySQLUser(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);
            OMySQLUser(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,const OUString& _Name);

            // XUser
            virtual void SAL_CALL changePassword( const OUString& objPassword, const OUString& newPassword ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        };

        class OUserExtend;
        typedef ::comphelper::OPropertyArrayUsageHelper<OUserExtend> OUserExtend_PROP;

        class OUserExtend : public OMySQLUser,
                            public OUserExtend_PROP
        {
        protected:
            OUString m_Password;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
        public:
            OUserExtend(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);

            virtual void construct() override;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YUSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
