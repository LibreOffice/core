/*************************************************************************
 *
 *  $RCSfile: ftpcontentprovider.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: abi $ $Date: 2002-10-23 08:00:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FTP_FTPCONTENTPROVIDER_HXX_
#define _FTP_FTPCONTENTPROVIDER_HXX_

#include <vector>
#include <osl/mutex.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include "ftphandleprovider.hxx"
#include "ftpurl.hxx"

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.

#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME         "com.sun.star.ucb.FTPContentProvider"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME_LENGTH  35
#define MYUCP_URL_SCHEME        "ftp"
#define MYUCP_URL_SCHEME_LENGTH 3
#define MYUCP_CONTENT_TYPE      "application/ftp-content"    // UCB Content Type.


/**
 *  Definition of ftpcontentprovider
 */



namespace ftp {


    class FTPLoaderThread;


    class FTPContentProvider:
        public ::ucb::ContentProviderImplHelper,
        public ::com::sun::star::container::XContainerListener,
        public ::com::sun::star::lang::XComponent,
        public FTPHandleProvider
    {
    public:

        FTPContentProvider(const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xMSF );
        ~FTPContentProvider();

        // XInterface
        XINTERFACE_DECL()

        // XTypeProvider
        XTYPEPROVIDER_DECL()

        // XServiceInfo
        XSERVICEINFO_DECL()

        // XContentProvider
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        queryContent(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier )
            throw( com::sun::star::ucb::IllegalIdentifierException,
                   com::sun::star::uno::RuntimeException );

        // XComponent

        virtual void SAL_CALL
        dispose(  )
            throw (::com::sun::star::uno::RuntimeException)
        {
            if(m_xContainer.is())
            {
                m_xContainer->removeContainerListener(this);
                m_xContainer =
                    com::sun::star::uno::Reference<
                    com::sun::star::container::XContainer>(0);
            }
        }

        virtual void SAL_CALL
        addEventListener(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XEventListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException) { }

        virtual void SAL_CALL
        removeEventListener(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XEventListener >& aListener )
            throw (::com::sun::star::uno::RuntimeException) { }

        // XConainerListener ( deriver from XEventListener )

        virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
            throw (::com::sun::star::uno::RuntimeException)
        {
            m_xContainer =
                com::sun::star::uno::Reference<
                com::sun::star::container::XContainer>(0);
        }

        virtual void SAL_CALL
        elementInserted(
            const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException) { }

        virtual void SAL_CALL
        elementRemoved(
            const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException)  { }

        virtual void SAL_CALL
        elementReplaced(
            const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException);


        /** FTPHandleProvider.
         */

        virtual CURL* handle();

        virtual bool forHost(const rtl::OUString& host,
                             const rtl::OUString& port,
                             const rtl::OUString& username,
                             rtl::OUString& password,
                             rtl::OUString& account);

        virtual bool setHost(const rtl::OUString& host,
                             const rtl::OUString& port,
                             const rtl::OUString& username,
                             const rtl::OUString& password,
                             const rtl::OUString& account);


        struct ServerInfo {
            rtl::OUString host;
            rtl::OUString port;
            rtl::OUString username;
            rtl::OUString password;
            rtl::OUString account;
        };

    private:

        osl::Mutex m_aMutex;
        FTPLoaderThread *m_ftpLoaderThread;

        sal_Int32     m_eType;
        rtl::OUString m_aFtpProxy;
        rtl::OUString m_aNoProxyList;

        // used for access to ftp-proxy
        com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProviderManager > m_xManager;

        com::sun::star::uno::Reference<
        com::sun::star::container::XContainer> m_xContainer;

        std::vector<ServerInfo> m_ServerInfo;

        void init();

        com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >
        getConfiguration() const;

        com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
        getHierAccess(
            const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >& sProvider,
            const char* file ) const;

        rtl::OUString
        getKey(
            const com::sun::star::uno::Reference<
            com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
            const char* key) const;

        sal_Int32
        getIntKey(
            const com::sun::star::uno::Reference<
            com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
            const char* key) const;

        com::sun::star::uno::Reference<com::sun::star::ucb::XContentProvider>
        getHttpProvider()
            throw(com::sun::star::uno::RuntimeException);

        bool ShouldUseFtpProxy(const FTPURL& aURL) const;

    };  // end class FTPContentProvider

}       // end namespace ftp

#endif

