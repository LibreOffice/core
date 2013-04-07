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

#ifndef _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#define _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#include "MNSDeclares.hxx"
#include <osl/thread.hxx>

#include <MNSInclude.hxx>
#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/mozilla/XCodeProxy.hpp>
#include "com/sun/star/mozilla/XProxyRunner.hpp"

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
            virtual OUString SAL_CALL getProfileName(  ) throw (::com::sun::star::uno::RuntimeException);

        public:
            sal_Int32 StartProxy(RunArgs * args,::com::sun::star::mozilla::MozillaProductType aProduct,const OUString &aProfile); //Call this to start proxy

        protected:
            nsresult testLDAPConnection();
            nsresult InitLDAP(sal_Char* sUri, sal_Unicode* sBindDN, sal_Unicode* sPasswd,sal_Bool * nUseSSL);
            nsresult QueryHelperStub();
        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XProxyRunner > xRunner;

            RunArgs * m_Args;
            ::com::sun::star::mozilla::MozillaProductType m_Product;
            OUString m_Profile;
#if OSL_DEBUG_LEVEL > 0
            oslThreadIdentifier m_oThreadID;
#endif
        };
    }
}
#endif //_CONNECTIVITY_MAB_MOZABHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
