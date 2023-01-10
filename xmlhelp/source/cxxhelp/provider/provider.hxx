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

#pragma once

#include <memory>
#include <rtl/ustring.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace chelp {

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.

#define MYUCP_URL_SCHEME        "vnd.sun.star.help"
inline constexpr OUStringLiteral MYUCP_CONTENT_TYPE = u"application/vnd.sun.star.xmlhelp";    // UCB Content Type.

    class Databases;

    typedef cppu::ImplInheritanceHelper< ::ucbhelper::ContentProviderImplHelper, css::container::XContainerListener, css::lang::XComponent> ContentProvider_Base;
    class ContentProvider : public ContentProvider_Base
    {
    public:
        explicit ContentProvider(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        virtual ~ContentProvider() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XContentProvider
        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL queryContent(
                const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;

        // Additional interfaces

        // XComponent

        virtual void SAL_CALL
        dispose(  ) override;

        virtual void SAL_CALL
        addEventListener( const css::uno::Reference< css::lang::XEventListener >& ) override {}

        virtual void SAL_CALL
        removeEventListener( const css::uno::Reference< css::lang::XEventListener >& ) override {}

        // XContainerListener ( derive from XEventListener )

        virtual void SAL_CALL
        disposing( const css::lang::EventObject& /*Source*/ ) override
        {
            m_xContainer.clear();
        }

        virtual void SAL_CALL
        elementInserted( const css::container::ContainerEvent& ) override {}

        virtual void SAL_CALL
        elementRemoved( const css::container::ContainerEvent& ) override {}

        virtual void SAL_CALL
        elementReplaced( const css::container::ContainerEvent& Event ) override;

        // Non-interface methods.

    private:
        bool                           isInitialized;
        std::unique_ptr<Databases>     m_pDatabases;
        css::uno::Reference<css::container::XContainer> m_xContainer;

        // private methods

        void init();

        static void subst( OUString& instpath );
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
