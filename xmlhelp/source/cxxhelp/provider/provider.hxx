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

#ifndef _PROVIDER_HXX
#define _PROVIDER_HXX

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>

namespace chelp {

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.

#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME1   "com.sun.star.help.XMLHelp"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME_LENGTH1 25

#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME2  "com.sun.star.ucb.HelpContentProvider"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME_LENGTH2 36

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.

#define MYUCP_URL_SCHEME        "vnd.sun.star.help"
#define MYUCP_URL_SCHEME_LENGTH 18
#define MYUCP_CONTENT_TYPE      "application/vnd.sun.star.xmlhelp"    // UCB Content Type.

    class Databases;

    class ContentProvider :
        public ::ucbhelper::ContentProviderImplHelper,
        public ::com::sun::star::container::XContainerListener,
        public ::com::sun::star::lang::XComponent
    {
    public:
        ContentProvider(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        virtual ~ContentProvider();

        // XInterface
        XINTERFACE_DECL()

            // XTypeProvider
            XTYPEPROVIDER_DECL()

            // XServiceInfo
            XSERVICEINFO_DECL()

            // XContentProvider
            virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContent > SAL_CALL
        queryContent( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::ucb::XContentIdentifier >& Identifier )
            throw( ::com::sun::star::ucb::IllegalIdentifierException,
                   ::com::sun::star::uno::RuntimeException );

        // Additional interfaces

        // XComponent

        virtual void SAL_CALL
        dispose(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)xListener;
        }

        virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)aListener;
        }

        // XConainerListener ( deriver from XEventListener )

        virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)Source;
            m_xContainer = com::sun::star::uno::Reference<com::sun::star::container::XContainer>(0);
        }

        virtual void SAL_CALL
        elementInserted( const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)Event;
        }

        virtual void SAL_CALL
        elementRemoved( const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)Event;
        }

        virtual void SAL_CALL
        elementReplaced( const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException);

        // Non-interface methods.

    private:

        osl::Mutex     m_aMutex;
        bool           isInitialized;
        OUString  m_aScheme;
        Databases*     m_pDatabases;
        com::sun::star::uno::Reference<com::sun::star::container::XContainer> m_xContainer;

        // private methods

        void init();

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
        getConfiguration() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >
        getHierAccess( const ::com::sun::star::uno::Reference<  ::com::sun::star::lang::XMultiServiceFactory >& sProvider,
                       const char* file ) const;

        OUString
        getKey( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
                const char* key ) const;

      sal_Bool
      getBooleanKey(
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
                    const char* key) const;

      void subst( OUString& instpath ) const;
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
