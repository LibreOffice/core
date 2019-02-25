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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HUSER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HUSER_HXX

#include <connectivity/sdbcx/VUser.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>

namespace connectivity
{
    namespace hsqldb
    {
        typedef connectivity::sdbcx::OUser OUser_TYPEDEF;

        class OHSQLUser : public OUser_TYPEDEF
        {
            css::uno::Reference< css::sdbc::XConnection > m_xConnection;

            static OUString getPrivilegeString(sal_Int32 nRights);
            // return the privileges and additional the grant rights
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void findPrivilegesAndGrantPrivileges(const OUString& objName, sal_Int32 objType,sal_Int32& nRights,sal_Int32& nRightsWithGrant);
        public:
            virtual void refreshGroups() override;
        public:
            OHSQLUser(  const css::uno::Reference< css::sdbc::XConnection >& _xConnection);
            OHSQLUser(  const css::uno::Reference< css::sdbc::XConnection >& _xConnection,const OUString& Name);

            // XUser
            virtual void SAL_CALL changePassword( const OUString& objPassword, const OUString& newPassword ) override;
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) override;
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) override;
            virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) override;
            virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) override;
        };

        class OUserExtend;
        typedef ::comphelper::OPropertyArrayUsageHelper<OUserExtend> OUserExtend_PROP;

        class OUserExtend : public OHSQLUser,
                            public OUserExtend_PROP
        {
            OUString m_Password;
        protected:
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
        public:
            OUserExtend(const css::uno::Reference< css::sdbc::XConnection >& _xConnection);

            virtual void construct() override;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HUSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
