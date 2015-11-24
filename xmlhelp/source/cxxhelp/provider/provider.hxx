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

#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_PROVIDER_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_PROVIDER_HXX

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

#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME2  "com.sun.star.ucb.HelpContentProvider"

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.

#define MYUCP_URL_SCHEME        "vnd.sun.star.help"
#define MYUCP_CONTENT_TYPE      "application/vnd.sun.star.xmlhelp"    // UCB Content Type.

    class Databases;

    class ContentProvider :
        public ::ucbhelper::ContentProviderImplHelper,
        public css::container::XContainerListener,
        public css::lang::XComponent
    {
    public:
        explicit ContentProvider(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        virtual ~ContentProvider();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
            throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL acquire()
            throw() override;
        virtual void SAL_CALL release()
            throw() override;

        // XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
            throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
            throw( css::uno::RuntimeException, std::exception ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException,
                   std::exception ) override;

        static OUString getImplementationName_Static();

        static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

        static css::uno::Reference< css::lang::XSingleServiceFactory > createServiceFactory(
                const css::uno::Reference< css::lang::XMultiServiceFactory >& rxServiceMgr );

        // XContentProvider
        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL queryContent(
                const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
            throw( css::ucb::IllegalIdentifierException,
                   css::uno::RuntimeException, std::exception ) override;

        // Additional interfaces

        // XComponent

        virtual void SAL_CALL
        dispose(  )
            throw (css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL
        addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override
        {
            (void)xListener;
        }

        virtual void SAL_CALL
        removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
            throw (css::uno::RuntimeException, std::exception) override
        {
            (void)aListener;
        }

        // XConainerListener ( derive from XEventListener )

        virtual void SAL_CALL
        disposing( const css::lang::EventObject& /*Source*/ )
            throw (css::uno::RuntimeException, std::exception) override
        {
            m_xContainer.clear();
        }

        virtual void SAL_CALL
        elementInserted( const css::container::ContainerEvent& Event )
            throw (css::uno::RuntimeException, std::exception) override
        {
            (void)Event;
        }

        virtual void SAL_CALL
        elementRemoved( const css::container::ContainerEvent& Event )
            throw (css::uno::RuntimeException, std::exception) override
        {
            (void)Event;
        }

        virtual void SAL_CALL
        elementReplaced( const css::container::ContainerEvent& Event )
            throw (css::uno::RuntimeException, std::exception) override;

        // Non-interface methods.

    private:

        osl::Mutex     m_aMutex;
        bool           isInitialized;
        OUString  m_aScheme;
        Databases*     m_pDatabases;
        css::uno::Reference<css::container::XContainer> m_xContainer;

        // private methods

        void init();

        css::uno::Reference< css::lang::XMultiServiceFactory >
        getConfiguration() const;

        static css::uno::Reference< css::container::XHierarchicalNameAccess >
        getHierAccess( const css::uno::Reference<  css::lang::XMultiServiceFactory >& sProvider,
                       const char* file );

        static OUString
        getKey( const css::uno::Reference< css::container::XHierarchicalNameAccess >& xHierAccess,
                const char* key );

        static bool
        getBooleanKey(
                    const css::uno::Reference< css::container::XHierarchicalNameAccess >& xHierAccess,
                    const char* key);

        static void subst( OUString& instpath );
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
