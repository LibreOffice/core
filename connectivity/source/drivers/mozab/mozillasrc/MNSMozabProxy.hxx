/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#define _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#include "MNSDeclares.hxx"
#include <osl/thread.hxx>

#include <MNSInclude.hxx>
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_MOZILLA_XCODEPROXY_HDL_
#include <com/sun/star/mozilla/XCodeProxy.hpp>
#endif
#ifndef _COM_SUN_STAR_MOZILLA_XPROXYRUNNER_HDL_
#include "com/sun/star/mozilla/XProxyRunner.hdl"
#endif

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
