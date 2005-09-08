/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSMozabProxy.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:28:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#define _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#ifndef _CONNECTIVITY_MAB_NS_DECLARES_HXX_
#include "MNSDeclares.hxx"
#endif
#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

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
