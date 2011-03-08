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

#ifndef _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#define _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#include "MNSDeclares.hxx"
#include <osl/thread.hxx>

#include <MNSInclude.hxx>
#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/mozilla/XCodeProxy.hpp>
#include "com/sun/star/mozilla/XProxyRunner.hdl"

namespace connectivity
{
    namespace mozab
    {
        namespace ProxiedFunc
        {
            enum
            {
                FUNC_TESTLDAP_INIT_LDAP=1,
                FUNC_TESTLDAP_IS_LDAP_CONNECTED,
                FUNC_TESTLDAP_RELEASE_RESOURCE,
                FUNC_GET_TABLE_STRINGS,
                FUNC_EXECUTE_QUERY,
                FUNC_QUERYHELPER_CREATE_NEW_CARD,
                FUNC_QUERYHELPER_DELETE_CARD,
                FUNC_QUERYHELPER_COMMIT_CARD,
                FUNC_QUERYHELPER_RESYNC_CARD,
                FUNC_NEW_ADDRESS_BOOK
            };
        }

        struct RunArgs
        {
            sal_Int32 funcIndex; //Function Index
            sal_Int32 argCount;  //parameter count
            void *  arg1;
            void *  arg2;
            void *  arg3;
            void *  arg4;
            void *  arg5;
            void *  arg6;
                RunArgs()
                    {
                arg1 = NULL;
                arg2 = NULL;
                arg3 = NULL;
                arg4 = NULL;
                arg5 = NULL;
                arg6 = NULL;
                    }
        };
        typedef RunArgs RunArgs;
        typedef ::cppu::WeakImplHelper1< ::com::sun::star::mozilla::XCodeProxy > MNSMozabProxy_BASE;


    class MNSMozabProxy : public MNSMozabProxy_BASE
        {
        public:
            MNSMozabProxy();
            virtual ~MNSMozabProxy();

            //XCodeProxy
            virtual sal_Int32 SAL_CALL run(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::mozilla::MozillaProductType SAL_CALL getProductType(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getProfileName(  ) throw (::com::sun::star::uno::RuntimeException);

        public:
            sal_Int32 StartProxy(RunArgs * args,::com::sun::star::mozilla::MozillaProductType aProduct,const ::rtl::OUString &aProfile); //Call this to start proxy

        protected:
            nsresult testLDAPConnection();
            nsresult InitLDAP(sal_Char* sUri, sal_Unicode* sBindDN, sal_Unicode* sPasswd,sal_Bool * nUseSSL);
            nsresult QueryHelperStub();
        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XProxyRunner > xRunner;

            RunArgs * m_Args;
            ::com::sun::star::mozilla::MozillaProductType m_Product;
            ::rtl::OUString m_Profile;
#if OSL_DEBUG_LEVEL > 0
            oslThreadIdentifier m_oThreadID;
#endif
        };
    }
}
#endif //_CONNECTIVITY_MAB_MOZABHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
