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
            OAdoUser(OCatalog* _pParent,sal_Bool _bCase,  const ::rtl::OUString& _Name);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            // XUser
            virtual void SAL_CALL changePassword( const ::rtl::OUString& objPassword, const ::rtl::OUString& newPassword ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL grantPrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            WpADOUser getImpl() const { return m_aUser;}
        };

        class OUserExtend;
        typedef ::comphelper::OPropertyArrayUsageHelper<OUserExtend> OUserExtend_PROP;

        class OUserExtend : public OAdoUser,
                            public OUserExtend_PROP
        {
        protected:
            ::rtl::OUString m_Password;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        public:
            OUserExtend(OCatalog* _pParent,sal_Bool _bCase,ADOUser* _pUser=NULL);
            OUserExtend(OCatalog* _pParent,sal_Bool _bCase,const ::rtl::OUString& _Name);

            virtual void construct();
            ::rtl::OUString getPassword() const { return m_Password;}
        };
    }
}
#endif // _CONNECTIVITY_ADO_USER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
