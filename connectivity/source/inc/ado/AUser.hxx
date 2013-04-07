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

#ifndef _CONNECTIVITY_ADO_USER_HXX_
#define _CONNECTIVITY_ADO_USER_HXX_

#include "connectivity/sdbcx/VUser.hxx"
#include "ado/Awrapadox.hxx"

namespace connectivity
{
    namespace ado
    {
        class OCatalog;
        typedef connectivity::sdbcx::OUser OUser_TYPEDEF;
        typedef connectivity::sdbcx::OUser_BASE OUser_BASE_TYPEDEF;

        class OAdoUser : public OUser_TYPEDEF
        {
        protected:
            WpADOUser   m_aUser;
            OCatalog*   m_pCatalog;

            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                            sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                         )
                         throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        public:
            virtual void refreshGroups();
        public:
            OAdoUser(OCatalog* _pParent,sal_Bool _bCase,    ADOUser* _pUser=NULL);
            OAdoUser(OCatalog* _pParent,sal_Bool _bCase,  const OUString& _Name);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            // XUser
            virtual void SAL_CALL changePassword( const OUString& objPassword, const OUString& newPassword ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            WpADOUser getImpl() const { return m_aUser;}
        };

        class OUserExtend;
        typedef ::comphelper::OPropertyArrayUsageHelper<OUserExtend> OUserExtend_PROP;

        class OUserExtend : public OAdoUser,
                            public OUserExtend_PROP
        {
        protected:
            OUString m_Password;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        public:
            OUserExtend(OCatalog* _pParent,sal_Bool _bCase,ADOUser* _pUser=NULL);
            OUserExtend(OCatalog* _pParent,sal_Bool _bCase,const OUString& _Name);

            virtual void construct();
            OUString getPassword() const { return m_Password;}
        };
    }
}
#endif // _CONNECTIVITY_ADO_USER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
